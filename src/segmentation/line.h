#ifndef __LINE_H
#define __LINE_H

#include "marklist.h"

#define START_OF_LINE 1
#define END_OF_LINE   2
#define START_PRUNED  10
#define SPACE         20

typedef struct {
  int num_marks;
  marktype *mark;
} linetype;

void line_init(linetype *z);
void line_add(linetype *z, marklistptr list);
void line_addmark(linetype *z, marktype *m);
void line_remove(linetype *z, int line);
void line_free(linetype *z); 

void line_dump(linetype *z);
void line_sort_xpos(linetype *z);
marklistptr lines_to_listcopy(linetype **lines, int num_lines);
marklistptr lines_to_listcopy_indirect(linetype **lines, linetype *indirect,int num_lines);

int line_stats(linetype *z);
int line_stats_indirect(linetype *z, linetype *indirect);




void line_test();

#endif
