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

#endif
