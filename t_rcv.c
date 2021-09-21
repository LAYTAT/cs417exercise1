//
// Created by Chanha Kim on 9/21/21.
//
#define SIZE 512
#include "net_include.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char * argv[]) {

    int sock_fd, new_sock;
    socklen_t addr_size;
    long int on = 1;
    struct sockaddr_in name, new_addr;
    struct hostent     h_ent, *p_h_ent;
    FILE * fp;
    int ret;
    char buf[SIZE];
    char dest_fname[SIZE];
    fd_set             mask;
    fd_set             read_mask, write_mask, excep_mask;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd<0) {
        perror("Net_server: socket");
        exit(1);
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        perror("Net_server: setsockopt error \n");
        exit(1);
    }

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);

    if ( bind( sock_fd, (struct sockaddr *)&name, sizeof(name) ) < 0 ) {
        perror("Net_server: bind");
        exit(1);
    }

    if (listen(sock_fd, 4) < 0) {
        perror("Net_server: listen");
        exit(1);
    }

    addr_size = sizeof(new_addr);
    new_sock = accept(sock_fd, (struct sockaddr *) &new_addr, &addr_size);



    ret = recv(new_sock, buf, SIZE, 0);
    if (ret < 0) {
        perror("receiving filename failed");
        exit(0);
    }
    stpcpy(dest_fname, buf);
    memset(buf,0,sizeof(buf));

    fp = fopen(dest_fname, "w");
    if (fp == NULL) {
        perror("Failed to open a file");
        exit(1);
    }

    for(;;){

        ret = recv(new_sock, buf, SIZE, 0);
        if (ret <= 0) {
            break;
        }
        fwrite(buf, 1, strlen(buf), fp);
        memset(buf,0,sizeof(buf));
    }


    return 0;


}
