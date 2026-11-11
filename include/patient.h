/*
 * patient.h - Function prototypes for patient management operations.
 */
#ifndef PATIENT_H
#define PATIENT_H

#include "types.h"

int loadPatients(Patient arr[], int max);
void savePatients(Patient arr[], int count);
int getNextPatientId(Patient arr[], int count);
int findPatientById(Patient arr[], int count, int id);
int registerPatient(Patient arr[], int *count);
void listAllPatients(Patient arr[], int count);
void searchPatient(Patient arr[], int count);
void updatePatient(Patient arr[], int count);
void patientVisitHistory(int patient_id);
void printPatientDetails(const Patient *p);
void patientMenu(Patient arr[], int *count);

#endif /* PATIENT_H */
