/**
 * File name : convert_color.h
 *
 * File Description : This is a header file for convert_color.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _CONVERT_COLOR_H_
#define _CONVERT_COLOR_H_
#endif 

void Convert_RGB_to_XYZ(double* rgb, double* xyz);
void Convert_XYZ_to_sRGB(double* xyz,double* srgb,unsigned char linear_flg);
void Convert_RGB_to_sRGB_img(unsigned char*** inp_img,
unsigned int height, unsigned int width, unsigned char ***out_img);


