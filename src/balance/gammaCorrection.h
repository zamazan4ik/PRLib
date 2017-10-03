//
// Created by zamazan4ik on 27.09.17.
//

#ifndef PRLIB_GAMMACORRECTION_H
#define PRLIB_GAMMACORRECTION_H

#include <opencv2/core/core.hpp>

namespace prl
{

void gammaCorrection(const cv::Mat& inputImage, cv::Mat& outputImage,
                     const double k, const double gamma);

}

#endif //PRLIB_GAMMACORRECTION_H
