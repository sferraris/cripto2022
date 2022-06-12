//
// Created by santi on 6/6/2022.
//

#include "encryption.h"

const EVP_CIPHER * get_cipher(const char * encryption, const char * block_cipher);
int encrypt(const EVP_CIPHER *cipher, const unsigned char *in,unsigned char out_buffer[MAX_ENCR_LENGTH]);
int decrypt(const EVP_CIPHER *cipher,unsigned char in[MAX_ENCR_LENGTH],unsigned char *out_buffer, int in_length);
unsigned char * parse_in_file(const char * file_name, unsigned int *size);

/**
 * @param cipher recibe el cipher que se va a utilizar para encriptar la entrada. Por ejemplo, aes128 cbc = EVP_aes_128_cbc()
 * @param in la entrada que se quiere cifrar
 * @return retorna FAILURE (-1) si falla, retorna la longitud de la encripción si no falla
 */
int encrypt(const EVP_CIPHER *cipher, const unsigned char *in, unsigned char out_buffer[MAX_ENCR_LENGTH])
{

    int in_length, out_length, temp_length;

    unsigned char *key = "0123456789012345";/*128 bits = 16 bytes*/
    unsigned char iv[] = "5432109876543210";

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    EVP_CIPHER_CTX_init(ctx);

    EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv);

    in_length = strlen(in);



    if (!EVP_EncryptUpdate(ctx, out_buffer, &out_length, in, in_length)) {
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    if (!EVP_EncryptFinal_ex(ctx, out_buffer + out_length, &temp_length)) {
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    EVP_CIPHER_CTX_cleanup(ctx);

    return out_length + temp_length;
}

int decrypt(const EVP_CIPHER *cipher,unsigned char in[MAX_ENCR_LENGTH],unsigned char *out_buffer, int in_length)
{

    int out_length, temp_length;

    unsigned char *key = "0123456789012345";/*128 bits = 16 bytes*/
    unsigned char iv[] = "5432109876543210";

    printf("Encriptado en hexa %x\n", in);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    EVP_CIPHER_CTX_init(ctx);

    EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv);

    if (!EVP_DecryptUpdate(ctx, out_buffer, &out_length, in, in_length)) {
        printf("En el update\n");
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    printf("Length: %d\n", out_length);
    printf("Buffer: %s\n", out_buffer);
    printf("Length: %d\n", strlen(out_buffer));

    if (!EVP_DecryptFinal_ex(ctx, out_buffer + out_length, &temp_length)) {
        printf("En el final\n");
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    EVP_CIPHER_CTX_cleanup(ctx);

    return out_length + temp_length;
}

unsigned char * parse_in_file(const char * file_name, unsigned int *size){

    int file_size;

    FILE * file;

    const char *extension = strrchr(file_name, '.');
    int ext_size = strlen(extension) + 1;

    printf("File Name: %s, Extension: %s, size: %d \n", file_name, extension, ext_size);

    file = fopen(file_name, "rb+");
    if ( file == NULL)
        return FAILURE;

    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    int total_size = 4 + file_size + ext_size;

    unsigned char * out_text = malloc(total_size);

    *size = total_size;

    //agregar size al principio
    for(int i = 3; i >= 0; i--){
        char byte = (char)((file_size >> i*8) & 0xFF);
        out_text[3-i] = byte;
    }

    fread(out_text + 4, sizeof (unsigned char), file_size, file);

    fclose(file);

    memcpy(out_text + 4 + file_size, extension, ext_size);

    memcpy(out_text + 4 + file_size + ext_size, extension, '\0');

    return out_text;
}

int encrypt_text(const char * encryption, const char * block_cipher, const char * file_name, unsigned char encrypted_text[MAX_ENCR_LENGTH]){

    unsigned int text_to_encrypt_size;

    unsigned char * text_to_encrypt = parse_in_file(file_name, &text_to_encrypt_size);

    const EVP_CIPHER * cipher= get_cipher(encryption, block_cipher);

    return encrypt(cipher, text_to_encrypt, encrypted_text);
}

const EVP_CIPHER * get_cipher(const char * encryption, const char * block_cipher){

    char * aux = malloc(25);

    if ( strcmp(encryption, "aes128") == 0) strcat(aux, "aes-128");
    else if ( strcmp(encryption, "aes192") == 0) strcat(aux, "aes-192");
    else if ( strcmp(encryption, "aes256") == 0) strcat(aux, "aes-256");
    else if ( strcmp(encryption, "des") == 0) strcat(aux, "des");
    else return NULL;

    strcat(aux,"-");
    strcat(aux, block_cipher);

    return EVP_get_cipherbyname(aux);
}
