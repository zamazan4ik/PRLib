/*
 * Module: pbmtools
 *
 * Functions for reading/writing .pbm files and pixel
 * manipulation.
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "marklist.h"
#include "pbmtools.h"
#include "utils.h"


/*
Frees the memory used in the bitmap structure.
usage: pbm_free(&bitmap);
Sets bitmap to be NULL on return.
*/
void pbm_free(Pixel*** bitmap, int rows)
{
/*  int r;*/
    if (rows == 0)
    { rows++; }

    FREE_2D(*bitmap, rows);

/*  for(r=0;r<rows;r++)
		free((*bitmap)[r]);
  free(*bitmap);*/
    (*bitmap) = NULL;
}


/*
This function allocates the bitmap.
usage: bitmap=pbm_alloc(cols,rows);
*/
Pixel** pbm_alloc(int cols, int rows)
{
    Pixel** p;
    int newc = (cols + PBITS) / PBITS;

    if (newc == 0)
    { newc++; }
    if (rows == 0)
    { rows++; }

    if (rows < 0)
    { fprintf(stderr, "error: rows=%d\n", rows); }

    CALLOC_2D(p, rows, newc, Pixel);

    return p;
}


Pixel** pbm_copy(Pixel** bitmap, int cols, int rows)
{
    Pixel** p;
    int newc = (cols + PBITS) / PBITS;
    int x, y;

    if ((cols == 0) || (rows == 0))
    { return NULL; }

    p = pbm_alloc(cols, rows);
    for (x = 0; x < newc; x++)
    {
        for (y = 0; y < rows; y++)
        {
            p[y][x] = bitmap[y][x];
        }
    }
    return p;
}
