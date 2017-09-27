#ifndef PRLIB_thinZhangSuen_h
#define PRLIB_thinZhangSuen_h

#include <opencv2/core/core.hpp>

namespace prl
{

/**
 * @brief Code for thinning a binary image using Zhang-Suen algorithm.
 * 
 * @param inputImage Image for processing with range = [0;255].
 * @param outputImage Resulting image.
 * 
 */
void thinZhangSuen(cv::Mat& inputImage, cv::Mat& outputImage);
}
#endif // PRLIB_thinZhangSuen_h
