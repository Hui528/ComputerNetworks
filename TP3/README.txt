Summary: Implemented a Student Performance Analysis system with UDP and TCP.

Details: 
serverA.c: backend serverA stores data file (dataA.csv) and communicate with the main server through UDP;
serverB.c: backend serverB stores data file (dataB.csv) and communicate with the main server through UDP; 
servermain.c: retrieve a list of Departments and specific department's student IDs from each backend server and communicate with clients
client.c: communicate with servermain.c through TCP

extra creadit description:
If the student's department has other students, searching this student's department, and find the one with closest GPA (this is how I define "similarity");
If the student's department has no other students, searching all departments to find the one with closest GPA.
