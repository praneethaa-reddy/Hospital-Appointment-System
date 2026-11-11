/*
 * billing.h - Function prototypes for billing and invoice operations.
 */
#ifndef BILLING_H
#define BILLING_H

#include "types.h"

int loadBills(Bill arr[], int max);
void saveBills(Bill arr[], int count);
int findBillById(Bill arr[], int count, int id);
int findBillByAppointment(Bill arr[], int count, int appointment_id);
int generateBill(Bill arr[], int *count,
                 Appointment *appt,
                 Doctor doctors[], int dcount,
                 Date today);
void generateBillById(int appointmentId);
void applyDiscount(Bill arr[], int count, int bill_id, float discount_pct);
void printBill(Bill arr[], int count,
               Appointment appointments[], int acount,
               Patient patients[], int pcount,
               Doctor doctors[], int dcount,
               int bill_id);
void billingMenu(Bill arr[], int *bcount,
                 Appointment appointments[], int acount,
                 Patient patients[], int pcount,
                 Doctor doctors[], int dcount,
                 Date today);

#endif /* BILLING_H */
