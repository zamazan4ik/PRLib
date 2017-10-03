#ifndef NICKBinarizerImpl_h__
#define NICKBinarizerImpl_h__

#include <opencv2/core/core.hpp>

namespace prl
{

/*!
* \brief NICK binarization algorithm implementation.
* \param imageCopy Image for processing.
* \param imageNICK Resulting binary image.
* \param windowSize Size of sliding window.
* \param thresholdCoefficient Coefficient for threshold calculation.
* \param morphIterationCount Count of morphology operation in postprocessing.
* \details This function implements algorithm described in article
* "Comparison of Niblack inspired Binarization methods for ancient documents".
*/
void binarizeNICK(
		cv::Mat& imageCopy, cv::Mat& imageNICK,
		int windowSize = 21,
		double thresholdCoefficient = -0.01,
		int morphIterationCount = 0);

}
#endif // NICKBinarizerImpl_h__
