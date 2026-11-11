/*
 * doctor.h - Function prototypes for doctor management operations.
 */
#ifndef DOCTOR_H
#define DOCTOR_H

#include "types.h"

/* Loads all doctors from DOCTORS_FILE into arr; returns the count loaded. */
int loadDoctors(Doctor arr[], int max);

/* Persists the doctors array (count entries) to DOCTORS_FILE. */
void saveDoctors(Doctor arr[], int count);

/* Interactive prompt to register a new doctor; returns the new doctor id. */
int addDoctor(Doctor arr[], int *count);

/* Displays full profile for the doctor with the given id. */
void viewDoctor(Doctor arr[], int count, int id);

/* Interactive prompt to update fields of an existing doctor record. */
void updateDoctor(Doctor arr[], int count, int id);

/* Sets is_active=0 for the doctor with the given id (soft deactivation). */
void deactivateDoctor(Doctor arr[], int count, int id);

/* Prints a tabular listing of all active doctors. */
void listAllDoctors(Doctor arr[], int count);

/* Returns the array index of the doctor with the given id, or -1 if absent. */
int findDoctorById(Doctor arr[], int count, int id);

/* Returns 1 if the doctor is available on the weekday represented by day (0=Sun..6=Sat). */
int isDoctorAvailable(Doctor *doc, int day);

/* Entry point for the doctor management sub-menu. */
void doctorMenu(Doctor arr[], int *count);

#endif /* DOCTOR_H */
