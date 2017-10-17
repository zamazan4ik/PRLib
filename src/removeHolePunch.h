#ifndef PRLIB_REMOVEHOLEPUNCH_H
#define PRLIB_REMOVEHOLEPUNCH_H

#include <opencv2/core/core.hpp>

namespace prl
{
void removeHolePunch(const cv::Mat& inputImage, cv::Mat& outputImage);
}

#endif //PRLIB_REMOVEHOLEPUNCH_H
