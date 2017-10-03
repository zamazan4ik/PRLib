#include "BackgroundNormalizationFilter_Lepton.h"

using namespace IPL::Filtration;

#include "IPL_Exceptions/IPL_Exceptions.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "LeptonTools.h"


BackgroundNormalizationFilter_Lepton::BackgroundNormalizationFilter_Lepton(ProcessingProfile& profile) :
	BackgroundNormalizationFilter(profile)
{
}


BackgroundNormalizationFilter_Lepton::~BackgroundNormalizationFilter_Lepton(void)
{
}

void IPL::Filtration::BackgroundNormalizationFilter_Lepton::Modify(
	const RasterImage& inputImage, RasterImage& outputImage)
{
	cv::Mat inputImageMat;
	inputImage.ToMat(inputImageMat);

	if (inputImageMat.empty()) {
		throw IPL::Exceptions::IPL_ProcessingException_InvalidParameter(
			"Input image for flipping is empty");
	}

	cv::Mat outputImageMat;

	{
		PIX* pixs = leptCreatePixFromMat(&inputImageMat);
		
		/* Normalize for varying background */
		PIX* pixn = pixBackgroundNormSimple(pixs, nullptr, nullptr);

		pixDestroy(&pixs);

		outputImageMat = leptCreateMatFromPix(pixn);

		pixDestroy(&pixn);
	}

	outputImage.FromMat(outputImageMat);
}
