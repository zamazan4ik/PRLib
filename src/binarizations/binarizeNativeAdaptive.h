#ifndef PRLIB_binarizeNativeAdaptive_h
#define PRLIB_binarizeNativeAdaptive_h

#include <opencv2/core/core.hpp>

namespace prl
{

/*!
 * \brief Native adaptive thresholding.
 * \param inputImageMat Image for binarization.
 * \param outputImageMat Resulting image.
 * \param isGaussianBlurReqiured \parblock Flag of usage Gaussian blur.
 * If this flag equals true then Gaussian blur is used.
 * Else median blur is used. \endparblock
 * \param medianBlurKernelSize Kernel size for median blur.
 * \param GaussianBlurKernelSize Kernel size for Gaussian blur.
 * \param GaussianBlurSigma Gaussian blur \f$\sigma\f$.
 * \param isAdaptiveThresholdCalculatedByGaussian \parblock
 * Flag of base of adaptive thresholding type.
 * If this flag equals true then Gaussian kernel is used.
 * Else mean kernel is used. \endparblock
 * \param adaptiveThresholdingMaxValue \parblock 
 * New value for pixel which intensity greater than threshold value.
 * \endparblock
 * \param adaptiveThresholdingBlockSize Kernel size for adaptive thresholding procedure.
 * \param adaptiveThresholdingShift Shifting value for adaptive thresholding procedure.
 * \param bilateralFilterBlockSize \parblock 
 * Kernel size for bilateral filtration. If less than 0 then filtration isn't used.
 * \endparblock
 * \param bilateralFilterColorSigma Sigma (\f$\sigma_r\f$) for intensity range.
 * \param bilateralFilterSpaceSigma Sigma (\f$\sigma_d\f$) for space range.
 * \details This function consists of next steps:
 * -# Median/Gaussian blur.
 * -# Adaptive threshold.
 * -# Bilateral filter.
 */
void binarizeNativeAdaptive(
		cv::Mat& inputImageMat, cv::Mat& outputImageMat,
		bool isGaussianBlurReqiured,
		int medianBlurKernelSize,
		int GaussianBlurKernelSize,
		double GaussianBlurSigma,
		bool isAdaptiveThresholdCalculatedByGaussian,
		double adaptiveThresholdingMaxValue,
		int adaptiveThresholdingBlockSize,
		double adaptiveThresholdingShift,
		int bilateralFilterBlockSize,
		double bilateralFilterColorSigma,
		double bilateralFilterSpaceSigma);

}
#endif // PRLIB_binarizeNativeAdaptive_h
