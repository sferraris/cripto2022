//
// Created by santi on 6/2/2022.
// http://paulbourke.net/dataformats/bmp/parse.c
//

#ifndef CRIPTO2022_BMPREADER_H
#define CRIPTO2022_BMPREADER_H
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"

#define EXT_SIZE 10

typedef struct {
    unsigned short int type;                 /* Magic identifier            */
    unsigned int size;                       /* File size in bytes          */
    unsigned short int reserved1, reserved2;
    unsigned int offset;                     /* Offset to image data, bytes */
} HEADER;
typedef struct {
    unsigned int size;               /* Header size in bytes      */
    int width,height;                /* Width and height of image */
    unsigned short int planes;       /* Number of colour planes   */
    unsigned short int bits;         /* Bits per pixel            */
    unsigned int compression;        /* Compression type          */
    unsigned int imagesize;          /* Image size in bytes       */
    int xresolution,yresolution;     /* Pixels per meter          */
    unsigned int ncolours;           /* Number of colours         */
    unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;

void readHeader(HEADER * header, INFOHEADER * infoheader, FILE * fptr);
void set_bmp_lsb1(INFOHEADER * infoheader, FILE * fptr, FILE * out, const unsigned char * in_text, unsigned int size);
unsigned char * set_out_lsb1(INFOHEADER * infoheader, FILE * fptr, unsigned int * size, int encripted);
void set_bmp_lsb4(INFOHEADER * infoheader, FILE * fptr, FILE * out, const unsigned char * in_text, unsigned int size);
unsigned char * set_out_lsb4(INFOHEADER * infoheader, FILE * fptr, unsigned int * size, int encripted);
void set_bmp_lsbi(INFOHEADER * infoheader, FILE * fptr, FILE * out, const unsigned char * in_text, unsigned int size, HEADER * header);
unsigned char * set_out_lsbi(INFOHEADER * infoheader, FILE * fptr, unsigned int * size, int encripted);

#endif //CRIPTO2022_BMPREADER_H
