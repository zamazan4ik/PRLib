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
#include "line.h"


/* 
 * marktype_read, simply reads the bitmap, does not set *any* of the fields
 * in marktype, ie. area (.set) is not assigned 
 */
void
marktype_read(FILE* fp, marktype* im)
{
    if (pbm_isapbmfile(fp))
    {
        Pixel** p;
        p = pbm_readfile(fp, &(im->w), &(im->h), &(im->resolution));
        im->bitmap = p;
    }
    else
    {
        error("Can only read PBM files at the moment", "", "");
    }
}


/*
 * marktype_write writes the bitmap
 */
void
marktype_write(FILE* fp, const marktype im)
{
    pbm_writefile(fp, im.bitmap, im.w, im.h, im.resolution);
}


/*
 * marktype_writenamed, uses a filename instead of a FILE pointer.
 */
void
marktype_writenamed(char* fn, marktype im)
{
    FILE* tempfp;

    if (strcmp(fn, "-") == 0)
    {
        tempfp = stdout;
    }
    else
    {
        tempfp = fopen(fn, "wb");
        if (tempfp == NULL)
        {
            fprintf(stderr, "marktype_writenamed(), can't write file: %s\n", fn);
            return;
        }
    }

    pbm_writefile(tempfp, im.bitmap, im.w, im.h, im.resolution);
    fclose(tempfp);
}


int
marktype_readnamed(char* fn, marktype* im)
{
    FILE* tempfp;
    Pixel** temp;
    unsigned long magic;
    int err;
    char* lookfn;
    char* fn2 = NULL;
    char* msg = "temporary file";

    if (strcmp(fn, "-") == 0)
    {
        fprintf(stderr, "error - can't read from named file \"-\"!!!!\n");
        return 1;
    }
    magic = magic_popnamed(fn, &err) >> 16;    /* only interested in first 2 bytes */

    if (err != 0)
    {
        return 1;            /* failure */
        fprintf(stderr, "error - can't open file '%s'\n", fn);
        return 1;
    }
    if (!((magic == MAGIC_P1) || (magic == MAGIC_P4)))
    {
        return 1;
    }
    if (fn2)
    {
        tempfp = fopen(fn2, "rb");
        lookfn = msg;
    }
    else
    {
        tempfp = fopen(fn, "rb");
        lookfn = fn;
    }

    if (tempfp == NULL)
    {
        fprintf(stderr, "readnamed: can't open file: %s\n", lookfn);
        return 1;
    }
    if (pbm_isapbmfile(tempfp))
    {
        if (tempfp == NULL)
        {
            fprintf(stderr, "marktype_readname: can't open file: %s\n", lookfn);
            return 1;
        }
        marktype_init(im);
        temp = pbm_readfile(tempfp, &(im->w), &(im->h), &(im->resolution));
        im->bitmap = temp;
        im->imagew = im->w;
        im->imageh = im->h;
        fclose(tempfp);
    }
    else
    {
        if (fn2)
        {
            remove(fn2);
        }

        fprintf(stderr, "marktype_readnamed(): file is not bilevel pbm format\n");
        return 1;
    }

    if (fn2)
    {
        remove(fn2);
    }

    return 0;            /* success */
}


/*
 * marktype_dump, writes out (some) of the fields to a FILE pointer.
 * (Mainly used for debugging)
 */
void
marktype_dump(FILE* fp, marktype d)
{
    fprintf(fp, "symnum: %d\n", d.symnum);
    fprintf(fp, "w: %d, h: %d\n", d.w, d.h);
    fprintf(fp, "xpos: %d, ypos: %d\n", d.xpos, d.ypos);
    fprintf(fp, "xcen: %d, ycen: %d\n", d.xcen, d.ycen);
    fprintf(fp, "xoffset: %d, yoffset: %d\n", d.xoffset, d.yoffset);
    fprintf(fp, "area: %d, h_runs: %d, v_runs: %d\n", d.set, d.h_runs, d.v_runs);

    marktype_writeascii(fp, d);
}


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


void
marklist_reconstruct_using_wh(marklistptr list, marktype* im, int w, int h)
{
    marklistptr step;
    int imagew = 0, imageh = 0;

    imagew = w;
    imageh = h;

    marktype_alloc(im, imagew, imageh);

    for (step = list; step; step = step->next)
    {
        marktype_placeat(*im, step->data, step->data.xpos, step->data.ypos);
    }
}


void
marklist_reconstruct_show_lines(marklistptr listret, marktype* im)
{
    marklistptr step;
    int i;

    marklist_reconstruct(listret, im);

    for (step = listret; step; step = step->next)
    {
        if (step->data.start_of_line)
        {
            if (step->data.start_of_line == START_OF_LINE)
            {
                for (i = -20; i <= 0; i++)
                {
                    ppix((*im), step->data.xpos + i, step->data.ypos + step->data.h, 1);
                    ppix((*im), step->data.xpos + i, step->data.ypos + step->data.h + 1, 1);
                }
                for (i = -20; i <= 0; i++)
                {
                    ppix((*im), step->data.xpos - 20, step->data.ypos + step->data.h + i, 1);
                    ppix((*im), step->data.xpos - 20 + 1, step->data.ypos + step->data.h + i, 1);
                }
            }
            else if (step->data.start_of_line == END_OF_LINE)
            {
                for (i = 0; i <= 20; i++)
                {
                    ppix((*im), step->data.xpos + step->data.w + 20 - i, step->data.ypos + step->data.h, 1);
                    ppix((*im), step->data.xpos + step->data.w + 20 - i, step->data.ypos + step->data.h + 1, 1);
                }
                for (i = -20; i <= 0; i++)
                {
                    ppix((*im), step->data.xpos + step->data.w + 20, step->data.ypos + step->data.h + i, 1);
                    ppix((*im), step->data.xpos + step->data.w + 20 + 1, step->data.ypos + step->data.h + i, 1);
                }
            }
            else if (step->data.start_of_line == SPACE)
            {
                for (i = 0; i < step->data.w; i++)
                {
                    ppix((*im), step->data.xpos + i, step->data.ypos + 3, 1);
                    ppix((*im), step->data.xpos + i, step->data.ypos + 4, 1);
                }
            } /* endif */
        }
    }
}

void
marklist_reconstruct_only_wh(const marklistptr list, marktype* im)
{
    int imagew = 0, imageh = 0;

    if (list != NULL)
    {
        imagew = list->data.imagew;
        imageh = list->data.imageh;
    }
    else
    {
        imagew = imageh = 0;
    }

    im->w = imagew;
    im->imagew = imagew;
    im->h = imageh;
    im->imageh = imageh;
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


void
marklist_append(marklistptr* list, marklistptr append)
{
    marklistptr step = *list, step_append;

    for (step_append = append; step_append; step_append = step_append->next)
    {
        if ((*list) == NULL)
        {
            step = marklist_addcopy(list, step_append->data);
        }
        else
        {
            step = marklist_addcopy(&step, step_append->data);
        }
    }
}


/* 
 * marklist_average, returns a newly created mark reprenting the average of
 * the marklist.  
 * 
 * The name of the mark is set to "?".  
 * 
 * (Perhaps it would be a good idea to see if all the marks had the same
 * name, and if they didn't set to "?"  else copy the old name...)
 */
void marklist_average(marklistptr list, marktype* avgmark)
{
    int w = 0, h = 0;
    int t = INT_MAX, b = 0, l = INT_MAX, r = 0, matched = 0;
    marktype d, temp;
    marklistptr step;
    int** a = nullptr;
    int x, y;
    int set;

    if (list == nullptr)
    {
        error("marklist_average", "Can't average 'nothing'!", "");
    }

    for (step = list; step; step = step->next)
    {
        matched++;
        temp = step->data;
        w = std::max (w, temp.w);
        h = std::max (h, temp.h);
        r = std::max<int>(r, temp.xcen);
        l = std::min<int>(l, temp.xcen);
        b = std::max<int>(b, temp.ycen);
        t = std::min<int>(t, temp.ycen);
    }
    w = w + (r - l);
    h = h + (b - t);

    if (matched > 1)
    {
        int x_off, y_off;

        CALLOC_2D(a, w, h, int);

        for (x = 0; x < w; x++)
        {
            for (y = 0; y < h; y++)
            {
                a[x][y] = 0;
            }
        }

        for (step = list; step; step = step->next)
        {
            x_off = r - step->data.xcen;
            y_off = b - step->data.ycen;
            for (x = 0; x < step->data.w; x++)
            {
                for (y = 0; y < step->data.h; y++)
                {
                    if (pbm_getpixel (step->data.bitmap, x, y))
                    {
                        assert(x + x_off >= 0);
                        assert(x + x_off < w);
                        assert(y + y_off >= 0);
                        assert(y + y_off < h);
                        a[x + x_off][y + y_off]++;
                    }
                }
            }
        }

        set = 0;
        d.w = w;
        d.h = h;
        marktype_alloc(&d, w, h);
        for (x = 0; x < w; x++)
        {
            for (y = 0; y < h; y++)
            {
                /* > gives better CR than >= */
                if (a[x][y] > std::max (1, (matched / 2)))
                {
                    pbm_putpixel (d.bitmap, x, y, 1);
                    set++;
                }
            }
        }
        FREE_2D(a, w);

        assert(set != 0);

        marktype_adj_bound(&d);
        d.set = set;
        d.name = (char*) realloc(d.name, sizeof(char) * (strlen("?") + 1));
        strcpy(d.name, "?");
        marktype_calc_centroid(&d);
        /*marktype_area (&d);*/
    }
    else
    {
        d = marktype_copy(list->data);
    }

    /*  k_fill(&d,3);*/

    *avgmark = d;
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


int marklist_length(marklistptr list)
{
    int count = 0;

    while (list)
    {
        count++;
        list = list->next;
    }
    return count;
}


void marklist_dump(FILE* fp, marklistptr list)
{
    int count = 0;

    while (list)
    {
        marktype_dump(fp, list->data);

        count++;
        list = list->next;
    }
}


void
marklist_stats(const marklistptr list, FILE* fp)
{
    float area = 0;
    float w_area = 0;
    float width = 0;
    float height = 0;
    int num = 0;
    marklistptr step;
    marktype im2;

    if (fp == nullptr)
    {
        return;
    }

    marklist_reconstruct_only_wh(list, &im2);

    step = list;
    while (step)
    {
        area += step->data.set;
        w_area += ((step->data.w * step->data.h) - step->data.set);
        width += step->data.w;
        height += step->data.h;
        num++;
        step = step->next;
    }
    fprintf(fp, "Num_in_list:         %d\n", num);
    fprintf(fp, "Image_width:         %d\n", im2.w);
    fprintf(fp, "Image_height:        %d\n", im2.h);
    fprintf(fp, "lib_black_pixels:    %.0f\n", area);
    fprintf(fp, "lib_white_pixels:    %.0f\n", w_area);
    fprintf(fp, "Percent_area_in_lib: %.1f\n", (w_area + area) * 100.0 / (im2.w * im2.h));
    fprintf(fp, "Mark_avg_black_area: %.1f\n", area / num);
    fprintf(fp, "Mark_avg_white_area: %.1f\n", w_area / num);
    fprintf(fp, "Mark_avg_width:      %.1f\n", width / num);
    fprintf(fp, "Mark_avg_height:     %.1f\n", height / num);
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


void
marklist_free(marklistptr* list)
{
    marklistptr n;

    while ((*list) != NULL)
    {
        n = (*list)->next;
        marktype_free(&((*list)->data));
/*      pbm_free(&((*list)->data.bitmap),(*list)->data.h); */
        free((*list));
        (*list) = n;
    }
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


marklistptr
marklist_next(marklistptr list)
{
    return list->next;
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


marklistptr
marklist_prune_under_pixels(marklistptr* list, int pixels, int _temp)
{
    int count, i, deleted = 0, num = 0;
    marktype d;
    marklistptr ret = NULL, step;

    if (pixels > 0)
    {
        count = marklist_length(*list);
        i = 0;
        if (count)
        {
            do
            {
                marklist_getat(*list, i, &d);
                if (d.set <= pixels)
                {
                    if (ret == NULL)
                    {
                        step = marklist_addcopy(&ret, d);
                    }
                    else
                    {
                        step = marklist_addcopy(&step, d);
                    }

                    marklist_removeat(list, i);
                    i--;
                    count--;
                    deleted += d.set;
                    num++;
                }
                i++;
            }
            while (i < count);
        }
    }
    return ret;
}


marklistptr
marklist_prune_under_area(marklistptr* list, float area_mm, int DPI)
{
    int count, i, deleted = 0, num = 0;
    marktype d;
    int pixels;
    marklistptr ret = NULL, step;

    pixels = ROUND ((area_mm / 25.4 / 25.4) * (DPI * DPI));

    /*fprintf(stderr,"under %d\n",pixels);*/

    if (pixels > 0)
    {
        count = marklist_length(*list);
        i = 0;
        if (count)
        {
            do
            {
                marklist_getat(*list, i, &d);
                if (d.set <= pixels)
                {
                    if (ret == NULL)
                    {
                        step = marklist_addcopy(&ret, d);
                    }
                    else
                    {
                        step = marklist_addcopy(&step, d);
                    }

                    marklist_removeat(list, i);
                    i--;
                    count--;
                    deleted += d.set;
                    num++;
                }
                i++;
            }
            while (i < count);
        }
    }
    /*fprintf(stderr,"%d (%d pixels)\n",num,deleted);*/
    return ret;
}

void
marklist_prune_over_area(marklistptr* list, float area_mm, int DPI)
{
    int count, i;
    marktype d;
    int pixels;

    pixels = ROUND ((area_mm / 25.4 / 25.4) * (DPI * DPI));

    /*fprintf(stderr,"over %d\n",pixels);*/


    if (pixels > 0)
    {
        count = marklist_length(*list);
        i = 0;
        if (count)
        {
            do
            {
                marklist_getat(*list, i, &d);
                if (d.set >= pixels)
                {
                    marklist_removeat(list, i);
                    i--;
                    count--;
                }
                i++;
            }
            while (i < count);
        }
    }
}


void
marklist_prune_under_size(marklistptr* list, float size_mm, int DPI)
{
    int count, i;
    marktype d;
    int pixels;

    pixels = mm2pixels (size_mm, DPI);


    if (pixels)
    {
        count = marklist_length(*list);
        i = 0;
        if (count)
        {
            do
            {
                marklist_getat(*list, i, &d);
                if ((d.w <= pixels) || (d.h <= pixels))
                {
                    marklist_removeat(list, i);
                    i--;
                    count--;
                }
                i++;
            }
            while (i < count);
        }
    }
}

void
marklist_prune_over_size(marklistptr* list, float size_mm, int DPI)
{
    int count, i;
    marktype d;
    int pixels;

    pixels = mm2pixels (size_mm, DPI);

    if (pixels)
    {
        count = marklist_length(*list);
        i = 0;
        if (count)
        {
            do
            {
                marklist_getat(*list, i, &d);
                if ((d.w > pixels) || (d.h > pixels))
                {
                    marklist_removeat(list, i);
                    i--;
                    count--;
                }
                i++;
            }
            while (i < count);
        }
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


void
marktype_line(marktype im, int x1, int y1, int x2, int y2)
{
    int x, y, sigx, sigy;
    int absx, absy, d, dx, dy;

    dx = x2 - x1;
    if (dx < 0)
    {
        absx = -dx;
        sigx = -1;
    }
    else
    {
        absx = dx;
        sigx = 1;
    }
    absx = absx << 1;

    dy = y2 - y1;
    if (dy < 0)
    {
        absy = -dy;
        sigy = -1;
    }
    else
    {
        absy = dy;
        sigy = 1;
    }
    absy = absy << 1;

    x = x1;
    y = y1;

    if (absx > absy)
    {
        d = absy - (absx > 1);
        while (1)
        {
            ppix (im, x, y, 1);
/*      pbm_putpixel(im.bitmap,x,y,1); */
            if (x == x2)
            {
                return;
            }
            if (d >= 0)
            {
                y += sigy;
                d -= absx;
            }
            x += sigx;
            d += absy;
        }
    }
    else
    {
        d = absx - (absy > 1);
        while (1)
        {
            ppix (im, x, y, 1);
/*      pbm_putpixel(im.bitmap,x,y,1); */
            if (y == y2)
            {
                return;
            }
            if (d >= 0)
            {
                x += sigx;
                d -= absy;
            }
            y += sigy;
            d += absx;
        }
    }
}


/* range inclusive */
marktype
marktype_getfrom(marktype image,
                 int x1, int y1,
                 int x2, int y2)
{
    int w, h, i, j, p;
    int set = 0;
    marktype m;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;

    /* copy over all the fields */
    m = image;
    {
        marktype m2;
        marktype_alloc(&m2, w, h);
        m.w = m2.w;
        m.h = m2.h;
        m.bitmap = m2.bitmap;
        m.name = NULL;
    }


    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            p = gpix(image, x1 + i, y1 + j);
            if (p)
            {
                set++;
                ppix(m, i, j, p);
            }
        }
    }
    m.set = set;
    m.xpos += x1;
    m.ypos += y1;
    marktype_calc_centroid(&m);

    return m;
}

int marktype_crc(marktype* m)
{
    int i, j;
    int crc = 13;
    int p;

    assert(m);

    for (j = 0; j < m->h; j++)
    {
        for (i = 0; i < m->w; i++)
        {
            p = gpix((*m), i, j);
            crc = 3 + (p + 7) * i * j + crc + i + j * 2;
        }
    }
    return crc;
}

/*
 * list_bound_size
 *
 * Split large marks into a set of smaller marks. A
 * mark is split if the width **and** the height are
 * too large.
 */

marklistptr
list_bound_size(marklistptr list,
                int bound_w,
                int bound_h)
{
    marklistptr step = NULL, step2 = NULL, list2 = NULL;
    int pos;

    /* shortcut */
    if ((bound_w == 0) && (bound_h == 0))
    {
        return list;
    }

    for (pos = 0, step = list; step; pos++, step = step->next)
    {
        int s_w = 1, s_h = 1;

        if (bound_w)
        {
            s_w = (int) ceil(step->data.w * 1.0 / bound_w);
        }
        if (bound_h)
        {
            s_h = (int) ceil(step->data.h * 1.0 / bound_h);
        }

        /* if we need to split it */
        if ((s_w > 1) || (s_h > 1))
        {
            int i, j;

            for (j = 0; j < s_h; j++)
            {
                for (i = 0; i < s_w; i++)
                {
                    int x1, x2, y1, y2;
                    marktype get;

                    if (bound_w)
                    {
                        x1 = i * bound_w;
                        x2 = ((i + 1) * bound_w) - 1;
                        if (x2 >= step->data.w)
                        { x2 = step->data.w - 1; }
                    }
                    else
                    {
                        x1 = 0;
                        x2 = step->data.w - 1;
                    }

                    if (bound_h)
                    {
                        y1 = j * bound_h;
                        y2 = ((j + 1) * bound_h) - 1;
                        if (y2 >= step->data.h)
                        { y2 = step->data.h - 1; }
                    }
                    else
                    {
                        y1 = 0;
                        y2 = step->data.h - 1;
                    }

                    get = marktype_getfrom(step->data, x1, y1, x2, y2);

                    if (get.set)
                    {
                        /*	    fprintf(stderr,"pos: %d (%d %d, into %dx%d) -- %d %d %d %d\n",pos,
                            step->data.w,step->data.h,s_w,s_h,
                            x1,y1,x2,y2);*/

                        /* add each of the marks in the sub-regions */

                        if (step2 == NULL)
                        {
                            step2 = marklist_addcopy(&list2, get);
                        }
                        else
                        {
                            step2 = marklist_addcopy(&step2, get);
                        }
                    }
                }
            }
        }
        else
        {
            /* didn't need to split it, so we'll just copy it */
            if (step2 == NULL)
            {
                step2 = marklist_addcopy(&list2, step->data);
            }
            else
            {
                step2 = marklist_addcopy(&step2, step->data);
            }
        }

    }
    marklist_free(&list);

    return list2;
}

/* 
 * only reads in ascii library 
 */
int
marktype_readascii(FILE* fp, marktype* d)
{
    int r, c, rows, cols, numread, ch1 = 'C';
    char* str1 = NULL, * line = NULL, * str2 = NULL;
    int symnum, xpos, ypos, baseline, topline, imagew, imageh;
    int atleast = 0;

    rows = cols = -1;
    symnum = xpos = ypos = baseline = topline = imagew = imageh = 0;
    atleast = 0; /* Mark:, Char: and Cols: must appear for each mark */
    while (1)
    {
        if ((line = fgoodgets(fp)) == NULL)
        {
            return 1;
        }
        str1 = (char*) strdup(line);
        numread = sscanf(line, "%s", str1);
        if (numread != 1)
        {
            fprintf(stderr, "%s: -> %s, %d\n", line, str1, numread);
            free(str1);
            free(line);
            return 0;
        }
        if (strcmp(str1, "Mark:") == 0)
        {
            sscanf(line, "%*s %d", &symnum);
            atleast++;
        }
        else if (strcmp(str1, "Char:") == 0)
        {
            if (str2)
            { free(str2); }
            str2 = (char*) strdup(line);
            sscanf(line, "%*s %s", str2);
            atleast++;
        }
        else if (strcmp(str1, "Baseline:") == 0)
        {
            sscanf(line, "%*s %d", &baseline);
        }
        else if (strcmp(str1, "Topline:") == 0)
        {
            sscanf(line, "%*s %d", &topline);
        }
        else if (strcmp(str1, "Xpos:") == 0)
        {
            sscanf(line, "%*s %d", &xpos);
        }
        else if (strcmp(str1, "Ypos:") == 0)
        {
            sscanf(line, "%*s %d", &ypos);
        }
        else if (strcmp(str1, "ImageW:") == 0)
        {
            sscanf(line, "%*s %d", &imagew);
        }
        else if (strcmp(str1, "ImageH:") == 0)
        {
            sscanf(line, "%*s %d", &imageh);
        }
        else if (strcmp(str1, "Cols:") == 0)
        {
            atleast++;
            sscanf(line, "%*s %d %*s %d\n", &cols, &rows);
            /* Cols: %d Rows: %d, is the last set of options, so we break */
            free(str1);
            str1 = NULL;
            free(line);
            line = NULL;
            break;
        }

        free(str1);
        str1 = NULL;
        free(line);
        line = NULL;
    }

    if ((cols < 0) || (rows < 0) || (atleast < 3))
    {
        return 0;
    }
    marktype_alloc(d, cols, rows);
    d->symnum = symnum;
    d->xpos = (short) xpos;
    d->ypos = (short) ypos;
    d->baseline = baseline;
    d->topline = topline;
    {
        int alloclen = 2;
        if (str2)
        {
            alloclen = strlen(str2) + 1;
        }
        d->name = (char*) realloc(d->name, sizeof(char) * alloclen);
    }
    d->imagew = imagew;
    d->imageh = imageh;
    if (str2 == NULL)
    {
        strcpy(d->name, "?");
    }
    else
    {
        strcpy(d->name, str2);
        free(str2);
    }

    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            ch1 = getc (fp);
            if ((ch1 == 'X') || (ch1 == 'C'))
                pbm_putpixel (d->bitmap, c, r, 1);
            else pbm_putpixel (d->bitmap, c, r, 0);
        }
        ch1 = getc (fp);
    }

    marktype_calc_centroid(d);
    marktype_area(d);

    return 1;
}

/* reads a library, ignore the mark num in the file, simply overwrites */
int
marklist_readascii(char libraryname[], marklistptr* library)
{
    int err, count;
    FILE* lib;
    marktype d;
    marklistptr step;

    marktype_init(&d);

    if (strcmp(libraryname, "stdin") == 0)
    {
        lib = stdin;
    }
    else if (strcmp(libraryname, "-") == 0)
    {
        lib = stdin;
    }
    else
    {
        lib = fopen(libraryname, "rb");
    }
    if (lib == NULL)
    {
        fprintf(stderr, "marklist_readascii(): Trouble opening library file '%s'.\n", libraryname);
        exit(1);
    }
    *library = NULL;
    count = 0;
    while (!isEOF(lib))
    {
        err = marktype_readascii(lib, &d);
        if (!err)
        {
            error("marklist_readascii()", "unknown format of the library file.", "");
        }
        d.symnum = count;
        count++;
        if ((*library) == NULL)
        {
            step = marklist_add(library, d);
        }
        else
        {
            step = marklist_add(&step, d);
        }
    }
    count = marklist_length(*library);

    if ((strcmp(libraryname, "stdin") != 0) &&
        (strcmp(libraryname, "-") != 0))
    {
        fclose(lib);
    }

    return count;
}


void
marklist_writeascii(char libraryname[], marklistptr library)
{
    FILE* lib;
    marklistptr step;

    if (strcmp(libraryname, "stderr") == 0)
    {
        lib = stderr;
    }
    else if (strcmp(libraryname, "stdout") == 0)
    {
        lib = stdout;
    }
    else if (strcmp(libraryname, "-") == 0)
    {
        lib = stdout;
    }
    else
    {
        lib = fopen(libraryname, "wb");
    }
    if (lib == NULL)
    {
        fprintf(stderr, "marklist_writeascii(): can't create file '%s'.\n", libraryname);
        return;
    }
    for (step = library; step; step = step->next)
    {
        marktype_writeascii(lib, step->data);
    }
    fclose(lib);
}


