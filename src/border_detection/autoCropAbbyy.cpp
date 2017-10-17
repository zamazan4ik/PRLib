#include "autoCropAbbyy.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "houghLine.h"
#include "resize.h"
#include "autoCropUtils.h"
#include "imageLibCommon.h"
#include "warp.h"


bool prl::getContourAbbyy(cv::Mat& src, std::vector<cv::Point>& resultContour)
{
    //TODO: https://habrahabr.ru/company/abbyy/blog/200448/
    //1) Resize with Gaussian smoothing to 256 at most
    cv::Mat imageToProc = src.clone();

    prl::resize(src, imageToProc, 0, 0, 256);

    //2) Choose the most informative channel

    //2.1) Split to channels
   /* std::vector<cv::Mat> channels(imageToProc.channels());
    cv::split(imageToProc, channels);

    //2.2) Calculate histograms
    std::vector<cv::MatND> hist(channels.size());
    int c = 0;
    for (size_t i = 0; i < channels.size(); ++i)
    {
        //cv::calcHist(channels[i], 1, cv::Mat(), hist[i], 2, {32, 32},std::vector<float>({0.0f, 256.0f}));
        cv::calcHist(channels[i], {0}, cv::Mat(), hist[i], std::vector<int>({32, 32}),
                     std::vector<float>({0.0f, 256.0f}));
    }*/


    //3) Do medianBlur
    /*for (size_t i = 0; i < 3; ++i)
    {
        cv::medianBlur(imageToProc, imageToProc, 3);
    }

    for (size_t i = 0; i < 3; ++i)
    {
        cv::medianBlur(imageToProc, imageToProc, 5);
    }

    //4) Run Canny Edge detector
    cv::Mat canny_output;
    cv::Canny(imageToProc, canny_output, 25, 50);

    //TODO: Remove findcontour later
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, CV_RETR_CCOMP,
                 CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    std::vector<std::vector<cv::Point>> contoursApproxed(contours.size());
    std::vector<cv::Point> result;
    double maxArea = 0.0;
    for (size_t i = 0; i < contours.size(); ++i)
    {
        cv::approxPolyDP(cv::Mat(contours[i]), contoursApproxed[i], contours[i].size() * 0.2, true);

        if (!isQuadrangle(contoursApproxed[i]))
        {
            continue;
        }

        double area = cv::contourArea(contoursApproxed[i]);
        if (area > maxArea)
        {
            maxArea = area;
            result = contoursApproxed[i];
        }
    }*/
    std::vector<cv::Point> result;
    prl::findHoughLineContour(imageToProc, result);

    cropVerticesOrdering(result);

    cv::Mat out;
    warpCrop(imageToProc, out, result);
    cv::imwrite("abbyy_out.jpg", out);

    resultContour = result;
    return true;
}
