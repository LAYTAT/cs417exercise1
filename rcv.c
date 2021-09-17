//
// Created by JJ Lay on 9/14/21.
//
#include "net_include.h"

int calAckFromWindowStart(const int * begin, int size) {
    int count = 0;
    for (int i = 0; i < size ; ++i) {
        if (begin[i] == 1) {
            count++;
        } else {
            return count;
        }
    }
    return count;
}

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
    packet_buf.type         = 0; //indicates this packet is not filled with any info
    struct timeval          timeout;
    int                     window_start = 0;
    int                     window_slots[WINDOW_SIZE];
    struct packet           reply_to_init_pckt;
    struct File_Data        filedataBuf[WINDOW_SIZE];
    int                     received_counter = 0; // count util there should be a feedback
    struct packet           feedback;
    feedback.type           = 3;

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
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &timeout);
        if (num > 0) {
            if ( FD_ISSET( socket_fd, &read_mask) ) {
                from_len = sizeof(from_addr);
                bytes_recved = recvfrom( socket_fd, mess_buf, sizeof(mess_buf), 0,
                                  (struct sockaddr *)&from_addr,
                                  &from_len ); // receiving data and store it in the message buffer

                if (bytes_recved != -1) { // if received message, write it into a packet struct
                    memcpy(&packet_buf, &mess_buf, sizeof(packet_buf));
                }

                if (packet_buf.type == 0) {
                    printf("packet is not successfully received.");
                }

                switch (packet_buf.type) {

                    //received packet is a init packet
                    case 1:
                        if ( status == 0 ) {
                            client_addr = from_addr;
                            status = 1; //mark as occupied
                            reply_to_init_pckt.type = 5; // indicates acceptation to sender
                        } else {
                            reply_to_init_pckt.type = 4; // indicates rejection to sender
                        }
                        sendto(socket_fd, &reply_to_init_pckt, sizeof(reply_to_init_pckt), 0, (struct sockaddr *) &from_addr, sizeof(from_addr));
                        break;
                        // TODO: add timeout for read packet
                        // if it is a sender packet, which contains the file data
                    case 2:
                        // packet at a valid position in the current window
                        if ( packet_buf.seq_num >= window_start && packet_buf.seq_num - window_start < WINDOW_SIZE )
                        {
                            int idx_in_window = packet_buf.seq_num - window_start;
                            window_slots[idx_in_window] = 1;
                            memcpy(&filedataBuf[packet_buf.seq_num - window_start], &packet_buf.data, sizeof(struct File_Data));
                            received_counter ++;

                            // time to send feedback
                            if (received_counter == window_start - 1) {
                                received_counter = 0;
                                memset(&feedback, 0, sizeof (feedback));

                                int ackFromWindowStart = calAckFromWindowStart(window_slots, WINDOW_SIZE);
                                feedback.cumu_acks = window_start + ackFromWindowStart;

                                // generate nack[WINDOW_SIZE]
                                for(int i = 0; i < WINDOW_SIZE; ++i) {
                                    if(window_slots[i] != 1) {
                                        feedback.nack[i] = window_start + i;
                                    }
                                }

                                //TODO: write cumulated packets into a file
                                window_start = window_start + ackFromWindowStart;

                                // send feedback to the sender
                                sendto(socket_fd, &feedback, sizeof(feedback), 0, (struct sockaddr *) &client_addr, sizeof(client_addr));
                            }
                        }
                        break;

                    // received last sender packetk
                    case 6:
                            //TODO: write final into file and close file
                    default:
                        printf("unknown type of packet received.");
                }

            }
        } else {
            printf("not receiving anything from anyone");
            fflush(0);
        }
    }

    return 0;
}
