socket:
	gcc -Wall -o healthcenterserver healthcenterserver.c -lresolv -lnsl -lsocket -lpthread
	gcc -Wall -o patient1 patient1.c -lresolv -lnsl -lsocket
	gcc -Wall -o patient2 patient2.c -lresolv -lnsl -lsocket
	gcc -Wall -o doctor doctor.c -lresolv -lnsl -lsocket
