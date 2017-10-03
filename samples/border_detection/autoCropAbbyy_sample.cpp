#include "autoCropAbbyy.h"
#include "warp.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

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

    std::vector<cv::Point> resultContour;
    bool result = prl::getContourAbbyy(inputImage, resultContour);

    if(result)
    {
        prl::warpCrop(inputImage, outputImage, resultContour);
    }
    else
    {
        outputImage = inputImage;
    }
    cv::imwrite(outputImageFilename, outputImage);
}

