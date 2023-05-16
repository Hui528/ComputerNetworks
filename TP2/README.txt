Summary: Implemented socket programming between servers using UDP

Details:
serverA.c: backend serverA stores data file (dataA.txt) and communicate with the main server through UDP;
serverB.c: backend serverB stores data file (dataB.txt) and communicate with the main server through UDP; 
servermain.c: retrieve a list of Departments and specific department's student IDs from each backend server
