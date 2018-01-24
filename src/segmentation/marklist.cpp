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
marktype
marktype_copy(marktype d)
{
    marktype d2;

    d2 = d;
    d2.bitmap = pbm_copy(d.bitmap, d2.w, d2.h);
    if (d.name)
    {
        d2.name = (char*) strdup(d.name);
    }
    return d2;
}


void
marktype_init(marktype* d)
{
    d->symnum = 0;
    d->xpos = d->ypos = 0;
    d->w = d->h = 0;
    d->xcen = d->ycen = 0;
    d->d_xcen = d->d_ycen = 0;
    d->xoffset = d->yoffset = 0;
    d->baseline = 0;
    d->topline = 0;
    d->name = NULL;
    d->bitmap = NULL;
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
void
marktype_alloc(marktype* d, int w, int h)
{
    marktype_init(d);
    d->bitmap = pbm_alloc(w, h);
    d->w = w;
    d->imagew = w;
    d->h = h;
    d->imageh = h;
}


void
marktype_free(marktype* d)
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
void
marktype_calc_centroid(marktype* b)
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
void
marktype_area(marktype* d)
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


void
marklist_reconstruct(marklistptr list, marktype* im)
{
    marklistptr step;
    int imagew = 0, imageh = 0;
    int min_w = 0, min_h = 0, max_w = 0, max_h = 0;

    for (step = list; step; step = step->next)
    {
        if (step->data.imagew > max_w)
        { max_w = step->data.imagew; }
        if (step->data.imagew < min_w)
        { min_w = step->data.imagew; }
        if (step->data.imageh > max_h)
        { max_h = step->data.imageh; }
        if (step->data.imageh < min_h)
        { min_h = step->data.imageh; }
    }

    imagew = max_w;
    imageh = max_h;

    marktype_alloc(im, imagew, imageh);

    for (step = list; step; step = step->next)
    {
        marktype_placeat(*im, step->data, step->data.xpos, step->data.ypos);
    }
}

/* places 'mark' at position (x,y) in 'image' bitmap. Crops if the image doesn't fit */

void
marktype_placeat(marktype image, marktype mark, int x, int y)
{
    int i, j, g;

    for (j = 0; j < mark.h; j++)
    {
        for (i = 0; i < mark.w; i++)
        {
            g = gpix (mark, i, j);
            if (g)
                ppix (image, x + i, y + j, g);
        }
    }
}


void
marktype_adj_bound(marktype* m)
{
    int x, y, w, h;
    int xpos, ypos, baseline, topline;
    int top = 0, left = 0, right = 0, bot = 0;
    marktype temp;

    w = m->w;
    h = m->h;
    xpos = m->xpos;
    ypos = m->ypos;
    baseline = m->baseline;
    topline = m->topline;

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            if (pbm_getpixel (m->bitmap, x, y))
            {
                top = y;
                x = w;
                y = h;
            }
        }
    }
    for (y = h - 1; y >= 0; y--)
    {
        for (x = 0; x < w; x++)
        {
            if (pbm_getpixel (m->bitmap, x, y))
            {
                bot = y;
                x = w;
                y = -1;
            }
        }
    }
    for (x = 0; x < w; x++)
    {
        for (y = 0; y < h; y++)
        {
            if (pbm_getpixel (m->bitmap, x, y))
            {
                left = x;
                x = w;
                y = h;
            }
        }
    }
    for (x = w - 1; x >= 0; x--)
    {
        for (y = 0; y < h; y++)
        {
            if (pbm_getpixel (m->bitmap, x, y))
            {
                right = x;
                x = -1;
                y = h;
            }
        }
    }
    {
        Pixel** bitmap;

        temp = *m;
        temp.w = right - left + 1;
        temp.h = bot - top + 1;

        /* allocate the bitmap field */
        marktype_alloc(&temp, temp.w, temp.h);

        /* save the pointer */
        bitmap = temp.bitmap;

        /* copy all fields */
        temp = *m;

        /* reset the three fields we wanted to save */
        temp.bitmap = bitmap;
        temp.w = right - left + 1;
        temp.h = bot - top + 1;
    }

    temp.xpos = xpos + left;
    temp.ypos = ypos + top;
    temp.baseline = baseline - (top);
    temp.topline = topline - (top);

    for (x = 0; x < temp.w; x++)
    {
        for (y = 0; y < temp.h; y++)
            pbm_putpixel (temp.bitmap, x, y, pbm_getpixel(m->bitmap, x + left, y + top));
    }

    marktype_free(m);

    if (m->name)
    {
        temp.name = (char*) realloc(temp.name, sizeof(char) * (strlen(m->name) + 1));
        strcpy(temp.name, m->name);
    }
    marktype_calc_centroid(&temp);

    *m = temp;
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
            error("marklist_add", "OUT of Memory in marklist_add", "");
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
            error("marklist_add", "OUT of Memory in marklist_add", "");
        }
        p = p->next;
    }
    p->data = m;
    p->next = NULL;
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

/*
 * marklist_getat, gets the mark from position [0, marklist_length-1]
 */
marklistptr marklist_getat(marklistptr list, int pos, marktype* d)
{
    int count = 0;

    while (list)
    {
        if (count == pos)
        {
            *d = list->data;
            return list;
        }
        count++;
        list = list->next;
    }
    fprintf(stderr, "marklist_getat(): access off ends of list: pos %d\n", pos);
    /*abort ();*/
    return NULL;
}


int
marklist_removeat(marklistptr* list, int pos)
{
    marklistptr c, n, p = NULL;
    int count = 0;

    if ((pos == 0) && (*list))
    {    /* if removing the 1st element */
        /* change courtesy of Tim Theobald (Pegasus software) */
        p = *list;
        *list = (*list)->next;
        marktype_free(&(p->data));
        free(p);
        return 1;
    }
    else
    {
        c = *list;
        while (c)
        {
            if (count == pos)
            {    /* two cases, no previous, >=1 previous */
                if (p == NULL)
                {    /* no previous */
                    n = c->next;
                    marktype_free(&(c->data));
                    free(c);
                    c = n;
                    return 1;
                }
                else
                {
                    /* >=1 previous */
                    n = c->next;
                    marktype_free(&(c->data));
                    free(c);
                    p->next = n;
                    return 1;
                }
            }
            count++;
            p = c;
            c = c->next;
        }
    }
    fprintf(stderr, "marklist_removeat(): access off ends of list: pos %d\n", pos);
    /*abort ();*/
    return 0;            /* nothing was removed */
}


/* only writes out ascii form */
void
marktype_writeascii(FILE* fp, marktype d)
{
    int r, c, rows, cols;
    int xtot = d.xcen, ytot = d.ycen;

    rows = d.h;
    cols = d.w;

    /*
     * calls calc_centroid, so the C is placed in the correct location
     */
    marktype_calc_centroid(&d);
    marktype_area(&d);

    fprintf(fp, "Mark: %d\n", d.symnum);
    fprintf(fp, "Char: %s\n", (d.name == NULL) ? "?" : d.name);
    fprintf(fp, "Baseline: %d\n", d.baseline);
    fprintf(fp, "Topline: %d\n", d.topline);
    fprintf(fp, "Xpos: %d\n", d.xpos);
    fprintf(fp, "Ypos: %d\n", d.ypos);
    fprintf(fp, "ImageW: %d\n", d.imagew);
    fprintf(fp, "ImageH: %d\n", d.imageh);
    fprintf(fp, "Cols: %d Rows: %d\n", d.w, d.h);
    for (r = 0; r < d.h; r++)
    {
        for (c = 0; c < d.w; c++)
        {
            if ((r == ytot) && (c == xtot))
                putc (pbm_getpixel(d.bitmap, c, r) ? 'C' : '!', fp);
            else
                putc (pbm_getpixel(d.bitmap, c, r) ? 'X' : '.', fp);
        }
        putc ('\n', fp);
    }
}

static int MAX_FS = 8192;

static int bottom = 0, top = 0;
typedef struct
{
    unsigned short x, y;
} fillpostype;


fillpostype* filla = NULL;

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


void
marktype_fill_cleanup(void)
{
    if (filla)
    {
        free(filla);
    }
    filla = NULL;
    MAX_FS = 8192;
}


void
marktype_fillextract8(marktype image,
                      marktype* mark,
                      int i, int j, int xl, int yt,
                      unsigned int lookfor, unsigned int fillcol)
{
    static int l, r, t, start_up, start_down;
    static int m_in, m_ax;

    bottom = top = 0;

    if (filla == NULL)
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


void
marktype_fillextract4(marktype image,
                      marktype* mark,
                      int i, int j, int xl, int yt,
                      unsigned int lookfor, unsigned int fillcol)
{
    int l, r, t, start_up, start_down;

    top = bottom = 0;

    if (filla == NULL)
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
