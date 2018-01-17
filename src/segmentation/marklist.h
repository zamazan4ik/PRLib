#ifndef __MARKLIST_H
#define __MARKLIST_H

#include <stdlib.h>
#include <stdio.h>
#include "pbmtools.h"


#define ppix(image,c,r,v)\
      pbm_putpixel_trunc(image.bitmap,c,r,v,image.w,image.h)

#define gpix(image,c,r)\
      (pbm_getpixel_trunc(image.bitmap,c,r,image.w,image.h))


typedef struct {
	int symnum;		  /* component number during processing */
	short xpos,ypos;	  /* the top left position in the document */
	int w,h;		  /* the width/height of the component */
	short xcen,ycen;      /* the centroid positions */
	float d_xcen,d_ycen;      /* the centroid positions */
	short xoffset,yoffset;
	short baseline;
        short topline;
	char *name;
	Pixel **bitmap;
	short resolution;          /* in dpi */
        int imagew,imageh;
        short h_runs,v_runs;

        unsigned char start_of_line;
        short grp;

        float skew_of_line;  
        int ypos_of_line;

	int set;
        int class_old; /* added by eri */
        int blength;
} marktype;


typedef struct marklisttype *marklistptr;
typedef struct marklisttype {
	marktype data;
	marklistptr next;
}marklisttype;


/* The following functions operate on the 'marktype' */

void        marktype_read(FILE *fp, marktype *im);
void        marktype_write(FILE *fp, const marktype im);
int         marktype_readnamed(char *fn, marktype *im);
void        marktype_writenamed(char *fn, marktype im);
void        marktype_dump(FILE *fp,marktype d);
marktype    marktype_copy(marktype d);
void        marktype_alloc(marktype *d,int w, int h);
void        marktype_init(marktype *d);
void        marktype_free(marktype *d);

void        marktype_fill_cleanup(void);
void        marktype_calc_centroid(marktype *b);
void        marktype_adj_bound(marktype *m);
void        marktype_area(marktype *m);
void        marktype_placeat(marktype image, marktype mark,int x, int y);
marktype    marktype_getfrom(marktype image, int x1,int y1,int x2,int y2);

int         marktype_wh_runs(marktype d);
int         marktype_wv_runs(marktype d);
int         marktype_bh_runs(marktype d);
int         marktype_bv_runs(marktype d);

void        marktype_fill8(marktype image, int i, int j, unsigned int lookfor, unsigned int fillcol);
void        marktype_fill4(marktype image, int i, int j, unsigned int lookfor, unsigned int fillcol);
void        marktype_fillextract8(marktype image,marktype *mark,int i,int j,int xl,int yt,unsigned int lookfor, unsigned int fillcol);
void        marktype_fillextract4(marktype image,marktype *mark,int i,int j,int xl,int yt,unsigned int lookfor, unsigned int fillcol);
void        marktype_line(marktype im,int x1,int y11,int x2, int y2);

int         marktype_readascii(FILE *fp, marktype *d);
void        marktype_writeascii(FILE *fp, marktype d);


/* whereas the following functions operate on a list of marktype's, named a 'marklist' */

void        marklist_append(marklistptr *list, marklistptr append);

void        marklist_average(marklistptr list, marktype *avgmark);
marklistptr marklist_add(marklistptr *list, marktype m);
marklistptr marklist_addcopy(marklistptr *list, marktype m);
int 	    marklist_length(marklistptr list);
void 	    marklist_dump(FILE *fp, marklistptr list);
marklistptr marklist_getat(marklistptr list, int pos, marktype *d);
void 	    marklist_free(marklistptr *list);
int    	    marklist_removeat(marklistptr *list, int pos);
marklistptr marklist_next(marklistptr list);

marklistptr marklist_prune_under_pixels (marklistptr * list, int pixels, int DPI);

marklistptr marklist_prune_under_area(marklistptr *list, float area_mm, int DPI);
void        marklist_prune_over_area(marklistptr *list, float area_mm, int DPI);
void        marklist_prune_under_size(marklistptr *list, float size_mm, int DPI);
void        marklist_prune_over_size(marklistptr *list, float size_mm, int DPI);

int         marklist_readascii(char libraryname[], marklistptr *library);
void        marklist_writeascii(char libraryname[], marklistptr library);

void marklist_reconstruct(marklistptr list, marktype *im);
void marklist_reconstruct_using_wh (marklistptr list, marktype * im, int w, int h);

void marklist_reconstruct_show_lines(marklistptr list, marktype *im);

float rms_pbm_diff(marktype bm1, marktype bm2);
void marklist_reconstruct_only_wh (const marklistptr list, marktype * im);
int   marktype_crc(marktype *m);
void  marklist_stats (const marklistptr list, FILE *fp);

marklistptr list_bound_size(marklistptr list,int bound_w,int bound_h);

#endif

