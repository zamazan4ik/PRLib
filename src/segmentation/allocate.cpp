#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "allocate.h"


/**************************************************************************

The following program allocates arrays of 1, 2, 3 and 4 dimensions.
The 1D array allocation function is called "alloc_array".
The 2D array allocation function is called "alloc_img"
The 3D array allocation function is called "alloc_vol"
The 4D array allocation function is called "alloc_vols"
These functions are meant to be called by other subroutines for the sole 
purpose of allocation.

Notes:
 
 - The basic algorithm is written for allocating an array using calloc.
 - Array priority order dim1 > dim2 > dim3 > dim4

 ************************************************************************/

/*************************************************************************

Description:

 This function allocates a 1D array.
  
     size_array   : number of elements in array
     size_element : size of each element in array (eg: float, short)

**************************************************************************/

void* alloc_array(int size_array, size_t size_element)
{

    void* pt;

    if ((pt = calloc((size_t) size_array, size_element)) == nullptr)
    {
        fprintf(stderr, "==> calloc() error\n");
        exit(-1);
    }
    return (pt);

}

/*****************************************************************************

Description:

 This function allocates a 2D array

     ppt  : array of 1D arrays.
     pt   : 1D array that has cells for each element of the 2D array 

*****************************************************************************/

void** alloc_img(int dim1, int dim2, size_t size_element)
{

    int i;
    void** ppt;
    char* pt;

    ppt = (void**) alloc_array(dim1, sizeof(void*));
    pt = (char*) alloc_array(dim1 * dim2, size_element);

    for (i = 0; i < dim1; i++)
    {
        ppt[i] = pt + i * dim2 * size_element;
    }

    return (ppt);
}

/***************************************************************************

Description:

 This function allocates a 3D array

     i, j : loop variables
     pppt : the 3D array
     ppt  : one level down the pppt array - contains dim2 for each row in pppt
     pt   : one level down the ppt array - contains dim3 for each column in ppt

*****************************************************************************/

void*** alloc_vol(int dim1, int dim2, int dim3, size_t size_element)
{
    char* pt;
    void** ppt, *** pppt;

    pppt = (void***) alloc_array(dim1, sizeof(void**));
    ppt = (void**) alloc_array(dim1 * dim2, sizeof(void*));
    pt = (char*) alloc_array(dim1 * dim2 * dim3, size_element);

    for (int i = 0; i < dim1; i++)
    {
        pppt[i] = ppt + i * dim2;
    }

    for (int j = 0; j < dim1 * dim2; j++)
    {
        ppt[j] = pt + j * dim3 * size_element;
    }

    return pppt;
}

/************************************************************************

Description:

 This function allocates a 4D array

     i, j, k : loop variables
     ppppt   : the 4D array
     pppt    : one level down the ppppt array - contains dim2 for each row in ppppt
     ppt     : one level down the pppt array - contains dim3 for each column in pppt
     pt      : one level down the ppt array - contains dim4 for each dim3 in ppt

*************************************************************************/

void**** alloc_vols(int dim1, int dim2, int dim3, int dim4, size_t size_element)
{
    int i, j, k;
    char* pt;
    void** ppt, *** pppt, **** ppppt;

    ppppt = (void****) alloc_array(dim1, sizeof(void***));
    pppt = (void***) alloc_array(dim1 * dim2, sizeof(void**));
    ppt = (void**) alloc_array(dim1 * dim2 * dim3, sizeof(void*));
    pt = (char*) alloc_array(dim1 * dim2 * dim3 * dim4, size_element);

    for (i = 0; i < dim1; i++)
    {
        ppppt[i] = pppt + i * dim2;
    }

    for (j = 0; j < dim1 * dim2; j++)
    {
        pppt[j] = ppt + j * dim3;
    }

    for (k = 0; k < dim1 * dim2 * dim3; k++)
    {
        ppt[k] = pt + k * dim4 * size_element;
    }
    return (ppppt);
}

/*****************************************************************************

Author: C. Bouman

Description:

 This routine frees an n-dimensional array allocated by alloc_array
                                                          "  _img
							  "  _vol
							  "  _vols
  
******************************************************************************/

void multifree(void* r, int d)
{
    void** p;
    void* next;
    int i;

    for (p = (void**) r, i = 0; i < d; p = (void**) next, i++)
    {
        if (p != NULL)
        {
            next = *p;
            free((void*) p);
        }
    }
}

