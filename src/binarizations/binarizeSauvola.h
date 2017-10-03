#ifndef PRLIB_binarizeSauvola_h
#define PRLIB_binarizeSauvola_h

#include <opencv2/core/core.hpp>

namespace prl
{

/*!
* \brief Sauvola binarization algorithm implementation.
* \param imageCopy Image for processing.
* \param imageSauvola Resulting binary image.
* \param windowSize Size of sliding window.
* \param thresholdCoefficient Coefficient for threshold calculation.
* \param morphIterationCount Count of morphology operation in postprocessing.
* \details This function implements algorithm described in article
* "Efficient Implementation of Local Adaptive Thresholding Techniques Using Integral Images".
*/
void binarizeSauvola(
		cv::Mat& imageCopy, cv::Mat& imageSauvola,
		int windowSize = 101,
		double thresholdCoefficient = 0.01,
		int morphIterationCount = 2);
}
#endif // PRLIB_binarizeSauvola_h
