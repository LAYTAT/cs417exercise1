#include "net_include.h"

#define BUF 50
//ncp send files in packets
//ncp is the client
int gethostname(char*,size_t);

int main(int argc, char* argv[]) {

  float lrp; //loss rate percentage
  char source_fname[BUF]; //source file name
  char dest_fn[BUF]; //destination file name
  char comp_name[BUF]; //computer name, i.e.ugrad1 of the HOST (i.e. host_name)
  FILE * fr; /* Pointer to source file, which we read */
  char buf[BUFSIZE + 1];
  int nread;
  int ss;  /*socket for sending*/
  struct sockaddr_in serv_addr; /*server address info*/
  struct hostent h_ent;
  struct hostent *p_h_ent;
  int host_num;
  int seq_num;
  int wind_num = 0; //keeps track of the start of the window index
  int server_flag= 0; /* 1: server ready, 0: server not ready */
  fd_set mask;
  fd_set read_mask, write_mask, excep_mask;
  struct timeval timeout;
  struct packet Initial_Packet;
  struct packet Recieved_Packet;
  struct packet Send_Packet;
  int NORMAL_TIMEOUT = 10;
  int n, m;
  int serv_len;
  int num;
  int ack = -1;
  int total_packets;
  int ret = 0;

  //check command line args
  if (argc != 4) {
    perror("invalid command line argument.\n");
    exit(1);
  }

  lrp = atof(argv[1]);
  strcpy(source_fname, argv[2]);

  int div = strlen(argv[3]) - 6;
  for (int i = 0; i < div; i++) {
    dest_fn[i] = argv[3][i];
  }
  for (int i = div; i < strlen(argv[3]); i++) {
    comp_name[i-div] = argv[3][i];
  }
  dest_fn[strlen(argv[3])-7] = 0;
  //comp_name[6] = 0;
  memcpy(comp_name, "localhost", 9); //TODO: to be deleted
  comp_name[9] = 0;

    //open source file and parse so it can send
  /* Open the source file for reading */
  if((fr = fopen(source_fname, "r")) == NULL) {
    perror("fopen");
    exit(0);
  }

  fseek(fr, 0, SEEK_END);
  total_packets = ftell(fr) / sizeof(struct File_Data) + 1;
  rewind(fr);

  
  //implement socket as a client
  ss = socket(AF_INET, SOCK_DGRAM, 0); /* socket for sending (udp) */
  if (ss<0) {
    perror("Ucast: socket");
    exit(1);
  }
  p_h_ent = gethostbyname(comp_name);
  if ( p_h_ent == NULL ) {
    printf("Ucast: gethostbyname error.\n");
    exit(1);
  }
  memcpy( &h_ent, p_h_ent, sizeof(h_ent));
  memcpy( &host_num, h_ent.h_addr_list[0], sizeof(host_num) );

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = host_num; 
  serv_addr.sin_port = htons(PORT);
  

  FD_ZERO( &mask );
  FD_ZERO( &write_mask );
  FD_ZERO( &excep_mask );
  FD_SET( ss, &mask );

  //make initial packet
  Initial_Packet.type = 1;
  Initial_Packet.seq_num = -1;
  Initial_Packet.size = strlen(dest_fn); //length of file name
  Initial_Packet.cumu_acks = -1;
  memset(Initial_Packet.nack, 0, WINDOW_SIZE * sizeof(int));
  struct File_Data fn;
  memcpy(fn.data, dest_fn, strlen(dest_fn));
  Initial_Packet.data = fn;

  //keep sending the initial packet until it recieves a feedback
  for(;;) {

    read_mask = mask;

    timeout.tv_sec = NORMAL_TIMEOUT;
    timeout.tv_usec = 0;
    sendto(ss, &Initial_Packet, sizeof(Initial_Packet), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    num = select(FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &timeout);

    if (num > 0) {
      if (FD_ISSET(ss, &read_mask)) {
        serv_len = sizeof(serv_addr);
        n = recvfrom(ss, &Recieved_Packet, sizeof(Recieved_Packet), 0, (struct sockaddr *) &serv_addr, &serv_len);
        if (n == -1 )
            perror("recvfrom in initial process:");

        if (Recieved_Packet.type == 4) { //reject
          server_flag = 0;
          break;
        } else if (Recieved_Packet.type == 5) {//server ready
          server_flag = 1;
          break;
        }
      }
    } else { //timeout
      continue;
    }
  }

  /*Allocate enough memory for the buffer that contains the char pointers and the corresponding arrray*/
  char ** window_data = (char **) malloc(sizeof(char *) * WINDOW_SIZE); //array of char pointers to the specific string
  for (int i = 0; i < WINDOW_SIZE; i++) {
    window_data[i] = (char *) malloc(BUFSIZE + 1);  //buf_size + 1 for the null character
  }

  //initialize window buffer with data pointers; (first window numbered data); 
  for (int i = 0; i < WINDOW_SIZE; i++) {
    nread = fread(buf, 1, BUFSIZE, fr);
    buf[nread] = 0; //add null character
    if (nread == 0) {
        break;
    }
    memcpy(window_data[i], buf, strlen(buf)+1);
    memset(buf, 0, sizeof(buf)); 
    nread = 0;
  }

  struct File_Data data_buf;

  //send initialized to the server (first window-sized packets)
  if (server_flag == 1) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
      memset(&data_buf, 0, sizeof(data_buf));
      memset(&Send_Packet, 0, sizeof(Send_Packet));

      if (i == total_packets -1) {
        Send_Packet.type = 6;
      } else {
        Send_Packet.type = 2;
      }

      Send_Packet.size = strlen(window_data[i]);
      Send_Packet.seq_num = i;
      memcpy(data_buf.data, window_data[i], strlen(window_data[i]));
      Send_Packet.data = data_buf;
      ret = sendto(ss, &Send_Packet, sizeof(Send_Packet), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
      if (ret == -1) {
          perror("sendto for initialization: ");
      }

      if (i == total_packets - 1) {
          break;
      }
    }
  }


  //send the file data
  while (server_flag == 1) { //server is ready to recieve
    read_mask = mask;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    memset(&Recieved_Packet, 0, sizeof(Recieved_Packet));

    num = select(FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &timeout);
    if (num > 0) {
      if (FD_ISSET(ss, &read_mask)) {
        serv_len = sizeof(serv_addr);
        n = recvfrom(ss, &Recieved_Packet, sizeof(Recieved_Packet), 0, (struct sockaddr *) &serv_addr, &serv_len);
        if (n == -1) {
            perror("recvfrom for feedback: ");
        }

        if (Recieved_Packet.type == 7) { //last packet recieved by the server
          printf("Last Packet Delivered Succesfully.\n");
          break;
        } else if (Recieved_Packet.type == 3) { //feedback packet recieved
          /*respond to nacks + slide/update window + send packets*/

          //respond to nacks
          for (int i = 0; i < (int)(sizeof(Recieved_Packet.nack)/sizeof(Recieved_Packet.nack[0])); i++) { 
            if(Recieved_Packet.nack[i] != - 1) {
                memset(&data_buf, 0, sizeof(data_buf));
                memset(&Send_Packet, 0, sizeof(Send_Packet));
                seq_num = Recieved_Packet.nack[i];
                m = seq_num % WINDOW_SIZE; //index in the window

                Send_Packet.type = 2;
                Send_Packet.size = strlen(window_data[m]);
                Send_Packet.seq_num = m;
                memcpy(data_buf.data, window_data[m], sizeof(window_data));
                Send_Packet.data = data_buf;
                ret = sendto(ss, &Send_Packet, sizeof(Send_Packet), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

                if (ret == -1) {
                    perror("sendto for feedback: ");
                }
            }
          }

          // slide/update window + send those packets
          ack = Recieved_Packet.cumu_acks; //sequence number of the cum_acks, i.e. ack % 50, is the end location of window update

          int begin_i = wind_num % WINDOW_SIZE; //beginning index to modify window
          //int end_i = ack % WINDOW_SIZE; //last index to modify window
          int x = ack - wind_num + 1; //number of modifications have to be made
          int i = begin_i; //index of current location of modification
          while (x != 0) {
            if (i == WINDOW_SIZE) {
              i = i % WINDOW_SIZE;
            }
            memset(buf, 0, sizeof(buf)); 
            memset(window_data[i], 0, sizeof(window_data[i]));
            nread = fread(buf, 1, BUFSIZE, fr);
            buf[nread] = 0; //add null character
            memcpy(window_data[i], buf, strlen(buf)+1);

            memset(&data_buf, 0, sizeof(data_buf));
            memset(&Send_Packet, 0, sizeof(Send_Packet));
            seq_num = i + WINDOW_SIZE * (wind_num/WINDOW_SIZE + 1);

            if (seq_num == total_packets) {
              Send_Packet.type = 6;
            } else {
              Send_Packet.type = 2;
            }

            Send_Packet.size = strlen(window_data[i]);
            Send_Packet.seq_num = seq_num;
            memcpy(data_buf.data, window_data[i], sizeof(window_data));
            Send_Packet.data = data_buf;
            ret = sendto(ss, &Send_Packet, sizeof(Send_Packet), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
            if (ret == -1) {
                perror("sendto: ");
            }
            wind_num++;
            x--;
            i++;


            if (seq_num == total_packets) {
              continue;
            }

          }
        } else {
          printf("recieved unexpected packet type.\n");
          break;
        }
      } else { //timeout
        printf(".");
        fflush(0);
      }
      
    }

  }

  for (int i =0; i < WINDOW_SIZE; i++) {
    free(window_data[i]);
  }

  fclose(fr);

  return 0;
}

