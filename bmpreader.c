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
    /* Read and check the header */
    ReadUShort(fptr, &header->type, FALSE);
    ReadUInt(fptr, &header->size, FALSE);
    ReadUShort(fptr, &header->reserved1, FALSE);
    ReadUShort(fptr, &header->reserved2, FALSE);
    ReadUInt(fptr, &header->offset, FALSE);

    /* Read and check the information header */
    if (fread(infoheader, sizeof(INFOHEADER), 1, fptr) != 1) {
        fprintf(stderr, "Failed to read BMP info header\n");
        exit(-1);
    }
}

/* Read the image */
void set_bmp_lsb1(INFOHEADER * infoheader, FILE * fptr, FILE * out, const unsigned char * in_text, unsigned int size) {
    int i, j, k, cp=0, b=7;
    unsigned char c, bit;

    int text_size = 0;
    text_size = text_size | in_text[0];
    text_size = (text_size << 8) | in_text[1];
    text_size = (text_size << 8) | in_text[2];
    text_size = (text_size << 8) | in_text[3];

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0;k<3;k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }
                if (cp<size) {
                    bit = (in_text[cp] >> b) & 0x1;
                    if (--b < 0) {
                        b = 7;
                        cp++;
                    }
                    c = ((c >> 1) << 1) | bit;
                }
                fwrite(&c, 1, 1, out);
            } /* k */
        } /* i */
    } /* j */
}

void set_out_lsb1(INFOHEADER * infoheader, FILE * fptr, unsigned char * out, int * size) {
    int i, j, k, p=3, b=7, size_c = 0;
    unsigned char c, t=0, bit;
    unsigned char chars[4];
    int * aux_size;

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
                            aux_size = (int *) chars;
                            *size = *aux_size;
                        }
                    } else if (size_c < *size || t != 0) {
                        out[size_c++] = t;
                    } else {
                        out[size_c] = 0;
                        return;
                    }
                    t = 0;
                }
            } /* k */
        } /* i */
    } /* j */
}

void set_bmp_lsb4(INFOHEADER * infoheader, FILE * fptr, FILE * out, const unsigned char * in_text, unsigned int size) {
    int i, j, k, p=3, cp=0, b=1, ep=0, inflag = 0, endflag = 0;
    unsigned char c, t, bit;

    unsigned char * chars = (unsigned char *)&size;

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }
                if (cp<size) {
                    bit = (in_text[cp] >> (b*4)) & 0xF;
                    if (--b < 0) {
                        b = 1;
                        cp++;
                    }
                    c = ((c >> 4) << 4) | bit;
                }
                fwrite(&c, 1, 1, out);
            } /* k */
        } /* i */
    } /* j */
}

void set_out_lsb4(INFOHEADER * infoheader, FILE * fptr, unsigned char * out, int * size) {
    int i, j, k, p=3, b=1, size_c = 0;
    unsigned char c, t=0, bit;
    unsigned char chars[4];
    int * aux_size;

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
                            aux_size = (int *) chars;
                            *size = *aux_size;
                        }
                    } else if (size_c < *size || t != 0) {
                        out[size_c++] = t;
                    } else {
                        out[size_c] = 0;
                        return;
                    }
                    t=0;
                }
            } /* k */
        } /* i */
    } /* j */
}

void set_bmp_lsbi(INFOHEADER * infoheader, FILE * fptr, FILE * out, const unsigned char * in_text, unsigned int size, HEADER * header) {
    int i, j, k, b=7, pp=0, cp=0;
    unsigned char c, bit, original_bit, stbits;
    int changed[4] = {0, 0, 0, 0};
    int not_changed[4] = {0, 0, 0, 0};
    int pattern[4];

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }
                if (cp<size) {
                    bit = (in_text[cp] >> b) & 0x1;
                    if (--b < 0) {
                        b = 7;
                        cp++;
                    }
                    stbits = (c >> 1) & 0x3;
                    original_bit = c & 0x1;
                    if (original_bit == bit) {
                        not_changed[stbits]++;
                    } else {
                        changed[stbits]++;
                    }
                } else {
                    goto snd_part;
                }
            } /* k */
        } /* i */
    } /* j */
    snd_part:
    for (k=0; k<4; k++) {
        pattern[k] = (changed[k] > not_changed[k]) ? 1 : 0;
    }

    fseek(fptr, header->offset, SEEK_SET);
    cp = 0;

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }
                if (cp<size) {
                    if (pp < 4) {
                        bit = pattern[pp++];
                    } else {
                        bit = (in_text[cp] >> b) & 0x1;
                        if (--b < 0) {
                            b = 7;
                            cp++;
                        }
                        c = ((c >> 1) << 1) | bit;
                        stbits = (c >> 1) & 0x3;
                        if (pattern[stbits] == 1) {
                            bit = (~bit) & 0x1;
                        }
                    }
                    c = ((c >> 1) << 1) | bit;
                }
                fwrite(&c, 1, 1, out);
            } /* k */
        } /* i */
    } /* j */
}

void set_out_lsbi(INFOHEADER * infoheader, FILE * fptr, unsigned char * out, int * size) {
    int i, j, k, p=3, b=7, size_c=0, pp=0;
    unsigned char c, t=0, bit, stbits;
    unsigned char chars[4];
    int pattern[4];
    int * aux_size;

    for (j=0;j<infoheader->height;j++) {
        for (i=0;i<infoheader->width;i++) {
            for (k=0; k<3; k++) {
                if (fread(&c, sizeof(unsigned char), 1, fptr) != 1) {
                    fprintf(stderr, "Image read failed\n");
                    exit(-1);
                }

                bit = c & 0x1;

                if (pp < 4) {
                    pattern[pp++] = bit;
                } else {
                    stbits = (c >> 1) & 0x3;
                    if (pattern[stbits] == 1) {
                        bit = (~bit) & 0x1;
                    }
                    t = t | bit;

                    if (b-- > 0) {
                        t = t << 1;
                    } else {
                        b = 7;
                        if (p >= 0) {
                            chars[p] = t;
                            if (--p < 0) {
                                aux_size = (int *) chars;
                                *size = *aux_size;
                            }
                        } else if (size_c < *size || t != 0) {
                            out[size_c++] = t;
                        } else {
                            out[size_c] = 0;
                            return;
                        }
                        t = 0;
                    }
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