//
// Created by zamazan4ik on 27.09.17.
//

#ifndef PRLIB_BALANCESIMPLEWHITE_H
#define PRLIB_BALANCESIMPLEWHITE_H

#include <opencv2/core/core.hpp>

namespace prl
{

void simpleWhiteBalance(const cv::Mat& inputImage, cv::Mat& outputImage,
                        const double k);

}

#endif //PRLIB_BALANCESIMPLEWHITE_H
