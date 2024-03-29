/*
** server.c -- a stream socket server demo
*/

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

#define PORT "33692" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold

#define MAXDATASIZE 1024 // max number of bytes we can get at once

#define MAXNAMESIZE 20

// hash table related code: Department name and Backend server ID
struct node
{
    char *data;
    char *key;
    struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;

char name[MAXNAMESIZE];

int total_backendServer = 0;

int insertFirst(char *key, char *code)
{
    // check whether key already exists
    struct node *current = head;
    while (current != NULL && strcmp(current->data, code) == 0)
    {

        if (strcmp(current->key, key) == 0) // key already exists
        {
            return 1;
        }
        else
        {
            current = current->next;
        }
    }

    // create a link
    struct node *link = (struct node *)malloc(sizeof(struct node));

    char *key_copy = (char *)malloc(MAXNAMESIZE);
    // memset(&key_copy, '\0', sizeof(key_copy));
    strcpy(key_copy, key);
    link->key = key_copy;
    // printf("link->key length: %d\n", sizeof(link->key));

    // char *data_copy = (char *)malloc(strlen(data));
    char *code_copy = (char *)malloc(strlen(code));
    strcpy(code_copy, code);
    link->data = code_copy;

    // point it to old first node
    link->next = head;

    // point first to new first node
    head = link;
    return 0;
}

// find a link with given key
struct node *find(char *key)
{
    struct node *current = head;
    if (head == NULL)
    {
        return NULL;
    }
    // printf("Inside find func(head): %s\n", head->data);
    while (strcmp(current->key, key) != 0)
    {
        // printf("key: Inside find func: %s\n", current->key);

        if (current->next == NULL)
        {
            return NULL;
        }
        else
        {
            current = current->next;
        }
    }
    return current;
}

// server related code

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

// using code from http://www.beej.us/guide/bgnet/
// get port, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_port);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_port);
}

int main(void)
{
    // read list.txt and store info with hash table

    printf("Main server is up and running.\n");

    FILE *fp;
    char line_1[10];
    int code = 0;
    char *line_2 = NULL;
    char *line_3 = NULL;
    size_t len = 0;
    ssize_t read;

    char *pos;
    int cur_backendServer = 0;

    int BackendServerIDs[10];
    int countDepartmentsInBackendServerIDs[10];
    int countingIndex = 0;

    int client_1 = 0;
    int client_2 = 0;

    fp = fopen("list.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    printf("Main server has read the department list from list.txt.\n");

    while ((fscanf(fp, "%d", &code)) != -1)
    {
        sprintf(line_1, "%d", code);
        getline(&line_2, &len, fp);
        if (getline(&line_2, &len, fp) == -1)
        {
            perror("Server: fail to read department name");
            return 1;
        }

        line_2 = strtok(line_2, "\r");
        line_2 = strtok(line_2, "\n");
        line_2 = strtok(line_2, ",");
        while (line_2 != NULL)
        {

            if (insertFirst(line_2, line_1) == 0)
            {
                cur_backendServer += 1;
                total_backendServer += 1;
            }
            line_2 = strtok(NULL, ",");
        }

        BackendServerIDs[countingIndex] = code;
        countDepartmentsInBackendServerIDs[countingIndex] = cur_backendServer;
        countingIndex += 1;

        cur_backendServer = 0;
    }

    printf("Total number of Backend Servers: %d\n", total_backendServer);

    for (int i = 0; i < countingIndex; i++)
    {
        printf("Backend Servers %d contains %d distinct departments\n", BackendServerIDs[i], countDepartmentsInBackendServerIDs[i]);
    }

    fclose(fp);

    // server related code start here
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    // struct sockaddr_storage their_addr; // connector's address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char buf[MAXDATASIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // using code from http://www.beej.us/guide/bgnet/
    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
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

    if (listen(sockfd, BACKLOG) == -1)
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

    // printf("server: waiting for connections...\n");

    while (1)
    { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
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

        // printf("Client port is: %d\n", client_port);

        // inet_ntop(their_addr.ss_family,
        //           get_in_addr((struct sockaddr *)&their_addr),
        //           s, sizeof s);
        // printf("server: got connection from %s\n", s);

        if (!fork())
        {
            close(sockfd);
            while (1)
            {
                int receive = recv(new_fd, buf, MAXDATASIZE, 0);
                if (receive == -1)
                {
                    perror("recv");
                }
                else if (receive == 0)
                {
                    break;
                }

                if (client_port == client_1)
                {
                    printf("Main server has received the request on Department %s from client%d using TCP over port %s\n", buf, 1, PORT);
                }
                else
                {
                    printf("Main server has received the request on Department %s from client%d using TCP over port %s\n", buf, 2, PORT);
                }
                // // show client port number
                // int client_port = ntohs(their_addr.sin_port);
                // printf("Client connected from: %d\n", client_port);

                struct node *found = find(buf);
                if (found == NULL)
                {
                    printf("%s does not show up in backend server ", buf);
                    printf("%d", BackendServerIDs[0]);
                    for (int i = 1; i < countingIndex - 1; i++)
                    {
                        printf(", %d", BackendServerIDs[i]);
                    }
                    printf("\n");

                    if (send(new_fd, "Not found", strlen("Not found"), 0) == -1)
                        perror("send");

                    if (client_port == client_1)
                    {
                        printf("The Main Server has sent “%s: Not found” to client%d using TCP over port %s\n", buf, 1, PORT);
                    }
                    else
                    {
                        printf("The Main Server has sent “%s: Not found” to client%d using TCP over port %s\n", buf, 2, PORT);
                    }
                }
                else
                {
                    printf("%s shows up in backend server %s\n", buf, found->data);
                    if (send(new_fd, found->data, strlen(found->data), 0) == -1)
                        perror("send");
                    if (client_port == client_1)
                    {
                        printf("Main Server has sent searching result to client%d using TCP over port %s\n", 1, PORT);
                    }
                    else
                    {
                        printf("Main Server has sent searching result to client%d using TCP over port %s\n", 2, PORT);
                    }
                    bzero(buf, sizeof(buf));
                }
            }
        }
        close(new_fd);
    }

    return 0;
}
