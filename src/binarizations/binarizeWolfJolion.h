#ifndef PRLIB_binarizeWolfJolion_h
#define PRLIB_binarizeWolfJolion_h

#include <opencv2/core/core.hpp>

namespace prl
{

/*!
* \brief WolfJolion binarization algorithm implementation.
* \param imageCopy Image for processing.
* \param imageWolfJolion Resulting binary image.
* \param windowSize Size of sliding window.
* \param thresholdCoefficient Coefficient for threshold calculation.
* \param morphIterationCount Count of morphology operation in postprocessing.
* \details This function implements algorithm described in article
* "Comparison of Niblack inspired Binarization methods for ancient documents".
*/
void binarizeWolfJolion(
		cv::Mat& imageCopy, cv::Mat& imageWolfJolion,
		int windowSize = 101,
		double thresholdCoefficient = 0.01,
		int morphIterationCount = 2);
}
#endif // PRLIB_binarizeWolfJolion_h
