#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void embed_lsb1(struct params * params) { //TODO
    readbmp(params->p);
}

void embed_lsb4(struct params * params) { //TODO
    printf("in: %s\n", params->in);
    printf("p: %s\n", params->p);
    printf("out: %s\n", params->out);
    printf("steg: lsb4\n");
    if (params->pass != NULL) {
        printf("a: %s\n", params->a);
        printf("m: %s\n", params->m);
        printf("pass: %s\n", params->pass);
    }
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

void extract_lsb1(struct params * params) { //TODO
    printf("p: %s\n", params->p);
    printf("out: %s\n", params->out);
    printf("steg: lsb1\n");
    if (params->pass != NULL) {
        printf("a: %s\n", params->a);
        printf("m: %s\n", params->m);
        printf("pass: %s\n", params->pass);
    }
}

void extract_lsb4(struct params * params) { //TODO
    printf("p: %s\n", params->p);
    printf("out: %s\n", params->out);
    printf("steg: lsb4\n");
    if (params->pass != NULL) {
        printf("a: %s\n", params->a);
        printf("m: %s\n", params->m);
        printf("pass: %s\n", params->pass);
    }
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
}