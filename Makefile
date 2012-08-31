CC = gcc
#CFLAGS = -Wall -O2 -pipe
CFLAGS = -Wall -O0 -g -pipe

#all: test lsmgtk dbglin dbgpln
all: dbgpln noniso

pfolsm.o: pfolsm.c pfolsm.h Makefile

test: pfolsm.o test.c Makefile
	$(CC) $(CFLAGS) -o test test.c pfolsm.o -lm

lsmgtk:  pfolsm.o lsmgtk.c Makefile
	$(CC) $(CFLAGS) -o lsmgtk lsmgtk.c pfolsm.o `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

dbglin: dbglin.c Makefile
	$(CC) $(CFLAGS) -o dbglin dbglin.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

dbgpln: dbgpln.c Makefile
	$(CC) $(CFLAGS) -o dbgpln dbgpln.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

click: click.c Makefile
	$(CC) $(CFLAGS) -o click click.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

noniso: noniso.c Makefile
	$(CC) $(CFLAGS) -o noniso noniso.c 

clean:
	rm -rf *~ *.o *.dSYM lsmgtk dbglin dbgpln click test noniso
