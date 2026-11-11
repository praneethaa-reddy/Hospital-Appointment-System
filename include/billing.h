/*
 * billing.h - Function prototypes for billing and payment operations.
 */
#ifndef BILLING_H
#define BILLING_H

#include "types.h"

/* Loads all bills from BILLS_FILE into arr; returns the count loaded. */
int loadBills(Bill arr[], int max);

/* Persists the bills array (count entries) to BILLS_FILE. */
void saveBills(Bill arr[], int count);

/* Generates a new bill for the given appointment; returns the new bill id. */
int generateBill(Bill arr[], int *count,
                 Appointment *appt,
                 Doctor doctors[], int dcount,
                 Date today);

/* Applies a discount percentage to the bill with the given id. */
void applyDiscount(Bill arr[], int count, int bill_id, float discount_pct);

/* Prints a formatted invoice for the bill with the given id. */
void printBill(Bill arr[], int count,
               Appointment appointments[], int acount,
               Patient patients[], int pcount,
               Doctor doctors[], int dcount,
               int bill_id);

/* Returns the array index of the bill with the given id, or -1 if absent. */
int findBillById(Bill arr[], int count, int id);

/* Returns the index of the bill linked to appointment_id, or -1. */
int findBillByAppointment(Bill arr[], int count, int appointment_id);

/* Entry point for the billing management sub-menu. */
void billingMenu(Bill arr[], int *bcount,
                 Appointment appointments[], int acount,
                 Patient patients[], int pcount,
                 Doctor doctors[], int dcount,
                 Date today);

#endif /* BILLING_H */
