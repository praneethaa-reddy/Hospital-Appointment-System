/*
 * report.h - Function prototypes for reporting and analytics.
 */
#ifndef REPORT_H
#define REPORT_H

#include "types.h"

/* Prints a summary of daily appointments for the given date. */
void reportDailySummary(Appointment arr[], int acount,
                        Doctor doctors[], int dcount,
                        Patient patients[], int pcount,
                        Date date);

/* Prints total revenue collected over the given date range (inclusive). */
void reportRevenue(Bill arr[], int bcount,
                   Appointment appointments[], int acount,
                   Date from, Date to);

/* Prints appointment counts grouped by doctor for the given date range. */
void reportDoctorLoad(Appointment arr[], int acount,
                      Doctor doctors[], int dcount,
                      Date from, Date to);

/* Prints visit counts grouped by patient for all time. */
void reportPatientVisits(Appointment arr[], int acount,
                         Patient patients[], int pcount);

/* Prints counts of each AppointmentStatus across all appointments. */
void reportStatusBreakdown(Appointment arr[], int acount);

/* Entry point for the reports sub-menu. */
void reportMenu(Appointment appointments[], int acount,
                Bill bills[], int bcount,
                Patient patients[], int pcount,
                Doctor doctors[], int dcount);

#endif /* REPORT_H */
