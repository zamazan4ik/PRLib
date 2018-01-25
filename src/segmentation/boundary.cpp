/*
 * Module: boundary
 *
 * This module implements a collection of boundary following and
 * mark extraction routines.  The marks can be extracted from a
 * bitmap, either as 4 or 8 connected and either nested or
 * non-nested.
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998
 *
 */
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "marklist.h"
#include "pbmtools.h"
#include "boundary.h"

#define boundary4_x(x) ((x)==0?1:((x)==2?-1 : 0))
#define boundary4_y(x) ((x)==1?1:((x)==3?-1 : 0))

#define boundary8_x(x) ((((x)==7)||((x)==0)||((x)==1))?1:((((x)==3)||((x)==4)||((x)==5))?-1 : 0))
#define boundary8_y(x) ((((x)==1)||((x)==2)||((x)==3))?1:((((x)==5)||((x)==6)||((x)==7))?-1 : 0))

int g_x1 = 0;
int g_y1 = 0;
int g_x2 = 0;
int g_y2 = 0;


const int REALLOCINC = 128;

/*
 *
 *
 * lrboundary()
 *
 *
 */

void lrboundary_init(lrboundarytype* b)
{
    if (b)
    {
        b->len = 0;
        b->l = nullptr;
        b->r = nullptr;
        b->y = nullptr;
    }
}

void lrboundary_free(lrboundarytype* b)
{
    if (b)
    {
        if (b->l)
        { free(b->l); }
        if (b->r)
        { free(b->r); }
        if (b->y)
        { free(b->y); }
        lrboundary_init(b);
    }
}

void lrboundary_add(lrboundarytype* b, const int y, const int l, const int r)
{
    assert(b);

    if ((b->len % REALLOCINC) == 0)
    {
        b->l = (int*) realloc(b->l, sizeof(int) * (b->len + REALLOCINC));
        assert(b->l);
        b->r = (int*) realloc(b->r, sizeof(int) * (b->len + REALLOCINC));
        assert(b->r);
        b->y = (int*) realloc(b->y, sizeof(int) * (b->len + REALLOCINC));
        assert(b->y);
    }

    b->l[b->len] = l;
    b->r[b->len] = r;
    b->y[b->len] = y;

    b->len++;
}


void boundary_init(boundarytype* b)
{
    if (b)
    {
        b->len = 0;
        b->x = nullptr;
        b->y = nullptr;
    }
}

void boundary_free(boundarytype* b)
{
    if (b)
    {
        if (b->x)
        { free(b->x); }
        if (b->y)
        { free(b->y); }
        boundary_init(b);
    }
}

void boundary_bounds(boundarytype* b, int* xl, int* xr, int* yt, int* yb)
{
    int i;

    assert(b);
    assert(xl);
    assert(xr);
    assert(yt);
    assert(yb);

    for (i = 0; i < b->len; i++)
    {
        if (i == 0)
        {
            *xl = *xr = b->x[i];
            *yt = *yb = b->y[i];
        }
        else
        {
            if (b->x[i] < *xl)
            {
                *xl = b->x[i];
            }
            else if (b->x[i] > *xr)
            {
                *xr = b->x[i];
            }

            if (b->y[i] < *yt)
            {
                *yt = b->y[i];
            }
            else if (b->y[i] > *yb)
            {
                *yb = b->y[i];
            }
        }
    }
}

void boundary_start(boundarytype* b, int* startx, int* starty)
{
    assert(b);
    assert(startx);
    assert(starty);

    if (b->len > 0)
    {
        *startx = b->x[0];
        *starty = b->y[0];
    }
    else
    {
        *startx = *starty = 0;
    }
}

void boundary_add(boundarytype* b, const int x, const int y)
{
    assert(b);

    if ((b->len % REALLOCINC) == 0)
    {
        b->x = (int*) realloc(b->x, sizeof(int) * (b->len + REALLOCINC));
        assert(b->x);
        b->y = (int*) realloc(b->y, sizeof(int) * (b->len + REALLOCINC));
        assert(b->y);
    }

    b->x[b->len] = x;
    b->y[b->len] = y;

    b->len++;
}


void boundary_trace_inside8(const marktype* image,
                            boundarytype* b,
                            int i,
                            int j)
{
    int startx, starty, startdir;
    int dir;
    int found, t, tempdir;
    unsigned int pix, pix2;
    int first = 1;
    int tx, ty;

    assert(image);
    assert(b);
    assert((i >= 0) && (i < image->w));
    assert((j >= 0) && (j < image->h));

    boundary_init(b);

    do
    {
        pix = gpix((*image), i, j);
        pix2 = gpix((*image), i - 1, j);
        if (pix2 == 1)
        {
            i--;
        }
    }
    while (pix2 == 1);

    dir = 4;

    for (tempdir = dir, t = 0; t < 8; tempdir = (tempdir + 1) % 8, t++)
    {
        tx = i + boundary8_x(tempdir);
        ty = j + boundary8_y(tempdir);
        if (gpix ((*image), tx, ty) == pix)
        {
            dir = tempdir;
            break;
        }
    }


    startdir = dir;
    startx = i;
    starty = j;


    if ((gpix((*image), i, j - 1)) && (gpix((*image), i - 1, j - 1)))
    {
        dir = 7;
        fprintf(stderr, "dir=7\n");
    }



    /* start direction 0, +ve increments are clockwise adjustments*/
    pix = gpix((*image), startx, starty);
    while (1)
    {
        /*
         * if we were outside the component:
         * tempdir=(dir+2)%8, and tempdir=(tempdir+7)%8
         */
        found = 0;
        for (tempdir = (dir + 6) % 8, t = 0; t < 8; tempdir = (tempdir + 1) % 8, t++)
        {
            tx = i + boundary8_x(tempdir);
            ty = j + boundary8_y(tempdir);
            if (gpix ((*image), tx, ty) == pix)
            {
                dir = tempdir;
                found = 1;
                break;
            }
        }
        if (first == 1)
        {
            startdir = dir;
        }

        boundary_add(b, i, j);

        if ((found == 0) ||
            ((first == 0) && (i == startx) && (j == starty) && (dir == startdir))
                )
        {
            break;
        }

        first = 0;

        i += boundary8_x (dir);
        j += boundary8_y (dir);
    }
}


void boundary_trace_inside4(const marktype* image,
                            boundarytype* b,
                            int i,
                            int j)
{
    int startx, starty, startdir;
    int dir;
    int found, t, tempdir;
    unsigned int pix, pix2;
    int first = 1;
    int tx, ty;

    assert(image);
    assert(b);
    assert((i >= 0) && (i < image->w));
    assert((j >= 0) && (j < image->h));

    boundary_init(b);

    do
    {
        pix = gpix((*image), i, j);
        pix2 = gpix((*image), i - 1, j);
        if (pix2 == 1)
        {
            i--;
        }
    }
    while (pix2 == 1);


    dir = 0;
    startdir = dir;
    startx = i;
    starty = j;
    /* start direction 0, +ve increments are clockwise adjustments*/
    pix = gpix((*image), startx, starty);
    while (1)
    {
        /*
         * if we were outside the component:
         * tempdir=(dir+2)%8, and tempdir=(tempdir+7)%8
         */
        found = 0;
        for (tempdir = (dir + 3) % 4, t = 0; t < 4; tempdir = (tempdir + 1) % 4, t++)
        {
            tx = i + boundary4_x(tempdir);
            ty = j + boundary4_y(tempdir);
            if (gpix ((*image), tx, ty) == pix)
            {
                dir = tempdir;
                found = 1;
                break;
            }
        }
        if (first == 1)
        {
            startdir = dir;
        }
        boundary_add(b, i, j);

        if ((found == 0) ||
            ((first == 0) && (i == startx) && (j == starty) && (dir == startdir))
                )
        {
            break;
        }

        first = 0;

        i += boundary4_x (dir);
        j += boundary4_y (dir);
    }
}


void boundary_max_lr(const boundarytype* b, lrboundarytype* lrb)
{
    int i, j, miny = 0, maxy = 0, l, r;

    lrboundary_init(lrb);
    for (i = 0; i < b->len; i++)
    {
        if ((i == 0) || (b->y[i] < miny))
        {
            miny = b->y[i];
        }
        if ((i == 0) || (b->y[i] > maxy))
        {
            maxy = b->y[i];
        }
    }

    for (j = miny; j <= maxy; j++)
    {
        l = r = -1;
        for (i = 0; i < b->len; i++)
        {
            if (b->y[i] == j)
            {
                if ((l < 0) || (b->x[i] < l))
                {
                    l = b->x[i];
                }
                if ((r < 0) || (b->x[i] > r))
                {
                    r = b->x[i];
                }
            }
        }
        lrboundary_add(lrb, j, l, r);
    }
}


int next_pixel(const marktype* image, int* x, int* y)
{
    if ((image == nullptr) || (image->w <= 0) || (image->h <= 0))
    {
        return 1;
    }

    assert(x && y);
    assert((*x >= 0) && (*y >= 0) && (*x < image->w) && (*y < image->h));

    /*
     * find the next pixel
     */
    while (gpix ((*image), *x, *y) == 0)
    {
        (*x)++;
        if (*x >= image->w)
        {
            *x = 0;
            (*y)++;
            if (*y >= image->h)
            {
                /*
                 * reset the counters to 0 at the end of the image
                 */
                *x = *y = 0;
                return 1;
            }
        }
    }
    return 0;
}



marktype boundary_extract_nested(marktype* image, int x, int y, int conn)
{
    marktype d;
    int xl, xr, yt, yb;
    int startx, starty;
    boundarytype b;

    assert(image);
    assert((x >= 0) && (y >= 0));
    assert((conn == 4) || (conn == 8));

    if (conn == 8)
    {
        boundary_trace_inside8(image, &b, x, y);
    }
    else
    {
        boundary_trace_inside4(image, &b, x, y);
    }

    boundary_bounds(&b, &xl, &xr, &yt, &yb);

    marktype_alloc(&d, xr - xl + 1, yb - yt + 1);
    d.imagew = image->imagew;
    d.imageh = image->imageh;
    d.xpos = xl;
    d.ypos = yt;

    boundary_start(&b, &startx, &starty);

    if (conn == 8)
    {
        marktype_fillextract8(*image, &d, startx, starty, xl, yt, 1, 0);
    }
    else
    {
        marktype_fillextract4(*image, &d, startx, starty, xl, yt, 1, 0);
    }
    marktype_calc_centroid(&d);
    marktype_area(&d);
    boundary_free(&b);
    return d;
}


marktype boundary_extract_non_nested(marktype* image, int x, int y, int conn)
{
    marktype d;
    int xl, xr, yt, yb;
    lrboundarytype lrb;
    boundarytype b;
    int p;

    assert(image);
    assert((x >= 0) && (y >= 0));
    assert((conn == 4) || (conn == 8));

    if (conn == 8)
    {
        boundary_trace_inside8(image, &b, x, y);
    }
    else
    {
        boundary_trace_inside4(image, &b, x, y);
    }
    boundary_bounds(&b, &xl, &xr, &yt, &yb);

    boundary_max_lr(&b, &lrb);

    marktype_alloc(&d, xr - xl + 1, yb - yt + 1);
    d.imagew = image->imagew;
    d.imageh = image->imageh;
    d.xpos = xl;
    d.ypos = yt;

    for (int i = 0; i < lrb.len; i++)
    {
        for (int l = lrb.l[i]; l <= lrb.r[i]; l++)
        {
            p = pbm_getpixel (image->bitmap, l, lrb.y[i]);
            pbm_putpixel(d.bitmap, l - xl, lrb.y[i] - yt, p);
            if (p)
                pbm_putpixel(image->bitmap, l, lrb.y[i], 0);
        }
    }

    marktype_calc_centroid(&d);
    marktype_area(&d);

    lrboundary_free(&lrb);
    boundary_free(&b);
    return d;
}

marklistptr extract_all_marks(marklistptr list, marktype image, int nested, int conn)
{
    int x, y, sym;
    marklistptr step = list;

    assert((nested == 0) || (nested == 1));
    assert((conn == 4) || (conn == 8));

    g_x2 = image.w - 1;
    g_y2 = image.h - 1;

    x = y = 0;
    sym = 0;
    while ((next_pixel(&image, &x, &y) == 0))
    {
        marktype d;

        if (nested == 1)
        {
            d = boundary_extract_nested(&image, x, y, conn);
        }
        else
        {
            d = boundary_extract_non_nested(&image, x, y, conn);
        }
        d.symnum = sym++;

        if (list == nullptr)
        {
            step = marklist_addcopy(&list, d);
        }
        else
        {
            step = marklist_addcopy(&step, d);
        }

        marktype_free(&d);
    }
    marktype_fill_cleanup();

    return list;
}
