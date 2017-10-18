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

#ifndef PRLIB_FORMATCONVERT_HPP
#define PRLIB_FORMATCONVERT_HPP

#include <opencv2/core/core.hpp>
#include "leptonica/alltypes.h"
#include <leptonica/allheaders.h>

Pix* leptCreatePixFromMat(cv::Mat* m)
{
    const unsigned char* imagedata = m->data;
    int width = m->size().width;
    int height = m->size().height;
    int bytes_per_pixel = m->channels();
    int bytes_per_line = m->step;

    //The following code is based on tesseract's ImageThresholder.SetImage function
    int bpp = bytes_per_pixel * 8;
    if (bpp == 0) bpp = 1;
    Pix* pix = pixCreate(width, height, bpp == 24 ? 32 : bpp);
    l_uint32* data = pixGetData(pix);
    int wpl = pixGetWpl(pix);
    switch (bpp) {
        case 1:
            for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
                for (int x = 0; x < width; ++x) {
                    if (imagedata[x / 8] & (0x80 >> (x % 8)))
                        CLEAR_DATA_BIT(data, x);
                    else
                        SET_DATA_BIT(data, x);
                }
            }
            break;

        case 8:
            // Greyscale just copies the bytes in the right order.
            for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
                for (int x = 0; x < width; ++x)
                    SET_DATA_BYTE(data, x, imagedata[x]);
            }
            break;

        case 24:
            // Put the colors in the correct places in the line buffer.
            for (int y = 0; y < height; ++y, imagedata += bytes_per_line) {
                for (int x = 0; x < width; ++x, ++data) {
                    SET_DATA_BYTE(data, COLOR_RED, imagedata[3 * x]);
                    SET_DATA_BYTE(data, COLOR_GREEN, imagedata[3 * x + 1]);
                    SET_DATA_BYTE(data, COLOR_BLUE, imagedata[3 * x + 2]);
                }
            }
            break;

        case 32:
            // Maintain byte order consistency across different endianness.
            for (int y = 0; y < height; ++y, imagedata += bytes_per_line, data += wpl) {
                for (int x = 0; x < width; ++x) {
                    data[x] = (imagedata[x * 4] << 24) | (imagedata[x * 4 + 1] << 16) |
                              (imagedata[x * 4 + 2] << 8) | imagedata[x * 4 + 3];
                }
            }
            break;

        default:
            CV_Error(CV_StsError, "Cannot convert RAW image to Pix\n");
    }
    pixSetYRes(pix, 300);
    return pix;
}

cv::Mat leptCreateMatFromPix(Pix* pix)
{
    int width = pixGetWidth(pix);
    int height = pixGetHeight(pix);
    int bytes_per_pixel = pixGetDepth(pix);

    int type = 0;

    switch (bytes_per_pixel) {
        case 1:
        case 8:
            type = CV_8UC1;
            break;
        case 24:
            type = CV_8UC3;
            break;
        case 32:
            type = CV_8UC3;
            break;
        default:
            CV_Error(CV_StsError, "Cannot convert Pix image to cv::Mat\n");
            break;
    }

    cv::Mat mObj(height, width, type);
    cv::Mat* m = &mObj;

    unsigned char* imagedata = m->data;
    int bytes_per_line = m->step;

    //The following code is based on tesseract's ImageThresholder.SetImage function
    int bpp = bytes_per_pixel;

    l_uint32* data = pixGetData(pix);
    int wpl = pixGetWpl(pix);

    const char values[] = {char(255), 0};

    switch (bpp) {
        case 1:

            for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
                for (int x = 0; x < width; ++x) {
                    imagedata[x] = values[GET_DATA_BIT(data, x)];
                    /*if (imagedata[x / 8] & (0x80 >> (x % 8))) {
                        CLEAR_DATA_BIT(data, x);
                    } else {
                        SET_DATA_BIT(data, x);
                    }*/
                }
            }
            break;

        case 8:
            // Greyscale just copies the bytes in the right order.
            for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
                for (int x = 0; x < width; ++x) {
                    //SET_DATA_BYTE(data, x, imagedata[x]);
                    imagedata[x] = GET_DATA_BYTE(data, x);
                }
            }
            break;

        case 24:
            // Put the colors in the correct places in the line buffer.
            for (int y = 0; y < height; ++y, imagedata += bytes_per_line) {
                for (int x = 0; x < width; ++x, ++data) {
                    imagedata[3 * x + 0] = GET_DATA_BYTE(data, COLOR_RED);
                    imagedata[3 * x + 1] = GET_DATA_BYTE(data, COLOR_GREEN);
                    imagedata[3 * x + 2] = GET_DATA_BYTE(data, COLOR_BLUE);

                    /*SET_DATA_BYTE(data, COLOR_RED, imagedata[3 * x]);
                    SET_DATA_BYTE(data, COLOR_GREEN, imagedata[3 * x + 1]);
                    SET_DATA_BYTE(data, COLOR_BLUE, imagedata[3 * x + 2]);*/
                }
            }
            break;

        case 32:
            // Maintain byte order consistency across different endianness.
            for (int y = 0; y < height; ++y, imagedata += bytes_per_line, data += wpl) {
                for (int x = 0; x < width; ++x) {
                    imagedata[x * 3 + 0] = (data[x] >> 24) & 0xff;
                    imagedata[x * 3 + 1] = (data[x] >> 16) & 0xff;
                    imagedata[x * 3 + 2] = (data[x] >> 8) & 0xff;
                    //imagedata[x * 4 + 3] = (data[x]) & 0xff;

                    //data[x] = (imagedata[x * 4] << 24) | (imagedata[x * 4 + 1] << 16) |
                    //	(imagedata[x * 4 + 2] << 8) | imagedata[x * 4 + 3];
                }
            }
            break;

        default:
            CV_Error(CV_StsError, "Cannot convert Pix image to cv::Mat\n");
    }

    return mObj;
}


namespace prl
{
PIX* ImgOpenCvToLepton(const cv::Mat src);
cv::Mat ImgLeptonToOpenCV(const PIX* src);
}

#endif //PRLIB_FORMATCONVERT_HPP
