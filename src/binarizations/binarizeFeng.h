#ifndef PRLIB_FengBinarizerImpl_h
#define PRLIB_FengBinarizerImpl_h

#include <opencv2/core/core.hpp>

namespace prl
{

/*!
* \brief Feng binarization algorithm implementation.
* \param imageCopy Image for processing.
* \param imageFeng Resulting binary image.
* \param windowSize Size of sliding window.
* \param thresholdCoefficient_alpha1 Coefficient \f$\alpha_1\f$ for threshold calculation.
* \param thresholdCoefficient_k1 Coefficient \f$k_1\f$ for threshold calculation.
* \param thresholdCoefficient_k2 Coefficient \f$k_2\f$ for threshold calculation.
* \param thresholdCoefficient_gamma Coefficient \f$\gamma\f$ for threshold calculation.
* \param morphIterationCount Count of morphology operation in postprocessing.
* \details This function implements algorithm described in article
* "Comparison of Niblack inspired Binarization methods for ancient documents".
*/
void binarizeFeng(
		cv::Mat& imageCopy, cv::Mat& imageFeng,
		int windowSize,
		double thresholdCoefficient_alpha1,
		double thresholdCoefficient_k1,
		double thresholdCoefficient_k2,
		double thresholdCoefficient_gamma,
		int morphIterationCount);

}
#endif // PRLIB_FengBinarizerImpl_h
