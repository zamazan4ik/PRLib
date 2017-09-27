//
// Created by zamazan4ik on 23.09.17.
//

#ifndef PRLIB_UTILS_HPP
#define PRLIB_UTILS_HPP

namespace prl
{

static const double EQ_DELTA = 1e-7;

bool eq_d(const double v1, const double v2, const double delta = EQ_DELTA);

double compareImages(const cv::Mat& image1, const cv::Mat& image2);

}
#endif //PRLIB_UTILS_HPP
