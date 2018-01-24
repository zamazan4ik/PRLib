#include "segmentation_ccc.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <exception>

#include <opencv2/imgproc/imgproc.hpp>


#include "getopt.h"
#include "allocate.h"
#include "Main_def.h"
#include "segmentation.h"




/* Internal function declaration */
//static void usage(void);

namespace prl
{
    void segmentation_ccc(const cv::Mat& inputImage, cv::Mat& outputImage)
    {
        if (inputImage.channels() != 3)
        {
            throw std::invalid_argument("Input image hasn't 3 channels.");
        }
        cv::Mat convImage;
        cv::cvtColor(inputImage, convImage, CV_BGR2RGB);

        int height, width;
        char color_flg;
        int i;
        bool error_flag = false;
        int ret;
        div_t d1;
        char block_flg = FLG_OFF;

        /* default parameter values */
        double text_cost = 0.0; /* Weight toward text */
        unsigned int block = 32;  /* block size */
        unsigned int multi_lyr_itr; /* multi-layer iteration */
        unsigned int dpi = 300;   /* resolution (dpi) */
        /* # of vertical dynamic programming iteration */
        unsigned int dynamic_itr_num = 20;
        unsigned int tmp;

        Seg_parameter seg_para;

        /* Read lambda parameters and multiscale layers */
        seg_para.multi_lyr_itr = 1;
        seg_para.lambda = (double**) alloc_img(1, 4, sizeof(double));
        // TODO: Predefined values
        //30.680974 21.939354 36.658849 56.000098
        seg_para.lambda[0][0] = 30.680974;
        seg_para.lambda[0][1] = 21.939354;
        seg_para.lambda[0][2] = 36.658849;
        seg_para.lambda[0][3] = 56.000098;
        multi_lyr_itr = seg_para.multi_lyr_itr;

        if (dpi <= 0 || block <= 0 || dynamic_itr_num <= 0 || multi_lyr_itr <= 0)
        {
            throw std::invalid_argument("Parameters are out of range.");
        }

        /* Set block size */
        if (block_flg == FLG_OFF)
        {
            block = (unsigned int) (block * (unsigned int) (std::floor((double) dpi / 300)));
        }
        /* Check block size value */
        d1 = div(block, 4);
        if (d1.rem != 0)
        {
            throw std::invalid_argument("Block size should be dividable by 4");
        }

        height = inputImage.rows;
        width = inputImage.cols;

        /* Check Color or Grayscale */
        if (inputImage.channels() == 1 && inputImage.depth() == CV_8U)
        {
            throw std::invalid_argument("Sorry, this software does not accept grayscale images.");
        }
        else if (inputImage.channels() == 3 && inputImage.depth() == CV_8U)
        {
            color_flg = FLG_COLOR;
        }
        else
        {
            throw std::invalid_argument("Sorry, this software does not accept your image color mode.\n"
                                                "Please use 8bit full RGB mode: samplesperpixel = 3 and bitspersample = 8");
        }

        /* Check multi-layer iteration value */
        tmp = block;
        seg_para.min_block = block;
        for (i = 1; i <= multi_lyr_itr; i++)
        {
            if ((tmp < height / 5) && (tmp < width / 5))
            {
                tmp = tmp * MULTI_LAYER_RATE;
            }
            else
            {
                multi_lyr_itr = i - 1;
                break;
            }
        }
        seg_para.max_block = tmp / MULTI_LAYER_RATE;

        if (multi_lyr_itr == 0)
        {
            throw std::invalid_argument("Block size is too large.");
        }


        /* Set parameter already known */
        seg_para.height = height;
        seg_para.width = width;
        seg_para.dynamic_itr_num = dynamic_itr_num;
        seg_para.text_cost = text_cost;

        /* Memory allocation */
        cv::Mat bin_mask(height, width, CV_8UC3);
        /* Segmentation */

        ret = segmentation(convImage, bin_mask, &seg_para);

        if (ret == FLG_NG)
        {
        }

        cv::Mat outputImageBin(height, width, CV_8U, cv::Scalar(255));

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if(bin_mask.at<uchar>({i, j}) > 0)
                {
                    outputImageBin.at<uchar>({j, i}) = 0;
                }
            }
        }

        outputImage = outputImageBin.clone();

        /* Free memories */
        multifree(seg_para.lambda, 2);
    }
}

