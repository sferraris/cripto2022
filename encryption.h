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

#define MAX_ENCR_LENGTH 50000
#define FAILURE -1

/**
 *
 * @param encryption params->a
 * @param block_cipher params->m
 * @param file_name params->in
 * @param encrypted_text unsigned char encrypted_text[MAX_ENCR_LENGTH];
 * @return retorna el tamaño de la encripción
 * Ejemplo de llamada:
 * unsigned char encrypted_text[MAX_ENCR_LENGTH];
 * int encryption_size = encrypt_text(params->a, params->m, params->in, encrypted_text);
 */
int encrypt_text(const char * encryption, const char * block_cipher, unsigned char encrypted_text[MAX_ENCR_LENGTH], const char * pass, unsigned char * text_to_encrypt, unsigned int text_to_encrypt_size);

int decrypt_text(const char * encryption, const char * block_cipher, unsigned char text_to_decrypt[MAX_ENCR_LENGTH], int encrypted_size, unsigned char * decrypted_text, char* extension, const char * pass);

unsigned char * parse_in_file(const char * file_name, unsigned int *size);

#endif //CRIPTO2022_ENCRYPTION_H
