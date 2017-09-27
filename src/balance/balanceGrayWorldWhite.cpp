#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


void prl::getAverageValues(const cv::Mat& src, double* ml, double* ma, double* mb, const double p)
{
    for (int i = 0; i < src.rows; ++i)
    {
        for (int j = 0; j < src.cols; ++j)
        {
            cv::Vec3b v1 = src.at<cv::Vec3b>(i, j);
            double lc = pow(v1.val[0], p);
            double ac = pow(v1.val[1], p);
            double bc = pow(v1.val[2], p);

            *ma += ac;
            *mb += bc;
            *ml += lc;
        }
    }

    *ml = pow(*ml / (src.cols * src.rows), 1.0 / p);
    *ma = pow(*ma / (src.cols * src.rows), 1.0 / p);
    *mb = pow(*mb / (src.cols * src.rows), 1.0 / p);
}


void prl::grayWorldWhiteBalance(const cv::Mat& inputImage, cv::Mat& outputImage)
{
    cv::Mat inputImageMat = inputImage.clone();

    if (inputImageMat.empty())
    {
        throw std::invalid_argument("GrayWorldWhiteBalance exception: input image is empty");
    }

    if (inputImageMat.channels() != 3)
    {
        throw std::invalid_argument("GrayWorldWhiteBalance exception: input image hasn't 3 channels");
    }

    cv::Mat outputImageMat;
    outputImageMat = inputImageMat.clone();

    double ma = 0.0, mb = 0.0, ml = 0.0;

    getAverageValues(inputImageMat, &ml, &ma, &mb, m_pNorm);

    double r = (ma + mb + ml) / 3.0;

    if (m_withMax)
    {
        r = std::max(ma, std::max(mb, ml));
    }

    double r_div_ml = r / ml;
    double r_div_ma = r / ma;
    double r_div_mb = r / mb;

    auto itout = outputImageMat.begin<cv::Vec3b>();
    for (auto it = inputImageMat.begin<cv::Vec3b>(), itend = inputImageMat.end<cv::Vec3b>(); it != itend; ++it, ++itout)
    {
        cv::Vec3b v1 = *it;

        double l = v1.val[0];
        double a = v1.val[1];
        double b = v1.val[2];

        //l = l * (r / ml);
        //a = a * (r / ma);
        //b = b * (r / mb);
        l *= r_div_ml;
        a *= r_div_ma;
        b *= r_div_mb;

        //l = std::min(255.0, l);
        //a = std::min(255.0, a);
        //b = std::min(255.0, b);
        if (l > 255.0)
        { l = 255.0; }
        if (a > 255.0)
        { a = 255.0; }
        if (b > 255.0)
        { b = 255.0; }

        v1.val[0] = static_cast<uchar>(l);
        v1.val[1] = static_cast<uchar>(a);
        v1.val[2] = static_cast<uchar>(b);

        *itout = v1;
    }

    outputImage = outputImageMat.clone();
}
