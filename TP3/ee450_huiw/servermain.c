#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "33692" // port with TCP client

#define BACKLOG 10 // how many pending connections queue will hold

#define MAXDATASIZE 1024 // max number of bytes we can get at once

#define MAXNAMESIZE 20

#define IP "127.0.0.1"

#define PORT_SERVERA 30692

#define PORT_SERVERB 31692

#define PORT_SERVERMAIN 32692 // port with UDP server

// using code from http://www.beej.us/guide/bgnet/
void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

int main(void)
{
    char *not_found = "Student not found";
    char *dpt_not_found = "Dpt not found";

    char message[MAXDATASIZE];
    memset(message, '\0', sizeof(message));

    // TCP server related code

    int sockfd_srv, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    // struct sockaddr_storage their_addr; // connector's address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    int client_1 = 0;
    int client_2 = 0;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // using code from http://www.beej.us/guide/bgnet/
    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd_srv = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd_srv, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd_srv, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd_srv);
            perror("server: bind");
            continue;
        }

        break;
    }

    // using code from http://www.beej.us/guide/bgnet/
    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd_srv, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // UDP Client related code

    int sockfd_clt;
    struct sockaddr_in srcaddr, serverAaddr, serverBaddr;
    int n1, n2, n3;
    socklen_t len;
    char *query_bootup = "Bootup";
    char deparments_serverA[MAXDATASIZE];
    char deparments_serverB[MAXDATASIZE];

    printf("Main server is up and running.\n");

    // IPv4, set to AF_INET6 to use IPv6
    if ((sockfd_clt = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("servermain error: socket");
        exit(EXIT_FAILURE);
    }

    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_port = htons(PORT_SERVERMAIN);
    srcaddr.sin_addr.s_addr = inet_addr(IP);

    if (bind(sockfd_clt, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0)
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
    sendto(sockfd_clt, (const char *)query_bootup, strlen(query_bootup),
           0, (const struct sockaddr *)&serverAaddr,
           sizeof(serverAaddr));

    n1 = recvfrom(sockfd_clt, (char *)deparments_serverA, MAXDATASIZE,
                  MSG_WAITALL, (struct sockaddr *)&serverAaddr,
                  &len);
    deparments_serverA[n1] = '\0';

    printf("Main server has received the department list from server A using UDP over port %d\n", PORT_SERVERMAIN);

    // get list of departments on serverB
    sendto(sockfd_clt, (const char *)query_bootup, strlen(query_bootup),
           0, (const struct sockaddr *)&serverBaddr,
           sizeof(serverBaddr));

    n2 = recvfrom(sockfd_clt, (char *)deparments_serverB, MAXDATASIZE,
                  MSG_WAITALL, (struct sockaddr *)&serverBaddr,
                  &len);
    deparments_serverB[n2] = '\0';

    printf("Main server has received the department list from server B using UDP over port %d\n", PORT_SERVERMAIN);

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
    { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd_srv, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd == -1)
        {
            exit(1);
        }

        int client_port = ntohs(their_addr.sin_port);
        if (client_1 == 0)
        {
            client_1 = client_port;
        }
        else
        {
            client_2 = client_port;
        }

        if (!fork())
        {
            close(sockfd_srv);
            while (1)
            {
                int receive = recv(new_fd, message, MAXDATASIZE, 0);
                if (receive == -1)
                {
                    perror("recv");
                }
                else if (receive == 0)
                {
                    break;
                }
                // parse message and send requests to serverA.c and serverB.c

                char buffer[MAXDATASIZE];
                memset(&buffer, '\0', sizeof(buffer));

                char msg_copy[MAXDATASIZE];
                memset(&msg_copy, '\0', sizeof(msg_copy));
                strcpy(msg_copy, message);

                char *target_dpt = strtok(msg_copy, " ");
                char *target_id = strtok(NULL, " ");

                if (client_port == client_1)
                {
                    printf("Main server has received the request on Student %s in %s from client %d using TCP over port %s\n", target_id, target_dpt, 1, PORT);
                }
                else
                {
                    printf("Main server has received the request on Student %s in %s from client %d using TCP over port %s\n", target_id, target_dpt, 2, PORT);
                }

                if (strstr(deparments_serverA, target_dpt) != NULL)
                {
                    printf("%s shows up in server A\n", target_dpt);
                    printf("Main Server has sent request of Student %s to server A using UDP over port %d\n", target_id, PORT_SERVERMAIN);

                    sendto(sockfd_clt, (const char *)message, strlen(message),
                           0, (const struct sockaddr *)&serverAaddr,
                           sizeof(serverAaddr));

                    n3 = recvfrom(sockfd_clt, (char *)buffer, MAXDATASIZE,
                                  MSG_WAITALL, (struct sockaddr *)&serverAaddr,
                                  &len);

                    if (strcmp(buffer, not_found) == 0)
                    {
                        printf("Main server has received “Student %s: Not found” from server A\n", target_id);
                        if (send(new_fd, buffer, strlen(buffer), 0) == -1)
                            perror("send");
                        if (client_port == client_1)
                        {
                            printf("Main Server has sent message to client %d using TCP over %s\n", 1, PORT);
                        }
                        else
                        {
                            printf("Main Server has sent message to client %d using TCP over %s\n", 2, PORT);
                        }
                    }
                    else
                    {
                        printf("Main server has received searching result of Student %s from server A\n", target_id);

                        if (send(new_fd, buffer, strlen(buffer), 0) == -1)
                            perror("send");
                        if (client_port == client_1)
                        {
                            printf("Main Server has sent searching result(s) to client %d using TCP over port %s\n", 1, PORT);
                        }
                        else
                        {
                            printf("Main Server has sent searching result(s) to client %d using TCP over port %s\n", 2, PORT);
                        }
                    }
                }
                else if (strstr(deparments_serverB, target_dpt) != NULL)
                {
                    printf("%s shows up in server B\n", target_dpt);
                    printf("Main Server has sent request of Student %s to server B using UDP over port %d\n", target_id, PORT_SERVERMAIN);
                    sendto(sockfd_clt, (const char *)message, strlen(message),
                           0, (const struct sockaddr *)&serverBaddr,
                           sizeof(serverBaddr));

                    n3 = recvfrom(sockfd_clt, (char *)buffer, MAXDATASIZE,
                                  MSG_WAITALL, (struct sockaddr *)&serverBaddr,
                                  &len);

                    if (strcmp(buffer, not_found) == 0)
                    {
                        printf("Main server has received “Student %s: Not found” from server B\n", target_id);
                        if (send(new_fd, buffer, strlen(buffer), 0) == -1)
                            perror("send");
                        if (client_port == client_1)
                        {
                            printf("Main Server has sent message to client %d using TCP over %s\n", 1, PORT);
                        }
                        else
                        {
                            printf("Main Server has sent message to client %d using TCP over %s\n", 2, PORT);
                        }
                    }
                    else
                    {
                        printf("Main server has received searching result of Student %s from server B\n", target_id);

                        if (send(new_fd, buffer, strlen(buffer), 0) == -1)
                            perror("send");

                        if (client_port == client_1)
                        {
                            printf("Main Server has sent searching result(s) to client %d using TCP over port %s\n", 1, PORT);
                        }
                        else
                        {
                            printf("Main Server has sent searching result(s) to client %d using TCP over port %s\n", 2, PORT);
                        }
                    }
                }
                else
                {
                    printf("%s does not show up in server A&B\n", target_dpt);
                    if (send(new_fd, dpt_not_found, strlen(dpt_not_found), 0) == -1)
                        perror("send");
                    if (client_port == client_1)
                    {
                        printf("Main Server has sent “%s: Not found” to client %d using TCP over port %s\n", target_dpt, 1, PORT);
                    }
                    else
                    {
                        printf("Main Server has sent “%s: Not found” to client %d using TCP over port %s\n", target_dpt, 2, PORT);
                    }
                }
                memset(message, '\0', sizeof(message));
            }
        }
        close(new_fd);
    }

    return 0;
}
