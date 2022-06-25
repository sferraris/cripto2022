//
// Created by santi on 6/6/2022.
//

#include "encryption.h"

const EVP_CIPHER * get_cipher(const char * encryption, const char * block_cipher);
int encrypt(const EVP_CIPHER *cipher, const unsigned char *in, unsigned char * out_buffer, int in_length, const char * pass);
int decrypt(const EVP_CIPHER *cipher,unsigned char * in,unsigned char *out_buffer, int in_length, const char * pass);
unsigned char * parse_in_file(const char * file_name, unsigned int *size);

int encrypt(const EVP_CIPHER *cipher, const unsigned char *in, unsigned char * out_buffer, int in_length, const char * pass)
{

    OpenSSL_add_all_algorithms();

    int out_length, temp_length;

    const unsigned char * salt = NULL;
    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

    const EVP_MD * dgst= EVP_sha256();

    if(!dgst) {
        fprintf(stderr, "no dgst\n");
        exit(1);
    }

    if(!EVP_BytesToKey(cipher, dgst, salt,(unsigned char *) pass,strlen(pass), 1, key, iv))
    {
        fprintf(stderr, "EVP_BytesToKey failed\n");
        exit(1);
    }


    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv);

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

int decrypt(const EVP_CIPHER *cipher,unsigned char * in,unsigned char *out_buffer, int in_length, const char * pass)
{

    OpenSSL_add_all_algorithms();

    int out_length, temp_length;

    const unsigned char * salt = NULL;
    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

    const EVP_MD * dgst= EVP_sha256();

    if(!dgst) {
        fprintf(stderr, "no dgst\n");
        exit(1);
    }

    if(!EVP_BytesToKey(cipher, dgst, salt,(unsigned char *) pass,strlen(pass), 1, key, iv))
    {
        fprintf(stderr, "EVP_BytesToKey failed\n");
        exit(1);
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    if (!EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv)){
        printf("Falla en el init\n");
    }

    if (!EVP_DecryptUpdate(ctx, out_buffer, &out_length, in, in_length)) {
        printf("Falla en el update\n");
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    if (!EVP_DecryptFinal_ex(ctx, out_buffer + out_length, &temp_length)) {
        printf("Falla en el final\n");
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

    file = fopen(file_name, "rb+");
    if ( file == NULL) {
        printf("No se pudo abrir el archivo a esconder.\n");
        return NULL;
    }
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    int total_size = 4 + file_size + ext_size;

    unsigned char * out_text = malloc(total_size);

    *size = total_size;

    for(int i = 3; i >= 0; i--){
        char byte = (char)((file_size >> i*8) & 0xFF);
        out_text[3-i] = byte;
    }
    fread(out_text + 4, sizeof (unsigned char), file_size, file);
    fclose(file);
    memcpy(out_text + 4 + file_size, extension, ext_size);

    return out_text;
}

int encrypt_text(const char * encryption, const char * block_cipher, unsigned char ** encrypted_text, const char * pass, unsigned char * text_to_encrypt, unsigned int text_to_encrypt_size){
    const EVP_CIPHER * cipher= get_cipher(encryption, block_cipher);
    if (cipher == NULL) {
        printf("Hubo un error al desecriptar el texto.\n");
    }

    unsigned int final_size = text_to_encrypt_size;

    if ( strcmp(encryption, "aes128") == 0 && final_size % AES_128_BLOCK != 0) final_size = final_size + AES_128_BLOCK - (final_size % AES_128_BLOCK);
    else if ( strcmp(encryption, "aes192") == 0 && final_size % AES_192_BLOCK != 0) final_size = final_size + AES_192_BLOCK - (final_size % AES_192_BLOCK);
    else if ( strcmp(encryption, "aes256") == 0 && final_size % AES_256_BLOCK != 0) final_size = final_size + AES_256_BLOCK - (final_size % AES_256_BLOCK);
    else if ( strcmp(encryption, "des") == 0 && final_size % DES_BLOCK != 0) final_size = final_size + DES_BLOCK - (final_size % DES_BLOCK);

    *encrypted_text = malloc(final_size + 1);

    return encrypt(cipher, text_to_encrypt, *encrypted_text, text_to_encrypt_size, pass);
}

int decrypt_text(const char * encryption, const char * block_cipher, unsigned char * text_to_decrypt, int encrypted_size, unsigned char * decrypted_text, char* extension, const char * pass){
    const EVP_CIPHER * cipher= get_cipher(encryption, block_cipher);
    if (cipher == NULL) {
        printf("Hubo un error al desecriptar el texto.\n");
    }

    unsigned char * info_from_decryption = malloc(encrypted_size);

    decrypt(cipher, text_to_decrypt, info_from_decryption, encrypted_size, pass);

    int text_size = 0;
    text_size = text_size | info_from_decryption[0];
    text_size = (text_size << 8) | info_from_decryption[1];
    text_size = (text_size << 8) | info_from_decryption[2];
    text_size = (text_size << 8) | info_from_decryption[3];

    memcpy(decrypted_text, info_from_decryption + 4, text_size);

    char * aux_ext = strrchr((const char *)info_from_decryption + 4 + text_size, '.');
    strcpy(extension, aux_ext);

    return text_size;
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
