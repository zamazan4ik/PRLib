#ifndef PRLIB_COCOCLUST_Binarizator_h
#define PRLIB_COCOCLUST_Binarizator_h

#include <opencv2/core/core.hpp>

#include "imageLibCommon.h"

namespace prl
{

/*!
 * \brief binarizeCOCOCLUST
 * \param[in] inputImage Input image.
 * \param[out] outputImage Output image.
 * \param[in] T_S Threshold for preliminary clustering.
 * \param[in] CLAHEClipLimit \parblock
 * Parameter of local contrast enhancement procedure
 * (if less or equal than 0 then procedure is not used).
 * \endparblock
 * \param[in] GaussianBlurKernelSize kernel size of Gaussian blur procedure.
 * \param[in] CannyUpperThresholdCoeff Coefficient for upper threshold of Canny edge detector.
 * \param[in] CannyLowerThresholdCoeff Coefficient for lower threshold of Canny edge detector.
 * \param[in] CannyMorphIters \parblock
 * Parameter of erode and dilatation (if equal than 0 then procedures are not used).
 * \endparblock
 */
void binarizeCOCOCLUST(cv::Mat& inputImage, cv::Mat& outputImage,
                       const float T_S = 45,
                       double CLAHEClipLimit = 3.0,
                       int GaussianBlurKernelSize = 19,
                       double CannyUpperThresholdCoeff = 0.15,
                       double CannyLowerThresholdCoeff = 0.05,
                       int CannyMorphIters = 4);
}
#endif // PRLIB_COCOCLUST_Binarizator_h
