#ifndef MedianBlurATFilter_OpenCV_h__
#define MedianBlurATFilter_OpenCV_h__

#include <opencv2/core/core.hpp>

namespace prl
{

void binarizeAT(const cv::Mat& inputImage, cv::Mat& outputImage, const int medianKernelSize,
                     const double maxValue, const int blockSize, const int shift);

}

#endif // MedianBlurATFilter_OpenCV_h__

