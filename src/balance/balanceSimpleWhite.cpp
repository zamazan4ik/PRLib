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

#include "balanceSimpleWhite.h"

#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


void prl::simpleWhiteBalance(const cv::Mat& inputImage, cv::Mat& outputImage,
                             const double k)
{
    cv::Mat inputImageMat = inputImage.clone();

    if (inputImageMat.empty())
    {
        throw std::invalid_argument(
                "SimpleWhiteBalance: input image is empty.");
    }

    if (inputImageMat.channels() != 3)
    {
        throw std::invalid_argument(
                "SimpleWhiteBalance: input image hasn't 3 channels.");
    }

    cv::Mat outputImageMat;
    outputImageMat = inputImageMat.clone();

    int hists[3][256];
    memset(hists, 0, 3 * 256 * sizeof(int));

    for (size_t y = 0; y < inputImageMat.rows; ++y)
    {
        const uchar* const ptr = inputImageMat.ptr<uchar>(static_cast<int>(y));
        for (size_t x = 0; x < inputImageMat.cols; ++x)
        {
            /*for (size_t j = 0; j < 3; ++j)
            {
                hists[j][ptr[x * 3 + j]] += 1;
            }*/
            ++(hists[0][ptr[x * 3 + 0]]);
            ++(hists[1][ptr[x * 3 + 1]]);
            ++(hists[2][ptr[x * 3 + 2]]);
        }
    }

    // cumulative hist
    size_t total = inputImageMat.cols * inputImageMat.rows;
    int vmin[3], vmax[3];
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 255; ++j)
        {
            hists[i][j + 1] += hists[i][j];
        }
        vmin[i] = 0;
        vmax[i] = 255;
        while (hists[i][vmin[i]] < k * total)
        {
            vmin[i] += 1;
        }
        while (hists[i][vmax[i]] > (1 - k) * total)
        {
            vmax[i] -= 1;
        }
        if (vmax[i] < 255 - 1)
        {
            vmax[i] += 1;
        }
    }

    float vmax0_vmin0_dist_mul255 = 255.0f / (vmax[0] - vmin[0]);
    float vmax1_vmin1_dist_mul255 = 255.0f / (vmax[1] - vmin[1]);
    float vmax2_vmin2_dist_mul255 = 255.0f / (vmax[2] - vmin[2]);

    for (size_t y = 0; y < outputImageMat.rows; ++y)
    {
        uchar* ptr = outputImageMat.ptr<uchar>(static_cast<int>(y));
        for (size_t x = 0; x < outputImageMat.cols; ++x)
        {
            //for (size_t j = 0; j < 3; ++j)
            //{
            //	uchar val = ptr[x * 3 + j];
            //	if (val < vmin[j])
            //		val = vmin[j];
            //	if (val > vmax[j])
            //		val = vmax[j];

            //	ptr[x * 3 + j] = static_cast<uchar>((val - vmin[j]) * 255.0 / (vmax[j] - vmin[j]));
            //}

            size_t usedPtrPos = x * 3;

            uchar val0 = ptr[usedPtrPos + 0];
            uchar val1 = ptr[usedPtrPos + 1];
            uchar val2 = ptr[usedPtrPos + 2];

            if (val0 < vmin[0])
            { val0 = vmin[0]; }
            if (val0 > vmax[0])
            { val0 = vmax[0]; }
            if (val1 < vmin[1])
            { val1 = vmin[1]; }
            if (val1 > vmax[1])
            { val1 = vmax[1]; }
            if (val2 < vmin[2])
            { val2 = vmin[2]; }
            if (val2 > vmax[2])
            { val2 = vmax[2]; }

            ptr[usedPtrPos + 0] = static_cast<uchar>((val0 - vmin[0]) * vmax0_vmin0_dist_mul255);
            ptr[usedPtrPos + 1] = static_cast<uchar>((val1 - vmin[1]) * vmax1_vmin1_dist_mul255);
            ptr[usedPtrPos + 2] = static_cast<uchar>((val2 - vmin[2]) * vmax2_vmin2_dist_mul255);
        }
    }

    outputImage = outputImageMat.clone();
}

