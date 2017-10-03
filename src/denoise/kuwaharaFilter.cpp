
#include "kuwaharaFilter.h"

#include <cmath>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "imageLibCommon.h"

static const double PI              = 3.14159265;
static const double GAUSSIAN_SIGMA  = 4.0;
static const double ECCEN_TUNING    = 1.0;
static const double SIGMA_R         = 3.0;
static const double SIGMA_S         = 3.0;
static const double SHARPNESS_Q     = 8.0;
static const int    SECTOR_N        = 8;



// TODO: remove global variables
int map_circle_width, local_circle_width;
double ***div_circle;
std::vector<cv::Mat> div_circle_weight;


// Prototypes
cv::Mat anisotropic_kuwahara(const cv::Mat& src_img);
cv::Mat computationKernel(const cv::Mat src_image, const cv::Mat& eigenVec_ori_cos,
                          const cv::Mat& eigenVec_ori_sin, const cv::Mat& amo_anisotropy);


void div_circle_initialize()
{

    double map_angle = 0.0, w_min = 0.0, w_max = 0.0;

    cv::Mat gau_kernel = getGaussianKernel2D(map_circle_width, SIGMA_R);
    cv::Mat div_rotate;
    cv::Point2d map_centroid;

    int c_i = 0, c_j = 0, map_i = 0, map_j = 0;


    div_circle = new double** [SECTOR_N];
    for (int s = 0; s < SECTOR_N; s++)
    {
        div_circle_weight[s].create(map_circle_width, map_circle_width, CV_64FC1);
        div_circle_weight[s].setTo(0);

        div_circle[s] = new double* [map_circle_width];
        for (int c_i = 0; c_i < map_circle_width; c_i++)
        {
            div_circle[s][c_i] = new double[map_circle_width];
        }
    }

    for (int s = 0; s < SECTOR_N; s++)
    {
        if (s <= 1)
        {
            for (int c_i = 0; c_i < map_circle_width; c_i++)
            {
                for (int c_j = 0; c_j < map_circle_width; c_j++)
                {
                    map_i = c_i - map_circle_width / 2;
                    map_j = c_j - map_circle_width / 2;

                    if (std::sqrt(std::pow(map_i, 2.0) + std::pow(map_j, 2.0)) < map_circle_width / 2)
                    {
                        map_angle = atan2(map_i, map_j);

                        if (map_angle >= (2.0 * s - 1) * PI / SECTOR_N && map_angle <= (2.0 * s + 1) * PI / SECTOR_N)
                        {
                            div_circle_weight[s].at<double>(c_i, c_j) = 1.0;
                        }

                    }

                }
            }
            cv::GaussianBlur(div_circle_weight[s], div_circle_weight[s], cv::Size(13, 13), SIGMA_S);

            // multiply per element
            cv::multiply(div_circle_weight[s], gau_kernel, div_circle_weight[s]);


            // calculate the max & min value of map_circle
            cv::minMaxLoc(div_circle_weight[s], &w_min, &w_max);

            // Normalize
            div_circle_weight[s] = div_circle_weight[s] / w_max;
        }
        else
        {
            map_centroid.x = map_circle_width / 2;
            map_centroid.y = map_circle_width / 2;

            if (s % 2 == 0)
            {
                div_rotate = cv::getRotationMatrix2D(map_centroid, s * (-360.0 / SECTOR_N), 1.0);
                cv::warpAffine(div_circle_weight[0], div_circle_weight[s], div_rotate, div_circle_weight[s].size());
            }

            else
            {
                div_rotate = cv::getRotationMatrix2D(map_centroid, (s - 1) * (-360.0 / SECTOR_N), 1.0);
                cv::warpAffine(div_circle_weight[1], div_circle_weight[s], div_rotate, div_circle_weight[s].size());
            }
        }

        for (int c_i = 0; c_i < map_circle_width; c_i++)
        {
            for (int c_j = 0; c_j < map_circle_width; c_j++)
            {
                div_circle[s][c_i][c_j] = div_circle_weight[s].at<double>(c_i, c_j);
            }
        }
    }

    gau_kernel.release();
    div_rotate.release();
    map_centroid.~Point_();
}



/*kuwaharaFilter::kuwaharaFilter()
{
    //map_circle_width = (int) (2 * ceil(2 * SIGMA_R) + 1);
    //local_circle_width = (int) (2 * ceil(2 * SIGMA_R) + 1);

    div_circle_initialize();
}*/


void prl::denoiseKuwahara(const cv::Mat& inputImage, cv::Mat& outputImage, size_t iterations)
{
    //Init

    map_circle_width = (int) (2 * std::ceil(2 * SIGMA_R) + 1);
    local_circle_width = (int) (2 * std::ceil(2 * SIGMA_R) + 1);

    div_circle_initialize();




    cv::Mat src_img = inputImage.clone();

    if (src_img.type() != CV_64FC3)
    {
        src_img.convertTo(src_img, CV_64FC3);
        src_img /= 255.0;
    }
    // TODO: WTF?
    //resizeMat(src_img, 0.8);

    cv::Mat filtered_img = src_img.clone().setTo(0);

    cv::Mat src_clone = src_img.clone();

    for (size_t i = 0; i < iterations; ++i)
    {
        filtered_img = anisotropic_kuwahara(src_clone);

        src_clone = filtered_img.clone();
    }

    outputImage = filtered_img;
}

void tensorComputation(const cv::Mat& src_img, cv::Mat& eigenVec_ori_cos, cv::Mat& eigenVec_ori_sin,
                       cv::Mat& amo_anisotropy)
{
    cv::Mat src_gau = src_img.clone().setTo(0);
    cv::GaussianBlur(src_img, src_gau, cv::Size(3, 3), GAUSSIAN_SIGMA);

    cv::Mat src_dx_split[3], src_dy_split[3], src_gau_split[3];
    cv::split(src_gau, src_gau_split);

    for (size_t i = 0; i < 3; i++)
    {
        src_dx_split[i] = cv::Mat(src_img.rows, src_img.cols, CV_64FC1).setTo(0);
        src_dy_split[i] = cv::Mat(src_img.rows, src_img.cols, CV_64FC1).setTo(0);

        cv::Sobel(src_gau_split[i], src_dx_split[i], CV_64FC1, 1, 0, 1);
        cv::Sobel(src_gau_split[i], src_dy_split[i], CV_64FC1, 0, 1, 1);
    }

    cv::Mat src_dx = src_img.clone().setTo(0), src_dy = src_img.clone().setTo(0);

    cv::merge(src_dx_split, 3, src_dx);
    cv::merge(src_dy_split, 3, src_dy);

    cv::Mat struct_tensor, eigen_val, eigen_vec;

    double lambda_one = 0.0, lambda_two = 0.0;

    eigen_val.create(2, 1, CV_64FC1);
    eigen_vec.create(2, 1, CV_64FC1);

    for (size_t i = 0; i < src_img.rows; i++)
    {
        for (size_t j = 0; j < src_img.cols; j++)
        {
            struct_tensor = cv::Mat(2, 2, CV_64FC1).setTo(0);

            cv::Vec3d dx_temp = src_dx.at<cv::Vec3d>(i, j), dy_temp = src_dy.at<cv::Vec3d>(i, j);

            for (size_t k = 0; k < 3; k++)
            {
                struct_tensor.at<double>(0, 0) += std::pow(dx_temp[k], 2.0);
                struct_tensor.at<double>(1, 0) += dx_temp[k] * dy_temp[k];
                struct_tensor.at<double>(0, 1) += dx_temp[k] * dy_temp[k];
                struct_tensor.at<double>(1, 1) += std::pow(dy_temp[k], 2.0);
            }


            cv::eigen(struct_tensor, eigen_val, eigen_vec);

            lambda_one = ((double*) eigen_val.data)[0];
            lambda_two = ((double*) eigen_val.data + 1)[0];

            double eigenVec_ori = atan2(lambda_one - struct_tensor.at<double>(0, 0),
                                        (-1) * struct_tensor.at<double>(1, 0));

            eigenVec_ori_cos.at<double>(i, j) = cos(eigenVec_ori);
            eigenVec_ori_sin.at<double>(i, j) = sin(eigenVec_ori);
            amo_anisotropy.at<double>(i, j) = (lambda_one - lambda_two) / (lambda_one + lambda_two);
        }
    }
}


cv::Mat anisotropic_kuwahara(const cv::Mat& src_img)
{
    cv::Mat eigenVec_ori_cos = cv::Mat(src_img.rows, src_img.cols, CV_64FC1).setTo(0);
    cv::Mat eigenVec_ori_sin = cv::Mat(src_img.rows, src_img.cols, CV_64FC1).setTo(0);
    cv::Mat amo_anisotropy = cv::Mat(src_img.rows, src_img.cols, CV_64FC1).setTo(0);

    tensorComputation(src_img, eigenVec_ori_cos, eigenVec_ori_sin, amo_anisotropy);

    cv::Mat src_split[3], filtered_split[3];
    cv::Mat filtered_img = src_img.clone().setTo(0);

    cv::split(src_img, src_split);

    //#pragma omp parallel for
    for (size_t i = 0; i < 3; i++)
    {
        filtered_split[i] = computationKernel(src_split[i], eigenVec_ori_cos, eigenVec_ori_sin, amo_anisotropy);
    }

    cv::merge(filtered_split, 3, filtered_img);

    return filtered_img;
}


cv::Mat computationKernel(const cv::Mat src_image, const cv::Mat& eigenVec_ori_cos,
                          const cv::Mat& eigenVec_ori_sin, const cv::Mat& amo_anisotropy)
{
    cv::Mat filtered_image = src_image.clone().setTo(0);

    // temp use
    double temp_i = 0, temp_j = 0, temp_A = 0.0, temp_B = 0.0, temp_C = 0.0, temp_D = 0.0, temp_cos = 0.0, temp_sin = 0.0, temp_adA = 0.0, temp_Ada = 0.0, temp_det = 0.0;

    int map_i = 0, map_j = 0, half_l_width = 0, half_m_width = 0;
    double div_mean[8] = {0.0}, div_s[8] = {0.0}, weight_alpha[8] = {0.0},
           normalize_k = 0.0, normalize_alpha = 0.0, eccen_S_A[2][2];

    double div_temp_data = 0.0, map_circle_data = 0.0, add_temp = 0.0, l_m_ratio = 0.0;

    cv::Point2d map_centroid(map_circle_width / 2, map_circle_width / 2);


    half_m_width = map_circle_width / 2;
    half_l_width = local_circle_width / 2;

    l_m_ratio = (double) local_circle_width / map_circle_width;

    double** m_circle, ** l_circle;
    m_circle = new double* [map_circle_width];
    l_circle = new double* [local_circle_width];


    for (int i = 0; i < map_circle_width; i++)
    {
        m_circle[i] = new double[map_circle_width];

        if (i < local_circle_width)
        {
            l_circle[i] = new double[local_circle_width];
        }
    }


    for (int i = 1; i < src_image.rows - 1; i++)
    {
        for (int j = 1; j < src_image.cols - 1; j++)
        {
            temp_cos = eigenVec_ori_cos.at<double>(i, j);
            temp_sin = eigenVec_ori_sin.at<double>(i, j);
            temp_adA = (ECCEN_TUNING) / (ECCEN_TUNING + amo_anisotropy.at<double>(i, j));
            temp_Ada = (ECCEN_TUNING + amo_anisotropy.at<double>(i, j)) / (ECCEN_TUNING);

            temp_A = temp_cos * temp_adA;
            temp_B = (-1) * temp_sin * temp_Ada;
            temp_C = temp_sin * temp_adA;
            temp_D = temp_cos * temp_Ada;

            temp_det = 1.0 / (temp_A * temp_D - temp_B * temp_C);

            eccen_S_A[0][0] = temp_D * temp_det;
            eccen_S_A[1][0] = (-1) * temp_B * temp_det;
            eccen_S_A[0][1] = (-1) * temp_C * temp_det;
            eccen_S_A[1][1] = temp_A * temp_det;


            for (int c_i = 0; c_i < map_circle_width; c_i++)
            {
                for (int c_j = 0; c_j < map_circle_width; c_j++)
                {
                    m_circle[c_i][c_j] = 0;

                    // temp_A = c_i upper, temp_B = c_j upper
                    temp_i = (int) (l_m_ratio * c_i) - half_l_width;
                    temp_j = (int) (l_m_ratio * c_j) - half_l_width;


                    if (sqrt(temp_i * temp_i + temp_j * temp_j) <= half_l_width)
                    {
                        map_i = (int) eccen_S_A[0][0] * temp_i + eccen_S_A[0][1] * temp_j;
                        map_j = (int) eccen_S_A[1][0] * temp_i + eccen_S_A[1][1] * temp_j;

                        // map eclipse shape local texture to circle
                        if (map_i + i >= 0 && map_i + i < src_image.rows && map_j + j >= 0 &&
                            map_j + j < src_image.cols)
                        {
                            m_circle[c_i][c_j] = src_image.at<double>(map_i + i, map_j + j);
                        }

                    }

                }
            }

            for (int s = 0; s < SECTOR_N; s++)
            {
                div_mean[s] = 0.0;
                div_s[s] = 0.0;
                weight_alpha[s] = 0.0;
                normalize_k = 0.00000001;

                for (int c_i = 0; c_i <= map_circle_width - 1; c_i++)
                {
                    for (int c_j = 0; c_j <= map_circle_width - 1; c_j++)
                    {
                        div_temp_data = div_circle[s][c_i][c_j];
                        map_circle_data = m_circle[c_i][c_j];

                        if (map_circle_data <= 1 && map_circle_data >= 0.0000001
                            && div_temp_data <= 1 && div_temp_data >= 0.0000001)
                        {
                            div_mean[s] += map_circle_data * div_temp_data;
                            div_s[s] += map_circle_data * map_circle_data * div_temp_data;
                            normalize_k += div_temp_data;
                        }
                    }
                }

                div_mean[s] = div_mean[s] / normalize_k;
                div_s[s] = std::sqrt(div_s[s] / normalize_k - div_mean[s] * div_mean[s]);
            }


            size_t min_index = 0;
            double min = INFINITY;
            for (size_t s = 0; s < SECTOR_N; ++s)
            {
                if (min > div_s[s])
                {
                    min = div_s[s];
                    min_index = s;
                }
            }

            filtered_image.at<double>(i, j) = std::max<double>(std::min<double>(div_mean[min_index], 1), 0);
        }
    }

    for (int i = 0; i < map_circle_width; i++)
    {
        delete[] m_circle[i];
    }

    delete[] m_circle;

    return filtered_image;
}
