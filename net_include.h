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
//#define PORT 10280

#define MAX_MESS_LEN 1400

#define BUFSIZE 300
#define MAX_NACK 10

struct packet {
    int type;           // 0 for Sender Init Packet
                        // 1 for Receiver Response to Senser
                        // 2 for Sender Packet
                        // 3 for Feedback Packet
    int seq_num;        // for Sender Packet
    int size;           // final Sender Packet size will be smaller than BUFSIZE
    int cumu_acks;      // cumulative acks specified by  Feedback Packet
    int nack[MAX_NACK]; // lost packets specified by Feedback Packet
    unsigned char data[BUFSIZE]; // where the Sender Packet stores their data and Sender Init Packet store the dest file name
};
