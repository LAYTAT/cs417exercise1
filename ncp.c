#include "net_include.h"

#define WIND_SIZE 50

//ncp send files in packets

int main(int argc, char* argv[]) {
  //check command line args
  if (argc != 4) {
    perrer("invalid command line argument.\n");
    exit(1);
  }
  
  //loss rate percentage: lrp
  float lrp;
  lrp = atof(argv[1]);

  //source_fname
  char * source_fname = argv[2];

  //dest_fname and comp_name
  char dest_fn[strlen(argv[3])-6];
  char comp_name[7];
  int div = strlen(argv[3]) - 6;
  for (int i = 0; i < div; i++) {
    dest_fn[i] = argv[3][i];
  }
  for (int i = div; i < strlen(argv[3]); i++) {
    comp_name[i-div] = argv[3][i];
  }
  dest_fn[strlen(argv[3])-7] = 0;
  comp_name[6] = 0;
  )

  

  

  

  


  



  
  
}
