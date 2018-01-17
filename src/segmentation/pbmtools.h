#ifndef __PBMTOOLS_H
#define __PBMTOOLS_H

#include "utils.h"

#include <stdio.h>
#include <sys/types.h>

typedef unsigned long Pixel;
typedef Pixel Gray;


/* 2^PSH = PBITS, ie. 2^5 = 32 */
#define PBITS 32
#define PSH 5


/* The next two are ordered pixels col0=left, cols31=right */

/*
#define pbm_getpixel(bitmap,c,r)  \
        ((bitmap[r][(c)>>PSH] >> ((PBITS-1)-((c)&(PBITS-1))))&1)
#define pbm_putpixel(bitmap,c,r,val) \
  if((val))  \
         bitmap[r][(c)>>PSH] |=  (((unsigned)1<<(PBITS-1)) >> ((c)&(PBITS-1))); \
  else \
         bitmap[r][(c)>>PSH] &= ~(((unsigned)1<<(PBITS-1)) >> ((c)&(PBITS-1))) 
*/


 

/* The next two defines are ordered pixel cols0=right, cols31=left */

#define pbm_getpixel(bitmap,c,r)                            \
         ((bitmap[r][(c)>>PSH] >> ((c)&(PBITS-1)))&1)
#define pbm_putpixel(bitmap,c,r,val)                        \
         if((val))                                          \
              bitmap[r][(c)>>PSH] |= (1<<((c)&(PBITS-1)));  \
         else                                               \
              bitmap[r][(c)>>PSH] &= ~(1<<((c)&(PBITS-1)))


#define pbm_putpixel_trunc(bitmap,c,r,val,cols,rows) \
       if(((c)>=0)&&((c)<cols)&&((r)>=0)&&((r)<rows)) \
             pbm_putpixel(bitmap,c,r,val)

#define pbm_getpixel_trunc(bitmap,c,r,cols,rows)       \
       ((((c)>=0)&&((c)<cols)&&((r)>=0)&&((r)<rows)) ?  \
             pbm_getpixel(bitmap,c,r) : 0)
       



int    pbm_isapbmfile(FILE *fp);
void   pbm_free(Pixel ***bitmap,int rows);
Pixel  **pbm_copy(Pixel **bitmap, int cols, int rows);
Pixel  **pbm_alloc(int cols, int rows);
Pixel **pbm_readfile(FILE *fp,int *cols, int *rows, short *resolution);
/*Pixel  **pbm_readfile(FILE *fp,int *cols, int *rows);*/
void   pbm_writefile(FILE *fp,Pixel **bitmap,int cols, int rows, int resolution);

void   pbm_putpixel_range(Pixel **bitmap,int c,int r,int val, int cols, int rows);
int    pbm_getpixel_range(Pixel **bitmap,int c, int r, int cols, int rows);

void pbm_clear(Pixel **bitmap, int cols, int rows); 

#define pgm_getpixel(image,x,y) image[y][x]
#define pgm_putpixel(image,x,y,val) image[y][x]=val


int    **pgm_alloc(int rows, int cols);
void   pgm_free(int ***image,int rows);
void   pgm_write(FILE *fp, int **image, int nc, int nr, int maxval);
int    **pgm_read(FILE *fp, int *nc, int *nr, int *maxval);
#endif
