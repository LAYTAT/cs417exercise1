//
// Created by JJ Lay on 9/14/21.
//
#include "net_include.h"

struct File_Data {
    unsigned char data[BUFSIZE];
};

int main(){
    struct sockaddr_in      serv_addr;            // storing own addr, use for binding
    struct sockaddr_in      client_addr;          // storing current client addr
    int                     status = 0;       // indicate current status: 0 for available, 1 for occupied
    struct sockaddr_in      from_addr;            // storing any incoming addr
    socklen_t               from_len;
    int                     socket_fd;
    fd_set                  mask;
    fd_set                  read_mask, write_mask, excep_mask;
    int                     bytes_recved;
    int                     num;
    char                    mess_buf[MAX_MESS_LEN];
    struct packet           packet_buf;
    struct timeval          timeout;
    int                     window_start;
    struct packet           reply_to_init_pckt;
    struct File_Data        filedata_buf[WINDOW_SIZE];
    int                     infile_idx = 0;

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
                bytes_recved = recvfrom( socket_fd, mess_buf, sizeof(mess_buf), 0,
                                  (struct sockaddr *)&from_addr,
                                  &from_len ); // receiving data and store it in the message buffer

                if(bytes_recved != -1) { // if received message, write it into a packet struct
                    memcpy(&packet_buf, &mess_buf, sizeof(packet_buf));
                }

                if ( packet_buf.type == 0 ) { //received packet is a init packet
                    if ( status == 0 ) {
                        client_addr = from_addr;
                        status = 1; //mark as occupied
                        reply_to_init_pckt.type = 5; // indicates acception to sender
                    } else {
                        reply_to_init_pckt.type = 4; // indicates rejection to sender
                    }
                    sendto(socket_fd, &reply_to_init_pckt, sizeof(reply_to_init_pckt), 0, (struct sockaddr *) &from_addr, sizeof(from_addr));
                }

                if ( packet_buf.type == 2 ) { // if it is a sender packet, which contains the file data
                    memcpy(filedata_buf);
                }

                // packet unserilazation
                // if initial packet recieved and server not busy
                mess_buf[bytes_recved] = 0;

                // check if not in middle of receiving
            }
        } else {
            printf("not receiving anything from anyone");
            fflush(0);
        }
    }

    return 0;
}
