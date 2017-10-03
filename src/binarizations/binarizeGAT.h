#ifndef PRLIB_MedianBlurGATFilter_OpenCV_h
#define PRLIB_MedianBlurGATFilter_OpenCV_h

#include <opencv2/core/core.hpp>

namespace prl
{

void binarizeGAT(const cv::Mat& inputImage, cv::Mat& outputImage, const int gaussianKernelSize,
                      const double sigmaX, const double sigmaY,
                      const double maxValue, const int blockSize, const int shift);

}

#endif // PRLIB_MedianBlurGATFilter_OpenCV_h

