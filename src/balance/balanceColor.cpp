#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


void prl::colorBalance(const cv::Mat& inputImage, cv::Mat& outputImage)
{
    if (inputImage.IsEmpty())
    {
        throw std::invalid_argument("ColorBalance: Input image for filtration is empty");
    }

    if (inputImage.ChannelsCount() != 3 && inputImage.ChannelsCount() != 4)
    {
        throw std::invalid_argument("ColorBalance: image should be colored");
    }

    cv::Mat imageCopy = inputImage.clone();

    if (inputImage.ChannelsCount() == 4)
    {
        cv::cvtColor(imageCopy, imageCopy, CV_BGRA2BGR);
    }

    std::vector<cv::Mat> srcChans(imageCopy.channels());
    cv::split(imageCopy, srcChans);

    srcChans[0].convertTo(srcChans[0], CV_32FC1, 1.0 / 255.0);
    srcChans[2].convertTo(srcChans[2], CV_32FC1, 1.0 / 255.0);

    cv::pow(srcChans[0], 1.0 / this->m_ColorBalanceGamma, srcChans[0]);
    cv::pow(srcChans[2], 1.0 * this->m_ColorBalanceGamma, srcChans[2]);

    srcChans[0].convertTo(srcChans[0], CV_8UC1, 255.0);
    srcChans[2].convertTo(srcChans[2], CV_8UC1, 255.0);

    cv::Mat convertedTmp;

    cv::merge(srcChans, convertedTmp);

    if (this->m_SaturationGamma != 1.0)
    {
        cv::Mat hsvImg;

        cv::cvtColor(convertedTmp, hsvImg, CV_BGR2HSV);

        cv::split(hsvImg, srcChans);

        srcChans[1].convertTo(srcChans[1], CV_32FC1, 1.0 / 255.0);

        cv::pow(srcChans[1], 1.0 * this->m_SaturationGamma, srcChans[1]);

        srcChans[1].convertTo(srcChans[1], CV_8UC1, 255.0);

        cv::merge(srcChans, convertedTmp);

        cv::cvtColor(convertedTmp, convertedTmp, CV_HSV2BGR);
    }

    outputImage = convertedTmp.clone();
}