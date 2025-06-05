/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_AES.h
  by Syafiqlim @ syafiqlimx
*****************************/

#ifndef ESP32_MYSQL_AES_H
#define ESP32_MYSQL_AES_H

#include "mbedtls/aes.h"
#include <string>
using namespace std;

class ESP32_MySQL_AES
{
public:
    ESP32_MySQL_AES();
    ~ESP32_MySQL_AES();

    void init(const char* key);
    string encrypt(const char* plaintext, size_t length);

private:
    mbedtls_aes_context aes;
    char key[32];

    void pad(const char* input, size_t length, char* output, size_t &outputLength);
    void charsToHex(const char* input, size_t length, string &output);
};

#endif // ESP32_MYSQL_AES_H
