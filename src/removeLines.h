#ifndef PRLIB_REMOVELINES_H
#define PRLIB_REMOVELINES_H

#include <opencv2/core/core.hpp>

namespace prl
{
void removeLines(const cv::Mat& inputImage, cv::Mat& outputImage);
}

#endif //PRLIB_REMOVELINES_H
