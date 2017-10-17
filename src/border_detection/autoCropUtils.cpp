#include "autoCropUtils.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "imageLibCommon.h"

//////////////////////////////////////////////////////////////////////////


//! Calculate degree between two vectors
double dotDegree(cv::Point& common, cv::Point& a, cv::Point& b)
{
    cv::Point2d vecA = cv::Point(a.x - common.x, a.y - common.y);
    cv::Point2d vecB = cv::Point(b.x - common.x, b.y - common.y);

    double normA = sqrt(vecA.x * vecA.x + vecA.y * vecA.y);
    double normB = sqrt(vecB.x * vecB.x + vecB.y * vecB.y);

    vecA.x = vecA.x / normA;
    vecA.y = vecA.y / normA;

    vecB.x = vecB.x / normB;
    vecB.y = vecB.y / normB;

    double costheta = (vecA.x * vecB.x + vecA.y * vecB.y);

    double rad = acos(costheta);
    const double PI = CV_PI;
    double degree = rad * 180 / PI;

    return degree;
}

//! Function checks whether polygon rectangle
int CheckRectangle(std::vector<cv::Point>& contour, MarkerRectangle* rectangle)
{
    //! Check count of corners
    if (contour.size() != 4)
    {
        return BAD_RECTANGLE;
    }

    double capaDeg = 25;
    double upperThreshDegree = 180 - capaDeg;
    double lowerThreshDegree = capaDeg;

    double degrees[4];

    std::vector<cv::Point>& corners = contour;

    //! Get degree for each corner
    for (size_t i = 0; i < 4; ++i)
    {
        int next = (i + 1) % 4;
        int prev = (i - 1);
        if (prev == -1)
        {
            prev = 3;
        }
        degrees[i] = dotDegree(corners[i], corners[next], corners[prev]);
    }

    //! Check value of degrees
    int passCount = 0;
    for (size_t i = 0; i < 4; ++i)
    {
        if ((degrees[i] <= upperThreshDegree) &&
            (degrees[i] >= lowerThreshDegree))
        {
            ++passCount;
        }
    }

    if (passCount == 4)
    {
        //! If all degrees are valid then fill output rectangle
        for (size_t i = 0; i < 4; ++i)
        {
            rectangle->outerCorners[i] = cv::Point2f(static_cast<float>(corners[i].x),
                                                     static_cast<float>(corners[i].y));
            rectangle->outerDegrees[i] = degrees[i];
        }
        return GOOD_RECTANGLE;
    }
    else
    {
        // not rectangle
        return BAD_RECTANGLE;
    }
}

/////////////////////////////////////////////////////////////////

bool IsQuadrangularConvex(std::vector<cv::Point>& resultContour)
{
    if (resultContour.empty())
    {
        return false;
    }

    //! Find convex hull of points
    std::vector<cv::Point> convexHullPoints;
    cv::convexHull(cv::Mat(resultContour), convexHullPoints, false);

    return (convexHullPoints.size() == 4);
}


//bool findDocumentContour(cv::Mat &source, vector<cv::Point2f> &resultContour)
//{
//	if (source.empty()) {
//		throw IPL::Exceptions::IPL_ProcessingException_InvalidParameter(
//			_TXT("Input image for contours detection is empty"));
//	}
//
//	vector<vector<cv::Point> > contours;
//	vector<cv::Vec4i> hierarchy;
//
//	cv::findContours(
//		source, contours, hierarchy, 
//		CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, 
//		cv::Point(0, 0));
//
//	double maxArea = source.cols * source.rows * 0.2;
//
//	//! Approximate contours to polygons + get bounding rectangles and circles
//	vector<vector<cv::Point> > contoursPolygons(contours.size());
//	vector<cv::RotatedRect> minRect(contours.size());
//
//	vector<MarkerRectangle> markerRectangles;
//
//	float maxRotatedRectArea = 0;
//	int maxRotatedRectAreaPos = -1;
//
//	const double lengthCriteria = (source.cols + source.rows) * 1.8;
//
//	for (size_t i = 0; i < contours.size(); ++i) {
//		//convexHull( Mat(contours[i]), contours[i], false );
//
//		//! Approximate contour by polygon
//		cv::approxPolyDP(cv::Mat(contours[i]), contoursPolygons[i], contours[i].size() * 0.2, true);
//
//		//! Get minimal area rectangle for contour
//		minRect[i] = cv::minAreaRect(cv::Mat(contoursPolygons[i]));
//
//		//if (minRect[i].size.area() < maxArea) {
//		//	continue;
//		//}
//
//		bool isContoursPolygonTooBig = 
//			arcLength(contoursPolygons[i], true) > lengthCriteria;
//		bool isContoursPolygonTooSmall = arcLength(contoursPolygons[i], true) < 100;
//		bool hasContoursPolygon4Conrners = contoursPolygons[i].size() == 4;
//		if ( isContoursPolygonTooBig || isContoursPolygonTooSmall ||
//			!hasContoursPolygon4Conrners ) {
//
//				continue;
//		}
//
//		MarkerRectangle markerRectangle;
//
//		//! Check whether approximation polygon rectangle
//		int checkRectangleResult = CheckRectangle(contoursPolygons[i], &markerRectangle);
//
//		bool isGoodRectangle = (checkRectangleResult == GOOD_RECTANGLE);
//		bool isRectangleBig = (minRect[i].size.area() > maxArea);
//		if (isGoodRectangle && isRectangleBig) {
//
//			markerRectangles.push_back(markerRectangle);
//
//			//! If position of the biggest minimal area rectangle then save it
//			if (minRect[i].size.area() > maxRotatedRectArea) {
//				maxRotatedRectAreaPos = static_cast<int>(markerRectangles.size()) - 1;
//			}
//		}
//	}
//
//	if (maxRotatedRectAreaPos >= 0) {
//		resultContour.clear();
//
//		//! Copy rectangle to output array
//		for (size_t i = 0; i < 4; ++i) {
//			resultContour.push_back(markerRectangles[maxRotatedRectAreaPos].outerCorners[i]);
//		}
//
//		return true;
//	}
//
//	return false;
//}

bool findDocumentContour(cv::Mat& source, std::vector<cv::Point2f>& resultContour)
{
    if (source.empty())
    {
        throw std::invalid_argument("Input image for contours detection is empty");
    }

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(
            source, contours, hierarchy,
            CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE,
            cv::Point(0, 0));

    double maxArea = static_cast<double>(source.cols * source.rows) * 0.2;

    //! Approximate contours to polygons + get bounding rectangles and circles
    std::vector<std::vector<cv::Point> > contoursPolygons(contours.size());
    std::vector<cv::RotatedRect> minRect(contoursPolygons.size());

    float maxRotatedRectArea = 0;
    int maxRotatedRectAreaPos = -1;

    const double lengthCriteria = static_cast<double>(source.cols + source.rows) * 1.8;

    double minArea = static_cast<double>(source.cols * source.rows) * 0.1;

    std::vector<cv::Point> maxCurve;

    for (size_t i = 0; i < contours.size(); ++i)
    {
        //! Approximate contour by polygon
        cv::approxPolyDP(cv::Mat(contours[i]), contoursPolygons[i], contours[i].size() * 0.2, true);

        bool isContoursPolygonTooBig =
                arcLength(contoursPolygons[i], true) > lengthCriteria;
        bool isContoursPolygonTooSmall = arcLength(contoursPolygons[i], true) < 100;
        bool hasContoursPolygon4Conrners = contoursPolygons[i].size() == 4;
        if (isContoursPolygonTooBig || isContoursPolygonTooSmall ||
            !hasContoursPolygon4Conrners)
        {
            continue;
        }

        double contourArea = cv::contourArea(contoursPolygons[i]);

        if (contourArea > minArea)
        {
            minArea = contourArea;
            maxCurve = contoursPolygons[i];
        }

    }

    if (!maxCurve.empty() && IsQuadrangularConvex(maxCurve))
    {
        for (size_t i = 0; i < 4; ++i)
        {
            resultContour.push_back(
                    cv::Point2f(
                            static_cast<float>(maxCurve[i].x),
                            static_cast<float>(maxCurve[i].y)
                    )
            );
        }

        return true;
    }

    return false;
}

void binarizeByLocalVariances(cv::Mat& image, cv::Mat& result)
{
    if (image.empty())
    {
        throw std::invalid_argument("binarizeByLocalVariances: Input image for binarization is empty");
    }

    //! Get map of local variance
    cv::Mat varianceMap;
    Mat2LocalVarianceMap(image, varianceMap);

    // for intermediate results
    cv::Mat result1;
    cv::Mat result2;

    // minimal and maximal local variance values for each channel
    // these values used for threshold estimation
    cv::Vec3f globalMaxVariance;
    cv::Vec3f globalMinVariance;

    cv::Mat varianceMapWithoutAdditionalProcessing = varianceMap.clone();

    //! Split to channels
    std::vector<cv::Mat> channels(3);
    cv::split(varianceMap, channels);

    //! Find minimal and maximal local variance values
    double localMinVariance, localMaxVariance;
    minMaxLoc(channels[0], &localMinVariance, &localMaxVariance);
    globalMinVariance[0] = static_cast<float>(localMinVariance);
    globalMaxVariance[0] = static_cast<float>(localMaxVariance);
    minMaxLoc(channels[1], &localMinVariance, &localMaxVariance);
    globalMinVariance[1] = static_cast<float>(localMinVariance);
    globalMaxVariance[1] = static_cast<float>(localMaxVariance);
    minMaxLoc(channels[2], &localMinVariance, &localMaxVariance);
    globalMinVariance[2] = static_cast<float>(localMinVariance);
    globalMaxVariance[2] = static_cast<float>(localMaxVariance);

    //! Get the first intermediate result
    const float variancethresholdValue = 10.0f;
    result1 = 255.0 * (
            (channels[0] > variancethresholdValue) |
            (channels[1] > variancethresholdValue) |
            (channels[2] > variancethresholdValue));

    result1.convertTo(result1, CV_8UC1, 1, 0);


    //! Create new filter kernel (contrast adjustment mask)
    const float mask[] = {0, -1, 0,
                          -1, 16, -1,
                          0, -1, 0};
    //! Set filter kernel parameters
    const int kernelSize = 3;
    //const float areaPixelsCount = static_cast<float>(kernelSize * kernelSize); // for 3x3 area

    cv::Mat kernel = cv::Mat::zeros(kernelSize, kernelSize, CV_32F);
    kernel.at<float>(0, 0) = mask[0];
    kernel.at<float>(0, 1) = mask[1];
    kernel.at<float>(0, 2) = mask[2];
    kernel.at<float>(1, 0) = mask[3];
    kernel.at<float>(1, 1) = mask[4];
    kernel.at<float>(1, 2) = mask[5];
    kernel.at<float>(2, 0) = mask[6];
    kernel.at<float>(2, 1) = mask[7];
    kernel.at<float>(2, 2) = mask[8];

    //! Filtration for contrast adjustment
    cv::filter2D(varianceMap, varianceMap, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);

    //! Calculate threshold for filtered local variance values
    cv::Vec3f globalMinMaxDist = globalMaxVariance - globalMinVariance;
    cv::Vec3f globalMinMaxHalfDist = globalMinMaxDist / 2;
    cv::Vec3f varianceThreshold = globalMinMaxHalfDist * 0.125;

    //! Separate map of variance values to channels
    split(varianceMap, channels);

    //! Obtain the second intermediate result as combination of thresholded channels
    result2 = 255.0 * (
            (channels[0] > varianceThreshold[0]) |
            (channels[1] > varianceThreshold[1]) |
            (channels[2] > varianceThreshold[2]));

    result2.convertTo(result2, CV_8UC1, 1, 0);


    //! Calculate logarithm of variance map
    cv::log(varianceMapWithoutAdditionalProcessing, varianceMapWithoutAdditionalProcessing);

    cv::split(varianceMapWithoutAdditionalProcessing, channels);
    varianceMapWithoutAdditionalProcessing = channels[0] + channels[1] + channels[2];

    //! Execute gamma correction for variance map in logarithmic scale
    cv::Mat varianceMapWithGammaCorrection = varianceMapWithoutAdditionalProcessing.clone();
    double localMinLogVariance, localMaxLogVariance;
    //! Get min and max
    cv::minMaxLoc(varianceMapWithGammaCorrection, &localMinLogVariance, &localMaxLogVariance);
    //! Change range to [0;1]
    varianceMapWithGammaCorrection.convertTo(
            varianceMapWithGammaCorrection, CV_32FC1,
            1.0 / (localMaxLogVariance - localMinLogVariance),
            -localMinLogVariance / (localMaxLogVariance - localMinLogVariance));

    double gamma = 2;
    //! Gamma correction
    cv::pow(varianceMapWithGammaCorrection, gamma, varianceMapWithGammaCorrection);
    //! Convert to normal scale
    cv::convertScaleAbs(varianceMapWithGammaCorrection, varianceMapWithGammaCorrection, 255.0, 0);
    //! Binarize
    cv::adaptiveThreshold(varianceMapWithGammaCorrection, varianceMapWithGammaCorrection,
                          127.0, CV_ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 15, 0);

    //! Try to extract noise for variance map in logarithmic scale
    cv::Mat noiseOfLogVarianceMap = varianceMapWithoutAdditionalProcessing.clone();
    noiseOfLogVarianceMap -= cv::mean(noiseOfLogVarianceMap);
    //cv::exp(-noiseOfLogVarianceMap.mul(noiseOfLogVarianceMap) / (2.0), noiseOfLogVarianceMap);
    cv::exp(-noiseOfLogVarianceMap.mul(noiseOfLogVarianceMap) * 0.5, noiseOfLogVarianceMap); // -V678
    //! Convert to normal scale
    cv::convertScaleAbs(noiseOfLogVarianceMap, noiseOfLogVarianceMap, 127.0, 0);

    // Get the third intermediate result
    //! Get particularly denoised result
    result = 255 * ((varianceMapWithGammaCorrection - noiseOfLogVarianceMap) > 25);

    //! Get final result
    result = result & result1 & result2;
}

// not used
void binarizeByLocalVariancesWithoutFilters(cv::Mat& image, cv::Mat& result)
{
    if (image.empty())
    {
        throw std::invalid_argument("Input image for binarization is empty");
    }

    cv::Mat tmp;
    cv::Mat result1;
    cv::Mat result2;
    result1 = cv::Mat::zeros(image.size(), CV_8UC1);
    result2 = cv::Mat::zeros(image.size(), CV_8UC1);

    cv::Mat varianceMap = cv::Mat::zeros(image.size(), CV_32FC3);

    cv::copyMakeBorder(image, tmp, 1, 1, 1, 1, cv::BORDER_REPLICATE);

    const float areaPixelsCount = 9; // for 3x3 area
    const float varianceMinValue = 0.01f;

    cv::Vec3f meanVariance;

    cv::Vec3f globalMaxVariance;
    cv::Vec3f globalMinVariance;

    globalMaxVariance[0] = globalMaxVariance[1] = globalMaxVariance[2] = -1;
    globalMinVariance[0] = globalMinVariance[1] = globalMinVariance[2] = 1000000;

    for (int i = 1; i < tmp.cols - 1; ++i)
    {
        for (int j = 1; j < tmp.rows - 1; ++j)
        {
            cv::Vec3f neighbor0 = tmp.at<cv::Vec3b>(cv::Point(i - 1, j - 1));
            cv::Vec3f neighbor0Sqr = neighbor0.mul(neighbor0); // -V678

            cv::Vec3f neighbor1 = tmp.at<cv::Vec3b>(cv::Point(i + 0, j - 1));
            cv::Vec3f neighbor1Sqr = neighbor1.mul(neighbor1); // -V678

            cv::Vec3f neighbor2 = tmp.at<cv::Vec3b>(cv::Point(i + 1, j - 1));
            cv::Vec3f neighbor2Sqr = neighbor2.mul(neighbor2); // -V678


            cv::Vec3f neighbor3 = tmp.at<cv::Vec3b>(cv::Point(i - 1, j + 0));
            cv::Vec3f neighbor3Sqr = neighbor3.mul(neighbor3); // -V678

            cv::Vec3f center = tmp.at<cv::Vec3b>(cv::Point(i + 0, j + 0));
            cv::Vec3f centerSqr = center.mul(center); // -V678

            cv::Vec3f neighbor4 = tmp.at<cv::Vec3b>(cv::Point(i + 1, j + 0));
            cv::Vec3f neighbor4Sqr = neighbor4.mul(neighbor4); // -V678


            cv::Vec3f neighbor5 = tmp.at<cv::Vec3b>(cv::Point(i - 1, j + 1));
            cv::Vec3f neighbor5Sqr = neighbor5.mul(neighbor5); // -V678

            cv::Vec3f neighbor6 = tmp.at<cv::Vec3b>(cv::Point(i + 0, j + 1));
            cv::Vec3f neighbor6Sqr = neighbor6.mul(neighbor6); // -V678

            cv::Vec3f neighbor7 = tmp.at<cv::Vec3b>(cv::Point(i + 1, j + 1));
            cv::Vec3f neighbor7Sqr = neighbor7.mul(neighbor7); // -V678

            // sum(x[i])
            cv::Vec3f elementsSum =
                    neighbor0 + neighbor1 + neighbor2 +
                    neighbor3 + center + neighbor4 +
                    neighbor5 + neighbor6 + neighbor7;
            // (sum(x[i]))^2
            cv::Vec3f elementsSumSqr = elementsSum.mul(elementsSum); // -V678
            // sum( x[i]^2 )
            cv::Vec3f elementsSqrSum =
                    neighbor0Sqr + neighbor1Sqr + neighbor2Sqr +
                    neighbor3Sqr + centerSqr + neighbor4Sqr +
                    neighbor5Sqr + neighbor6Sqr + neighbor7Sqr;

            cv::Vec3f variance = (areaPixelsCount * elementsSqrSum - elementsSumSqr) /
                                 (areaPixelsCount * areaPixelsCount);
            //        Vec3f variance = ( elementsSqrSum - elementsSumSqr / areaPixelsCount )
            /// areaPixelsCount;

            for (int k = 0; k < 3; ++k)
            {
                if (variance[k] < varianceMinValue)
                {
                    variance[k] = varianceMinValue;
                }

                if (variance[k] > globalMaxVariance[k])
                {
                    globalMaxVariance[k] = variance[k];
                }

                if (variance[k] < globalMinVariance[k])
                {
                    globalMinVariance[k] = variance[k];
                }
            }

            float maxVariance = variance[0] > variance[1] ? variance[0] : variance[1];
            maxVariance = maxVariance > variance[2] ? maxVariance : variance[2];
            result1.at<uchar>(cv::Point(i - 1, j - 1)) = 255 * (maxVariance > 10);

            varianceMap.at<cv::Vec3f>(cv::Point(i - 1, j - 1)) = variance;
        }
    }

    cv::Vec3f globalMinMaxDist = globalMaxVariance - globalMinVariance;
    cv::Vec3f globalMinMaxHalfDist = globalMinMaxDist / 2;
    cv::Vec3f conditionArray = globalMinMaxHalfDist * 0.125;

    const float mask[] = {0, -1, 0, -1, 16, -1, 0, -1, 0};

    cv::copyMakeBorder(varianceMap, varianceMap, 1, 1, 1, 1, cv::BORDER_REFLECT);
    for (int i = 1; i < varianceMap.cols - 1; ++i)
    {
        for (int j = 1; j < varianceMap.rows - 1; ++j)
        {
            cv::Vec3f neighbor0 = varianceMap.at<cv::Vec3f>(cv::Point(i - 1, j - 1)) * mask[0];
            cv::Vec3f neighbor1 = varianceMap.at<cv::Vec3f>(cv::Point(i + 0, j - 1)) * mask[1];
            cv::Vec3f neighbor2 = varianceMap.at<cv::Vec3f>(cv::Point(i + 1, j - 1)) * mask[2];

            cv::Vec3f neighbor3 = varianceMap.at<cv::Vec3f>(cv::Point(i - 1, j + 0)) * mask[3];
            cv::Vec3f center = varianceMap.at<cv::Vec3f>(cv::Point(i + 0, j + 0)) * mask[4];
            cv::Vec3f neighbor4 = varianceMap.at<cv::Vec3f>(cv::Point(i + 1, j + 0)) * mask[5];

            cv::Vec3f neighbor5 = varianceMap.at<cv::Vec3f>(cv::Point(i - 1, j + 1)) * mask[6];
            cv::Vec3f neighbor6 = varianceMap.at<cv::Vec3f>(cv::Point(i + 0, j + 1)) * mask[7];
            cv::Vec3f neighbor7 = varianceMap.at<cv::Vec3f>(cv::Point(i + 1, j + 1)) * mask[8];

            cv::Vec3f elementsSum =
                    neighbor0 + neighbor1 + neighbor2 +
                    neighbor3 + center + neighbor4 +
                    neighbor5 + neighbor6 + neighbor7;

            for (size_t k = 0; k < 3; ++k)
            {
                elementsSum[k] = (elementsSum[k] > conditionArray[k] ? 1.0f : 0.0f);
            }

            result2.at<uchar>(cv::Point(i - 1, j - 1)) =
                    (uchar) (255 * (
                            std::max(elementsSum[0], std::max(elementsSum[1], elementsSum[2])))
                    );
        }
    }

    result = result1 & result2;
}

bool isQuadrangle(const std::vector<cv::Point>& contour)
{
    return contour.size() == 4;
}
