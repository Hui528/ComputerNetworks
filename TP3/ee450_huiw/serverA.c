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

struct score
{
    int sc;
    struct score *next;
};

struct ID
{
    char *id; // Student IDs are non-negative integer numbers less than 20 digits
    float GPA;
    float rank;
    struct score *score;
    struct ID *next;
};

struct department
{
    char *name; // The length of a department name can vary from 1 letter to at most 20 letters
    float GPA_max;
    float GPA_min;
    float GPA_mean;
    float GPA_var;
    int count_ids;
    struct ID *ID; // at most 100 student IDs
    struct department *next;
};

struct department *department_head = NULL;

// return 1 when success, 0 when fail
int addStudentToDpt(char *dpt, struct ID *id)
{
    struct department *dpt_crt = department_head;
    while (dpt_crt->next != NULL)
    {
        dpt_crt = dpt_crt->next;
        if (strcmp(dpt_crt->name, dpt) == 0)
        {
            struct ID *old_id = dpt_crt->ID->next;
            dpt_crt->ID->next = id;
            id->next = old_id;
            dpt_crt->count_ids += 1;
            return 1;
        }
    }
    // dpt does not exist in dpt list
    struct department *dpt_new = (struct department *)malloc(sizeof(struct department));
    char *name_copy = (char *)malloc(MAXLENGTH);
    strcpy(name_copy, dpt);
    dpt_new->name = name_copy;
    dpt_new->ID = (struct ID *)malloc(sizeof(struct ID)); // dummy ID
    dpt_new->ID->next = id;
    dpt_new->count_ids = 1;

    dpt_crt->next = dpt_new;

    return 1;
}

void calculation()
{
    struct department *dpt = department_head->next;
    while (dpt != NULL)
    {
        float GPA_max = 0.0;
        float GPA_min = 100.0;
        float GPA_total = 0.0;
        float GPAs[dpt->count_ids];
        int index = 0;

        struct ID *ID = dpt->ID->next;
        while (ID != NULL)
        {
            int count = 0;
            int total = 0;
            struct score *sc = ID->score->next;
            while (sc != NULL)
            {
                if (sc->sc >= 0)
                {
                    total += sc->sc;
                    count += 1;
                }
                sc = sc->next;
            }
            ID->GPA = (float)(total) / (float)(count);
            if (ID->GPA > GPA_max)
                GPA_max = ID->GPA;
            if (ID->GPA < GPA_min)
                GPA_min = ID->GPA;
            GPA_total += ID->GPA;
            GPAs[index] = ID->GPA;
            index += 1;

            ID = ID->next;
        }

        dpt->GPA_max = GPA_max;
        dpt->GPA_min = GPA_min;
        dpt->GPA_mean = GPA_total / (float)(dpt->count_ids);

        // calculate GPA Variance
        float diff_sqr = 0.0;
        for (int i = 0; i < dpt->count_ids; i++)
        {
            diff_sqr += (GPAs[i] - dpt->GPA_mean) * (GPAs[i] - dpt->GPA_mean);
        }
        dpt->GPA_var = diff_sqr / (float)(dpt->count_ids);

        struct ID *ID_for_rank = dpt->ID->next;
        while (ID_for_rank != NULL)
        {
            int count_lower = 0;
            for (int i = 0; i < dpt->count_ids; i++)
            {
                if (GPAs[i] < ID_for_rank->GPA)
                {
                    count_lower += 1;
                }
            }
            ID_for_rank->rank = (float)(count_lower) / (float)(dpt->count_ids);
            ID_for_rank = ID_for_rank->next;
        }

        dpt = dpt->next;
    }
}

char *strtok_single(char *str, char const *delims)
{
    static char *start = NULL;
    char *token = NULL;
    if (str != NULL)
    {
        start = str;
    }
    if (start == NULL)
    {
        return NULL;
    }
    token = start;
    start = strpbrk(start, delims); // Returns a pointer to the first occurrence in str1 of any of the characters that are part of str2, or a null pointer if there are no matches.
    if (start != NULL)
    {
        *start++ = '\0';
    }
    return token;
}

char *findRec(struct ID *start, struct ID *targetID)
{
    char *res = NULL;
    float diff = 100.0;
    while (start != NULL)
    {
        if (start != targetID)
        {
            if (start->GPA >= targetID->GPA && diff > (start->GPA - targetID->GPA))
            {
                diff = start->GPA - targetID->GPA;
                res = start->id;
            }
            else if (start->GPA < targetID->GPA && diff > (targetID->GPA - start->GPA))
            {
                diff = targetID->GPA - start->GPA;
                res = start->id;
            }
        }
        start = start->next;
    }
    if (res != NULL)
    {
        return res;
    }

    struct department *dpt = department_head->next;
    while (dpt != NULL)
    {
        struct ID *start = dpt->ID->next;
        while (start != NULL)
        {
            if (start != targetID)
            {
                if (start->GPA >= targetID->GPA && diff > (start->GPA - targetID->GPA))
                {
                    diff = start->GPA - targetID->GPA;
                    res = start->id;
                }
                else if (start->GPA < targetID->GPA && diff > (targetID->GPA - start->GPA))
                {
                    diff = targetID->GPA - start->GPA;
                    res = start->id;
                }
            }
            start = start->next;
        }
        dpt = dpt->next;
    }
    return res;
}

int printInfo()
{
    struct department *dpt = department_head->next;
    while (dpt != NULL)
    {
        printf("department: %s max: %f min: %f mean: %f var: %f count_ids: %d\n", dpt->name, dpt->GPA_max, dpt->GPA_min, dpt->GPA_mean, dpt->GPA_var, dpt->count_ids);
        struct ID *ID = dpt->ID->next;
        while (ID != NULL)
        {
            printf("ID: %s GPA: %f rank: %f\n", ID->id, ID->GPA, ID->rank);
            struct score *sc = ID->score->next;
            while (sc != NULL)
            {
                printf("%d\n", sc->sc);
                sc = sc->next;
            }
            ID = ID->next;
            printf("---------\n");
        }
        dpt = dpt->next;
        printf("------------------------\n");
    }
    return 0;
}

int main()
{
    char *query_bootup = "Bootup";
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    char *cur_dpt = NULL;
    char *cur_id = NULL;
    char *cur_score = NULL;
    department_head = (struct department *)malloc(sizeof(struct department)); // dummy department

    printf("Server A is up and running using UDP on port %d\n", PORT);

    fp = fopen("dataA.csv", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // skip first line
    if (getline(&line, &len, fp) == -1)
    {
        perror("ServerA: fail to read dataA.csv");
        return 1;
    }

    while (getline(&line, &len, fp) != -1)
    {
        line = strtok(line, "\r");
        line = strtok(line, "\n");

        cur_dpt = strtok_single(line, ",");

        struct ID *id = (struct ID *)malloc(sizeof(struct ID));
        cur_id = strtok_single(NULL, ",");
        char *id_copy = (char *)malloc(MAXLENGTH);
        strcpy(id_copy, cur_id);
        id->id = id_copy;

        struct score *score_head = (struct score *)malloc(sizeof(struct score));
        struct score *score_current = score_head;

        cur_score = strtok_single(NULL, ",");
        while (cur_score != NULL)
        {
            score_current->next = (struct score *)malloc(sizeof(struct score));
            score_current = score_current->next;
            score_current->sc = *cur_score ? atoi(cur_score) : -1;
            cur_score = strtok_single(NULL, ",");
        }

        id->score = score_head;

        addStudentToDpt(cur_dpt, id);
    }

    calculation();
    // printInfo();

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

        struct department *temp_dpt = department_head->next;

        // reply all departments' names
        if (strcmp(buffer, query_bootup) == 0)
        {
            int pos = 0;
            char department_list[300]; // The length of a department name can vary from 1 letter to at most 20 letters, There are at most 10 department names in total.
            memset(department_list, '\0', sizeof(department_list));
            while (temp_dpt != NULL)
            {
                strcpy(department_list + pos, temp_dpt->name);
                pos += strlen(temp_dpt->name);
                strcpy(department_list + pos, " ");
                pos += 1;
                temp_dpt = temp_dpt->next;
            }
            strcpy(department_list + pos - 1, "\0");
            sendto(sockfd, (const char *)department_list, strlen(department_list),
                   0, (const struct sockaddr *)&clientaddr,
                   addrlen);
            printf("Server A has sent a department list to Main Server\n");
        }

        else // build the ID list to send back
        {
            char *target_dpt = strtok(buffer, " ");
            char *target_id = strtok(NULL, " ");
            printf("Server A has received a request for Student %s in %s\n", target_id, target_dpt);
            while (temp_dpt != NULL)
            {
                if (strcmp(temp_dpt->name, target_dpt) == 0)
                {
                    break;
                }
                temp_dpt = temp_dpt->next;
            }
            struct ID *temp_id = temp_dpt->ID->next;
            while (temp_id != NULL)
            {
                if (strcmp(temp_id->id, target_id) == 0)
                {
                    break;
                }
                temp_id = temp_id->next;
            }
            if (temp_id == NULL)
            {
                printf("Student %s does not show up in %s\n", target_id, target_dpt);
                char *not_found = "Student not found";
                sendto(sockfd, (const char *)not_found, strlen(not_found),
                       0, (const struct sockaddr *)&clientaddr,
                       addrlen);
                printf("Server A has sent “Student %s not found” to Main Server\n", target_id);
            }
            else
            {
                char *rec = findRec(temp_dpt->ID->next, temp_id);
                char std_rec[MAXDATASIZE];
                memset(std_rec, '\0', sizeof(std_rec));
                strcpy(std_rec, rec);
                printf("Server A calculated following academic statistics for Student %s in %s:\n", target_id, target_dpt);
                printf("Student GPA: %f\n", temp_id->GPA);
                printf("Percentage Rank: %f\n", temp_id->rank);
                printf("Department GPA Mean: %f\n", temp_dpt->GPA_mean);
                printf("Department GPA Variance: %f\n", temp_dpt->GPA_var);
                printf("Department Max GPA: %f\n", temp_dpt->GPA_max);
                printf("Department Min GPA: %f\n", temp_dpt->GPA_min);

                int pos = 0;
                char result[300];
                memset(result, '\0', sizeof(result));
                sprintf(result, "%f %f %f %f %f %f %s", temp_id->GPA, temp_id->rank, temp_dpt->GPA_mean, temp_dpt->GPA_var, temp_dpt->GPA_max, temp_dpt->GPA_min, std_rec);

                sendto(sockfd, (const char *)result, strlen(result),
                       0, (const struct sockaddr *)&clientaddr,
                       addrlen);

                printf("Server A has sent the result to Main Server\n");
            }
        }
    }

    return 0;
}