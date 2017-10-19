#include "deskew.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char**argv)
{
    const std::string inputImageFilename = argv[1];
    const std::string outputImageFilename = argv[2];

    if (inputImageFilename.empty())
    {
        throw std::invalid_argument("Input image filename is empty.");
    }

    if (outputImageFilename.empty())
    {
        throw std::invalid_argument("Output image file name is empty.");
    }

    cv::Mat inputImage = cv::imread(inputImageFilename);
    cv::Mat outputImage;

    std::cout << "Possible angle by findOrientation function: " << prl::findOrientation(inputImage) << std::endl;

    std::cout << "Possible angle by findAngle function: " << prl::findAngle(inputImage) << std::endl;

    prl::deskew(inputImage, outputImage);

    cv::imwrite(outputImageFilename, outputImage);
}

