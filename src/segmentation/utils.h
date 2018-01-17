#ifndef __UTILS_H
#define __UTILS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>


#ifndef PI
#define PI 3.14159265358979323846
#endif

#define LOG2(x)  (log(x)/0.69314718055994530941)

 
/* round a to nearest integer towards 0 */
#define FLOOR(a)                ((a)>0 ? (int)(a) : -(int)(-(a)))
 
/* round a to nearest integer away from 0 */
#define CEIL(a) \
((a)==(int)(a) ? (a) : (a)>0 ? 1+(int)(a) : -(1+(int)(-(a))))
 
/* round a to nearest int */
#define ROUND(a)        ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))
 
/* take sign of a, either -1, 0, or 1 */
#define ZSGN(a)         (((a)<0) ? -1 : (a)>0 ? 1 : 0)
 
/* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)          (((a)<0) ? -1 : 0)

 
/* swap a and b (see Gem by Wyvill) */
#define SWAP(a,b)       { a^=b; b^=a; a^=b; }
 
/* linear interpolation from l (when a=0) to h (when a=1)*/
/* (equal to (a*h)+((1-a)*l) */
#define LERP(a,l,h)     ((l)+(((h)-(l))*(a)))
 
/* clamp the input to the specified range */
#define CLAMP(v,l,h)    ((v)<(l) ? (l) : (v) > (h) ? (h) : v)
 
 


/*
#define FLOOR(x) (int)(x)
#define CEIL(x) (int)((x)+0.99999)
#define ROUND(x) ((x)>=0 ? (int)((x)+0.5) : (-(int)((-(x))+0.5)))
*/

#define mm2pixels(mm,DPI) ROUND((DPI*(mm))/25.4)
#define inches2pixels(in,DPI) ROUND(DPI*(in))

#define pixels2inches(pixels,DPI) ((pixels)/DPI)
#define pixels2mm(pixels,DPI) inches2mm(pixels2inches(pixels,DPI))

#define mm2inches(mm) ((mm)/25.4)
#define inches2mm(inches) ((inches)*25.4)

#define RAD2DEG(rad) ((rad)*180.0/PI)
#define DEG2RAD(deg) ((deg)*PI/180.0)




#define MAGIC_P1      0x5031         /* P1   - pbm */
#define MAGIC_P2      0x5032         /* P2   - pgm */
#define MAGIC_P3      0x5033         /* P3   - ppm */
#define MAGIC_P4      0x5034         /* P4   - rawbits pbm */
#define MAGIC_P5      0x5035         /* P5   - rawbits pgm */
#define MAGIC_P6      0x5036         /* P6   - rawbits ppm */
#define MAGIC_TIFF    0x4d4d         /* TIFF header -- big endian */
#define MAGIC_TIFF_sml    0x4949     /* TIFF header -- small endian */



#define TIMEOUT 5 

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


#define CALLOC_3D(var,third_z,rows_y,cols_x,type) {int _temp3;\
        assert(third_z>=0);\
        CALLOC(var,third_z, type **)\
        for(_temp3=0;_temp3<third_z;_temp3++)\
	   CALLOC_2D(var[_temp3],rows_y,cols_x,type)}

#define CALLOC_4D(var,fourth_f,third_z,rows_y,cols_x,type) {int _temp4;\
        CALLOC(var,fourth_f, type ***)\
        for(_temp4=0;_temp4<fourth_f;_temp4++)\
	   CALLOC_3D(var[_temp4],third_z,rows_y,cols_x,type)}


#define FREE_2D(var,rows_y) {int _temp2;\
	for(_temp2=0;_temp2<rows_y;_temp2++)\
	      FREE((var)[_temp2]);\
        FREE(var);}

#define FREE_3D(var,third_z,rows_y) {int _temp3;\
	for(_temp3=0;_temp3<third_z;_temp3++)\
	      FREE_2D((var)[_temp3],rows_y);\
        FREE(var);}

#define FREE_4D(var,fourth_f,third_z,rows_y) {int _temp4;\
	for(_temp4=0;_temp4<fourth_f;_temp4++)\
	      FREE_3D((var)[_temp4],third_z,rows_y);\
        FREE(var);}

			       
			       

char *fgoodgets(FILE *fp);
/*void   readline(char str[], FILE *fp);*/


int    isEOF(FILE *fp);

unsigned char  magic_getbyte(FILE *fp);
unsigned char  magic_popbyte(FILE *fp);
unsigned short magic_getshort(FILE *fp);
unsigned short magic_popshort(FILE *fp);
unsigned long  magic_getlong(FILE *fp);
unsigned long  magic_poplong(FILE *fp);

void   magic_write(FILE *fp, unsigned long magic_num);
void   magic_check(FILE *fp, unsigned long magic_num);
unsigned long magic_popnamed(char fn[], int *err);


void   error(char *prog, char *message, char *extra);
void   warn(char *prog, char *message, char *extra);
 


int    getint(FILE *fp);
int    isinteger(char s[]);
int    isfloat(char s[]);

#endif
