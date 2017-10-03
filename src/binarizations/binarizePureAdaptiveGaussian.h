#ifndef PureAdaptiveGaussianThresholding_OpenCV_h__
#define PureAdaptiveGaussianThresholding_OpenCV_h__

#include <opencv2/core/core.hpp>

namespace prl
{

void binarizePureAdaptiveGaussian(const cv::Mat& inputImage, cv::Mat& outputImage,
                                       const double maxValue, const int blockSize, const int shift);

}

#endif // PureAdaptiveGaussianThresholding_OpenCV_h__

