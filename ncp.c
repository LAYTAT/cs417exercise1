#include "net_include.h"

#define WIND_SIZE 10
#define BUF_SIZE 300
//ncp send files in packets
//ncp is the client
int gethostname(char*,size_t);
char * seq_to_addr(int sequence_number, int window_number, char ** window);


int main(int argc, char* argv[]) {

  float lrp; //loss rate percentage
  char * source_fname; //source file name
  char * dest_fn; //destination file name
  char * comp_name; //computer name, i.e.ugrad1 of the HOST (i.e. host_name)
  FILE * fr; /* Pointer to source file, which we read */
  char * buf[BUF_SIZE+1];
  int nread;
  int ss;  /*socket for sending*/
  struct sockaddr_in serv_addr; /*server address info*/
  struct hostent h_ent;
  struct hostent *p_h_ent;
  int host_num;
  int seq_num;
  int wind_num = 0;
  char ** window;
  int server_flag= 0; /* 1: server ready, 0: server not ready */
  fd_set mask;
  fd_set read_mask, write_mask, excep_mask;
  struct timeval timeout;
  struct packet Initial_Packet;
  struct packet Recieved_Packet;
  int NORMAL_TIMEOUT = 10;
  int n;
  int serv_len;
  int num;

  //check command line args
  if (argc != 4) {
    perror("invalid command line argument.\n");
    exit(1);
  }

  lrp = atof(argv[1]);
  source_fname = argv[2];

  dest_fn[strlen(argv[3])-6];
  comp_name[7];
  int div = strlen(argv[3]) - 6;
  for (int i = 0; i < div; i++) {
    dest_fn[i] = argv[3][i];
  }
  for (int i = div; i < strlen(argv[3]); i++) {
    comp_name[i-div] = argv[3][i];
  }
  dest_fn[strlen(argv[3])-7] = 0;
  comp_name[6] = 0;

  //open source file and parse so it can send
  /* Open the source file for reading */
  if((fr = fopen(source_fname, "r")) == NULL) {
    perror("fopen");
    exit(0);
  }
  
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
  Initial_Packet.nack[WINDOW_SIZE] = 0;
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
        n = recvfrom(ss, &Recieved_Packet, sizeof(Recieved_Packet), 0, (struct sockaddr *) &serv_addr, serv_len);
        if (Recieved_Packet.type == 4) { //rejecte
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
    window_data[i] = (char *) malloc(BUF_SIZE+1);  //buf_size + 1 for the null character
  }

  //initialize window buffer with data pointers; (first window numbered data)
  for (int i = 0; i < WINDOW_SIZE; i++) {
    nread = fread(buf, 1, BUF_SIZE, fr);
    buf[nread] = 0; //add null character
    memcpy(window_data[i], buf, strlen(buf)+1);
    memset(buf, 0, sizeof(buf)); 
  }


  //send the file data
  while (server_flag == 1) { //server is ready to recieve
    read_mask = mask;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;

    memset(&Recieved_Packet, 0, sizeof(Recieved_Packet));

    num = select(FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &timeout);
    if (num > 0) {
      if (FD_ISSET(ss, &read_mask)) {
        serv_len = sizeof(serv_addr);
        n = recvfrom(ss, &Recieved_Packet, sizeof(Recieved_Packet), 0, (struct sockaddr *) &serv_addr, serv_len);
        if (Recieved_Packet.type == 7) { //last packet recieved by the server
          break;
        } else if (Recieved_Packet.type = 3) { //feedback packet recieved
          
          
        }


      }
      n = 0;
    }


  }


  free(window);



  fclose(fr);


  return 0;
}

//window stores window_sized addresses of corresponding window_sized seqeunce numbers 
//sequence number is the overall location of the data within the file
//wind_num denotes how far the window has slided
char * seq_to_addr(int seq_num, int wind_num, char**window) {
  if (seq_num - wind_num >= 50) {
    fprintf(1, "invalid sequence number.\n");
    return NULL;
  }
  return window[seq_num % WIND_SIZE];
}

