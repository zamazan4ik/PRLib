#ifndef PRLIB_CORRECTNUIL_H
#define PRLIB_CORRECTNUIL_H

#include <opencv2/core/core.hpp>

namespace prl
{
void correctNUIL(const cv::Mat& inputImage, cv::Mat& outputImage, int structuringElementSize = 31);
}

#endif //PRLIB_CORRECTNUIL_H
