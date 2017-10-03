//
// Created by zamazan4ik on 27.09.17.
//

#ifndef PRLIB_BalanceGrayWorldWhite_h
#define PRLIB_BalanceGrayWorldWhite_h

#include <opencv2/core/core.hpp>

namespace prl
{

void grayWorldWhiteBalance(const cv::Mat& inputImage, cv::Mat& outputImage,
                           const double pNorm, const bool withMax);

}

#endif //PRLIB_BalanceGrayWorldWhite_h
