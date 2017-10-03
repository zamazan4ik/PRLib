#ifndef BackgroundNormalization_Lepton_h__
#define BackgroundNormalization_Lepton_h__

#include <opencv2/core/core.hpp>

namespace prl
{
void backgroundNormalization(const cv::Mat& inputImage, cv::Mat& outputImage);
}

#endif // BackgroundNormalization_Lepton_h__
