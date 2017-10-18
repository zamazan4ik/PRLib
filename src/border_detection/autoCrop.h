#ifndef PRLIB_SCANNED_DOCUMENT_IMAGE_AUTO_CROP_H_
#define PRLIB_SCANNED_DOCUMENT_IMAGE_AUTO_CROP_H_

#include <opencv2/core/core.hpp>

#include <vector>

namespace prl
{

/*!
 * \brief Crop scanned document.
 * \param[in] sourceImage Input image.
 * \param[out] croppedImage Cropped image.
 * \return true if image cropped.
 */
bool ScannedDocumentImageAutoCrop(cv::Mat& sourceImage, cv::Mat& croppedImage);

}
#endif // PRLIB_SCANNED_DOCUMENT_IMAGE_AUTO_CROP_H_
