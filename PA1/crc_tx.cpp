#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <inttypes.h>
#include <fstream>

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

int main()
{
    std::ifstream file("dataTx.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            char crc[] = "000000000000000"; // r = 16 bits
            char data[line.length() + 1];
            strcpy(data, line.c_str());
            char codeword[strlen(data) + 16];
            strcpy(codeword, data);
            strcpy(codeword + strlen(data), crc);
            crc_tx(data, crc, codeword);
            printf("Codeword is:\n");
            printf("%s", codeword);
            printf("\n");
            printf("CRC is\n");
            printf("%s", crc);
            printf("\n\n");
        }
        file.close();
    }
    // char data[] = "1010101010";
    // char crc[] = "0000";
    // char codeword[strlen(data) + 4];
    // crc_tx(data, crc, codeword);
    // printf("%s", data);
    // printf("\n");
    // printf("%s", crc);
    // printf("\n");
    // printf("%s", codeword);
    // return 0;
}