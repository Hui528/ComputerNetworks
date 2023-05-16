#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <inttypes.h>
#include <fstream>

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

int main()
{
    std::ifstream file("dataRx.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            char crc[] = "0000000000000000"; // store remainder
            char data[line.length() + 1];
            strcpy(data, line.c_str());
            char codeword[strlen(data) + 16];
            strcpy(codeword, data);
            strcpy(codeword + strlen(data), crc);
            if (crc_rx(data))
                printf("%s\n", "Pass");
            else
                printf("%s\n", "Not Pass");
        }
        file.close();
    }
}