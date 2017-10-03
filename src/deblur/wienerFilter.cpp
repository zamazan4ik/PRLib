#include "WienerFilterImpl.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <complex>
#include <vector>
#include <memory>

#include <iostream>

#include "IPL_Exceptions/IPL_Exceptions.h"

using namespace std;


typedef double ComplexValueType;
typedef cv::Vec<ComplexValueType, 2> ComplexValue;

ComplexValueType& realRef(ComplexValue& value)
{
	return value[0];
}

ComplexValueType& imagRef(ComplexValue& value)
{
	return value[1];
}

ComplexValueType real(const ComplexValue& value)
{
	return value[0];
}

ComplexValueType imag(const ComplexValue& value)
{
	return value[1];
}

ComplexValue conj(const ComplexValue& value)
{
	ComplexValue result = value;

	imagRef(result) = -imag(result);

	return result;
}

ComplexValue mul(const ComplexValue& value, double coeff)
{
	ComplexValue result = value;

	realRef(result) *= coeff;
	imagRef(result) *= coeff;

	return result;
}

ComplexValue mul(const ComplexValue& value1, const ComplexValue& value2)
{
	ComplexValue result;

	realRef(result) = ( real(value1) * real(value2) - imag(value1) * imag(value2) );
	imagRef(result) = ( real(value1) * imag(value2) + imag(value1) * real(value2) );

	return result;
}

ComplexValue abs(const ComplexValue& value)
{
	ComplexValue result;

	result = mul(value, conj(value));

	return result;
}

ComplexValue div(const ComplexValue& value, double coeff)
{
	ComplexValue result = value;

	realRef(result) /= coeff;
	imagRef(result) /= coeff;

	return result;
}

ComplexValue div(const ComplexValue& value1, const ComplexValue& value2)
{
	ComplexValue result;

	result = mul( value1, conj(value2) ) / real( abs(value2) );

	return result;
}


//////////////////////////////////////////////////////////////////////////
class GaussFilter
{
public:
	GaussFilter(int filterKernelHeight, int filterKernelWidth, double sigma = 3);

	cv::Mat filterKernel;
};

GaussFilter::GaussFilter(
	int filterKernelHeight, int filterKernelWidth, double sigma/* = 3*/)
{
	vector<cv::Mat> tmpFilterKernel;
	tmpFilterKernel.push_back(cv::Mat(filterKernelWidth, filterKernelHeight, CV_64F));
	tmpFilterKernel.push_back(cv::Mat(filterKernelWidth, filterKernelHeight, CV_64F));

	double sigmaSqr = sigma * sigma;

	double amplitude = 1 / (sigma * sqrt(2 * CV_PI));

	int halfFilterKernelWidth = filterKernelWidth / 2;
	int halfFilterKernelHeight = filterKernelHeight / 2;

	for (int i = 0; i < filterKernelHeight; ++i) {

		int y = i - halfFilterKernelHeight;
		double ySqr = y * y;

		for (int j = 0; j < filterKernelWidth; ++j) {

			int x = j - halfFilterKernelWidth;
			double xSqr = x * x;

			double exponent = static_cast<double>( (ySqr + xSqr) / (2.0 * sigmaSqr) );

			tmpFilterKernel[0].at<ComplexValueType>(i, j) = amplitude * exp(-exponent);
			tmpFilterKernel[1].at<ComplexValueType>(i, j) = 0;
		}
	}

	cv::merge(tmpFilterKernel, this->filterKernel);
}

//////////////////////////////////////////////////////////////////////////
class WienerFilter
{
public:
    WienerFilter(
		int filterKernelHeight, int filterKernelWidth,
        double coeffWiener,
        double sigmaGauss);
	
	cv::Mat filterKernel;
};


WienerFilter::WienerFilter(
	int filterKernelHeight, int filterKernelWidth,
	double coeffWiener,
	double sigmaGauss)
{
	GaussFilter gaussFilter( filterKernelHeight, filterKernelWidth, sigmaGauss );
	
	this->filterKernel = cv::Mat(filterKernelWidth, filterKernelHeight, CV_64FC2);

	vector<cv::Mat> tmpFilterKernel;
	tmpFilterKernel.push_back(cv::Mat(filterKernelWidth, filterKernelHeight, CV_64F));
	tmpFilterKernel.push_back(cv::Mat(filterKernelWidth, filterKernelHeight, CV_64F));

	//cv::Mat& tmp1 = tmpFilterKernel[0];
	//cv::Mat& tmp2 = tmpFilterKernel[1];

	cv::Mat gaussFilterFourie;
	cv::dft(gaussFilter.filterKernel, gaussFilterFourie, cv::DFT_COMPLEX_OUTPUT);

	for (int i = 0; i < filterKernelHeight; ++i) {
		for (int j = 0; j < filterKernelWidth; ++j) {
			ComplexValue gaussFilterFourieAbs = gaussFilterFourie.at<ComplexValue>(i, j);
			ComplexValue tmpValue = div(gaussFilterFourieAbs, 
				mul(gaussFilterFourie.at<ComplexValue>(i, j), 
				( real(gaussFilterFourieAbs) + coeffWiener) ) );

			tmpFilterKernel[0].at<ComplexValueType>(i, j) = real(tmpValue);
			tmpFilterKernel[1].at<ComplexValueType>(i, j) = imag(tmpValue);
		}
	}

	cv::merge(tmpFilterKernel, this->filterKernel);
}
//////////////////////////////////////////////////////////////////////////


void WienerFilterImpl(
	cv::Mat& inputImage, 
	cv::Mat& outputImage, 
	int filterKernelWidth/* = 11*/, int filterKernelHeight/* = 11*/,
	double coeffWiener/* = 1*/,
	double sigmaGauss/* = 5*/) 
{
	if (inputImage.empty()) {
		throw std::invalid_argument("Input image for deblurring is empty"));
	}

	if ( !( (filterKernelWidth > 1) && (filterKernelHeight > 1) ) ) {
		throw std::invalid_argument("Parameters must satisfy the following condition: \
			( (filterKernelWidth > 1) && (filterKernelHeight > 1) ) "));
	}

	if ( !( sigmaGauss > 0 ) ) {
		throw std::invalid_argument("Parameters must satisfy the following condition: \
			( sigmaGauss > 0 ) "));
	}

	int inputImageHeight = inputImage.rows;
	int inputImageWidth = inputImage.cols;


	//////////////////////////////////////////////////////////////////////////

	cv::Mat kernelImage = cv::Mat(filterKernelHeight, filterKernelWidth, CV_64FC1);

	double sigmaSqr = sigmaGauss * sigmaGauss;

	double amplitude = 1 / (sigmaGauss * sqrt(2 * CV_PI));

	int halfFilterKernelWidth = filterKernelWidth / 2;
	int halfFilterKernelHeight = filterKernelHeight / 2;

	for (int i = 0; i < filterKernelHeight; ++i) {

		int y = i - halfFilterKernelHeight;
		double ySqr = y * y;

		for (int j = 0; j < filterKernelWidth; ++j) {

			int x = j - halfFilterKernelWidth;
			double xSqr = x * x;

			double exponent = static_cast<double>( (ySqr + xSqr) / (2.0 * sigmaSqr) );

			kernelImage.at<double>(i, j) = amplitude * exp(-exponent);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	cv::Mat kernelMatrix = cv::Mat::zeros(inputImageHeight, inputImageWidth, CV_64FC1);

	{
		double sumKernelElements = 0.0;

		int inputImageWidthHalf = inputImageWidth / 2;
		int inputImageHeightHalf = inputImageHeight / 2;

		/*std::shared_ptr<double[]> kernelTempMatrix(
			new double[inputImageWidth * inputImageWidth], std::default_delete<double[]>());*/

		std::vector<double> kernelTempMatrix(inputImageWidth * inputImageWidth);

		for (int y = 0; y < inputImageHeight; ++y) {
			for (int x = 0; x < inputImageWidth; ++x) {

				int index = y * inputImageWidth + x;
				double value = 0.0;

				// if we are in the kernel area (of small kernelImage), then take pixel values. 
				// Otherwise keep 0
				if ( abs(x - inputImageWidthHalf) < (filterKernelWidth - 2) / 2 && 
					 abs(y - inputImageHeightHalf) < (filterKernelHeight - 2) / 2) {
						int xLocal = x - (inputImageWidth - filterKernelWidth) / 2;
						int yLocal = y - (inputImageHeight - filterKernelHeight) / 2;

						value = kernelImage.at<double>(yLocal, xLocal);
				}

				kernelTempMatrix[index] = value;
				sumKernelElements += abs(value);
			}
		}

		// Zero-protection
		if (sumKernelElements == 0.0) {
			sumKernelElements = 1.0;
		}

		// Normalize
		double sumKernelElementsBack = 1.0 / sumKernelElements;
		for (int i = 0; i < inputImageWidth * inputImageHeight; ++i) {
			kernelTempMatrix[i] *= sumKernelElementsBack;
		}

		// Translate kernel, because we don't use centered FFT 
		// (by multiply input image on pow(-1, x + y))
		// so we need to translate kernel by (width / 2) to the left 
		// and by (height / 2) to the up
		for (int y = 0; y < inputImageHeight; ++y) {
			for (int x = 0; x < inputImageWidth; ++x) {

				int xTranslated = (x + inputImageWidth / 2) % inputImageWidth;
				int yTranslated = (y + inputImageHeight / 2) % inputImageHeight;
				(reinterpret_cast<double*>(kernelMatrix.data))[y * inputImageWidth + x] = 
					kernelTempMatrix[yTranslated * inputImageWidth + xTranslated];

// 				kernelMatrix.at<double>(y, x) = 
// 					kernelTempMatrix.get()[yTranslated * inputImageWidth + xTranslated];
			}
		}

	}

	cv::Mat kernelFFT;
	cv::dft(kernelMatrix, kernelFFT, cv::DFT_COMPLEX_OUTPUT);

	//! Rewrite this code using OpenCV functions
	for (int y = 0; y < inputImageHeight; ++y) {
		for (int x = 0; x < inputImageWidth; ++x) {

			double pointValueReal = kernelFFT.at< std::complex<double> >(y, x).real();
			double pointValueImag = kernelFFT.at< std::complex<double> >(y, x).imag();

			double pointValueRealSqr = pointValueReal * pointValueReal;
			double pointValueImagSqr = pointValueImag * pointValueImag;

			double energyValue = pointValueRealSqr + pointValueImagSqr;

			double wienerValue = pointValueReal / (energyValue + coeffWiener);
			
			kernelFFT.at< std::complex<double> >(y, x) *= wienerValue;
		}
	}

	//////////////////////////////////////////////////////////////////////////


	vector<cv::Mat> inputImageChannels, resultImageChannels;
	cv::split(inputImage, inputImageChannels);

	for (cv::Mat& inputChannel : inputImageChannels) {

		cv::Mat realTypeInputChannel;
		inputChannel.convertTo(realTypeInputChannel, CV_64F);

		cv::Mat inputChannelFFT = cv::Mat::zeros(realTypeInputChannel.size(), realTypeInputChannel.type());
		cv::dft(realTypeInputChannel, inputChannelFFT, cv::DFT_COMPLEX_OUTPUT);

		//inputChannelFFT *= 1.0 / (inputImageWidth * inputImageHeight);

		cv::Mat outputChannelFFT;
		cv::mulSpectrums(inputChannelFFT, kernelFFT, outputChannelFFT, 0);

		cv::Mat complexTypeResultChannel;
		cv::idft(outputChannelFFT, complexTypeResultChannel, cv::DFT_SCALE);

		vector<cv::Mat> resultRealTypeSubchannels;
		cv::split(complexTypeResultChannel, resultRealTypeSubchannels);

		resultRealTypeSubchannels[0].convertTo(resultRealTypeSubchannels[0], CV_8UC1);
		resultImageChannels.push_back( resultRealTypeSubchannels[0] );
	}

	cv::merge(resultImageChannels, outputImage);

}

