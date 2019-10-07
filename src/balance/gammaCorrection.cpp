/*
    MIT License

    Copyright (c) 2017 Alexander Zaitsev

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "gammaCorrection.h"

#include <cmath>
#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "utils.h"

namespace prl
{
template<typename T, size_t count>
void applyGammaCorrection(cv::Mat& outputImageMat, const std::vector<unsigned char>& lut)
{
	typedef cv::Vec<T, count> cvVec;

	for (auto it = outputImageMat.begin<cvVec>(), end = outputImageMat.end<cvVec>(); it != end; ++it)
	{
		if (count >= 4) (*it)[3] = lut[((*it)[3])];
		if (count >= 3) (*it)[2] = lut[((*it)[2])];
		if (count >= 2) (*it)[1] = lut[((*it)[1])];
		if (count >= 1) (*it)[0] = lut[((*it)[0])];
	}
}


void gammaCorrection(const cv::Mat& inputImage, cv::Mat& outputImage,
					 const double k, const double gamma)
{
	cv::Mat inputImageMat = inputImage.clone();

	if (inputImageMat.empty())
	{
		throw std::invalid_argument(
				"Invalid parameter for GammaCorrectionFilter_OpenCV");
	}

	cv::Mat outputImageMat;

	std::vector<unsigned char> lut(256);
	for (size_t i = 0; i < lut.size(); ++i)
	{
		lut[i] = cv::saturate_cast<uchar>(std::pow((double) (i / 255.0), gamma) * 255.0);
	}
	outputImageMat = inputImageMat.clone();
	const int channels = outputImageMat.channels();

	//Converting to 3 channels
	if (channels == 4)
	{
		cv::cvtColor(inputImageMat, outputImageMat, cv::COLOR_BGRA2BGR);
	}

	//Applying gamma correction
	switch (channels)
	{
		case 1:
		{
			applyGammaCorrection<uchar, 1>(outputImageMat, lut);
			break;
		}
		case 2:
		{
			applyGammaCorrection<uchar, 2>(outputImageMat, lut);
			break;
		}
		case 3:
		{
			applyGammaCorrection<uchar, 3>(outputImageMat, lut);
			break;
		}
	}

	//Step two: we multiply on some coefficient, if it doesn't equal to 1.0
	if (!eq_d(k, 1.0))
	{
		outputImageMat *= k;
	}

	outputImage = outputImageMat.clone();
}
}
