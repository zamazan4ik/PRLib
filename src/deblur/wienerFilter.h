#ifndef WienerFilterImpl_h__
#define WienerFilterImpl_h__

#include <opencv2/core/core.hpp>

void WienerFilterImpl(
	cv::Mat& inputImage, 
	cv::Mat& outputImage, 
	int filterKernelWidth = 11, int filterKernelHeight = 11, 
	double coeffWiener = 1,
	double sigmaGauss = 5);

#endif // WienerFilterImpl_h__
