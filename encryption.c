//
// Created by santi on 6/6/2022.
//

#include "encryption.h"

int encrypt()
{

    const EVP_CIPHER *cipher;
    cipher = EVP_get_cipherbyname("aes-256-ofb");

    printf("Cipher: %s", cipher);
    return 0;
}
