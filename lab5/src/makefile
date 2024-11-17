all: mutex fact deadlock

# ./fact -k 1732 --pnum=4 --mod=6571

mutex:  
	gcc mutex.c -pthread -o mutex


fact:
	gcc factmod.c -o fact -pthread

deadlock:
	gcc deadlock.c -o deadlock -pthread

clean: 
	rm mutex fact deadlock

