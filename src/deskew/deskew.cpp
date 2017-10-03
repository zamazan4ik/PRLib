#include "deskew.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#if defined(WIN32) || defined(_MSC_VER)
#include <windows.h>
#else

#include "compatibility.h"

#endif

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4305)
#	pragma warning(disable : 4005)
#endif // _MSC_VER

#include <leptonica/allheaders.h>

#ifdef _MSC_VER
#	pragma warning(pop)
#endif // _MSC_VER

#if defined(_WIN32) || defined(_MSC_VER)
#include <direct.h>
#include <codecvt>
#else

#include <unistd.h>

#endif //WIN32

#include <algorithm>
#include <functional>
#include <cctype>
#include <list>
#include <map>

#include <leptonica/pix.h>

#include "formatConvert.h"

#ifndef M_PI

#ifdef CV_PI
#define M_PI CV_PI
#else
#define M_PI 3.141592653
#endif

#endif // !M_PI

#include "formatConvert.h"
#include "utils.h"
#include "rotate.h"

double FindOrientation(const cv::Mat& input)
{
    PIX* pix = prl::ImgOpenCvToLepton(input);
    if (!pix)
    {
        return 0;
    }

    l_int32 iOrientation = 0;
    {
        l_float32 fUpConf;
        l_float32 fLeftConf;
        if (pixOrientDetectDwa(pix, &fUpConf, &fLeftConf, 0, 0) != 0)
        {
            if (pix)
            {
                pixDestroy(&pix);
            }

            return 0;
        }

        if (makeOrientDecision(fUpConf, fLeftConf, 0.0, 0.0, &iOrientation, 0) != 0)
        {
            if (pix)
            {
                pixDestroy(&pix);
            }

            return 0;
        }
    }

    double angle = 0;
    if (iOrientation == L_TEXT_ORIENT_UP)
    {
        angle = 0.0;
    }
    else if (iOrientation == L_TEXT_ORIENT_LEFT)
    {
        angle = 90.0;
    }
    else if (iOrientation == L_TEXT_ORIENT_DOWN)
    {
        angle = 180.0;
    }
    else if (iOrientation == L_TEXT_ORIENT_RIGHT)
    {
        angle = 270.0;
    }
    else
    { // if (iOrientation == L_TEXT_ORIENT_UNKNOWN)
        angle = 0.0;
    }

    pixDestroy(&pix);

    return angle;
}


double FindAngle(const cv::Mat& input_orig)
{
    // AA: we need black-&-white image here even if none of threshold algorithms were called before
    //cv::adaptiveThreshold(input, input, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 15, 5);
    cv::Mat input = input_orig.clone();

    cv::Size imgSize = input.size();
    cv::bitwise_not(input, input);
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(input, lines, 1, CV_PI / 180, 100, imgSize.width / 8.f, 20);
    cv::Mat disp_lines(imgSize, CV_8UC1, cv::Scalar(0, 0, 0));

    const int nb_lines = static_cast<int>(lines.size());
    if (!nb_lines)
    {
        return 0;
    }

    std::vector<double> cv_angles = std::vector<double>(nb_lines);

    for (int i = 0; i < nb_lines; ++i)
    {
        cv::line(
                disp_lines,
                cv::Point(lines[i][0], lines[i][1]),
                cv::Point(lines[i][2], lines[i][3]),
                cv::Scalar(255, 0, 0));

        cv_angles[i] = atan2(
                (double) lines[i][3] - lines[i][1],
                (double) lines[i][2] - lines[i][0]);
    }

    const double delta = 0.01; //difference is less than 1 deg.
    std::list<std::pair<double, int> > t_diff;

    for (std::vector<double>::iterator it = cv_angles.begin(); it != cv_angles.end(); ++it)
    {
        bool found = false;
        // bypass list
        for (std::list<std::pair<double, int>>::iterator elem = t_diff.begin();
             elem != t_diff.end(); ++elem)
        {
            if (prl::eq_d(*it, elem->first, delta))
            {
                elem->second++;
                found = true;
                break;
            }
        }
        if (!found)
        {
            std::pair<double, int> p(*it, 0);
            t_diff.push_back(p);
        }
    }

    std::pair<double, int> max_elem =
            *(std::max_element(t_diff.begin(), t_diff.end(),
                               [](const std::pair<double, int>& v1,
                                  const std::pair<double, int>& v2)
                               { return v1.second < v2.second; }));

    const double cv_angle = max_elem.first * 180 / M_PI;

    cv::bitwise_not(input, input);

    return cv_angle;
}

bool Deskew(cv::Mat& inputImage, cv::Mat& deskewedImage)
{
    CV_Assert(!inputImage.empty());

    cv::Mat processingImage;

    if (inputImage.channels() != 1)
    {
        cv::cvtColor(inputImage, processingImage, CV_BGR2GRAY);
    }
    else
    {
        processingImage = inputImage.clone();
    }

    //TODO: Should we use here another binarization algorithm?
    cv::threshold(processingImage, processingImage, 128, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

    double angle = FindAngle(processingImage);

    if ((angle != 0) && (angle <= DBL_MAX && angle >= -DBL_MAX))
    {
        prl::rotate(inputImage, deskewedImage, angle);
    }
    else
    {
        deskewedImage = inputImage.clone();
    }

    angle = FindOrientation(deskewedImage);

    if (angle != 0)
    {
        prl::rotate(deskewedImage, deskewedImage, angle);
    }

    if (deskewedImage.empty())
    {
        return false;
    }

    return true;
}