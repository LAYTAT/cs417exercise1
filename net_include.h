#include <stdio.h>

#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <netdb.h>

#include <errno.h>

#define PORT	     10040

#define MAX_MESS_LEN 1400

#define file_to_be_send "ONE_GB_FILE"
#define BUFSIZE 600

struct packet {
    int seq_num;
    int size;
    int data[BUFSIZE];
};

