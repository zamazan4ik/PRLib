/********************************************************************************************************
    Filename: TIFF_RW.c    
    Version : 1.0                  
	Author  : Du-Yong Ng                            
    Date    : 7/18/04                               
    Email   : dng@purdue.edu                        
    Comment : 
              These subroutines were coded to read and write TIFF files using the TIFF 6.0 specifications.
			  This is a higher level I/O interface that wraps around the TIFF Library 6.0 lower level I/O 
			  from www.libtiff.org.
			  This version supports several compression (both encoding and decoding) methods:
			  OJPEG (read only), LZW , Macintosh RLE (Packbits), ZIP/Deflate, Adobe Deflate, 
			  CCITT Group 3 and 4, CCITT with modified Huffman RLE and standard JPEG.
			  The read and written bytes are filled in the native machine fillorder.
			  
	
	Revision: You are welcome to improve and update this version.          
 *******************************************************************************************************/

#ifndef _TIFF_RW
#define _TIFF_RW
#include "tiffio.h"

typedef int boolean;
#define	TRUE	1
#define	FALSE	0


#include <opencv2/core/core.hpp>

typedef struct {
	
  uint8		**mono;		/* monochrome data, or indices    */
					/* of color-map; indexed as       */
					/* mono[row][col]                 */
  uint16		*rmap, *gmap;   /* RGB color map                  */
  uint16		*bmap;

  uint8			***color;	/* full-color RGB data; indexed   */
					/* as color[plan][row][col],      */
					/* with planes being red,  green, */
					/* and blue, respectively         */	
                    /* Y component of YCbCr image   = color[0]  */
  					/* L component of La*b* image  = color[0]   */
  					/* chrominance[0][row][col]	  */ 
					/*	Cb of YCbCr image	= color[1]  */
					/*	a* of La*b* image	=color[1]  */
					/* chrominance[1][row][col]	  */ 
					/*	Cr of YCbCr image = color[2]	  */
					/*	b* of La*b* image = color[2]	  */
  /*									  */
  /*  			IMPORTANT NOTE !!!!!				  */
  /*      luminance is not scaled to its range.  			  */
  /*	  e.g. L of La*b* is usually stored as 0-255 with a unit 0.394531 */
  /*	  when using La*b*, one should scale L back to its range [0,100]  */
  


  /* optinal parameters,  default values are set 			  */
  uint32	width;				/* image width */
  uint32	height;				/* image height */
  uint16	bitspersample;			/* image bits/sample */
  uint16	samplesperpixel;		/* image samples/pixel */
  uint16	orientation;			/* image orientation */
  uint16	photometric;			/* image photometric interpretation */
  
  /* NOTE: The default compression scheme in TIFF_WRITE_FILE is set to NO_COMPRESSION. It the user prefers to use:
  other supported format, the following tags
  MUST be EXPLICITLY specified. To use other compression schemes not specified here, the TIFF_WRITE_FILE subroutine needs to be modified.
  The user is responsible for specifying the correct compression scheme and relevant tags.*/
  /* Usage:
  1) No compression:
   compress = COMPRESSION_NONE;
  2) packbits:
  	compress = COMPRESSION_PACKBITS;
  3) ZIP/Deflate OR Adobe_Deflate
  	compress = COMPRESSION_DEFLATE OR COMPRESSION_ADOBE_DEFLATE
  	zip_quality = Z_BEST_COMPRESSION;  Z_BEST_SPEED or Z_NO_COMPRESSION. The default quality is Z_DEFAULT_COMPRESSION, There is no need to specify this; 
  4) OJPEG: READ ONLY, this decoder works for some old JPEG formats, NOT all.
  	compress = COMPRESSION_OJPEG;
  5) JPEG:
    compress = COMPRESSION_JPEG;
    jpg_quality = 75;
    jpegcolormode =JPEGCOLORMODE_RGB ;JPEGCOLORMODE_RAW or JPEGCOLORMODE_RGB
    for more control over other JPEG options, the user needs to add these features in the read and write subroutines.
  6) LZW :
   compression = COMPRESSION_LZW;
   predictor = 1; 1 for both 1-bit, 8-bit and 24-bit samples or 2 only for 8-bit and 24-bit samples
  7) CCITT Group 3 and Group 4
  a) compress= COMPRESSION_CCITTFAX3 = COMPRESSION_CCITT_T4;  CCITT Group 3 fax encoding for binary encoding
     compress_option = GROUP3OPT_2DENCODING, GROUP3OPT_UNCOMPRESSED or GROUP3OPT_FILLBITS
  b) compress = COMPRESSION_CCITTFAX4 = COMPRESSION_CCITT_T6; CCITT Group 4 fax encoding for binary encoding
     compress_option = GROUP4OPT_UNCOMPRESSED
  c) CCITT with modified Huffman RLE
  	 compress = COMPRESSION_CCITTRLE 
  */
   
  uint16    compress;            /* TIFF compression flag or type. Look at tiff.h for these tags */
  uint32	compress_option;  /* to be used with CCITT compression schemes look at tiff.h for the group options, example: compress_option = GROUP3OPT_2DENCODING */
  int       jpg_quality;        /* JPEG quality: to be used with JPEG compression: example :  jpg_quality = 75 */
  int       jpegcolormode;      /* to be used with JPEG compression: example: jpegcolormode = JPEGCOLORMODE_RGB*/
  int       predictor;         /* to be used with LZW compression,  example: predictor = 1 or 2. 2 may not be supported.*/
  int       zip_quality;       /* to be used with Zip/Deflate/Adobe deflate */
  
  uint16	planarConfig ;
  uint8		IsTile;
  uint32	rowsperstrip ;
  uint32	tilewidth ;
  uint32	tildheight ;
  float		XResolution ;			/* number of pixels per ResolutionUnit  */						  
  float		YResolution ;			/* number of pixels per ResolutionUnit  */						
  uint16	ResolutionUnit ;		/* absolute unit of meas. Can be either RESUNIT_NONE, RESUNIT_INCH or RESUNIT_CENTIMETER */
   /*	    	FILLORDER_MSB2LSB  	1	/* most significant -> least */
  /*	    	FILLORDER_LSB2MSB  	2	/* least significant -> most */
  /* Important: TIFFReadEncodedStrip and TIFFWriteEncodedStrip automatically read and write bytes to the native machine fillorder */
  uint16	fillorder ;  			
 

 
  uint16	horizSubSampling ;		/* for YCbCr image only, */
  uint16	vertSubSampling ;		/* currently, must be 1, */
  						            /* unless JPEG compressed */
  char image_description[200];
  char software[200];
  char date[100];
} TIFF_img ;

double **allocate_2DArray_double(int width,int height);
int **allocate_2DArray_int(int width,int height);
uint8 **allocate_2DArray_uint8(int width,int height);
void free_2D(void **pt);
void free_TIFF_img ( TIFF_img *img );
TIFF_img *Read_TIFF_File (char *filename, boolean *error_flag);
int Write_TIFF_File(TIFF_img *write_img, char *outFilename);
TIFF_img *Create_New_img(uint32 width, uint32 height, uint16 samplesperpixel, uint16 bitspersample, uint16 photometric, boolean *error_flag);





#endif /* _TIFF_RW */
