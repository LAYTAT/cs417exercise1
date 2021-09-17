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

#define PORT 10040
//#define PORT

#define MAX_MESS_LEN 1400

#define BUFSIZE 300
#define MAX_NACK 10

struct packet {
    int seq_num;
    int size;
    int data[BUFSIZE];
};

struct feedback {
    //feedback info
    int cumu_acks; // cumulative acks up until now
    int nack[MAX_NACK];
};
