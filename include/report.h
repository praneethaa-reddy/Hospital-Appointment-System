/*
 * report.h - Function prototypes for reporting, analytics, and backup.
 */
#ifndef REPORT_H
#define REPORT_H

#include "types.h"

void reportToday(void);
void reportThisWeek(void);
void reportThisMonth(void);
void reportDoctorStats(void);
void reportCancellations(void);
void reportBusiestDoctorAndDay(void);
void reportRevenue(void);
void reportNextAvailableSlot(void);
void reportFollowUpReminders(void);
void generateBackup(void);
void showReportsMenu(void);

void reportMenu(Appointment appointments[], int acount,
                Bill bills[], int bcount,
                Patient patients[], int pcount,
                Doctor doctors[], int dcount);

#endif /* REPORT_H */
