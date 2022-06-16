#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encryption.h"
#include "bmpreader.h"

struct params;

struct parser_steg_transition {
    int (*when)(const char* s);
    void    (*act)(struct params * params);
};

struct parser_cripto_transition {
    int (*when)(const char* s);
    void (*set)(const char* s, struct params * params);
    char* def;
};

struct params {
    const char* in;
    const char* p;
    const char* out;
    const char* pass;
    const char* a;
    const char* m;
    const char* steg;
    const struct parser_steg_transition * func;
};

int is_lsb1(const char* s) {
    return strcmp(s, "lsb1") == 0;
}

int is_lsb4(const char* s) {
    return strcmp(s, "lsb4") == 0;
}

int is_lsbi(const char* s) {
    return strcmp(s, "lsbi") == 0;
}

int is_a(const char* s) {
    return strcmp(s, "-a") == 0;
}

int is_m(const char* s) {
    return strcmp(s, "-m") == 0;
}

int is_pass(const char* s) {
    return strcmp(s, "-pass") == 0;
}

void set_a(const char* s, struct params * params) {
    params->a = s;
}

void set_m(const char* s, struct params * params) {
    params->m = s;
}

void set_pass(const char* s, struct params * params) {
    params->pass = s;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot;
}

void prepare_embed(struct params * params, FILE ** in, FILE ** fptr, FILE ** out, const char ** ext, HEADER * header, INFOHEADER * infoheader) {
    if ((*in = fopen(params->in, "r")) == NULL) {
        fprintf(stderr, "Unable to open in file \"%s\"\n", params->in);
        exit(-1);
    }
    if ((*fptr = fopen(params->p, "r")) == NULL) {
        fprintf(stderr, "Unable to open BMP file \"%s\"\n", params->p);
        exit(-1);
    }
    *out = fopen(params->out, "wt");

    *ext = get_filename_ext(params->in);

    readHeader(header, infoheader, *fptr);

    fwrite(&header->type, sizeof (unsigned short int), 1, *out);
    fwrite(&header->size, sizeof (unsigned int), 1, *out);
    fwrite(&header->reserved1, sizeof (unsigned short int), 1, *out);
    fwrite(&header->reserved2, sizeof (unsigned short int), 1, *out);
    fwrite(&header->offset, sizeof (unsigned int), 1, *out);
    fwrite(infoheader, sizeof (INFOHEADER), 1, *out);

    fseek(*fptr, header->offset, SEEK_SET);
    fseek(*out,header->offset,SEEK_SET);
}

void embed_lsb1(struct params * params) { //TODO
    HEADER header;
    INFOHEADER infoheader;
    const char * ext;
    FILE * in;
    FILE * fptr;
    FILE * out;

    prepare_embed(params, &in, &fptr, &out, &ext, &header, &infoheader);

    set_bmp_lsb1(&infoheader, in, fptr, out, ext);

    fclose(in);
    fclose(fptr);
    fclose(out);
}

void embed_lsb4(struct params * params) { //TODO
    HEADER header;
    INFOHEADER infoheader;
    const char * ext;
    FILE * in;
    FILE * fptr;
    FILE * out;

    prepare_embed(params, &in, &fptr, &out, &ext, &header, &infoheader);

    set_bmp_lsb4(&infoheader, in, fptr, out, ext);

    fclose(in);
    fclose(fptr);
    fclose(out);
}

void embed_lsbi(struct params * params) { //TODO
    printf("in: %s\n", params->in);
    printf("p: %s\n", params->p);
    printf("out: %s\n", params->out);
    printf("steg: lsbi\n");
    if (params->pass != NULL) {
        printf("a: %s\n", params->a);
        printf("m: %s\n", params->m);
        printf("pass: %s\n", params->pass);
    }
}

void prepare_extract(struct params * params, FILE ** fptr, FILE ** out, HEADER * header, INFOHEADER * infoheader) {
    if ((*fptr = fopen(params->p, "r")) == NULL) {
        fprintf(stderr, "Unable to open BMP file \"%s\"\n", params->p);
        exit(-1);
    }
    *out = fopen(params->out, "wt");

    readHeader(header, infoheader, *fptr);

    fseek(*fptr, header->offset, SEEK_SET);
}

void extract_lsb1(struct params * params) { //TODO
    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;

    prepare_extract(params, &fptr, &out, &header, &infoheader);

    set_out_lsb1(&infoheader, fptr, out);

    fclose(fptr);
    fclose(out);
}

void extract_lsb4(struct params * params) { //TODO

    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;

    prepare_extract(params, &fptr, &out, &header, &infoheader);

    set_out_lsb4(&infoheader, fptr, out);

    fclose(fptr);
    fclose(out);
}

void extract_lsbi(struct params * params) { //TODO
    printf("p: %s\n", params->p);
    printf("out: %s\n", params->out);
    printf("steg: lsbi\n");
    if (params->pass != NULL) {
        printf("a: %s\n", params->a);
        printf("m: %s\n", params->m);
        printf("pass: %s\n", params->pass);
    }
}

static const struct parser_steg_transition EMBED_STEG_TRANSITION [] = {
        {.when = is_lsb1, .act = embed_lsb1},
        {.when = is_lsb4, .act = embed_lsb4},
        {.when = is_lsbi, .act = embed_lsbi}
};

static const struct parser_steg_transition EXTRACT_STEG_TRANSITION [] = {
        {.when = is_lsb1, .act = extract_lsb1},
        {.when = is_lsb4, .act = extract_lsb4},
        {.when = is_lsbi, .act = extract_lsbi}
};

static const struct parser_cripto_transition CRIPTO_TRANSITION [] = {
        {.when = is_a, .set = set_a, .def = "aes128"},
        {.when = is_m, .set = set_m, .def = "cbc"},
        {.when = is_pass, .set = set_pass, .def = NULL}
};

int main (int argc, char const *argv[]) {

    //Para que funcione DES

    /*OSSL_PROVIDER * legacy;
    OSSL_PROVIDER * _default;

    legacy = OSSL_PROVIDER_load(NULL, "legacy");
    if (legacy == NULL) {
        printf("Failed to load Legacy provider\n");
        exit(EXIT_FAILURE);
    }
    _default = OSSL_PROVIDER_load(NULL, "default");
    if (_default == NULL) {
        printf("Failed to load Default provider\n");
        OSSL_PROVIDER_unload(legacy);
        exit(EXIT_FAILURE);
    } */

     struct params * params = malloc(sizeof (struct params));
     int p;

     if (strcmp(argv[1], "-embed") == 0) {
         params->in = argv[3];
         params->func = EMBED_STEG_TRANSITION;
         p = 2;
     } else {
         params->func = EXTRACT_STEG_TRANSITION;
         p = 0;
     }

     params->p = argv[3+p];
     params->out = argv[5+p];
     params->steg = argv[7+p];

     if (argc == 8+p) {
         params->pass = NULL;
     } else {
         for (int i=0; i < 3; i++) {
             if (CRIPTO_TRANSITION[i].when(argv[8+p])) {
                 CRIPTO_TRANSITION[i].set(argv[9+p], params);
                 p+=2;
             } else {
                 CRIPTO_TRANSITION[i].set(CRIPTO_TRANSITION[i].def, params);
             }
         }
     }

     for (int i=0; i < 3; i++) {
         if (params->func[i].when(params->steg)) {
             params->func[i].act(params);
             break;
         }
     }

     printf("START\n");

     printf("Pass: %s\n", params->pass);

     unsigned char encrypted_text[MAX_ENCR_LENGTH];
     int encryption_size = encrypt_text(params->a, params->m, params->in, encrypted_text, params->pass);
     printf("Encryption size: %d\n", encryption_size);

     printf("Encriptado en hexa %s\n", encrypted_text);

     unsigned char decrypted_text[MAX_ENCR_LENGTH];
     char extension[10];
     int text_size = decrypt_text(params->a, params->m, encrypted_text, encryption_size, decrypted_text, extension, params->pass);

     printf("Text Size afuera: %d\n", text_size);

     free(params);

    /*OSSL_PROVIDER_unload(legacy);
    OSSL_PROVIDER_unload(_default);*/
    return 0;
}