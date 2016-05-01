all: mphi sphi

mphi : monitores_philosophers.c
		gcc -o mphi monitores_philosophers.c -lpthread
sphi : semaphores_philosophers.c
		gcc -o sphi semaphores_philosophers.c -lpthread

clean:
		rm mphi sphi
