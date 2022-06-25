//
// Created by santi on 6/6/2022.
//

#ifndef CRIPTO2022_ENCRYPTION_H
#define CRIPTO2022_ENCRYPTION_H



#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include <openssl/aes.h>



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAILURE -1
#define DES_BLOCK 8
#define AES_128_BLOCK 16
#define AES_192_BLOCK 24
#define AES_256_BLOCK 32

int encrypt_text(const char * encryption, const char * block_cipher, unsigned char ** encrypted_text, const char * pass, unsigned char * text_to_encrypt, unsigned int text_to_encrypt_size);

int decrypt_text(const char * encryption, const char * block_cipher, unsigned char * text_to_decrypt, int encrypted_size, unsigned char * decrypted_text, char* extension, const char * pass);

unsigned char * parse_in_file(const char * file_name, unsigned int *size);

#endif //CRIPTO2022_ENCRYPTION_H
