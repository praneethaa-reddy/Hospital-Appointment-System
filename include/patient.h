/*
 * patient.h - Function prototypes for patient management operations.
 */
#ifndef PATIENT_H
#define PATIENT_H

#include "types.h"

/* Loads all patients from PATIENTS_FILE into arr; returns the count loaded. */
int loadPatients(Patient arr[], int max);

/* Persists the patients array (count entries) to PATIENTS_FILE. */
void savePatients(Patient arr[], int count);

/* Interactive prompt to register a new patient; returns the new patient id. */
int addPatient(Patient arr[], int *count);

/* Displays full details for the patient with the given id. */
void viewPatient(Patient arr[], int count, int id);

/* Interactive prompt to update fields of an existing patient record. */
void updatePatient(Patient arr[], int count, int id);

/* Marks a patient record as deleted (soft-delete by id). */
void deletePatient(Patient arr[], int *count, int id);

/* Searches arr by name substring or phone; prints all matches. */
void searchPatient(Patient arr[], int count, const char *query);

/* Prints a tabular listing of all active patient records. */
void listAllPatients(Patient arr[], int count);

/* Returns the array index of the patient with the given id, or -1 if absent. */
int findPatientById(Patient arr[], int count, int id);

/* Entry point for the patient management sub-menu. */
void patientMenu(Patient arr[], int *count);

#endif /* PATIENT_H */
