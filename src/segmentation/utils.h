#ifndef __UTILS_H
#define __UTILS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

/* round a to nearest int */
#define ROUND(a)        ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))



#define CALLOC(var,cols_x,type) {assert(cols_x>=1);\
        var=(type *)calloc(cols_x,sizeof(type));\
        assert(var);}
			       
#define REALLOC(var,cols_x,type) {assert(cols_x>=1);\
        var=(type *)realloc(var,(cols_x)*sizeof(type));\
        assert(var);}
			       
#define FREE(var) {if(var)free(var);}

#define CALLOC_2D(var,rows_y,cols_x,type) {int _temp2;\
        assert(rows_y>=1);\
        CALLOC(var,rows_y,type *)\
        for(_temp2=0;_temp2<rows_y;_temp2++)\
             CALLOC(var[_temp2],cols_x,type)}

#define FREE_2D(var,rows_y) {int _temp2;\
	for(_temp2=0;_temp2<rows_y;_temp2++)\
	      FREE((var)[_temp2]);\
        FREE(var);}


#endif
