/*
 * appointment.h - Function prototypes for appointment scheduling and queue management.
 */
#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "types.h"

int loadAppointments(Appointment arr[], int max);
void saveAppointments(Appointment arr[], int count);
int getNextAppointmentId(Appointment arr[], int count);
int findAppointmentById(Appointment arr[], int count, int id);
int isSlotBooked(Appointment arr[], int count, int doctor_id, Date date, int slot);
int isSlotFree(int doctorId, Date d, int slot);
int countBookedSlots(Appointment arr[], int count, int doctor_id, Date date);
int countTodayAppointments(Appointment arr[], int count, Date today);

int loadWaiting(WaitingEntry queue[], int max);
void saveWaiting(WaitingEntry queue[], int count);
int countWaiting(WaitingEntry queue[], int count);
void addToWaitingList(int patientId, int doctorId, Date d, int type);
int promoteFromWaitingList(int doctorId, Date d, int slot);

int bookAppointment(int patientId, int doctorId, Date d, int slot, int type);
int bookAppointmentInteractive(Patient patients[], int pcount, Doctor doctors[], int dcount, Date today);
void cancelAppointment(int appointmentId);
void cancelAppointmentInteractive(void);
void rescheduleAppointmentInteractive(Doctor doctors[], int dcount, Date today);
void showDailySchedule(Date d);
void markAppointmentStatus(Doctor doctors[], int dcount, Date today);
int emergencyBookingInteractive(Patient patients[], int pcount, Doctor doctors[], int dcount, Date today);
void viewFreeSlotsInteractive(Doctor doctors[], int dcount);
void appointmentMenu(Appointment arr[], int *acount,
                     Patient patients[], int pcount,
                     Doctor doctors[], int dcount,
                     WaitingEntry queue[], int *wcount,
                     Date today);

#endif /* APPOINTMENT_H */
