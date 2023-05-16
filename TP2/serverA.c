#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define IP "127.0.0.1"
#define PORT 30692
#define MAXDATASIZE 1024
#define MAXLENGTH 25

struct ID
{
    char *id; // Student IDs are non-negative integer numbers less than 20 digits
    struct ID *next;
};

struct department
{
    char *name;    // The length of a department name can vary from 1 letter to at most 20 letters
    struct ID *ID; // at most 100 student IDs
    int countID;
    struct department *next;
};

struct department *department_head = NULL;
struct department *department_current = NULL;

int containsID(char *id, struct ID *list)
{
    list = list->next;
    while (list != NULL)
    {
        if (strcmp(list->id, id) == 0) // key already exists
        {
            return 1;
        }
        list = list->next;
    }
    return 0;
}

int printInfo(struct department *list)
{
    list = list->next;
    while (list != NULL)
    {
        printf("department: %s\n", list->name);
        printf("count: %d\n", list->countID);
        struct ID *id = list->ID;
        id = id->next;
        while (id != NULL)
        {
            printf("%s\n", id->id);
            id = id->next;
        }
        printf("---------------\n");
        list = list->next;
    }
    return 0;
}

int main()
{
    char *query_bootup = "Bootup";
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    char cur_name[30];
    char *cur_id = NULL;
    department_head = (struct department *)malloc(sizeof(struct department)); // dummy department
    department_current = department_head;

    printf("Server A is up and running using UDP on port %d\n", PORT);

    fp = fopen("dataA.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((fscanf(fp, "%s", cur_name)) != -1)
    {
        int count = 0;
        getline(&line, &len, fp);
        if (getline(&line, &len, fp) == -1)
        {
            perror("ServerA: fail to read student IDs");
            return 1;
        }

        line = strtok(line, "\r");
        line = strtok(line, "\n");
        cur_id = strtok(line, ",");

        struct ID *ID_head = (struct ID *)malloc(sizeof(struct ID)); // dummy ID
        struct ID *ID_current = ID_head;

        while (cur_id != NULL)
        {
            if (containsID(cur_id, ID_head) == 0)
            {
                ID_current->next = (struct ID *)malloc(sizeof(struct ID));
                ID_current = ID_current->next;

                char *id_copy = (char *)malloc(MAXLENGTH);
                strcpy(id_copy, cur_id);
                ID_current->id = id_copy;
                count += 1;
            }
            cur_id = strtok(NULL, ",");
        }
        department_current->next = (struct department *)malloc(sizeof(struct department));
        department_current = department_current->next;
        char *name_copy = (char *)malloc(MAXLENGTH);
        strcpy(name_copy, cur_name);
        department_current->name = name_copy;
        department_current->ID = ID_head;
        department_current->countID = count;
    }

    // printInfo(department_head);

    int sockfd;
    char buffer[MAXDATASIZE];
    struct sockaddr_in serveraddr, clientaddr;
    int n;
    socklen_t addrlen;

    // IPv4, set to AF_INET6 to use IPv6
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("serverA error: socket");
        exit(EXIT_FAILURE);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    memset(&clientaddr, 0, sizeof(clientaddr));

    serveraddr.sin_family = AF_INET; // IPv4
    serveraddr.sin_addr.s_addr = inet_addr(IP);
    serveraddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&serveraddr,
             sizeof(serveraddr)) < 0)
    {
        perror("serverA error: bind failed");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(clientaddr);

    while (1)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXDATASIZE,
                     MSG_WAITALL, (struct sockaddr *)&clientaddr,
                     &addrlen);
        buffer[n] = '\0';

        struct department *temp = department_head->next;

        // reply all departments' names
        if (strcmp(buffer, query_bootup) == 0)
        {
            int pos = 0;
            char department_list[300]; // The length of a department name can vary from 1 letter to at most 20 letters, There are at most 10 department names in total.
            memset(department_list, '\0', sizeof(department_list));
            while (temp != NULL)
            {
                strcpy(department_list + pos, temp->name);
                pos += strlen(temp->name);
                strcpy(department_list + pos, " ");
                pos += 1;
                temp = temp->next;
            }
            strcpy(department_list + pos - 1, "\0");
            sendto(sockfd, (const char *)department_list, strlen(department_list),
                   0, (const struct sockaddr *)&clientaddr,
                   addrlen);
            printf("Server A has sent a department list to Main Server\n");
        }

        else // build the ID list to send back
        {
            printf("Server A has received a request for %s\n", buffer);
            while (temp != NULL)
            {
                if (strcmp(temp->name, buffer) == 0)
                {
                    break;
                }
                temp = temp->next;
            }
            int pos = 0;
            char ID_list[MAXLENGTH * (1 + temp->countID)];
            memset(ID_list, '\0', sizeof(ID_list));

            char countID[5];
            memset(countID, '\0', sizeof(countID));
            sprintf(countID, "%d", temp->countID);
            strcpy(ID_list + pos, countID);
            pos += strlen(countID);
            strcpy(ID_list + pos, " ");
            pos += 1;

            struct ID *temp_id = temp->ID->next;
            while (temp_id != NULL)
            {
                strcpy(ID_list + pos, temp_id->id);
                pos += strlen(temp_id->id);
                strcpy(ID_list + pos, ",");
                pos += 1;
                temp_id = temp_id->next;
            }
            strcpy(ID_list + pos - 1, ".");

            char copy[MAXLENGTH * (1 + temp->countID)];
            memset(copy, '\0', sizeof(copy));
            strcpy(copy, ID_list);
            char *num = strtok(copy, " ");
            char *IDs = strtok(NULL, " ");

            printf("Server A found %s distinct students for %s: %s\n", num, buffer, IDs);

            sendto(sockfd, (const char *)ID_list, strlen(ID_list),
                   0, (const struct sockaddr *)&clientaddr,
                   addrlen);

            printf("Server A has sent the results to Main Server\n");
        }
    }

    return 0;
}