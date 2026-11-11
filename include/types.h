/*
 * types.h - Shared type definitions, struct declarations, and macros.
 */
#ifndef TYPES_H
#define TYPES_H

/* ----------------------------- Macros ------------------------------------ */
#define MAX_SLOTS        16
#define MAX_WAITING      50
#define MAX_PATIENTS    500
#define MAX_DOCTORS      50
#define MAX_APPOINTMENTS 2000
#define MAX_BILLS        2000

#define DATA_DIR           "data/"
#define PATIENTS_FILE      "data/patients.dat"
#define DOCTORS_FILE       "data/doctors.dat"
#define APPOINTMENTS_FILE  "data/appointments.dat"
#define BILLS_FILE         "data/bills.dat"
#define USERS_FILE         "data/users.txt"
#define WAITING_FILE       "data/waiting.dat"

/* ----------------------------- Structs ----------------------------------- */

/* Represents a calendar date. */
typedef struct {
    int day;
    int month;
    int year;
} Date;

/* Represents a registered patient. */
typedef struct {
    int  id;
    char name[50];
    int  age;
    char gender;
    char phone[15];
    char blood_group[4];
    char address[80];
} Patient;

/* Represents a doctor in the system. */
typedef struct {
    int   id;
    char  name[50];
    char  specialization[30];
    char  available_days[8];   /* e.g. "MTWRFSS" subset */
    float fee;
    int   slots_per_day;
    int   is_active;
} Doctor;

/* Appointment type classification. */
typedef enum {
    TYPE_ROUTINE   = 0,
    TYPE_EMERGENCY = 1
} AppointmentType;

/* Appointment lifecycle status. */
typedef enum {
    STATUS_BOOKED    = 0,
    STATUS_COMPLETED = 1,
    STATUS_CANCELLED = 2,
    STATUS_NOSHOW    = 3
} AppointmentStatus;

/* Represents a single appointment booking. */
typedef struct {
    int  id;
    int  patient_id;
    int  doctor_id;
    Date date;
    int  slot;      /* 1-16 */
    int  type;      /* AppointmentType */
    int  status;    /* AppointmentStatus */
    char notes[100];
} Appointment;

/* Represents a bill generated for an appointment. */
typedef struct {
    int   id;
    int   appointment_id;
    float fee;
    float discount;
    float total;
    Date  date;
} Bill;

/* Represents an entry in the emergency waiting queue. */
typedef struct {
    int  patient_id;
    int  doctor_id;
    Date date;
    int  type;      /* AppointmentType */
} WaitingEntry;

#endif /* TYPES_H */
