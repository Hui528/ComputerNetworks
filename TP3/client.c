/*
** client.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT "33692"

#define MAXDATASIZE 1024 // max number of bytes we can get at once

int main(int argc, char *argv[])
{
    char *not_found = "Student not found";
    char *dpt_not_found = "Dpt not found";

    printf("Client is up and running.\n");
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_in my_addr;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("127.0.0.1", SERVER_PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // using code from http://www.beej.us/guide/bgnet/
    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    // using code from http://www.beej.us/guide/bgnet/
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    // Get my port
    bzero(&my_addr, sizeof(my_addr));
    socklen_t len = sizeof(my_addr);
    getsockname(sockfd, (struct sockaddr *)&my_addr, &len);
    int myPort = ntohs(my_addr.sin_port);

    freeaddrinfo(servinfo); // all done with this structure

    while (1)
    {
        char target_dpt[MAXDATASIZE];
        char target_id[MAXDATASIZE];
        memset(&target_dpt, '\0', sizeof(target_dpt));
        memset(&target_id, '\0', sizeof(target_id));
        printf("Enter Department Name: ");
        scanf("%s", target_dpt);
        printf("Enter student ID: ");
        scanf("%s", target_id);

        char message[MAXDATASIZE * 2];
        memset(&message, '\0', sizeof(message));
        strcpy(message, target_dpt);
        strcpy(message + strlen(target_dpt), " ");
        strcpy(message + strlen(target_dpt) + 1, target_id);

        char buf[MAXDATASIZE];
        memset(&buf, '\0', sizeof(buf));

        if (send(sockfd, message, strlen(target_dpt) + 1 + strlen(target_id), 0) == -1)
        {
            perror("client: failed to send msg\n");
            return 1;
        }

        printf("Client has sent %s and Student %s to Main Server using TCP over port %d\n", target_dpt, target_id, myPort);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            return 1;
        }

        if (strcmp(buf, dpt_not_found) == 0)
        {
            printf("%s: Not found\n", target_dpt);
        }
        else if (strcmp(buf, not_found) == 0)
        {
            printf("Student %s: Not found\n", target_id);
        }
        else
        {
            printf("The performance statistics for Student %s in %s is:\n", target_id, target_dpt);
            printf("Student GPA: %s\n", strtok(buf, " "));
            printf("Percentage Rank: %s\n", strtok(NULL, " "));
            printf("Department GPA Mean: %s\n", strtok(NULL, " "));
            printf("Department GPA Variance: %s\n", strtok(NULL, " "));
            printf("Department Max GPA: %s\n", strtok(NULL, " "));
            printf("Department Min GPA: %s\n", strtok(NULL, " "));
            printf("Friend Recommendation: %s\n", strtok(NULL, " "));
        }
        printf("\n-----Start a new request-----\n");
    }

    close(sockfd);

    return 0;
}
