#include "binarizePureAdaptiveGaussian.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdexcept>

void prl::binarizePureAdaptiveGaussian(const cv::Mat& inputImage, cv::Mat& outputImage,
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

    if (inputImageMat.channels() != 1)
    {
        cv::cvtColor(inputImageMat, outputImageMat, CV_BGR2GRAY);
    }
    /*else
    {
        if (this->IsNewImageRequired())
        {
            outputImageMat = inputImageMat.clone();
        }
        else
        {
            outputImageMat = inputImageMat;
        }
    }*/

    cv::adaptiveThreshold(
            outputImageMat, outputImageMat,
            maxValue,
            cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY,
            blockSize, shift);

    outputImage = outputImageMat;
}