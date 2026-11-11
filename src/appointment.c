/*
 * appointment.c - Appointment scheduling, queue management, and emergency handling.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h"
#include "appointment.h"
#include "patient.h"
#include "doctor.h"
#include "billing.h"
#include "utils.h"

/* Loads appointment records from APPOINTMENTS_FILE into arr; returns record count. */
int loadAppointments(Appointment arr[], int max)
{
    FILE *fp = fopen(APPOINTMENTS_FILE, "rb");
    if (!fp) return 0;
    int count = (int)fread(arr, sizeof(Appointment), (size_t)max, fp);
    fclose(fp);
    return count;
}

/* Writes count appointment records from arr to APPOINTMENTS_FILE. */
void saveAppointments(Appointment arr[], int count)
{
    FILE *fp = fopen(APPOINTMENTS_FILE, "wb");
    if (!fp) {
        printf("Error: Unable to open appointments file for writing.\n");
        return;
    }
    fwrite(arr, sizeof(Appointment), (size_t)count, fp);
    fclose(fp);
}

/* Returns the next available appointment ID. */
int getNextAppointmentId(Appointment arr[], int count)
{
    if (count == 0) return 1;
    int max_id = arr[0].id;
    for (int i = 1; i < count; i++) {
        if (arr[i].id > max_id) max_id = arr[i].id;
    }
    return max_id + 1;
}

/* Returns array index of the appointment with the given id, or -1 if not found. */
int findAppointmentById(Appointment arr[], int count, int id)
{
    for (int i = 0; i < count; i++) {
        if (arr[i].id == id) return i;
    }
    return -1;
}

/* Checks if a doctor's slot is already booked on a date in arr. */
int isSlotBooked(Appointment arr[], int count, int doctor_id, Date date, int slot)
{
    for (int i = 0; i < count; i++) {
        if (arr[i].doctor_id == doctor_id &&
            compareDates(arr[i].date, date) == 0 &&
            arr[i].slot == slot &&
            arr[i].status == STATUS_BOOKED) {
            return 1;
        }
    }
    return 0;
}

/* Checks if a slot is free by reading from APPOINTMENTS_FILE directly. */
int isSlotFree(int doctorId, Date d, int slot)
{
    Appointment arr[MAX_APPOINTMENTS];
    int count = loadAppointments(arr, MAX_APPOINTMENTS);
    return !isSlotBooked(arr, count, doctorId, d, slot);
}

/* Checks if a patient has an existing booked appointment in the same slot and date. */
static int hasPatientConflict(int patientId, Date d, int slot, int excludeApptId)
{
    Appointment arr[MAX_APPOINTMENTS];
    int count = loadAppointments(arr, MAX_APPOINTMENTS);
    for (int i = 0; i < count; i++) {
        if (arr[i].id == excludeApptId) continue;
        if (arr[i].patient_id == patientId &&
            compareDates(arr[i].date, d) == 0 &&
            arr[i].slot == slot &&
            arr[i].status == STATUS_BOOKED) {
            return 1;
        }
    }
    return 0;
}

/* Counts booked slots for a doctor on a specific date. */
int countBookedSlots(Appointment arr[], int count, int doctor_id, Date date)
{
    int booked = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i].doctor_id == doctor_id &&
            compareDates(arr[i].date, date) == 0 &&
            arr[i].status == STATUS_BOOKED) {
            booked++;
        }
    }
    return booked;
}

/* Counts today's appointments that are currently booked. */
int countTodayAppointments(Appointment arr[], int count, Date today)
{
    int total = 0;
    for (int i = 0; i < count; i++) {
        if (compareDates(arr[i].date, today) == 0 &&
            arr[i].status == STATUS_BOOKED) {
            total++;
        }
    }
    return total;
}

/* Loads waiting queue from WAITING_FILE into queue; returns count. */
int loadWaiting(WaitingEntry queue[], int max)
{
    FILE *fp = fopen(WAITING_FILE, "rb");
    if (!fp) return 0;
    int count = (int)fread(queue, sizeof(WaitingEntry), (size_t)max, fp);
    fclose(fp);
    return count;
}

/* Saves waiting queue to WAITING_FILE. */
void saveWaiting(WaitingEntry queue[], int count)
{
    FILE *fp = fopen(WAITING_FILE, "wb");
    if (!fp) {
        printf("Error: Unable to open waiting list file for writing.\n");
        return;
    }
    fwrite(queue, sizeof(WaitingEntry), (size_t)count, fp);
    fclose(fp);
}

/* Returns the total number of waiting entries in the queue. */
int countWaiting(WaitingEntry queue[], int count)
{
    (void)queue;
    return count;
}

/* Appends an entry to the waiting queue (emergency entries placed before routine). */
void addToWaitingList(int patientId, int doctorId, Date d, int type)
{
    WaitingEntry queue[MAX_WAITING];
    int count = loadWaiting(queue, MAX_WAITING);

    if (count >= MAX_WAITING) {
        printf("Error: Waiting list is full.\n");
        return;
    }

    WaitingEntry entry;
    entry.patient_id = patientId;
    entry.doctor_id  = doctorId;
    entry.date       = d;
    entry.type       = type;

    if (type == TYPE_EMERGENCY) {
        /* Insert at the end of the emergency block, before routine ones */
        int insert_idx = 0;
        while (insert_idx < count && queue[insert_idx].type == TYPE_EMERGENCY) {
            insert_idx++;
        }
        for (int i = count; i > insert_idx; i--) {
            queue[i] = queue[i - 1];
        }
        queue[insert_idx] = entry;
    } else {
        queue[count] = entry;
    }

    count++;
    saveWaiting(queue, count);
    printf("Patient ID %d added to waiting list for Dr. %d.\n", patientId, doctorId);
}

/* Books an appointment record directly with clash validation. */
int bookAppointment(int patientId, int doctorId, Date d, int slot, int type)
{
    if (!isSlotFree(doctorId, d, slot)) {
        printf("Error: Selected slot %d is already booked.\n", slot);
        return -1;
    }

    if (hasPatientConflict(patientId, d, slot, -1)) {
        printf("Error: Patient already has an appointment booked at this slot.\n");
        return -1;
    }

    Appointment arr[MAX_APPOINTMENTS];
    int count = loadAppointments(arr, MAX_APPOINTMENTS);

    if (count >= MAX_APPOINTMENTS) {
        printf("Error: Appointment database is full.\n");
        return -1;
    }

    Appointment appt;
    memset(&appt, 0, sizeof(Appointment));
    appt.id         = getNextAppointmentId(arr, count);
    appt.patient_id = patientId;
    appt.doctor_id  = doctorId;
    appt.date       = d;
    appt.slot       = slot;
    appt.type       = type;
    appt.status     = STATUS_BOOKED;
    if (type == TYPE_EMERGENCY) {
        strncpy(appt.notes, "Emergency Booking", sizeof(appt.notes) - 1);
    } else {
        strncpy(appt.notes, "Routine Consultation", sizeof(appt.notes) - 1);
    }

    arr[count] = appt;
    count++;
    saveAppointments(arr, count);

    return appt.id;
}

/* Promotes the first waiting patient into a freed slot. */
int promoteFromWaitingList(int doctorId, Date d, int slot)
{
    WaitingEntry queue[MAX_WAITING];
    int count = loadWaiting(queue, MAX_WAITING);

    int target_idx = -1;
    for (int i = 0; i < count; i++) {
        if (queue[i].doctor_id == doctorId &&
            compareDates(queue[i].date, d) == 0) {
            target_idx = i;
            break;
        }
    }

    if (target_idx == -1) return 0;

    WaitingEntry promoted = queue[target_idx];

    /* Dequeue and shift */
    for (int i = target_idx; i < count - 1; i++) {
        queue[i] = queue[i + 1];
    }
    count--;
    saveWaiting(queue, count);

    /* Book the slot for promoted patient */
    int appt_id = bookAppointment(promoted.patient_id, promoted.doctor_id,
                                  promoted.date, slot, promoted.type);
    if (appt_id > 0) {
        printf(COLOR_GREEN "\n[WAITING LIST PROMOTION] Patient ID %d automatically booked for Slot %d!\n" COLOR_RESET,
               promoted.patient_id, slot);
        return 1;
    }
    return 0;
}

/* Cancels an appointment and automatically triggers waiting list promotion. */
void cancelAppointment(int appointmentId)
{
    Appointment arr[MAX_APPOINTMENTS];
    int count = loadAppointments(arr, MAX_APPOINTMENTS);

    int idx = findAppointmentById(arr, count, appointmentId);
    if (idx < 0) {
        printf("Error: Appointment ID %d not found.\n", appointmentId);
        return;
    }

    if (arr[idx].status != STATUS_BOOKED) {
        printf("Error: Only booked appointments can be cancelled.\n");
        return;
    }

    arr[idx].status = STATUS_CANCELLED;
    saveAppointments(arr, count);

    int doc_id = arr[idx].doctor_id;
    Date d     = arr[idx].date;
    int slot   = arr[idx].slot;

    printColored("Appointment successfully cancelled. Slot is now free.\n", COLOR_GREEN);

    /* Automatically promote waiting list */
    promoteFromWaitingList(doc_id, d, slot);
}

/* Interactively cancels an appointment. */
void cancelAppointmentInteractive(void)
{
    printHeader("CANCEL APPOINTMENT");
    int apptId;
    printf("Enter Appointment ID to cancel: ");
    if (!readInt(&apptId)) return;

    cancelAppointment(apptId);
}

/* Interactively reschedules an existing booked appointment. */
void rescheduleAppointmentInteractive(Doctor doctors[], int dcount, Date today)
{
    printHeader("RESCHEDULE APPOINTMENT");
    int apptId;
    printf("Enter Appointment ID to reschedule: ");
    if (!readInt(&apptId)) return;

    Appointment arr[MAX_APPOINTMENTS];
    int count = loadAppointments(arr, MAX_APPOINTMENTS);
    int idx = findAppointmentById(arr, count, apptId);
    if (idx < 0) {
        printf("Error: Appointment ID %d not found.\n", apptId);
        return;
    }

    if (arr[idx].status != STATUS_BOOKED) {
        printf("Error: Appointment cannot be rescheduled (current status is not Booked).\n");
        return;
    }

    int didx = findDoctorById(doctors, dcount, arr[idx].doctor_id);
    if (didx < 0) {
        printf("Error: Associated doctor not found.\n");
        return;
    }

    Doctor *doc = &doctors[didx];
    printf("Current Appointment: Dr. %s | Slot %d\n", doc->name, arr[idx].slot);

    Date new_date;
    while (1) {
        char dbuf[32];
        printf("Enter New Date (DD-MM-YYYY): ");
        readString(dbuf, sizeof(dbuf));
        if (sscanf(dbuf, "%d-%d-%d", &new_date.day, &new_date.month, &new_date.year) != 3 ||
            !isValidDate(new_date)) {
            printf("Invalid date. Please use DD-MM-YYYY.\n");
            continue;
        }

        if (compareDates(new_date, today) < 0) {
            printf("Past dates are rejected.\n");
            continue;
        }

        long diff = dateToDays(new_date) - dateToDays(today);
        if (diff > 30) {
            printf("Appointments can only be booked up to 30 days ahead.\n");
            continue;
        }

        int wd = getWeekday(new_date);
        if (doc->available_days[wd] != '1') {
            printf("Doctor is not available on this day.\n");
            continue;
        }
        break;
    }

    /* Display slots for new date */
    char date_buf[16];
    dateToString(new_date, date_buf);
    printf("\n%s (%s)  Date: %s\n", doc->name, doc->specialization, date_buf);
    printf("%-5s %-12s %-10s\n", "Slot", "Time", "Status");
    printSeparator();

    for (int s = 1; s <= doc->slots_per_day; s++) {
        char tbuf[16];
        slotToTime(s, tbuf);
        int booked = isSlotBooked(arr, count, doc->id, new_date, s);
        printf("%-5d %-12s %-10s\n", s, tbuf, booked ? "Booked" : "FREE");
    }
    printSeparator();

    int new_slot;
    printf("Choose new slot number: ");
    if (!readInt(&new_slot) || new_slot < 1 || new_slot > doc->slots_per_day) {
        printf("Invalid slot selection.\n");
        return;
    }

    if (isSlotBooked(arr, count, doc->id, new_date, new_slot)) {
        printf("Error: Slot %d is already booked.\n", new_slot);
        return;
    }

    if (hasPatientConflict(arr[idx].patient_id, new_date, new_slot, apptId)) {
        printf("Error: Patient already has an appointment booked at this slot.\n");
        return;
    }

    /* Save old slot info for promotion */
    int old_slot = arr[idx].slot;
    Date old_date = arr[idx].date;
    int old_doc = arr[idx].doctor_id;

    arr[idx].date = new_date;
    arr[idx].slot = new_slot;
    saveAppointments(arr, count);

    printColored("Appointment successfully rescheduled.\n", COLOR_GREEN);

    /* Promote waiting list into old slot */
    promoteFromWaitingList(old_doc, old_date, old_slot);
}

/* Interactively books an appointment with comprehensive clash prevention. */
int bookAppointmentInteractive(Patient patients[], int pcount, Doctor doctors[], int dcount, Date today)
{
    printHeader("BOOK APPOINTMENT");

    if (pcount == 0) {
        printf("No registered patients found. Please register a patient first.\n");
        return -1;
    }
    if (dcount == 0) {
        printf("No active doctors found.\n");
        return -1;
    }

    int patientId;
    printf("Enter Patient ID: ");
    if (!readInt(&patientId)) return -1;

    int pidx = findPatientById(patients, pcount, patientId);
    if (pidx < 0) {
        printf("Patient ID %d not found.\n", patientId);
        return -1;
    }
    printf("Patient: %s (Age: %d)\n", patients[pidx].name, patients[pidx].age);

    /* Choose Doctor */
    printf("\nAvailable Doctors:\n");
    for (int i = 0; i < dcount; i++) {
        if (!doctors[i].is_active) continue;
        printf("  %d. Dr. %-20s (%-15s) Fee: %.2f\n",
               doctors[i].id, doctors[i].name, doctors[i].specialization, doctors[i].fee);
    }

    int doctorId;
    printf("\nEnter Doctor ID: ");
    if (!readInt(&doctorId)) return -1;

    int didx = findDoctorById(doctors, dcount, doctorId);
    if (didx < 0 || !doctors[didx].is_active) {
        printf("Doctor not found or inactive.\n");
        return -1;
    }
    Doctor *doc = &doctors[didx];

    /* Date input & validations */
    Date book_date;
    while (1) {
        char dbuf[32];
        printf("Enter Appointment Date (DD-MM-YYYY): ");
        readString(dbuf, sizeof(dbuf));
        if (sscanf(dbuf, "%d-%d-%d", &book_date.day, &book_date.month, &book_date.year) != 3 ||
            !isValidDate(book_date)) {
            printf("Invalid date. Please use DD-MM-YYYY format.\n");
            continue;
        }

        if (compareDates(book_date, today) < 0) {
            printf("Past dates are rejected.\n");
            continue;
        }

        long diff = dateToDays(book_date) - dateToDays(today);
        if (diff > 30) {
            printf("Appointments can only be booked up to 30 days ahead.\n");
            continue;
        }

        int wd = getWeekday(book_date);
        if (doc->available_days[wd] != '1') {
            printf("Doctor does not consult on this weekday.\n");
            continue;
        }
        break;
    }

    /* Check free slots */
    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    char dstr[16];
    dateToString(book_date, dstr);
    printf("\nDr. %s (%s)  Date: %s\n", doc->name, doc->specialization, dstr);
    printf("%-5s %-12s %-10s\n", "Slot", "Time", "Status");
    printSeparator();

    int free_count = 0;
    for (int s = 1; s <= doc->slots_per_day; s++) {
        char tbuf[16];
        slotToTime(s, tbuf);
        int booked = isSlotBooked(appts, acount, doc->id, book_date, s);
        if (!booked) free_count++;
        printf("%-5d %-12s %-10s\n", s, tbuf, booked ? "Booked" : "FREE");
    }
    printSeparator();

    if (free_count == 0) {
        printf("All slots are booked for Dr. %s on this date.\n", doc->name);
        printf("Would you like to join the Waiting List? (Y/N): ");
        char resp[8];
        readString(resp, sizeof(resp));
        if (toupper((unsigned char)resp[0]) == 'Y') {
            addToWaitingList(patientId, doc->id, book_date, TYPE_ROUTINE);
            printColored("Added to waiting list.\n", COLOR_GREEN);
        }
        return -1;
    }

    int slot_choice;
    printf("Choose a slot number: ");
    if (!readInt(&slot_choice) || slot_choice < 1 || slot_choice > doc->slots_per_day) {
        printf("Invalid slot selection.\n");
        return -1;
    }

    int appt_id = bookAppointment(patientId, doc->id, book_date, slot_choice, TYPE_ROUTINE);
    if (appt_id > 0) {
        printColored("Appointment successfully booked!\n", COLOR_GREEN);
        printf("Appointment ID: %d\n", appt_id);
    }
    return appt_id;
}

/* Interactively performs emergency booking with queue prioritization. */
int emergencyBookingInteractive(Patient patients[], int pcount, Doctor doctors[], int dcount, Date today)
{
    printHeader("EMERGENCY BOOKING");

    if (pcount == 0 || dcount == 0) {
        printf("Patients or doctors not configured.\n");
        return -1;
    }

    int patientId;
    printf("Enter Patient ID: ");
    if (!readInt(&patientId)) return -1;

    int pidx = findPatientById(patients, pcount, patientId);
    if (pidx < 0) {
        printf("Patient ID %d not found.\n", patientId);
        return -1;
    }

    printf("\nAvailable Doctors:\n");
    for (int i = 0; i < dcount; i++) {
        if (!doctors[i].is_active) continue;
        printf("  %d. Dr. %-20s (%-15s)\n",
               doctors[i].id, doctors[i].name, doctors[i].specialization);
    }

    int doctorId;
    printf("\nEnter Doctor ID: ");
    if (!readInt(&doctorId)) return -1;

    int didx = findDoctorById(doctors, dcount, doctorId);
    if (didx < 0) {
        printf("Doctor not found.\n");
        return -1;
    }
    Doctor *doc = &doctors[didx];

    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    /* Find earliest free slot today */
    int assigned_slot = -1;
    for (int s = 1; s <= doc->slots_per_day; s++) {
        if (!isSlotBooked(appts, acount, doc->id, today, s)) {
            assigned_slot = s;
            break;
        }
    }

    if (assigned_slot != -1) {
        int appt_id = bookAppointment(patientId, doc->id, today, assigned_slot, TYPE_EMERGENCY);
        if (appt_id > 0) {
            char tbuf[16];
            slotToTime(assigned_slot, tbuf);
            printColored("Emergency appointment successfully booked at earliest slot today!\n", COLOR_GREEN);
            printf("Appointment ID : %d\n", appt_id);
            printf("Assigned Slot  : %d (%s)\n", assigned_slot, tbuf);
            return appt_id;
        }
    } else {
        printf("All slots are booked for Dr. %s today.\n", doc->name);
        printf("Adding patient to Emergency Waiting List with top priority.\n");
        addToWaitingList(patientId, doc->id, today, TYPE_EMERGENCY);
        printColored("Patient placed at front of waiting list.\n", COLOR_GREEN);
    }
    return -1;
}

/* Displays all appointments for a date grouped by doctor. */
void showDailySchedule(Date d)
{
    char dstr[16];
    dateToString(d, dstr);

    char title[64];
    snprintf(title, sizeof(title), "DAILY SCHEDULE FOR %s", dstr);
    printHeader(title);

    Doctor doctors[MAX_DOCTORS];
    int dcount = loadDoctors(doctors, MAX_DOCTORS);

    Patient patients[MAX_PATIENTS];
    int pcount = loadPatients(patients, MAX_PATIENTS);

    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    int total_shown = 0;

    for (int didx = 0; didx < dcount; didx++) {
        if (!doctors[didx].is_active) continue;
        int doc_id = doctors[didx].id;

        int doc_has_appts = 0;
        for (int i = 0; i < acount; i++) {
            if (appts[i].doctor_id == doc_id && compareDates(appts[i].date, d) == 0) {
                doc_has_appts = 1;
                break;
            }
        }
        if (!doc_has_appts) continue;

        printf("\nDr. %s (%s) [Fee: %.2f]\n",
               doctors[didx].name, doctors[didx].specialization, doctors[didx].fee);
        printf("%-5s %-12s %-20s %-12s %-10s\n",
               "Slot", "Time", "Patient", "Type", "Status");
        printSeparator();

        for (int slot = 1; slot <= doctors[didx].slots_per_day; slot++) {
            for (int i = 0; i < acount; i++) {
                if (appts[i].doctor_id == doc_id &&
                    compareDates(appts[i].date, d) == 0 &&
                    appts[i].slot == slot) {

                    char tbuf[16];
                    slotToTime(slot, tbuf);

                    int pidx = findPatientById(patients, pcount, appts[i].patient_id);
                    char pname[64] = "Unknown";
                    if (pidx >= 0) snprintf(pname, sizeof(pname), "%s", patients[pidx].name);

                    const char *tname = (appts[i].type == TYPE_EMERGENCY) ? "Emergency" : "Routine";
                    const char *sname = "Booked";
                    const char *scolor = COLOR_YELLOW;

                    if (appts[i].status == STATUS_COMPLETED) {
                        sname = "Completed";
                        scolor = COLOR_GREEN;
                    } else if (appts[i].status == STATUS_CANCELLED) {
                        sname = "Cancelled";
                        scolor = COLOR_RED;
                    } else if (appts[i].status == STATUS_NOSHOW) {
                        sname = "No-Show";
                        scolor = COLOR_RED;
                    }

                    printf("%-5d %-12s %-20s %-12s ", slot, tbuf, pname, tname);
                    printColored(sname, scolor);
                    printf("\n");
                    total_shown++;
                }
            }
        }
        printSeparator();
    }

    if (total_shown == 0) {
        printf("No appointments scheduled for %s.\n", dstr);
    }
}

/* Interactively updates appointment status to Completed or No-show. */
void markAppointmentStatus(Doctor doctors[], int dcount, Date today)
{
    printHeader("UPDATE APPOINTMENT STATUS");
    int apptId;
    printf("Enter Appointment ID: ");
    if (!readInt(&apptId)) return;

    Appointment appts[MAX_APPOINTMENTS];
    int count = loadAppointments(appts, MAX_APPOINTMENTS);
    int idx = findAppointmentById(appts, count, apptId);
    if (idx < 0) {
        printf("Appointment ID %d not found.\n", apptId);
        return;
    }

    printf("Current Status: ");
    if (appts[idx].status == STATUS_BOOKED) printColored("Booked\n", COLOR_YELLOW);
    else if (appts[idx].status == STATUS_COMPLETED) printColored("Completed\n", COLOR_GREEN);
    else if (appts[idx].status == STATUS_CANCELLED) printColored("Cancelled\n", COLOR_RED);
    else printColored("No-Show\n", COLOR_RED);

    printf("\nChoose New Status:\n");
    printf("1. Completed\n");
    printf("2. No-Show\n");
    printf("0. Cancel\n");
    printf("Select option: ");

    int choice;
    if (!readInt(&choice)) return;

    if (choice == 1) {
        appts[idx].status = STATUS_COMPLETED;
        saveAppointments(appts, count);
        printColored("Appointment marked as COMPLETED.\n", COLOR_GREEN);

        printf("\nGenerate consultation bill now? (Y/N): ");
        char resp[8];
        readString(resp, sizeof(resp));
        if (toupper((unsigned char)resp[0]) == 'Y') {
            Bill bills[MAX_BILLS];
            int bcount = loadBills(bills, MAX_BILLS);
            generateBill(bills, &bcount, &appts[idx], doctors, dcount, today);
        }
    } else if (choice == 2) {
        appts[idx].status = STATUS_NOSHOW;
        saveAppointments(appts, count);
        printColored("Appointment marked as NO-SHOW.\n", COLOR_RED);
    }
}

/* Interactively displays free slots for a doctor on a selected date. */
void viewFreeSlotsInteractive(Doctor doctors[], int dcount)
{
    printHeader("VIEW FREE SLOTS");
    if (dcount == 0) {
        printf("No doctors found.\n");
        return;
    }

    int doctorId;
    printf("Enter Doctor ID: ");
    if (!readInt(&doctorId)) return;

    int didx = findDoctorById(doctors, dcount, doctorId);
    if (didx < 0) {
        printf("Doctor ID %d not found.\n", doctorId);
        return;
    }
    Doctor *doc = &doctors[didx];

    Date d;
    while (1) {
        char dbuf[32];
        printf("Enter Date (DD-MM-YYYY): ");
        readString(dbuf, sizeof(dbuf));
        if (sscanf(dbuf, "%d-%d-%d", &d.day, &d.month, &d.year) == 3 && isValidDate(d)) break;
        printf("Invalid date.\n");
    }

    int wd = getWeekday(d);
    if (doc->available_days[wd] != '1') {
        printf("Doctor does not consult on this day.\n");
        return;
    }

    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    WaitingEntry queue[MAX_WAITING];
    int wcount = loadWaiting(queue, MAX_WAITING);

    char dstr[16];
    dateToString(d, dstr);
    printf("\n%s (%s)  Date: %s\n", doc->name, doc->specialization, dstr);
    printf("%-5s %-12s %-10s\n", "Slot", "Time", "Status");
    printSeparator();

    int free_count = 0;
    for (int s = 1; s <= doc->slots_per_day; s++) {
        char tbuf[16];
        slotToTime(s, tbuf);
        int booked = isSlotBooked(appts, acount, doc->id, d, s);
        if (!booked) free_count++;
        printf("%-5d %-12s %-10s\n", s, tbuf, booked ? "Booked" : "FREE");
    }
    printSeparator();

    int w_for_doc = 0;
    for (int i = 0; i < wcount; i++) {
        if (queue[i].doctor_id == doc->id && compareDates(queue[i].date, d) == 0) {
            w_for_doc++;
        }
    }
    printf("Free slots: %d/%d | Waiting list count: %d\n", free_count, doc->slots_per_day, w_for_doc);
}

/* Appointment management sub-menu. */
void appointmentMenu(Appointment arr[], int *acount,
                     Patient patients[], int pcount,
                     Doctor doctors[], int dcount,
                     WaitingEntry queue[], int *wcount,
                     Date today)
{
    (void)arr; (void)acount; (void)queue; (void)wcount;
    int choice;
    while (1) {
        printHeader("APPOINTMENT MANAGEMENT");
        printf("1. Book Routine Appointment\n");
        printf("2. Emergency Booking\n");
        printf("3. View Free Slots\n");
        printf("4. Daily Schedule View\n");
        printf("5. Cancel Appointment\n");
        printf("6. Reschedule Appointment\n");
        printf("7. Mark Completed / No-Show\n");
        printf("0. Back to Main Menu\n");
        printSeparator();
        printf("Select option: ");

        if (!readInt(&choice)) {
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1:
                bookAppointmentInteractive(patients, pcount, doctors, dcount, today);
                break;
            case 2:
                emergencyBookingInteractive(patients, pcount, doctors, dcount, today);
                break;
            case 3:
                viewFreeSlotsInteractive(doctors, dcount);
                break;
            case 4: {
                Date d;
                char dbuf[32];
                printf("Enter Date for Schedule (Enter for Today): ");
                readString(dbuf, sizeof(dbuf));
                if (strlen(dbuf) == 0) {
                    d = today;
                } else if (sscanf(dbuf, "%d-%d-%d", &d.day, &d.month, &d.year) != 3 || !isValidDate(d)) {
                    printf("Invalid date.\n");
                    break;
                }
                showDailySchedule(d);
                break;
            }
            case 5:
                cancelAppointmentInteractive();
                break;
            case 6:
                rescheduleAppointmentInteractive(doctors, dcount, today);
                break;
            case 7:
                markAppointmentStatus(doctors, dcount, today);
                break;
            case 0:
                return;
            default:
                printf("Invalid choice.\n");
                break;
        }
    }
}
