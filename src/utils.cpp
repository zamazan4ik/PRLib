//
// Created by zamazan4ik on 23.09.17.
//

#include <cmath>
#include <stdexcept>

#include <opencv2/core/core.hpp>

bool prl::eq_d(const double v1, const double v2, const double delta)
{
    return std::abs(v1 - v2) <= delta;
}

double prl::compareImages(const cv::Mat& image1, const cv::Mat& image2)
{
    cv::Mat image1Mat = image1.clone();
    cv::Mat image2Mat = image2.clone();

    if (image1Mat.empty())
    {
        throw std::invalid_argument("Input image #1 is empty");
    }

    if (image2Mat.empty())
    {
        throw std::invalid_argument("Input image #2 is empty");
    }

    bool isImageSizesEqual = image1Mat.size == image2Mat.size;
    bool isImageTypesEqual = image1Mat.type() == image2Mat.type();
    bool isImageChannelsCountsEqual = image1Mat.channels() == image2Mat.channels();
    if (!isImageSizesEqual ||
        !isImageTypesEqual ||
        !isImageChannelsCountsEqual)
    {
        return 0.0;
    }

    cv::Mat comparisonResultImage = (image1Mat == image2Mat); // -V601
    //comparisonResultImage *= 1.0f / 255.0f;
    comparisonResultImage &= cv::Scalar_<uchar>(1, 1, 1, 1);

    cv::Scalar comparisonResultImageChannelsSum = cv::sum(comparisonResultImage);
    comparisonResultImageChannelsSum /= static_cast<double>(image1Mat.total());

    double percent = 0.0;

    if (comparisonResultImage.channels() == 1)
    {
        percent = comparisonResultImageChannelsSum[0];
    }
    else
    {
        percent = 2.0f;

        for (int ch = 0; ch < comparisonResultImage.channels(); ++ch)
        {
            percent = std::min(comparisonResultImageChannelsSum[ch], percent);
        }
    }

    return percent;
}