#include "net_include.h"

#define WIND_SIZE 50
#define BUF_SIZE 30
//ncp send files in packets
//ncp is the client

int main(int argc, char* argv[]) {

  float lrp; //loss rate percentage
  char * source_fname; //source file name
  char * dest_fn; //destination file name
  char * comp_name; //computer name, i.e.ugrad1
  FILE * fr; /* Pointer to source file, which we read */
  char * buf[BUF_SIZE];
  int nread;

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

  //implement reading files and packaging them and sending 

  fclose(fr);


  return 0;


  
  
}
