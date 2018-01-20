/**
 * File name : segmentation.h
 *
 * File Description : This is a external header file for segmentation.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _SEGMENTATION_H_
#define _SEGMENTATION_H_

#define MULTI_LAYER_RATE 2

#include <array>

#include <opencv2/core/core.hpp>

/***************************************************/
/*  External structure Definition                  */
/***************************************************/
typedef struct
{
    std::array<double,4> lambda;
    unsigned int height; /* image height */
    unsigned int width;  /* image width */
    unsigned int min_block;  /* minimum block size */
    unsigned int max_block;  /* maximum block size */
    unsigned int dynamic_itr_num; /* vertical dynamic programming
                                   iteration */
    unsigned int multi_lyr_itr;   /* multi-layer iteration */
    double text_cost;             /* Weights for text for the final layer */

    /* Temporary storage for multi-layer segmentation */
    unsigned char** S_b;      /* class of current layer */
    unsigned char** binmsk;   /* binary mask of current layer */
    unsigned char** prev_S_b; /* class of previous coarser layer */
    unsigned char** prev_binmsk; /* binary mask of previous layer */
    unsigned int cur_block;   /* current block size for multi-layer seg */
    unsigned int prev_nh;
    unsigned int prev_nw;
    unsigned int cur_nh;
    unsigned int cur_nw;
    unsigned int cur_lyr_itr;
} Seg_parameter;

/***************************************************/
/*  External function declarations                 */
/***************************************************/
int segmentation(const cv::Mat& inputImage, cv::Mat& bin_msk, Seg_parameter* seg_para);

#endif
