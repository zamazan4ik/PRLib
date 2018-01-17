/**
 * File name : convert_type.c
 *
 * File Description : Convert precision of image 
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


/**
 * Function Name : Convert_uchar_to_int 
 *
 * Function Description :
 * Convert unsigned char to int 
 *
 * Input       : inp_img, input image 
 *             : height, image height 
 *             : width, image width
 * Output      : out_img, output image 
 * Version : 1.0
 */

void Convert_uchar_to_int 
(
    unsigned char*** inp_img,              
    unsigned int height,
    unsigned int width,
    int*** out_img 
)
{
    int i,j,k;

    for ( k = 0 ; k < 3 ; k++ )
    {
        for ( i = 0 ; i < height ; i++ )
        {
            for ( j = 0 ; j < width ; j++ )
            {
                out_img[k][i][j] = (int)(inp_img[k][i][j]);
            }
        }
    }
}

/**
 * Function Name : Convert_int_to_uchar 
 *
 * Function Description :
 * Convert int to unsigned char 
 *
 * Input       : inp_img, input image 
 *             : height, image height 
 *             : width, image width
 * Output      : out_img, output image 
 * Version : 1.0
 */

void Convert_int_to_uchar 
(
    int*** inp_img,              
    unsigned int height,
    unsigned int width,
    unsigned char*** out_img 
)
{
    int i,j,k;
    int tmp;

    for ( k = 0 ; k < 3 ; k++ )
    {
        for ( i = 0 ; i < height ; i++ )
        {
            for ( j = 0 ; j < width ; j++ )
            {
                tmp = inp_img[k][i][j];
                if ( tmp > 0 && tmp < 256 )
                {
                    out_img[k][i][j] = (unsigned char)tmp;
                }
                else if ( tmp < 0 )
                {
                    out_img[k][i][j] = 0;
                }
                else if ( tmp > 255 )
                {
                    out_img[k][i][j] = 255;
                }
            }
        }
    }
}
