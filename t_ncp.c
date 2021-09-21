//
// t_ncp, THE CLIENT TCP
//
#define SIZE 512
#include "net_include.h"
#include "stdlib.h"
#include "string.h"

void file_send(FILE * fp, int sockfd, char * file_name) {
    int x;
    int n;
    char data[SIZE];
    x = send(sockfd, file_name, strlen(file_name)+1, 0); //send file name
    if ( x<0 ) {
        perror("..");
    }
    memset(data, 0, sizeof(data));
    n = fread(data, 1, SIZE, fp);
    while(n > 0) { // while fp doesn't reach the EOF...
        data[n] = 0;
        x = send(sockfd, data, strlen(data), 0);
        if (x == -1) {
            perror("Sending File Failed");
            exit(1);
        }
        memset(data, 0, sizeof(data));
        n = fread(data, 1, SIZE, fp);
    }
}

int main (int argc, char * argv[]) {

    int lrp = 0;
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent     h_ent, *p_h_ent;
    FILE * fp;
    int ret;
    char source_fname[SIZE];
    char dest_fname[SIZE];
    char comp_name[SIZE];


    //check command line args
    if (argc != 4) {
        perror("invalid command line argument.\n");
        exit(1);
    }

    lrp = atof(argv[1]);
    strcpy(source_fname, argv[2]);

    int div = strlen(argv[3]) - 6;
    for (int i = 0; i < div; i++) {
        dest_fname[i] = argv[3][i];
    }
    for (int i = div; i < strlen(argv[3]); i++) {
        comp_name[i - div] = argv[3][i];
    }
    dest_fname[strlen(argv[3]) - 7] = 0;
    //comp_name[6] = 0;
    memcpy(comp_name, "localhost", 9); //TODO: to be deleted
    comp_name[9] = 0;

    //argument parsing finished
    //lets make a socket

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error making a socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    p_h_ent = gethostbyname(comp_name);
    if ( p_h_ent == NULL ) {
        printf("t_ncp: gethostbyname error.\n");
        exit(1);
    }
    memcpy( &h_ent, p_h_ent, sizeof(h_ent) );
    memcpy( &server_addr.sin_addr, h_ent.h_addr_list[0],  sizeof(server_addr.sin_addr) );

    //connect to the server
    ret = connect(sockfd, (struct sockddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("Failed to connect to server");
        exit(1);
    }

    fp = fopen(source_fname, "r");
    if (fp == NULL) {
        perror("Failed to open a file");
        exit(1);
    }

    file_send(fp, sockfd, dest_fname);
    printf("Sent the file!\n");
    fclose(fp);

    return 0;
}