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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "TIFF_RW.h"


/*********************** Check the bound of color map if it is an pallette image*******************************************************/

static int checkcmap(int n, uint16* r, uint16* g, uint16* b)
{
    while (n-- > 0)
    {
        if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
        { return (16); }
    }

    return (8);
}

/*************************** For color palette image ***************************************************/
static void CheckAndCorrectColormap(int n, uint16* r, uint16* g, uint16* b)
{
    int i;

    for (i = 0; i < n; i++)
    {
        if (r[i] >= 256 || g[i] >= 256 || b[i] >= 256)
        {
            fprintf(stdout, "\t16-bit colormap \n");
            fprintf(stdout, "%d r%d,  g%d,  b%d\n", i, r[i], g[i], b[i]);
            return;
        }
    }

    /* fprintf(stderr, "Warning: Scaling 8-bit colormap to 16-bit\n"); */
#define    CVT(x)        (((x) * ((1L<<16)-1)) / 255)
    for (i = 0; i < n; i++)
    {
        r[i] = CVT(r[i]);
        g[i] = CVT(g[i]);
        b[i] = CVT(b[i]);
    }
#undef CVT
}

/*********************** 2D memory allocation for double data type using double indirection*******************************************************/
double** allocate_2DArray_double(int width, int height)
{
    int i;
    double** pt2pt; /* This is a pointer to a double data type pointer */
    double* pt; /* This is a pointer to an array of double data type values */

    /* Allocate memory to store the pointer (address) of the beginning of each row */
    /* the size of this 1D array is equal to the height of the 2D array */
    if ((pt2pt = (double**) _TIFFmalloc(height * sizeof(double*))) == NULL)
    {
        return NULL;
    }

    /* Allocate memory to for the 2D array */
    if ((pt = (double*) _TIFFmalloc(width * height * sizeof(double))) == NULL)
    {
        free((void*) pt2pt);
        return NULL;
    }

    /* Store the pointer (address) of the beginning of each row of the 2D array to pt2pt */
    /* Note: the pointer to the 2D array is stored in the first element of pt2pt */
    /*       Every increment in pt points to the next double data */

    for (i = 0; i < height; i++)
    {
        pt2pt[i] = pt + i * width;
    }


    return (pt2pt);
}

/*********************** 2D memory allocation for integer data type using double indirection*******************************************************/
int** allocate_2DArray_int(int width, int height)
{
    int i;
    int** pt2pt; /* This is a pointer to an integer pointer */
    int* pt; /* This is a pointer to an array of integer values */
    /*size_t size = sizeof(char);*/
    /* Allocate memory to store the pointer (address) of the beginning of each row */
    /* the size of this 1D array is equal to the height of the 2D array */
    if ((pt2pt = (int**) _TIFFmalloc(height * sizeof(int*))) == NULL)
    {
        return NULL;
    }

    /* Allocate memory to for the 2D array */
    if ((pt = (int*) _TIFFmalloc(width * height * sizeof(int))) == NULL)
    {
        free((void*) pt2pt);
        return NULL;
    }

    /* Store the pointer (address) of the beginning of each row of the 2D array to pt2pt */
    /* Note: the pointer to the 2D array is stored in the first element of pt2pt */
    /*       Every increment in pt points to the next integer data */

    for (i = 0; i < height; i++)
    {
        pt2pt[i] = pt + i * width;
    }


    return (pt2pt);
}

/**************************** 2D memory allocation for 8-bit unsigned integer (char) data type using double indirection**************************************************/
uint8** allocate_2DArray_uint8(int width, int height)
{
    int i;
    uint8** pt2pt; /* This is a pointer to an 8-bit integer pointer */
    uint8* pt; /* This is a pointer to an array of 8-bit integer values */
    /*size_t size = sizeof(uint8);*/
    /* Allocate 1D array of memory to store the pointer (address) of the beginning of each row */
    /* the size of this 1D array is equal to the height of the 2D array */
    if ((pt2pt = (uint8**) malloc(height * sizeof(uint8*))) == NULL)
    {
        return NULL;
    }


    /* Allocate memory to for the 2D array */
    if ((pt = (uint8*) malloc(width * height * sizeof(uint8))) == NULL)
    {
        free((void*) pt2pt);
        return NULL;
    }

    /* Store the pointer (address) of the beginning of each row of the 2D array to pt2pt */
    /* Note: the pointer to the 2D array is stored in the first element of pt2pt */
    /*       Every increment in pt points to the next 8-bit unsigned integer data */

    for (i = 0; i < height; i++)
    {
        pt2pt[i] = pt + i * width;
    }


    return (pt2pt);
}

/************************** Free memory of a 2D array ****************************************************/
void free_2D(void** pt)
{
    /* Free the memory of the 2D array which is pointed to by the pointer in the first element of pt */
    if (pt[0] != NULL)
    {
        free((void*) pt[0]);
    }
    /* free the memory of the 1D array that stores the pointer to the beginning of the rows of the 2D array */
    if (pt != NULL)
    {
        free((void*) pt);
    }
}

/************************* Free all the 2D buffers and finally the TIFF_img structure *****************************************************/

void free_TIFF_img(TIFF_img* img)
{
    int i;

    if (img->mono != NULL)
    {
        free_2D((void**) img->mono);
    }

    if (img->rmap != NULL)
    {
        free((void*) img->rmap);
    }

    if (img->gmap != NULL)
    {
        free((void*) img->gmap);
    }

    if (img->bmap != NULL)
    {
        free((void*) img->bmap);
    }
    if (img->color != NULL)
    {
        for (i = 0; i < 3; i++)
        {

            if (img->color[i] != NULL)
            {
                free_2D((void**) img->color[i]);
            }
        }

        free((void*) img->color);
    }

    if (img != NULL)
    {
        free((void*) img);
    }

}

/**************************************** Create new image **************************************************************
   This subroutine only allocates memory for a new TIFF_img and performs necessary initialization.
   The error_flag needs to be initialized to TRUE before it is passed to this subroutine. 
   If no error occurs, error_flag = FALSE and a newly allocated and initialized will be returned
   If an error occurs, error_flag = TRUE and the partially allocated TIFF_img will be returned to the calling function. 
   This partially allocated MUST be deallocated with free_TIFF_img subroutine before exiting program to prevent memory leak 
 ************************************************************************************************************************/
TIFF_img* Create_New_img(uint32 width, uint32 height, uint16 samplesperpixel, uint16 bitspersample, uint16 photometric,
                         boolean* error_flag)
{
    TIFF_img* new_img = NULL;
    int i, k;
    struct tm time_value;
    char time_array[200];
    time_t value;
    new_img = (TIFF_img*) malloc(sizeof(TIFF_img));
    if (new_img == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for new image\n");
        *error_flag = TRUE;
        return new_img;
    }
    new_img->bitspersample = bitspersample;
    new_img->height = height;
    new_img->width = width;
    new_img->samplesperpixel = samplesperpixel;
    new_img->photometric = photometric;
    /*Default values*/
    /* Only this planarConfig is supported when using TIFFReadEncodedStrip and TIFFWriteEncodedStrip*/
    new_img->planarConfig = PLANARCONFIG_CONTIG;
    /*new_img->fillorder = FILLORDER_MSB2LSB; the fillorder is not initialized
    since it will be overwritten with the native machine fillorder in the Write_TIFF_File subroutine.*/
    new_img->XResolution = 0.0;
    new_img->YResolution = 0.0;
    new_img->ResolutionUnit = RESUNIT_NONE;
    /*new_img->rowsperstrip = (uint32) -1; is not initialized. It will be calculated in the Write_TIFF_File subroutine */
    new_img->compress = COMPRESSION_NONE; /* Initialized to no compression. To use compression, the user can overwrite this option */
    (void) sprintf(new_img->image_description, "%s", "Only the prescreening part is completed");
    (void) sprintf(new_img->software, "%s", "Digital Video Capture Image Analysis Tools: Version-Beta 1.0");
    time(&value);
    time_value = *localtime(&value);
    (void) strftime(new_img->date, sizeof(time_array), "%Y:%m:%d %H:%M:%S", &time_value);
    new_img->mono = NULL;
    new_img->color = NULL;
    new_img->rmap = NULL;
    new_img->gmap = NULL;
    new_img->bmap = NULL;
    if ((samplesperpixel == 1 && bitspersample == 1) || (samplesperpixel == 1 && bitspersample == 8))
    {
        switch (photometric)
        {
            case PHOTOMETRIC_PALETTE:

                new_img->mono = allocate_2DArray_uint8(width, height);
                if (new_img->mono == NULL)
                {
                    fprintf(stderr, "Failed to allocate memory for new image.\n");
                    *error_flag = TRUE;
                    return new_img;
                }
                k = (1 << bitspersample);    /* 2^bitspersample */
                new_img->rmap = (uint16*) _TIFFmalloc(k * sizeof(uint16));
                new_img->gmap = (uint16*) _TIFFmalloc(k * sizeof(uint16));
                new_img->bmap = (uint16*) _TIFFmalloc(k * sizeof(uint16));
                if ((new_img->rmap == NULL) || (new_img->gmap == NULL) || (new_img->bmap == NULL))
                {
                    fprintf(stderr, "Failed to allocate memory for color maps.\n");
                    *error_flag = TRUE;
                    return new_img;
                }
                /* Has to be initialied.  ow. may cause confusion between */
                /* 8-bit and 16-bit color map */
                for (i = 0; i < k; i++)
                {
                    new_img->rmap[i] = 0;
                    new_img->gmap[i] = 0;
                    new_img->bmap[i] = 0;
                }
                break;
            case PHOTOMETRIC_MINISWHITE:
            case PHOTOMETRIC_MINISBLACK:
            case PHOTOMETRIC_CIELAB:

                new_img->mono = allocate_2DArray_uint8(width, height);
                if (new_img->mono == NULL)
                {
                    fprintf(stderr, "Failed to allocate memory for new image.\n");
                    *error_flag = TRUE;
                    return new_img;
                }

                break;
            default :
                fprintf(stderr, " Invalid photometric value for an 1-bit or 8-bit image.\n");
                *error_flag = TRUE;
                return new_img;
        }
    }
    else if (samplesperpixel == 3 && bitspersample == 8)
    {

        switch (photometric)
        {
            case PHOTOMETRIC_RGB:
            case PHOTOMETRIC_YCBCR:
            case PHOTOMETRIC_CIELAB:

                new_img->color = (uint8***) malloc(samplesperpixel * sizeof(uint8**));
                if (new_img->color == NULL)
                {
                    fprintf(stderr, "Failed to allocate memory for new_img->color\n");
                    *error_flag = TRUE;
                    return new_img;
                }

                new_img->color[0] = NULL;
                new_img->color[1] = NULL;
                new_img->color[2] = NULL;

                for (i = 0; i < samplesperpixel; i++)
                {

                    new_img->color[i] = allocate_2DArray_uint8(width, height);
                    if (new_img->color[i] == NULL)
                    {
                        fprintf(stderr, "Failed to allocate memory for color planes\n");
                        *error_flag = TRUE;
                        return new_img;
                    }

                }

                break;
            default:
            {
                fprintf(stderr, "Invalid photometric value for a color image.\n");
                *error_flag = TRUE;
                return new_img;
            }
        }

    }
    else
    {
        fprintf(stderr, "Image with bitspersample = %d, samplesperpixel = %d\n is not supported.\n");
        *error_flag = TRUE;
        return new_img;
    }

    return new_img;
}

/********************************** Read TIFF File **********************************************************************
   This subroutine only allocates memory for a new TIFF_img, reads the input.tif into this new TIFF_img.
   The error_flag needs to be initialized to TRUE before it is passed to this subroutine. 
   If no error occurs, error_flag = TRUE and the input TIFF_img will be returned
   If an error occurs, error_flag = FALSE and the partially allocated input TIFF_img will be returned to the calling function. 
   This partially allocated MUST be deallocated with free_TIFF_img subroutine before exiting program to prevent memory leak 
 ************************************************************************************************************************/


TIFF_img* Read_TIFF_File(char* filename, boolean* error_flag)
{
    TIFF_img* read_img = NULL;
    uint32 diroff = 0;
    TIFF* in;
    int i, j, k, bytesRead, cmap, ii;
    uint8* buf_in = NULL;
    tdata_t read_buf = NULL;
    tstrip_t strip, end_strip;
    tsize_t stripsize_in;
    register uint8 temp_byte, mask;
    uint32 index_row = 0, index_col = 0;
    uint16* rmap, * gmap, * bmap;


    /* Open TIFF file */
    in = TIFFOpen(filename, "r");
    if (in == NULL)
    {
        fprintf(stderr, "Read_TIFF error: can not open %s.", filename);
        *error_flag = TRUE;
        return read_img;
    }

    /* diroff is the offset of a directory when there are */
    /* more than one image (such as Thumbnail) in the input tiff file         */
    if (diroff != 0 && !TIFFSetSubDirectory(in, diroff))
    {
        TIFFError(TIFFFileName(in),
                  "Error, setting subdirectory at %#x", diroff);
        fprintf(stderr, "Only support single image TIFF file.\n");
        (void) TIFFClose(in);
        *error_flag = TRUE;
        return NULL;
    }

    /* allocate memory for TIFF_img structure */
    if ((read_img = (TIFF_img*) malloc(sizeof(TIFF_img))) == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for TIFF_img structure\n");
        TIFFClose(in);
        *error_flag = TRUE;
        printf("returning to main\n");
        return read_img;
    }

    read_img->mono = NULL;
    read_img->color = NULL;
    read_img->rmap = NULL;
    read_img->gmap = NULL;
    read_img->bmap = NULL;

    /* Read the relevant Tags into the TIFF_img structure */
    TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &(read_img->bitspersample));
    if ((read_img->bitspersample != 8) && (read_img->bitspersample != 1))
    {
        fprintf(stderr, "Sorry, only handle 8-bit or 1-bit samples.\n");
        fprintf(stderr, "No %d-bit samples.\n", read_img->bitspersample);
        TIFFClose(in);
        *error_flag = TRUE;
        return read_img;
    }

    TIFFGetField(in, TIFFTAG_PLANARCONFIG, &(read_img->planarConfig));

    if (read_img->planarConfig == PLANARCONFIG_SEPARATE)
    {
        fprintf(stderr,
                "Sorry, only handle single image plane, not separate.\n");
        TIFFClose(in);
        *error_flag = TRUE;
        return read_img;

    }

    TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &(read_img->width));
    TIFFGetField(in, TIFFTAG_IMAGELENGTH, &(read_img->height));


    TIFFGetField(in, TIFFTAG_FILLORDER, &(read_img->fillorder));
    TIFFGetField(in, TIFFTAG_SAMPLESPERPIXEL, &(read_img->samplesperpixel));
    TIFFGetField(in, TIFFTAG_XRESOLUTION, &(read_img->XResolution));
    TIFFGetField(in, TIFFTAG_YRESOLUTION, &(read_img->YResolution));
    TIFFGetField(in, TIFFTAG_RESOLUTIONUNIT, &(read_img->ResolutionUnit));
    TIFFGetField(in, TIFFTAG_PHOTOMETRIC, &(read_img->photometric));
    TIFFGetField(in, TIFFTAG_ROWSPERSTRIP, &(read_img->rowsperstrip));
    TIFFGetField(in, TIFFTAG_COMPRESSION, &(read_img->compress));

    switch (read_img->compress)
    {
        case COMPRESSION_NONE:
        case COMPRESSION_PACKBITS:
        case COMPRESSION_OJPEG:
        case COMPRESSION_CCITTRLE:
            break;
        case COMPRESSION_DEFLATE:
        case COMPRESSION_ADOBE_DEFLATE:
            TIFFGetField(in, TIFFTAG_ZIPQUALITY, read_img->zip_quality);
            break;
        case COMPRESSION_JPEG:
            TIFFGetField(in, TIFFTAG_JPEGCOLORMODE, &(read_img->jpegcolormode));
            TIFFGetField(in, TIFFTAG_JPEGQUALITY, &(read_img->jpg_quality));
            break;
        case COMPRESSION_LZW:
            TIFFGetField(in, TIFFTAG_PREDICTOR, &(read_img->predictor));
        case COMPRESSION_CCITTFAX3:
            TIFFGetField(in, TIFFTAG_GROUP3OPTIONS, &(read_img->compress_option));
            break;
        case COMPRESSION_CCITTFAX4:
            TIFFGetField(in, TIFFTAG_GROUP4OPTIONS, &(read_img->compress_option));
            break;
        default:
            fprintf(stderr, "This compression is currently NOT configured or supported!\n");

    }
    /*allocating memory for 2D structure */
    switch (read_img->samplesperpixel)
    {
        case 1:
        {    /* samplesperpixel  =1, begin*/
            /*confirm the photometric interpretation information */
            switch (read_img->photometric)
            {
                case PHOTOMETRIC_MINISBLACK:
                case PHOTOMETRIC_MINISWHITE:
                    /* allocate 2D image buffer for read_img for both bilevel and grayscale*/
                    if ((read_img->mono = allocate_2DArray_uint8(read_img->width, read_img->height)) == NULL)
                    {
                        printf("Error allocating memory for color structure.\n");
                        if (read_buf != NULL)
                        {
                            _TIFFfree(read_buf);
                        }
                        TIFFClose(in);
                        *error_flag = TRUE;
                        return read_img;
                    }
                    break;
                case PHOTOMETRIC_PALETTE:
                    /* allocate 2D image buffer for read_img for both pallette image*/
                    if ((read_img->mono = allocate_2DArray_uint8(read_img->width, read_img->height)) == NULL)
                    {
                        printf("Error allocating memory for color structure.\n");
                        if (read_buf != NULL)
                        {
                            _TIFFfree(read_buf);
                        }
                        TIFFClose(in);
                        *error_flag = TRUE;
                        return read_img;
                    }
                    if (!TIFFGetField(in, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap))
                    {
                        fprintf(stderr, "No colormap (not a valid palette image).\n");
                        if (read_buf != NULL)
                        {
                            _TIFFfree(read_buf);
                        }
                        TIFFClose(in);
                        *error_flag = TRUE;
                        return read_img;
                    }
                    read_img->rmap = (uint16*) malloc((1 << (read_img->bitspersample)) * sizeof(uint16));
                    read_img->gmap = (uint16*) malloc((1 << (read_img->bitspersample)) * sizeof(uint16));
                    read_img->bmap = (uint16*) malloc((1 << (read_img->bitspersample)) * sizeof(uint16));
                    if (read_img->rmap == NULL || read_img->gmap == NULL || read_img->bmap == NULL)
                    {
                        fprintf(stderr, "failed to allocate memory for color map.\n");
                        if (read_buf != NULL)
                        {
                            _TIFFfree(read_buf);
                        }
                        TIFFClose(in);
                        *error_flag = TRUE;
                        return read_img;
                    }
                    cmap = checkcmap(1 << (read_img->bitspersample), rmap, gmap, bmap);

                    for (ii = (1 << (read_img->bitspersample)) - 1; ii >= 0; ii--)
                    {

                        if (cmap == 16)
                        {
                            /*
                             * Convert 16-bit colormap to 8-bit.
                             */
#define    CVT(x)        (((x) * 255) / ((1L<<16)-1))
                            read_img->rmap[ii] = CVT(rmap[ii]);
                            read_img->gmap[ii] = CVT(gmap[ii]);
                            read_img->bmap[ii] = CVT(bmap[ii]);
#undef CVT
                        }
                        else
                        {
                            read_img->rmap[ii] = rmap[ii];
                            read_img->gmap[ii] = gmap[ii];
                            read_img->bmap[ii] = bmap[ii];
                        }

                    }
                    break;
                default:
                    fprintf(stderr, "Invalid  photometric value for a monochrome or a bilevel image\n");
                    if (read_buf != NULL)
                    {
                        _TIFFfree(read_buf);
                    }
                    (void) TIFFClose(in);
                    *error_flag = TRUE;
                    return read_img;
            }
            break;
        }
        case 3:
        {
            /* confirm photometric interpretation information and extract relevant information if necessary */
            switch (read_img->photometric)
            {

                case PHOTOMETRIC_RGB: /*PHOTOMETRIC_RGB */
                    break;
                case PHOTOMETRIC_YCBCR:
                    /******************************************************************/
                    /* IMPORTANT:  */
                    /* (1) If the image is JPEG compressed, it will be upsample and  */
                    /*     change into RGB.  Unless user modifies the following code */
                    /*             if(compress ....) {
                    /*            .....
                    /*        }
                    /* (2) Currently, it can not read a YCbYr image if the subsampling
                    /*     rates are not 1
                    /******************************************************************/


                    TIFFGetField(in, TIFFTAG_YCBCRSUBSAMPLING,
                                 &(read_img->horizSubSampling), &(read_img->vertSubSampling));

                    if (read_img->compress != COMPRESSION_JPEG &&
                        (read_img->horizSubSampling != 1 && read_img->vertSubSampling != 1))
                    {
                        fprintf(stderr, "Does not support YCbCr image with subsmapling rates");
                        fprintf(stderr, " other than 1, unless it is JPEG compressed \n");
                        if (read_buf != NULL)
                        {
                            _TIFFfree(read_buf);
                        }
                        TIFFClose(in);
                        *error_flag = TRUE;
                        return read_img;
                    }
                    if (read_img->compress == COMPRESSION_JPEG && read_img->planarConfig == PLANARCONFIG_CONTIG)
                    {
                        /* can rely on libjpeg to convert to RGB */
                        /* XXX should restore current state on exit */
                        TIFFSetField(in, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
                    }

                    break;
                case PHOTOMETRIC_CIELAB:
                    break;
                default:
                {
                    fprintf(stderr, "The photometric %d is not supported.\n", read_img->photometric);
                    if (read_buf != NULL)
                    {
                        _TIFFfree(read_buf);
                    }
                    TIFFClose(in);
                    *error_flag = TRUE;
                    return read_img;
                }
            }
            /* allocate buffer, process and data to the buffer */
            if ((read_img->color = (uint8***) malloc(read_img->samplesperpixel * sizeof(uint8**))) == NULL)
            {
                fprintf(stderr, "Error allocating memory for color structure.\n");
                if (read_buf != NULL)
                {
                    _TIFFfree(read_buf);
                }
                TIFFClose(in);
                *error_flag = TRUE;
                return read_img;
            }
            read_img->color[0] = NULL;
            read_img->color[1] = NULL;
            read_img->color[2] = NULL;
            for (i = 0; i < 3; i++)
            {
                if ((read_img->color[i] = allocate_2DArray_uint8(read_img->width, read_img->height)) == NULL)
                {
                    fprintf(stderr, "Error allocating memory for color structure.\n");
                    if (read_buf != NULL)
                    {
                        _TIFFfree(read_buf);
                    }
                    TIFFClose(in);
                    *error_flag = TRUE;
                    return read_img;
                }
            }
        }
            break;
        default:
            fprintf(stderr, " bitspersample = %d is not supported for samplesperpixel = 1 image.\n",
                    read_img->bitspersample);
            if (read_buf != NULL)
            {
                _TIFFfree(read_buf);
            }
            TIFFClose(in);
            *error_flag = TRUE;
            return read_img;
    }




    /* read bytes from file */
    stripsize_in = TIFFStripSize(in);
    if ((read_buf = _TIFFmalloc(TIFFStripSize(in))) == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for reading buffer.\n");
        TIFFClose(in);
        *error_flag = TRUE;
        return read_img;
    }
    end_strip = TIFFNumberOfStrips(in);
    for (strip = 0; strip < end_strip; strip++)
    {
        /* read one strip at a time, process (decompress if necessary) and fill in the image buffer */
        bytesRead = TIFFReadEncodedStrip(in, strip, read_buf, (tsize_t) -1);
        if (bytesRead < 0)
        {
            fprintf(stderr, "Error: Can't read strip #%d\n", strip);
            if (read_buf != NULL)
            {
                _TIFFfree(read_buf);
            }
            TIFFClose(in);
            *error_flag = TRUE;
            return read_img;
        }
        buf_in = (uint8*) read_buf;
        /*Handle different image type seperately */
        switch (read_img->samplesperpixel)
        {
            case 1:
            {    /* samplesperpixel  =1, begin*/
                /* Process data */
                if (read_img->bitspersample == 1) /* bitspersample =1, begin*/
                {
                    if (!TIFFIsMSB2LSB(in))
                    {
                        for (i = 0; i < bytesRead; i++)
                        {

                            mask = 1;
                            temp_byte = (uint8) (*buf_in++);
                            mask = 1;
                            for (j = 0; j < 8; j++)
                            {
                                read_img->mono[index_row][index_col] =
                                        (temp_byte & mask) >> j;
                                mask <<= 1;
                                index_col++;
                                if (index_col >= read_img->width)
                                {
                                    index_col = 0;
                                    index_row++;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        for (i = 0; i < bytesRead; i++)
                        {
                            temp_byte = (uint8) (*buf_in++);
                            mask = 128;
                            for (j = 0; j < 8; j++)
                            {
                                read_img->mono[index_row][index_col] =
                                        (temp_byte & mask) >> (7 - j);
                                mask >>= 1;
                                index_col++;
                                if (index_col >= read_img->width)
                                {
                                    index_col = 0;
                                    index_row++;
                                    break;
                                }
                            }
                        }
                    }
                } /* bitspersample =1, end*/
                else if (read_img->bitspersample == 8) /* bitspersample =8, begin*/
                {
                    for (i = 0; i < bytesRead; i++)
                    {
                        read_img->mono[index_row][index_col] = (uint8) (*buf_in++);
                        index_col++;
                        if (index_col >= read_img->width)
                        {
                            index_col = 0;
                            index_row++;
                        }
                    }
                }


                break;
            }  /* samplesperpixel  =1, end*/
            case 3:    /* samplesperpixel =3, begin*/
            {

                for (i = 3; i <= bytesRead; i += 3)
                {
                    read_img->color[0][index_row][index_col] = (uint8) (*buf_in++);
                    read_img->color[1][index_row][index_col] = (uint8) (*buf_in++);
                    read_img->color[2][index_row][index_col] = (uint8) (*buf_in++);
                    index_col++;
                    if (index_col >= read_img->width)
                    {
                        index_col = 0;
                        index_row++;
                    }
                }

                break;
            } /* samplesperpixel =3, end*/

        }
    }
    if (read_buf != NULL)
    {
        _TIFFfree(read_buf);
    }
    TIFFClose(in);
    return read_img;
}

/********************************** Write TIFF File : return 0 if write is unsuccessful, 1 if successful********************************************/
/* This subroutine only write the image to an output file. It does NOT deallocate the TIFF_img. The TIFF_img needs to be
   deallocated using free_TIFF_img subroutine after calling this function if it's no longer needed.                       */

int Write_TIFF_File(TIFF_img* write_img, char* outFilename)
{
    TIFF* out = NULL;
    uint8* buf_out = NULL;
    tdata_t write_buf = NULL;
    register uint8 temp_byte, mask;
    tsize_t stripsize_out, bytes2write;
    tstrip_t strip, end_strip;
    uint32 row_out, index_row, index_col, i, j;
    int WRITE = TRUE;
    uint32 rowsperstrip = (uint32) -1;

    /* Open image file for writing */
    out = TIFFOpen(outFilename, "w");
    if (out == NULL)
    {
        fprintf(stderr, "%s: Cannot open file for output.\n", outFilename);
        return FALSE;
    }
    /* setting relevant tags for the output image */
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, write_img->width);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, write_img->height);
    /*TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);*/
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, write_img->samplesperpixel);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, write_img->bitspersample);
    /* Store the fillorder according to that of the native machine */
    if (TIFFIsMSB2LSB(out))
    {
        TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    }
    else
    {
        TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_LSB2MSB);
    }
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, write_img->planarConfig);
    TIFFSetField(out, TIFFTAG_XRESOLUTION, write_img->XResolution);
    TIFFSetField(out, TIFFTAG_YRESOLUTION, write_img->YResolution);
    TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, write_img->ResolutionUnit);
    TIFFSetField(out, TIFFTAG_IMAGEDESCRIPTION, write_img->image_description);
    TIFFSetField(out, TIFFTAG_SOFTWARE, write_img->software);
    TIFFSetField(out, TIFFTAG_DATETIME, write_img->date);
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, write_img->planarConfig);
    /* compute default strip size */
    rowsperstrip = TIFFDefaultStripSize(out, rowsperstrip);
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
    write_img->rowsperstrip = rowsperstrip;

    switch (write_img->compress)
    {
        case COMPRESSION_NONE:
        case COMPRESSION_PACKBITS:
        case COMPRESSION_CCITTRLE:
            break;
        case COMPRESSION_DEFLATE:
        case COMPRESSION_ADOBE_DEFLATE:
            break;
        case COMPRESSION_JPEG:
            TIFFSetField(out, TIFFTAG_COMPRESSION, write_img->compress);
            if (write_img->photometric == PHOTOMETRIC_RGB && write_img->jpegcolormode == JPEGCOLORMODE_RGB)
            {
                write_img->photometric = PHOTOMETRIC_YCBCR;
            }
            TIFFSetField(out, TIFFTAG_JPEGQUALITY, write_img->jpg_quality);
            TIFFSetField(out, TIFFTAG_JPEGCOLORMODE, write_img->jpegcolormode);
            break;
        case COMPRESSION_LZW:
            TIFFSetField(out, TIFFTAG_COMPRESSION, write_img->compress);
            if (write_img->predictor != 1)
            {
                TIFFSetField(out, TIFFTAG_PREDICTOR, write_img->predictor);
            }
            break;
        case COMPRESSION_CCITTFAX3:
            TIFFSetField(out, TIFFTAG_COMPRESSION, write_img->compress);
            TIFFSetField(out, TIFFTAG_GROUP3OPTIONS, write_img->compress_option);
            break;
        case COMPRESSION_CCITTFAX4:
            TIFFSetField(out, TIFFTAG_COMPRESSION, write_img->compress);
            TIFFSetField(out, TIFFTAG_GROUP4OPTIONS, write_img->compress_option);
            break;
        default:
            fprintf(stderr, "This compression currently is NOT supported. No compression is used now!\n");
            TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    }
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, write_img->photometric);

    /* prepare and write image data to file */
    stripsize_out = TIFFStripSize(out);

    write_buf = _TIFFmalloc(stripsize_out);

    if (write_buf != NULL)
    {
        end_strip = TIFFNumberOfStrips(out);
        row_out = 0;
        index_row = 0;
        index_col = 0;

        for (strip = 0; strip < end_strip; strip++)
        {
            /* calculate the number of bytes to write. Make sure the last strip which is usually truncated is taken care of properly */
            bytes2write = (row_out + write_img->rowsperstrip > write_img->height) ?
                          TIFFVStripSize(out, write_img->height - row_out) : stripsize_out;

            buf_out = (uint8*) write_buf;

            switch (write_img->samplesperpixel)
            {
                case 1:

                    if (write_img->bitspersample == 1)
                    {
                        /* for 1-bit image, need to make sure the byte order is the same as that
                        of the native machine before copying the data to the write buffer */
                        if (TIFFIsMSB2LSB(out))
                        {
                            for (i = 1; i <= bytes2write; i++)
                            {
                                temp_byte = 0;
                                mask = 128;
                                for (j = 0; j < 8; j++)
                                {
                                    if (write_img->mono[index_row][index_col] > 0)
                                    {
                                        temp_byte = temp_byte | mask;
                                    }
                                    mask >>= 1;
                                    index_col++;
                                    if (index_col >= write_img->width)
                                    {
                                        index_col = 0;
                                        index_row++;
                                        break;
                                    }
                                }
                                (*buf_out++) = temp_byte;
                            }
                        }
                        else
                        {
                            for (i = 1; i <= bytes2write; i++)
                            {
                                temp_byte = 0;
                                mask = 1;
                                for (j = 0; j < 8; j++)
                                {
                                    if (write_img->mono[index_row][index_col] > 0)
                                    {
                                        temp_byte = temp_byte | mask;
                                    }
                                    mask <<= 1;
                                    index_col++;
                                    if (index_col >= write_img->width)
                                    {
                                        index_col = 0;
                                        index_row++;
                                        break;
                                    }
                                }
                                (*buf_out++) = temp_byte;

                            }


                        }
                    }
                    else if (write_img->bitspersample == 8)
                    {
                        /* for 8-bit image, just copy the write buffer */
                        for (i = 1; i <= bytes2write; i++)
                        {
                            (*buf_out++) = write_img->mono[index_row][index_col];
                            index_col++;
                            if (index_col >= write_img->width)
                            {
                                index_col = 0;
                                index_row++;
                            }
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Only 1-bit or 8-bit are supported.\n");
                        WRITE = FALSE;

                    }
                    break;
                case 3:
                    /* for 24-bit image, just copy the data to the write buffer */
                    for (i = 3; i <= bytes2write; i += 3)
                    {
                        (*buf_out++) = write_img->color[0][index_row][index_col];
                        (*buf_out++) = write_img->color[1][index_row][index_col];
                        (*buf_out++) = write_img->color[2][index_row][index_col];
                        index_col++;
                        if (index_col >= write_img->width)
                        {
                            index_col = 0;
                            index_row++;
                        }
                    }
                    break;
                default:
                    /* Error handling */
                    fprintf(stderr, "Samplesperpixel = %d is not supported\n",
                            write_img->samplesperpixel);
                    WRITE = FALSE;
                    break;
            }

            if ((index_row > write_img->height) || (index_row == write_img->height && index_col > 0))
            {
                /* check the written image dimension */
                fprintf(stderr, "Output rows %d > image height %d",
                        index_row, write_img->height);
                WRITE = FALSE;
                break;
            }
            if (WRITE)
            {
                /* write (and compress if necessary) the data in the write buffer to the file */
                if (TIFFWriteEncodedStrip(out, strip, write_buf, bytes2write) < 0)
                {
                    fprintf(stderr, "Failed to write image to %s\n", outFilename);
                    break;
                }

            }
            else
            {
                break;
            }
            row_out += write_img->rowsperstrip;


        }
    }
    if (WRITE)
    {
        /* Make sure the directory is 'clean' */
        if (!TIFFWriteDirectory(out))
        {
            fprintf(stderr, "Error writing TIFF directory.\n");
            if (write_buf != NULL)
            { _TIFFfree(write_buf); }
            if (out != NULL)
            { (void) TIFFClose(out); }
            return FALSE;
        }
        else
        {
            if (write_buf != NULL)
            { _TIFFfree(write_buf); }
            if (out != NULL)
            { (void) TIFFClose(out); }
            return TRUE;
        }
    }
    else
    {
        fprintf(stderr, "Error writing image.\n");
        if (write_buf != NULL)
        { _TIFFfree(write_buf); }
        if (out != NULL)
        { (void) TIFFClose(out); }
        return FALSE;

    }

}
