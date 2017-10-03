#ifndef PRLIB_HoughLine_h__
#define PRLIB_HoughLine_h__

#include "opencv2/core/core.hpp"

namespace prl
{

bool houghLineContourDetector(
        cv::Mat* inputImage,
        const double scaleX, const double scaleY,
        const int nProcessedImageSize,
        int* points);

}
#endif // PRLIB_HoughLine_h__
