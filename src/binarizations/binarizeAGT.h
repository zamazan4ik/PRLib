#ifndef PRLIB_binarizeAGT_h
#define PRLIB_binarizeAGT_h

#include <opencv2/core/core.hpp>

namespace prl
{
void binarizeAGT(const cv::Mat& inputImage, cv::Mat& outputImage, const int medianKernelSize,
                 const double maxValue, const int blockSize, const int shift);
}

#endif // PRLIB_binarizeAGT_h

