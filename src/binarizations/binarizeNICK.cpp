#include "binarizeNICK.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <stdexcept>
#include <algorithm>

void prl::binarizeNICK(
        cv::Mat& imageCopy, cv::Mat& imageNICK,
        int windowSize,
        double thresholdCoefficient,
        int morphIterationCount)
{
    if (imageCopy.empty())
    {
        throw std::invalid_argument("Input image for binarization is empty");
    }

    if (!((windowSize > 1) && ((windowSize % 2) == 1)))
    {
        throw std::invalid_argument("Window size must satisfy the following condition: \
			( (windowSize > 1) && ((windowSize % 2) == 1) ) ");
    }

    if (imageCopy.channels() != 1)
    {
        cv::cvtColor(imageCopy, imageCopy, CV_BGR2GRAY);
    }

    const int usedFloatType = CV_64FC1;

    //! parameters and constants of algorithm
    int w = std::min(windowSize, std::min(imageCopy.cols, imageCopy.rows));;
    int wSqr = w * w;
    double wSqrBack = 1.0 / static_cast<double>(wSqr);
    const double k = thresholdCoefficient;
    //const double R = 128;
    //const double RBack = 1.0 / R;


    cv::Mat localDevianceValues;
    cv::Mat localMeanValues;

    cv::Rect processingRect(w / 2, w / 2, imageCopy.cols - w, imageCopy.rows - w);

    {
        //! add borders
        copyMakeBorder(imageCopy, imageCopy, w / 2, w / 2, w / 2, w / 2, cv::BORDER_REPLICATE);


        cv::Mat integralImage;
        cv::Mat integralImageSqr;

        //! get integral image, ...
        cv::integral(imageCopy, integralImage, integralImageSqr, usedFloatType);
        //! ... crop it and ...
        integralImage = integralImage(cv::Rect(1, 1, integralImage.cols - 1, integralImage.rows - 1));
        //! get square
        integralImageSqr =
                integralImageSqr(cv::Rect(1, 1, integralImageSqr.cols - 1, integralImageSqr.rows - 1));

        //! create storage for local means
        localMeanValues = cv::Mat(integralImage.size() - processingRect.size(), usedFloatType);

        //! create filter for local means calculation
        cv::Mat localMeanFilterKernel = cv::Mat::zeros(w, w, usedFloatType);
        localMeanFilterKernel.at<double>(0, 0) = wSqrBack;
        localMeanFilterKernel.at<double>(w - 1, 0) = -wSqrBack;
        localMeanFilterKernel.at<double>(w - 1, w - 1) = wSqrBack;
        localMeanFilterKernel.at<double>(0, w - 1) = -wSqrBack;
        //! get local means
        cv::filter2D(integralImage(processingRect), localMeanValues, usedFloatType,
                     localMeanFilterKernel, cv::Point(-1, -1), 0.0, cv::BORDER_REFLECT);

        //! create storage for local deviations
        cv::Mat localMeanValuesSqr = localMeanValues.mul(localMeanValues); // -V678

        //! create filter for local deviations calculation
        cv::Mat localWeightedSumsFilterKernel = localMeanFilterKernel;

        //! get local deviations
        cv::filter2D(integralImageSqr(processingRect), localDevianceValues, usedFloatType,
                     localWeightedSumsFilterKernel);

        localDevianceValues -= localMeanValuesSqr;
        cv::sqrt(localDevianceValues, localDevianceValues);
    }

    //! calculate WolfJolion thresholds
    double imageMin;
    cv::minMaxLoc(imageCopy, &imageMin);

    double devianceMin, devianceMax;
    cv::minMaxLoc(localDevianceValues, &devianceMin, &devianceMax);

    cv::Mat C = localMeanValues.mul(localMeanValues); // -V678
    localDevianceValues = localDevianceValues.mul(localDevianceValues); // -V678
    C = C + localDevianceValues;
    cv::sqrt(C, C);
    cv::Mat thresholdsValues;
    cv::addWeighted(localMeanValues, 1, C, k, 0, thresholdsValues);

    thresholdsValues.convertTo(thresholdsValues, CV_8UC1);

    //! get binarized image
    imageNICK = imageCopy(processingRect) > thresholdsValues;

    //! apply morphology operation if them required
    if (morphIterationCount > 0)
    {
        cv::dilate(imageNICK, imageNICK, cv::Mat(), cv::Point(-1, -1), morphIterationCount);
        cv::erode(imageNICK, imageNICK, cv::Mat(), cv::Point(-1, -1), morphIterationCount);
    }
    else if (morphIterationCount < 0)
    {
        cv::erode(imageNICK, imageNICK, cv::Mat(), cv::Point(-1, -1), -morphIterationCount);
        cv::dilate(imageNICK, imageNICK, cv::Mat(), cv::Point(-1, -1), -morphIterationCount);
    }
}
