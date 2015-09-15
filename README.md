online-medical-appointment-system
=================================

Simulate an online medical appointment system using a hybrid	architecture of TCP and UDP sockets

The project has three major phases:
  1)	Authenticating	into the health center server,
  2)	requesting available appointments and reserving one of them, and
  3) sending insurance information to the corresponding doctor to get an estimated cost for the visit.

a. What the code files are and what each one of them does.

    (i) healthcenterserver.c : Create HealthCenter Serve(TCP socket  Port - 21720) and handle incoming connections(concurrent) from patients and information requests . ( Multithreading implemented )
    (ii) patient1.c  : Create client patient1 to connect to health center server ( TCP socket ) to make an appointment and connect to doctor server ( UDP socket ) to get cost corresponding to his insurance with the appointed doctor.
    (iii) patient2.c  : Create client patient2 to connect to health center server ( TCP socket ) to make an appointment and connect to doctor server ( UDP socket ) to get cost corresponding to his insurance with the appointed doctor.
    (iv) doctor.c : Create 2 doctor servers - doctor1 and doctor2 (UDP sockets) to handles UDP requests from patients and send cost w.r.t received insurance plan of patient.
    (v)  healthcenterserver.h : Datastructures for users_info, availabilityinfo, server_handler_args.
    (vi) doctor.h : Datastructure for doc_insurance_info.
    (vii) patient1.h : Datastructures for patient_info , doctor_info.
    (viii) patient2.h : Datastructures for patient_info , doctor_info.

b. What should one do to run your programs.

    (i)   make socket
    (ii)  ./healthcenterserver
    (iii) ./doctor
    (iv) ./patient1
    (v)  ./patient2

c. The format of all the messages exchanged.
       Phase1 :
                Patient (client ) sends auhentication request to healthcenterserver.   "authenticate username password"
                Health center server sends response  to patient. "success" or "failure"
       Phase2 :
                Patient sends request to Healthcenter server for available slots. "available"
                Health center server sends availability list to patient. eg: "1 Tue 01pm"
                Patient sends preference information  to healthcenterserver. eg : "selection 1"
                Health center server sends doctor and port information to patient eg:  "doc2 41720".
        Phase3 :
                Patient sends his insurance plan information to doctor. eg: "insurance1"
                Doctor sends  cost estimation to patient. eg: "30"

d. Reused Code: Did you use code from	anywhere for your project?
       code blocks used from www.beej.us/guide/bgnet/  for stream socket server - healthcenterserver.c and client - patient1.c amd patient2.c
