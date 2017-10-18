#include "correctNUIL.h"

#include <stdexcept>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

void filterChannelNUIL(
        cv::Scalar& channelsMeanScalar, size_t channelNumber,
        int structuringElementSize,
        cv::Mat& channelCopy,
        cv::Mat& filteredChannel)
{
    //! Image auto invert.
    if (channelsMeanScalar[static_cast<int>(channelNumber)] < 128)
    {
        channelCopy ^= 255;
    }

    //! Morphology operation.
    int selSize = structuringElementSize;
    cv::Mat sel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(selSize, selSize));

    cv::morphologyEx(channelCopy, filteredChannel, cv::MORPH_BLACKHAT, sel);

    //! Invertion.
    filteredChannel ^= 255;
}

void prl::correctNUIL(const cv::Mat& inputImage, cv::Mat& outputImage, int structuringElementSize)
{
    if (inputImage.empty())
    {
        throw std::invalid_argument("Input image for filtration is empty");
    }

    cv::Scalar mean;
    mean = cv::mean(inputImage);

    if (inputImage.channels() > 1)
    {
        std::vector<cv::Mat> channels(inputImage.channels());

        cv::split(inputImage, channels);

        for (size_t channelNumber = 0; channelNumber < channels.size(); ++channelNumber)
        {
            filterChannelNUIL(
                    mean, channelNumber,
                    structuringElementSize,
                    channels[channelNumber], channels[channelNumber]);
        }

        cv::merge(channels, outputImage);

    }
    else
    {
        cv::Mat imageCopy = inputImage.clone();
        filterChannelNUIL(
                mean, 0,
                structuringElementSize,
                imageCopy, outputImage);
    }
}