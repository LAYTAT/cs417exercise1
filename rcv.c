//
// Created by JJ Lay on 9/14/21.
//
#include "net_include.h"

#define NAME_LENGTH 80

int gethostname(char*,size_t);

void PromptForHostName( char *my_name, char *host_name, size_t max_len );

// socket parameters
int _bind;
int _socket;
//struct sockaaddr_in address;
//socklen_t addr_length = sizeof(struct sockaaddr_in);

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

void receivePackets() {
    // try to receive length UDP packets at a time
    for (int i = 0; i < length; ++i) {

        //recv(s, )
    }
}

int main(){
    struct sockaddr_in    name;
    struct sockaddr_in    send_addr;
    struct sockaddr_in    from_addr;
    socklen_t             from_len;
    struct hostent        h_ent;
    struct hostent        *p_h_ent;
    char                  host_name[NAME_LENGTH] = {'\0'};
    char                  my_name[NAME_LENGTH] = {'\0'};
    int                   host_num;
    int                   from_ip;
    int                   ss,sr;
    fd_set                mask;
    fd_set                read_mask, write_mask, excep_mask;
    int                   bytes;
    int                   num;
    char                  mess_buf[MAX_MESS_LEN];
    char                  input_buf[80];
    struct timeval        timeout;
    bool                  receiving_flag;

    sr = socket(AF_INET, SOCK_DGRAM, 0);  /* socket for receiving (udp) */
    if (sr<0) {
        perror("rcv: socket err");
        exit(1);
    }

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);

    if ( bind( sr, (struct sockaddr *)&name, sizeof(name) ) < 0 ) {
        perror("rcv: bind err");
        exit(1);
    }

    FD_ZERO( &mask );
    FD_ZERO( &write_mask );
    FD_ZERO( &excep_mask );
    FD_SET( sr, &mask );
    for(;;)
    {
        read_mask = mask;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &timeout);
        if (num > 0) {
            if ( FD_ISSET( sr, &read_mask) ) {
                from_len = sizeof(from_addr);
                bytes = recvfrom( sr, mess_buf, sizeof(mess_buf), 0,
                                  (struct sockaddr *)&from_addr,
                                  &from_len );
                mess_buf[bytes] = 0;

                // check if not in middle of receiving
                if (receiving_flag) {
                    struct packet not_ready;
                    //packet
                    bytes = read(0, not_ready, sizeof(not_ready));
                    input_buf[bytes] = 0;

                    sendto(sr, input_buf, strlen(input_buf), 0,
                           (struct sockaddr *) &from_addr, sizeof(from_addr));
                }
            }
        } else {
            printf(".");
            fflush(0);
        }
    }

    return 0;
}
