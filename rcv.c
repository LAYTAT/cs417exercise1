//
// Created by JJ Lay on 9/14/21.
//
#include "net_include.h"
#include "arpa/inet.h" //for inet_ntop used for show debug info

#define TIMEOUT_FEEDBACK_IN_SECONDS 3
#define TIMEOUT_ENDPACKET_IN_SECONDS 3
#define NUMS_OF_END_TO_SEND 20
#define NORMAL_TIMEOUT 1

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

void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char * argv[]){
    struct sockaddr_in      serv_addr;                              // storing own addr, use for binding
    struct sockaddr_in      client_addr;                            // storing current client addr
    int                     status = 0;                             // indicate current status: 0 for available, 1 for occupied
    struct sockaddr_in      from_addr;                              // storing any incoming addr
    socklen_t               from_len;
    int                     socket_fd;
    fd_set                  mask;
    fd_set                  read_mask, write_mask, excep_mask;
    int                     bytes_recved;
    int                     num;
    char                    mess_buf[MAX_MESS_LEN];
    struct packet           packet_buf;
    packet_buf.type         = 0;                                    //indicates this packet is not filled with any info
    struct timeval          timeout;
    int                     window_start = 0;
    int                     window_slots[WINDOW_SIZE];
                            memset(window_slots, 0, WINDOW_SIZE);
    struct packet           reply_to_init_pckt;
    struct File_Data        window[WINDOW_SIZE];
    int                     received_counter = 0;                   // count util there should be a feedback
    struct packet           feedback;
                            feedback.type = 3;
    FILE *                  fPtr = NULL;                            //file pointer to write stuff into
    int                     ready_packet_flag = 0;
    int                     EVERY_100M_IN_NUMS_OF_PACKET = (104857600/sizeof(struct File_Data));
    struct timeval          com_start_timestmp;
    struct timeval          finished_timestmp;
    struct timeval          last_timestamp;
    int                     last_seq;
    int                     ret = -1;
    int                     idx_in_window;
    int                     feedback_timeout_flag = 0;
    struct timeval          feedback_timestamp;
    int                     final_seq_num = -1;

    //debug usage
    //char                    s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        printf("please just enter lost_rate,\n usage: rcv loss_rate_percent\n");
        exit(1);
    }

   /* add loss rate to send function */
    int lrp = atoi(argv[1]);
    if (lrp > MAX_LOSS_RATE_PERCENT ) {
        printf("%d is invalid loss rate, using %d now \n", lrp, MAX_LOSS_RATE_PERCENT);
        lrp = MAX_LOSS_RATE_PERCENT;
    }
    /* Call this once to initialize the coat routine */
    sendto_dbg_init(lrp);

    /* server socket for communicating with clients */
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

    /*server created!*/
    if ( bind( socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0 ) {
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
        timeout.tv_sec = NORMAL_TIMEOUT;
        timeout.tv_usec = 0;
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &timeout);
        if (num > 0) {
            if ( FD_ISSET( socket_fd, &read_mask) ) {
                from_len = sizeof(from_addr);
                /* receiving data and store it in the message buffer */
                bytes_recved = recvfrom( socket_fd, mess_buf, sizeof(mess_buf), 0,
                                  (struct sockaddr *)&from_addr,
                                  &from_len );

                /* for debug usage */
//                printf("rcv: got packet from %s \n",
//                       inet_ntop(from_addr.sin_family,
//                                 get_in_addr((struct sockaddr *)&from_addr),
//                                 s,
//                                 sizeof (s)
//                       ));

                /* if received message, write it into a packet struct  */
                if (bytes_recved != -1) {
                    memcpy(&packet_buf, &mess_buf, sizeof(packet_buf));
                } else {
                    perror("rcv: recvfrom error");
                    exit(1);
                }
                if (bytes_recved < sizeof(struct packet)) {
                    printf("packet is corrupt.\n");
                }

                if (packet_buf.type == 0) {
                    printf("packet is not successfully received.\n");
                }

                switch (packet_buf.type) {
                    /* received packet is a init packet */
                    case 1:{
                        if ( status == 0 ) {
                            client_addr = from_addr;
                            status = 1; //mark server as occupied
                            reply_to_init_pckt.type = 5; // indicates acceptation to sender

                            /* open file with dest file name */
                            int filename_size = packet_buf.size;
                            char destFilename[filename_size + 1];
                            memcpy(destFilename, &packet_buf.data, sizeof (char) * filename_size);
                            destFilename[filename_size] = 0;
                            fPtr = fopen(destFilename, "w"); // open the file with binary mode
                            if (fPtr == NULL) {
                                printf("file opening failed\n");
                            }

                            /* set timer for ready pakcet */
                            timeout.tv_sec = 20;
                            ready_packet_flag = 1;

                            /* this is where communication starts */
                            gettimeofday(&com_start_timestmp, NULL);

                        } else {
                            /* indicates rejection to sender */
                            reply_to_init_pckt.type = 4;
                            printf("rejection for receiving sent\n");
                        }
                        int bytes_send = sendto_dbg(socket_fd,
                                                    (const char *)  &reply_to_init_pckt,
                                                    sizeof(reply_to_init_pckt),
                                                    0,
                                                    (struct sockaddr *) &client_addr,
                                                    sizeof(client_addr));
                        if(bytes_send!=sizeof(reply_to_init_pckt))
                            printf("send reply failed\n");
                        break;
                    }

                    /* if it is a sender packet, which contains the file data */
                    case 2:{
                        /* stop timer for feedback */
                        feedback_timeout_flag = 0;

                        /*remove flag for ready pakcet*/
                        timeout.tv_sec = NORMAL_TIMEOUT;
                        ready_packet_flag = 0;

                        /* packet at a valid position in the current window */
                        idx_in_window = packet_buf.seq_num - window_start;

                        if ( packet_buf.seq_num >= window_start && idx_in_window < WINDOW_SIZE && window_slots[idx_in_window] == 0)
                        {
                            memcpy(&window[idx_in_window], &packet_buf.data, sizeof(struct File_Data));
                            window_slots[idx_in_window] = 1;
                        } else {
                            printf("packet already received or ahead of window\n");
                        }

                        /*         TODO: Both the sender (ncp) and the receiver (rcv) programs should report two statistics
                         *          every 100Mbytes of data sent/received IN ORDER (all the data from the beginning of
                         *          the file to that point was received with no gaps):
                         *          1) The total amount of data (in Mbytes) successfully transferred by that time.
                         *          2) The average transfer rate of the last 100Mbytes sent/received (in Mbits/sec).
                         *          */

                        if (window_start == 0) {
                            /* get start time */
                            gettimeofday(&last_timestamp, NULL);
                            last_seq = 0;
                        }
                        if (window_start != 0 && window_start % EVERY_100M_IN_NUMS_OF_PACKET == 0) {
                            struct timeval timestamp;
                            gettimeofday(&timestamp, NULL);
                            printf("The total amount of data (in Mbytes) successfully transferred is %0.2f\n The average transfer rate is %0.2f",
                                   ((float)window_start * sizeof (struct File_Data)) / (1024 * 1024 ),
                                   (((float)(window_start - last_seq)* sizeof (struct File_Data)) / (1024 * 1024 )) * 8  /  (timestamp.tv_sec - last_timestamp.tv_sec)
                            );
                            gettimeofday(&last_timestamp, NULL);
                            last_seq = window_start;
                        }

                        /* windwow sliding */
                        /* update cumu ack when receive a valid packet*/
                        int ackFromWindowStart = calAckFromWindowStart(window_slots, WINDOW_SIZE);
                        /* renew window slots */
                        int tmp = ackFromWindowStart - 1;
                        while(tmp >= 0) {
                            // window_slots[i] == 0 mean not received
                            window_slots[tmp] = 0;
                            tmp--;
                        }
                        /* write cumulated packets into a file */
                        //int start_idx_of_file_buf = window_start % WINDOW_SIZE;
                        window_start = window_start + ackFromWindowStart;

                        /* write cumulated datas data into file */
                        //fwrite(window , sizeof(struct File_Data), ackFromWindowStart , fPtr);
                        for ( int i = 0 ; i < ackFromWindowStart; ++i ){
                            //int idx_of_file_buf = (start_idx_of_file_buf + i) % WINDOW_SIZE;
                            //fwrite(&window[idx_of_file_buf] , sizeof(struct File_Data), 1 , fPtr);
                            fwrite(&window[i] , sizeof(struct File_Data), 1 , fPtr);
                        } // TODO: need better file writing

                        if (window_start == final_seq_num) {
                            //send confirmation feedback
                            memset(&feedback, -1, sizeof(feedback));/*clean feedback before writing infos on it*/
                            feedback.type = 7;
                            feedback.cumu_acks = final_seq_num;
                            for (int i = 0; i < NUMS_OF_END_TO_SEND; ++i){
                                /* send feedback to the sender */
                                if (sendto_dbg(socket_fd,
                                               (const char *) &feedback,
                                               sizeof(feedback),
                                               0,
                                               (struct sockaddr *) &client_addr,
                                               sizeof(client_addr)) < 0) {
                                    perror("sendto for feedback:");
                                }
                            }
                        }

                        /* send feedback */
                        received_counter ++;
                        /* keep track in order to know when to send feedback */
                        if (received_counter == WINDOW_SIZE / FEEDBACK_DEVIDER) {
                            received_counter =  0;

                            /* send feedback to the sender */
                            memset(&feedback, -1, sizeof (feedback));
                            // generate nack[WINDOW_SIZE]
                            for(int i = 0; i < idx_in_window; ++i) { //TODO
                                if(window_slots[i] != 1) { // window_slots[i] == 1 mean received
                                    feedback.nack[i] = window_start + i;
                                }
                            }
                            feedback.type = 3;
                            ackFromWindowStart = calAckFromWindowStart(window_slots, WINDOW_SIZE);
                            feedback.cumu_acks = window_start + ackFromWindowStart - 1;
                            ret = sendto_dbg(socket_fd,
                                             (const char *) &feedback,
                                             sizeof(feedback),
                                             0,
                                             (struct sockaddr *) &client_addr,
                                             sizeof(client_addr));
                            if (ret < 0)
                            {
                                perror("sendto for feedback:");
                            } else if (ret < sizeof(feedback)) {
                                printf("sendto in feedback: sent data is smaller than actual data");
                            }
                            /* start timer for feedback */
                            feedback_timeout_flag = 1;
                            gettimeofday(&feedback_timestamp, NULL);
                        }

                        break;
                    }

                    /* received last sender packetk */
                    case 6: {
                        final_seq_num = packet_buf.seq_num;
                        //* check current window if all file datas valid
                        int current_ack_seq_num = calAckFromWindowStart(window_slots, WINDOW_SIZE) + window_start;
                        /* packet at a valid position in the current window */
                        idx_in_window = packet_buf.seq_num - window_start;
                        if (packet_buf.seq_num >= window_start && idx_in_window < WINDOW_SIZE &&
                            window_slots[idx_in_window] == 0) {
                            memcpy(&window[idx_in_window], &packet_buf.data, sizeof(struct File_Data));
                            window_slots[idx_in_window] = 1;
                        } else {
                            printf("packet already received or ahead of window\n");
                        }

                        // not all before final packets are received
                        if (current_ack_seq_num != packet_buf.seq_num) {

                            //send feedback with nacks
                            memset(&feedback, -1, sizeof(feedback));/*clean feedback before writing infos on it*/
                            /* generate nack[WINDOW_SIZE] and put into feedback*/
                            for (int i = 0; i < idx_in_window; ++i) { //TODO
                                if (window_slots[i] != 1) {
                                    feedback.nack[i] = window_start + i;
                                }
                            }
                            feedback.type = 3;
                            feedback.cumu_acks = current_ack_seq_num;
                            /* send feedback to the sender */
                            if (sendto_dbg(socket_fd,
                                           (const char *) &feedback,
                                           sizeof(feedback),
                                           0,
                                           (struct sockaddr *) &client_addr,
                                           sizeof(client_addr)) < 0) {
                                perror("sendto for feedback:");
                            }
                            /* start timer for feedback */
                            feedback_timeout_flag = 1;
                            gettimeofday(&feedback_timestamp, NULL);
                        }
                            /* if all files received
                                * report statistics and write them all to file*/
                        else {

                            printf("The size of the file transferred is %.2f Bytes\n ",
                                   (float) current_ack_seq_num * sizeof(struct File_Data));
                            gettimeofday(&finished_timestmp, NULL);
                            printf("The amount of time required for the transfer is %d seconds\n ",
                                   (int) (finished_timestmp.tv_sec - com_start_timestmp.tv_sec));
                            printf("average rate from when the communication occurred (in Mbits/sec) is %.5f\n ",
                                   (float) current_ack_seq_num * sizeof(struct File_Data) * 8
                                   /
                                   ((float) (finished_timestmp.tv_sec - com_start_timestmp.tv_sec) * 1024 * 1024));

                            /*write final into file and close file*/
                            int ackFromWindowStart = calAckFromWindowStart(window_slots, WINDOW_SIZE);
                            //int start_idx_of_file_buf = window_start % WINDOW_SIZE;
                            for (int i = 0; i < ackFromWindowStart; ++i) {
                                //int idx_of_file_buf = (start_idx_of_file_buf + i) % WINDOW_SIZE;
                                //fwrite(&window[idx_of_file_buf], sizeof(struct File_Data), 1, fPtr);
                                fwrite(&window[i], packet_buf.size, 1, fPtr);
                            }
                            /* Close file to save file data */

                            fclose(fPtr);
                            status = 0; // make server available now
                            window_start = 0;

                            //send confirmation feedback
                            memset(&feedback, -1, sizeof(feedback));/*clean feedback before writing infos on it*/
                            feedback.type = 7;
                            feedback.cumu_acks = final_seq_num;
                            for (int i = 0; i < NUMS_OF_END_TO_SEND; ++i){
                                /* send feedback to the sender */
                                if (sendto_dbg(socket_fd,
                                               (const char *) &feedback,
                                               sizeof(feedback),
                                               0,
                                               (struct sockaddr *) &client_addr,
                                               sizeof(client_addr)) < 0) {
                                    perror("sendto for feedback:");
                                }
                            }
                        }
                        break;
                    }
                    default:
                        printf("unknown type of packet received.\n");
                }
            }
        } else {
            /*if timeout for readypacket */
            if (ready_packet_flag) {
                /*  resend the ready packet */
                sendto_dbg(socket_fd,
                           (const char *) &reply_to_init_pckt,
                           sizeof(reply_to_init_pckt),
                           0,
                           (struct sockaddr *) &client_addr,
                           sizeof(client_addr));

                printf("timeout for ready packet... resending ready packet\n");
            }

            printf("not receiving anything from anyone...\n");
            fflush(0);
        }

        if(feedback_timeout_flag){
            struct timeval tmp;
            gettimeofday(&tmp, NULL);
            if (tmp.tv_sec - feedback_timestamp.tv_sec >= TIMEOUT_FEEDBACK_IN_SECONDS){
                /* resend feedback to the sender */
                if (sendto_dbg(socket_fd,
                               (const char *) &feedback,
                               sizeof(feedback),
                               0,
                               (struct sockaddr *) &client_addr,
                               sizeof(client_addr)) < 0) {
                    perror("sendto for feedback:");
                }
            }
        }
    }
    return 0;
}
