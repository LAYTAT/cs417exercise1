#include "net_include.h"


#define WIND_SIZE 50
#define BUF_SIZE 300
//ncp send files in packets
//ncp is the client
int gethostname(char*,size_t);

int main(int argc, char* argv[]) {

  float lrp; //loss rate percentage
  char * source_fname; //source file name
  char * dest_fn; //destination file name
  char * comp_name; //computer name, i.e.ugrad1 of the HOST (i.e. host_name)
  FILE * fr; /* Pointer to source file, which we read */
  char * buf[BUF_SIZE];
  int nread;
  int ss;  /*socket for sending*/
  struct sockaddr_in serv_addr; /*server address info*/
  struct hostent        h_ent;
  struct hostent        *p_h_ent;
  int host_num;



  //check command line args
  if (argc != 4) {
    perrer("invalid command line argument.\n");
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


  












  //implement reading files and packaging them and sending 

  fclose(fr);


  return 0;


  
  
}
