//
// Created by santi on 6/2/2022.
//

#include "bmpreader.h"

/*
   Sample program to read a limited number of BMP file types
   Write out lots of diagnostics
   Write out a 24 bit RAW image file
*/

#define TRUE  1
#define FALSE 0

int ReadUShort(FILE *,unsigned short *,int);
int ReadUInt(FILE *,unsigned int *,int);

void readHeader(HEADER * header, INFOHEADER * infoheader, FILE * fptr) {
    COLOURINDEX colourindex[256];

    /* Read and check the header */
    ReadUShort(fptr, &header->type, FALSE);
    fprintf(stderr, "ID is: %d, should be %d\n", header->type, 'M' * 256 + 'B');
    ReadUInt(fptr, &header->size, FALSE);
    fprintf(stderr, "File size is %d bytes\n", header->size);
    ReadUShort(fptr, &header->reserved1, FALSE);
    ReadUShort(fptr, &header->reserved2, FALSE);
    ReadUInt(fptr, &header->offset, FALSE);
    fprintf(stderr, "Offset to image data is %d bytes\n", header->offset);

    /* Read and check the information header */
    if (fread(infoheader, sizeof(INFOHEADER), 1, fptr) != 1) {
        fprintf(stderr, "Failed to read BMP info header\n");
        exit(-1);
    }
    fprintf(stderr, "Image size = %d x %d\n", infoheader->width, infoheader->height);
    fprintf(stderr, "Number of colour planes is %d\n", infoheader->planes);
    fprintf(stderr, "Bits per pixel is %d\n", infoheader->bits);
    fprintf(stderr, "Compression type is %d\n", infoheader->compression);
    fprintf(stderr, "Number of colours is %d\n", infoheader->ncolours);
    fprintf(stderr, "Number of required colours is %d\n", infoheader->importantcolours);
}

/* Read the image */
void set_bmp_lsb1(INFOHEADER * infoheader, FILE * in, FILE * fptr, FILE * out, const char * ext) {
    int i, j, k, size, p=3, b=7, ep=0, inflag = 0, endflag = 0;
    unsigned char c, t, bit;
    int ext_length = strlen(ext);

    fseek(in, 0L, SEEK_END);
    size = ftell(in);
    rewind(in);

    if (8*(4+size+ext_length+1) > infoheader->imagesize) {
        printf("El archivo bmp no puede albergar el archivo a ocultar completamente\n");
        return;
    }

    unsigned char * chars = (unsigned char *)&size;

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }
                if (endflag != 1) {
                    if (p >= 0) {
                        bit = (chars[p] >> b) & 0x1;
                        if (--b < 0) {
                            b = 7;
                            p--;
                        }
                    } else if (inflag != 1) {
                        if (b == 7) {
                            if (fread(&t, sizeof(unsigned char), 1, in) != 1) {
                                inflag = 1;
                                t = '.';
                            }
                        }
                        bit = (t >> b) & 0x1;
                        if (--b < 0) {
                            b = 7;
                        }
                    } else if (ep < ext_length){
                        bit = (ext[ep] >> b) & 0x1;
                        if (--b < 0) {
                            b = 7;
                            ep++;
                        }
                    } else {
                        bit = 0;
                        if (--b < 0) {
                            endflag = 1;
                        }
                    }
                    c = ((c >> 1) << 1) | bit;
                }
                fwrite(&c, 1, 1, out);
            } /* k */
        } /* i */
    } /* j */
}

void set_out_lsb1(INFOHEADER * infoheader, FILE * fptr, FILE * out) {
    int i, j, k, p=3, b=7, size_c = 0, ep=0;
    int * size;
    unsigned char c, t=0, bit;
    unsigned char chars[4];
    char ext[10]; //TODO ver de dejarlo asi o malloquarlo para q sea extendible

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }

                bit = c & 0x1;
                t = t | bit;

                if (b-- > 0) {
                    t = t << 1;
                } else {
                    b = 7;
                    if (p >= 0) {
                        chars[p] = t;
                        if (--p < 0) {
                            size = (int *) chars;
                            printf("Size: %d\n", *size);
                        }
                    } else if (size_c++ < *size) {
                        fwrite(&t, 1, 1, out);
                    } else {
                        ext[ep++] = t;
                        if (t == 0) {
                            printf("File extension: %s\n", ext);
                            return;
                        }
                    }
                    t = 0;
                }
            } /* k */
        } /* i */
    } /* j */
}

void set_bmp_lsb4(INFOHEADER * infoheader, FILE * in, FILE * fptr, FILE * out, const char * ext) {
    int i, j, k, size, p=3, b=1, ep=0, inflag = 0, endflag = 0;
    unsigned char c, t, bit;
    int ext_length = strlen(ext);

    fseek(in, 0L, SEEK_END);
    size = ftell(in);
    rewind(in);

    if (2*(4+size+ext_length+1) > infoheader->imagesize) {
        printf("El archivo bmp no puede albergar el archivo a ocultar completamente\n");
        return;
    }

    unsigned char * chars = (unsigned char *)&size;

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }
                if (endflag != 1) {
                    if (p >= 0) {
                        bit = (chars[p] >> (b*4)) & 0xF;
                        if (--b < 0) {
                            b = 1;
                            p--;
                        }
                    } else if (inflag != 1) {
                        if (b == 1) {
                            if (fread(&t, sizeof(unsigned char), 1, in) != 1) {
                                inflag = 1;
                                t = '.';
                            }
                        }
                        bit = (t >> (b*4)) & 0xF;
                        if (--b < 0) {
                            b = 1;
                        }
                    } else if (ep < ext_length){
                        bit = (ext[ep] >> (b*4)) & 0xF;
                        if (--b < 0) {
                            b = 1;
                            ep++;
                        }
                    } else {
                        bit = 0;
                        if (--b < 0) {
                            endflag = 1;
                        }
                    }
                    c = ((c >> 4) << 4) | bit;
                }
                fwrite(&c, 1, 1, out);
            } /* k */
        } /* i */
    } /* j */
}

void set_out_lsb4(INFOHEADER * infoheader, FILE * fptr, FILE * out) {
    int i, j, k, p=3, b=1, size_c = 0, ep=0;
    int * size;
    unsigned char c, t=0, bit;
    unsigned char chars[4];
    char ext[10]; //TODO ver de dejarlo asi o malloquarlo para q sea extendible

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }
                bit = c & 0xF;
                t = t | bit;

                if (b-- > 0) {
                    t = t << 4;
                } else {
                    b = 1;
                    if (p >= 0) {
                        chars[p] = t;
                        if (--p < 0) {
                            size = (int *) chars;
                            printf("Size: %d\n", *size);
                        }
                    } else if (size_c++ < *size) {
                        fwrite(&t, 1, 1, out);
                    } else {
                        ext[ep++] = t;
                        if (t == 0) {
                            printf("File extension: %s\n", ext);
                            return;
                        }
                    }
                    t=0;
                }
            } /* k */
        } /* i */
    } /* j */
}

/*
   Read a possibly byte swapped unsigned short integer
*/
int ReadUShort(FILE *fptr,short unsigned *n,int swap)
{
    unsigned char *cptr,tmp;
    if (fread(n,2,1,fptr) != 1) {
        return (FALSE);
    }
    if (swap) {
        cptr = (unsigned char *)n;
        tmp = cptr[0];
        cptr[0] = cptr[1];
        cptr[1] =tmp;
    }
    return(TRUE);
}

/*
   Read a possibly byte swapped unsigned integer
*/
int ReadUInt(FILE *fptr,unsigned int *n,int swap)
{
    unsigned char *cptr,tmp;

    if (fread(n,4,1,fptr) != 1)
        return(FALSE);
    if (swap) {
        cptr = (unsigned char *)n;
        tmp = cptr[0];
        cptr[0] = cptr[3];
        cptr[3] = tmp;
        tmp = cptr[1];
        cptr[1] = cptr[2];
        cptr[2] = tmp;
    }
    return(TRUE);
}