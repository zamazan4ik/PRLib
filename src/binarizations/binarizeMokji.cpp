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

#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace prl
{

void binarizeMokji(const cv::Mat& inputImage, cv::Mat& outputImage,
                   size_t max_edge_width = 3, size_t min_edge_magnitude = 20)
{
    if (max_edge_width < 1)
    {
        throw std::invalid_argument("mokjiThreshold: invalud max_edge_width");
    }
    if (min_edge_magnitude < 1)
    {
        throw std::invalid_argument("mokjiThreshold: invalid min_edge_magnitude");
    }

    cv::Mat gray;
    cv::cvtColor(inputImage, gray, CV_BGR2GRAY);

    const int dilateSize = (max_edge_width + 1) * 2 - 1;

    cv::Mat dilatedImage;
    cv::dilate(gray, dilatedImage, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(dilateSize, dilateSize)));

    unsigned matrix[256][256];
    memset(matrix, 0, sizeof(matrix));

    int const w = inputImage.cols;
    int const h = inputImage.rows;
    unsigned char const* src_line = gray.data;
    int const src_stride = gray.cols;
    unsigned char const* dilated_line = dilatedImage.data;
    int const dilated_stride = dilatedImage.cols;

    src_line += max_edge_width * src_stride;
    dilated_line += max_edge_width * dilated_stride;
    for (int y = max_edge_width; y < h - (int) max_edge_width; ++y)
    {
        for (int x = max_edge_width; x < w - (int) max_edge_width; ++x)
        {
            unsigned const pixel = src_line[x];
            unsigned const darkest_neighbor = dilated_line[x];
            assert(darkest_neighbor <= pixel);

            ++matrix[darkest_neighbor][pixel];
        }
        src_line += src_stride;
        dilated_line += dilated_stride;
    }

    unsigned nominator = 0;
    unsigned denominator = 0;
    for (unsigned m = 0; m < 256 - min_edge_magnitude; ++m)
    {
        for (unsigned n = m + min_edge_magnitude; n < 256; ++n)
        {
            assert(n >= m);

            unsigned const val = matrix[m][n];
            nominator += (m + n) * val;
            denominator += val;
        }
    }

    if (denominator == 0)
    {
        cv::threshold(inputImage, outputImage, 128, 255, CV_THRESH_BINARY);
    }

    const double threshold = 0.5 * nominator / denominator;
    cv::threshold(inputImage, outputImage, (int) (threshold + 0.5), 255, CV_THRESH_BINARY);
}

}

