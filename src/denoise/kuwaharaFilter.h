#ifndef PRLIB_KUWAHARA_FILTER_H
#define PRLIB_KUWAHARA_FILTER_H

#include <opencv2/core/core.hpp>

#include <cmath>
#include <vector>

namespace prl
{
    void denoiseKuwahara(const cv::Mat& input_img, cv::Mat& outputImage, size_t iterations);
}


/*class kuwaharaFilter {
public:
    kuwaharaFilter();

    Mat imageFilter(const Mat& input_img, int iterations);
private:
    void tensorComputation (const Mat& src_img, Mat& eigenVec_ori_cos, Mat& eigenVec_ori_sin, Mat& amo_anisotropy);

    Mat anisotropic_kuwahara(const Mat& src_img);

    Mat computationKernel(const Mat src_image, const Mat& eigenVec_ori_cos, const Mat& eigenVec_ori_sin, const Mat& amo_anisotropy);

    void div_circle_initialize();




    Mat getGaussianKernel2D ( int ksize, double sigma );

    Mat div_circle_weight[SECTOR_N];



    double ***div_circle;
    int map_circle_width, local_circle_width;
};*/

#endif //PRLIB_KUWAHARA_FILTER_H

