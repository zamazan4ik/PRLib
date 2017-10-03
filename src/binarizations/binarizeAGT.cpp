#include "binarizeAGT.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


void prl::binarizeAGT(const cv::Mat& inputImage, cv::Mat& outputImage, const int medianKernelSize,
                      const double maxValue, const int blockSize, const int shift)
{
    cv::Mat inputImageMat = inputImage;

    if (inputImageMat.empty())
    {
        throw std::invalid_argument("Input image for binarization is empty");
    }

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
            cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY,
            blockSize, shift);

    outputImage = outputImageMat;
}
