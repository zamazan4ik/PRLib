#include "segmentation_ccc.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <exception>
#include <vector>

#include <opencv2/imgproc/imgproc.hpp>


#include "allocate.h"
#include "Main_def.h"
#include "segmentation.h"


namespace prl
{
    void segmentation_ccc(const cv::Mat& inputImage, std::vector<cv::Mat>& outputImages)
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
        // TODO: Predefined values
        //30.680974 21.939354 36.658849 56.000098
        seg_para.lambda[0] = 30.680974;
        seg_para.lambda[1] = 21.939354;
        seg_para.lambda[2] = 36.658849;
        seg_para.lambda[3] = 56.000098;
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


        //! Fill layers
        //! 1) Text layer
        //! 2) Foreground layer
        //! 3) Background layer

        outputImages.resize(3);

        cv::threshold(outputImageBin, outputImageBin, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

        outputImages[0] = outputImageBin.clone();

        inputImage.copyTo(outputImages[1], 255 - outputImageBin);
        outputImages[1].setTo(cv::Scalar(255, 255, 255), outputImageBin);
        inputImage.copyTo(outputImages[2], outputImageBin);
        outputImages[2].setTo(cv::Scalar(255, 255, 255), 255 - outputImageBin);
    }

    cv::Vec3b findMedianColor(const cv::Mat& fgImage, const cv::Mat& textImage)
    {
        std::vector<int> red, green, blue;
        std::vector<int> result;

        //Collect color values
        for (int i = 0; i < fgImage.rows; i++)
        {
            for (int j = 0; j < fgImage.cols; j++)
            {
                if (textImage.at<uchar>(j, i) == 0)
                {
                    red.push_back(fgImage.at<cv::Vec3b>(j, i)[2]);
                    green.push_back(fgImage.at<cv::Vec3b>(j, i)[1]);
                    blue.push_back(fgImage.at<cv::Vec3b>(j, i)[0]);
                }
            }
        }

        const int VECTOR_SIZE = red.size();

        //Sort values
        std::sort(red.begin(), red.end());
        std::sort(green.begin(), green.end());
        std::sort(blue.begin(), blue.end());

        int redInt, blueInt, greenInt;

        //Get median value for each colour channel
        redInt = VECTOR_SIZE % 2 != 0 ? red.at(std::floor(VECTOR_SIZE / 2)) :
                 (red.at(VECTOR_SIZE / 2) + red.at((VECTOR_SIZE / 2) - 1)) / 2;
        greenInt = VECTOR_SIZE % 2 != 0 ? green.at(std::floor(VECTOR_SIZE / 2)) :
                   (green.at(VECTOR_SIZE / 2) + green.at((VECTOR_SIZE / 2) - 1)) / 2;
        blueInt = VECTOR_SIZE % 2 != 0 ? blue.at(std::floor(VECTOR_SIZE / 2)) :
                  (blue.at(VECTOR_SIZE / 2) + blue.at((VECTOR_SIZE / 2) - 1)) / 2;

        return {blueInt, greenInt, redInt};
    }

    void reduceTo64Colors(cv::Mat& inputImage, const cv::Mat& textImage)
    {
        for (int i = 0; i < inputImage.rows; i++)
        {
            for (int j = 0; j < inputImage.cols; j++)
            {
                // operator XXXXXXXX & 11000000 equivalent to  XXXXXXXX AND 11000000 (=192)
                // operator 01000000 >> 2 is a 2-bit shift to the right = 00010000
                if(textImage.at<uchar>(j, i) == 0)
                {
                    for (int k = 0; k < inputImage.channels(); ++k)
                    {
                        inputImage.at<cv::Vec3b>(j, i)[k] &= 192;
                    }
                }
            }
        }
    }

    void ForegroundOptimize(cv::Mat& fgImage, const cv::Mat& textImage)
    {
        // TODO: Possibly we should disable it for quality reason
        //! Reduce colormap
        reduceTo64Colors(fgImage, textImage);

        //! Found median color
        auto medianColor = findMedianColor(fgImage, textImage);

        //! Fill with median color
        for (int i = 0; i < fgImage.rows; i++)
        {
            for (int j = 0; j < fgImage.cols; j++)
            {
                if(textImage.at<uchar>(j, i) != 0)
                {
                    fgImage.at<cv::Vec3b>(j, i) = medianColor;
                }
            }
        }
    }

    void BackgroundOptimization(cv::Mat& bgImage, const cv::Mat& textImage)
    {

    }

    void segmentMRC(const cv::Mat& inputImage, std::vector<cv::Mat>& outputImages)
    {
        std::vector<cv::Mat> layers;

        //! Get layers
        segmentation_ccc(inputImage, layers);

        //! Optimize foreground filling
        //ForegroundOptimize(layers[MRC_Layers::Foreground], layers[MRC_Layers::Text]);

        //BackgroundOptimization(layers[MRC_Layers::Background], layers[MRC_Layers::Text]);

        const bool isDownsamplingRequired = true;
        //TODO: Play with downsampling coefficients
        const double foregroundRatio = 2.0, backgroundRatio = 2.0;

        if(isDownsamplingRequired)
        {
            //! Downsample foreground layer
            cv::resize(layers[MRC_Layers::Foreground], layers[MRC_Layers::Foreground], cv::Size(), 1.0 / foregroundRatio, 1.0 / foregroundRatio, cv::InterpolationFlags::INTER_CUBIC);
            //! Downsample background layer
            cv::resize(layers[MRC_Layers::Background], layers[MRC_Layers::Background], cv::Size(), 1.0 / backgroundRatio, 1.0 / backgroundRatio, cv::InterpolationFlags::INTER_CUBIC);
        }

        outputImages = layers;
    }

}

