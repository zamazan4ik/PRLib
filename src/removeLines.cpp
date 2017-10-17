#include "removeLines.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

void prl::removeLines(const cv::Mat& inputImage, cv::Mat& outputImage)
{
    cv::Mat gray;
    if (inputImage.channels() == 3)
    {
        cvtColor(inputImage, gray, CV_BGR2GRAY);
    }
    else
    {
        gray = inputImage;
    }

    cv::Mat bw;
    cv::adaptiveThreshold(~gray, bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 15, -2);

    // Create the images that will use to extract the horizonta and vertical lines
    cv::Mat horizontal = bw.clone();
    cv::Mat vertical = bw.clone();

    // Specify size on horizontal axis
    int horizontalsize = horizontal.cols / 30;

    // Create structure element for extracting horizontal lines through morphology operations
    cv::Mat horizontalStructure = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(horizontalsize,1));

    // Apply morphology operations
    cv::erode(horizontal, horizontal, horizontalStructure, cv::Point(-1, -1));
    cv::dilate(horizontal, horizontal, horizontalStructure, cv::Point(-1, -1));

    // Specify size on vertical axis
    int verticalsize = vertical.rows / 30;

    // Create structure element for extracting vertical lines through morphology operations
    cv::Mat verticalStructure = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, verticalsize));

    // Apply morphology operations
    cv::erode(vertical, vertical, verticalStructure, cv::Point(-1, -1));
    cv::dilate(vertical, vertical, verticalStructure, cv::Point(-1, -1));

    outputImage = bw.clone();
    outputImage = outputImage - horizontal;
    outputImage = outputImage - vertical;

    cv::bitwise_not(outputImage, outputImage);
}
