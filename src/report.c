/*
 * report.c - Reports module for the hospital appointment system.
 * Provides statistical and operational reports from the data files.
 *
 * Compile: gcc -Wall -Wextra -g -Iinclude -c src/report.c -o obj/report.o
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "types.h"
#include "report.h"
#include "utils.h"

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */

/* Load all patients from PATIENTS_FILE into buf; returns count. */
static int loadPatients(Patient *buf, int max)
{
    FILE *f = fopen(PATIENTS_FILE, "rb");
    if (!f) return 0;
    int n = (int)fread(buf, sizeof(Patient), max, f);
    fclose(f);
    return n;
}

/* Load all doctors from DOCTORS_FILE into buf; returns count. */
static int loadDoctors(Doctor *buf, int max)
{
    FILE *f = fopen(DOCTORS_FILE, "rb");
    if (!f) return 0;
    int n = (int)fread(buf, sizeof(Doctor), max, f);
    fclose(f);
    return n;
}

/* Load all appointments from APPOINTMENTS_FILE into buf; returns count. */
static int loadAppointments(Appointment *buf, int max)
{
    FILE *f = fopen(APPOINTMENTS_FILE, "rb");
    if (!f) return 0;
    int n = (int)fread(buf, sizeof(Appointment), max, f);
    fclose(f);
    return n;
}

/* Load all bills from BILLS_FILE into buf; returns count. */
static int loadBills(Bill *buf, int max)
{
    FILE *f = fopen(BILLS_FILE, "rb");
    if (!f) return 0;
    int n = (int)fread(buf, sizeof(Bill), max, f);
    fclose(f);
    return n;
}

/* Find doctor by ID; returns pointer into buf or NULL. */
static Doctor *findDoctor(Doctor *buf, int n, int id)
{
    for (int i = 0; i < n; i++)
        if (buf[i].id == id) return &buf[i];
    return NULL;
}

/* Convert Date to days since epoch (simple formula). */
static long dateToDaysLocal(Date d)
{
    /* Treat each year as 365 days + leap year correction via mktime. */
    struct tm t = {0};
    t.tm_year = d.year - 1900;
    t.tm_mon  = d.month - 1;
    t.tm_mday = d.day;
    time_t tt = mktime(&t);
    if (tt == (time_t)-1) return 0;
    return (long)(tt / 86400);
}

/* Return 0=Sun..6=Sat for a given Date. */
static int getWeekdayLocal(Date d)
{
    struct tm t = {0};
    t.tm_year = d.year - 1900;
    t.tm_mon  = d.month - 1;
    t.tm_mday = d.day;
    mktime(&t);
    return t.tm_wday;
}

/* Return 1 if two dates are equal. */
static int dateEq(Date a, Date b)
{
    return a.day == b.day && a.month == b.month && a.year == b.year;
}

/* -------------------------------------------------------------------------
 * 1. reportToday
 * Summary of today's appointments.
 * ------------------------------------------------------------------------- */
void reportToday(void)
{
    Date today;
    getTodayDate(&today);

    Appointment appts[MAX_APPOINTMENTS];
    int n = loadAppointments(appts, MAX_APPOINTMENTS);

    int total = 0, completed = 0, cancelled = 0, booked = 0, emergency = 0;
    for (int i = 0; i < n; i++) {
        if (!dateEq(appts[i].date, today)) continue;
        total++;
        if (appts[i].status == STATUS_COMPLETED) completed++;
        if (appts[i].status == STATUS_CANCELLED)  cancelled++;
        if (appts[i].status == STATUS_BOOKED)     booked++;
        if (appts[i].type   == TYPE_EMERGENCY)    emergency++;
    }

    printHeader("TODAY'S APPOINTMENTS REPORT");
    char buf[20];
    dateToString(today, buf);
    printf("  Date           : %s\n", buf);
    printf("  Total          : %d\n", total);
    printf("  Booked         : %d\n", booked);
    printf("  Completed      : %d\n", completed);
    printf("  Cancelled      : %d\n", cancelled);
    printf("  Emergency      : %d\n", emergency);
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 2. reportThisWeek
 * Summary of appointments for the current Mon-Sun week.
 * ------------------------------------------------------------------------- */
void reportThisWeek(void)
{
    Date today;
    getTodayDate(&today);

    /* Find Monday of this week. */
    int wday = getWeekdayLocal(today); /* 0=Sun..6=Sat */
    int daysFromMon = (wday == 0) ? 6 : wday - 1;

    struct tm t = {0};
    t.tm_year = today.year - 1900;
    t.tm_mon  = today.month - 1;
    t.tm_mday = today.day - daysFromMon;
    mktime(&t);

    Date weekStart;
    weekStart.day   = t.tm_mday;
    weekStart.month = t.tm_mon + 1;
    weekStart.year  = t.tm_year + 1900;

    /* Sunday = weekStart + 6 days. */
    t.tm_mday += 6;
    mktime(&t);
    Date weekEnd;
    weekEnd.day   = t.tm_mday;
    weekEnd.month = t.tm_mon + 1;
    weekEnd.year  = t.tm_year + 1900;

    long startDays = dateToDaysLocal(weekStart);
    long endDays   = dateToDaysLocal(weekEnd);

    Appointment appts[MAX_APPOINTMENTS];
    int n = loadAppointments(appts, MAX_APPOINTMENTS);

    int total = 0, completed = 0, cancelled = 0, booked = 0, noshow = 0, emergency = 0;
    for (int i = 0; i < n; i++) {
        long d = dateToDaysLocal(appts[i].date);
        if (d < startDays || d > endDays) continue;
        total++;
        if (appts[i].status == STATUS_COMPLETED) completed++;
        if (appts[i].status == STATUS_CANCELLED)  cancelled++;
        if (appts[i].status == STATUS_BOOKED)     booked++;
        if (appts[i].status == STATUS_NOSHOW)     noshow++;
        if (appts[i].type   == TYPE_EMERGENCY)    emergency++;
    }

    char s1[20], s2[20];
    dateToString(weekStart, s1);
    dateToString(weekEnd,   s2);
    printHeader("THIS WEEK'S APPOINTMENTS REPORT");
    printf("  Week           : %s to %s\n", s1, s2);
    printf("  Total          : %d\n", total);
    printf("  Booked         : %d\n", booked);
    printf("  Completed      : %d\n", completed);
    printf("  Cancelled      : %d\n", cancelled);
    printf("  No-Show        : %d\n", noshow);
    printf("  Emergency      : %d\n", emergency);
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 3. reportThisMonth
 * Summary of appointments for the current calendar month.
 * ------------------------------------------------------------------------- */
void reportThisMonth(void)
{
    Date today;
    getTodayDate(&today);

    Appointment appts[MAX_APPOINTMENTS];
    int n = loadAppointments(appts, MAX_APPOINTMENTS);

    int total = 0, completed = 0, cancelled = 0, booked = 0, noshow = 0, emergency = 0;
    for (int i = 0; i < n; i++) {
        if (appts[i].date.month != today.month ||
            appts[i].date.year  != today.year) continue;
        total++;
        if (appts[i].status == STATUS_COMPLETED) completed++;
        if (appts[i].status == STATUS_CANCELLED)  cancelled++;
        if (appts[i].status == STATUS_BOOKED)     booked++;
        if (appts[i].status == STATUS_NOSHOW)     noshow++;
        if (appts[i].type   == TYPE_EMERGENCY)    emergency++;
    }

    printHeader("THIS MONTH'S APPOINTMENTS REPORT");
    printf("  Month          : %02d/%04d\n", today.month, today.year);
    printf("  Total          : %d\n", total);
    printf("  Booked         : %d\n", booked);
    printf("  Completed      : %d\n", completed);
    printf("  Cancelled      : %d\n", cancelled);
    printf("  No-Show        : %d\n", noshow);
    printf("  Emergency      : %d\n", emergency);
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 4. reportDoctorStats
 * Per active doctor: unique patients, appointment count, revenue.
 * Sorted by revenue descending.
 * ------------------------------------------------------------------------- */
void reportDoctorStats(void)
{
    Doctor  docs[MAX_DOCTORS];
    int     nd = loadDoctors(docs, MAX_DOCTORS);

    Appointment appts[MAX_APPOINTMENTS];
    int         na = loadAppointments(appts, MAX_APPOINTMENTS);

    Bill bills[MAX_BILLS];
    int  nb = loadBills(bills, MAX_BILLS);

    /* Build stats per doctor index. */
    typedef struct {
        int   doc_idx;
        int   appt_count;
        float revenue;
        int   patient_ids[MAX_APPOINTMENTS];
        int   patient_count;
    } DoctorStat;

    DoctorStat stats[MAX_DOCTORS];
    int        nstats = 0;

    for (int i = 0; i < nd; i++) {
        if (!docs[i].is_active) continue;
        stats[nstats].doc_idx       = i;
        stats[nstats].appt_count    = 0;
        stats[nstats].revenue       = 0.0f;
        stats[nstats].patient_count = 0;
        nstats++;
    }

    /* Count appointments and revenue per active doctor. */
    for (int i = 0; i < na; i++) {
        for (int j = 0; j < nstats; j++) {
            if (docs[stats[j].doc_idx].id != appts[i].doctor_id) continue;
            stats[j].appt_count++;
            /* Track unique patient IDs. */
            int found = 0;
            for (int k = 0; k < stats[j].patient_count; k++) {
                if (stats[j].patient_ids[k] == appts[i].patient_id) {
                    found = 1; break;
                }
            }
            if (!found && stats[j].patient_count < MAX_APPOINTMENTS)
                stats[j].patient_ids[stats[j].patient_count++] = appts[i].patient_id;
            break;
        }
    }

    /* Sum bills per active doctor via appointment_id lookup. */
    for (int b = 0; b < nb; b++) {
        for (int i = 0; i < na; i++) {
            if (appts[i].id != bills[b].appointment_id) continue;
            for (int j = 0; j < nstats; j++) {
                if (docs[stats[j].doc_idx].id == appts[i].doctor_id) {
                    stats[j].revenue += bills[b].total;
                    break;
                }
            }
            break;
        }
    }

    /* Sort by revenue descending (simple bubble sort). */
    for (int i = 0; i < nstats - 1; i++) {
        for (int j = i + 1; j < nstats; j++) {
            if (stats[j].revenue > stats[i].revenue) {
                DoctorStat tmp = stats[i];
                stats[i] = stats[j];
                stats[j] = tmp;
            }
        }
    }

    printHeader("DOCTOR STATISTICS REPORT");
    printf("  %-25s %-18s %8s %10s %12s\n",
           "Doctor", "Specialization", "Patients", "Appts", "Revenue");
    printSeparator();
    for (int j = 0; j < nstats; j++) {
        Doctor *d = &docs[stats[j].doc_idx];
        printf("  %-25s %-18s %8d %10d %12.2f\n",
               d->name, d->specialization,
               stats[j].patient_count,
               stats[j].appt_count,
               stats[j].revenue);
    }
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 5. reportCancellations
 * Cancellation rate and no-show rate as percentages.
 * ------------------------------------------------------------------------- */
void reportCancellations(void)
{
    Appointment appts[MAX_APPOINTMENTS];
    int n = loadAppointments(appts, MAX_APPOINTMENTS);

    int total = n, cancelled = 0, noshow = 0;
    for (int i = 0; i < n; i++) {
        if (appts[i].status == STATUS_CANCELLED) cancelled++;
        if (appts[i].status == STATUS_NOSHOW)    noshow++;
    }

    float cancel_rate = (total > 0) ? (100.0f * cancelled / total) : 0.0f;
    float noshow_rate = (total > 0) ? (100.0f * noshow    / total) : 0.0f;

    printHeader("CANCELLATIONS & NO-SHOWS REPORT");
    printf("  Total Appointments   : %d\n", total);
    printf("  Cancelled            : %d (%.2f%%)\n", cancelled, cancel_rate);
    printf("  No-Show              : %d (%.2f%%)\n", noshow,    noshow_rate);
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 6. reportBusiestDoctorAndDay
 * Doctor with most completed appointments; weekday with most appointments.
 * ------------------------------------------------------------------------- */
void reportBusiestDoctorAndDay(void)
{
    Doctor docs[MAX_DOCTORS];
    int    nd = loadDoctors(docs, MAX_DOCTORS);

    Appointment appts[MAX_APPOINTMENTS];
    int         na = loadAppointments(appts, MAX_APPOINTMENTS);

    /* Count completed appointments per doctor. */
    int doc_count[MAX_DOCTORS] = {0};
    for (int i = 0; i < na; i++) {
        if (appts[i].status != STATUS_COMPLETED) continue;
        for (int j = 0; j < nd; j++) {
            if (docs[j].id == appts[i].doctor_id) {
                doc_count[j]++;
                break;
            }
        }
    }

    /* Find doctor with max completed. */
    int best_doc = -1;
    for (int j = 0; j < nd; j++) {
        if (best_doc < 0 || doc_count[j] > doc_count[best_doc])
            best_doc = j;
    }

    /* Count appointments per weekday (0=Sun..6=Sat). */
    int day_count[7] = {0};
    const char *day_names[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    for (int i = 0; i < na; i++) {
        int wd = getWeekdayLocal(appts[i].date);
        day_count[wd]++;
    }

    int best_day = 0;
    for (int d = 1; d < 7; d++) {
        if (day_count[d] > day_count[best_day])
            best_day = d;
    }

    printHeader("BUSIEST DOCTOR AND DAY REPORT");
    if (best_doc >= 0 && nd > 0)
        printf("  Busiest Doctor : %s (%d completed appointments)\n",
               docs[best_doc].name, doc_count[best_doc]);
    else
        printf("  Busiest Doctor : No data\n");

    printf("  Busiest Day    : %s (%d total appointments)\n",
           day_names[best_day], day_count[best_day]);
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 7. reportRevenue
 * Prompt for date range; sum all bill totals within that range.
 * ------------------------------------------------------------------------- */
void reportRevenue(void)
{
    printHeader("REVENUE REPORT");
    printf("  Enter start date (DD MM YYYY): ");
    Date start;
    if (scanf("%d %d %d", &start.day, &start.month, &start.year) != 3) {
        clearInputBuffer();
        printf("  Invalid date input.\n");
        return;
    }
    clearInputBuffer();

    printf("  Enter end date   (DD MM YYYY): ");
    Date end;
    if (scanf("%d %d %d", &end.day, &end.month, &end.year) != 3) {
        clearInputBuffer();
        printf("  Invalid date input.\n");
        return;
    }
    clearInputBuffer();

    if (!isValidDate(start) || !isValidDate(end)) {
        printf("  Invalid date(s) entered.\n");
        return;
    }

    long startDays = dateToDaysLocal(start);
    long endDays   = dateToDaysLocal(end);
    if (startDays > endDays) {
        printf("  Start date must be before or equal to end date.\n");
        return;
    }

    Bill bills[MAX_BILLS];
    int  nb = loadBills(bills, MAX_BILLS);

    float total_revenue = 0.0f;
    int   count = 0;
    for (int i = 0; i < nb; i++) {
        long d = dateToDaysLocal(bills[i].date);
        if (d < startDays || d > endDays) continue;
        total_revenue += bills[i].total;
        count++;
    }

    char s1[20], s2[20];
    dateToString(start, s1);
    dateToString(end,   s2);
    printf("  Period         : %s to %s\n", s1, s2);
    printf("  Bills counted  : %d\n", count);
    printf("  Total Revenue  : %.2f\n", total_revenue);
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 8. reportNextAvailableSlot
 * Prompt for specialization; find earliest free slot within next 7 days.
 * ------------------------------------------------------------------------- */
void reportNextAvailableSlot(void)
{
    printHeader("NEXT AVAILABLE SLOT REPORT");
    char spec[30];
    printf("  Enter specialization: ");
    readString(spec, sizeof(spec));

    Doctor docs[MAX_DOCTORS];
    int    nd = loadDoctors(docs, MAX_DOCTORS);

    Appointment appts[MAX_APPOINTMENTS];
    int         na = loadAppointments(appts, MAX_APPOINTMENTS);

    Date today;
    getTodayDate(&today);

    /* Weekday char maps: 0=Sun,1=Mon,2=Tue,3=Wed,4=Thu,5=Fri,6=Sat */
    const char wd_chars[] = "UMTWHFS"; /* Sun=U, Mon=M, Tue=T, Wed=W, Thu=H, Fri=F, Sat=S */

    int found = 0;

    /* Iterate over next 7 days. */
    for (int day_offset = 0; day_offset < 7 && !found; day_offset++) {
        struct tm tm_check = {0};
        tm_check.tm_year = today.year - 1900;
        tm_check.tm_mon  = today.month - 1;
        tm_check.tm_mday = today.day + day_offset;
        mktime(&tm_check);

        Date check;
        check.day   = tm_check.tm_mday;
        check.month = tm_check.tm_mon + 1;
        check.year  = tm_check.tm_year + 1900;

        char wd_char = wd_chars[tm_check.tm_wday];

        for (int di = 0; di < nd && !found; di++) {
            /* Check active, matching specialization, works this weekday. */
            if (!docs[di].is_active) continue;
            if (strstr(docs[di].specialization, spec) == NULL &&
                strstr(spec, docs[di].specialization) == NULL) {
                /* Case-insensitive partial match attempt. */
                char spLow[30], docLow[30];
                strncpy(spLow,  spec,                   sizeof(spLow) - 1);
                strncpy(docLow, docs[di].specialization, sizeof(docLow) - 1);
                spLow[sizeof(spLow)-1]   = '\0';
                docLow[sizeof(docLow)-1] = '\0';
                for (int x = 0; spLow[x];  x++) spLow[x]  = (char)tolower((unsigned char)spLow[x]);
                for (int x = 0; docLow[x]; x++) docLow[x] = (char)tolower((unsigned char)docLow[x]);
                if (!strstr(docLow, spLow) && !strstr(spLow, docLow)) continue;
            }
            if (!strchr(docs[di].available_days, wd_char)) continue;

            /* Check each slot. */
            for (int slot = 1; slot <= docs[di].slots_per_day && !found; slot++) {
                int taken = 0;
                for (int ai = 0; ai < na; ai++) {
                    if (appts[ai].doctor_id != docs[di].id) continue;
                    if (!dateEq(appts[ai].date, check)) continue;
                    if (appts[ai].slot != slot) continue;
                    if (appts[ai].status == STATUS_CANCELLED ||
                        appts[ai].status == STATUS_NOSHOW) continue;
                    taken = 1;
                    break;
                }
                if (!taken) {
                    char date_str[20], time_str[20];
                    dateToString(check, date_str);
                    slotToTime(slot, time_str);
                    printf("  %-25s | %-12s | Slot %-3d | %s\n",
                           docs[di].name, date_str, slot, time_str);
                    found = 1;
                }
            }
        }
    }

    if (!found) {
        printf("  No available slot found within the next 7 days for '%s'.\n", spec);
    }
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 9. reportFollowUpReminders
 * Patients with a completed appointment in last 30 days who have no
 * appointment scheduled in the next 3 days.
 * ------------------------------------------------------------------------- */
void reportFollowUpReminders(void)
{
    Date today;
    getTodayDate(&today);
    long todayDays = dateToDaysLocal(today);

    Appointment appts[MAX_APPOINTMENTS];
    int         na = loadAppointments(appts, MAX_APPOINTMENTS);

    Patient patients[MAX_PATIENTS];
    int     np = loadPatients(patients, MAX_PATIENTS);

    Doctor docs[MAX_DOCTORS];
    int    nd = loadDoctors(docs, MAX_DOCTORS);

    printHeader("FOLLOW-UP REMINDERS");
    printf("  %-25s %-15s %-25s %-12s\n",
           "Patient Name", "Phone", "Last Doctor Seen", "Last Appt");
    printSeparator();

    int printed = 0;
    for (int pi = 0; pi < np; pi++) {
        /* Find latest completed appointment in last 30 days. */
        int    best_ai  = -1;
        long   best_day = 0;

        for (int ai = 0; ai < na; ai++) {
            if (appts[ai].patient_id != patients[pi].id) continue;
            if (appts[ai].status != STATUS_COMPLETED) continue;
            long d = dateToDaysLocal(appts[ai].date);
            if (d < todayDays - 30 || d > todayDays) continue;
            if (best_ai < 0 || d > best_day) {
                best_ai  = ai;
                best_day = d;
            }
        }
        if (best_ai < 0) continue;

        /* Check if any appointment is booked in next 3 days. */
        int has_future = 0;
        for (int ai = 0; ai < na; ai++) {
            if (appts[ai].patient_id != patients[pi].id) continue;
            if (appts[ai].status != STATUS_BOOKED) continue;
            long d = dateToDaysLocal(appts[ai].date);
            if (d >= todayDays && d <= todayDays + 3) {
                has_future = 1;
                break;
            }
        }
        if (has_future) continue;

        /* Print reminder. */
        Doctor *doc = findDoctor(docs, nd, appts[best_ai].doctor_id);
        char date_str[20];
        dateToString(appts[best_ai].date, date_str);
        printf("  %-25s %-15s %-25s %-12s\n",
               patients[pi].name,
               patients[pi].phone,
               doc ? doc->name : "Unknown",
               date_str);
        printed++;
    }

    if (printed == 0)
        printf("  No follow-up reminders at this time.\n");
    printSeparator();
}

/* -------------------------------------------------------------------------
 * 10. generateBackup
 * Copies all .dat files and users.txt to data/backup_DDMMYYYY_HHMMSS/.
 * ------------------------------------------------------------------------- */
void generateBackup(void)
{
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);

    char backup_dir[64];
    snprintf(backup_dir, sizeof(backup_dir),
             "data/backup_%02d%02d%04d_%02d%02d%02d",
             lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900,
             lt->tm_hour, lt->tm_min,  lt->tm_sec);

    /* Create backup directory. */
#ifdef _WIN32
    char mkdir_cmd[128];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir \"%s\" 2>nul", backup_dir);
    system(mkdir_cmd);
#else
    char mkdir_cmd[128];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", backup_dir);
    system(mkdir_cmd);
#endif

    /* Files to back up. */
    const char *files[] = {
        PATIENTS_FILE,
        DOCTORS_FILE,
        APPOINTMENTS_FILE,
        BILLS_FILE,
        WAITING_FILE,
        USERS_FILE,
        NULL
    };

    for (int i = 0; files[i] != NULL; i++) {
        char src_path[128];
        char dst_path[128];
        strncpy(src_path, files[i], sizeof(src_path) - 1);
        src_path[sizeof(src_path) - 1] = '\0';

        /* Extract filename from path. */
        const char *fname = strrchr(files[i], '/');
        fname = fname ? fname + 1 : files[i];
        snprintf(dst_path, sizeof(dst_path), "%s/%s", backup_dir, fname);

        /* Binary file copy. */
        FILE *src = fopen(src_path, "rb");
        if (!src) continue; /* Skip missing files silently. */

        FILE *dst = fopen(dst_path, "wb");
        if (!dst) {
            fclose(src);
            continue;
        }

        char cbuf[4096];
        size_t bytes;
        while ((bytes = fread(cbuf, 1, sizeof(cbuf), src)) > 0)
            fwrite(cbuf, 1, bytes, dst);

        fclose(src);
        fclose(dst);
    }

    printf("Backup created at %s\n", backup_dir);
}

/* -------------------------------------------------------------------------
 * 11. showReportsMenu
 * Prints the reports sub-menu and dispatches to report functions.
 * ------------------------------------------------------------------------- */
void showReportsMenu(void)
{
    int choice;
    do {
        printHeader("REPORTS MENU");
        printf("  1.  Today's Appointments\n");
        printf("  2.  This Week's Appointments\n");
        printf("  3.  This Month's Appointments\n");
        printf("  4.  Doctor Statistics\n");
        printf("  5.  Cancellation & No-Show Rates\n");
        printf("  6.  Busiest Doctor and Day\n");
        printf("  7.  Revenue Report (by date range)\n");
        printf("  8.  Next Available Slot\n");
        printf("  9.  Follow-Up Reminders\n");
        printf("  10. Generate Backup\n");
        printf("  0.  Back to Main Menu\n");
        printSeparator();
        printf("  Enter choice: ");

        if (!readInt(&choice)) {
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1:  reportToday();              break;
            case 2:  reportThisWeek();           break;
            case 3:  reportThisMonth();          break;
            case 4:  reportDoctorStats();        break;
            case 5:  reportCancellations();      break;
            case 6:  reportBusiestDoctorAndDay(); break;
            case 7:  reportRevenue();            break;
            case 8:  reportNextAvailableSlot();  break;
            case 9:  reportFollowUpReminders();  break;
            case 10: generateBackup();           break;
            case 0:  break;
            default:
                printf("  Invalid choice. Please try again.\n");
        }
    } while (choice != 0);
}

/* Dispatcher for report sub-menu called from main. */
void reportMenu(Appointment appointments[], int acount,
                Bill bills[], int bcount,
                Patient patients[], int pcount,
                Doctor doctors[], int dcount)
{
    (void)appointments; (void)acount; (void)bills; (void)bcount;
    (void)patients; (void)pcount; (void)doctors; (void)dcount;
    showReportsMenu();
}
