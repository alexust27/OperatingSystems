#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

void makeNonBlocking(int FD) {
    int flag = fcntl(FD, F_GETFL);
    if (flag < 0) {
        error("fcntl");
    }
    flag |= O_NONBLOCK;
    if (fcntl(FD, F_SETFL, flag) < 0) {
        error("fcntl");
    }
}

int main(int argc, char *argv[]) {

    const int buf_size = 256;
    int serverSocket, newSocket, clilen;
    char buffer[buf_size];
    struct sockaddr_in server_address, client_address;
    int n;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no address provided\n");
        exit(1);
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        error("ERROR in opening socket");
    }

    bzero((char *) &server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = 4444;

    if (bind(serverSocket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        error("ERROR on binding");
    }

    int list = listen(serverSocket, 1);
//    listen(serverSocket, 5);
    if (list == -1) {
        error("listen");
    }

    int epollFD = epoll_create(1);
    if (epollFD == -1) {
        error("epoll_create");
    }
    int MAX = 100;
    struct epoll_event ev, events[MAX];
    ev.data.fd = serverSocket;
    ev.events = EPOLLIN;

    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, serverSocket, &ev) < 0) {
        error("epoll_ctl");
    }

    while (1) {
        int cntEvents = epoll_wait(epollFD, events, MAX, -1);
        if (cntEvents < 0) {
            error("epoll_wait");
        }

        for (int i = 0; i < cntEvents; i++) {
            if (events[i].data.fd == serverSocket) {
                clilen = sizeof(client_address);
                newSocket = accept(serverSocket, (struct sockaddr *) &client_address, &clilen);
                if (newSocket < 0) {
                    error("accept");
                }

                makeNonBlocking(newSocket);
                ev.data.fd = newSocket;
                ev.events = EPOLLIN | EPOLLET;

                if (epoll_ctl(epollFD, EPOLL_CTL_ADD, newSocket, &ev) < 0) {
                    error("epool_ctl");
                }
            } else {
                bzero(buffer, buf_size);
                n = read(events[i].data.fd, buffer, buf_size - 1);
                if (strcmp(buffer, "exit\n") == 0) {
                    perror("сlient disconnected");
                    close(newSocket);
                    continue;
                }
                if (n < 0) {
                    close(newSocket);
                    perror("ERROR reading from socket");
                    continue;
                }
                printf("Here is the message: %s", buffer);
                n = write(events[i].data.fd, "I got your message", 18);
                if (n < 0) {
                    if (epoll_ctl(epollFD, EPOLL_CTL_DEL, events[i].data.fd, &ev) < 0) {
                        error("epoll_ctl");
                    }
//                    close(newSocket);
                    perror("ERROR writing to socket");
                    continue;
                }
            }
        }
    }
}
