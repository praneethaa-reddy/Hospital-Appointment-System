/*
 * utils.h - Utility function prototypes for date, validation, and I/O helpers.
 */
#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* ANSI color escape codes. */
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_RESET   "\033[0m"

/* Fills d with today's date from the system clock. */
void getTodayDate(Date *d);

/* Converts a Date to an integer day count since an internal epoch. */
long dateToDays(Date d);

/* Returns the weekday of d: 0=Sunday, 1=Monday, ..., 6=Saturday. */
int getWeekday(Date d);

/* Returns 1 if d represents a valid calendar date, 0 otherwise. */
int isValidDate(Date d);

/* Returns 1 if year is a leap year, 0 otherwise. */
int isLeapYear(int year);

/* Writes the date into buf as "DD-MM-YYYY". buf must hold at least 11 chars. */
void dateToString(Date d, char *buf);

/* Compares two dates: returns -1 if a<b, 0 if equal, 1 if a>b. */
int compareDates(Date a, Date b);

/* Writes the human-readable time range for slot into buf (e.g. "09:00 AM"). */
void slotToTime(int slot, char *buf);

/* Returns 1 if phone contains exactly 10 decimal digits, 0 otherwise. */
int validatePhone(const char *phone);

/* Returns 1 if bg is one of A+,A-,B+,B-,AB+,AB-,O+,O-, 0 otherwise. */
int validateBloodGroup(const char *bg);

/* Discards all remaining characters in stdin up to and including a newline. */
void clearInputBuffer(void);

/* Reads one integer from stdin into *val. Returns 1 on success, 0 otherwise. */
int readInt(int *val);

/* Reads a line from stdin into buf (up to size-1 chars), trimming the newline. */
void readString(char *buf, int size);

/* Prints a formatted section header containing title. */
void printHeader(const char *title);

/* Prints a full-width line of dashes. */
void printSeparator(void);

/* Prints text using the given ANSI color code, then resets the color. */
void printColored(const char *text, const char *color);

#endif /* UTILS_H */
