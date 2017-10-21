# FileName        : Makefile
# Description     : Generates executables in Server Folder. 
# File Author Name: Bhallaji Venkatesan
# Reference	  : shttp://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/example-1/Makefile 


CC=gcc

CFLAGS= -O0 -g  

OUTPUT=server

CFILES= udp_server.c

SRCS= ${CFILES}
OBJS= $(CFILES:.c=.o) 

all:  ${OUTPUT}

clean:
	-rm -f *.o *.NEW *~ *.d
	-rm -f ${OUTPUT} ${GARBAGE}

server:udp_server.o
	$(CC) $(CFLAGS) -o $@ udp_server.o 

.c.o:
	$(CC) $(CFLAGS) -c $<
