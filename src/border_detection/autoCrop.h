#ifndef PRLIB_SCANNED_DOCUMENT_IMAGE_AUTO_CROP_H_
#define PRLIB_SCANNED_DOCUMENT_IMAGE_AUTO_CROP_H_

#include <opencv2/core/core.hpp>

#include <vector>

namespace prl
{

/*!
 * \brief Get document contour 
 * \param[in] sourceImage Input image.
 * \param[out] resultContour Document contour.
 * \return true if documents borders was detected.
 * \details This function detects document borders on base of Canny edge detector, 
 * local variance values estimation, text lines detection and feature detection.
 */
bool GetDocumentContour(
        cv::Mat& sourceImage,
        const double scaleX, const double scaleY,
        std::vector<cv::Point2f>& resultContour,
        bool useSimpleOnly = false,
        int* successMethodNumber = NULL);

bool GetDocumentContour(
        cv::Mat& sourceImage,
        const int nProcessedImageSize,
        std::vector<cv::Point2f>& resultContour,
        bool useSimpleOnly = false,
        int* successMethodNumber = NULL);

bool GetDocumentContour(
        cv::Mat& sourceImage,
        const double scaleX, const double scaleY,
        const int nProcessedImageSize,
        std::vector<cv::Point2f>& resultContour,
        bool useSimpleOnly = false,
        int* successMethodNumber = NULL);

/*!
 * \brief Transform image to crop document.
 * \param[in] sourceImage Input image.
 * \param[out] destImage Output image.
 * \param[in] points Document contour.
 * \details Crop image and do warp transformation to rectangle. 
 * Calculates aspect ratio of source image and keeps it after crop.
 */
void warpCropAI(
        cv::Mat& sourceImage,
        cv::Mat& destImage,
        const std::vector<cv::Point2f>& points,
        int borderMode = cv::BORDER_CONSTANT, const cv::Scalar& borderValue = cv::Scalar());

/*!
 * \brief Transform image to crop document.
 * \param[in] sourceImage Input image.
 * \param[out] destImage Output image.
 * \param[in] x0 Top left x coordinate.
 * \param[in] y0 Top left y coordinate.
 * \param[in] x1 Top right x coordinate.
 * \param[in] y1 Top right y coordinate.
 * \param[in] x2 Bottom right x coordinate.
 * \param[in] y2 Bottom right y coordinate.
 * \param[in] x3 Bottom left x coordinate.
 * \param[in] y3 Bottom left y coordinate.
 * \details Crop image and do warp transformation to rectangle. 
 * Calculates aspect ratio of source image and keeps it after crop.
 */
void warpCropAI(
        cv::Mat& sourceImage, cv::Mat& destImage,
        int x0, int y0,
        int x1, int y1,
        int x2, int y2,
        int x3, int y3,
        int borderMode = cv::BORDER_CONSTANT, const cv::Scalar& borderValue = cv::Scalar());

/*!
 * \brief Crop scanned document.
 * \param[in] sourceImage Input image.
 * \param[out] croppedImage Cropped image.
 * \return true if image cropped.
 */
bool ScannedDocumentImageAutoCrop(cv::Mat& sourceImage, cv::Mat& croppedImage);

}
#endif // PRLIB_SCANNED_DOCUMENT_IMAGE_AUTO_CROP_H_
