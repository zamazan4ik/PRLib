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
 * \brief Crop scanned document.
 * \param[in] sourceImage Input image.
 * \param[out] croppedImage Cropped image.
 * \return true if image cropped.
 */
bool ScannedDocumentImageAutoCrop(cv::Mat& sourceImage, cv::Mat& croppedImage);

}
#endif // PRLIB_SCANNED_DOCUMENT_IMAGE_AUTO_CROP_H_
