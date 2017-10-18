#include "autoCrop.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "resize.h"
#include "autoCropUtils.h"
#include "warp.h"

namespace prl
{

bool documentContour(
        cv::Mat& inputImage,
        const double scaleX, const double scaleY,
        std::vector<cv::Point2f>& resultContour)
{
    cv::Mat imageToProc;

    //! Store source image size
    cv::Size sourceImageSize(inputImage.size());

    cv::Size newImageSize;

    if (scaleX > 0 && scaleY > 0)
    {
        newImageSize = cv::Size(
                static_cast<int>(inputImage.cols * scaleX),
                static_cast<int>(inputImage.rows * scaleY)
        );

        cv::resize(inputImage, imageToProc, newImageSize, 0, 0, cv::INTER_AREA);

    }
    else
    {
        int longSide = std::max(inputImage.cols, inputImage.rows);

        int scaleFactorX = 1;
        int scaleFactorY = 1;

        imageToProc = inputImage.clone();

        while (longSide / 2 >= 256)
        {
            cv::pyrDown(imageToProc, imageToProc);

            longSide = std::max(imageToProc.cols, imageToProc.rows);
            scaleFactorX *= 2;
            scaleFactorY *= 2;
        }

        newImageSize = cv::Size(inputImage.cols / scaleFactorX, inputImage.rows / scaleFactorY);
    }


    // Find the most informative channel
    std::vector<double> mean, stddev;
    cv::meanStdDev(imageToProc, mean, stddev);

    size_t mostInfoChannelInd = std::distance(stddev.begin(), std::max_element(stddev.begin(), stddev.end()));

    std::vector<cv::Mat> channels;
    cv::split(imageToProc, channels);
    imageToProc = channels[mostInfoChannelInd];

    // Canny
    cv::Mat resultCanny;
    double upper = 50;
    double lower = 25;
    cv::Canny(imageToProc, resultCanny, lower, upper);

    bool isContourDetected = false;

    const size_t CountTry = 2;
    size_t DilateCoeff = 2;
    for (size_t i = 0; i < CountTry; ++i)
    {
        isContourDetected = findDocumentContour(resultCanny, resultContour);
        if (isContourDetected)
        {
            ScaleContour(resultContour, newImageSize, sourceImageSize);
            cropVerticesOrdering(resultContour);
            return true;
        }
        resultContour.clear();

        cv::dilate(
                resultCanny, resultCanny,
                cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)),
                cv::Point(-1, -1), DilateCoeff);
        cv::erode(
                resultCanny, resultCanny,
                cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)),
                cv::Point(-1, -1), DilateCoeff);

        ++DilateCoeff;
    }

    return false;
}

bool autoCrop(cv::Mat& inputImage, cv::Mat& outputImage)
{
    if (inputImage.empty())
    {
        throw std::invalid_argument("autoCrop: Image for cropping is empty");
    }

    switch (inputImage.type())
    {
        case CV_8UC3:
            break;
        case CV_8UC1:
            cv::cvtColor(inputImage, inputImage, CV_GRAY2BGR);
            break;
        default:
            if (inputImage.empty())
            {
                throw std::invalid_argument(
                        "autoCrop: Invalid type of image for cropping (required 8 or 24 bits per pixel)");
            }
            break;
    }

    std::vector<cv::Point2f> resultContour;

    //! Try to detect border
    if (!documentContour(inputImage, -1, -1, resultContour))
    {
        return false;
    }

    std::vector<cv::Point> temp(resultContour.begin(), resultContour.end());

    //! Crop area
    warpCrop(inputImage, outputImage, temp);

    return true;
}
}