#include "BaseDeblurFilter_OpenCV.h"

using namespace IPL::Filtration;

#include "IPL_Exceptions/IPL_Exceptions.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

BaseDeblurFilter_OpenCV::BaseDeblurFilter_OpenCV(ProcessingProfile& profile) :
	BaseDeblurFilter(profile)
{
}


BaseDeblurFilter_OpenCV::~BaseDeblurFilter_OpenCV(void)
{
}

void IPL::Filtration::BaseDeblurFilter_OpenCV::Deblur(
	const RasterImage& inputImage, RasterImage& outputImage)
{
	cv::Mat inputImageMat;
	inputImage.ToMat(inputImageMat);

	if (inputImageMat.empty()) {
		throw IPL::Exceptions::IPL_ProcessingException_InvalidParameter(
			"Input image for deblurring is empty");
	}

	cv::Mat outputImageMat;

	std::vector<cv::Mat> channels;
	cv::split(inputImageMat, channels);

	for (cv::Mat& channel : channels) {
		cv::Mat channelfloat;
		channel.convertTo(channelfloat, CV_32F);
		cv::GaussianBlur(
			channelfloat, channel, 
			cv::Size(this->m_GaussianKernelSize, this->m_GaussianKernelSize), 
			this->m_SigmaX, this->m_SigmaY);
		cv::addWeighted(
			channelfloat, 2.0 * this->m_SourceImageWeight, 
			channel, 2.0 * this->m_SourceImageWeight - 2.0, 
			0.0, 
			channel);

		// convert back to 8bits gray scale
		channel.convertTo(channel, CV_8U);
	}

	cv::merge(channels, outputImageMat);

	outputImage.FromMat(outputImageMat);
}
