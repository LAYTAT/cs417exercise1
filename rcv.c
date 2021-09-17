//
// Created by JJ Lay on 9/14/21.
//
#include "net_include.h"


// socket parameters
int PORT;
int _bind;
int _socket;
//struct sockaaddr_in address;
socklen_t addr_length = sizeof(struct sockaaddr_in);

// transferred file parameters
int recv_len = 0;
int send_len = 0;
int file;
int fileSize;
int remain_data = 0;
int received_data = 0;

// Packets parameters
int length = 6; // Number of packets to be sent at a single time
struct packet _packet;
struct packet packets[5];
struct feedback {
    //feedback info
    int cumu_acks; // cumulative acks up until now
    int nack[length];
};

void* receivePackets() {
    // try to receive length UDP packets at a time
    for (int i = 0; i < length; ++i) {

        //recv(s, )
    }
}

int main(){
    printf("hello there!")
    return 0;
}