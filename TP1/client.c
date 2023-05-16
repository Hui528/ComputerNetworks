/*
** client.c -- a stream socket client demo
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

// using code from http://www.beej.us/guide/bgnet/
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    printf("Client is up and running.\n");
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // char port[10];
    // memset(&port, '\0', 10);
    // itoa(ntohs(sa.sin_port), port, 10);

    // printf("Local port is: %d\n", (int)ntohs(sa.sin_port));

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

    // inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
    //           s, sizeof s);
    // printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    while (1)
    {
        char message[MAXDATASIZE];
        memset(&message, '\0', sizeof(message));
        printf("Enter Department Name: ");
        scanf("%s", message);

        if (send(sockfd, message, sizeof(message), 0) == -1)
        {
            perror("client: failed to send msg\n");
            return 1;
        }

        printf("Client has sent Department %s to Main Server using TCP.\n", message);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            return 1;
        }
        buf[numbytes] = '\0';

        if (strcmp(buf, "Not found") == 0)
        {
            printf("%s not found.\n", message);
        }
        else
        {
            printf("Client has received results from Main Server: %s is associated with backend server %s.\n", message, buf);
        }
        printf("\n-----Start a new query-----\n");
    }

    close(sockfd);

    return 0;
}
