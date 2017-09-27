#include "binarizeAGT.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


void prl::binarizeAGT(const cv::Mat& inputImage, cv::Mat& outputImage)
{
    cv::Mat inputImageMat;
    inputImage.ToMat(inputImageMat);

    if (inputImageMat.empty())
    {
        throw std::invalid_argument("Input image for binarization is empty");
    }

    if (!this->m_IsBinarizationByChannelRequired)
    {
        if (inputImageMat.channels() != 1)
        {
            cv::cvtColor(inputImageMat, inputImageMat, CV_BGR2GRAY);
        }
    }

    cv::Mat outputImageMat;
    cv::Mat tempOutputImageMat;

    if (this->IsNewImageRequired())
    {
        tempOutputImageMat = inputImageMat;
    }

    cv::medianBlur(inputImageMat, tempOutputImageMat, this->m_KernelSize);

    if (inputImageMat.channels() != 1)
    {
        cv::cvtColor(tempOutputImageMat, outputImageMat, CV_BGR2GRAY);
    }
    else
    {
        if (this->IsNewImageRequired())
        {
            outputImageMat = tempOutputImageMat.clone();
        }
        else
        {
            outputImageMat = tempOutputImageMat;
        }
    }

    cv::adaptiveThreshold(
            outputImageMat, outputImageMat,
            this->m_MaxValue,
            cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY,
            this->m_BlockSize, this->m_Shift);

    outputImage.FromMat(outputImageMat);
}
