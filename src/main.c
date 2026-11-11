/*
 * main.c - Application entry point, login, dashboard, and main menu.
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

/* Global data stores */
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

        while (fgets(line, sizeof(line), fp) != NULL) {
            int len = (int)strlen(line);
            if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
            if (sscanf(line, "%63[^:]:%63s", file_user, file_pass) == 2) {
                if (strcmp(username, file_user) == 0 &&
                    strcmp(password, file_pass) == 0) {
                    fclose(fp);
                    printColored("Login successful.\n", COLOR_GREEN);
                    return 1;
                }
            }
        }
        fclose(fp);
        printColored("Invalid credentials. Please try again.\n", COLOR_RED);
    }

    printColored("Too many failed attempts. Exiting.\n", COLOR_RED);
    return 0;
}

/* Displays the operational dashboard for the given date. */
static void showDashboard(Date today)
{
    char date_buf[11];
    int  i, booked, free_slots;

    dateToString(today, date_buf);
    printHeader("DASHBOARD");
    printf("Date : %s\n", date_buf);
    printf("Today's appointments : %d\n",
           countTodayAppointments(g_appointments, g_acount, today));
    printf("Emergency waiting    : %d\n",
           countWaiting(g_waiting, g_wcount));

    printSeparator();
    printf("%-4s %-20s %-10s %-10s\n", "ID", "Doctor", "Booked", "Free");
    printSeparator();

    for (i = 0; i < g_dcount; i++) {
        if (!g_doctors[i].is_active) continue;
        booked     = countBookedSlots(g_appointments, g_acount,
                                      g_doctors[i].id, today);
        free_slots = g_doctors[i].slots_per_day - booked;
        if (free_slots < 0) free_slots = 0;
        printf("%-4d %-20s %-10d %-10d\n",
               g_doctors[i].id, g_doctors[i].name, booked, free_slots);
    }
    printSeparator();
}

/* Prints the numbered main menu. */
static void showMainMenu(void)
{
    printf("\n");
    printSeparator();
    printf("  MAIN MENU\n");
    printSeparator();
    printf("  1. Patient Management\n");
    printf("  2. Doctor Management\n");
    printf("  3. Appointments\n");
    printf("  4. Billing\n");
    printf("  5. Reports\n");
    printf("  0. Exit\n");
    printSeparator();
    printf("Select option: ");
}

int main(void)
{
    Date today;
    int  choice;

    getTodayDate(&today);

    if (!login()) {
        return EXIT_FAILURE;
    }

    g_pcount = loadPatients(g_patients, MAX_PATIENTS);
    g_dcount = loadDoctors(g_doctors, MAX_DOCTORS);
    g_acount = loadAppointments(g_appointments, MAX_APPOINTMENTS);
    g_bcount = loadBills(g_bills, MAX_BILLS);
    g_wcount = loadWaiting(g_waiting, MAX_WAITING);

    showDashboard(today);

    while (1) {
        showMainMenu();
        if (!readInt(&choice)) {
            clearInputBuffer();
            continue;
        }

        switch (choice) {
        case 1:
            patientMenu(g_patients, &g_pcount);
            break;
        case 2:
            doctorMenu(g_doctors, &g_dcount);
            break;
        case 3:
            appointmentMenu(g_appointments, &g_acount,
                            g_patients, g_pcount,
                            g_doctors,  g_dcount,
                            g_waiting,  &g_wcount,
                            today);
            break;
        case 4:
            billingMenu(g_bills, &g_bcount,
                        g_appointments, g_acount,
                        g_patients, g_pcount,
                        g_doctors,  g_dcount,
                        today);
            break;
        case 5:
            reportMenu(g_appointments, g_acount,
                       g_bills, g_bcount,
                       g_patients, g_pcount,
                       g_doctors,  g_dcount);
            break;
        case 0:
            printColored("Goodbye.\n", COLOR_GREEN);
            return EXIT_SUCCESS;
        default:
            printColored("Invalid option. Please try again.\n", COLOR_YELLOW);
            break;
        }
    }

    return EXIT_SUCCESS;
}
