#ifndef PRLIB_thinGuoHall_h
#define PRLIB_thinGuoHall_h

#include <opencv2/core/core.hpp>

namespace prl
{

/**
 * @brief Code for thinning a binary image using Guo Hall algorithm.
 * 
 * @param inputImage Image for processing with range = [0;255].
 * @param outputImage Resulting image.
 */
void thinGuoHall(cv::Mat& inputImage, cv::Mat& outputImage);
}
#endif // PRLIB_thinGuoHall_h
