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


void pbm_putpixel_range(Pixel** bitmap, int c, int r, int val, int cols, int rows)
{
    if (((c) >= 0) && ((c) < cols) && ((r) >= 0) && ((r) < rows))
        pbm_putpixel(bitmap, c, r, val);
    else
    {
        fprintf(stderr, "putpitxel (%d %d) outside (%d %d)\n", c, r, cols, rows);
    }
}

int pbm_getpixel_range(Pixel** bitmap, int c, int r, int cols, int rows)
{
    if (((c) >= 0) && ((c) < cols) && ((r) >= 0) && ((r) < rows))
    {
        return pbm_getpixel(bitmap, c, r);
    }
    else
    {
        fprintf(stderr, "getpixel (%d %d) outside (%d %d)\n", c, r, cols, rows);
        return 0;
    }
}


/*
Returns 1 if *fp points to a PBM (magic is P1 or P4) file.
leaves the fp in it's original position.
usage: if(pbm_isapbmfile(fp)) process(fp);
*/
int pbm_isapbmfile(FILE* fp)
{
    unsigned short magic;
    magic = magic_getshort(fp);

    return ((magic == MAGIC_P4) || (magic == MAGIC_P1));
}


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


void pbm_clear(Pixel** bitmap, int cols, int rows)
{
    int newc = (cols + PBITS) / PBITS;
    int x, y;

    for (x = 0; x < newc; x++)
    {
        for (y = 0; y < rows; y++)
        {
            bitmap[y][x] = 0;
        }
    }
}


/*
Reads in a pbm file and returns a pointer to the allocated bitmap, 
in the P4 format.
Assumes fp was opened and keeps it that way.
usage: bitmap=pbm_readfile(fp,&cols,&rows);
*/
Pixel** pbm_readfile(FILE* fp, int* cols, int* rows, short* resolution)
{
    Pixel** p = NULL;
    char* s = NULL;
    unsigned char* buf;
    int r, c, ii = 0;
    int p1 = 0, k;
    int arg;

    *resolution = 0;

    do
    {
        if (s != NULL)
        {
            free(s);
        }
        s = fgoodgets(fp);
    }
    while (s[0] == '#');

    if (strcmp(s, "P1") == 0)
    { p1 = 1; }
    else if (strcmp(s, "P4") == 0)
    { p1 = 0; }
    else
    { error("pbm_readfile", "only pbm files are supported", ""); }

    free(s);
    s = NULL;

    do
    {
        if (s != NULL)
        {
            free(s);
            s = NULL;
        }
        s = fgoodgets(fp);
        if (s)
        {
            if (strncmp(s, "# resolution: ", 14) == 0)
            {
                ii = sscanf(s, "%*s %*s %hd", resolution);
                if (ii != 1)
                { *resolution = 0; }
            }
        }
    }
    while (s[0] == '#');

    arg = sscanf(s, "%d %d", cols, rows);
    free(s);
    s = NULL;

    if (arg == 1)
    {
        s = fgoodgets(fp);
        arg = sscanf(s, "%d", rows);
        if (arg != 1)
        { error("pbm_readfile", "bizzare pbm format", ""); }
        free(s);
        s = NULL;
    }

    p = pbm_alloc(*cols, *rows);
    buf = (unsigned char*) malloc((*cols + 2) * sizeof(unsigned char));
    if (!buf)
    { error("pbm_readfile", "out of memory", ""); }

    if (p1)
    {
        for (r = 0; r < *rows; r++)
        {
            for (c = 0; c < *cols; c++)
            {
                fscanf(fp, "%d", &ii);
                pbm_putpixel(p, c, r, ii);
            }
        }
    }
    else
    {
        for (r = 0; r < *rows; r++)
        {
            c = 0;
            k = fread(buf, 1, (*cols + 7) / 8, fp);
            if (k != (*cols + 7) / 8)
            {
                fprintf(stderr, "*warning* file too short!\n");
            }
            for (c = 0; c < *cols; c++)
            {
                if (!(c % 8))
                { ii = buf[c / 8]; }
                if (ii < 0)
                { error("pbm_readfile", "file reading error", ""); }
                if (ii & 128) pbm_putpixel(p, c, r, 1);
                ii <<= 1;
            }
        }
    } /* p4 type */
    free(buf);
    return p;
}


/*
Writes a pbm file given a file pointer and bitmap. Assumes that
fp is already open, and keeps in that way.
usage: pbm_writefile(fp,bitmap,cols,rows);
*/
void pbm_writefile(FILE* fp, Pixel** bitmap, int cols, int rows, int resolution)
{
    int r, c, ii = 0, j;
    unsigned char* buf;

    buf = (unsigned char*) malloc(((cols + 14) / 8) * sizeof(unsigned char));

    fprintf(fp, "%s\n", "P4");
    if (resolution)
    {
        fprintf(fp, "# resolution: %d\n", resolution);
    }
    fprintf(fp, "%d %d\n", cols, rows);

    if ((cols > 0) && (rows >= 0))
    {
        for (r = 0; r < rows; r++)
        {
            for (c = 0; c < cols; c += 8)
            {
                ii = 0;
                for (j = 0; j < 8; j++)
                {
                    if (c + j < cols)
                    {
                        if (pbm_getpixel(bitmap, c + j, r))
                        { ii |= (128 >> j); }
                    }
                }
                buf[c / 8] = ii;
            }
            fwrite(buf, 1, (cols + 7) / 8, fp);
        }
    }
    free(buf);
    if (ferror(fp))
    {
        fprintf(stderr, "pbm_writefile: error occured while writing file.\n");
        return;
    }
}


/*
Writes a pbm file given a file pointer and bitmap. Assumes that
fp is already open, and keeps in that way.
usage: pbm_writefile(fp,bitmap,cols,rows);
*/
void pbm_writefile_xy(FILE* fp, Pixel** bitmap, int x, int y, int cols, int rows, int resolution)
{
    int r, c, ii = 0, j;
    unsigned char* buf;

    buf = (unsigned char*) malloc(((cols + 14) / 8) * sizeof(unsigned char));

    fprintf(fp, "%s\n", "P4");
    if (resolution)
    {
        fprintf(fp, "# resolution: %d\n", resolution);
    }
    fprintf(fp, "%d %d\n", cols, rows);

    if ((cols > 0) && (rows >= 0))
    {
        for (r = 0; r < rows; r++)
        {
            for (c = 0; c < cols; c += 8)
            {
                ii = 0;
                for (j = 0; j < 8; j++)
                {
                    if (c + j < cols)
                    {
                        if (pbm_getpixel(bitmap, c + j, r))
                        { ii |= (128 >> j); }
                    }
                }
                buf[c / 8] = ii;
            }
            fwrite(buf, 1, (cols + 7) / 8, fp);
        }
    }
    free(buf);
    if (ferror(fp))
    {
        fprintf(stderr, "pbm_writefile: error occured while writing file.\n");
        return;
    }
}


/* works best is cols is >= rows, and iterate x faster than y */
int** pgm_alloc(int cols, int rows)
{
    int** p;
/*  int i;*/

    CALLOC_2D(p, rows, cols, int);

    return p;
}


void pgm_free(int*** image, int rows)
{
/*  int r;*/

    FREE_2D(*image, rows);

/*
  for(r=0;r<rows;r++)
		free((*image)[r]);
  free(*image);
*/
    (*image) = NULL;
}


int** pgm_read(FILE* fp, int* nc, int* nr, int* maxval)
{
    int i, j;
    char* s = NULL;
    int ii = 0;
    int arg;
    int** pp;
    int p2 = 0;

    do
    {
        if (s)
        { free(s); }
        s = fgoodgets(fp);
    }
    while (s[0] == '#');

    if ((strcmp(s, "P5") != 0) && (strcmp(s, "P2") != 0))
    {
        error("pgm_read", "only pgm P5/P2 files are supported", "");
    }

    if (strcmp(s, "P2") == 0)
    {
        p2 = 1;
    }

    free(s);
    s = NULL;

    do
    {
        if (s)
        { free(s); }
        s = fgoodgets(fp);
    }
    while (s[0] == '#');

    arg = sscanf(s, "%d %d", nc, nr);
    free(s);
    s = NULL;

    do
    {
        if (s)
        { free(s); }
        s = fgoodgets(fp);
    }
    while (s[0] == '#');

    arg = sscanf(s, "%d", maxval);
    free(s);
    s = NULL;

    pp = pgm_alloc(*nc, *nr);

    for (j = 0; j < *nr; j++)
    {
        for (i = 0; i < *nc; i++)
        {
            if (p2 == 0)
            {
                ii = fgetc(fp);
            }
            else
            {
                fscanf(fp, "%d", &ii);
            }
            pp[j][i] = ii;
        }
    }
    return pp;
}


void pgm_write(FILE* fp, int** image, int nc, int nr, int maxval)
{
    int x, y, c;
    int p2;


    if (maxval <= 255)
    { /* go into P5 mode */
        p2 = 0;
        fprintf(fp, "P5\n");
        fprintf(fp, "%d %d\n%d\n", nc, nr, maxval);
    }
    else
    {
        p2 = 1;
        fprintf(fp, "P2\n");
        fprintf(fp, "%d %d\n%d\n", nc, nr, maxval);
    }

    c = 0;
    for (y = 0; y < nr; y++)
    {
        for (x = 0; x < nc; x++)
        {
            if (p2 == 1)
            {
                fprintf(fp, "%d ", pgm_getpixel(image, x, y));
            }
            else
            {
                fputc(pgm_getpixel(image, x, y), fp);
            }
        }
        if (p2 == 1)
        {
            fprintf(fp, "\n");
        }
    }
    fclose(fp);
}
