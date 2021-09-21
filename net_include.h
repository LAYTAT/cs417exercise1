#include <stdio.h>

#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <netdb.h>
#include "cs417Ex1/sendto_dbg.h"
#include <sys/time.h>
#include <errno.h>

#define PORT (10040)
//#define PORT (10280)
#define MAX_MESS_LEN (1400)
#define BUFSIZE 100 //(10)
#define WINDOW_SIZE 100//(3)
#define FEEDBACK_DEVIDER (100)
#define MAX_LOSS_RATE_PERCENT (25)

struct File_Data {
    unsigned char data[BUFSIZE];
};

struct packet {
    int type;           // 0 for uninitialize packet
                        // 1 for Sender Init Packet
                        // 2 for Sender Packet
                        // 3 for Feedback Packet
                        // 4 for rejection to sender
                        // 5 for saying ready to sender
                        // 6 for last Sender Packect
                        // 7 for last ack
    int seq_num;        // for Sender Packet , -1 for nothing
    int size;           // final Sender Packet size will be smaller than BUFSIZE 
                        // also can store the lenght of filename
    int cumu_acks;      // cumulative acks specified by  Feedback Packet
    int nack[WINDOW_SIZE]; // lost packets specified by Feedback Packet
    struct File_Data data; // where the Sender Packet stores their data and Sender Init Packet store the dest file name
};
