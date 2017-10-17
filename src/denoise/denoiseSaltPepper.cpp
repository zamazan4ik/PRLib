#include "denoiseSaltPepper.h"

#include <opencv2/imgproc/imgproc.hpp>

void prl::denoiseSaltPepper(const cv::Mat& inputImage, cv::Mat& outputImage, int kernelSize, int times)
{
    outputImage = inputImage.clone();
    for(size_t i = 0; i < times; ++i)
    {
        cv::medianBlur(outputImage, outputImage, kernelSize);
    }
}