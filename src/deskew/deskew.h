#ifndef PRLIB_deskew_h
#define PRLIB_deskew_h

#include <opencv2/core/core.hpp>

namespace prl
{

/**
 * @brief Deskew image of document.
 * @param inputImage Image for deskewing.
 * @param deskewedImage Deskewed image.
 * @return true if processing successful.
 *
 * \note Implementation of this procedure is based on
 * <a href="http://www.leptonica.com/">Leptonica library</a>.
 */
bool Deskew(cv::Mat& inputImage, cv::Mat& deskewedImage);

/**
 * @brief Find orientation of an image.
 * @param inputImage Image for detecting.
 * @return Angle which describes orientation.
 *
 * \note Implementation of this procedure is based on
 * <a href="http://www.leptonica.com/">Leptonica library</a>.
 */
double FindOrientation(const cv::Mat& input);

/**
 * @brief Find angle of an image.
 * @param inputImage Image for deskewing.
 * @return Angle of an image.
 *
 * \note Implementation of this procedure is based on
 * <a href="http://www.leptonica.com/">Leptonica library</a>.
 */
double FindAngle(const cv::Mat& input_orig);
}

#endif // PRLIB_deskew_h
