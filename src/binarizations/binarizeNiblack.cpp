#include "binarizeNiblack.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <stdexcept>

void prl::binarizeNiblack(
        cv::Mat& imageCopy, cv::Mat& imageNiblack,
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

    //! add borders
    cv::copyMakeBorder(imageCopy, imageCopy, w / 2, w / 2, w / 2, w / 2, cv::BORDER_REPLICATE);
    cv::Rect processingRect(w / 2, w / 2, imageCopy.cols - w, imageCopy.rows - w);

    cv::Mat integralImage;
    cv::Mat integralImageSqr;

    //! get integral image, ...
    integral(imageCopy, integralImage, integralImageSqr, usedFloatType);
    //! ... crop it and ...
    integralImage = integralImage(cv::Rect(1, 1, integralImage.cols - 1, integralImage.rows - 1));
    //! get square
    integralImageSqr =
            integralImageSqr(cv::Rect(1, 1, integralImageSqr.cols - 1, integralImageSqr.rows - 1));

    //! create storage for local means
    cv::Mat localMeanValues(integralImage.size() - processingRect.size(), usedFloatType);

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

    cv::Mat localDevianceValues;

    //! get local deviations
    cv::filter2D(integralImageSqr(processingRect), localDevianceValues, usedFloatType,
                 localWeightedSumsFilterKernel);

    localDevianceValues -= localMeanValuesSqr;
    cv::sqrt(localDevianceValues, localDevianceValues);

    //! calculate Niblack thresholds
    cv::Mat thresholdsValues = localMeanValues + k * localDevianceValues;
    thresholdsValues.convertTo(thresholdsValues, CV_8UC1);

    //! get binarized image
    imageNiblack = imageCopy(processingRect) > thresholdsValues;

    //! apply morphology operation if them required
    if (morphIterationCount > 0)
    {
        cv::dilate(imageNiblack, imageNiblack, cv::Mat(), cv::Point(-1, -1), morphIterationCount);
        cv::erode(imageNiblack, imageNiblack, cv::Mat(), cv::Point(-1, -1), morphIterationCount);
    }
    else
    {
        if (morphIterationCount < 0)
        {
            cv::erode(imageNiblack, imageNiblack, cv::Mat(), cv::Point(-1, -1), -morphIterationCount);
            cv::dilate(imageNiblack, imageNiblack, cv::Mat(), cv::Point(-1, -1), -morphIterationCount);
        }
    }
}