#include "binarizeAT.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdexcept>

void prl::binarizeAT(const cv::Mat& inputImage, cv::Mat& outputImage, const int medianKernelSize,
                     const double maxValue, const int blockSize, const int shift)
{
    cv::Mat inputImageMat = inputImage;

    if (inputImageMat.empty())
    {
        throw std::invalid_argument("Input image for binarization is empty");
    }

    /*if (!this->m_IsBinarizationByChannelRequired)
    {
        if (inputImageMat.channels() != 1)
        {
            cv::cvtColor(inputImageMat, inputImageMat, CV_BGR2GRAY);
        }
    }*/

    cv::Mat outputImageMat;
    cv::Mat tempOutputImageMat;

    cv::medianBlur(inputImageMat, tempOutputImageMat, medianKernelSize);

    if (inputImageMat.channels() != 1)
    {
        cv::cvtColor(tempOutputImageMat, outputImageMat, CV_BGR2GRAY);
    }

    cv::adaptiveThreshold(
            outputImageMat, outputImageMat,
            maxValue,
            cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY,
            blockSize, shift);

    outputImage = outputImageMat;
}
