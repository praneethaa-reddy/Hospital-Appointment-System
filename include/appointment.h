/*
 * appointment.h - Function prototypes for appointment scheduling operations.
 */
#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "types.h"

/* Loads all appointments from APPOINTMENTS_FILE into arr; returns count. */
int loadAppointments(Appointment arr[], int max);

/* Persists the appointments array (count entries) to APPOINTMENTS_FILE. */
void saveAppointments(Appointment arr[], int count);

/* Interactive prompt to book a new appointment; returns the new appointment id. */
int bookAppointment(Appointment arr[], int *count,
                    Patient patients[], int pcount,
                    Doctor doctors[], int dcount,
                    Date today);

/* Cancels the appointment with the given id. */
void cancelAppointment(Appointment arr[], int count, int id);

/* Marks the appointment with the given id as STATUS_COMPLETED. */
void completeAppointment(Appointment arr[], int count, int id);

/* Marks the appointment with the given id as STATUS_NOSHOW. */
void markNoShow(Appointment arr[], int count, int id);

/* Prints all appointments for the given doctor on the given date. */
void viewDoctorSchedule(Appointment arr[], int count,
                        Doctor doctors[], int dcount,
                        int doctor_id, Date date);

/* Prints all appointments for the given patient. */
void viewPatientHistory(Appointment arr[], int count,
                        Patient patients[], int pcount,
                        int patient_id);

/* Returns the array index of the appointment with the given id, or -1. */
int findAppointmentById(Appointment arr[], int count, int id);

/* Returns 1 if the given slot is already booked for doctor_id on date. */
int isSlotBooked(Appointment arr[], int count,
                 int doctor_id, Date date, int slot);

/* Counts booked slots for doctor_id on date. */
int countBookedSlots(Appointment arr[], int count,
                     int doctor_id, Date date);

/* Counts today's appointments with STATUS_BOOKED. */
int countTodayAppointments(Appointment arr[], int count, Date today);

/* Loads the waiting queue from WAITING_FILE; returns count. */
int loadWaiting(WaitingEntry queue[], int max);

/* Persists the waiting queue (count entries) to WAITING_FILE. */
void saveWaiting(WaitingEntry queue[], int count);

/* Adds a patient to the emergency waiting queue. */
void addToWaiting(WaitingEntry queue[], int *count,
                  int patient_id, int doctor_id, Date date);

/* Counts emergency entries in the waiting queue. */
int countWaiting(WaitingEntry queue[], int count);

/* Entry point for the appointment management sub-menu. */
void appointmentMenu(Appointment arr[], int *acount,
                     Patient patients[], int pcount,
                     Doctor doctors[], int dcount,
                     WaitingEntry queue[], int *wcount,
                     Date today);

#endif /* APPOINTMENT_H */
