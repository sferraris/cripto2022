//
// Created by santi on 6/6/2022.
//

#include "encryption.h"

const EVP_CIPHER * get_cipher(const char * encryption, const char * block_cipher);
int encrypt(const EVP_CIPHER *cipher, const unsigned char *in, unsigned char out_buffer[MAX_ENCR_LENGTH], int in_length);
int decrypt(const EVP_CIPHER *cipher,unsigned char in[MAX_ENCR_LENGTH],unsigned char *out_buffer, int in_length);
unsigned char * parse_in_file(const char * file_name, unsigned int *size);

/**
 * @param cipher recibe el cipher que se va a utilizar para encriptar la entrada. Por ejemplo, aes128 cbc = EVP_aes_128_cbc()
 * @param in la entrada que se quiere cifrar
 * @return retorna FAILURE (-1) si falla, retorna la longitud de la encripción si no falla
 */
int encrypt(const EVP_CIPHER *cipher, const unsigned char *in, unsigned char out_buffer[MAX_ENCR_LENGTH], int in_length)
{

    OpenSSL_add_all_algorithms();

    int out_length, temp_length;

    //unsigned char *key = "0123456789012345";/*128 bits = 16 bytes*/
   // unsigned char iv[] = "5432109876543210";

    const unsigned char * salt = NULL;
    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

    const EVP_MD * dgst= EVP_sha256();

    if(!dgst) {
        fprintf(stderr, "no dgst\n");
        exit(1);
    }

    const char * pass = "pass";

    if(!EVP_BytesToKey(cipher, dgst, salt,(unsigned char *) pass,strlen(pass), 1, key, iv))
    {
        fprintf(stderr, "EVP_BytesToKey failed\n");
        exit(1);
    }


    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    //EVP_CIPHER_CTX_init(ctx);

    EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv);

    unsigned char out_buffer2[MAX_ENCR_LENGTH];

    if (!EVP_EncryptUpdate(ctx, out_buffer, &out_length, in, in_length)) {
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    if (!EVP_EncryptFinal_ex(ctx, out_buffer + out_length, &temp_length)) {
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    EVP_CIPHER_CTX_cleanup(ctx);

    printf("Encriptado en hexa: %s\n", out_buffer);

    return out_length + temp_length;
}

int decrypt(const EVP_CIPHER *cipher,unsigned char in[MAX_ENCR_LENGTH],unsigned char *out_buffer, int in_length)
{

    OpenSSL_add_all_algorithms();

    int out_length, temp_length;

    //unsigned char *key = "0123456789012345";/*128 bits = 16 bytes*/
    //unsigned char iv[] = "5432109876543210";

    const unsigned char * salt = NULL;
    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

    const EVP_MD * dgst= EVP_sha256();

    if(!dgst) {
        fprintf(stderr, "no dgst\n");
        exit(1);
    }

    const char * pass = "pass";

    if(!EVP_BytesToKey(cipher, dgst, salt,(unsigned char *) pass,strlen(pass), 1, key, iv))
    {
        fprintf(stderr, "EVP_BytesToKey failed\n");
        exit(1);
    }

    printf("Encriptado en hexa %s\n", in);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    //EVP_CIPHER_CTX_init(ctx);

    if (!EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv)){
        printf("falla el init\n");
    }

    if (!EVP_DecryptUpdate(ctx, out_buffer, &out_length, in, in_length)) {
        printf("En el update\n");
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    printf("Out Length: %d\n", out_length);

    if (!EVP_DecryptFinal_ex(ctx, out_buffer + out_length, &temp_length)) {
        printf("En el final\n");
        EVP_CIPHER_CTX_cleanup(ctx);
        return FAILURE;
    }

    EVP_CIPHER_CTX_cleanup(ctx);
    printf("in length: %d\n", in_length);
    printf("TOTAL SIZE: %d\n", out_length + temp_length);

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

    printf("SIZE TO ENCRYPT: %d\n", text_to_encrypt_size);

    printf("size : %d %d %d %d\n", text_to_encrypt[0], text_to_encrypt[1], text_to_encrypt[2], text_to_encrypt[3]);

    printf("TEXTO PLANO: %s\n", text_to_encrypt + 4);

    const EVP_CIPHER * cipher= get_cipher(encryption, block_cipher);

    return encrypt(cipher, text_to_encrypt, encrypted_text, text_to_encrypt_size);
}

int decrypt_text(const char * encryption, const char * block_cipher, unsigned char text_to_decrypt[MAX_ENCR_LENGTH], int encrypted_size, unsigned char * decrypted_text, char* extension){

    printf("DECRYPT ENTER\n");
    //TODO text
    //Tengo que recibir el metodo de encripcion ej aes128cbc
    //Tengo que recibir la encripcion y su tamaño -> tamaño en un int encripcion en un char[]
    //la encripcion viene de la forma encripcion(tamaño real || datos archivo || extensión)
    //1. lo desencripto, y recibo el length (que deberia ser igual al que le mande -> todo check en ambos)
    // y en el out buffer recibo tamaño real || datos archivo || extensión
    //2. Luego, los primeros 4 bytes son el size, lo extraigo
    //3. los siguientes size bytes son el mensaje, lo extraigo
    //4. y lo que queda es la extension, lo extraigo
    //5. retorno el size del mensaje en el valor de retorno, el mensaje en un buffer y la extension en otro buffer

    const EVP_CIPHER * cipher= get_cipher(encryption, block_cipher);

    printf("Encription: %s, cipher: %s\n", encryption, block_cipher);

    unsigned char * info_from_decryption = malloc(MAX_ENCR_LENGTH);

    unsigned int decrypted_text_size = decrypt(cipher, text_to_decrypt, info_from_decryption, encrypted_size);

    printf("Se tendria que cumplir creo(? --> %d == %d\n", decrypted_text_size, encrypted_size);

    int text_size = 0;
    text_size = text_size | info_from_decryption[0];
    text_size = (text_size << 8) | info_from_decryption[1];
    text_size = (text_size << 8) | info_from_decryption[2];
    text_size = (text_size << 8) | info_from_decryption[3];
    printf("Text Size: %d\n", text_size);

    memcpy(decrypted_text, info_from_decryption + 4, text_size);

    extension = strrchr((const char *)info_from_decryption + 4 + text_size, '.');

    printf("Decrypted Text: %s\n", decrypted_text);

    printf("Extension: %s\n", extension);

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
