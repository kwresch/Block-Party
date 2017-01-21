#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "file_headers.h"
#include "definitions.h"

int main(int argc, char *argv[]) {
    int pipes[2], sock, newsock, len, n, pid;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[256];

    // Start Listening

    pipe(pipes);

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Unable to fork\n");
        exit(1);
    }
    if (pid == 0) {
        // Child Process
        close(pipes[0]);
        start_stream(pipes[1]);
        // Shouldn't reach here
        exit(0);

    } else {
        // Parent Process
        close(pipes[1]);
    }

    // Socket Server

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        fprintf(stderr, "Unable to open socket\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Unable to bind socket\n");
        exit(1);
    }

    listen(sock, 8);
    len = sizeof(cli_addr);

    while (1) {
        if ((newsock = accept(sock, (struct sockaddr *) &cli_addr, &len)) < 0) {
            fprintf(stderr, "Unable to accept connection\n");
            exit(1);
        }

        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Unable to fork\n");
            exit(1);
        }

        if (pid == 0) {
            // Child Process
            close(sock);
            
            stream_audio(newsock, pipes[0], 2);
        } else {
            // Parent Process
            close(newsock);
        }
    }
}