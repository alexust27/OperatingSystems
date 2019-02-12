#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

const int MAX = 100, buf_size = 256;

void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int clientSocket, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[buf_size];


    if (argc == 0) {
        fprintf(stderr, "please type the address\n");
        exit(0);
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);


    if (clientSocket < 0) {
        error("ERROR with openning socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "no such host\n");
        exit(0);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = 4444;

    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);

    if (connect(clientSocket, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    int epollFD = epoll_create(1);
    if (epollFD < 0) {
        error("epoll_create");
    }


    struct epoll_event ev, events[MAX];
    ev.events = EPOLLOUT;
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
        error("epoll_ctl");
    }

    ev.data.fd = clientSocket;
    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientSocket, &ev) < 0) {
        error("epoll_ctl");
    }

    while (1) {

        int cntEvents = epoll_wait(epollFD, events, MAX, -1);
        if (cntEvents < 0) {
            error("epoll_wait");
        }

        for (int i = 0; i < cntEvents; ++i) {
            if (events[i].data.fd == STDIN_FILENO) {
                size_t len_line;
                char *line;
//                sleep(1);
                printf("Please enter the message: ");
                getline(&line, &len_line, stdin);
                bcopy(line, buffer, len_line);

                n = (int) write(clientSocket, buffer, strlen(buffer));
                if (strcmp(buffer, "exit\n") == 0) {
                    close(clientSocket);
                    return 0;
                }
                if (n < 0) {
                    error("ERROR writing to socket");
                }
            }
            else {
                bzero(buffer, buf_size);

                n = (int) read(clientSocket, buffer, buf_size - 1);
                if (n < 0) {
                    error("ERROR reading from socket");
                }

                printf("%s\n", buffer);
            }
        }
    }
}
