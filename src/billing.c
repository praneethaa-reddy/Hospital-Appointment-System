/*
 * billing.c - Billing module for the hospital appointment system.
 * Handles bill generation, storage, retrieval, and history display.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "billing.h"
#include "utils.h"
#include "appointment.h"
#include "patient.h"
#include "doctor.h"

/* -----------------------------------------------------------------------
 * loadBills
 * Reads up to max Bill records from BILLS_FILE into arr.
 * Returns the number of records loaded (0 if file not found).
 * ----------------------------------------------------------------------- */
int loadBills(Bill arr[], int max) {
    FILE *fp = fopen(BILLS_FILE, "rb");
    if (!fp) return 0;
    int count = (int)fread(arr, sizeof(Bill), (size_t)max, fp);
    fclose(fp);
    return count;
}

/* -----------------------------------------------------------------------
 * saveBills
 * Writes count Bill records from arr to BILLS_FILE (overwrites existing).
 * ----------------------------------------------------------------------- */
void saveBills(Bill arr[], int count) {
    FILE *fp = fopen(BILLS_FILE, "wb");
    if (!fp) {
        printf("Error: Could not open bills file for writing.\n");
        return;
    }
    fwrite(arr, sizeof(Bill), (size_t)count, fp);
    fclose(fp);
}

/* -----------------------------------------------------------------------
 * findBillById
 * Returns the array index of the bill with the given id, or -1 if absent.
 * ----------------------------------------------------------------------- */
int findBillById(Bill arr[], int count, int id) {
    int i;
    for (i = 0; i < count; i++) {
        if (arr[i].id == id) return i;
    }
    return -1;
}

/* -----------------------------------------------------------------------
 * findBillByAppointment
 * Returns the array index of the bill linked to appointment_id, or -1.
 * ----------------------------------------------------------------------- */
int findBillByAppointment(Bill arr[], int count, int appointment_id) {
    int i;
    for (i = 0; i < count; i++) {
        if (arr[i].appointment_id == appointment_id) return i;
    }
    return -1;
}

/* -----------------------------------------------------------------------
 * getNextBillId (internal helper)
 * Returns max bill id + 1, or 2001 if no bills exist.
 * ----------------------------------------------------------------------- */
static int getNextBillId(Bill arr[], int count) {
    if (count == 0) return 2001;
    int maxId = arr[0].id;
    int i;
    for (i = 1; i < count; i++) {
        if (arr[i].id > maxId) maxId = arr[i].id;
    }
    return maxId + 1;
}

/* -----------------------------------------------------------------------
 * hasRepeatVisit (internal helper)
 * Returns 1 if the patient had a STATUS_COMPLETED appointment with the
 * same doctor within the 7 days before today (not including today), else 0.
 * ----------------------------------------------------------------------- */
static int hasRepeatVisit(Appointment appts[], int acount,
                           int patientId, int doctorId) {
    Date today;
    getTodayDate(&today);
    long todayDays = dateToDays(today);
    int i;
    for (i = 0; i < acount; i++) {
        if (appts[i].patient_id != patientId)       continue;
        if (appts[i].doctor_id  != doctorId)         continue;
        if (appts[i].status     != STATUS_COMPLETED) continue;
        long diff = todayDays - dateToDays(appts[i].date);
        /* Repeat visit: completed 1..7 days before today */
        if (diff >= 1 && diff <= 7) return 1;
    }
    return 0;
}

/* -----------------------------------------------------------------------
 * applyDiscount
 * Applies discount_pct (e.g. 10.0 for 10%) to the bill with the given id.
 * Updates the discount and total fields and persists to disk.
 * ----------------------------------------------------------------------- */
void applyDiscount(Bill arr[], int count, int bill_id, float discount_pct) {
    int idx = findBillById(arr, count, bill_id);
    if (idx < 0) {
        printf("Error: Bill ID %d not found.\n", bill_id);
        return;
    }
    float discAmount   = arr[idx].fee * (discount_pct / 100.0f);
    arr[idx].discount += discAmount;
    arr[idx].total     = arr[idx].fee - arr[idx].discount;
    saveBills(arr, count);
}

/* -----------------------------------------------------------------------
 * generateBill
 * Creates a bill for the given appointment, applying senior-citizen (10%)
 * and repeat-visit (5%) discounts as applicable. Saves to BILLS_FILE and
 * prints the formatted receipt. Returns the new bill id, or -1 on error.
 * Does nothing (returns -1) if appointment is already billed.
 * ----------------------------------------------------------------------- */
int generateBill(Bill arr[], int *count,
                 Appointment *appt,
                 Doctor doctors[], int dcount,
                 Date today) {
    /* Reject duplicate billing */
    if (findBillByAppointment(arr, *count, appt->id) >= 0) {
        printf("Error: Appointment %d has already been billed.\n", appt->id);
        return -1;
    }

    if (appt->status != STATUS_COMPLETED) {
        printf("Error: Cannot bill appointment %d - status is not Completed.\n", appt->id);
        return -1;
    }

    /* Locate the doctor for base fee */
    int didx = findDoctorById(doctors, dcount, appt->doctor_id);
    if (didx < 0) {
        printf("Error: Doctor ID %d not found.\n", appt->doctor_id);
        return -1;
    }
    Doctor *doctor = &doctors[didx];

    /* Load patient for discount evaluation */
    Patient patients[MAX_PATIENTS];
    int pcount = loadPatients(patients, MAX_PATIENTS);
    int pidx = findPatientById(patients, pcount, appt->patient_id);
    if (pidx < 0) {
        printf("Error: Patient ID %d not found.\n", appt->patient_id);
        return -1;
    }
    Patient *patient = &patients[pidx];

    /* Load all appointments for repeat-visit check */
    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    /* Calculate fee and discounts */
    float baseFee    = doctor->fee;
    float seniorDisc = 0.0f;
    float repeatDisc = 0.0f;

    if (patient->age >= 60) {
        seniorDisc = baseFee * 0.10f;
    }
    if (hasRepeatVisit(appts, acount, patient->id, doctor->id)) {
        repeatDisc = baseFee * 0.05f;
    }

    float totalDiscount = seniorDisc + repeatDisc;
    float total         = baseFee - totalDiscount;

    if (*count >= MAX_BILLS) {
        printf("Error: Bill storage is full.\n");
        return -1;
    }

    /* Build and append the new bill */
    Bill *b        = &arr[*count];
    b->id             = getNextBillId(arr, *count);
    b->appointment_id = appt->id;
    b->fee            = baseFee;
    b->discount       = totalDiscount;
    b->total          = total;
    b->date           = today;
    (*count)++;

    saveBills(arr, *count);

    /* Print the formatted receipt */
    char dateStr[12];
    dateToString(b->date, dateStr);
    printf("\n------------- BILL No: %04d -------------\n", b->id);
    printf("Date: %s\n", dateStr);
    printf("Patient : %s (ID %04d)\n", patient->name, patient->id);
    printf("Doctor  : Dr. %s (%s)\n", doctor->name, doctor->specialization);
    printf("-----------------------------------------\n");
    printf("%-32s%8.2f\n", "Fee", b->fee);
    if (seniorDisc > 0.0f) {
        printf("%-32s%8.2f\n", "Senior Citizen Discount", -seniorDisc);
    }
    if (repeatDisc > 0.0f) {
        printf("%-32s%8.2f\n", "Repeat Visit Discount", -repeatDisc);
    }
    printf("-----------------------------------------\n");
    printf("%-32s%8.2f\n", "Total Payable", b->total);
    printf("Wishing you a speedy recovery.\n");
    printf("-----------------------------------------\n\n");

    return b->id;
}

/* -----------------------------------------------------------------------
 * generateBillById
 * Standalone bill generation helper for an appointment ID.
 * ----------------------------------------------------------------------- */
void generateBillById(int appointmentId) {
    Bill bills[MAX_BILLS];
    int bcount = loadBills(bills, MAX_BILLS);

    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    int aidx = findAppointmentById(appts, acount, appointmentId);
    if (aidx < 0) {
        printf("Error: Appointment ID %d not found.\n", appointmentId);
        return;
    }

    Doctor docs[MAX_DOCTORS];
    int dcount = loadDoctors(docs, MAX_DOCTORS);

    Date today;
    getTodayDate(&today);

    generateBill(bills, &bcount, &appts[aidx], docs, dcount, today);
}

/* -----------------------------------------------------------------------
 * printBill
 * Prints a formatted invoice for the bill with the given id,
 * resolving patient, doctor, and appointment details from their arrays.
 * ----------------------------------------------------------------------- */
void printBill(Bill arr[], int count,
               Appointment appointments[], int acount,
               Patient patients[], int pcount,
               Doctor doctors[], int dcount,
               int bill_id) {
    int bidx = findBillById(arr, count, bill_id);
    if (bidx < 0) {
        printf("Error: Bill ID %d not found.\n", bill_id);
        return;
    }
    Bill *b = &arr[bidx];

    int apptIdx = findAppointmentById(appointments, acount, b->appointment_id);
    if (apptIdx < 0) {
        printf("Error: Associated appointment not found for Bill %d.\n", b->id);
        return;
    }
    Appointment *appt = &appointments[apptIdx];

    int pidx = findPatientById(patients, pcount, appt->patient_id);
    int didx = findDoctorById(doctors, dcount, appt->doctor_id);
    if (pidx < 0 || didx < 0) {
        printf("Error: Patient or doctor record missing for Bill %d.\n", b->id);
        return;
    }
    Patient *patient = &patients[pidx];
    Doctor  *doctor  = &doctors[didx];

    /*
     * Reconstruct individual discount amounts for display.
     * Senior citizen (10%) is computed first; remainder is repeat-visit.
     */
    float seniorDisc = 0.0f;
    float repeatDisc = 0.0f;
    if (patient->age >= 60) {
        seniorDisc = b->fee * 0.10f;
    }
    repeatDisc = b->discount - seniorDisc;
    if (repeatDisc < 0.0f) repeatDisc = 0.0f;

    char dateStr[12];
    dateToString(b->date, dateStr);
    printf("\n------------- BILL No: %04d -------------\n", b->id);
    printf("Date: %s\n", dateStr);
    printf("Patient : %s (ID %04d)\n", patient->name, patient->id);
    printf("Doctor  : Dr. %s (%s)\n", doctor->name, doctor->specialization);
    printf("-----------------------------------------\n");
    printf("%-32s%8.2f\n", "Fee", b->fee);
    if (seniorDisc > 0.0f) {
        printf("%-32s%8.2f\n", "Senior Citizen Discount", -seniorDisc);
    }
    if (repeatDisc > 0.0f) {
        printf("%-32s%8.2f\n", "Repeat Visit Discount", -repeatDisc);
    }
    printf("-----------------------------------------\n");
    printf("%-32s%8.2f\n", "Total Payable", b->total);
    printf("Wishing you a speedy recovery.\n");
    printf("-----------------------------------------\n\n");
}

/* -----------------------------------------------------------------------
 * billingMenu
 * Entry point for the billing management sub-menu.
 * Handles bill generation, viewing, history, and discount application.
 * ----------------------------------------------------------------------- */
void billingMenu(Bill arr[], int *bcount,
                 Appointment appointments[], int acount,
                 Patient patients[], int pcount,
                 Doctor doctors[], int dcount,
                 Date today) {
    int choice;
    int running = 1;

    while (running) {
        printf("\n--- Billing Menu ---\n");
        printf("1. Generate Bill for Appointment\n");
        printf("2. View Bill by ID\n");
        printf("3. Bill History\n");
        printf("4. Apply Additional Discount\n");
        printf("0. Back\n");
        printf("Enter choice: ");

        if (!readInt(&choice)) {
            printf("Invalid input.\n");
            continue;
        }

        switch (choice) {
            case 1: {
                int apptId;
                printf("Enter Appointment ID: ");
                if (!readInt(&apptId)) { printf("Invalid input.\n"); break; }
                int apptIdx = findAppointmentById(appointments, acount, apptId);
                if (apptIdx < 0) {
                    printf("Error: Appointment ID %d not found.\n", apptId);
                    break;
                }
                generateBill(arr, bcount, &appointments[apptIdx], doctors, dcount, today);
                break;
            }
            case 2: {
                int billId;
                printf("Enter Bill ID: ");
                if (!readInt(&billId)) { printf("Invalid input.\n"); break; }
                printBill(arr, *bcount, appointments, acount,
                          patients, pcount, doctors, dcount, billId);
                break;
            }
            case 3: {
                /* Bill history: search by date range or patient ID */
                int sub;
                printf("\n--- Bill History ---\n");
                printf("1. Search by Date Range\n");
                printf("2. Search by Patient ID\n");
                printf("Enter choice: ");
                if (!readInt(&sub)) { printf("Invalid input.\n"); break; }

                if (sub == 1) {
                    /* Date-range search */
                    Date start, end;
                    printf("Enter start date:\n");
                    printf("  Day: ");   if (!readInt(&start.day))   { printf("Invalid.\n"); break; }
                    printf("  Month: "); if (!readInt(&start.month)) { printf("Invalid.\n"); break; }
                    printf("  Year: ");  if (!readInt(&start.year))  { printf("Invalid.\n"); break; }
                    printf("Enter end date:\n");
                    printf("  Day: ");   if (!readInt(&end.day))   { printf("Invalid.\n"); break; }
                    printf("  Month: "); if (!readInt(&end.month)) { printf("Invalid.\n"); break; }
                    printf("  Year: ");  if (!readInt(&end.year))  { printf("Invalid.\n"); break; }

                    if (!isValidDate(start) || !isValidDate(end)) {
                        printf("Error: Invalid date.\n"); break;
                    }
                    if (compareDates(start, end) > 0) {
                        printf("Error: Start date after end date.\n"); break;
                    }

                    int found = 0, i;
                    for (i = 0; i < *bcount; i++) {
                        Bill *b = &arr[i];
                        if (compareDates(b->date, start) < 0) continue;
                        if (compareDates(b->date, end)   > 0) continue;
                        int ai = findAppointmentById(appointments, acount, b->appointment_id);
                        if (ai < 0) continue;
                        int pi = findPatientById(patients, pcount, appointments[ai].patient_id);
                        int di = findDoctorById(doctors, dcount, appointments[ai].doctor_id);
                        if (pi < 0 || di < 0) continue;
                        char ds[12]; dateToString(b->date, ds);
                        printf("Bill %04d | %s | %-20s | Dr. %-20s | Total: %.2f\n",
                               b->id, ds, patients[pi].name, doctors[di].name, b->total);
                        found = 1;
                    }
                    if (!found) printf("No bills found in that date range.\n");

                } else if (sub == 2) {
                    /* Patient ID search */
                    int patId;
                    printf("Enter Patient ID: ");
                    if (!readInt(&patId)) { printf("Invalid input.\n"); break; }
                    int pi = findPatientById(patients, pcount, patId);
                    if (pi < 0) { printf("Error: Patient %d not found.\n", patId); break; }

                    int found = 0, i;
                    for (i = 0; i < *bcount; i++) {
                        Bill *b = &arr[i];
                        int ai = findAppointmentById(appointments, acount, b->appointment_id);
                        if (ai < 0) continue;
                        if (appointments[ai].patient_id != patId) continue;
                        int di = findDoctorById(doctors, dcount, appointments[ai].doctor_id);
                        if (di < 0) continue;
                        char ds[12]; dateToString(b->date, ds);
                        printf("Bill %04d | %s | Dr. %-20s | Total: %.2f\n",
                               b->id, ds, doctors[di].name, b->total);
                        found = 1;
                    }
                    if (!found) printf("No bills found for Patient ID %d.\n", patId);
                } else {
                    printf("Invalid choice.\n");
                }
                break;
            }
            case 4: {
                int billId;
                float pct;
                printf("Enter Bill ID: ");
                if (!readInt(&billId)) { printf("Invalid input.\n"); break; }
                printf("Enter discount percentage: ");
                char buf[32];
                readString(buf, sizeof(buf));
                if (sscanf(buf, "%f", &pct) != 1 || pct < 0.0f || pct > 100.0f) {
                    printf("Invalid percentage.\n"); break;
                }
                applyDiscount(arr, *bcount, billId, pct);
                printf("Discount applied.\n");
                break;
            }
            case 0:
                running = 0;
                break;
            default:
                printf("Invalid choice.\n");
                break;
        }
    }
}
