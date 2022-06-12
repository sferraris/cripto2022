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

#define MAX_ENCR_LENGTH 1024
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
int encrypt_text(const char * encryption, const char * block_cipher, const char * file_name, unsigned char encrypted_text[MAX_ENCR_LENGTH]);


#endif //CRIPTO2022_ENCRYPTION_H
