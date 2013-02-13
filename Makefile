# Planar First-Order Level Set Method.
# 
# Copyright (C) 2012 Roland Philippsen. All rights reserved.
#
# Released under the BSD 3-Clause License.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
# 
# - Neither the name of the copyright holder nor the names of
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.

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
	$(CC) $(CFLAGS) -o dbgpln dbgpln.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` -lm

click: click.c Makefile
	$(CC) $(CFLAGS) -o click click.c `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

noniso: noniso.c Makefile
	$(CC) $(CFLAGS) -o noniso noniso.c -lm

clean:
	rm -rf *~ *.o *.dSYM lsmgtk dbglin dbgpln click test noniso
