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

#include "binarizeGAT.h"

#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


void prl::binarizeGAT(const cv::Mat& inputImage, cv::Mat& outputImage, const int gaussianKernelSize,
                      const double sigmaX, const double sigmaY,
                      const double maxValue, const int blockSize, const int shift)
{
    cv::Mat inputImageMat = inputImage.clone();

    if (inputImageMat.empty())
    {
        throw std::invalid_argument("Input image for binarization is empty");
    }

    if (inputImageMat.channels() != 1)
    {
        cv::cvtColor(inputImageMat, inputImageMat, cv::COLOR_BGR2GRAY);
    }

    cv::Mat outputImageMat;
    cv::Mat tempOutputImageMat;

    cv::GaussianBlur(inputImageMat, tempOutputImageMat,
                     cv::Size(gaussianKernelSize, gaussianKernelSize),
                     sigmaX, sigmaY);

    if (inputImageMat.channels() != 1)
    {
        cv::cvtColor(tempOutputImageMat, outputImageMat, cv::COLOR_BGR2GRAY);
    }

    cv::adaptiveThreshold(
            outputImageMat, outputImageMat,
            maxValue,
            cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY,
            blockSize, shift);

    outputImage = outputImageMat.clone();
}
