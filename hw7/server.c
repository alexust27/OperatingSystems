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
#include <sys/un.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

#define num_clients 10000
int clients_fd[num_clients];

void send_fd(int socket, int fd)  // send fd by socket
{
    struct msghdr msg = {0};
    char buf[CMSG_SPACE(sizeof(fd))];
    memset(buf, '\0', sizeof(buf));
    char iobuffer[1];
    struct iovec io = {.iov_base = iobuffer, .iov_len = 1};

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));


    *((int *) CMSG_DATA(cmsg)) = fd;

    if (sendmsg(socket, &msg, 0) < 0)
        error("Failed to send message\n");
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
    int serverSocket, newSocket = 0, clilen;
    char buffer[buf_size];
    struct sockaddr_un server_address, client_address;
    int n;
    for (int i = 0; i < num_clients; ++i) {
        clients_fd[i] = -1;
    }

    serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        error("ERROR in opening socket");
    }

    bzero((char *) &server_address, sizeof(server_address));

    server_address.sun_family = AF_UNIX;
    const char *path = "sock.soc";

    unlink(path);
    memcpy(server_address.sun_path, path, strlen(path));
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
            } else if (events[i].events == (EPOLLIN)) {
                bzero(buffer, buf_size);
                n = read(events[i].data.fd, buffer, buf_size - 1);
                if (n < 0) {
                    perror("ERROR reading from socket");
                    close(events[i].data.fd);
                    continue;
//                    break;
                }
                if (strcmp(buffer, "exit\n") == 0) {
                    perror("сlient disconnected");
                    close(newSocket);
                    continue;
                }
                if (strcmp(buffer, "close") == 0) {
                    printf("close file");
                    close(clients_fd[events[i].data.fd]);
                    close(events[i].data.fd);
                    continue;
                }

                int fd = open(buffer, O_CREAT | O_CREAT | O_WRONLY | O_RDONLY,
                              S_IRWXU | S_IRWXG | S_IRWXO);
                if (fd < 0) {
                    perror("can't open file");
                    if (write(events[i].data.fd, "can't open file", 15) < 0) {
                        perror("writing to socket");
//                        close(events[i].data.fd);
                    }
                    continue;
                }

                printf("Client_%d open file %s\n", events[i].data.fd, buffer);


                send_fd(events[i].data.fd, fd);
//                        sleep(10);

                clients_fd[events[i].data.fd] = fd;

            } else {
                if (clients_fd[events[i].data.fd] != -1) {
                    close(clients_fd[events[i].data.fd]);
                    printf("close %d", events[i].data.fd);
                    clients_fd[events[i].data.fd] = -1;
                }
                close(events[i].data.fd);
            }
        }
    }
}
