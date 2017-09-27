#include "thinGuoHall.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * @param  im    Binary image with range = 0-1
 * @param  iter  0=even, 1=odd
 */
void thinGuoHallIteration(cv::Mat& imageUnderProcessing, int iteration)
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

            int C = ((!p2) & (p3 | p4)) + ((!p4) & (p5 | p6)) +
                    ((!p6) & (p7 | p8)) + ((!p8) & (p9 | p2));
            int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
            int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
            int N = N1 < N2 ? N1 : N2;
            int m = iteration == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4); // -V564

            if (C == 1 && (N >= 2 && N <= 3) && (m == 0))
            {
                marker_p1 = 1;
            }
        }
    }

    imageUnderProcessing &= ~marker;
}

void prl::ThinningGuoHallImpl(cv::Mat& inputImage, cv::Mat& outputImage)
{
    if (inputImage.empty())
    {
        throw std::invalid_argument("Input image for thinning is empty"));
    }
    // we work with color images
    if (inputImage.type() != CV_8UC3 && inputImage.type() != CV_8UC1)
    {
        throw std::invalid_argument("Invalid type of image for thinning (required 8 or 24 bits per pixel)"));
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
        cvtColor(imageUnderProcess, imageUnderProcess, CV_BGR2GRAY);
    }

    imageUnderProcess &= 1;

    cv::Mat prev = cv::Mat::zeros(imageUnderProcess.size(), CV_8UC1);
    cv::Mat diff;

    do
    {
        thinningGuoHallIteration(imageUnderProcess, 0);
        thinningGuoHallIteration(imageUnderProcess, 1);

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