CC=gcc

CFLAGS = -g -c -Wall -pedantic
#CFLAGS = -ansi -c -Wall -pedantic

all: ucast net_client net_server myip file_copy

net_server: net_server.o
	    $(CC) -o net_server net_server.o  

net_client: net_client.o
	    $(CC) -o net_client net_client.o

ucast: ucast.o
	    $(CC) -o ucast ucast.o

myip: myip.o
	    $(CC) -o myip myip.o

file_copy: file_copy.o
	    $(CC) -o file_copy file_copy.o

clean:
	rm *.o
	rm net_server 
	rm net_client
	rm ucast
	rm myip
	rm file_copy

%.o:    %.c
	$(CC) $(CFLAGS) $*.c


