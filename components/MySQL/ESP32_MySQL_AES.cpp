/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 * 
 * Copyright (c) 2024 Syafiqlim
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/**************************** 
  ESP32_MySQL_Aes256_Impl.h
  by Syafiqlim @ syafiqlimx
*****************************/

#include <ESP32_MySQL_AES.hpp>
#include <cstring>

ESP32_MySQL_AES::ESP32_MySQL_AES() {
    mbedtls_aes_init(&aes);
}

ESP32_MySQL_AES::~ESP32_MySQL_AES() {
    mbedtls_aes_free(&aes);
}

void ESP32_MySQL_AES::init(const char* key) {
    memcpy(this->key, key, 32);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char *)this->key, 256);
}

void ESP32_MySQL_AES::pad(const char* input, size_t length, char* output, size_t &outputLength) {
    size_t padLength = 16 - (length % 16);
    outputLength = length + padLength;
    memcpy(output, input, length);
    memset(output + length, padLength, padLength);
}

void ESP32_MySQL_AES::charsToHex(const char* input, size_t length, string &output) {
    const char hexDigits[] = "0123456789abcdef";
    for (size_t i = 0; i < length; i++) {
        char b = input[i];
        output += hexDigits[b >> 4];
        output += hexDigits[b & 0x0F];
    }
}

string ESP32_MySQL_AES::encrypt(const char* plaintext, size_t length) {
    char paddedPlaintext[64];  // Adjust size as necessary
    size_t paddedLength;
    pad(plaintext, length, paddedPlaintext, paddedLength);

    char ciphertext[64];  // Adjust size as necessary
    for (size_t i = 0; i < paddedLength; i += 16) {
        mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT,(const unsigned char *) paddedPlaintext + i, (unsigned char *) ciphertext + i);
    }

    string hexCiphertext;
    charsToHex(ciphertext, paddedLength, hexCiphertext);

    return hexCiphertext;
}
