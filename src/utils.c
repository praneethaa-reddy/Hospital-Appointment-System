/*
 * utils.c - Implementations of all shared utility functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "utils.h"

/* Fills d with today's date from the system clock. */
void getTodayDate(Date *d)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    d->day   = tm_info->tm_mday;
    d->month = tm_info->tm_mon + 1;
    d->year  = tm_info->tm_year + 1900;
}

/* Returns 1 if year is a leap year, 0 otherwise. */
int isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* Converts a Date to an integer day count since an internal epoch (1970-01-01). */
long dateToDays(Date d)
{
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_mday = d.day;
    t.tm_mon  = d.month - 1;
    t.tm_year = d.year - 1900;
    t.tm_isdst = -1;
    time_t epoch = mktime(&t);
    if (epoch == (time_t)-1) return -1;
    return (long)(epoch / 86400L);
}

/* Returns the weekday of d: 0=Sunday, 1=Monday, ..., 6=Saturday. */
int getWeekday(Date d)
{
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_mday = d.day;
    t.tm_mon  = d.month - 1;
    t.tm_year = d.year - 1900;
    t.tm_isdst = -1;
    mktime(&t);
    return t.tm_wday;
}

/* Returns 1 if d represents a valid calendar date, 0 otherwise. */
int isValidDate(Date d)
{
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (d.year < 1900 || d.year > 9999) return 0;
    if (d.month < 1   || d.month > 12)  return 0;

    if (isLeapYear(d.year)) {
        days_in_month[2] = 29;
    }

    if (d.day < 1 || d.day > days_in_month[d.month]) return 0;
    return 1;
}

/* Writes the date into buf as "DD-MM-YYYY". buf must hold at least 11 chars. */
void dateToString(Date d, char *buf)
{
    sprintf(buf, "%02d-%02d-%04d", d.day, d.month, d.year);
}

/* Compares two dates: returns -1 if a<b, 0 if equal, 1 if a>b. */
int compareDates(Date a, Date b)
{
    if (a.year  != b.year)  return a.year  < b.year  ? -1 : 1;
    if (a.month != b.month) return a.month < b.month ? -1 : 1;
    if (a.day   != b.day)   return a.day   < b.day   ? -1 : 1;
    return 0;
}

/* Writes the human-readable start time for slot into buf (e.g. "09:00 AM"). */
void slotToTime(int slot, char *buf)
{
    int hour, minute;
    if (slot >= 1 && slot <= 8) {
        int offset  = (slot - 1) * 30;
        hour   = 9 + offset / 60;
        minute = offset % 60;
        sprintf(buf, "%02d:%02d AM", hour, minute);
    } else if (slot >= 9 && slot <= 16) {
        int offset  = (slot - 9) * 30;
        hour   = 14 + offset / 60;
        minute = offset % 60;
        if (hour < 12)
            sprintf(buf, "%02d:%02d AM", hour, minute);
        else if (hour == 12)
            sprintf(buf, "%02d:%02d PM", hour, minute);
        else
            sprintf(buf, "%02d:%02d PM", hour - 12, minute);
    } else {
        sprintf(buf, "Invalid");
    }
}

/* Returns 1 if phone contains exactly 10 decimal digits, 0 otherwise. */
int validatePhone(const char *phone)
{
    int i;
    if (!phone) return 0;
    if ((int)strlen(phone) != 10) return 0;
    for (i = 0; i < 10; i++) {
        if (!isdigit((unsigned char)phone[i])) return 0;
    }
    return 1;
}

/* Returns 1 if bg is one of A+,A-,B+,B-,AB+,AB-,O+,O-, 0 otherwise. */
int validateBloodGroup(const char *bg)
{
    const char *valid[] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-", NULL};
    int i;
    if (!bg) return 0;
    for (i = 0; valid[i] != NULL; i++) {
        if (strcmp(bg, valid[i]) == 0) return 1;
    }
    return 0;
}

/* Discards all remaining characters in stdin up to and including a newline. */
void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/* Reads one integer from stdin into *val. Returns 1 on success, 0 otherwise. */
int readInt(int *val)
{
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) return 0;
    if (sscanf(buf, "%d", val) != 1) return 0;
    return 1;
}

/* Reads a line from stdin into buf (up to size-1 chars), trimming the newline. */
void readString(char *buf, int size)
{
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
}

/* Prints a formatted section header containing title. */
void printHeader(const char *title)
{
    int i, len, pad;
    const int width = 60;
    printf("\n");
    printSeparator();
    len = (int)strlen(title);
    pad = (width - len) / 2;
    for (i = 0; i < pad; i++) printf(" ");
    printf("%s\n", title);
    printSeparator();
}

/* Prints a full-width line of dashes. */
void printSeparator(void)
{
    int i;
    for (i = 0; i < 60; i++) printf("-");
    printf("\n");
}

/* Prints text using the given ANSI color code, then resets the color. */
void printColored(const char *text, const char *color)
{
    printf("%s%s%s", color, text, COLOR_RESET);
}
