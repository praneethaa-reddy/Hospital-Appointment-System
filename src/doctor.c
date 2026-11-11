/*
 * doctor.c - Doctor management module.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h"
#include "doctor.h"
#include "appointment.h"
#include "utils.h"

/* Loads doctor records from DOCTORS_FILE into arr; returns record count. */
int loadDoctors(Doctor arr[], int max)
{
    FILE *fp = fopen(DOCTORS_FILE, "rb");
    if (!fp) return 0;
    int count = (int)fread(arr, sizeof(Doctor), (size_t)max, fp);
    fclose(fp);
    return count;
}

/* Writes count doctor records from arr to DOCTORS_FILE. */
void saveDoctors(Doctor arr[], int count)
{
    FILE *fp = fopen(DOCTORS_FILE, "wb");
    if (!fp) {
        printf("Error: Unable to open doctors file for writing.\n");
        return;
    }
    fwrite(arr, sizeof(Doctor), (size_t)count, fp);
    fclose(fp);
}

/* Returns the next available doctor ID. */
int getNextDoctorId(Doctor arr[], int count)
{
    if (count == 0) return 101;
    int max_id = arr[0].id;
    for (int i = 1; i < count; i++) {
        if (arr[i].id > max_id) max_id = arr[i].id;
    }
    return max_id + 1;
}

/* Returns array index of the doctor with the given id, or -1 if not found. */
int findDoctorById(Doctor arr[], int count, int id)
{
    for (int i = 0; i < count; i++) {
        if (arr[i].id == id) return i;
    }
    return -1;
}

/* Checks if a doctor is available on a specific weekday (0=Sun..6=Sat). */
int isDoctorAvailable(const Doctor *doc, int weekday)
{
    if (weekday < 0 || weekday > 6) return 0;
    return doc->available_days[weekday] == '1';
}

/* Prints the working days of a doctor. */
void printDoctorDays(const Doctor *d)
{
    const char *names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    int printed = 0;
    for (int i = 0; i < 7; i++) {
        if (d->available_days[i] == '1') {
            if (printed) printf(" ");
            printf("%s", names[i]);
            printed = 1;
        }
    }
    if (!printed) printf("None");
}

/* Interactively adds a new doctor. */
int addDoctor(Doctor arr[], int *count)
{
    if (*count >= MAX_DOCTORS) {
        printf("Error: Doctor capacity reached.\n");
        return -1;
    }

    Doctor d;
    memset(&d, 0, sizeof(Doctor));
    d.id = getNextDoctorId(arr, *count);
    d.is_active = 1;

    printHeader("ADD NEW DOCTOR");

    while (1) {
        printf("Doctor Name: ");
        readString(d.name, sizeof(d.name));
        if (strlen(d.name) > 0) break;
        printf("Name cannot be empty.\n");
    }

    while (1) {
        printf("Specialization (e.g., Cardiology): ");
        readString(d.specialization, sizeof(d.specialization));
        if (strlen(d.specialization) > 0) break;
        printf("Specialization cannot be empty.\n");
    }

    while (1) {
        char buf[32];
        printf("Consultation Fee: ");
        readString(buf, sizeof(buf));
        if (sscanf(buf, "%f", &d.fee) == 1 && d.fee > 0.0f) break;
        printf("Invalid fee. Enter a positive number.\n");
    }

    while (1) {
        char buf[32];
        printf("Slots Per Day (1-16, press Enter for default 16): ");
        readString(buf, sizeof(buf));
        if (strlen(buf) == 0) {
            d.slots_per_day = 16;
            break;
        }
        if (sscanf(buf, "%d", &d.slots_per_day) == 1 &&
            d.slots_per_day >= 1 && d.slots_per_day <= 16) {
            break;
        }
        printf("Invalid slot count. Enter a number between 1 and 16.\n");
    }

    /* Available days: Sunday is always off, ask for Mon to Sat */
    d.available_days[0] = '0';
    d.available_days[7] = '\0';
    const char *days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    printf("\nSet Working Days (Y/N):\n");
    for (int i = 1; i <= 6; i++) {
        while (1) {
            char resp[16];
            printf("  %s (Y/N): ", days[i - 1]);
            readString(resp, sizeof(resp));
            char c = (char)toupper((unsigned char)resp[0]);
            if (c == 'Y') {
                d.available_days[i] = '1';
                break;
            }
            if (c == 'N') {
                d.available_days[i] = '0';
                break;
            }
            printf("Please enter Y or N.\n");
        }
    }

    arr[*count] = d;
    (*count)++;
    saveDoctors(arr, *count);

    printColored("Doctor registered successfully.\n", COLOR_GREEN);
    printf("Assigned Doctor ID: %d\n", d.id);
    return d.id;
}

/* Displays all active doctors in a formatted table. */
void listAllDoctors(Doctor arr[], int count)
{
    printHeader("ALL ACTIVE DOCTORS");
    int active_count = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i].is_active) active_count++;
    }

    if (active_count == 0) {
        printf("No active doctors found.\n");
        return;
    }

    printf("%-5s %-20s %-18s %-10s %-20s %-6s\n",
           "ID", "Name", "Specialization", "Fee", "Available Days", "Slots");
    printSeparator();

    for (int i = 0; i < count; i++) {
        if (!arr[i].is_active) continue;
        printf("%-5d Dr. %-16s %-18s %-10.2f ",
               arr[i].id, arr[i].name, arr[i].specialization, arr[i].fee);
        printDoctorDays(&arr[i]);
        printf("  %-6d\n", arr[i].slots_per_day);
    }
    printSeparator();
    printf("Total Active Doctors: %d\n", active_count);
}

/* Displays active doctors filtered by specialization keyword. */
void viewDoctorsBySpecialization(Doctor arr[], int count)
{
    char keyword[32];
    printf("Enter Specialization: ");
    readString(keyword, sizeof(keyword));

    char klower[32];
    for (size_t i = 0; i <= strlen(keyword); i++) {
        klower[i] = (char)tolower((unsigned char)keyword[i]);
    }

    printHeader("FILTERED DOCTORS");
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (!arr[i].is_active) continue;

        char slower[32];
        for (size_t j = 0; j <= strlen(arr[i].specialization); j++) {
            slower[j] = (char)tolower((unsigned char)arr[i].specialization[j]);
        }

        if (strstr(slower, klower) != NULL) {
            if (!found) {
                printf("%-5s %-20s %-18s %-10s %-20s\n",
                       "ID", "Name", "Specialization", "Fee", "Working Days");
                printSeparator();
            }
            printf("%-5d Dr. %-16s %-18s %-10.2f ",
                   arr[i].id, arr[i].name, arr[i].specialization, arr[i].fee);
            printDoctorDays(&arr[i]);
            printf("\n");
            found++;
        }
    }

    if (!found) {
        printf("No active doctors found with specialization '%s'.\n", keyword);
    } else {
        printSeparator();
        printf("Matching Doctors: %d\n", found);
    }
}

/* Interactively updates doctor information. */
int updateDoctor(Doctor arr[], int count)
{
    int id;
    printf("Enter Doctor ID to update: ");
    if (!readInt(&id)) return 0;

    int idx = findDoctorById(arr, count, id);
    if (idx < 0 || !arr[idx].is_active) {
        printf("Active doctor with ID %d not found.\n", id);
        return 0;
    }

    Doctor *d = &arr[idx];
    printHeader("UPDATE DOCTOR DETAILS");
    printf("Current Details: Dr. %s | %s | Fee: %.2f | Slots: %d\n",
           d->name, d->specialization, d->fee, d->slots_per_day);
    printf("Working Days: ");
    printDoctorDays(d);
    printf("\n\n");

    char buf[64];
    printf("New Name (Enter to keep '%s'): ", d->name);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(d->name, buf, sizeof(d->name) - 1);

    printf("New Specialization (Enter to keep '%s'): ", d->specialization);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) strncpy(d->specialization, buf, sizeof(d->specialization) - 1);

    printf("New Fee (Enter to keep %.2f): ", d->fee);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) {
        float f;
        if (sscanf(buf, "%f", &f) == 1 && f > 0.0f) d->fee = f;
    }

    printf("New Slots Per Day 1-16 (Enter to keep %d): ", d->slots_per_day);
    readString(buf, sizeof(buf));
    if (strlen(buf) > 0) {
        int s;
        if (sscanf(buf, "%d", &s) == 1 && s >= 1 && s <= 16) d->slots_per_day = s;
    }

    printf("Update working days? (Y/N): ");
    readString(buf, sizeof(buf));
    if (toupper((unsigned char)buf[0]) == 'Y') {
        const char *days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        d->available_days[0] = '0';
        for (int i = 1; i <= 6; i++) {
            while (1) {
                char resp[16];
                printf("  %s (Y/N): ", days[i - 1]);
                readString(resp, sizeof(resp));
                char c = (char)toupper((unsigned char)resp[0]);
                if (c == 'Y') { d->available_days[i] = '1'; break; }
                if (c == 'N') { d->available_days[i] = '0'; break; }
            }
        }
    }

    saveDoctors(arr, count);
    printColored("Doctor details updated successfully.\n", COLOR_GREEN);
    return 1;
}

/* Deactivates a doctor if no future booked appointments exist. */
int removeDoctor(Doctor arr[], int count)
{
    int id;
    printf("Enter Doctor ID to remove: ");
    if (!readInt(&id)) return 0;

    int idx = findDoctorById(arr, count, id);
    if (idx < 0 || !arr[idx].is_active) {
        printf("Active doctor with ID %d not found.\n", id);
        return 0;
    }

    /* Check for future booked appointments */
    Appointment appts[MAX_APPOINTMENTS];
    int acount = loadAppointments(appts, MAX_APPOINTMENTS);

    Date today;
    getTodayDate(&today);

    for (int i = 0; i < acount; i++) {
        if (appts[i].doctor_id == id &&
            appts[i].status == STATUS_BOOKED &&
            compareDates(appts[i].date, today) >= 0) {
            printColored("Cannot remove doctor: Doctor has future booked appointments.\n", COLOR_RED);
            return 0;
        }
    }

    arr[idx].is_active = 0;
    saveDoctors(arr, count);
    printColored("Doctor successfully deactivated.\n", COLOR_GREEN);
    return 1;
}

/* Displays free slots for a doctor on a specific date. */
void viewDoctorFreeSlots(Doctor arr[], int count, Date today)
{
    (void)today;
    viewFreeSlotsInteractive(arr, count);
}

/* Doctor management sub-menu. */
void doctorMenu(Doctor arr[], int *count)
{
    int choice;
    while (1) {
        printHeader("DOCTOR MANAGEMENT");
        printf("1. Add New Doctor\n");
        printf("2. View All Doctors\n");
        printf("3. View Doctors by Specialization\n");
        printf("4. Update Doctor Details\n");
        printf("5. Deactivate Doctor\n");
        printf("6. View Doctor Free Slots\n");
        printf("0. Back to Main Menu\n");
        printSeparator();
        printf("Select option: ");

        if (!readInt(&choice)) {
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1:
                addDoctor(arr, count);
                break;
            case 2:
                listAllDoctors(arr, *count);
                break;
            case 3:
                viewDoctorsBySpecialization(arr, *count);
                break;
            case 4:
                updateDoctor(arr, *count);
                break;
            case 5:
                removeDoctor(arr, *count);
                break;
            case 6: {
                Date today;
                getTodayDate(&today);
                viewDoctorFreeSlots(arr, *count, today);
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
