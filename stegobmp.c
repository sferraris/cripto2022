#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/provider.h>

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

unsigned char * prepare_embed(struct params * params, FILE ** fptr, FILE ** out, HEADER * header, INFOHEADER * infoheader, unsigned int * final_size) {
    unsigned int before_size;
    unsigned char * before_text;

    if ((*fptr = fopen(params->p, "r")) == NULL) {
        fprintf(stderr, "Unable to open BMP file \"%s\"\n", params->p);
        exit(-1);
    }
    *out = fopen(params->out, "wt");

    before_text = parse_in_file(params->in, &before_size);
    *final_size = before_size;

    readHeader(header, infoheader, *fptr);

    if (infoheader->compression != 0) {
        printf("El archivo no debe estar comprimido.\n");
        exit(1);
    }

    fwrite(&header->type, sizeof (unsigned short int), 1, *out);
    fwrite(&header->size, sizeof (unsigned int), 1, *out);
    fwrite(&header->reserved1, sizeof (unsigned short int), 1, *out);
    fwrite(&header->reserved2, sizeof (unsigned short int), 1, *out);
    fwrite(&header->offset, sizeof (unsigned int), 1, *out);
    fwrite(infoheader, sizeof (INFOHEADER), 1, *out);

    fseek(*fptr, header->offset, SEEK_SET);
    fseek(*out,header->offset,SEEK_SET);

    if (params->pass != NULL) {
        unsigned char encrypted_text[MAX_ENCR_LENGTH];
        int encryption_size = encrypt_text(params->a, params->m, encrypted_text, params->pass, before_text, before_size);
        free(before_text);

        unsigned char * final_text = malloc(encryption_size + 4);
        for(int i = 3; i >= 0; i--){
            char byte = (char)((encryption_size >> i*8) & 0xFF);
            final_text[3-i] = byte;
        }
        *final_size = encryption_size+4;
        memcpy(final_text+4, encrypted_text, encryption_size);
        return final_text;
    }
    return before_text;
}

void embed_lsb1(struct params * params) {
    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;
    unsigned int size;

    unsigned char * in_text = prepare_embed(params, &fptr, &out, &header, &infoheader, &size);

    if (8*size > infoheader.imagesize) {
        printf("El archivo bmp no puede albergar el archivo a ocultar completamente\n");
        return;
    }

    set_bmp_lsb1(&infoheader, fptr, out, in_text, size);

    free(in_text);
    fclose(fptr);
    fclose(out);

    printf("El archivo se embebio con LSB1 correctamente.\n");
}

void embed_lsb4(struct params * params) {
    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;
    unsigned int size;

    unsigned char * in_text = prepare_embed(params, &fptr, &out, &header, &infoheader, &size);

    if (2*size > infoheader.imagesize) {
        printf("El archivo bmp no puede albergar el archivo a ocultar completamente\n");
        return;
    }

    set_bmp_lsb4(&infoheader, fptr, out, in_text, size);

    free(in_text);
    fclose(fptr);
    fclose(out);

    printf("El archivo se embebio con LSB4 correctamente.\n");
}

void embed_lsbi(struct params * params) {
    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;
    unsigned int size;

    unsigned char * in_text = prepare_embed(params, &fptr, &out, &header, &infoheader, &size);

    if (8*size+4 > infoheader.imagesize) {
        printf("El archivo bmp no puede albergar el archivo a ocultar completamente\n");
        return;
    }

    set_bmp_lsbi(&infoheader, fptr, out, in_text, size, &header);

    free(in_text);
    fclose(fptr);
    fclose(out);

    printf("El archivo se embebio con LSBI correctamente.\n");
}

void prepare_extract(struct params * params, FILE ** fptr, HEADER * header, INFOHEADER * infoheader) {
    if ((*fptr = fopen(params->p, "r")) == NULL) {
        fprintf(stderr, "Unable to open BMP file \"%s\"\n", params->p);
        exit(-1);
    }

    readHeader(header, infoheader, *fptr);

    if (infoheader->compression != 0) {
        printf("El archivo no debe estar comprimido.\n");
        exit(1);
    }

    fseek(*fptr, header->offset, SEEK_SET);
}

void extract_lsb1(struct params * params) {
    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;
    char extension[EXT_SIZE];
    unsigned int size;

    prepare_extract(params, &fptr, &header, &infoheader);

    unsigned char * fptr_text = set_out_lsb1(&infoheader, fptr, &size, params->pass != NULL);
    if (fptr_text == NULL) {
        printf("Hubo un problema al extraer el archivo.\n");
        return;
    }

    unsigned char * out_text = malloc(size);
    if (params->pass != NULL) {
        int text_size = decrypt_text(params->a, params->m, fptr_text, size, out_text, extension, params->pass);
        size = text_size;
    } else {
        memcpy(out_text, fptr_text, size);
        memcpy(extension, fptr_text + size, EXT_SIZE);
    }

    strcat(params->out, extension);

    out = fopen(params->out, "wt");
    fwrite(out_text, size, 1, out);
    free(fptr_text);
    free(out_text);
    fclose(fptr);
    fclose(out);

    printf("El archivo se extrajo con LSB1 correctamente.\n");
}

void extract_lsb4(struct params * params) {
    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;
    char extension[EXT_SIZE];
    unsigned int size;

    prepare_extract(params, &fptr, &header, &infoheader);
    unsigned char * fptr_text = set_out_lsb4(&infoheader, fptr, &size, params->pass != NULL);
    if (fptr_text == NULL) {
        printf("Hubo un problema al extraer el archivo.\n");
        return;
    }

    unsigned char * out_text = malloc(size);
    if (params->pass != NULL) {
        int text_size = decrypt_text(params->a, params->m, fptr_text, size, out_text, extension, params->pass);
        size = text_size;
    } else {
        memcpy(out_text, fptr_text, size);
        memcpy(extension, fptr_text + size, EXT_SIZE);
    }

    strcat(params->out, extension);
    out = fopen(params->out, "wt");

    fwrite(out_text, size, 1, out);

    free(fptr_text);
    free(out_text);
    fclose(fptr);
    fclose(out);

    printf("El archivo se extrajo con LSB4 correctamente.\n");
}

void extract_lsbi(struct params * params) {
    HEADER header;
    INFOHEADER infoheader;
    FILE * fptr;
    FILE * out;
    char extension[EXT_SIZE];
    unsigned int size;

    prepare_extract(params, &fptr, &header, &infoheader);

    unsigned char * fptr_text = set_out_lsbi(&infoheader, fptr, &size, params->pass != NULL);
    if (fptr_text == NULL) {
        printf("Hubo un problema al extraer el archivo.\n");
        return;
    }

    unsigned char * out_text = malloc(size);
    if (params->pass != NULL) {
        int text_size = decrypt_text(params->a, params->m, fptr_text, size, out_text, extension, params->pass);
        size = text_size;
    } else {
        memcpy(out_text, fptr_text, size);
        memcpy(extension, fptr_text + size, EXT_SIZE);
    }

    strcat(params->out, extension);
    out = fopen(params->out, "wt");

    fwrite(out_text, size, 1, out);

    free(fptr_text);
    free(out_text);
    fclose(fptr);
    fclose(out);

    printf("El archivo se extrajo con LSBI correctamente.\n");
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

    OSSL_PROVIDER * legacy;
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
    }

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

     free(params);

    OSSL_PROVIDER_unload(legacy);
    OSSL_PROVIDER_unload(_default);
    return 0;
}
