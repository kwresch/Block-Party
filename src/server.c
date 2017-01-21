#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "file_headers.h"

#define PORT 40001

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, clilen, n;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[256];
    FILE *fp;
    int fsize, bread, kbps, bps, bpms;
    char *bytes;


}