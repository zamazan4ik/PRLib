#ifndef NiblackBinarizerImpl_h__
#define NiblackBinarizerImpl_h__

#include <opencv2/core/core.hpp>

namespace prl
{

/*!
* \brief Niblack binarization algorithm implementation.
* \param imageCopy Image for processing.
* \param imageNiblack Resulting binary image.
* \param windowSize Size of sliding window.
* \param thresholdCoefficient Coefficient for threshold calculation.
* \param morphIterationCount Count of morphology operation in postprocessing.
* \details This function implements algorithm described in article
* "Comparison of Niblack inspired Binarization methods for ancient documents".
*/
void binarizeNiblack(
		cv::Mat& imageCopy, cv::Mat& imageNiblack,
		int windowSize,
		double thresholdCoefficient,
		int morphIterationCount);

}
#endif // NiblackBinarizerImpl_h__
