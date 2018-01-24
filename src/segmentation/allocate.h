/**
 * File name : allocate.h 
 *
 * File Description : This is a header file for allocate.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _ALLOCATE_H_
#define _ALLOCATE_H_

void *alloc_array(int size_array, size_t size_element);
void **alloc_img(int dim1, int dim2, size_t size_element);
void ***alloc_vol(int dim1, int dim2,int dim3, size_t size_element);
void ****alloc_vols(int dim1, int dim2,int dim3,int dim4, size_t size_element);
void multifree(void *r,int d);

#endif
