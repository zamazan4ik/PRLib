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

bool GetDocumentContour(
        cv::Mat &sourceImage,
        const double scaleX, const double scaleY,
        std::vector<cv::Point2f> &resultContour)
{
    cv::Mat imageToProc;

    //! Store source image size
    cv::Size sourceImageSize(sourceImage.size());

    cv::Size newImageSize;

    if (scaleX > 0 && scaleY > 0) {
        newImageSize = cv::Size(
                static_cast<int>(sourceImage.cols * scaleX),
                static_cast<int>(sourceImage.rows * scaleY)
        );

        cv::resize(sourceImage, imageToProc, newImageSize, 0, 0, cv::INTER_AREA);

    } else {
        int longSide = std::max(sourceImage.cols, sourceImage.rows);

        int scaleFactorX = 1;
        int scaleFactorY = 1;

        imageToProc = sourceImage.clone();

        while (longSide / 2 >= nProcessedImageSize) {
            cv::pyrDown(imageToProc, imageToProc);

            longSide = std::max(imageToProc.cols, imageToProc.rows);
            scaleFactorX *= 2;
            scaleFactorY *= 2;
        }

        newImageSize = cv::Size(sourceImage.cols / scaleFactorX, sourceImage.rows / scaleFactorY);
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
    for(size_t i = 0; i < CountTry; ++i)
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
                cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2,2)),
                cv::Point(-1, -1), DilateCoeff);
        cv::erode(
                resultCanny, resultCanny,
                cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2,2)),
                cv::Point(-1, -1), DilateCoeff);

        ++DilateCoeff;
    }

    return false;
}

bool ScannedDocumentImageAutoCrop(cv::Mat& sourceImage, cv::Mat& croppedImage)
{
    if (sourceImage.empty())
    {
        throw std::invalid_argument("ScannedDocumentImageAutoCrop: Image for cropping is empty");
    }

    switch (sourceImage.type())
    {
        case CV_8UC3:
            break;
        case CV_8UC1:
            cvtColor(sourceImage, sourceImage, CV_GRAY2BGR);
            break;
        default:
            if (sourceImage.empty())
            {
                throw std::invalid_argument(
                        "ScannedDocumentImageAutoCrop: Invalid type of image for cropping (required 8 or 24 bits per pixel)");
            }
            break;
    }

    std::vector<cv::Point2f> resultContour;

    //! Try to detect border
    if (!GetDocumentContour(sourceImage, -1, -1, resultContour))
    {
        return false;
    }

    //! Sort points
    /*if (!cropVerticesOrdering(resultContour)) {
        cout << _TXT("Document border contour points is not sorted.") << endl;
        return false;
    }*/

    std::vector<cv::Point> temp(resultContour.begin(), resultContour.end());

    //! Crop area
    warpCrop(sourceImage, croppedImage, temp);

    return true;
}
}