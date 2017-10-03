#ifndef PRLIB_basicDeblur_h
#define PRLIB_basicDeblur_h

#include <opencv2/core/core.hpp>

namespace prl
{
	void basicDeblur(const cv::Mat& inputImage, cv::Mat& outputImage,
					 size_t gaussianKernelSize = 0, double sigmaX = 9.0, double sigmaY = 0.0,
					 double imageWeight = 0.75);
}

#endif // PRLIB_basicDeblur_h
