/*
 * seed_data.c - Standalone sample data seeder for the hospital appointment system.
 *
 * Creates: 5 doctors, 20 patients, 30 appointments (mix of statuses),
 *          and bills for all completed appointments.
 *
 * Compile: gcc -Iinclude src/seed_data.c -o seed_data
 * Run:     ./seed_data   (or seed_data.exe on Windows)
 *
 * Makefile target (add to existing Makefile):
 *   seed:
 *       $(CC) $(CFLAGS) src/seed_data.c -o seed_data
 *
 * NOTE: This program does NOT link with the rest of the project.
 *       It redefines only what it needs from types.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -------------------------------------------------------------------------
 * Inline type definitions (mirror of types.h so this file is self-contained)
 * ------------------------------------------------------------------------- */
#define MAX_SLOTS       16
#define MAX_WAITING     50
#define DATA_DIR        "data/"
#define PATIENTS_FILE   "data/patients.dat"
#define DOCTORS_FILE    "data/doctors.dat"
#define APPOINTMENTS_FILE "data/appointments.dat"
#define BILLS_FILE      "data/bills.dat"
#define USERS_FILE      "data/users.txt"
#define WAITING_FILE    "data/waiting.dat"
#define MAX_PATIENTS    500
#define MAX_DOCTORS     50
#define MAX_APPOINTMENTS 2000
#define MAX_BILLS       2000

typedef struct { int day, month, year; } Date;

typedef struct {
    int  id;
    char name[50];
    int  age;
    char gender;
    char phone[15];
    char blood_group[4];
    char address[80];
} Patient;

typedef struct {
    int   id;
    char  name[50];
    char  specialization[30];
    char  available_days[8];
    float fee;
    int   slots_per_day;
    int   is_active;
} Doctor;

typedef enum { TYPE_ROUTINE = 0, TYPE_EMERGENCY = 1 } AppointmentType;
typedef enum {
    STATUS_BOOKED = 0,
    STATUS_COMPLETED = 1,
    STATUS_CANCELLED = 2,
    STATUS_NOSHOW = 3
} AppointmentStatus;

typedef struct {
    int  id;
    int  patient_id;
    int  doctor_id;
    Date date;
    int  slot;
    int  type;
    int  status;
    char notes[100];
} Appointment;

typedef struct {
    int   id;
    int   appointment_id;
    float fee;
    float discount;
    float total;
    Date  date;
} Bill;

/* -------------------------------------------------------------------------
 * Helper: get a date offset from today.
 * offset > 0 = future, offset < 0 = past.
 * ------------------------------------------------------------------------- */
static Date dateOffset(int offset)
{
    time_t now = time(NULL);
    struct tm t = *localtime(&now);
    t.tm_mday += offset;
    mktime(&t);
    Date d;
    d.day   = t.tm_mday;
    d.month = t.tm_mon + 1;
    d.year  = t.tm_year + 1900;
    return d;
}

/* -------------------------------------------------------------------------
 * Helper: write a block of structs to a binary file.
 * ------------------------------------------------------------------------- */
static int writeAll(const char *path, const void *data, size_t elem_size, int count)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "ERROR: cannot open %s for writing\n", path);
        return 0;
    }
    fwrite(data, elem_size, count, f);
    fclose(f);
    return 1;
}

/* -------------------------------------------------------------------------
 * Helper: ensure the data/ directory exists.
 * ------------------------------------------------------------------------- */
static void ensureDataDir(void)
{
#ifdef _WIN32
    system("if not exist data mkdir data");
#else
    system("mkdir -p data");
#endif
}

/* -------------------------------------------------------------------------
 * main - seed all data files
 * ------------------------------------------------------------------------- */
int main(void)
{
    ensureDataDir();

    /* --------------------------------------------------------------------- */
    /* 1. DOCTORS (5)                                                         */
    /* --------------------------------------------------------------------- */
    /*
     * available_days encoding:
     *   M=Mon, T=Tue, W=Wed, H=Thu, F=Fri, S=Sat, U=Sun
     */
    Doctor doctors[5];
    memset(doctors, 0, sizeof(doctors));

    /* Dr. Raj Kumar - Cardiology, fee=500, Mon-Sat */
    doctors[0].id = 1;
    strncpy(doctors[0].name,           "Dr. Raj Kumar",   sizeof(doctors[0].name) - 1);
    strncpy(doctors[0].specialization, "Cardiology",      sizeof(doctors[0].specialization) - 1);
    strncpy(doctors[0].available_days, "MTWHFS",          sizeof(doctors[0].available_days) - 1);
    doctors[0].fee          = 500.0f;
    doctors[0].slots_per_day = MAX_SLOTS;
    doctors[0].is_active    = 1;

    /* Dr. Meena Sharma - Pediatrics, fee=400, Mon-Fri */
    doctors[1].id = 2;
    strncpy(doctors[1].name,           "Dr. Meena Sharma", sizeof(doctors[1].name) - 1);
    strncpy(doctors[1].specialization, "Pediatrics",       sizeof(doctors[1].specialization) - 1);
    strncpy(doctors[1].available_days, "MTWHF",            sizeof(doctors[1].available_days) - 1);
    doctors[1].fee          = 400.0f;
    doctors[1].slots_per_day = MAX_SLOTS;
    doctors[1].is_active    = 1;

    /* Dr. Zaid Khan - Orthopedics, fee=600, Mon-Sat */
    doctors[2].id = 3;
    strncpy(doctors[2].name,           "Dr. Zaid Khan",   sizeof(doctors[2].name) - 1);
    strncpy(doctors[2].specialization, "Orthopedics",     sizeof(doctors[2].specialization) - 1);
    strncpy(doctors[2].available_days, "MTWHFS",          sizeof(doctors[2].available_days) - 1);
    doctors[2].fee          = 600.0f;
    doctors[2].slots_per_day = MAX_SLOTS;
    doctors[2].is_active    = 1;

    /* Dr. Priya Nair - Dermatology, fee=350, Tue-Sat */
    doctors[3].id = 4;
    strncpy(doctors[3].name,           "Dr. Priya Nair",  sizeof(doctors[3].name) - 1);
    strncpy(doctors[3].specialization, "Dermatology",     sizeof(doctors[3].specialization) - 1);
    strncpy(doctors[3].available_days, "TWHFS",           sizeof(doctors[3].available_days) - 1);
    doctors[3].fee          = 350.0f;
    doctors[3].slots_per_day = MAX_SLOTS;
    doctors[3].is_active    = 1;

    /* Dr. Arvind Das - Neurology, fee=700, Mon-Thu */
    doctors[4].id = 5;
    strncpy(doctors[4].name,           "Dr. Arvind Das",  sizeof(doctors[4].name) - 1);
    strncpy(doctors[4].specialization, "Neurology",       sizeof(doctors[4].specialization) - 1);
    strncpy(doctors[4].available_days, "MTWH",            sizeof(doctors[4].available_days) - 1);
    doctors[4].fee          = 700.0f;
    doctors[4].slots_per_day = MAX_SLOTS;
    doctors[4].is_active    = 1;

    if (!writeAll(DOCTORS_FILE, doctors, sizeof(Doctor), 5)) return 1;

    /* --------------------------------------------------------------------- */
    /* 2. PATIENTS (20)                                                       */
    /* At least 3 seniors (age >= 60).                                        */
    /* --------------------------------------------------------------------- */
    Patient patients[20];
    memset(patients, 0, sizeof(patients));

    /* id, name, age, gender, phone, blood_group, address */
    struct {
        int   id;
        char  name[50];
        int   age;
        char  gender;
        char  phone[15];
        char  bg[4];
        char  addr[80];
    } pdata[] = {
        {  1, "Arjun Mehta",       28, 'M', "9876543210", "O+",  "12 MG Road, Bangalore"      },
        {  2, "Sunita Verma",      35, 'F', "9123456789", "A+",  "5 Lake View, Mumbai"         },
        {  3, "Ramesh Pillai",     62, 'M', "9988776655", "B+",  "7 Cross Street, Chennai"     }, /* senior */
        {  4, "Kavya Reddy",       24, 'F', "9871234560", "AB-", "3 Park Lane, Hyderabad"      },
        {  5, "Mohammed Irfan",    45, 'M', "9000112233", "O-",  "88 Nizam Road, Hyderabad"    },
        {  6, "Deepa Krishnan",    33, 'F', "9444556677", "A-",  "21 Anna Nagar, Chennai"      },
        {  7, "Suresh Babu",       68, 'M', "9321987654", "B-",  "14 Gandhi Street, Coimbatore"},/* senior */
        {  8, "Preethi Nair",      29, 'F', "9654321098", "O+",  "9 Residency Road, Bangalore" },
        {  9, "Vinod Sharma",      41, 'M', "9811223344", "AB+", "77 Sector 15, Delhi"         },
        { 10, "Anita Joshi",       55, 'F', "9922334455", "A+",  "32 Juhu, Mumbai"             },
        { 11, "Karan Singh",       19, 'M', "9100223344", "B+",  "56 Rajouri Garden, Delhi"    },
        { 12, "Lakshmi Devi",      72, 'F', "9233445566", "O+",  "2 Temple Road, Madurai"      },/* senior */
        { 13, "Arun Kumar",        37, 'M', "9566778899", "A-",  "18 Civil Lines, Jaipur"      },
        { 14, "Meera Pillai",      26, 'F', "9677889900", "B-",  "11 Banjara Hills, Hyderabad" },
        { 15, "Ravi Shankar",      48, 'M', "9755443322", "O-",  "4 Salt Lake, Kolkata"        },
        { 16, "Pooja Gupta",       31, 'F', "9800998877", "AB+", "23 DLF Phase 2, Gurgaon"     },
        { 17, "Nitin Deshpande",   53, 'M', "9166554433", "O+",  "6 Shivaji Nagar, Pune"       },
        { 18, "Asha Thomas",       22, 'F', "9277665544", "A+",  "15 Vyttila, Kochi"           },
        { 19, "Gopal Rao",         60, 'M', "9388776655", "B+",  "30 Abids, Hyderabad"         },/* senior */
        { 20, "Fathima Begum",     39, 'F', "9499887766", "AB-", "50 Frazer Town, Bangalore"   },
    };

    for (int i = 0; i < 20; i++) {
        patients[i].id  = pdata[i].id;
        patients[i].age = pdata[i].age;
        patients[i].gender = pdata[i].gender;
        strncpy(patients[i].name,        pdata[i].name,  sizeof(patients[i].name) - 1);
        strncpy(patients[i].phone,       pdata[i].phone, sizeof(patients[i].phone) - 1);
        strncpy(patients[i].blood_group, pdata[i].bg,    sizeof(patients[i].blood_group) - 1);
        strncpy(patients[i].address,     pdata[i].addr,  sizeof(patients[i].address) - 1);
    }

    if (!writeAll(PATIENTS_FILE, patients, sizeof(Patient), 20)) return 1;

    /* --------------------------------------------------------------------- */
    /* 3. APPOINTMENTS (30)                                                   */
    /* Mix: completed, booked, cancelled, noshow, emergency.                  */
    /* Past dates = completed/cancelled/noshow; future = booked.              */
    /* --------------------------------------------------------------------- */
    Appointment appts[30];
    memset(appts, 0, sizeof(appts));

    /*
     * Layout:
     *   id, patient_id, doctor_id, date_offset, slot, type, status, notes
     * date_offset: negative = past, positive = future, 0 = today
     */
    struct {
        int  id, pid, did, day_off, slot, type, status;
        char notes[100];
    } adata[] = {
        /* Completed appointments (past) */
        {  1,  1, 1, -10, 2, TYPE_ROUTINE,   STATUS_COMPLETED, "Routine cardiac checkup"       },
        {  2,  2, 2,  -9, 4, TYPE_ROUTINE,   STATUS_COMPLETED, "Child fever evaluation"         },
        {  3,  3, 3,  -8, 1, TYPE_ROUTINE,   STATUS_COMPLETED, "Knee pain assessment"           },
        {  4,  4, 4,  -7, 3, TYPE_ROUTINE,   STATUS_COMPLETED, "Skin rash consultation"         },
        {  5,  5, 5,  -6, 5, TYPE_ROUTINE,   STATUS_COMPLETED, "Headache and dizziness"         },
        {  6,  6, 1,  -5, 2, TYPE_ROUTINE,   STATUS_COMPLETED, "Blood pressure follow-up"       },
        {  7,  7, 2,  -4, 6, TYPE_ROUTINE,   STATUS_COMPLETED, "Vaccination for child"          },
        {  8,  8, 3,  -3, 3, TYPE_ROUTINE,   STATUS_COMPLETED, "Post-surgery review"            },
        {  9,  9, 4,  -2, 4, TYPE_ROUTINE,   STATUS_COMPLETED, "Acne treatment"                 },
        { 10, 10, 5,  -1, 7, TYPE_ROUTINE,   STATUS_COMPLETED, "Migraine management"            },
        { 11, 11, 1, -15, 1, TYPE_ROUTINE,   STATUS_COMPLETED, "ECG review"                     },
        { 12, 12, 2, -12, 5, TYPE_ROUTINE,   STATUS_COMPLETED, "Pediatric asthma follow-up"     },
        /* Emergency appointments */
        { 13, 13, 1,  -3, 9, TYPE_EMERGENCY, STATUS_COMPLETED, "Acute chest pain - emergency"   },
        { 14, 14, 3,  -1,10, TYPE_EMERGENCY, STATUS_COMPLETED, "Fracture assessment - emergency" },
        /* Cancelled appointments (past) */
        { 15, 15, 1,  -6, 3, TYPE_ROUTINE,   STATUS_CANCELLED, "Patient cancelled"              },
        { 16, 16, 2,  -4, 2, TYPE_ROUTINE,   STATUS_CANCELLED, "Doctor unavailable"             },
        { 17, 17, 4,  -8, 5, TYPE_ROUTINE,   STATUS_CANCELLED, "Patient request"                },
        { 18, 18, 5,  -5, 1, TYPE_ROUTINE,   STATUS_CANCELLED, "Rescheduled"                    },
        /* No-show appointments (past) */
        { 19, 19, 3,  -7, 4, TYPE_ROUTINE,   STATUS_NOSHOW,    "Patient did not arrive"         },
        { 20, 20, 4,  -9, 2, TYPE_ROUTINE,   STATUS_NOSHOW,    "No-show"                        },
        { 21,  1, 5, -11, 3, TYPE_ROUTINE,   STATUS_NOSHOW,    "No-show"                        },
        { 22,  2, 1, -13, 6, TYPE_ROUTINE,   STATUS_NOSHOW,    "No-show"                        },
        /* Future booked appointments */
        { 23,  1, 1,   2, 2, TYPE_ROUTINE,   STATUS_BOOKED,    "Follow-up cardiac checkup"      },
        { 24,  3, 3,   3, 4, TYPE_ROUTINE,   STATUS_BOOKED,    "Physiotherapy review"           },
        { 25,  5, 5,   4, 1, TYPE_ROUTINE,   STATUS_BOOKED,    "Neurology consultation"         },
        { 26,  7, 2,   5, 3, TYPE_ROUTINE,   STATUS_BOOKED,    "Pediatric checkup"              },
        { 27, 10, 4,   6, 2, TYPE_ROUTINE,   STATUS_BOOKED,    "Dermatology follow-up"          },
        { 28, 15, 1,   7, 5, TYPE_ROUTINE,   STATUS_BOOKED,    "Cardiology new visit"           },
        { 29, 18, 3,   2, 7, TYPE_ROUTINE,   STATUS_BOOKED,    "Sports injury assessment"       },
        { 30, 20, 5,   3, 8, TYPE_ROUTINE,   STATUS_BOOKED,    "Neurology initial consultation" },
    };

    for (int i = 0; i < 30; i++) {
        appts[i].id         = adata[i].id;
        appts[i].patient_id = adata[i].pid;
        appts[i].doctor_id  = adata[i].did;
        appts[i].date       = dateOffset(adata[i].day_off);
        appts[i].slot       = adata[i].slot;
        appts[i].type       = adata[i].type;
        appts[i].status     = adata[i].status;
        strncpy(appts[i].notes, adata[i].notes, sizeof(appts[i].notes) - 1);
    }

    if (!writeAll(APPOINTMENTS_FILE, appts, sizeof(Appointment), 30)) return 1;

    /* --------------------------------------------------------------------- */
    /* 4. BILLS for completed appointments                                    */
    /* Senior discount: age >= 60 -> 10%.                                     */
    /* Emergency fee: same base fee, no extra charge.                         */
    /* --------------------------------------------------------------------- */

    /* Doctor fees by doctor id (1-indexed). */
    float doc_fees[6] = {0, 500.0f, 400.0f, 600.0f, 350.0f, 700.0f};

    Bill bills[MAX_BILLS];
    int  bill_count = 0;

    for (int i = 0; i < 30; i++) {
        if (appts[i].status != STATUS_COMPLETED) continue;

        /* Find patient age. */
        int patient_age = 0;
        for (int p = 0; p < 20; p++) {
            if (patients[p].id == appts[i].patient_id) {
                patient_age = patients[p].age;
                break;
            }
        }

        float base_fee = doc_fees[appts[i].doctor_id];
        float discount = 0.0f;

        /* Senior citizen discount: 10% if age >= 60. */
        if (patient_age >= 60)
            discount += base_fee * 0.10f;

        Bill b;
        memset(&b, 0, sizeof(b));
        b.id             = bill_count + 1;
        b.appointment_id = appts[i].id;
        b.fee            = base_fee;
        b.discount       = discount;
        b.total          = base_fee - discount;
        b.date           = appts[i].date;

        bills[bill_count++] = b;
    }

    if (!writeAll(BILLS_FILE, bills, sizeof(Bill), bill_count)) return 1;

    printf("Sample data seeded successfully.\n");
    return 0;
}
