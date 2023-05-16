#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define IP "127.0.0.1"
#define PORT_SERVERA 30692
#define PORT_SERVERB 31692
#define PORT_SERVERMAIN 32692
#define MAXDATASIZE 1024 // max number of bytes we can get at once

int main()
{
    int sockfd;
    char buffer[MAXDATASIZE];
    struct sockaddr_in srcaddr, serverAaddr, serverBaddr;
    int n1, n2, n3;
    socklen_t len;
    char *query_bootup = "Bootup";
    char deparments_serverA[MAXDATASIZE];
    char deparments_serverB[MAXDATASIZE];

    printf("Main server is up and running.\n");

    // IPv4, set to AF_INET6 to use IPv6
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("servermain error: socket");
        exit(EXIT_FAILURE);
    }

    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_port = htons(PORT_SERVERMAIN);
    srcaddr.sin_addr.s_addr = inet_addr(IP);

    if (bind(sockfd, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0)
    {
        perror("servermain error: bind");
        exit(1);
    }

    memset(&serverAaddr, 0, sizeof(serverAaddr));
    serverAaddr.sin_family = AF_INET;
    serverAaddr.sin_port = htons(PORT_SERVERA);
    serverAaddr.sin_addr.s_addr = inet_addr(IP);

    memset(&serverBaddr, 0, sizeof(serverBaddr));
    serverBaddr.sin_family = AF_INET;
    serverBaddr.sin_port = htons(PORT_SERVERB);
    serverBaddr.sin_addr.s_addr = inet_addr(IP);

    // get list of departments on serverA
    sendto(sockfd, (const char *)query_bootup, strlen(query_bootup),
           0, (const struct sockaddr *)&serverAaddr,
           sizeof(serverAaddr));

    n1 = recvfrom(sockfd, (char *)deparments_serverA, MAXDATASIZE,
                  MSG_WAITALL, (struct sockaddr *)&serverAaddr,
                  &len);
    deparments_serverA[n1] = '\0';

    printf("Main server has received the department list from server A using UDP over port %d\n", PORT_SERVERA);

    // get list of departments on serverB
    sendto(sockfd, (const char *)query_bootup, strlen(query_bootup),
           0, (const struct sockaddr *)&serverBaddr,
           sizeof(serverBaddr));

    n2 = recvfrom(sockfd, (char *)deparments_serverB, MAXDATASIZE,
                  MSG_WAITALL, (struct sockaddr *)&serverBaddr,
                  &len);
    deparments_serverB[n2] = '\0';

    printf("Main server has received the department list from server B using UDP over port %d\n", PORT_SERVERB);

    // List the results of which department serverA/B is responsible for
    char *name = NULL;
    char copy_A[MAXDATASIZE];
    memset(copy_A, '\0', sizeof(copy_A));
    strcpy(copy_A, deparments_serverA);
    name = strtok(copy_A, " ");
    printf("Server A\n");
    while (name != NULL)
    {
        printf("%s\n", name);
        name = strtok(NULL, " ");
    }
    char copy_B[MAXDATASIZE];
    memset(copy_B, '\0', sizeof(copy_B));
    strcpy(copy_B, deparments_serverB);
    name = strtok(copy_B, " ");
    printf("Server B\n");
    while (name != NULL)
    {
        printf("%s\n", name);
        name = strtok(NULL, " ");
    }

    while (1)
    {
        char message[MAXDATASIZE];
        memset(&message, '\0', sizeof(message));
        printf("Enter Department Name: ");
        scanf("%s", message);

        if (strstr(deparments_serverA, message) != NULL)
        {
            printf("%s shows up in server A\n", message);
            printf("The Main Server has sent request for %s to server A using UDP over port %d\n", message, PORT_SERVERA);
            sendto(sockfd, (const char *)message, strlen(message),
                   0, (const struct sockaddr *)&serverAaddr,
                   sizeof(serverAaddr));

            n3 = recvfrom(sockfd, (char *)buffer, MAXDATASIZE,
                          MSG_WAITALL, (struct sockaddr *)&serverAaddr,
                          &len);
            buffer[n3] = '\0';
            char *num = strtok(buffer, " ");
            char *IDs = strtok(NULL, " ");
            printf("The Main server has received searching result(s) of %s from serverA\n", message);
            printf("There are %s distinct students in %s. Their IDs are %s\n", num, message, IDs);
        }
        else if (strstr(deparments_serverB, message) != NULL)
        {
            printf("%s shows up in server B\n", message);
            printf("The Main Server has sent request for %s to server B using UDP over port %d\n", message, PORT_SERVERB);
            sendto(sockfd, (const char *)message, strlen(message),
                   0, (const struct sockaddr *)&serverBaddr,
                   sizeof(serverBaddr));

            n3 = recvfrom(sockfd, (char *)buffer, MAXDATASIZE,
                          MSG_WAITALL, (struct sockaddr *)&serverBaddr,
                          &len);
            buffer[n3] = '\0';
            char *num = strtok(buffer, " ");
            char *IDs = strtok(NULL, " ");
            printf("The Main server has received searching result(s) of %s from serverB\n", message);
            printf("There are %s distinct students in %s. Their IDs are %s\n", num, message, IDs);
        }
        else
        {
            printf("%s does not show up in server A&B\n", message);
        }
        printf("-----Start a new query-----\n");
    }

    close(sockfd);
    return 0;
}
