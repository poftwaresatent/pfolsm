CC = gcc
CPPFLAGS = -DLINUX
CFLAGS = -Wall -O2 -pipe

all: pfolsm

pfolsm: pfolsm.c Makefile
	$(CC) $(CFLAGS) -o pfolsm pfolsm.c -lm
