#ifndef PRLIB_AUTOCROPABBYY_HPP
#define PRLIB_AUTOCROPABBYY_HPP

#include <opencv2/core/core.hpp>

#include <vector>

namespace prl
{
/*!
* \brief Get document contour
* \param[in] sourceImage Input image.
* \param[out] resultContour Document contour.
* \return true if documents borders was detected.
* \details This function detects document borders on base of Abbyy
*  border detection algorithm.
*/
bool getContourAbbyy(
        cv::Mat& sourceImage,
        std::vector<cv::Point>& resultContour);
}

#endif //PRLIB_AUTOCROPABBYY_HPP
