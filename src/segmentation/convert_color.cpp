/**
 * File name : convert_color.c
 *
 * File Description : Convert a color space to another color space 
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Main_def.h"

#define nonli_func(x) ((x)>(0.008856)? (pow((x),(0.3333))):(7.787*(x)+0.1379))

#define XYZtosRGB_matrix {\
  { 3.2406,  -1.5372,  -0.4986},\
  {-0.9689,   1.8758,   0.0415},\
  { 0.0557,  -0.2040,   1.0570}}

#define REFWHITE_Y 100

/* External variables */
double calib_T[3][3];
double calib_a[3], calib_b[3], calib_c[3];

/**
 * Function Name : Read_calib_info 
 *
 * Function Description :
 * Read scanner calibration info for converting RGB to XYZ 
 *
 * Input       : filename, file name 
 * Output      : calib_T, calib_a, calib_b, calib_c (external variables) 
 * Version : 1.0
 */

void Read_calib_info
(
    char* filename              
)
{
    FILE* fp;

    /* read files */
    fp = fopen(filename, "rb") ;
    if( fp == NULL ) {
        fprintf(stderr, "%s file open error\n", filename) ;
    }
 
    fscanf (fp, "%lf %lf %lf", &calib_a[0], &calib_b[0], &calib_c[0]);
    fscanf (fp, "%lf %lf %lf", &calib_a[1], &calib_b[1], &calib_c[1]);
    fscanf (fp, "%lf %lf %lf", &calib_a[2], &calib_b[2], &calib_c[2]);

    fscanf (fp, "%lf %lf %lf", &calib_T[0][0], &calib_T[0][1], &calib_T[0][2]);
    fscanf (fp, "%lf %lf %lf", &calib_T[1][0], &calib_T[1][1], &calib_T[1][2]);
    fscanf (fp, "%lf %lf %lf", &calib_T[2][0], &calib_T[2][1], &calib_T[2][2]);
 
    fclose(fp);

}

/**
 * Function Name : Convert_RGB_to_XYZ 
 *
 * Function Description :
 * Convert RGB space to XYZ space
 *
 * Input       : rgb, RGB colors
 * Output      : xyz, XYZ colors
 * Version : 1.0
 */

void Convert_RGB_to_XYZ 
(
    double* rgb,              
    double* xyz 
)
{
    double rgb_lin[3];
    unsigned int i, j;

    /* Convert gamma RGB to linear RGB */
    rgb_lin[0] = 100*(calib_a[0]*pow(rgb[0]/255.0,calib_b[0])+calib_c[0]); 
    rgb_lin[1] = 100*(calib_a[1]*pow(rgb[1]/255.0,calib_b[1])+calib_c[1]); 
    rgb_lin[2] = 100*(calib_a[2]*pow(rgb[2]/255.0,calib_b[2])+calib_c[2]); 

    /* Convert linear RGB to XYZ */
    for ( j = 0 ; j < 3 ; j++ )
    {
        xyz[j] = 0;
        for ( i = 0 ; i < 3 ; i++ )
            xyz[j] += (double)(calib_T[j][i]*rgb_lin[i]);

        if ( xyz[j] < 0 ) 
        {
            xyz[j] = 0;
        }

    }
}

/**
 * Function Name : Convert_XYZ_to_sRGB 
 *
 * Function Description :
 * Convert XYZ space to sRGB space. If linear_flg is ON, this function 
 * returns linear sRGB value.
 *
 * Input       : xyz, XYZ colors
 * Output      : sRGB, sRGB colors
 * Version : 1.0
 */
void Convert_XYZ_to_sRGB 
(
    double* xyz,              
    double* srgb,
    unsigned char linear_flg 
)
{
    unsigned int i, j;
    double T[3][3] = XYZtosRGB_matrix;
    double XYZ[3], lin_sRGB[3];
    double refwhite_y = REFWHITE_Y;
    int tmp;

    /* Normalized by y-value */
    for ( i = 0 ; i < 3 ;i++ )
        XYZ[i] = (double)xyz[i]/refwhite_y;

    /* Calculate linear sRGB */
    for ( j = 0 ; j < 3 ; j++ )
    {
        lin_sRGB[j] = 0.0;
        for ( i = 0 ; i < 3 ; i++ )
            lin_sRGB[j] += (T[j][i]*XYZ[i]);

        if ( lin_sRGB[j] < 0.0 ) 
        {
            lin_sRGB[j] = 0.0;
        }
        else if ( lin_sRGB[j] > 1.0 )
        {
            lin_sRGB[j] = 1.0;
        }
    } 
  
    if ( linear_flg == FLG_ON )
    {
        for ( i = 0 ; i < 3 ; i++ )
            srgb[i] = (int)(255*lin_sRGB[i]);
        return;
    } 

    /* Calculate sRGB */
    for ( i = 0 ; i < 3 ;i++ )
    {
        if ( lin_sRGB[i] <= 0.0031308 )
        {
            tmp = (int)(255*(lin_sRGB[i]*12.92));
        }
        else
        {
            tmp = (int)(255*(1.055*pow(lin_sRGB[i],0.41666)-0.055));
        }
        if ( tmp > 255 )
            srgb[i] = 255;
        else if ( tmp < 0 )
            srgb[i] = 0;
        else
            srgb[i] = tmp; 
    }
    
}

/**
 * Function Name : Convert_RGB_to_sRGB_img 
 *
 * Function Description :
 * Convert scanned RGB space to sRGB space for entire image 
 *
 * Input       : inp_img, input image 
 *             : height, image height 
 *             : width, image width 
 * Output      : out_img, output image  
 * Version : 1.0
 */
void Convert_RGB_to_sRGB_img
(
    unsigned char*** inp_img,              
    unsigned int height,
    unsigned int width,
    unsigned char ***out_img
)
{
    unsigned int i, j, k;
    double tmp1[3], tmp2[3], tmp3[3];

    for ( i = 0 ; i < height ; i++ )
    {
        for ( j = 0 ; j < width ; j++ )
        {
            for ( k = 0 ; k < 3 ; k++ ) 
            {
                tmp1[k] = (double)inp_img[k][i][j];
            }
            /* convert RGB to sRGB */
            Convert_RGB_to_XYZ(tmp1, tmp2); 
            Convert_XYZ_to_sRGB(tmp2, tmp3, FLG_OFF); 

            for ( k = 0 ; k < 3 ; k++ ) 
              out_img[k][i][j] = (unsigned char)tmp3[k];
        }
    }
}

