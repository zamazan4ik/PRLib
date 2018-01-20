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
    int m;       /* Rows of image in pixels */
    int n;       /* Columns of image in pixels */
} CC_pixel;

typedef struct c_list
{
    CC_pixel pixels;       /* Neighbors unsearched yet */
    struct c_list* pnext;   /* Pointer to next data */
} CC_clist;

typedef struct
{
    int row;
    int col;
} ROWCOL;

typedef struct
{
    ROWCOL upperleft;
    ROWCOL lowerright;
    int first_flag;
} Corner_info;

void cnt_boundary_length(marklistptr list, unsigned int height, unsigned int width, std::vector<int>& bound_list);

void alloc_edge_memory(std::vector<int>& bound_list, unsigned int comp_num, unsigned char**** edge_ptr);

void free_edge_memory(int* bound_list, unsigned int comp_num, unsigned char*** edge);

void
calc_boundary(marklistptr list, std::vector<int>& bound_list, unsigned char*** inner_edge, unsigned char*** outer_edge, int height,
              int width, unsigned char*** input_img_c, unsigned char** bin_msk);

void calc_edge(std::vector<int>& bound_list, unsigned int comp_num, unsigned char*** inner_edge, unsigned char*** outer_edge,
               std::vector<double>& edge_depth, std::vector<double>& edge_std, std::vector<double>& edge_std2,
               std::vector<double>& edge_max, std::vector<double>& edge_min);

void calc_boundary_length_cc(marklistptr n, unsigned int height, unsigned int width, int* length);

void Addtail(CC_clist newlist, CC_clist** pstart, CC_clist** pen);

CC_clist Removehead(CC_clist** pstart, CC_clist** pend);

void calc_white_edge(marklistptr list, std::vector<double>& white_cc_edge,
                     std::vector<double>& black_cc_edge, std::vector<double>& area_list,
                     unsigned int height, unsigned int width, unsigned char*** input_img,
                     unsigned char** bin_msk, char flg_bound);

void calc_white_cc(marklistptr list, std::vector<double>& white_cc_pxl_list,
                   std::vector<unsigned int>& cnt_white,
                   unsigned int height, unsigned int width, unsigned char*** input_img,
                   unsigned char** bin_msk);

void reverse_cc(unsigned char** input_bin, unsigned char** bin_img,
                unsigned int height, unsigned int width, cv::Mat& flip_bin,
                cv::Mat& rem_bin);

void Region_growing(CC_pixel s, unsigned char** bin_msk, cv::Mat& out_bin, int width, int height);

void Region_growing_cnt(CC_pixel s, unsigned char** bin_msk, cv::Mat& out_bin, int width, int height,
                        unsigned int* cnt);

void ConnectedNeighbors_UCHAR(CC_pixel s, unsigned char** bin_msk,
                              int width, int height, int* M, CC_pixel c[4]);

void record_corner(marklistptr n, Corner_info* corner_info);

void
flip_reversed_cc(unsigned char*** input_img, unsigned char** bin_msk, unsigned int height, unsigned int width);

void make_feat(marklistptr list, unsigned int comp_num,
                      unsigned int height, unsigned int width, unsigned char*** input_img,
                      unsigned char** bin_msk, double** vector);

void QuickSort(std::vector<unsigned int>& array, std::vector<unsigned int>& index, unsigned int p, unsigned int r);

unsigned int Partition(std::vector<unsigned int>& array, std::vector<unsigned int>& index, unsigned int p, unsigned int r);

unsigned int find_percentile(const std::vector<unsigned int>& data_array, int num, unsigned int percent);

#endif


