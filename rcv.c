//
// Created by JJ Lay on 9/14/21.
//
#include "net_include.h"

#define NAME_LENGTH 80

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
    struct sockaddr_in    serv_addr;
    struct sockaddr_in    client_addr;
    struct sockaddr_in    from_addr;
    socklen_t             from_len;
    char                  my_name[NAME_LENGTH] = {'\0'};
    int                   socket_fd;
    fd_set                mask;
    fd_set                read_mask, write_mask, excep_mask;
    int                   bytes;
    int                   num;
    char                  mess_buf[MAX_MESS_LEN];
    char                  input_buf[80];
    struct timeval        timeout;
    int                  receiving_flag;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);  /* server socket for communicating with clients */
    if (socket_fd<0) {
        perror("rcv: socket err");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    memset(&from_addr, 0, sizeof(from_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    

    if ( bind( socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0 ) { /*server created!*/
        perror("rcv: bind err");
        exit(1);
    }


    FD_ZERO( &mask );
    FD_ZERO( &write_mask );
    FD_ZERO( &excep_mask );
    FD_SET( socket_fd, &mask );
    for(;;)
    {
        read_mask = mask;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &timeout);
        if (num > 0) {
            if ( FD_ISSET( socket_fd, &read_mask) ) {
                from_len = sizeof(from_addr);
                bytes = recvfrom( socket_fd, mess_buf, sizeof(mess_buf), 0,
                                  (struct sockaddr *)&from_addr,
                                  &from_len );


                //packet unserilazation 
                //if initial packet recieved and server not busy 
                mess_buf[bytes] = 0;

                // check if not in middle of receiving
                if (receiving_flag) {
                    struct packet not_ready;
                    //packet
                    bytes = read(0, not_ready, sizeof(not_ready));
                    input_buf[bytes] = 0;

                    sendto(socket_fd, input_buf, strlen(input_buf), 0,
                           (struct sockaddr *) &client_addr, sizeof(client_addr));
                }
            }
        } else {
            printf(".");
            fflush(0);
        }
    }

    return 0;
}
