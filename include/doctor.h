/*
 * doctor.h - Function prototypes for doctor management operations.
 */
#ifndef DOCTOR_H
#define DOCTOR_H

#include "types.h"

int loadDoctors(Doctor arr[], int max);
void saveDoctors(Doctor arr[], int count);
int getNextDoctorId(Doctor arr[], int count);
int findDoctorById(Doctor arr[], int count, int id);
int isDoctorAvailable(const Doctor *doc, int weekday);
void printDoctorDays(const Doctor *d);
int addDoctor(Doctor arr[], int *count);
void listAllDoctors(Doctor arr[], int count);
void viewDoctorsBySpecialization(Doctor arr[], int count);
int updateDoctor(Doctor arr[], int count);
int removeDoctor(Doctor arr[], int count);
void viewDoctorFreeSlots(Doctor arr[], int count, Date today);
void doctorMenu(Doctor arr[], int *count);

#endif /* DOCTOR_H */
