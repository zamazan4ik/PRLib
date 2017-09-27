#ifndef CIB_SCANNED_DOCUMENT_IMAGE_BORDER_DETECTION_UTILS_H_
#define CIB_SCANNED_DOCUMENT_IMAGE_BORDER_DETECTION_UTILS_H_

#include <opencv2/core/core.hpp>
#include <vector>


/*!
 * \def GOOD_RECTANGLE
 * \brief Code of correct rectangle.
 */
#define GOOD_RECTANGLE (0)

/*!
 * \def BAD_RECTANGLE
 * \brief Code of incorrect rectangle.
 */
#define BAD_RECTANGLE (-1)

/*!
 * \typedef MarkerRectangle
 * \struct tag_MarkerRectangle
 * \brief Detected marker rectangle.
 */
typedef struct tag_MarkerRectangle {
	/** Outer corners positions of rectangle. */
	cv::Point2f outerCorners[4];
	/** Outer degrees values. */
	double outerDegrees[4];

} MarkerRectangle;

//! Get angle between vectors.
double dotDegree(cv::Point &common, cv::Point &a, cv::Point &b);

/*!
 * \brief Check rectangle
 * \param[in] contour Contour.
 * \param[out] rectangle Resulted rectangle.
 * \return if contour is rectangle then GOOD_RECTANGLE else BAD_RECTANGLE.
 */
int CheckRectangle(
	vector<cv::Point> &contour, 
	MarkerRectangle* rectangle);

/*!
 * \brief Try to detect document contour.
 * \param[in] source Binary image.
 * \param[out] resultContour Resulted document contour.
 * \return true if contour detected.
 */
bool findDocumentContour(
	cv::Mat &source, 
	vector<cv::Point2f> &resultContour);

/*!
 * \brief Binarization on base of local variance estimation.
 * \param image Input image.
 * \param result Binarized image.
 */
void binarizeByLocalVariances(cv::Mat &image, cv::Mat &result);
//void binarizeByLocalVariances_without_filters(cv::Mat &image, cv::Mat &result);




void ScaleContour(vector<cv::Point2f> &contour, cv::Size &fromImageSize, cv::Size &toImageSize);
bool cropVerticesOrdering(vector<cv::Point2f> &pt);


#endif // CIB_SCANNED_DOCUMENT_IMAGE_BORDER_DETECTION_UTILS_H_
