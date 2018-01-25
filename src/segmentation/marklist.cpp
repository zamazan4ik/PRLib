/*
 * Module: marklist
 *
 * The marklist module implements the routines dealing with 
 * the marktype structure, or a list of the structures called
 * a marklist. 
 *
 * Functions include reading/writing, features, and list 
 * management.
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998 
 *
 */

#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "marklist.h"
#include "pbmtools.h"
#include "utils.h"


/* 
 * marktype_copy, make a completely new copy, creates a 
 * new bitmap as well.
 */
marktype marktype_copy(marktype d)
{
    marktype d2;

    d2 = d;
    d2.bitmap = pbm_copy(d.bitmap, d2.w, d2.h);
    if (d.name)
    {
        d2.name = strdup(d.name);
    }
    return d2;
}


void marktype_init(marktype* d)
{
    d->symnum = 0;
    d->xpos = d->ypos = 0;
    d->w = d->h = 0;
    d->xcen = d->ycen = 0;
    d->d_xcen = d->d_ycen = 0;
    d->xoffset = d->yoffset = 0;
    d->baseline = 0;
    d->topline = 0;
    d->name = nullptr;
    d->bitmap = nullptr;
    d->resolution = 0;
    d->imagew = d->imageh = 0;
    d->h_runs = d->v_runs = 0;

    d->start_of_line = 0;
    d->grp = -1;
    d->skew_of_line = 0;
    d->ypos_of_line = 0;

    d->set = 0;
}


/*
 * marktype_alloc, initialises all values in the structure,
 * and creates a bitmap of size w by h.
 *
 * the w and h values are set.
 */
void marktype_alloc(marktype* d, int w, int h)
{
    marktype_init(d);
    d->bitmap = pbm_alloc(w, h);
    d->w = w;
    d->imagew = w;
    d->h = h;
    d->imageh = h;
}


void marktype_free(marktype* d)
{
    if (d->bitmap)
    {
        pbm_free(&(d->bitmap), d->h);
    }
    if (d->name)
    {
        free(d->name);
    }
    marktype_init(d);
}


/*
 * marktype_calc_centroid, uses integers and keeps scaling to fix down, 
 * (meaning the result is not always correct!)
 * 
 * If there are no set pixels the centroid values are set to -1.
 */
void marktype_calc_centroid(marktype* b)
{
    int r, c;
    int count = 0;
    int xtot = 0, ytot = 0;

    for (r = 0; r < b->h; r++)
    {
        for (c = 0; c < b->w; c++)
        {
            if (pbm_getpixel (b->bitmap, c, r))
            {
                count++;
                xtot += c;
                ytot += r;
            }
        }
        if (xtot + ytot >= (INT_MAX / 2))
        {
            count = (count + 1) / 2;
            xtot = (xtot + 1) / 2;
            ytot = (ytot + 1) / 2;
        }
    }

    b->xcen = ROUND(xtot / (double) count);
    b->ycen = ROUND(ytot / (double) count);
    b->d_xcen = (float) (xtot / (double) count);
    b->d_ycen = (float) (ytot / (double) count);

}


/*
 * marktype_area, calculates the number of set pixels, as with all these
 * functions we assume the the number can fix in an integer.
 */
void marktype_area(marktype* d)
{
    int x, y;
    int set = 0;

    for (y = 0; y < d->h; y++)
    {
        for (x = 0; x < d->w; x++)
        {
            if (pbm_getpixel (d->bitmap, x, y))
            {
                set++;
            }
        }
    }
    d->set = set;
}


/*
 * marklist_add, adds the mark to the list, note that the bitmap is not 
 * duplicated, it is just linked in. NB: Watch for pointer aliasing--use
 * marklist_addcopy, if you don't want aliasing.
 */
marklistptr marklist_add(marklistptr* list, marktype m)
{
    marklistptr p;

    if (*list == nullptr)
    {
        *list = (marklistptr) malloc(sizeof(marklisttype));
        if ((*list) == nullptr)
        {
            //error("marklist_add", "OUT of Memory in marklist_add", "");
        }
        p = *list;
    }
    else
    {
        p = *list;
        while (p->next != nullptr)
        {
            p = p->next;
        }
        p->next = (marklistptr) malloc(sizeof(marklisttype));
        if (p->next == nullptr)
        {
            //error("marklist_add", "OUT of Memory in marklist_add", "");
        }
        p = p->next;
    }
    p->data = m;
    p->next = nullptr;
    return p;

}


/*
 * marklist_addcopy, creates a copy of the mark (creates a new bitmap),
 * and adds to the list.
 */
marklistptr marklist_addcopy(marklistptr* list, marktype m)
{
    marktype d;

    d = marktype_copy(m);
    return marklist_add(list, d);
}

static int MAX_FS = 8192;

static int bottom = 0, top = 0;
typedef struct
{
    unsigned short x, y;
} fillpostype;


fillpostype* filla = nullptr;

#define push(i, j) \
    if(top==MAX_FS){\
         MAX_FS+=8192;\
         REALLOC(filla,MAX_FS,fillpostype);}\
    filla[top].x=i;\
    filla[top].y=j;\
    top=(top+1);

#define pop(i, j) \
    top=(top-1);\
    i=filla[top].x;\
    j=filla[top].y


void marktype_fill_cleanup(void)
{
    if (filla)
    {
        free(filla);
    }
    filla = nullptr;
    MAX_FS = 8192;
}


void marktype_fillextract8(marktype image,
                      marktype* mark,
                      int i, int j, int xl, int yt,
                      unsigned int lookfor, unsigned int fillcol)
{
    static int l, r, t, start_up, start_down;
    static int m_in, m_ax;

    bottom = top = 0;

    if (filla == nullptr)
    CALLOC (filla, MAX_FS, fillpostype);

    if (pbm_getpixel (image.bitmap, i, j) == fillcol)
    {
        return;
    }
    push (i, j);
    while (top != bottom)
    {
        pop (l, j);
        r = l;

        while ((l - 1 >= 0) && (pbm_getpixel (image.bitmap, l - 1, j) == lookfor))
        {
            l--;
        }
        while ((r + 1 < image.w) && (pbm_getpixel (image.bitmap, r + 1, j) == lookfor))
        {
            r++;
        }
        start_up = start_down = 1;
        m_in = std::min (r + 1, image.w - 1);
        m_ax = std::max (l - 1, 0);

        for (t = m_in; t >= m_ax; t--)
        {
            if ((t >= l) && (t <= r))
            {
                /*pbm_putpixel (image.bitmap, t, j, fillcol);*/
                ppix(image, t, j, fillcol);

                if (mark)
                    /*pbm_putpixel (mark->bitmap, t - xl, j - yt, 1);*/
                    ppix((*mark), t - xl, j - yt, 1);
            }
            if ((j - 1 >= 0) && (pbm_getpixel (image.bitmap, t, j - 1) == lookfor))
            {
                if (start_up)
                {
                    push (t, j - 1);
                    start_up = 0;
                }
            }
            else
            {
                start_up = 1;
            }
            if ((j + 1 < image.h) && (pbm_getpixel (image.bitmap, t, j + 1) == lookfor))
            {
                if (start_down)
                {
                    push (t, j + 1);
                    start_down = 0;
                }
            }
            else
            {
                start_down = 1;
            }
        }
    }
}


void marktype_fillextract4(marktype image,
                      marktype* mark,
                      int i, int j, int xl, int yt,
                      unsigned int lookfor, unsigned int fillcol)
{
    int l, r, t, start_up, start_down;

    top = bottom = 0;

    if (filla == nullptr)
    CALLOC (filla, MAX_FS, fillpostype);

    if (pbm_getpixel (image.bitmap, i, j) == fillcol)
    {
        return;
    }
    push (i, j);
    while (top != bottom)
    {

        pop (l, j);
        r = l;

        while ((l - 1 >= 0) && (pbm_getpixel (image.bitmap, l - 1, j) == lookfor))
        {
            l--;
        }
        while ((r + 1 < image.w) && (pbm_getpixel (image.bitmap, r + 1, j) == lookfor))
        {
            r++;
        }
        start_up = start_down = 1;
        for (t = std::min (r, image.w - 1); t >= std::max (l, 0); t--)
        {
            if ((t >= l) && (t <= r))
            {
                /*pbm_putpixel (image.bitmap, t, j, fillcol);*/
                ppix(image, t, j, fillcol);

                if (mark)
                    /*pbm_putpixel (mark->bitmap, t - xl, j - yt, 1);*/
                    ppix((*mark), t - xl, j - yt, 1);
            }
            if ((j - 1 >= 0) && (pbm_getpixel (image.bitmap, t, j - 1) == lookfor))
            {
                if (start_up)
                {
                    push (t, j - 1);
                    start_up = 0;
                }
            }
            else
            {
                start_up = 1;
            }
            if ((j + 1 < image.h) && (pbm_getpixel (image.bitmap, t, j + 1) == lookfor))
            {
                if (start_down)
                {
                    push (t, j + 1);
                    start_down = 0;
                }
            }
            else
            {
                start_down = 1;
            }
        }
    }
}
