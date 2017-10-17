#ifndef PRLIB_DENOISESALTPEPPER_H
#define PRLIB_DENOISESALTPEPPER_H

#include <opencv2/core/core.hpp>

namespace prl
{
void denoiseSaltPepper(const cv::Mat& inputImage, cv::Mat& outputImage, int kernelSize, int times);
}

#endif //PRLIB_DENOISESALTPEPPER_H
