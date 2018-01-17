/**
 * File name : CCC_library_itnl.h
 *
 * File Description : This is a internal header file for CCC_library.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _CCC_LIBRARY_ITNL_H_
#define _CCC_LIBRARY_ITNL_H_

#define FLG_BOUND 1
#define FLG_ORIGINAL 2
#define RED_WHITECC_RATIO 8 /* redundant white cc ratio */
/* If cc contains a large white cc, it will be ignored */

/* structure */
typedef struct
{
    int     m;       /* Rows of image in pixels */
    int     n;       /* Columns of image in pixels */
} CC_pixel;

typedef struct c_list{
    CC_pixel  pixels;       /* Neighbors unsearched yet */
    struct c_list *pnext;   /* Pointer to next data */
} CC_clist;

typedef struct
{
    int     row;
    int     col;
} ROWCOL;

typedef struct {
    ROWCOL  upperleft;
    ROWCOL  lowerright;
    int     first_flag;
} Corner_info;

static void cnt_boundary_length( marklistptr list, unsigned int height, unsigned int width, int *bound_list);
static void alloc_edge_memory(int *bound_list, unsigned int comp_num, unsigned char ****edge_ptr);
static void free_edge_memory(int *bound_list, unsigned int comp_num, unsigned char ***edge);
static void  calc_boundary( marklistptr list, int *bound_list, unsigned char ***inner_edge,unsigned char ***outer_edge, int height, int width, unsigned char ***input_img_c, unsigned char **bin_msk);
static void calc_edge(int *bound_list, unsigned int comp_num, unsigned char ***inner_edge, unsigned char  ***outer_edge, double *edge_depth, double *edge_std, double *edge_std2, double* edge_max, double* edge_min);
static void calc_boundary_length_cc(marklistptr n, unsigned int height, unsigned int width, int *length);

static void Addtail(CC_clist newlist, CC_clist **pstart, CC_clist **pen);
static CC_clist Removehead(CC_clist **pstart, CC_clist **pend);

static void calc_white_edge(marklistptr list, double *white_cc_edge, double *black_cc_edge, double *area_list, unsigned int height, unsigned int width, 
unsigned char ***input_img, unsigned char **bin_msk,
char flg_bound);
static void calc_white_cc(marklistptr list, double *white_cc_pxl_list, 
unsigned int *cnt_white, 
unsigned int height, unsigned int width, unsigned char ***input_img, 
unsigned char **bin_msk);

static void reverse_cc(unsigned char **input_bin, unsigned char **bin_img,
unsigned int height, unsigned int width, unsigned char  **flip_bin, 
unsigned char **rem_bin);
static void Region_growing(CC_pixel s, unsigned char** bin_msk, unsigned char **out_bin, int width, int height);
static void Region_growing_cnt(CC_pixel s, unsigned char** bin_msk, unsigned char **out_bin, int width, int height, unsigned int* cnt);
static void ConnectedNeighbors_UCHAR(CC_pixel s, unsigned char** bin_msk,
int width, int height, int* M, CC_pixel c[4]);

static void record_corner( marklistptr n, Corner_info *corner_info);
static void flip_reversed_cc( unsigned char ***input_img, unsigned char **bin_msk, unsigned int height, unsigned int width);   

static void  make_feat( marklistptr list, unsigned int comp_num,
unsigned int height, unsigned int width, unsigned char ***input_img,
unsigned char **bin_msk, double **vector);

static void QuickSort(unsigned int *array, unsigned int *index,\
unsigned int p,unsigned int r);
static unsigned int Partition(unsigned int *array, unsigned int *index,\
unsigned int p, unsigned int r);
static unsigned int find_percentile(unsigned int *data_array, int num,
unsigned int  percent);

#endif


