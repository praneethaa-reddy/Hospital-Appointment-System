/*
 * main.c - Hospital Patient and Appointment Scheduling System.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "utils.h"
#include "patient.h"
#include "doctor.h"
#include "appointment.h"
#include "billing.h"
#include "report.h"

static Patient      g_patients[MAX_PATIENTS];
static int          g_pcount = 0;

static Doctor       g_doctors[MAX_DOCTORS];
static int          g_dcount = 0;

static Appointment  g_appointments[MAX_APPOINTMENTS];
static int          g_acount = 0;

static Bill         g_bills[MAX_BILLS];
static int          g_bcount = 0;

static WaitingEntry g_waiting[MAX_WAITING];
static int          g_wcount = 0;

/* Refreshes in-memory datasets from storage files. */
static void reloadData(void)
{
    g_pcount = loadPatients(g_patients, MAX_PATIENTS);
    g_dcount = loadDoctors(g_doctors, MAX_DOCTORS);
    g_acount = loadAppointments(g_appointments, MAX_APPOINTMENTS);
    g_bcount = loadBills(g_bills, MAX_BILLS);
    g_wcount = loadWaiting(g_waiting, MAX_WAITING);
}

/* Authenticates the user against USERS_FILE. Returns 1 on success, 0 otherwise. */
static int login(void)
{
    char username[64];
    char password[64];
    char line[256];
    char file_user[64];
    char file_pass[64];
    int  attempts;
    FILE *fp;

    for (attempts = 0; attempts < 3; attempts++) {
        printHeader("HOSPITAL APPOINTMENT SYSTEM - LOGIN");
        printf("Username: ");
        readString(username, sizeof(username));
        printf("Password: ");
        readString(password, sizeof(password));

        fp = fopen(USERS_FILE, "r");
        if (fp == NULL) {
            printColored("Error: Cannot open users file.\n", COLOR_RED);
            return 0;
        }

        int success = 0;
        while (fgets(line, sizeof(line), fp) != NULL) {
            int len = (int)strlen(line);
            if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
                line[len - 1] = '\0';
            }
            if (sscanf(line, "%63[^:]:%63s", file_user, file_pass) == 2) {
                if (strcmp(username, file_user) == 0 &&
                    strcmp(password, file_pass) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        fclose(fp);

        if (success) {
            printColored("\nLogin successful. Welcome!\n", COLOR_GREEN);
            return 1;
        }

        printColored("Invalid credentials. Please try again.\n", COLOR_RED);
        printf("(Attempt %d of 3)\n\n", attempts + 1);
    }

    printColored("Too many failed attempts. Access denied.\n", COLOR_RED);
    return 0;
}

/* Displays the operational dashboard matching expected console format. */
static void showDashboard(Date today)
{
    reloadData();

    const char *weekdays[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };
    int wd = getWeekday(today);
    char date_buf[16];
    dateToString(today, date_buf);

    int total_today = 0;
    int completed_today = 0;
    int cancelled_today = 0;

    for (int i = 0; i < g_acount; i++) {
        if (compareDates(g_appointments[i].date, today) == 0) {
            total_today++;
            if (g_appointments[i].status == STATUS_COMPLETED) completed_today++;
            if (g_appointments[i].status == STATUS_CANCELLED) cancelled_today++;
        }
    }

    int emergency_waiting = 0;
    for (int i = 0; i < g_wcount; i++) {
        if (compareDates(g_waiting[i].date, today) == 0 &&
            g_waiting[i].type == TYPE_EMERGENCY) {
            emergency_waiting++;
        }
    }

    printf("\n============== HOSPITAL SCHEDULE DASHBOARD ==============\n");
    printf("Today: %s, %s\n\n", weekdays[wd], date_buf);
    printf("Appointments Today : %-7d Completed : %-7d\n", total_today, completed_today);
    printf("Emergency Waiting  : %-7d Cancelled : %-7d\n\n", emergency_waiting, cancelled_today);

    printf("%-18s %-16s %-8s %-12s\n", "Doctor", "Specialization", "Booked", "Free Slots");
    printSeparator();

    for (int i = 0; i < g_dcount; i++) {
        if (!g_doctors[i].is_active) continue;

        int booked = countBookedSlots(g_appointments, g_acount, g_doctors[i].id, today);
        int free_slots = g_doctors[i].slots_per_day - booked;
        if (free_slots < 0) free_slots = 0;

        char booked_str[16];
        snprintf(booked_str, sizeof(booked_str), "%d/%d", booked, g_doctors[i].slots_per_day);

        int doc_waiting = 0;
        for (int w = 0; w < g_wcount; w++) {
            if (g_waiting[w].doctor_id == g_doctors[i].id &&
                compareDates(g_waiting[w].date, today) == 0) {
                doc_waiting++;
            }
        }

        char doc_name[64];
        snprintf(doc_name, sizeof(doc_name), "Dr. %s", g_doctors[i].name);

        if (doc_waiting > 0) {
            printf("%-18s %-16s %-8s %d (Waiting: %d)\n",
                   doc_name, g_doctors[i].specialization, booked_str, free_slots, doc_waiting);
        } else {
            printf("%-18s %-16s %-8s %d\n",
                   doc_name, g_doctors[i].specialization, booked_str, free_slots);
        }
    }
    printf("=========================================================\n");
    printf("Press Enter to continue...");
    char pause_buf[8];
    readString(pause_buf, sizeof(pause_buf));
}

/* Prints the numbered main menu. */
static void showMainMenu(void)
{
    printf("\n");
    printSeparator();
    printf("  HOSPITAL PATIENT AND APPOINTMENT SCHEDULING SYSTEM\n");
    printSeparator();
    printf("  1. Register Patient        6. Cancel / Reschedule\n");
    printf("  2. Search Patient          7. Daily Schedule\n");
    printf("  3. Add / View Doctors      8. Emergency Booking\n");
    printf("  4. Book Appointment        9. Billing\n");
    printf("  5. View Free Slots         10. Reports\n");
    printf("  0. Exit\n");
    printSeparator();
    printf("Enter your choice: ");
}

int main(void)
{
    Date today;
    int  choice;

    getTodayDate(&today);

    if (!login()) {
        return EXIT_FAILURE;
    }

    showDashboard(today);

    while (1) {
        reloadData();
        showMainMenu();
        if (!readInt(&choice)) {
            clearInputBuffer();
            continue;
        }

        switch (choice) {
        case 1:
            registerPatient(g_patients, &g_pcount);
            break;
        case 2:
            searchPatient(g_patients, g_pcount);
            break;
        case 3:
            doctorMenu(g_doctors, &g_dcount);
            break;
        case 4:
            bookAppointmentInteractive(g_patients, g_pcount, g_doctors, g_dcount, today);
            break;
        case 5:
            viewFreeSlotsInteractive(g_doctors, g_dcount);
            break;
        case 6: {
            int subchoice;
            printf("\n1. Cancel Appointment\n");
            printf("2. Reschedule Appointment\n");
            printf("0. Back\n");
            printf("Select option: ");
            if (readInt(&subchoice)) {
                if (subchoice == 1) cancelAppointmentInteractive();
                else if (subchoice == 2) rescheduleAppointmentInteractive(g_doctors, g_dcount, today);
            }
            break;
        }
        case 7: {
            Date d;
            char dbuf[32];
            printf("Enter Date for Daily Schedule (DD-MM-YYYY, or press Enter for Today): ");
            readString(dbuf, sizeof(dbuf));
            if (strlen(dbuf) == 0) {
                d = today;
            } else if (sscanf(dbuf, "%d-%d-%d", &d.day, &d.month, &d.year) != 3 || !isValidDate(d)) {
                printf("Invalid date. Displaying today's schedule.\n");
                d = today;
            }
            showDailySchedule(d);
            break;
        }
        case 8:
            emergencyBookingInteractive(g_patients, g_pcount, g_doctors, g_dcount, today);
            break;
        case 9:
            billingMenu(g_bills, &g_bcount,
                        g_appointments, g_acount,
                        g_patients, g_pcount,
                        g_doctors,  g_dcount,
                        today);
            break;
        case 10:
            showReportsMenu();
            break;
        case 0:
            printColored("\nExiting system. Have a great day!\n", COLOR_GREEN);
            return EXIT_SUCCESS;
        default:
            printColored("Invalid option. Please enter a number between 0 and 10.\n", COLOR_YELLOW);
            break;
        }
    }

    return EXIT_SUCCESS;
}
