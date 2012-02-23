CC = gcc
#CFLAGS = -Wall -O2 -pipe
CFLAGS = -Wall -O0 -g -pipe

all: test lsmgtk dbglin dbgpln

pfolsm.o: pfolsm.c pfolsm.h Makefile

test: pfolsm.o test.c Makefile
	$(CC) $(CFLAGS) -o test test.c pfolsm.o -lm

lsmgtk:  pfolsm.o lsmgtk.c Makefile
	$(CC) $(CFLAGS) -o lsmgtk lsmgtk.c pfolsm.o `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

dbglin: dbglin.c Makefile
	$(CC) $(CFLAGS) -o dbglin dbglin.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

dbgpln: dbgpln.c Makefile
	$(CC) $(CFLAGS) -o dbgpln dbgpln.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

clean:
	rm -f *~ *.o lsmgtk dbglin dbgpln test
