/*
    MIT License

    Copyright (c) 2017 Alexander Zaitsev

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "balanceGrayWorldWhite.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace prl
{

void getAverageValues(const cv::Mat& src, double* ml, double* ma, double* mb, const double p)
{
    for (int i = 0; i < src.rows; ++i)
    {
        for (int j = 0; j < src.cols; ++j)
        {
            cv::Vec3b v1 = src.at<cv::Vec3b>(i, j);
            double lc = std::pow(v1.val[0], p);
            double ac = std::pow(v1.val[1], p);
            double bc = std::pow(v1.val[2], p);

            *ma += ac;
            *mb += bc;
            *ml += lc;
        }
    }

    *ml = std::pow(*ml / (src.cols * src.rows), 1.0 / p);
    *ma = std::pow(*ma / (src.cols * src.rows), 1.0 / p);
    *mb = std::pow(*mb / (src.cols * src.rows), 1.0 / p);
}


void grayWorldWhiteBalance(const cv::Mat& inputImage, cv::Mat& outputImage,
                           const double pNorm, const bool withMax)
{
    cv::Mat inputImageMat = inputImage.clone();

    if (inputImageMat.empty())
    {
        throw std::invalid_argument("GrayWorldWhiteBalance: input image is empty");
    }

    if (inputImageMat.channels() != 3)
    {
        throw std::invalid_argument("GrayWorldWhiteBalance: input image hasn't 3 channels");
    }

    cv::Mat outputImageMat;
    outputImageMat = inputImageMat.clone();

    double ma = 0.0, mb = 0.0, ml = 0.0;

    getAverageValues(inputImageMat, &ml, &ma, &mb, pNorm);

    double r = (ma + mb + ml) / 3.0;

    if (withMax)
    {
        r = std::max(ma, std::max(mb, ml));
    }

    double r_div_ml = r / ml;
    double r_div_ma = r / ma;
    double r_div_mb = r / mb;

    auto itout = outputImageMat.begin<cv::Vec3b>();
    for (auto it = inputImageMat.begin<cv::Vec3b>(), itend = inputImageMat.end<cv::Vec3b>();
         it != itend; ++it, ++itout)
    {
        cv::Vec3b v1 = *it;

        double l = v1.val[0] * r_div_ml;
        double a = v1.val[1] * r_div_ma;
        double b = v1.val[2] * r_div_mb;

        l = std::min(255.0, l);
        a = std::min(255.0, a);
        b = std::min(255.0, b);

        v1.val[0] = static_cast<uchar>(l);
        v1.val[1] = static_cast<uchar>(a);
        v1.val[2] = static_cast<uchar>(b);

        *itout = v1;
    }

    outputImage = outputImageMat.clone();
}
}