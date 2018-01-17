/**
 * File name : Main_segment.c
 *
 * File Description : Cost Optimized Segmentation 
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date : 12/21/2008
 * Version : 1.20
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "TIFF_RW.h"

#include "getopt.h"
#include "allocate.h"
#include "Main_def.h"
#include "segmentation.h"

/* Required getopt variables */
char* optarg;
int optind;

/* Internal function declaration */
static void usage(void);


/*int main(int argc, char* argv[])
{
  FILE  *outfp;      *//* File pointer to output file *//*

  TIFF_img *input_img = NULL;  *//* File pointer to input TIFF file *//*
  int  height, width;
  char blockname[10], dpiname[10];
  char  infname[200];
  char  wname[10];
  char  outname[200] = "binary.tif"; *//* default *//*
  char  calib_filename[200], ditr_filename[200];
  char  color_flg;
  char  calib_flg = FLG_OFF;
  char  CCC_skip_flg = FLG_OFF;
  int  i, j;
  boolean error_flag=FALSE;
  unsigned char  **bin_msk;
  int  ret;
  int  iflg=0;
  div_t  d1;
  char  block_flg = FLG_OFF;
 
  *//* default parameter values *//*
  double text_cost = 0.0; *//* Weight toward text *//*
  unsigned int block=32;  *//* block size *//*
  unsigned int multi_lyr_itr; *//* multi-layer iteration *//*
  unsigned int dpi=300;   *//* resolution (dpi) *//*
  *//* # of vertical dynamic programming iteration *//*
  unsigned int dynamic_itr_num=20; 
  unsigned int tmp;

  Seg_parameter seg_para;
  char  argp;         *//* Command line switch bin *//*

  *//* parse command line options *//*
  while ( optind < argc ){
    if ((signed char)(argp = getopt(argc,argv,"i:o:s:c:n:d:w:S")) == -1)
    {
      fprintf(stderr,"Parsing arguments error\n");
      usage();
      exit(FLG_NG) ;
    }
    switch ( argp ){
      case 'i':  strcpy(infname,optarg);
                 iflg++;
                 break;
      case 'o':  strcpy(outname,optarg);
                 break;
      case 'c':  strcpy(calib_filename,optarg);
                 calib_flg = FLG_ON;
                 break;
      case 'n':  strcpy(ditr_filename, optarg);
                 dynamic_itr_num = atoi( ditr_filename );
                 break;
      case 's':  strcpy(blockname,optarg);
                 block = atoi( blockname );
                 block_flg = FLG_ON;
                 break;
      case 'd':  strcpy(dpiname,optarg);
                 dpi = atoi( dpiname );
                 iflg++;
                 break;
      case 'S':  CCC_skip_flg = FLG_ON;
                 break;
      case 'w':  strcpy(wname,optarg);
                 sscanf(wname,"%lf",&text_cost);
                 break;
    }
  }
  *//* parameter number check *//*
  if ( iflg != 2 ) {
    fprintf(stderr,"input parameters are missing\n");
    usage();
    exit(FLG_NG) ;
  }

  *//* Read lambda parameters and multiscale layers *//*
  //read_lambda("../src/para_files/lambda_para.txt", &seg_para);
  seg_para.multi_lyr_itr = 1;
  seg_para.lambda = (double **)alloc_img(1, 4, sizeof(double));
  //30.680974 21.939354 36.658849 56.000098
  seg_para.lambda[0][0] = 30.680974;
  seg_para.lambda[0][1] = 21.939354;
  seg_para.lambda[0][2] = 36.658849;
  seg_para.lambda[0][3] = 56.000098;
  multi_lyr_itr = seg_para.multi_lyr_itr;

  if ( dpi <= 0 || block <= 0 || dynamic_itr_num <= 0 || multi_lyr_itr <= 0 ) {
    fprintf(stderr,"parameters are out of range.\n");
    usage();
    exit(FLG_NG) ;
  }

  *//* Set block size *//*
  if ( block_flg == FLG_OFF ) {
    block = (unsigned int)(block*(unsigned int)(floor((double)dpi/300)));
  }
  *//* Check block size value *//*
  d1 = div(block, 4);
  if ( d1.rem != 0 ) {
    fprintf(stderr,"block size needs to be 4x number.\n");
    usage();
    exit(FLG_NG) ;
  }

  *//* Read a input TIFF image info  *//*
  input_img = (TIFF_img *)Read_TIFF_File(infname, &error_flag);
  height = input_img->height;
  width = input_img->width;

#ifdef DEBUG_SEG
  fprintf(stderr, "height = %d, width= %d\n",height, width) ;
#endif

  *//* Check Color or Grayscale *//*
  if ( input_img->samplesperpixel == 1 && input_img->bitspersample == 8 ) {
    color_flg = FLG_GRAYSCALE;
    printf("Sorry, this software does not accept grayscale images.\n");
    free_TIFF_img(input_img);
    exit(FLG_NG);
  }
  else if ( input_img->samplesperpixel == 3 && input_img->bitspersample == 8 ) {
    color_flg = FLG_COLOR;
  }
  else {
    fprintf(stderr, "Sorry, this software does not accept your image color mode.\n") ;
    fprintf(stderr, "Please use 8bit full RGB mode: samplesperpixel = 3 and bitspersample = 8\n") ;
    free_TIFF_img(input_img);
    return (FLG_NG);
  }

  *//* Check multi-layer iteration value *//*
  tmp = block; 
  seg_para.min_block = block;
  for ( i = 1 ; i <= multi_lyr_itr ; i++ ) {
    if ( (tmp < height/5) && (tmp < width/5) ) {
      tmp = tmp*MULTI_LAYER_RATE;
    }
    else {
      multi_lyr_itr = i-1;
      break;
    }
  }
  seg_para.max_block = tmp/MULTI_LAYER_RATE;
  #ifdef DEBUG_SEG
  fprintf(stderr, "Multi-layer iteration is set to %d\n",multi_lyr_itr) ;
  #endif
  if ( multi_lyr_itr == 0 ) {
    fprintf(stderr,"block size is too large.\n");
    free_TIFF_img(input_img);
    exit(FLG_NG) ;
  }


  *//* Set parameter already known *//*
  seg_para.height = height;
  seg_para.width  = width;
  seg_para.calib_flg = calib_flg;
  seg_para.calib_filename = calib_filename;
  seg_para.dynamic_itr_num = dynamic_itr_num;
  seg_para.CCC_skip_flg = CCC_skip_flg;
  seg_para.text_cost = text_cost;

  *//* Memory allocation *//*
  bin_msk = (unsigned char **)alloc_img(height, width, sizeof(unsigned char));

  *//* Segmentation *//*
  #ifdef DEBUG_PROGRESS
    printf("===== Start of segmentation\n");
  #endif
  ret = segmentation(input_img, bin_msk, &seg_para);

  if ( ret == FLG_NG ) { 
    free_TIFF_img(input_img);
    multifree(bin_msk, 2);
    return (FLG_NG);
  }  

  *//* Output segmentation image *//*
  output_tiff_UINT8(outname, (unsigned char *)bin_msk, FLG_BIN, height, width);

  *//* Free memories *//*
  multifree(seg_para.lambda,2);
  free_TIFF_img(input_img);
  multifree(bin_msk, 2);

  #ifdef DEBUG_PROGRESS
    printf("===== End of segmentation\n");
  #endif

}*/

/**
 * Function Name : usage
 *
 * Function Description :
 *
 * Version : 1.0
 */

static void usage(void)
{
    fprintf(stderr, "Usage: Main_segment -i input_file -d 300\n");
    fprintf(stderr, "  -i  <fname>  :input file name (TIFF format)\n");
    fprintf(stderr, "  -d  <num>    :input image resolution (in dpi)\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o  <fname>  :default = \"binary.tif\"\n");
    fprintf(stderr, "                segmentation output file name (TIFF format)\n");
    fprintf(stderr, "  -c  <fname>  :calibration file name (text)\n");
    fprintf(stderr, "  -s  <num>    :default = 32, block size (x4 number)\n");
    fprintf(stderr, "  -n  <num>    :default = 20\n");
    fprintf(stderr, "                # of vertical dynamic programming iteration\n");
    fprintf(stderr, "  -S           :perform only COS segmentation\n");
    fprintf(stderr, "  -w  <num>    :default = 0.0\n");
    fprintf(stderr, "               :weight for text segmentation\n");
}

/**
 * Function Name : output_tiff_UINT8
 *
 * Function Description :
 * Output a TIFF file (color, grayscale, or binary)
 *
 * Input       : filename,  output filename
                 img,       image data
                 color_flg, FLG_COLOR, FLG_GRAYSCALE, or FLG_BIN
                 height, width, 
 * Output      : TIFF file 
 * Version : 1.0
 */

void output_tiff_UINT8(
        char* filename,      /* filename */
        unsigned char* img,        /* image */
        char color_flg,      /* color flag */
        unsigned int height,   /* image height */
        unsigned int width,     /* image width */
        cv::Mat& image
)
{
    /* DEBUG function to create a TIFF file  */

    TIFF_img* output_img;
    unsigned char** img_g;
    unsigned char*** img_c;
    boolean error_flag = FALSE;
    int i, j, k;
    unsigned short samplesperpixel;
    unsigned short bitspersample;
    unsigned short photometric;

    /* allocate the output image */
    if (color_flg == FLG_GRAYSCALE)
    {
        samplesperpixel = 1;
        bitspersample = 8;
/*  photometric = PHOTOMETRIC_MINISWHITE;*/
        photometric = PHOTOMETRIC_MINISBLACK;
    }
    else if (color_flg == FLG_BIN)
    {
        samplesperpixel = 1;
        bitspersample = 1;
        photometric = PHOTOMETRIC_MINISWHITE;
/*  photometric = PHOTOMETRIC_MINISBLACK;*/
    }
    else if (color_flg == FLG_COLOR)
    {
        samplesperpixel = 3;
        bitspersample = 8;
        photometric = PHOTOMETRIC_RGB;
    }
    output_img = Create_New_img(width, height, samplesperpixel, bitspersample, photometric, &error_flag);

    /* Modify parameters */
    /* output_img->compress = COMPRESSION_NONE; */
    output_img->compress = COMPRESSION_PACKBITS;
    output_img->ResolutionUnit = RESUNIT_INCH;
    output_img->XResolution = (double) 72;
    output_img->YResolution = (double) 72;

    /* copy to output image */
    if (color_flg == FLG_GRAYSCALE || color_flg == FLG_BIN)
    {
        img_g = (unsigned char**) img;
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                output_img->mono[i][j] = img_g[i][j];
            }
        }
    }
    else if (color_flg == FLG_COLOR)
    {
        img_c = (unsigned char***) img;
        for (k = 0; k < 3; k++)
        {
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    output_img->color[k][i][j] = img_c[k][i][j];
                }
            }
        }
    }

    /* write the output image */
    if (!Write_TIFF_File(output_img, filename))
    {
        fprintf(stderr, "fail to write compressed image %s.\n", filename);
    }
    free_TIFF_img(output_img);
}


