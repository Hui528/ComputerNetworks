#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <inttypes.h>
#include <fstream>
#include <math.h>

void crc_tx(char data[], char crc[], char codeword[])
{
    char generator[] = "10001000000100001"; // r + 1 bits, r = 16
    char remainder[] = "0000000000000000";  // r + 1 = 17 bits
    int glen = strlen(generator);           // 17
    int datalen = strlen(data);
    char quot[datalen];
    char temp[glen];
    char key[glen];
    strcpy(key, generator);
    char input[datalen + glen - 1]; // datalen + r bits
    strncpy(input, data, datalen);
    for (int i = 0; i < glen - 1; i++)
    {
        input[datalen + i] = '0';
    }
    strncpy(temp, input, glen);
    for (int i = 0; i < datalen; i++)
    {
        quot[i] = temp[0];
        if (quot[i] == '0')
        {
            for (int j = 0; j < glen; j++)
            {
                key[j] = '0';
            }
        }
        else
        {
            strcpy(key, generator);
        }
        for (int j = glen - 1; j > 0; j--)
        {
            if (temp[j] == key[j])
            {
                remainder[j - 1] = '0';
            }
            else
            {
                remainder[j - 1] = '1';
            }
        }
        if (i != datalen - 1)
        {
            remainder[glen - 1] = input[i + glen];
        }
        strcpy(temp, remainder);
    }
    strcpy(remainder, temp);
    strncpy(crc, remainder, glen - 1);
    strcpy(codeword, data);
    strcpy(codeword + datalen, crc);
}

bool crc_rx(char data[])
{
    char generator[] = "10001000000100001";
    char remainder[] = "0000000000000000"; // r + 1 = 17 bits
    int glen = strlen(generator);
    int datalen = strlen(data);
    char quot[datalen - glen];
    char temp[glen];
    char key[glen];
    strcpy(key, generator);
    for (int i = 0; i < glen; i++)
    {
        temp[i] = data[i];
    }
    for (int i = 0; i < datalen - glen + 1; i++)
    {
        quot[i] = temp[0];
        if (quot[i] == '0')
        {
            for (int j = 0; j < glen; j++)
            {
                key[j] = '0';
            }
        }
        else
        {
            strcpy(key, generator);
        }
        for (int j = glen - 1; j > 0; j--)
        {
            if (temp[j] == key[j])
            {
                remainder[j - 1] = '0';
            }
            else
            {
                remainder[j - 1] = '1';
            }
        }
        if (i != datalen - glen)
        {
            remainder[glen - 1] = data[i + glen];
        }
        strcpy(temp, remainder);
    }
    strncpy(remainder, temp, glen - 1);
    for (int i = 0; i < glen - 1; i++)
    {
        if (remainder[i] == '1')
            return false;
    }
    return true;
}

void parity2d(char data[], char col[], char row[])
{
    int i = 0;
    int datalen = strlen(data);
    int colsum[] = {0, 0, 0, 0, 0, 0, 0, 0};
    int rowsum[] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < datalen; i++)
    {
        if (data[i] == '1')
        {
            rowsum[i / 8] += 1;
            colsum[i % 8] += 1;
        }
    }
    for (int i = 0; i < 8; i++)
    {
        if (colsum[i] % 2 == 1)
        {
            col[i] = '1';
        }
        if (rowsum[i] % 2 == 1)
        {
            row[i] = '1';
        }
    }
    int count = 0;
    for (int i = 0; i < strlen(row); i++)
    {
        if (row[i] == '1')
        {
            count += 1;
        }
    }
    if (count % 2 == 1)
    {
        row[strlen(row) - 1] = '1';
    }
}

bool check_parity2d(char data[], char col[], char row[], char error[])
{
    int errorlen = strlen(error);
    for (int i = 0; i < errorlen; i++)
    {
        if (error[i] == '0')
        {
            continue;
        }
        // error[i] == '1'
        if (i >= strlen(data) + strlen(col) + strlen(row))
        {
            break;
        }
        if (i >= strlen(data) + strlen(row) - 1)
        {
            if (row[i - (strlen(data) + strlen(row) - 1)] == '1')
            {
                col[i - (strlen(data) + strlen(row) - 1)] = '0';
            }
            else
            {
                col[i - (strlen(data) + strlen(row) - 1)] = '1';
            }
        }
        else
        {
            // i < strlen(data) + strlen(row) - 1
            if ((i + 1) % 9 == 0) // i + 1 / 9 == 0
            {
                if (row[((i + 1) / 9) - 1] == '1')
                {
                    row[((i + 1) / 9) - 1] = '0';
                }
                else
                {
                    row[((i + 1) / 9) - 1] = '1';
                }
            }
            else
            {
                if (data[i - ((i + 1) / 9)] == '1')
                {
                    data[i - ((i + 1) / 9)] = '0';
                }
                else
                {
                    data[i - ((i + 1) / 9)] = '1';
                }
            }
        }
    }
    char updated_col[] = "00000000";
    char updated_row[] = "000000000";
    parity2d(data, updated_col, updated_row);
    return strcmp(col, updated_col) == 0 && strcmp(row, updated_row) == 0;
}

bool check_crc(char codeword[], char error[])
{
    int errorlen = strlen(error);
    int codewordlen = strlen(codeword);
    for (int i = 0; i < errorlen; i++)
    {
        if (i > codewordlen)
        {
            break;
        }
        if (error[i] == '1')
        {
            if (codeword[i] == '1')
            {
                codeword[i] = '0';
            }
            else
            {
                codeword[i] = '1';
            }
        }
    }
    return crc_rx(codeword);
}

int main()
{
    std::ifstream file("dataVs.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            char *input = new char[line.length() + 1];
            strcpy(input, line.c_str());
            int splitIndex = 0;
            for (int i = 0; i < strlen(input); i++)
            {
                if (input[i] == ' ')
                {
                    splitIndex = i; // 64
                    break;
                }
            }
            char *data = new char[splitIndex + 1];                              // 64 + 1 = 65
            char *error = new char[strlen(input) - splitIndex];                 // 64 + 72 + 1 + 1 - 64 - 1 = 73
            strncpy(data, input, splitIndex);                                   // 64
            strncpy(error, input + splitIndex + 1, strlen(input) - splitIndex); // 64 + 72 + 1 + 1 - 64 - 1 = 73
            // printf("%s", data);
            // printf("\n");
            // printf("%s", error);
            // printf("\n");

            // parity2d
            char *temp_parity2d = new char[splitIndex + 1];
            strcpy(temp_parity2d, data);
            char col[] = "00000000";
            char row[] = "000000000";
            parity2d(temp_parity2d, col, row);
            printf("2D Parity: Col: ");
            printf("%s", col);
            printf("; Row: ");
            printf("%s", row);
            printf(";\t");
            bool res_parity2d = check_parity2d(temp_parity2d, col, row, error);
            if (res_parity2d)
            {
                printf("Result: Pass\n");
            }
            else
            {
                printf("Result: Not Pass\n");
            }
            // crc
            char *temp_crc = new char[splitIndex + 1];
            strcpy(temp_crc, data);
            char crc[] = "0000000000000000"; // r = 16 bits
            char *codeword = new char[strlen(temp_crc) + 16];
            strcpy(codeword, temp_crc);
            strcpy(codeword + strlen(temp_crc), crc);
            crc_tx(temp_crc, crc, codeword);

            printf("CRC: ");
            printf("%s", crc);
            printf(";\t");

            bool res_crc = check_crc(codeword, error);
            if (res_crc)
            {
                printf("Result: Pass\n");
            }
            else
            {
                printf("Result: Not Pass\n");
            }
            printf("==============================\n");
        }
        file.close();
    }
}