all: minicvs

CC = gcc
CFLAGS = -g -Wall
INCLUDE = . 
OBJECTS = strlcpy.o xmalloc.o valloc.o root.o  buffer.o
HEADERS = mcvs.h strlcpy.h xmalloc.h valloc.h buffer.h

.c.o:  $(HEADERS) -lz
#-lpthread
minicvs:  $(OBJECTS) $(HEADERS)
	gcc -o $@ $(OBJECTS) -lz 
#-lpthread
clean:
	rm -fr *.o minicvs
