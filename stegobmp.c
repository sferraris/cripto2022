#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char const *argv[]) {
    if (strcmp(argv[1], "-embed") == 0) {
        char* in = argv[3]; //FIXME abir el archivo directo
        printf("%s\n", in);
    }
}