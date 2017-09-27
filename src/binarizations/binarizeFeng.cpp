#include "binarizeFeng.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <stdexcept>

void prl::binarizeFeng(
        cv::Mat& imageCopy, cv::Mat& imageFeng,
        int windowSize,
        double thresholdCoefficient_alpha1,
        double thresholdCoefficient_k1,
        double thresholdCoefficient_k2,
        double thresholdCoefficient_gamma,
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

    cv::Mat localMeanValues;
    cv::Mat localDevianceValues;

    cv::Rect processingRect(w / 2, w / 2, imageCopy.cols - w, imageCopy.rows - w);

    {
        //! add borders
        cv::copyMakeBorder(imageCopy, imageCopy, w / 2, w / 2, w / 2, w / 2, cv::BORDER_REPLICATE);

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

    //! calculate Feng thresholds
    double imageMin;
    cv::minMaxLoc(imageCopy, &imageMin);
    double alpha1 = thresholdCoefficient_alpha1; // (0.1 + 0.2) / 2.0;
    double k1 = thresholdCoefficient_k1; // (0.15 + 0.25) / 2.0;
    double k2 = thresholdCoefficient_k2; // (0.01 + 0.05) / 2.0;
    double gamma = thresholdCoefficient_gamma; // 2.0;

    cv::Mat Rs = localDevianceValues;

    cv::Mat thresholdsValues;

    {
        cv::Mat tmpAlpha1;
        cv::divide(localDevianceValues, Rs, tmpAlpha1);
        cv::Mat tmpAlpha2;
        cv::pow(tmpAlpha1, gamma, tmpAlpha2);

        cv::Mat alpha2 = k1 * tmpAlpha2;
        cv::Mat alpha3 = k2 * tmpAlpha2;

        //Mat thresholdsValues = (1 - alpha1) * localMeanValues + alpha2.mul(tmpAlpha1).mul(localMeanValues - imageMin) + alpha3 * imageMin;

        double c1 = 1.0 - alpha1;
        cv::Mat c2 = tmpAlpha2.mul(tmpAlpha1);
        tmpAlpha2.release();

        addWeighted(alpha3, imageMin, c2, -imageMin, 0.0, tmpAlpha1);
        cv::Mat c3 = tmpAlpha1;
        //Mat thresholdsValues = localMeanValues.mul(c2 + c1) + c3;
        thresholdsValues = c2 + c1;
        thresholdsValues = thresholdsValues.mul(localMeanValues);
        thresholdsValues += c3;
    }

    thresholdsValues.convertTo(thresholdsValues, CV_8UC1);

    //! get binarized image
    imageFeng = imageCopy(processingRect) > thresholdsValues;

    //! apply morphology operation if them required
    if (morphIterationCount > 0)
    {
        cv::dilate(imageFeng, imageFeng, cv::Mat(), cv::Point(-1, -1), morphIterationCount);
        cv::erode(imageFeng, imageFeng, cv::Mat(), cv::Point(-1, -1), morphIterationCount);
    }
    else
    {
        if (morphIterationCount < 0)
        {
            cv::erode(imageFeng, imageFeng, cv::Mat(), cv::Point(-1, -1), -morphIterationCount);
            cv::dilate(imageFeng, imageFeng, cv::Mat(), cv::Point(-1, -1), -morphIterationCount);
        }
    }
}
