/*
 * patient.c - Patient management module.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h"
#include "patient.h"
#include "doctor.h"
#include "appointment.h"
#include "billing.h"
#include "utils.h"

/* Loads patient records from PATIENTS_FILE into arr; returns record count. */
int loadPatients(Patient arr[], int max)
{
    FILE *fp = fopen(PATIENTS_FILE, "rb");
    if (!fp) return 0;
    int count = (int)fread(arr, sizeof(Patient), (size_t)max, fp);
    fclose(fp);
    return count;
}

/* Writes count patient records from arr to PATIENTS_FILE. */
void savePatients(Patient arr[], int count)
{
    FILE *fp = fopen(PATIENTS_FILE, "wb");
    if (!fp) {
        printf("Error: Unable to open patients file for writing.\n");
        return;
    }
    fwrite(arr, sizeof(Patient), (size_t)count, fp);
    fclose(fp);
}

/* Returns the next available patient ID. */
int getNextPatientId(Patient arr[], int count)
{
    if (count == 0) return 1001;
    int max_id = arr[0].id;
    for (int i = 1; i < count; i++) {
        if (arr[i].id > max_id) max_id = arr[i].id;
    }
    return max_id + 1;
}

/* Returns array index of the patient with the given id, or -1 if not found. */
int findPatientById(Patient arr[], int count, int id)
{
    for (int i = 0; i < count; i++) {
        if (arr[i].id == id) return i;
    }
    return -1;
}

/* Prints detailed record of a single patient. */
void printPatientDetails(const Patient *p)
{
    printf("\n--- Patient Record (ID: %d) ---\n", p->id);
    printf("Name        : %s\n", p->name);
    printf("Age         : %d\n", p->age);
    printf("Gender      : %c\n", p->gender);
    printf("Phone       : %s\n", p->phone);
    printf("Blood Group : %s\n", p->blood_group);
    printf("Address     : %s\n", p->address);
    printf("--------------------------------\n");
}

/* Interactively registers a new patient with input validation. */
int registerPatient(Patient arr[], int *count)
{
    if (*count >= MAX_PATIENTS) {
        printf("Error: Maximum patient limit reached.\n");
        return -1;
    }

    Patient p;
    memset(&p, 0, sizeof(Patient));
    p.id = getNextPatientId(arr, *count);

    printHeader("REGISTER NEW PATIENT");

    /* Patient name */
    while (1) {
        printf("Enter Name: ");
        readString(p.name, sizeof(p.name));
        if (strlen(p.name) > 0) break;
        printf("Name cannot be empty. Please retry.\n");
    }

    /* Age */
    while (1) {
        printf("Enter Age (0 - 120): ");
        if (readInt(&p.age) && p.age >= 0 && p.age <= 120) break;
        printf("Invalid age. Enter a number between 0 and 120.\n");
    }

    /* Gender */
    while (1) {
        char gbuf[16];
        printf("Enter Gender (M/F/O): ");
        readString(gbuf, sizeof(gbuf));
        char g = (char)toupper((unsigned char)gbuf[0]);
        if (g == 'M' || g == 'F' || g == 'O') {
            p.gender = g;
            break;
        }
        printf("Invalid gender. Please enter M, F, or O.\n");
    }

    /* Phone number */
    while (1) {
        printf("Enter Phone (10 digits): ");
        readString(p.phone, sizeof(p.phone));
        if (validatePhone(p.phone)) break;
        printf("Invalid phone number. Must be exactly 10 digits.\n");
    }

    /* Blood group */
    while (1) {
        printf("Enter Blood Group (A+, A-, B+, B-, AB+, AB-, O+, O-): ");
        readString(p.blood_group, sizeof(p.blood_group));
        for (size_t i = 0; i < strlen(p.blood_group); i++) {
            p.blood_group[i] = (char)toupper((unsigned char)p.blood_group[i]);
        }
        if (validateBloodGroup(p.blood_group)) break;
        printf("Invalid blood group. Please re-enter.\n");
    }

    /* Address */
    printf("Enter Address: ");
    readString(p.address, sizeof(p.address));

    arr[*count] = p;
    (*count)++;
    savePatients(arr, *count);

    printColored("Patient successfully registered.\n", COLOR_GREEN);
    printf("Assigned Patient ID: %d\n", p.id);
    return p.id;
}

/* Displays a formatted table of all registered patients. */
void listAllPatients(Patient arr[], int count)
{
    printHeader("ALL REGISTERED PATIENTS");
    if (count == 0) {
        printf("No patients found in system.\n");
        return;
    }

    printf("%-6s %-22s %-5s %-7s %-12s %-6s %-20s\n",
           "ID", "Name", "Age", "Gender", "Phone", "Blood", "Address");
    printSeparator();

    for (int i = 0; i < count; i++) {
        printf("%-6d %-22s %-5d %-7c %-12s %-6s %-20s\n",
               arr[i].id,
               arr[i].name,
               arr[i].age,
               arr[i].gender,
               arr[i].phone,
               arr[i].blood_group,
               arr[i].address);
    }
    printSeparator();
    printf("Total Patients: %d\n", count);
}

/* Interactively searches patients by ID, name, or phone. */
void searchPatient(Patient arr[], int count)
{
    if (count == 0) {
        printf("No patients registered.\n");
        return;
    }

    printHeader("SEARCH PATIENT");
    printf("1. Search by ID\n");
    printf("2. Search by Name (partial match)\n");
    printf("3. Search by Phone Number\n");
    printf("0. Cancel\n");
    printf("Select option: ");

    int choice;
    if (!readInt(&choice)) return;

    if (choice == 1) {
        int id;
        printf("Enter Patient ID: ");
        if (!readInt(&id)) return;
        int idx = findPatientById(arr, count, id);
        if (idx >= 0) {
            printPatientDetails(&arr[idx]);
        } else {
            printf("Patient with ID %d not found.\n", id);
        }
    } else if (choice == 2) {
        char query[64];
        printf("Enter Name or partial keyword: ");
        readString(query, sizeof(query));

        char qlower[64];
        for (size_t i = 0; i <= strlen(query); i++) {
            qlower[i] = (char)tolower((unsigned char)query[i]);
        }

        int found = 0;
        for (int i = 0; i < count; i++) {
            char namelower[64];
            for (size_t j = 0; j <= strlen(arr[i].name); j++) {
                namelower[j] = (char)tolower((unsigned char)arr[i].name[j]);
            }
            if (strstr(namelower, qlower) != NULL) {
                printPatientDetails(&arr[i]);
                found++;
            }
        }
        if (!found) printf("No patients matching '%s'.\n", query);
    } else if (choice == 3) {
        char phone[20];
        printf("Enter Phone Number: ");
        readString(phone, sizeof(phone));

        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strstr(arr[i].phone, phone) != NULL) {
                printPatientDetails(&arr[i]);
                found++;
            }
        }
        if (!found) printf("No patient found with phone '%s'.\n", phone);
    }
}

/* Interactively updates an existing patient record. */
void updatePatient(Patient arr[], int count)
{
    int id;
    printf("Enter Patient ID to update: ");
    if (!readInt(&id)) return;

    int idx = findPatientById(arr, count, id);
    if (idx < 0) {
        printf("Patient with ID %d not found.\n", id);
        return;
    }

    Patient *p = &arr[idx];
    printPatientDetails(p);

    char buf[100];

    printf("New Name (press Enter to keep '%s'): ", p->name);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(p->name, buf, sizeof(p->name) - 1);

    printf("New Age (press Enter to keep %d): ", p->age);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) {
        int a;
        if (sscanf(buf, "%d", &a) == 1 && a >= 0 && a <= 120) p->age = a;
    }

    printf("New Gender M/F/O (press Enter to keep '%c'): ", p->gender);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) {
        char g = (char)toupper((unsigned char)buf[0]);
        if (g == 'M' || g == 'F' || g == 'O') p->gender = g;
    }

    printf("New Phone (press Enter to keep '%s'): ", p->phone);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0 && validatePhone(buf)) {
        strncpy(p->phone, buf, sizeof(p->phone) - 1);
    }

    printf("New Address (press Enter to keep '%s'): ", p->address);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(p->address, buf, sizeof(p->address) - 1);

    savePatients(arr, count);
    printColored("Patient record updated successfully.\n", COLOR_GREEN);
}

/* Displays complete visit history for a specific patient. */
void patientVisitHistory(int patient_id)
{
    Patient patients[MAX_PATIENTS];
    int pcount = loadPatients(patients, MAX_PATIENTS);
    int pidx = findPatientById(patients, pcount, patient_id);
    if (pidx < 0) {
        printf("Patient with ID %d not found.\n", patient_id);
        return;
    }

    Doctor doctors[MAX_DOCTORS];
    int dcount = loadDoctors(doctors, MAX_DOCTORS);

    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    Bill bills[MAX_BILLS];
    int bcount = loadBills(bills, MAX_BILLS);

    char title[100];
    snprintf(title, sizeof(title), "VISIT HISTORY: %s (ID %d)", patients[pidx].name, patient_id);
    printHeader(title);

    int count = 0;
    for (int i = 0; i < acount; i++) {
        if (appts[i].patient_id != patient_id) continue;
        count++;

        char date_str[16];
        dateToString(appts[i].date, date_str);

        char time_str[16];
        slotToTime(appts[i].slot, time_str);

        const char *status_str = "Booked";
        if (appts[i].status == STATUS_COMPLETED) status_str = "Completed";
        else if (appts[i].status == STATUS_CANCELLED) status_str = "Cancelled";
        else if (appts[i].status == STATUS_NOSHOW) status_str = "No-Show";

        const char *type_str = (appts[i].type == TYPE_EMERGENCY) ? "Emergency" : "Routine";

        /* Doctor details */
        char doc_name[96] = "Unknown";
        int didx = findDoctorById(doctors, dcount, appts[i].doctor_id);
        if (didx >= 0) {
            snprintf(doc_name, sizeof(doc_name), "Dr. %s (%s)",
                     doctors[didx].name, doctors[didx].specialization);
        }

        /* Check bill */
        float bill_total = 0.0f;
        int billed = 0;
        for (int b = 0; b < bcount; b++) {
            if (bills[b].appointment_id == appts[i].id) {
                bill_total = bills[b].total;
                billed = 1;
                break;
            }
        }

        printf("Appointment #%-4d | Date: %s | Time: %-9s | Type: %-9s\n",
               appts[i].id, date_str, time_str, type_str);
        printf("  Doctor : %s\n", doc_name);
        printf("  Status : %s\n", status_str);
        if (billed) {
            printf("  Bill   : Paid (Total: %.2f)\n", bill_total);
        } else if (appts[i].status == STATUS_COMPLETED) {
            printf("  Bill   : Pending generation\n");
        }
        printf("------------------------------------------------------------\n");
    }

    if (count == 0) {
        printf("No past or scheduled visits found for this patient.\n");
    } else {
        printf("Total records found: %d\n", count);
    }
}

/* Patient management interactive sub-menu. */
void patientMenu(Patient arr[], int *count)
{
    int choice;
    while (1) {
        printHeader("PATIENT MANAGEMENT");
        printf("1. Register New Patient\n");
        printf("2. View All Patients\n");
        printf("3. Search Patient\n");
        printf("4. Update Patient Details\n");
        printf("5. Patient Visit History\n");
        printf("0. Back to Main Menu\n");
        printSeparator();
        printf("Select option: ");

        if (!readInt(&choice)) {
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1:
                registerPatient(arr, count);
                break;
            case 2:
                listAllPatients(arr, *count);
                break;
            case 3:
                searchPatient(arr, *count);
                break;
            case 4:
                updatePatient(arr, *count);
                break;
            case 5: {
                int pid;
                printf("Enter Patient ID: ");
                if (readInt(&pid)) {
                    patientVisitHistory(pid);
                }
                break;
            }
            case 0:
                return;
            default:
                printf("Invalid choice. Please select again.\n");
                break;
        }
    }
}
