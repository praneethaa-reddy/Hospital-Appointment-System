# Hospital Patient and Appointment Scheduling System - Project Report

## 1. Problem Statement

Hospitals and outpatient clinics that maintain paper-based appointment registers face several operational challenges:
- Double bookings caused by simultaneous manual entries or lack of centralized slot tracking.
- Prolonged waiting times for patients due to unstructured scheduling.
- Misplaced or fragmented medical visit histories and consultation records.
- Inability to dynamically prioritize emergency consultations ahead of routine checkups.
- Difficulties in maintaining audit trails, generating invoices, and analyzing departmental workloads.

To address these issues, this project implements a console-based Hospital Patient and Appointment Scheduling System in standard C. The solution eliminates reliance on external libraries or third-party databases, persisting all state in binary and text files while enforcing data validation, concurrency clash rules, queue-based priority waiting lists, and automated billing discounts.

## 2. System Objectives

- Accurate Record Management: Maintain comprehensive patient and doctor profiles.
- Conflict Prevention: Enforce mutual exclusivity on doctor-slot pairings and patient schedules.
- Emergency Prioritization: Insert urgent consultations ahead of routine visits.
- Automated Queue Management: Implement a first-come, first-served queue with priority override that promotes waiting-list candidates when booked slots are cancelled.
- Consultation Billing: Compute consultation invoices with automated age-based (senior citizen 60+) and recency-based (repeat visits within 7 days) discounts.
- Operational Reporting: Provide departmental performance summaries, doctor workloads, cancellation metrics, revenue statistics, and data backup mechanisms.

## 3. Data Structures and Storage Design

### Core Data Structures

```c
typedef struct {
    int day;
    int month;
    int year;
} Date;

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

typedef struct {
    int  patient_id;
    int  doctor_id;
    Date date;
    int  type;
} WaitingEntry;
```

### Storage File Specifications

1. `data/patients.dat`: Binary records of `Patient` (160 bytes per entry).
2. `data/doctors.dat`: Binary records of `Doctor` (104 bytes per entry).
3. `data/appointments.dat`: Binary records of `Appointment` (136 bytes per entry).
4. `data/bills.dat`: Binary records of `Bill` (32 bytes per entry).
5. `data/waiting.dat`: Binary records of `WaitingEntry` (24 bytes per entry).
6. `data/users.txt`: User authentication records in `username:password` plain-text format.

## 4. Program Flowchart and Architectural Diagrams

### Architectural Flowchart

```text
       +---------------------------------------------+
       |                  Start                      |
       +---------------------------------------------+
                              |
                              v
       +---------------------------------------------+
       |   Initialize Date via time.h Clock          |
       |   Load Binary Files (patients, doctors,     |
       |   appointments, bills, waiting queue)       |
       +---------------------------------------------+
                              |
                              v
       +---------------------------------------------+
       |          User Authentication                |
       |        (3 Attempts allowed)                 |
       +---------------------------------------------+
                 |                         |
         [Success]                 [Failure 3x]
                 |                         |
                 v                         v
       +-----------------------+     +---------------+
       | Show Today's Schedule |     | Exit Program  |
       |       Dashboard       |     +---------------+
       +-----------------------+
                 |
                 v
       +-----------------------+
+----->|       Main Menu       |<-----------------------+
|      +-----------------------+                        |
|                 |                                     |
|    +------------+------------+                        |
|    |  Select Option (1 - 10) |                        |
|    +-------------------------+                        |
|                 |                                     |
|    +------------+---------------------------+         |
|    |                                        |         |
|    v                                        v         |
| [Patient / Doctor Ops]            [Appointment Ops]   |
| - Register Patient                - Book Appointment  |
| - Search Patient                  - Clash Detection   |
| - Update Patient                  - Emergency Booking |
| - Add / List Doctors              - Cancel / Promote  |
| - Deactivate Doctor               - Reschedule        |
|    |                                        |         |
|    |   +------------------------------------+         |
|    |   |                                              |
|    v   v                                              |
| [Billing & Reports]                                   |
| - Generate Bill (10% senior, 5% repeat)               |
| - Analytical Reports                                  |
| - Timestamped Backups                                 |
|    |                                                  |
|    +--------------------------------------------------+
```

### Queue Management and Slot Promotion Logic

```text
       [ Appointment Cancellation Triggered ]
                         |
                         v
       [ Mark Appointment Status as STATUS_CANCELLED ]
       [ Write updated record to appointments.dat     ]
                         |
                         v
       [ Scan data/waiting.dat for Doctor ID and Date ]
                         |
             +-----------+-----------+
             |                       |
       [Match Found]           [No Waiting Entries]
             |                       |
             v                       v
     [ Dequeue Candidate ]      [ Slot Remains FREE ]
     [ Book Freed Slot   ]           |
     [ Shift Queue Array ]           v
     [ Save waiting.dat  ]        [ Done ]
             |
             v
     [ Print Notification ]
     [ Patient Auto-Booked ]
```

## 5. Console User Interface and Workflow Captures

### 5.1 Startup Schedule Dashboard

```text
============== HOSPITAL SCHEDULE DASHBOARD ==============
Today: Wednesday, 11-11-2026

Appointments Today : 18      Completed : 7
Emergency Waiting  : 1       Cancelled : 2

Doctor              Specialization   Booked   Free Slots
------------------------------------------------------------
Dr. Rao             Cardiology       12/16    4
Dr. Meena           Pediatrics       9/16     7
Dr. Khan            Orthopedics      16/16    0 (Waiting: 3)
Dr. Priya Nair      Dermatology      0/16     16
Dr. Arvind Das      Neurology        0/16     16
=========================================================
Press Enter to continue...
```

### 5.2 Main Menu Navigation

```text
------------------------------------------------------------
  HOSPITAL PATIENT AND APPOINTMENT SCHEDULING SYSTEM
------------------------------------------------------------
  1. Register Patient        6. Cancel / Reschedule
  2. Search Patient          7. Daily Schedule
  3. Add / View Doctors      8. Emergency Booking
  4. Book Appointment        9. Billing
  5. View Free Slots         10. Reports
  0. Exit
------------------------------------------------------------
Enter your choice:
```

### 5.3 Demonstration: Booking and Clash Detection

When booking an appointment:
```text
------------------------------------------------------------
                       BOOK APPOINTMENT
------------------------------------------------------------
Enter Patient ID: 1002
Patient: Sneha Patel (Age: 28)

Available Doctors:
  101. Dr. Rao                  (Cardiology     ) Fee: 500.00
  102. Dr. Meena                (Pediatrics     ) Fee: 400.00
  103. Dr. Khan                 (Orthopedics    ) Fee: 600.00
  104. Dr. Priya Nair           (Dermatology    ) Fee: 350.00
  105. Dr. Arvind Das           (Neurology      ) Fee: 700.00

Enter Doctor ID: 101
Enter Appointment Date (DD-MM-YYYY): 11-11-2026

Dr. Rao (Cardiology)  Date: 11-11-2026
Slot  Time         Status
------------------------------------------------------------
1     09:00 AM     Booked
2     09:30 AM     Booked
3     10:00 AM     Booked
4     10:30 AM     Booked
5     11:00 AM     Booked
6     11:30 AM     Booked
7     12:00 PM     Booked
8     12:30 PM     Booked
9     02:00 PM     Booked
10    02:30 PM     Booked
11    03:00 PM     Booked
12    03:30 PM     Booked
13    04:00 PM     FREE
14    04:30 PM     FREE
15    05:00 PM     FREE
16    05:30 PM     FREE
------------------------------------------------------------
Choose a slot number: 1
Error: Selected slot 1 is already booked.
```

When a free slot is selected (e.g. Slot 13):
```text
Choose a slot number: 13
Appointment successfully booked!
Appointment ID: 34
```

Attempting to book a second appointment for patient 1002 in slot 13 on 11-11-2026 with Dr. Priya Nair triggers patient conflict detection:
```text
Enter Doctor ID: 104
Enter Appointment Date (DD-MM-YYYY): 11-11-2026
Choose a slot number: 13
Error: Patient already has an appointment booked at this slot.
```

### 5.4 Demonstration: Cancellation and Waiting List Promotion

When Dr. Khan's slots are completely full (16/16), patients join the waiting queue. Cancelling an appointment automatically frees the slot and promotes the first queued candidate:

```text
------------------------------------------------------------
                     CANCEL APPOINTMENT
------------------------------------------------------------
Enter Appointment ID to cancel: 24
Appointment successfully cancelled. Slot is now free.

[WAITING LIST PROMOTION] Patient ID 1015 automatically booked for Slot 3!
```

Patient 1015 (Emergency waiting entry) is dequeued from `waiting.dat` and booked into Slot 3 immediately.

### 5.5 Demonstration: Emergency Prioritization

```text
------------------------------------------------------------
                     EMERGENCY BOOKING
------------------------------------------------------------
Enter Patient ID: 1006
Available Doctors:
  101. Dr. Rao                  (Cardiology     )
  102. Dr. Meena                (Pediatrics     )
  103. Dr. Khan                 (Orthopedics    )
Enter Doctor ID: 101

Emergency appointment successfully booked at earliest slot today!
Appointment ID : 35
Assigned Slot  : 14 (04:30 PM)
```

### 5.6 Demonstration: Consultation Billing with Discounts

```text
------------- BILL No: 2001 -------------
Date: 11-11-2026
Patient : Ravi Kumar (ID 1042)
Doctor  : Dr. Rao (Cardiology)
-----------------------------------------
Fee                                500.00
Senior Citizen Discount            -50.00
-----------------------------------------
Total Payable                      450.00
Wishing you a speedy recovery.
-----------------------------------------
```

## 6. Testing and Verification Results

| Test Scenario | Input / Action | Expected Result | Actual Result | Status |
|---------------|----------------|-----------------|---------------|--------|
| Doctor Slot Clash | Book Doctor 101, Date 11-11-2026, Slot 1 twice | System rejects duplicate booking | "Error: Selected slot is already booked" | Pass |
| Patient Slot Conflict | Book Patient 1001 for Slot 1 with Doctor 101 and Doctor 102 | System blocks conflicting appointment | "Error: Patient already has an appointment booked at this slot" | Pass |
| Past Date Validation | Enter date 10-11-2026 (yesterday) | Rejects booking | "Past dates are rejected" | Pass |
| Leap Year Validation | Date 29-02-2028 vs 31-02-2027 | 2028 accepted, 2027 rejected | Validated accurately via calendar table and isLeapYear() | Pass |
| Doctor Off-day Validation | Book Doctor on unscheduled day | Rejection of booking | "Doctor does not consult on this weekday" | Pass |
| Waiting List Promotion | Cancel appointment on full schedule | Candidate promoted from waiting queue into freed slot | Automated promotion verified | Pass |
| Emergency Allocation | Emergency booking on current date | Earliest free slot allocated or top queue placement | Verified | Pass |
| Senior Discount | Patient age 60 vs age 59 | 10% discount applied only at age 60+ | Exactly verified | Pass |
| Repeat Visit Discount | Completed appointment within 7 days | Additional 5% discount applied | Verified | Pass |
| Non-numeric Input | Alphabetic string entered at integer prompt | Safe recovery without infinite loop | Handled safely via readInt() and clearInputBuffer() | Pass |
| Persistence Integrity | Exit application and restart | All records preserved from disk | Verified via binary file reloading | Pass |

## 7. Future Scope

1. Networked Multi-terminal Access: Extend file I/O to client-server sockets to allow simultaneous receptionist desks.
2. SMS and Email Notifications: Interface with standard SMTP or SMS gateway APIs for booking confirmation and appointment reminders.
3. Doctor Leave Management: Support ad-hoc doctor unavailability windows and bulk rescheduling.
4. Electronic Medical Records (EMR): Attach diagnostic lab reports, vitals, and prescription summaries to appointment records.
5. Patient Portal Interface: Provide patient-facing self-service kiosks or lightweight web endpoints for self-booking.
