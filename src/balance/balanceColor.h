#ifndef PRLIB_balanceColor_h
#define PRLIB_balanceColor_h

#include <opencv2/core/core.hpp>

namespace prl
{

void colorBalance(const cv::Mat& inputImage, cv::Mat& outputImage,
                  double colorBalanceGamma, double saturationGamma);

}
#endif //PRLIB_balanceColor_h
