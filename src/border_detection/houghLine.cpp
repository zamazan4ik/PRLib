#include "HoughLine.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <algorithm>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "binarizeLocalOtsu.h"
#include "imageLibCommon.h"
#include "utils.h"


bool intersection(cv::Point2f o1, cv::Point2f p1, cv::Point2f o2, cv::Point2f p2,
                  cv::Point2f& r)
{
    cv::Point2f x = o2 - o1;
    cv::Point2f d1 = p1 - o1;
    cv::Point2f d2 = p2 - o2;

    float cross = d1.x * d2.y - d1.y * d2.x;
    if (prl::eq_d(cross, 0.0))
    {
        return false;
    }

    double t1 = (x.x * d2.y - x.y * d2.x) / cross;
    r = o1 + d1 * t1;
    return true;
}

std::pair<cv::Point2f, cv::Point2f> fromVec2f(cv::Vec2f vec)
{
    float rho = vec[0], theta = vec[1];
    cv::Point2f pt1, pt2;
    double a = std::cos(theta), b = std::sin(theta);
    double x0 = a * rho, y0 = b * rho;
    pt1.x = static_cast<float>(cvRound(x0 + 1000 * (-b)));
    pt1.y = static_cast<float>(cvRound(y0 + 1000 * (a)));
    pt2.x = static_cast<float>(cvRound(x0 - 1000 * (-b)));
    pt2.y = static_cast<float>(cvRound(y0 - 1000 * (a)));

    return std::make_pair(pt1, pt2);
}

double distanceToLine(cv::Point line_start, cv::Point line_end, cv::Point point)
{
    double normalLength = std::hypot(line_end.x - line_start.x, line_end.y - line_start.y);
    double distance =
            (double) (
                    (point.x - line_start.x) * (line_end.y - line_start.y) -
                    (point.y - line_start.y) * (line_end.x - line_start.x)
            ) / normalLength;
    return std::fabs(distance);
}

void deleteSimilarLines(
        std::vector<cv::Vec2f>& inputLines,
        double minDist,
        const double stepDist,
        const int maxLines)
{
    std::vector<cv::Vec2f> tmpLines;
    std::vector<char> used(inputLines.size(), false);
    while (static_cast<int>(inputLines.size()) > maxLines)
    {
        std::fill(used.begin(), used.end(), false);
        tmpLines.clear();

        for (size_t i = 0; i < inputLines.size(); ++i)
        {
            if (used[i])
            {
                continue;
            }
            used[i] = true;
            tmpLines.push_back(inputLines[i]);
            auto vec1 = fromVec2f(inputLines[i]);
            for (size_t j = i + 1; j < inputLines.size(); ++j)
            {
                if (used[j])
                {
                    continue;
                }
                auto vec2 = fromVec2f(inputLines[j]);

                //Calculate minimal distance between two line segments
                double dist =
                        std::min(
                                std::min(
                                        distanceToLine(vec1.first, vec1.second, vec2.first),
                                        distanceToLine(vec1.first, vec1.second, vec2.second)
                                ),
                                std::min(
                                        distanceToLine(vec2.first, vec2.second, vec1.first),
                                        distanceToLine(vec2.first, vec2.second, vec1.second)
                                )
                        );

                //If distance < minDist, we delete this line
                if (dist < minDist)
                {
                    used[j] = true;
                }
            }
        }

        if (tmpLines.size() < 4)
        {
            inputLines.resize(maxLines);
            break;
        }
        inputLines = tmpLines;
        minDist += stepDist;
    }
}

std::vector<cv::Point2f> findMaxValidContour(const std::vector<cv::Vec2f>& lines, cv::Size size)
{
    double maxFoundedArea = 0.0;
    std::vector<cv::Point2f> resultVec;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        for (size_t j = 0; j < lines.size(); ++j)
        {
            if (i == j)
            {
                continue;
            }
            for (size_t k = 0; k < lines.size(); ++k)
            {
                if (i == k || j == k)
                {
                    continue;
                }
                for (size_t m = 0; m < lines.size(); ++m)
                {
                    if (i == m || j == m || k == m)
                    {
                        continue;
                    }

                    //Here we check lines for intersection.
                    std::vector<cv::Point2f> points(4);
                    std::vector<std::pair<cv::Point2f, cv::Point2f> > curLines;
                    curLines.push_back(fromVec2f(lines[i]));
                    curLines.push_back(fromVec2f(lines[j]));
                    curLines.push_back(fromVec2f(lines[k]));
                    curLines.push_back(fromVec2f(lines[m]));
                    curLines.push_back(curLines[0]);

                    bool allIntersect = true;
                    for (size_t ind = 1; ind < curLines.size(); ++ind)
                    {
                        bool areLinesIntersected =
                                intersection(
                                        curLines[ind].first, curLines[ind].second,
                                        curLines[ind - 1].first, curLines[ind - 1].second,
                                        points[ind - 1]
                                );

                        if (!areLinesIntersected)
                        {
                            allIntersect = false;
                            break;
                        }
                    }
                    if (!allIntersect)
                    {
                        continue;
                    }


                    bool validCoord = false;
                    for (const auto& val : points)
                    {
                        //Check point's coordinates
                        //Maybe be wrong, if corner is outside
                        if (val.x < 0 || val.x > size.width || val.y < 0 || val.y > size.height)
                        {
                            validCoord = true;
                            break;
                        }
                    }
                    if (validCoord)
                    {
                        continue;
                    }

                    //Find the largest convex quadrangular area
                    cv::Mat temp(points);
                    double tempArea = cv::contourArea(temp);
                    if (cv::isContourConvex(temp) && IsQuadrangularConvex(points) &&
                        tempArea > maxFoundedArea)
                    {
                        maxFoundedArea = tempArea;
                        resultVec = points;
                    }
                }
            }
        }
    }

    return resultVec;
}

bool findHoughLineContour(cv::Mat& inputImage,
                          const double scaleX, const double scaleY,
                          const int nProcessedImageSize,
                          std::vector<cv::Point2f>& resultContour)
{
    if (inputImage.empty())
    {
        throw std::invalid_argument("Input image is empty");
    }

    cv::Mat imageToProc;

    //! Store source image size
    cv::Size inputImageSize(inputImage.size());

    cv::Size newImageSize;

    if (scaleX > 0 && scaleY > 0)
    {
        newImageSize = cv::Size(
                static_cast<int>(inputImage.cols * scaleX),
                static_cast<int>(inputImage.rows * scaleY)
        );

        cv::resize(inputImage, imageToProc, newImageSize, 0, 0, cv::INTER_AREA);
    }
    else
    {
        int scaleFactorX = 1;
        int scaleFactorY = 1;

        int longSide = std::max(inputImage.cols, inputImage.rows);

        imageToProc = inputImage.clone();

        while (longSide > nProcessedImageSize)
        {
            cv::pyrDown(imageToProc, imageToProc);

            longSide = std::max(imageToProc.cols, imageToProc.rows);
            scaleFactorX *= 2;
            scaleFactorY *= 2;
        }

        newImageSize = cv::Size(inputImage.cols / scaleFactorX, inputImage.rows / scaleFactorY);
    }
    // TODO: 1) Median blur 6 times with  kernel sizes 3 and 5 - done
    //       2) Use binarization (LocalOtsu) - done
    //       3) Canny - done
    //       4) Dilate - done
    //       5) HoughLine - done
    //       6) Find largest area - done
    //       7) Add heuristics for angles


    // Median blur with two kernels
    for (size_t i = 0; i < 3; ++i)
    {
        cv::medianBlur(imageToProc, imageToProc, 3);
    }
    for (size_t i = 0; i < 3; ++i)
    {
        cv::medianBlur(imageToProc, imageToProc, 5);
    }

    // Binarization
    cv::Mat binarizedImage;
    prl::binarizeLocalOtsu(imageToProc, binarizedImage);

    // Canny
    cv::Mat resultCanny = binarizedImage;
    double upper = 50;
    double lower = 25;
    cv::Canny(imageToProc, resultCanny, lower, upper);

    // Dilate
    cv::dilate(resultCanny, resultCanny, cv::Mat(), cv::Point(-1, -1), 3);

    std::vector<cv::Vec2f> lines;
    const int thresholdHough = 50;
    cv::HoughLines(resultCanny, lines, 1, CV_PI / 180, thresholdHough, 0, 0);

    //Can't create quadrilateral with less than 4 lines
    if (lines.size() < 4)
    {
        return false;
    }

    //Delete similar lines
    deleteSimilarLines(lines, 5.0, 5.0, 20);

    //Bruteforce lines. Try to find 4 lines for building the largest area
    auto resultVec = findMaxValidContour(lines, resultCanny.size());

    if (resultVec.empty())
    {
        return false;
    }

    resultContour = resultVec;

    float xScale = static_cast<float>(inputImageSize.width) /
                   static_cast<float>(newImageSize.width);
    float yScale = static_cast<float>(inputImageSize.height) /
                   static_cast<float>(newImageSize.height);

    for (auto& contour : resultContour)
    {
        contour.x *= xScale;
        contour.y *= yScale;
    }

    return true;
}


bool prl::houghLineContourDetector(
        cv::Mat* inputImage,
        const double scaleX, const double scaleY,
        const int nProcessedImageSize,
        int* points)
{
    if (inputImage == nullptr)
    {
        throw std::invalid_argument("Input image pointer equals NULL");
    }

    if (inputImage->empty())
    {
        throw std::invalid_argument("Input image is empty");
    }

    if (points == nullptr)
    {
        throw std::invalid_argument("Output array should be allocated");
    }

    std::vector<cv::Point2f> resultContour;

    bool resultCode = findHoughLineContour(
            *inputImage,
            scaleX, scaleY,
            nProcessedImageSize,
            resultContour);

    if (!resultCode || resultContour.empty() || resultContour.size() != 4)
    {
        return false;
    }

    //////////////////////////////////////////////////////////////////////////

    size_t pointNumber = 0;
    for (const auto& point : resultContour)
    {
        points[pointNumber++] = static_cast<int>(point.x);
        points[pointNumber++] = static_cast<int>(point.y);
    }

    return true;
}