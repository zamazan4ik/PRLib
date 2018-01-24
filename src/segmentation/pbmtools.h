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
       



void   pbm_free(Pixel ***bitmap,int rows);
Pixel  **pbm_copy(Pixel **bitmap, int cols, int rows);
Pixel  **pbm_alloc(int cols, int rows);

#endif
