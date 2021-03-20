#ifndef __MARKLIST_H
#define __MARKLIST_H

#include <cstdlib>
#include <cstdio>
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

marktype    marktype_copy(marktype d);
void        marktype_alloc(marktype *d,int w, int h);
void        marktype_init(marktype *d);
void        marktype_free(marktype *d);

void        marktype_fill_cleanup(void);
void        marktype_calc_centroid(marktype *b);
void        marktype_area(marktype *m);
void        marktype_fillextract8(marktype image,marktype *mark,int i,int j,int xl,int yt,unsigned int lookfor, unsigned int fillcol);
void        marktype_fillextract4(marktype image,marktype *mark,int i,int j,int xl,int yt,unsigned int lookfor, unsigned int fillcol);

marklistptr marklist_add(marklistptr *list, marktype m);
marklistptr marklist_addcopy(marklistptr *list, marktype m);

#endif

