#ifndef PRLIB_REMOVEDOTS_H
#define PRLIB_REMOVEDOTS_H

#include <opencv2/core/core.hpp>

namespace prl
{
void removeDots(const cv::Mat& inputImage, cv::Mat& outputImage);
}

#endif //PRLIB_REMOVEDOTS_H
