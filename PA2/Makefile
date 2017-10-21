# FileName        : Makefile
# Description     : Generates executables in PA2 Folder. 
# File Author Name: Bhallaji Venkatesan
# Reference	  : shttp://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/example-1/Makefile 


CC=gcc

CFLAGS= -O0 -g  

OUTPUT=webserver

CFILES= webserver.c

SRCS= ${CFILES}
OBJS= $(CFILES:.c=.o) 

all:  ${OUTPUT}

clean:
	-rm -f *.o *.NEW *~ *.d
	-rm -f ${OUTPUT} ${GARBAGE}

client:udp_client.o
	$(CC) $(CFLAGS) -o $@ webserver.o 

.c.o:
	$(CC) $(CFLAGS) -c $<

