#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>


#define numOfCity 57
#define sizeBuf 256

char buffer[sizeBuf];

char *listOfCities[numOfCity] = {
        "Moscow", "Saint Petersburg", "Astrakhan", "Novosibirsk", "Saransk",
        "Novgorod", "Volgograd", "Yekaterinburg", "Pskov", "Vologda",
        "Krasnoyarsk", "Krasnodar", "Izhevsk", "Ufa", "Kazan", "Chelyabinsk",
        "Omsk", "Perm", "Samara", "Vladimir", "Warsaw", "Paris", "London",
        "Kirov", "Vilnius", "Tokyo", "Seoul", "Mexico", "New York", "Jakarta",
        "Helsinki", "Sydney", "Detroit", "Dubai", "Baku", "Cairo", "Dublin",
        "Frankfurt", "Khabarovsk", "Gibraltar", "Havana", "Islamabad", "Lima",
        "Ankara", "Caracas", "Madrid", "New Delhi", "Oslo", "Prague", "Quito",
        "Riga", "Rome", "Santiago", "Tbilisi", "Wellington", "Zagreb", "Yerevan"
};

void error(char *msg) {
    perror(msg);
    exit(1);
}

int checkStr(char str[]) {
    for (int i = 0; i < strlen(str); ++i) {
        if (!isalpha(str[i]) && str[i] != ' ' && str[i] != '\n')
            return 0;
    }
    return 1;
}


int main(int argc, char *argv[]) {

    char clientCities[numOfCity][sizeBuf];
    int idCity = 0;
    char used[numOfCity];
    for (int i = 0; i < numOfCity; ++i) {
        used[i] = 0;
    }

    int serverSocket, newSocket, clilen;
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
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (1) {
        clilen = sizeof(client_address);

        newSocket = accept(serverSocket, (struct sockaddr *) &client_address, &clilen);

        if (newSocket < 0) {
            error("ERROR on accept");
            break;
        }
        int servLastChar = -1;
        while (1) {
            bzero(buffer, sizeBuf);
            n = (int) read(newSocket, buffer, sizeBuf - 1);
            if (n < 0) {
                error("ERROR reading from socket");
                break;
            }

            if (strcmp("exit\n", buffer) == 0) {
                printf("client is disconnected\n");
                break;
            }

            int isfind = 0;
            for (int i = 0; i < idCity; ++i) {
                if (strcmp(buffer, clientCities[i]) == 0) {
                    isfind = 1;
                    break;
                }
            }
            if (!isfind) {
                strcpy(clientCities[idCity], buffer);
                idCity++;
            }

            if (servLastChar != -1 && (isfind || servLastChar != buffer[0] || checkStr(buffer) == 0)) {
                n = (int) write(newSocket, "!", 1);
                if (n < 0) {
                    perror("Error writing to socket");
                    break;
                }
                continue;
            }

            int lastCh = toupper(buffer[strlen(buffer) - 2]);

//            putchar(lastCh);

            int id = -1;
            for (int i = 0; i < numOfCity && id == -1; ++i) {
                if (!used[i] && lastCh == listOfCities[i][0]) {
                    id = i;
                    used[i] = 1;
                }
            }

            printf("Client's city: %s", buffer);
            if (id == -1) {
                printf("Client win\n");
                n = (int) write(newSocket, "win", sizeof(listOfCities[id]));
            } else {
                n = (int) write(newSocket, listOfCities[id], 4 * sizeof(listOfCities[id]));
                servLastChar = listOfCities[id][strlen(listOfCities[id]) - 1];
                servLastChar = toupper(servLastChar);
            }
            if (n < 0) {
                perror("ERROR writing to socket");
                break;
            }
            if (id == -1) break;
        }
        close(newSocket);
    }
    return 0;
}
