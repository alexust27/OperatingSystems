#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>

const int buf_size = 256;

void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int clientSocket, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[bu];

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

    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);

    if (connect(clientSocket, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }
    printf("Please enter the city (use English): ");
    while (1) {
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        n = write(clientSocket, buffer, strlen(buffer));
        if (n < 0) {
            error("ERROR writing to socket");
        }
        bzero(buffer, 256);
        n = read(clientSocket, buffer, 255);
        if (n < 0) {
            error("ERROR reading from socket");
        }
        if (strcmp("win", buffer) == 0) {
            printf("You win\n");
            break;
        }
        if (buffer[0] == '!') {
            printf("Please try again :");
            continue;
        }
        printf("%s\nWrite your city: ", buffer);
    }

    close(clientSocket);
    return 0;
}