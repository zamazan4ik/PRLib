/**
 * File name : segmentation.c
 *
 * File Description : Segmentation 
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date : 12/21/2008
 * Version : 1.10
 *
 *
 */

#include <cstring>
#include <cstdlib>
#include <cmath>

#include <opencv2/core/core.hpp>

#include "Main_def.h"
#include "allocate.h"
#include "segmentation.h"
#include "COS_library.h"
#include "CCC_library.h"

/* internal function */
void multiscale_seg(unsigned char*** img_sRGB, cv::Mat& bin_msk, Seg_parameter* seg_para);

/**
 * Function Name : segmentation
 *
 * Function Description :
 * Extract text from an input color image to create a binary image
 *
 * Input       : input_img, input image
 *             : seg_para, segmentation parameter info 
 * Output      : bin_msk, output binary image
 * Version : 1.0
 */

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int segmentation(const cv::Mat& inputImage, cv::Mat& bin_msk, Seg_parameter* seg_para)
{
    unsigned char*** img_sRGB;
    unsigned int height, width;

    /**** Get segmentation parameter info ****/
    height = seg_para->height;
    width = seg_para->width;

    /**********************************************************/
    /*                                                        */
    /*                    Color conversion                    */
    /*                                                        */
    /**********************************************************/

    img_sRGB = (unsigned char***) alloc_vol(3, height, width, sizeof(unsigned char));
    for (int k = 0; k < 3; k++)
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                img_sRGB[k][i][j] = inputImage.at<cv::Vec3b>({i, j})[k];
            }
        }
    }

    /**********************************************************/
    /*                                                        */
    /*                    Segmentation                        */
    /*                                                        */
    /**********************************************************/

    multiscale_seg(img_sRGB, bin_msk, seg_para);
    multifree(img_sRGB, 3);
    return FLG_OK;
}

/**
 * Function Name : multiscale_seg
 *
 * Function Description :
 * Multiscale segmentation 
 *
 * Input       : img_sRGB, input RGB image
 *             : seg_para, segmentation parameter info
 * Output      : bin_msk, output binary segmentation image
 * Version : 1.0
 */

void multiscale_seg(unsigned char*** img_sRGB, cv::Mat& bin_msk, Seg_parameter* seg_para)
{
    unsigned int height, width, nh, nw;
    unsigned int multi_lyr_itr;

    height = seg_para->height;
    width = seg_para->width;
    multi_lyr_itr = seg_para->multi_lyr_itr;

    seg_para->cur_block = seg_para->max_block;
    seg_para->prev_S_b = nullptr;
    seg_para->prev_binmsk = nullptr;

    for (int L = multi_lyr_itr - 1, cur_lyr_itr = 0; L >= 0; L--, cur_lyr_itr++)
    {
        seg_para->cur_lyr_itr = cur_lyr_itr;

        /*****  Cost Optimized Segmentation (COS) *****/

        nh = 2 * ((height - 1) / seg_para->cur_block + 1) - 1;
        nw = 2 * ((width - 1) / seg_para->cur_block + 1) - 1;
        seg_para->S_b = (unsigned char**) alloc_img(nh, nw, sizeof(unsigned char));
        seg_para->binmsk = (unsigned char**) alloc_img(height, width, sizeof(unsigned char));
        seg_para->cur_nh = nh;
        seg_para->cur_nw = nw;
        COS_segment(img_sRGB, seg_para);
        CCC_segment(img_sRGB, seg_para);

        if (L != multi_lyr_itr - 1)
        {
            multifree(seg_para->prev_S_b, 2);
            multifree(seg_para->prev_binmsk, 2);
        }
        if (L == 0)
        {
            multifree(seg_para->S_b, 2);
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    bin_msk.at<uchar>({i, j}) = seg_para->binmsk[i][j];
                }
            }
            multifree(seg_para->binmsk, 2);
        }
        else
        {
            seg_para->prev_S_b = seg_para->S_b;
            seg_para->prev_nh = nh;
            seg_para->prev_nw = nw;
            seg_para->prev_binmsk = seg_para->binmsk;
            seg_para->cur_block = (seg_para->cur_block) / 2;
        }
    }
}
