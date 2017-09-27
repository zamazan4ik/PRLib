#ifndef PRLIB_PureAdaptive_OpenCV_h
#define PRLIB_PureAdaptive_OpenCV_h

#include <opencv2/core/core.hpp>

namespace prl
{

void binarizePureAdaptive(const cv::Mat& inputImage, cv::Mat& outputImage);

}
#endif // PRLIB_PureAdaptive_OpenCV_h

