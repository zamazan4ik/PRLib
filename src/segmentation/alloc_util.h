/**
 * File name : alloc_util.h
 *
 * File Description : This is a header file for alloc_util.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _ALLOC_UTIL_H_
#define _ALLOC_UTIL_H_

char *G_malloc(int n);
char *G_calloc(int n,int m);
char *G_realloc(char *b,int n);
void G_dealloc(char *b);
double *G_alloc_vector(int n);
double **G_alloc_matrix(int rows,int cols);
void G_free_vector(double *v);
void G_free_matrix(double **m);
int *G_alloc_ivector(int n);
void G_free_ivector(int *v);

#endif
