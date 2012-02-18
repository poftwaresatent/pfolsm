CC = gcc
CPPFLAGS = -DLINUX
CFLAGS = -Wall -O2 -pipe

all: pfolsm hello-gtk

pfolsm: pfolsm.c Makefile
	$(CC) $(CFLAGS) -o pfolsm pfolsm.c -lm

hello-gtk: hello-gtk.c Makefile
	$(CC) $(CFLAGS) -o hello-gtk hello-gtk.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`
