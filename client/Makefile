#
#  $Id$
#
#
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met: 1. Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer. 2.
#  Redistributions in binary form must reproduce the above copyright notice,
#  this list of conditions and the following disclaimer in the documentation
#  and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
#  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.
#
#

CC      = gcc
DEBUG   = # -DEADBUG 
CFLAGS  = -g -static -Wall -I. -I./util 

INSTALL = /usr/bin/ginstall -c 

OBJS    =  env_vars.o main.o
PWD     = `pwd`
.c.o:
	$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@ 

all:	util global
	$(CC) -o minicvs $(OBJS) -l ./util/util.a -lz 	

global:	$(OBJS) 

util:	
	cd util; \
	make 
	
utilclean:
	cd util; \
	make clean
install:
	$(INSTALL) -c -m 0755 -g bin minicvs /bin
clean: utilclean
	rm -Rf *~ minicvs *.o 