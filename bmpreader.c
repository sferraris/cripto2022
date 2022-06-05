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

void readbmp(const char * p)
{
    int i,j;
    unsigned char r,g,b;
    HEADER header;
    INFOHEADER infoheader;
    COLOURINDEX colourindex[256];
    FILE *fptr;
    FILE *out;

    out = fopen("out.bmp", "wt");

    /* Open file */
    if ((fptr = fopen(p,"r")) == NULL) {
        fprintf(stderr,"Unable to open BMP file \"%s\"\n",p);
        exit(-1);
    }

    /* Read and check the header */
    ReadUShort(fptr,&header.type,FALSE);
    fprintf(stderr,"ID is: %d, should be %d\n",header.type,'M'*256+'B');
    ReadUInt(fptr,&header.size,FALSE);
    fprintf(stderr,"File size is %d bytes\n",header.size);
    ReadUShort(fptr,&header.reserved1,FALSE);
    ReadUShort(fptr,&header.reserved2,FALSE);
    ReadUInt(fptr,&header.offset,FALSE);
    fprintf(stderr,"Offset to image data is %d bytes\n",header.offset);

    /* Read and check the information header */
    if (fread(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
        fprintf(stderr,"Failed to read BMP info header\n");
        exit(-1);
    }
    fprintf(stderr,"Image size = %d x %d\n",infoheader.width,infoheader.height);
    fprintf(stderr,"Number of colour planes is %d\n",infoheader.planes);
    fprintf(stderr,"Bits per pixel is %d\n",infoheader.bits);
    fprintf(stderr,"Compression type is %d\n",infoheader.compression);
    fprintf(stderr,"Number of colours is %d\n",infoheader.ncolours);
    fprintf(stderr,"Number of required colours is %d\n",
            infoheader.importantcolours);

    /* Read the lookup table if there is one */
    for (i=0;i<255;i++) {
        colourindex[i].r = rand() % 256;
        colourindex[i].g = rand() % 256;
        colourindex[i].b = rand() % 256;
        colourindex[i].junk = rand() % 256;
    }
    if (infoheader.ncolours > 0) {
        for (i=0;i<infoheader.ncolours;i++) {
            if (fread(&colourindex[i].b,sizeof(unsigned char),1,fptr) != 1) {
                fprintf(stderr,"Image read failed\n");
                exit(-1);
            }
            if (fread(&colourindex[i].g,sizeof(unsigned char),1,fptr) != 1) {
                fprintf(stderr,"Image read failed\n");
                exit(-1);
            }
            if (fread(&colourindex[i].r,sizeof(unsigned char),1,fptr) != 1) {
                fprintf(stderr,"Image read failed\n");
                exit(-1);
            }
            if (fread(&colourindex[i].junk,sizeof(unsigned char),1,fptr) != 1) {
                fprintf(stderr,"Image read failed\n");
                exit(-1);
            }
            fprintf(stderr,"%3d\t%3d\t%3d\t%3d\n",i,
                    colourindex[i].r,colourindex[i].g,colourindex[i].b);
        }
    }

//    printf("%X\n", header.type);
//    printf("%X\n", header.size);
    fwrite(&header.type, sizeof (unsigned short int), 1, out);
    fwrite(&header.size, sizeof (unsigned int), 1, out);
    fwrite(&header.reserved1, sizeof (unsigned short int), 1, out);
    fwrite(&header.reserved2, sizeof (unsigned short int), 1, out);
    fwrite(&header.offset, sizeof (unsigned int), 1, out);
//    fclose(out);
//    return;
    fwrite(&infoheader, sizeof (INFOHEADER), 1, out);
    /* Seek to the start of the image data */
    fseek(fptr,header.offset,SEEK_SET);
    fseek(out,header.offset,SEEK_SET);

    /* Read the image */
    for (j=0;j<infoheader.height;j++) {
        for (i=0;i<infoheader.width;i++) {
            if (fread(&b,sizeof(unsigned char),1,fptr) != 1) {
                fprintf(stderr,"Image read failed\n");
                exit(-1);
            }
            if (fread(&g,sizeof(unsigned char),1,fptr) != 1) {
                fprintf(stderr,"Image read failed\n");
                exit(-1);
            }
            if (fread(&r,sizeof(unsigned char),1,fptr) != 1) {
                fprintf(stderr,"Image read failed\n");
                exit(-1);
            }
            fwrite(&b, 1, 1, out);
            fwrite(&g, 1, 1, out);
            fwrite(&r, 1, 1, out);
        } /* i */
    } /* j */

    fclose(fptr);
    fclose(out);
}

/*
   Read a possibly byte swapped unsigned short integer
*/
int ReadUShort(FILE *fptr,short unsigned *n,int swap)
{
    unsigned char *cptr,tmp;

    if (fread(n,2,1,fptr) != 1)
        return(FALSE);
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