#include "binarizeLocalOtsu.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "imageLibCommon.h"

#include <vector>
#include <stdexcept>

void prl::binarizeLocalOtsu(
        cv::Mat& inputImageMat, cv::Mat& outputImageMat,
        double maxValue,
        double CLAHEClipLimit,
        int GaussianBlurKernelSize,
        double CannyUpperThresholdCoeff,
        double CannyLowerThresholdCoeff,
        int CannyMorphIters)
{
    //! input image must be not empty
    if (inputImageMat.empty())
    {
        throw std::invalid_argument("Input image for binarization is empty");
    }

    if (!(maxValue >= 0 && maxValue <= 255))
    {
        throw std::invalid_argument("Max value must be in range [0; 255]");
    }

    cv::Mat imageToProc;
    cv::Mat imageSelectedColorSpace;

    //! we work with gray image
    if (inputImageMat.channels() != 1)
    {
        cv::cvtColor(inputImageMat, imageSelectedColorSpace, CV_RGB2GRAY);
    }
    else
    {
        imageSelectedColorSpace = inputImageMat.clone();
    }

    const int intensityChannelNo = 0;

    //! get intensity channel only
    {
        std::vector<cv::Mat> channels(imageSelectedColorSpace.channels());
        cv::split(imageSelectedColorSpace, channels);

        imageToProc = channels[intensityChannelNo].clone();
    }

    //! Enhance local contrast if it is required
    if (CLAHEClipLimit > 0)
    {
        EnhanceLocalContrastByCLAHE(imageToProc, imageToProc, CLAHEClipLimit, true);
    }

    cv::Mat resultCanny;

    //! detect edges by using Canny edge detector
    CannyEdgeDetection(imageToProc, resultCanny,
                       GaussianBlurKernelSize,
                       CannyUpperThresholdCoeff, CannyLowerThresholdCoeff,
                       CannyMorphIters);
    cv::dilate(resultCanny, resultCanny, cv::Mat(), cv::Point(-1, -1), 3);

    if (imageSelectedColorSpace.channels() != 3)
    {
        cv::cvtColor(imageSelectedColorSpace, imageSelectedColorSpace, CV_GRAY2BGR);
    }

    std::vector<std::vector<cv::Point>> contours;
    std::vector<std::vector<cv::Point>> contoursAll;

    //! find contours and remove not required
    {
        std::vector<cv::Vec4i> hierarchy;

        cv::findContours(resultCanny, contoursAll, hierarchy, CV_RETR_EXTERNAL,
                         CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

        contours = contoursAll;

        RemoveChildrenContours(contours, hierarchy);

        // remove unclosed contours
        //contours.erase(remove_if(contours.begin(), contours.end(), IsContourUnclosed),
        //	contours.end());

        // remove contours using next conditions
        // - The aspect ratio is constrained to lie between 0.1 and 10 to remove highly elongated
        //   components.
        // - Components larger than 0.6 times the image dimensions  are removed.
        // - Furthermore, small and spurious components with areas less than 8 pixels are not
        //   considered for further processing.
        //contours.erase(
        //	remove_if(
        //		contours.begin(), contours.end(),
        //		[&imageSelectedColorSpace](vector<Point>& testedContour) -> bool
        //		{
        //			return IsContourUncorrect(testedContour, imageSelectedColorSpace);
        //		}
        //	),
        //	contours.end()
        //);

    }

    //! binarization
    cv::Mat binarized(imageSelectedColorSpace.size(), CV_8UC1);
    {
        cv::Rect imageRectangle(0, 0, imageToProc.cols, imageToProc.rows);

        binarized.setTo(255);

        //! for each contour ...
        for (size_t contourNo = 0; contourNo < contours.size(); ++contourNo)
        {

            std::vector<cv::Point>& contour = contours[contourNo];

            //! ... get boundary rectangle ...
            cv::Rect contourBoundingRectangle = cv::boundingRect(contour);

            //! ... create copy and ...
            cv::Mat tmp = imageToProc(contourBoundingRectangle).clone();
            //! binarize it by Otsu
            cv::threshold(tmp, tmp, 128, maxValue, CV_THRESH_OTSU);

            //! Store binarization result to output image
            binarized(contourBoundingRectangle).setTo(0, tmp ^ 255);
        }
    }
    outputImageMat = binarized;
}
