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
#include <sys/un.h>

const int MAX = 100, buf_size = 256;

void error(char *msg) {
    perror(msg);
    exit(0);
}

int receive_fd(int socket)  // receive fd from socket
{
    struct msghdr msg = {0};
    char c_buffer[256];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    if (recvmsg(socket, &msg, 0) < 0)
        error("Failed to receive message\n");

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    unsigned char *data = CMSG_DATA(cmsg);
    perror("About to extract fd");

    int fd = *((int *) data);
    printf("Extracted fd %d\n", fd);

    return fd;
}


int main(int argc, char *argv[]) {
    int clientSocket, n;
    struct sockaddr_un serv_addr;
//    struct hostent *server;
    char buffer[256];


    if (argc == 0) {
        fprintf(stderr, "please type the address\n");
        exit(0);
    }

    clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);


    if (clientSocket < 0) {
        error("ERROR with openning socket");
    }


    serv_addr.sun_family = AF_UNIX;

    const char *path = "sock.soc";
//    bzero((char *) &serv_addr, sizeof(serv_addr));
    memcpy(serv_addr.sun_path, path, strlen(path));
    if (connect(clientSocket, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    int epollFD = epoll_create(1);
    if (epollFD < 0) {
        error("epoll_create");
    }


    struct epoll_event ev, events[MAX];
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
        error("epoll_ctl");
    }

    ev.data.fd = clientSocket;
    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientSocket, &ev) < 0) {
        error("epoll_ctl");
    }


    int fd = -1;
    printf("Please enter the name of file: \n");
    while (1) {

        int cntEvents = epoll_wait(epollFD, events, MAX, -1);
        if (cntEvents < 0) {
            error("epoll_wait");
        }

        for (int i = 0; i < cntEvents; ++i) {
            if (events[i].data.fd == STDIN_FILENO) {
                int len_line;
                len_line = (int) read(STDIN_FILENO, buffer, buf_size);
                buffer[len_line - 1] = '\0';
//                printf(buffer);

                n = (int) write(clientSocket, buffer, strlen(buffer));
                if (n < 0) {
                    error("ERROR writing to socket");
                }


            } else if (events[i].data.fd == clientSocket) {
                fd = receive_fd(clientSocket);
                if (fd < 0) {
                    error("can't get file from server");
                } else {
                    printf("get fd\n");
                }

                if (write(fd, "hello\n", 5) < 0) {
                    error("writing in file");
                }

                n = (int) write(clientSocket, "close", 5);
                if (n < 0) {
                    error("write to socket");
                }
                return 0;
            } else {
                return 0;
            }
        }
    }
}
