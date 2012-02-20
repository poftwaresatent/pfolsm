CC = gcc
CPPFLAGS = -DLINUX
CFLAGS = -Wall -O2 -pipe

all: test hello-gtk

pfolsm.o: pfolsm.c pfolsm.h Makefile

test: pfolsm.o Makefile
	$(CC) $(CFLAGS) -o test test.c pfolsm.o -lm

hello-gtk: hello-gtk.c Makefile
	$(CC) $(CFLAGS) -o hello-gtk hello-gtk.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

clean:
	rm -f *~ *.o hello-gtk test
