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


#include "balanceSimpleWhite.h"

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

    prl::simpleWhiteBalance(inputImage, outputImage, 25);

    cv::imwrite(outputImageFilename, outputImage);
}