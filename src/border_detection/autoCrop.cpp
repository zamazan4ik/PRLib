#include "autoCrop.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "resize.h"
#include "autoCropUtils.h"
#include "warp.h"

namespace prl
{

/*!
 * \brief Scale contour proportionally to image
 * \param[inout] contour Scaled contour.
 * \param[in] fromImageSize Current image size.
 * \param[in] toImageSize New image size.
 */
void ScaleContour(std::vector<cv::Point2f>& contour, cv::Size& fromImageSize, cv::Size& toImageSize)
{
    if (contour.empty())
    {
        throw std::invalid_argument("Contour for scaling is empty");
    }

    float xScale = static_cast<float>(toImageSize.width) /
                   static_cast<float>(fromImageSize.width);
    float yScale = static_cast<float>(toImageSize.height) /
                   static_cast<float>(fromImageSize.height);

    for (size_t i = 0; i < contour.size(); ++i)
    {
        contour[i].x *= xScale;
        contour[i].y *= yScale;
    }
}

/*!
 * \brief Contour points ordering.
 * \param[inout] pt Contour.
 * \return true if ordering is successful.
 */
bool cropVerticesOrdering(std::vector<cv::Point2f>& pt)
{
    if (pt.empty())
    {
        throw std::invalid_argument("Contour for ordering is empty");
    }

    //! This should be 4 for rectangles, but it allows to be extended to more complex cases
    int verticesNumber = static_cast<int>(pt.size());
    if (verticesNumber != 4)
    {
        throw std::invalid_argument("Points number in input contour isn't equal 4");
    }

    //! Find convex hull of points
    std::vector<cv::Point2f> points;
    points = pt;

    // indices of convex vertices
    std::vector<int> indices;
    cv::convexHull(points, indices, false);

    //! Find top left point, as starting point. This is the nearest to (0.0) point
    int minDistanceIdx = 0;
    double minDistanceSqr =
            pt[indices[0]].x * pt[indices[0]].x +
            pt[indices[0]].y * pt[indices[0]].y;

    for (int i = 1; i < verticesNumber; ++i)
    {
        double distanceSqr =
                pt[indices[i]].x * pt[indices[i]].x +
                pt[indices[i]].y * pt[indices[i]].y;

        if (distanceSqr < minDistanceSqr)
        {
            minDistanceIdx = i;
            minDistanceSqr = distanceSqr;
        }
    }

    //! Now starting point index in source array is in indices[minDistanceIdx]
    //! convex hull vertices are sorted clockwise in indices array, so transfer them to result
    //! beginning from starting point to the end of array...
    std::vector<cv::Point2f> ret(verticesNumber);
    int idx = 0;
    for (int i = minDistanceIdx; i < verticesNumber; ++i, ++idx)
    {
        ret[idx] = cv::Point2f(pt[indices[i]].x, pt[indices[i]].y);
    }

    //! ... and from the begin of array up to starting point
    for (int i = 0; i < minDistanceIdx; ++i, ++idx)
    {
        ret[idx] = cv::Point2f(pt[indices[i]].x, pt[indices[i]].y);
    }

    pt = ret;

    return true;
}

/*!
 * \brief Approximate contour on base of text lines detection.
 * \param[in] imgSource Input image.
 * \param[out] resultContour Resulting contour.
 * \return true if contour is detected.
 */
bool linesOfTextDetection(cv::Mat& imgSource, std::vector<cv::Point2f>& resultContour)
{
    if (imgSource.empty())
    {
        throw std::invalid_argument("Image for line detection is empty");
    }

    if (imgSource.channels() != 1)
    {
        cvtColor(imgSource, imgSource, CV_BGR2GRAY);
    }

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(imgSource, lines, 1, CV_PI / 180, 100, imgSource.size().width / 4.f, 10);

    //! Calculate angles of lines
    long nLines = static_cast<long>(lines.size());
    std::vector<double> angles(static_cast<int>(nLines));
    for (int i = 0; i < nLines; ++i)
    {
        double x1 = lines[i][0];
        double y1 = lines[i][1];
        double x2 = lines[i][2];
        double y2 = lines[i][3];

        angles.push_back(atan2(y2 - y1, x2 - x1));
    }

    //! Get filtered border values
    int left = imgSource.cols / 40;
    int right = imgSource.cols - left;
    int top = imgSource.rows / 40;
    int bottom = imgSource.rows - top;

    std::vector<double> matOfAngles = angles;

    //! Calculate mean and standard deviation of angles
    std::vector<double> mean;
    std::vector<double> stddev;
    cv::meanStdDev(matOfAngles, mean, stddev);

    double mn = mean[0];
    double sd = stddev[0];

    //! Sort out angles too deviating from mean value
    //! the idea is that lines of text are majority
    std::vector<cv::Point2f> vertices;
    std::vector<cv::Point2f> points;
    for (int i = 0; i < nLines; ++i)
    {
        if (std::abs(angles[i] - mn) < sd)
        {
            double x1 = lines[i][0];
            double y1 = lines[i][1];
            double x2 = lines[i][2];
            double y2 = lines[i][3];

            // border filtration
            if (x1 > left && x1 < right && x2 > left && x2 < right &&
                y1 > top && y1 < bottom && y2 > top && y2 < bottom)
            {
                cv::Point2f start(static_cast<float>(x1), static_cast<float>(y1));
                cv::Point2f end(static_cast<float>(x2), static_cast<float>(y2));

                vertices.push_back(start);
                vertices.push_back(end);
            }
        }
    }

    //! We need at least 3 lines to build a rectangle,
    //! each with 2 ends and 2 coordinates to each end
    if (vertices.size() >= 3 * 2 * 2)
    {
        points = vertices;
        cv::RotatedRect box = minAreaRect(cv::Mat(points));

        //! Expand box to whitespaces from text
        box.size.width += imgSource.cols / 40.f;
        box.size.height += imgSource.rows / 40.f;

        cv::Point2f rectPoints[4];
        box.points(rectPoints);

        //! Check if we out of page
        cv::Point2f apt[4];
        int fixedPoints = 0;
        for (int j = 0; j < 4; ++j)
        {
            bool fixedPoint = false;

            if (rectPoints[j].x < 0)
            {
                apt[j].x = 0;
                fixedPoint = true;
            }
            else if (rectPoints[j].x >= imgSource.cols)
            {
                apt[j].x = static_cast<float>(imgSource.cols - 1);
                fixedPoint = true;
            }
            else
            {
                apt[j].x = rectPoints[j].x;
            }

            if (rectPoints[j].y < 0)
            {
                apt[j].y = 0;
                fixedPoint = true;
            }
            else if (rectPoints[j].y >= imgSource.rows)
            {
                apt[j].y = static_cast<float>(imgSource.rows - 1);
                fixedPoint = true;
            }
            else
            {
                apt[j].y = rectPoints[j].y;
            }

            if (fixedPoint)
            {
                ++fixedPoints;
            }
        }

        if (fixedPoints > 1)
        {
            return false;
        }

        resultContour.resize(4);

        for (size_t i = 0; i < 4; ++i)
        {
            resultContour[i] = apt[i];
        }

        return true;
    }
    else
    {
        // nothing found, skip this method
        return false;
    }
}

/*!
 * \brief Approximate contour on base of feature detection.
 * \param[in] imgSource Input image.
 * \param[out] resultContour Resulting contour.
 * \return true if contour is detected.
 */
bool featureDetection(cv::Mat& imgSource, std::vector<cv::Point2f>& resultContour)
{
    if (imgSource.empty())
    {
        throw std::invalid_argument("Image for feature detection is empty");
    }

    std::vector<cv::KeyPoint> keypoints;

#ifndef OPENCV3
    cv::Ptr<cv::FastFeatureDetector> detector = cv::FastFeatureDetector::create();
    detector->detect(imgSource, keypoints);
#else
    cv::Ptr<cv::FastFeatureDetector> detector = cv::FastFeatureDetector::create();
    detector->detect(imgSource, keypoints);
#endif

    if (keypoints.size() == 0)
    {
        // no features found, skip that method
        return false;
    }

    //! Remove keypoints too near to frame edges
    int xMargin = imgSource.cols / 40;
    int yMargin = imgSource.rows / 40;

    std::vector<cv::KeyPoint>& listOfKeypoints = keypoints;
    std::vector<cv::KeyPoint> listOfBestKeypoints;
    for (cv::KeyPoint& kp : listOfKeypoints)
    {
        if (kp.pt.x > xMargin &&
            kp.pt.x < (imgSource.cols - xMargin) &&
            kp.pt.y > yMargin &&
            kp.pt.y < (imgSource.rows - yMargin))
        {

            listOfBestKeypoints.push_back(kp);
        }
    }

    //! Sort and select best keypoints
    int numKeypoints = static_cast<int>(listOfBestKeypoints.size());

    if (numKeypoints > 5000)
    {
        std::sort(
                listOfBestKeypoints.begin(),
                listOfBestKeypoints.end(),
                [](cv::KeyPoint& kp1, cv::KeyPoint& kp2) -> int
                {
                    return static_cast<int>(kp2.response - kp1.response);
                }
        );

        std::vector<cv::KeyPoint> listOfBestKeypoints_tmp(
                listOfBestKeypoints.begin(),
                listOfBestKeypoints.begin() + numKeypoints / 2);

        listOfBestKeypoints = listOfBestKeypoints_tmp;
    }


    std::vector<cv::Point2f> listOfBestPoints(listOfBestKeypoints.size());
    for (cv::KeyPoint& kp : listOfBestKeypoints)
    {
        listOfBestPoints.push_back(kp.pt);
    }

    std::vector<cv::Point2f> points;
    points = listOfBestPoints;

    cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));

    //! Expand box to have whitespace around text
    box.size.width += static_cast<float>(imgSource.cols) / 40.0f;
    box.size.height += static_cast<float>(imgSource.rows) / 40.0f;

    cv::Point2f rectPoints[4];
    box.points(rectPoints);

    //! Check if we are out of page
    cv::Point2f apt[4];
    for (int j = 0; j < 4; ++j)
    {
        if (rectPoints[j].x < 0)
        {
            apt[j].x = 0;
        }
        else if (rectPoints[j].x > imgSource.cols)
        {
            apt[j].x = static_cast<float>(imgSource.cols);
        }
        else
        {
            apt[j].x = rectPoints[j].x;
        }

        if (rectPoints[j].y < 0)
        {
            apt[j].y = 0;
        }
        else if (rectPoints[j].y > imgSource.rows)
        {
            apt[j].y = static_cast<float>(imgSource.rows);
        }
        else
        {
            apt[j].y = rectPoints[j].y;
        }
    }

    resultContour.resize(4);

    for (size_t i = 0; i < 4; ++i)
    {
        resultContour[i] = apt[i];
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool GetDocumentContour(
        cv::Mat& sourceImage,
        const double scaleX, const double scaleY,
        const int nProcessedImageSize,
        std::vector<cv::Point2f>& resultContour,
        bool useSimpleOnly/* = false*/,
        int* successMethodNumber/* = NULL*/)
{
    if (sourceImage.empty())
    {
        throw std::invalid_argument("Image for document contour detection is empty");
    }

    cv::Mat imageToProc;
    bool isContourDetected;

    //! Store source image size
    cv::Size sourceImageSize(sourceImage.size());

    cv::Size newImageSize;

    if (scaleX > 0 && scaleY > 0)
    {
        newImageSize = cv::Size(
                static_cast<int>(sourceImage.cols * scaleX),
                static_cast<int>(sourceImage.rows * scaleY)
        );

        cv::resize(sourceImage, imageToProc, newImageSize, 0, 0, cv::INTER_AREA);
    }
    else
    {
        int longSide = std::max(sourceImage.cols, sourceImage.rows);

        int scaleFactorX = 1;
        int scaleFactorY = 1;

        imageToProc = sourceImage.clone();

        while (longSide > nProcessedImageSize)
        {
            cv::pyrDown(imageToProc, imageToProc);

            longSide = std::max(imageToProc.cols, imageToProc.rows);
            scaleFactorX *= 2;
            scaleFactorY *= 2;
        }

        newImageSize = cv::Size(sourceImage.cols / scaleFactorX, sourceImage.rows / scaleFactorY);
    }

    ////! Get resizing parameters
    ////float xScale = sourceImage.cols / 256.0f;
    ////float yScale = sourceImage.rows / 256.0f;

    ////float image_scale = max(xScale, yScale);

    //cv::Size newImageSize(nProcessedImageSize, nProcessedImageSize);
    //float inSampleSize = 0;

    ////! Resize image
    //cv::resize(sourceImage, imageToProc, newImageSize, inSampleSize, inSampleSize, cv::INTER_AREA);

    //////////////////////////////////////////////////////////////////////////

    cv::Mat resultCanny;

    {
        cv::GaussianBlur(imageToProc, resultCanny, cv::Size(9, 9), 0);

        double upper = 50;
        double lower = 25;
        cv::Mat resultCannyTmp;
        cv::Canny(resultCanny, resultCannyTmp, lower, upper);
        cv::dilate(
                resultCannyTmp, resultCanny,
                cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)),
                cv::Point(-1, -1), 2);
    }

    isContourDetected = findDocumentContour(resultCanny, resultContour);

    if (isContourDetected)
    {
        // We have detected borders
        //findContoursDrawResult(image_to_proc.clone(), result_variances.clone(), "temp", true);

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////

    //! Try detect borders using local variances

    cv::Mat resultVariances, resultVariancesBackup;

    //! Make binarization based on local variances
    {
        cv::Mat tmp1(imageToProc.clone());
        binarizeByLocalVariances(tmp1, resultVariances);
    }

    resultVariancesBackup = resultVariances.clone();

    //! Try to find borders
    isContourDetected = findDocumentContour(resultVariances, resultContour);

    if (isContourDetected)
    {
        // We have detected borders
        //findContoursDrawResult(image_to_proc.clone(), result_variances.clone(), "temp", true);

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //! Try combinations (logical "AND" and logical "OR") results of Canny detector and
    //! local variances values binarization results

    //LOG_MESSAGE("Local variances and Canny combination (\"AND\") starts");

    resultVariances = resultVariancesBackup.clone() & resultCanny;

    isContourDetected = findDocumentContour(resultVariances, resultContour);

    if (isContourDetected)
    {
        // we have detected borders
        //findContoursDrawResult(image_to_proc.clone(), result_variances.clone(), "temp", true);

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //LOG_MESSAGE("Local variances and Canny combination (\"AND\") fails");

    //////////////////////////////////////////////////////////////////////////
    //! Try Canny border detector

    //LOG_MESSAGE("Canny (channels) starts");

    //! Split channels
    std::vector<cv::Mat> colorPlanes(imageToProc.channels());

    cv::Mat space = imageToProc.clone();
    cv::split(space, colorPlanes);

    //! Get edges by Canny detector for each channel
    // and combine them by logical OR
    resultCanny = cv::Mat::zeros(space.size(), CV_8UC1);
    for (size_t i = 0; i < colorPlanes.size(); ++i)
    {
        cv::Mat channel;
        cv::GaussianBlur(colorPlanes[i], channel, cv::Size(9, 9), 0);
        cv::Mat tmp;
        double otsuThresholdValue = cv::threshold(channel, tmp, 0, 255,
                                                  CV_THRESH_BINARY | CV_THRESH_OTSU);

        double upper = otsuThresholdValue;
        double lower = 0.5 * upper;
        cv::Canny(channel, channel, lower, upper);
        cv::dilate(channel, channel, cv::Mat(), cv::Point(-1, -1), 4);
        resultCanny = resultCanny | channel;
    }

    //! Try to find borders
    isContourDetected = findDocumentContour(resultCanny, resultContour);

    if (isContourDetected)
    {
        // Canny detector detects border
        //findContoursDrawResult(image_to_proc.clone(), result_variances.clone(), "temp", true);

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //LOG_MESSAGE("Canny (channels) fails");

    //////////////////////////////////////////////////////////////////////////

    if (useSimpleOnly)
    {

        if (!resultContour.empty())
        {
            ScaleContour(resultContour, newImageSize, sourceImageSize);
            cropVerticesOrdering(resultContour);
        }

        return !resultContour.empty();
    }

    //////////////////////////////////////////////////////////////////////////
    //! Try dilate of results of local variances values binarization

    //LOG_MESSAGE("Local variances dilate starts");

    resultVariances = resultVariancesBackup.clone();

    cv::dilate(resultVariances, resultVariances, cv::Mat(), cv::Point(-1, -1), 4);

    isContourDetected = findDocumentContour(resultVariances, resultContour);

    if (isContourDetected)
    {
        // we have detected borders
        //findContoursDrawResult(image_to_proc.clone(), result_variances.clone(), "temp", true);

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //LOG_MESSAGE("Local variances dilate fails");

    //////////////////////////////////////////////////////////////////////////

    //LOG_MESSAGE("Local variances and Canny combination (\"OR\") starts");

    resultVariances = resultVariancesBackup.clone() | resultCanny;

    isContourDetected = findDocumentContour(resultVariances, resultContour);

    if (isContourDetected)
    {
        // we have detected borders

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //LOG_MESSAGE("Local variances and Canny combination (\"OR\") fails");


    //////////////////////////////////////////////////////////////////////////
    //! Try text lines detector

    //LOG_MESSAGE("Text lines detector starts");

    {
        cv::Mat tmp1(imageToProc.clone());
        isContourDetected = linesOfTextDetection(tmp1, resultContour);
    }

    if (isContourDetected)
    {
        // we have detected borders

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //LOG_MESSAGE("Text lines detector fails");

    //////////////////////////////////////////////////////////////////////////
    //! Try features detector

    //LOG_MESSAGE("Feature detector starts");

    {
        cv::Mat tmp1(imageToProc.clone());
        isContourDetected = featureDetection(tmp1, resultContour);
    }

    if (isContourDetected)
    {
        // we have detected borders

        ScaleContour(resultContour, newImageSize, sourceImageSize);
        cropVerticesOrdering(resultContour);
        return true;
    }

    //LOG_MESSAGE("Feature detector fails");

    return false;
}

bool GetDocumentContour(
        cv::Mat& sourceImage,
        const double scaleX, const double scaleY,
        std::vector<cv::Point2f>& resultContour,
        bool useSimpleOnly /*= false*/,
        int* successMethodNumber /*= NULL*/)
{
    return GetDocumentContour(
            sourceImage,
            scaleX, scaleY,
            256,
            resultContour,
            useSimpleOnly,
            successMethodNumber
    );
}

bool GetDocumentContour(
        cv::Mat& sourceImage,
        const int nProcessedImageSize,
        std::vector<cv::Point2f>& resultContour,
        bool useSimpleOnly /*= false*/,
        int* successMethodNumber /*= NULL*/)
{
    return GetDocumentContour(
            sourceImage,
            -1, -1,
            nProcessedImageSize,
            resultContour,
            useSimpleOnly,
            successMethodNumber
    );
}


/*
void warpCropAI(cv::Mat& sourceImg, cv::Mat& destImg,
                int x0, int y0,
                int x1, int y1,
                int x2, int y2,
                int x3, int y3,
                int borderMode */
/*= cv::BORDER_CONSTANT*//*
,
                const cv::Scalar& borderValue */
/*= cv::Scalar()*//*
)
{
    if (sourceImg.empty())
    {
        throw std::invalid_argument("Image for warping is empty");
    }

    double side1 = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
    double side2 = sqrt((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));
    double side3 = sqrt((x3 - x0) * (x3 - x0) + (y3 - y0) * (y3 - y0));
    double side4 = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

    double aspect12 = side1 / side2;
    double aspect34 = side3 / side4;

    if (aspect12 < 1.0)
    {
        aspect12 = 1.0f / aspect12;
    }

    if (aspect34 < 1.0)
    {
        aspect34 = 1.0f / aspect34;
    }

    long bitmapWidth = cvRound(std::max(side1, side2) * aspect34);
    long bitmapHeight = cvRound(std::max(side3, side4) * aspect12);

    // bitmapWidth/bitmapHeight can be easily more than input w/h.
    // It leads to using more memory than expected.
    // We have to limit the cropped image size by the source image's size.
    // Special case: we can get size more than max texture size (80 thousands).
    double ratio = 1.0;

    if (side1 > bitmapWidth || side2 > bitmapWidth
        || side3 > bitmapHeight || side4 > bitmapHeight)
    {
        if (bitmapWidth > bitmapHeight)
        {
            ratio = sourceImg.cols / bitmapWidth;
        }
        else
        {
            ratio = sourceImg.rows / bitmapHeight;
        }
    }

    bitmapWidth = cvRound(bitmapWidth * ratio);
    bitmapHeight = cvRound(bitmapHeight * ratio);

    float srcBuff[] = {static_cast<float>(x0), static_cast<float>(y0),
                       static_cast<float>(x1), static_cast<float>(y1),
                       static_cast<float>(x2), static_cast<float>(y2),
                       static_cast<float>(x3), static_cast<float>(y3)};

    float dstBuff[] = {static_cast<float>(0), static_cast<float>(0),
                       static_cast<float>(bitmapWidth), static_cast<float>(0),
                       static_cast<float>(bitmapWidth), static_cast<float>(bitmapHeight),
                       static_cast<float>(0), static_cast<float>(bitmapHeight)};

    cv::Mat src(4, 1, CV_32FC2, srcBuff);
    cv::Mat dst(4, 1, CV_32FC2, dstBuff);

    cv::Mat perspectiveTransform = cv::getPerspectiveTransform(src, dst);
//    Mat croppedImage = src_img.clone();
    cv::warpPerspective(
            sourceImg, destImg, perspectiveTransform, cv::Size(bitmapWidth, bitmapHeight),
            cv::INTER_LINEAR,
            borderMode, borderValue);
}
*/

bool ScannedDocumentImageAutoCrop(cv::Mat& sourceImage, cv::Mat& croppedImage)
{
    if (sourceImage.empty())
    {
        throw std::invalid_argument("ScannedDocumentImageAutoCrop: Image for cropping is empty");
    }

    switch (sourceImage.type())
    {
        case CV_8UC3:
            break;
        case CV_8UC1:
            cvtColor(sourceImage, sourceImage, CV_GRAY2BGR);
            break;
        default:
            if (sourceImage.empty())
            {
                throw std::invalid_argument(
                        "ScannedDocumentImageAutoCrop: Invalid type of image for cropping (required 8 or 24 bits per pixel)");
            }
            break;
    }

    std::vector<cv::Point2f> resultContour;

    //! Try to detect border
    if (!GetDocumentContour(sourceImage, -1, -1, resultContour))
    {
        return false;
    }

    //! Sort points
    /*if (!cropVerticesOrdering(resultContour)) {
        cout << _TXT("Document border contour points is not sorted.") << endl;
        return false;
    }*/

    std::vector<cv::Point> temp(resultContour.begin(), resultContour.end());

    //! Crop area
    warpCrop(sourceImage, croppedImage, temp);

    return true;
}

std::vector<cv::Point> getContourAbbyy(const cv::Mat& src, size_t longSide /*= 256*/)
{
    //TODO: https://habrahabr.ru/company/abbyy/blog/200448/
    //1) Resize with Gaussian smoothing to 256 at most
    cv::Mat imageToProc;
    prl::resize(src, imageToProc, 0, 0, longSide);

    //2) Choose the most informative channel

    //2.1) Split to channels
    std::vector<cv::Mat> channels(imageToProc.channels());
    cv::split(imageToProc, channels);

    //2.2) Calculate histograms
    std::vector<cv::MatND> hist(channels.size());
    int c = 0;
    for (size_t i = 0; i < channels.size(); ++i)
    {
        //cv::calcHist(channels[i], 1, cv::Mat(), hist[i], 2, {32, 32},std::vector<float>({0.0f, 256.0f}));
        cv::calcHist(channels[i], {0}, cv::Mat(), hist[i], std::vector<int>({32, 32}),
                     std::vector<float>({0.0f, 256.0f}));
    }


    //3) Do medianBlur
    for (size_t i = 0; i < 3; ++i)
    {
        cv::medianBlur(imageToProc, imageToProc, 3);
    }

    for (size_t i = 0; i < 3; ++i)
    {
        cv::medianBlur(imageToProc, imageToProc, 5);
    }

    //4) Run Canny Edge detector
    cv::Mat canny_output;
    cv::Canny(imageToProc, canny_output, 25, 50);

    //TODO: Remove findcontour later
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, CV_RETR_CCOMP,
                 CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    std::vector<std::vector<cv::Point>> contoursApproxed(contours.size());
    std::vector<cv::Point> result;
    double maxArea = 0.0;
    for (size_t i = 0; i < contours.size(); ++i)
    {
        cv::approxPolyDP(cv::Mat(contours[i]), contoursApproxed[i], contours[i].size() * 0.2, true);

        if (!isQuadrangle(contoursApproxed[i]))
        {
            continue;
        }

        double area = cv::contourArea(contoursApproxed[i]);
        if (area > maxArea)
        {
            maxArea = area;
            result = contoursApproxed[i];
        }
    }
    //result = orderPoints(result);
    return result;
}
}