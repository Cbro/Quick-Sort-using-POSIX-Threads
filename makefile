CFLAGS =  -O3 -Fast 
LDFLAGS = -pthread
CC = gcc

qsort: qsort.o rng.o time.o
	$(LINK.c) qsort.o rng.o time.o -o qsort
	
qsort.o: qsort.c
	$(COMPILE.c) qsort.c
	
rng.o: rng.c
	$(COMPILE.c) rng.c
	
time.o: time.c
	$(COMPILE.c) time.c
	
clean:
	$(RM) qsort.o rng.o time.o qsort

indent:
	indent -kr -nut qsort.c
	indent -kr -nut rng.c
