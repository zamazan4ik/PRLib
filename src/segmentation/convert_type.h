/**
 * File name : convert_type.h
 *
 * File Description : This is a header file for convert_color.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _CONVERT_TYPE_H_
#define _CONVERT_TYPE_H_

void Convert_uchar_to_int(unsigned char*** inp_img,\
unsigned int height,unsigned int width, int*** out_img);
void Convert_int_to_uchar(int*** inp_img, unsigned int height,\
unsigned int width, unsigned char*** out_img);

#endif 


