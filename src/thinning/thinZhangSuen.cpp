#include "thinZhangSuen.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdexcept>


/**
* @brief Perform one thinning iteration.
* 
* @param  im    Binary image with range = 0-1
* @param  iter  0=even, 1=odd
*/
void thinZhangSuenIteration(cv::Mat& imageUnderProcessing, int iteration)
{
    cv::Mat marker = cv::Mat::zeros(imageUnderProcessing.size(), CV_8UC1);

    uchar* inputData = imageUnderProcessing.data;
    uchar* markerData = marker.data;

    for (int i = 1; i < imageUnderProcessing.rows - 1; ++i)
    {
        for (int j = 1; j < imageUnderProcessing.cols - 1; ++j)
        {
            uchar& marker_p1 = markerData[i * imageUnderProcessing.cols + j];

            uchar p2 = inputData[(i - 1) * imageUnderProcessing.cols + (j - 0)];
            uchar p3 = inputData[(i - 1) * imageUnderProcessing.cols + (j + 1)];
            uchar p4 = inputData[(i - 0) * imageUnderProcessing.cols + (j + 1)];
            uchar p5 = inputData[(i + 1) * imageUnderProcessing.cols + (j + 1)];
            uchar p6 = inputData[(i + 1) * imageUnderProcessing.cols + (j - 0)];
            uchar p7 = inputData[(i + 1) * imageUnderProcessing.cols + (j - 1)];
            uchar p8 = inputData[(i - 0) * imageUnderProcessing.cols + (j - 1)];
            uchar p9 = inputData[(i - 1) * imageUnderProcessing.cols + (j - 1)];

            int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                    (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                    (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                    (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);

            int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;

            int m1 = iteration == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iteration == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
            {
                marker_p1 = 1;
            }
        }
    }

    imageUnderProcessing &= ~marker;
}

void prl::thinZhangSuen(cv::Mat& inputImage, cv::Mat& outputImage)
{
    if (inputImage.empty())
    {
        throw std::invalid_argument("Input image for thinning is empty");
    }
    // we work with color images
    if (inputImage.type() != CV_8UC3 && inputImage.type() != CV_8UC1)
    {
        throw std::invalid_argument("Invalid type of image for thinning (required 8 or 24 bits per pixel)");
    }

    cv::Mat imageUnderProcess;

    if (inputImage.data == outputImage.data)
    {
        imageUnderProcess = inputImage;
    }
    else
    {
        imageUnderProcess = inputImage.clone();
    }

    if (imageUnderProcess.channels() == 3)
    {
        cv::cvtColor(imageUnderProcess, imageUnderProcess, CV_BGR2GRAY);
    }

    imageUnderProcess &= 1;

    cv::Mat prev = cv::Mat::zeros(imageUnderProcess.size(), CV_8UC1);
    cv::Mat diff;

    do
    {
        thinningZhangSuenIteration(imageUnderProcess, 0);
        thinningZhangSuenIteration(imageUnderProcess, 1);

        cv::absdiff(imageUnderProcess, prev, diff);
        imageUnderProcess.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    if (inputImage.data == outputImage.data)
    {
        imageUnderProcess *= 255;
    }
    else
    {
        outputImage = imageUnderProcess * 255;
    }
}
