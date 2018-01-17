#ifndef _BOUNDARY_H
#define _BOUNDARY_H

#include "marklist.h"

typedef struct {
  int len;
  int *x;
  int *y;
} boundarytype;

typedef struct {
  int len;
  int *y;
  int *l;
  int *r;
} lrboundarytype;

marklistptr extract_all_marks(marklistptr list, marktype image, int nested, int conn);
marklistptr extract_all_marks_bound(marklistptr list, marktype image, int nested, int conn, int x1, int y1, int x2, int y2);



#endif
