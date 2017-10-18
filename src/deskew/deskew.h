#ifndef PRLIB_deskew_h
#define PRLIB_deskew_h

#include <opencv2/core/core.hpp>

namespace prl
{

/**
 * @brief Deskew image of document.
 * @param inputImage Image for deskewing.
 * @param outputImage Deskewed image.
 * @return true if processing successful.
 *
 * \note Implementation of this procedure is based on
 * <a href="http://www.leptonica.com/">Leptonica library</a>.
 */
bool deskew(cv::Mat& inputImage, cv::Mat& outputImage);

/**
 * @brief Find orientation of an image.
 * @param inputImage Image for detecting.
 * @return Angle which describes orientation.
 *
 * \note Implementation of this procedure is based on
 * <a href="http://www.leptonica.com/">Leptonica library</a>.
 */
double findOrientation(const cv::Mat& inputImage);

/**
 * @brief Find angle of an image.
 * @param inputImage Image for deskewing.
 * @return Angle of an image.
 *
 * \note Implementation of this procedure is based on
 * <a href="http://www.leptonica.com/">Leptonica library</a>.
 */
double findAngle(const cv::Mat& inputImage);
}

#endif // PRLIB_deskew_h
