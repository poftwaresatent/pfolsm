CC = gcc
CPPFLAGS = -DLINUX
CFLAGS = -Wall -O2 -pipe

all: test hello-gtk dbglin

pfolsm.o: pfolsm.c pfolsm.h Makefile

test: pfolsm.o test.c Makefile
	$(CC) $(CFLAGS) -o test test.c pfolsm.o -lm

hello-gtk:  pfolsm.o hello-gtk.c Makefile
	$(CC) $(CFLAGS) -o hello-gtk hello-gtk.c pfolsm.o `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

dbglin: dbglin.c Makefile
	$(CC) $(CFLAGS) -o dbglin dbglin.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

clean:
	rm -f *~ *.o hello-gtk dbglin test
