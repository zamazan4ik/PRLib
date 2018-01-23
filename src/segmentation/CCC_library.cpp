/**
 * File name : CCC_library.c
 *
 * File Description : CCC segmentation library
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date : 1/1/2009
 * Version : 2.00
 *
 */

#include <array>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <exception>
#include <stdexcept>
#include <vector>

#include "TIFF_RW.h"
#include "Main_def.h"

#include "segmentation.h"
#include "marklist.h"
#include "boundary.h"
#include "allocate.h"
#include "classify.h"
#include "prior_calc.h"
#include "CCC_library_itnl.h"

#define boundary8_j(x) ((((x)==7)||((x)==0)||((x)==1))?1:((((x)==3)||((x)==4)||((x)==5))?-1 : 0))
#define boundary8_i(x) ((((x)==1)||((x)==2)||((x)==3))?1:((((x)==5)||((x)==6)||((x)==7))?-1 : 0))


#define  color_diff(a1, a2, a3, b1, b2, b3) ((((a1)-(b1))*((a1)-(b1)))+(((a2)-(b2))*((a2)-(b2)))+(((a3)-(b3))*((a3)-(b3))))

#define  ITERATION   100

/**
 * Function Name : CCC_segment
 *
 * Function Description :
 * Connected Component Classification (CCC)
 *
 * Input       : input_img, input color image
 * Input/Output: seg_para, parameter info & input/output binary image
 * Version : 1.0
 */

void CCC_segment(
        unsigned char ***input_img, /* i : input image */
        Seg_parameter *seg_para     /* io : segmentation parameters */
)
{
    marktype im;
    marklistptr list = NULL;
    marklistptr n;
    unsigned int cnt, comp_num, comp_cnt;
    short startx, starty;
    double **vector, **feataug;
    int ret;
    double text_cost, non_text_cost;
    double **ll;
    Nei_header *neighbors;
    Pixel_pos hw;
    Dist_para dis_para;
    std::array<double, 2> map;
    char stop_flg;
    int org_clus;
    unsigned int height, width;
    unsigned char **bin_msk;

    height = seg_para->height;
    width = seg_para->width;
    bin_msk = seg_para->binmsk;


    /******************************************************************/
    /*             Flip reversed cc                                   */
    /******************************************************************/
    flip_reversed_cc(input_img, bin_msk, height, width);

    /******************************************************************/
    /*             Connected component extraction                     */
    /******************************************************************/
    /* Bitmap allocation (pbm) for connected component extraction */
    marktype_alloc(&im, width, height);

    /* Conversion from 8bpp to 1bpp (pbm) */
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            pbm_putpixel(im.bitmap, j, i, bin_msk[i][j]);
        }
    }

    /* Extract connected-component (nested) */
    im.imagew = width;
    im.imageh = height;
    list = extract_all_marks(list, im, 1, 4);

    /******************************************************************/
    /*             Remove small connected components (optional)       */
    /******************************************************************/
    n = list;
    while (n != nullptr)
    {
        cnt = 0;
        startx = n->data.xpos; /* column */
        starty = n->data.ypos; /* row */
        for (int i = 0; i < n->data.h; i++)
        {
            for (int j = 0; j < n->data.w; j++)
            {
                if (pbm_getpixel(n->data.bitmap, j, i) == 1)
                {
                    cnt++;
                }
            }
        }
        if (cnt < 6)
        {
            for (int i = 0; i < n->data.h; i++)
            {
                for (int j = 0; j < n->data.w; j++)
                {
                    if (pbm_getpixel(n->data.bitmap, j, i) == 1)
                    {
                        bin_msk[starty + i][startx + j] = 0;
                    }
                }
            }
        }
        n = n->next;
    }
    marktype_free(&im);

    /******************************************************************/
    /*             Connected component extraction again               */
    /******************************************************************/
    list = nullptr;
    /* Bitmap allocation (pbm) for connected component extraction */
    marktype_alloc(&im, width, height);

    /* Conversion from 8bpp to 1bpp (pbm) */
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            pbm_putpixel(im.bitmap, j, i, bin_msk[i][j]);
        }
    }

    /* Extract connected-component (nested) */
    im.imagew = width;
    im.imageh = height;
    list = extract_all_marks(list, im, 1, 4);

    /* Count # of components */
    n = list;
    comp_num = 0;
    while (n != nullptr)
    {
        comp_num++;
        n = n->next;
    }

    /******************************************************************/
    /*             Feature vector calculation                         */
    /******************************************************************/
    /* Allocate memories for feature vector */
    vector = (double **) alloc_img(comp_num, FEAT_DIM, sizeof(double));
    make_feat(list, comp_num, height, width, input_img, bin_msk, vector);


    /******************************************************************/
    /*             MAP optimization for segmentation                  */
    /******************************************************************/
    /****** Calculate data term ******/
    ll = (double **) alloc_img(comp_num, 2, sizeof(double));
    std::vector<int> clus(comp_num);

    if (seg_para->cur_lyr_itr == seg_para->multi_lyr_itr - 1)
    {
        if (seg_para->text_cost >= 0.0)
        {
            text_cost = seg_para->text_cost;
        }
        else
        {
            non_text_cost = -(seg_para->text_cost);
        }
    }
    else
    {
        text_cost = 0.0;
        non_text_cost = 0.0;
    }
    ret = classify(vector, comp_num, ll, clus, FEAT_DIM, "para_files/CCC_GM_para", text_cost, non_text_cost);

    if (ret != 0)
    {
        throw std::logic_error("Classification failed.");
    }

    /****** Calculate prior term ******/
    /* Initialization of neighborhood que */
    init_neighbors(&neighbors, comp_num);

    /* Find neighbors */
    hw.i = height;
    hw.j = width;
    find_neighbors(list, &hw, neighbors);

    /* Calculate feature vector with augments */
    feataug = (double **) alloc_img(comp_num, FEATAUG_DIM, sizeof(double));
    make_feataug(comp_num, vector, neighbors, feataug);
    multifree(vector, 2);

    /* Calculate feature vector distance */
    calc_featdis(comp_num, feataug, neighbors);
    multifree(feataug, 2);


    /* MAP optimization */
    dis_para.p_val = EXP_P;
    dis_para.a_val = A_VAL;
    dis_para.b_val = B_VAL;

    for (int k = 0; k < ITERATION; k++)
    {
        stop_flg = FLG_ON;
        for (int i = 0; i < comp_num; i++)
        {
            org_clus = clus[i];
            for (int j = 0; j < 2; j++)
            {
                map[j] = -ll[i][j];
                /* calculate prior term */
                map[j] += (calc_prior(i, (int) j, neighbors, clus, &dis_para));
            }
            if (map[0] < map[1])
            {
                clus[i] = 0;
            }
            else
            {
                clus[i] = 1;
            }

            if (stop_flg == FLG_ON)
            {
                if (org_clus != clus[i])
                {
                    stop_flg = FLG_OFF;
                }
            }
        }
        if (stop_flg == FLG_ON)
        {
            break;
        }
    }

    /******************************************************************/
    /*             Remove non-text components from binary mask        */
    /******************************************************************/
    comp_cnt = 0;
    n = list;
    while (n != nullptr)
    {
        startx = n->data.xpos;
        starty = n->data.ypos;
        for (int i = 0; i < n->data.h; i++)
        {
            for (int j = 0; j < n->data.w; j++)
            {
                if (pbm_getpixel(n->data.bitmap, j, i) == 1)
                {
                    if (clus[comp_cnt] == 1)
                    {
                        bin_msk[starty + i][startx + j] = 1;
                    }
                    else
                    {
                        bin_msk[starty + i][startx + j] = 0;
                    }
                }
            }
        }
        n = n->next;
        comp_cnt++;
    }

    marktype_free(&im);
    multifree(ll, 2);
    free_neighbors(neighbors, comp_num);
}

void cnt_boundary_length(
        marklistptr list,       /* i : conncected component list */
        unsigned int height,    /* i : image height */
        unsigned int width,     /* i : image width */
        std::vector<int>& bound_list /* o : boundary length for each component */
)
{
/************************************************************
 *     count boundary length of each connected component   
 ************************************************************/

    marklistptr n;
    int comp_cnt, bound_cnt;

    n = list;
    comp_cnt = 0;

    while (n != NULL)
    {
        calc_boundary_length_cc(n, height, width, &bound_cnt);
        bound_list[comp_cnt] = bound_cnt;
        comp_cnt++;
        n = n->next;
    }
}

void alloc_edge_memory(
        std::vector<int>& bound_list,                 /* i : boundary list */
        unsigned int comp_num,             /* i : # of components */
        unsigned char ****edge_ptr        /* o : pointer to edge list */
)
{
    unsigned char ***edge;
    unsigned int i, k;

    edge = (unsigned char ***) alloc_array(3, sizeof(unsigned char **));
    for (k = 0; k < 3; k++)
    {
        edge[k] = (unsigned char **) alloc_array(comp_num, sizeof(unsigned char *));
        for (i = 0; i < comp_num; i++)
        {
            edge[k][i] = (unsigned char *) alloc_array(bound_list[i],
                                                       sizeof(unsigned char));
        }
    }
    *edge_ptr = edge;
}

void free_edge_memory(
        std::vector<int>& bound_list,                 /* i : boundary list */
        unsigned int comp_num,             /* i : # of components */
        unsigned char ***edge             /* i : pointer to edge list */
)
{
    unsigned char *edge_comp;

    for (int k = 0; k < 3; k++)
    {
        for (int i = 0; i < comp_num; i++)
        {
            edge_comp = &edge[k][i][0];
            multifree(edge_comp, 1);
        }
        multifree(edge[k], 1);
    }
    multifree(edge, 1);
}

void calc_boundary
        (
                marklistptr list,             /* i : conncected component list */
                std::vector<int>& bound_list,      /* i : boundary list */
                unsigned char ***inner_edge, /* o : inner edge */
                unsigned char ***outer_edge, /* o : outer edge */
                int height,                  /* i : image height */
                int width,                   /* i : image width */
                unsigned char ***input_img_c, /* i : input color image */
                unsigned char **bin_msk       /* i : binary mask */
        )
{
    marklistptr n;
    int comp_cnt, bound_cnt;
    int i, j, k, x, y;
    char **bin;
    int starty, startx, inner_i, inner_j, outer_i, outer_j;
    int axis_x, axis_y, axis_i1, axis_i2, axis_j1, axis_j2;
    char max_flg;
    double inner, outer;
    int window_in = 0;
    int window_out = 3;
    unsigned int cnt;

    n = list;
    comp_cnt = 0;

    while (n != NULL)
    {
/*
printf("======= comp # is %d ======\n",comp_cnt);
*/
        bin = (char **) alloc_img(n->data.h + 2, n->data.w + 2, sizeof(char));
        bound_cnt = 0;

        /* Make padding */
        for (i = 0; i < n->data.h; i++)
        {
            for (j = 0; j < n->data.w; j++)
            {
                bin[i + 1][j + 1] = pbm_getpixel(n->data.bitmap, j, i);
            }
        }

        /* Extract edges */
        startx = n->data.xpos; /* column */
        starty = n->data.ypos; /* row */
        for (i = -1; i < n->data.h; i++)
        {
            for (j = -1; j < n->data.w; j++)
            {
                if (bin[i + 1][j + 1] != bin[i + 2][j + 1])
                {
                    if (bin[i + 1][j + 1] == 0)
                    {
                        inner_i = i + 1;
                        inner_j = j;
                        outer_i = i;
                        outer_j = j;
                    }
                    else
                    {
                        inner_i = i;
                        inner_j = j;
                        outer_i = i + 1;
                        outer_j = j;
                    }

                    axis_i1 = starty + inner_i;
                    axis_j1 = startx + inner_j;
                    axis_i2 = starty + outer_i;
                    axis_j2 = startx + outer_j;
                    if (axis_i1 < 0 || axis_j1 < 0 || axis_i1 >= height
                        || axis_j1 >= width ||
                        axis_i2 < 0 || axis_j2 < 0 || axis_i2 >= height
                        || axis_j2 >= width)
                    {
                        continue;
                    }
/*
          printf("inner and outer location: (%d %d) (%d %d)\n",
                 axis_i1,axis_j1,axis_i2, axis_j2);
*/
                    for (k = 0; k < 3; k++)
                    {
                        /* collect inner info */
                        inner = 0;
                        cnt = 0;
                        for (x = -window_in; x <= window_in; x++)
                        {
                            for (y = -window_in; y <= window_in; y++)
                            {
                                axis_y = y + inner_j;
                                axis_x = x + inner_i;
                                if (axis_y < 0 || axis_x < 0 ||
                                    axis_y >= n->data.w || axis_x >= n->data.h)
                                {
                                    continue;
                                }
                                if (pbm_getpixel(n->data.bitmap, axis_y, axis_x) == 1)
                                {
                                    cnt++;
                                    inner += (double) input_img_c[k][starty + axis_x][startx + axis_y];
                                }
                            }
                        }
                        inner = inner / cnt;

                        /* collect outer info */
                        outer = 0;
                        cnt = 0;
                        for (x = -window_out; x <= window_out; x++)
                        {
                            for (y = -window_out; y <= window_out; y++)
                            {
                                axis_y = axis_j2 + y;
                                axis_x = axis_i2 + x;
                                if (axis_y < 0 || axis_x < 0 || axis_y >= width
                                    || axis_x >= height)
                                {
                                    continue;
                                }
                                if (bin_msk[axis_x][axis_y] == 0)
                                {
                                    cnt++;
                                    outer += (double) input_img_c[k][axis_x][axis_y];
                                }
                            }
                        }
                        outer = outer / cnt;

                        inner_edge[k][comp_cnt][bound_cnt] = inner;
                        outer_edge[k][comp_cnt][bound_cnt] = outer;
/*
          printf("inner = %f outer = %f\n",inner, outer);
*/
                    }
                    bound_cnt++;
                }
                if (bin[i + 1][j + 1] != bin[i + 1][j + 2])
                {
                    if (bin[i + 1][j + 1] == 0)
                    {
                        inner_i = i;
                        inner_j = j + 1;
                        outer_i = i;
                        outer_j = j;
                    }
                    else
                    {
                        inner_i = i;
                        inner_j = j;
                        outer_i = i;
                        outer_j = j + 1;
                    }

                    axis_i1 = starty + inner_i;
                    axis_j1 = startx + inner_j;
                    axis_i2 = starty + outer_i;
                    axis_j2 = startx + outer_j;
                    if (axis_i1 < 0 || axis_j1 < 0 || axis_i1 >= height
                        || axis_j1 >= width ||
                        axis_i2 < 0 || axis_j2 < 0 || axis_i2 >= height
                        || axis_j2 >= width)
                    {
                        continue;
                    }

                    for (k = 0; k < 3; k++)
                    {
                        /* collect inner info */
                        inner = 0;
                        cnt = 0;
                        for (x = -window_in; x <= window_in; x++)
                        {
                            for (y = -window_in; y <= window_in; y++)
                            {
                                axis_y = y + inner_j;
                                axis_x = x + inner_i;
                                if (axis_y < 0 || axis_x < 0 || axis_y >= n->data.w
                                    || axis_x >= n->data.h)
                                {
                                    continue;
                                }
                                if (pbm_getpixel(n->data.bitmap, axis_y, axis_x) == 1)
                                {
                                    cnt++;
                                    inner += (double) input_img_c[k][starty + axis_x][startx + axis_y];
                                }
                            }
                        }
                        inner = inner / cnt;

                        /* collect outer info */
                        outer = 0;
                        cnt = 0;
                        for (x = -window_out; x <= window_out; x++)
                        {
                            for (y = -window_out; y <= window_out; y++)
                            {
                                axis_y = axis_j2 + y;
                                axis_x = axis_i2 + x;
                                if (axis_y < 0 || axis_x < 0 || axis_y >= width
                                    || axis_x >= height)
                                {
                                    continue;
                                }
                                if (bin_msk[axis_x][axis_y] == 0)
                                {
                                    cnt++;
                                    outer += (double) input_img_c[k][axis_x][axis_y];
                                }
                            }
                        }
                        outer = outer / cnt;
                        inner_edge[k][comp_cnt][bound_cnt] = inner;
                        outer_edge[k][comp_cnt][bound_cnt] = outer;
                    }
                    bound_cnt++;
                }
            }
        }
        comp_cnt++;
        n = n->next;
    }
}

void calc_edge
        (
                std::vector<int>& bound_list,      /* i : boundary list */
                unsigned int comp_num,        /* i : # of cc */
                unsigned char ***inner_edge, /* i : inner edge */
                unsigned char ***outer_edge, /* i : outer edge */
                std::vector<double>& edge_depth,   /* o : edge depth */
                std::vector<double>& edge_std,     /* o : std of edge depth */
                std::vector<double>& edge_std2,    /* o : std of edge depth */
                std::vector<double>& edge_max,     /* o : edge depth */
                std::vector<double>& edge_min      /* o : edge depth */
        )
{
    double edge_total, edge_total2, back_total, back_total2, data, tmp, data2;
    double tmp2;

    for (int i = 0; i < comp_num; i++)
    {
        edge_total = 0.0;
        edge_total2 = 0.0;
        back_total = 0.0;
        back_total2 = 0.0;
        std::vector<unsigned int> data_array(bound_list[i]), sort_index(bound_list[i]);

        for (int j = 0; j < bound_list[i]; j++)
        {
            data = (double) color_diff(inner_edge[0][i][j], inner_edge[1][i][j], inner_edge[2][i][j],
                                       outer_edge[0][i][j], outer_edge[1][i][j], outer_edge[2][i][j]);
            data = std::sqrt(data);
            edge_total += data;
            edge_total2 += (data * data);

            data2 = (double) color_diff(0, 0, 0, outer_edge[0][i][j], outer_edge[1][i][j], outer_edge[2][i][j]);
            data2 = std::sqrt(data2);
            back_total += data2;
            back_total2 += (data2 * data2);
            data_array[j] = (unsigned int) data2;
        }
        edge_depth[i] = edge_total / bound_list[i];

        QuickSort(data_array, sort_index, 0, bound_list[i] - 1);
        edge_max[i] = (double) find_percentile(data_array, bound_list[i], 98);
        edge_min[i] = (double) find_percentile(data_array, bound_list[i], 2);

        tmp = edge_total / (double) bound_list[i];
        tmp2 = edge_total2 / (double) bound_list[i] - tmp * tmp;
        if (tmp2 <= 0.0)
        {
            tmp2 = 0.0;
        }
        edge_std[i] = std::sqrt(tmp2);
        tmp = back_total / (double) bound_list[i];
        tmp2 = back_total2 / (double) bound_list[i] - tmp * tmp;
        if (tmp2 <= 0.0)
        {
            tmp2 = 0.0;
        }
        edge_std2[i] = std::sqrt(tmp2);

    }
}


void calc_boundary_length_cc(
        marklistptr n,         /* i : a conncected component list */
        unsigned int height,
        unsigned int width,
        int *length     /* o : boundary length for the component */
)
{
/************************************************************
 *     count boundary length of a connected component   
 ************************************************************/

    int bound_cnt;
    char **bin;
    int i, j;
    int inner_i, inner_j, outer_i, outer_j;
    int axis_i1, axis_i2, axis_j1, axis_j2;
    int startx, starty;

    bin = (char **) alloc_img(n->data.h + 2, n->data.w + 2, sizeof(char));
    bound_cnt = 0;

    /* Make padding */
    for (i = 0; i < n->data.h; i++)
    {
        for (j = 0; j < n->data.w; j++)
        {
            bin[i + 1][j + 1] = pbm_getpixel(n->data.bitmap, j, i);
        }
    }

    /* Extract edges */
    startx = n->data.xpos; /* column */
    starty = n->data.ypos; /* row */
    for (i = -1; i < n->data.h; i++)
    {
        for (j = -1; j < n->data.w; j++)
        {
            if (bin[i + 1][j + 1] != bin[i + 2][j + 1])
            {
                if (bin[i + 1][j + 1] == 0)
                {
                    inner_i = i + 1;
                    inner_j = j;
                    outer_i = i;
                    outer_j = j;
                }
                else
                {
                    inner_i = i;
                    inner_j = j;
                    outer_i = i + 1;
                    outer_j = j;
                }

                axis_i1 = starty + inner_i;
                axis_j1 = startx + inner_j;
                axis_i2 = starty + outer_i;
                axis_j2 = startx + outer_j;
                if (axis_i1 < 0 || axis_j1 < 0 || axis_i1 >= height || axis_j1 >= width ||
                    axis_i2 < 0 || axis_j2 < 0 || axis_i2 >= height || axis_j2 >= width)
                {
                    continue;
                }
                bound_cnt++;
            }
            if (bin[i + 1][j + 1] != bin[i + 1][j + 2])
            {
                if (bin[i + 1][j + 1] == 0)
                {
                    inner_i = i;
                    inner_j = j + 1;
                    outer_i = i;
                    outer_j = j;
                }
                else
                {
                    inner_i = i;
                    inner_j = j;
                    outer_i = i;
                    outer_j = j + 1;
                }
                axis_i1 = starty + inner_i;
                axis_j1 = startx + inner_j;
                axis_i2 = starty + outer_i;
                axis_j2 = startx + outer_j;
                if (axis_i1 < 0 || axis_j1 < 0 || axis_i1 >= height || axis_j1 >= width ||
                    axis_i2 < 0 || axis_j2 < 0 || axis_i2 >= height || axis_j2 >= width)
                {
                    continue;
                }
                bound_cnt++;
            }
        }
    }
    *length = bound_cnt;
    multifree(bin, 2);
}

/**
 * Function Name : Removehead
 *
 * Function Description :
 * Get a node from chain
 *
 * Input/Output: pstart, starting address of chain
 *             : pend,   ending address of chain
 * Version : 1.0
 */

CC_clist Removehead
        (
                CC_clist **pstart,
                CC_clist **pend
        )
{
    CC_clist ret;

    if (*pstart)
    {/* If que is not empty */

        ret = **pstart;
        free(*pstart);
        *pstart = ret.pnext;

        if (!(*pstart))
        {/* If que has become empty */
            *pend = NULL;
        }
    }

    return ret;
}

/**
 * Function Name : Addtail
 *
 * Function Description :
 * Add a new node to chain
 *
 * Input/Output: newlist, new node
 *             : pstart, starting address of chain
 *             : pend,   ending address of chain
 * Version : 1.0
 */

void Addtail(
        CC_clist newlist,
        CC_clist **pstart,
        CC_clist **pend
)
{
    CC_clist *ptr;

    /* Memory allocation */
    ptr = (CC_clist *) malloc(sizeof(CC_clist));

    if (*pend)
    {/* If que is not empty */

        (*pend)->pnext = ptr;
    }
    else
    {
        *pstart = ptr;
    }
    *pend = ptr;

    **pend = newlist;
    (*pend)->pnext = NULL;

}

void calc_white_edge(
        marklistptr list,   /* i : conncected component list */
        std::vector<double>& white_cc_edge,  /* o : boundary length of cc embedded in black cc */
        std::vector<double>& black_cc_edge,  /* o : boundary length of black cc */
        std::vector<double>& area,           /* o : area of black cc */
        unsigned int height,   /* i : original image height */
        unsigned int width,    /* i : original image width */
        unsigned char ***input_img,   /* i : input image (color) */
        unsigned char **bin_msk,      /* i : binary mask */
        char flg_bound          /* i : FLG_BOUND of FLG_ORIGINAL */
)
{
/*********************************************************************
 *     Calculate # & pixels of white connected components embedded
 *     in black cc
 *********************************************************************/
    marklistptr n, n2;
    marktype im;
    int Startx, Starty, axis_i, axis_j;
    int i, j, x, y;
    marklistptr wlist;
    unsigned char **bin_flip;
    unsigned int comp_cnt;
    char break_flg;
    int length, total_edge_cnt;
    Corner_info corner_info;
    double tmp;

    n = list;
    comp_cnt = 0;
    while (n != nullptr)
    {
        /* Calculate surrounding area */
        area[comp_cnt] = (double) (n->data.w) * (n->data.h) / (double) (height * width);

        /* Flip binary image */
        bin_flip = (unsigned char **) alloc_img(n->data.h,
                                                n->data.w, sizeof(unsigned char));
        for (i = 0; i < n->data.h; i++)
        {
            for (j = 0; j < n->data.w; j++)
            {
                if (pbm_getpixel(n->data.bitmap, j, i) == 0)
                {
                    bin_flip[i][j] = 1;
                }
            }
        }

        /* Extract white connected components */

        marktype_alloc(&im, n->data.w, n->data.h);
        /* Conversion from 8bpp to 1bpp (pbm) */
        for (i = 0; i < n->data.h; i++)
        {
            for (j = 0; j < n->data.w; j++)
            {
                pbm_putpixel(im.bitmap, j, i, bin_flip[i][j]);
            }
        }

        /* Extract connected-component (nested) */
        im.imagew = n->data.w;
        im.imageh = n->data.h;
        wlist = nullptr;
        wlist = extract_all_marks(wlist, im, 1, 8);

        /* Count # of components */
        total_edge_cnt = 0;
        corner_info.first_flag = FLG_ON;
        n2 = wlist;
        while (n2 != nullptr)
        {

            break_flg = FLG_OFF;
            for (x = 0; x < n2->data.h; x++)
            {
                for (y = 0; y < n2->data.w; y++)
                {
                    if (pbm_getpixel(n2->data.bitmap, y, x) == 1)
                    {
                        Startx = n2->data.xpos; /* column */
                        Starty = n2->data.ypos; /* row */
                        axis_i = Starty + x;
                        axis_j = Startx + y;
                        if (axis_i == 0 || axis_j == 0 ||
                            axis_i == n->data.h - 1 || axis_j == n->data.w - 1)
                        {
                            break_flg = FLG_ON;
                            break;
                        }
                    }
                }
                if (break_flg == FLG_ON)
                {
                    break;
                }
            }
            if (break_flg == FLG_ON)
            {
                n2 = n2->next;
                continue;
            }

            calc_boundary_length_cc(n2, height, width, &length);
            total_edge_cnt += length;

            /* Update corner information */
            record_corner(n2, &corner_info);

            n2 = n2->next;
        }

        white_cc_edge[comp_cnt] = (double) total_edge_cnt;
        if (flg_bound == FLG_BOUND)
        {
            tmp = 2 * ((double) corner_info.lowerright.row - corner_info.upperleft.row + 1) +
                  +2 * ((double) corner_info.lowerright.col - corner_info.upperleft.col + 1);
            black_cc_edge[comp_cnt] = tmp;
        }
        else
        {
            black_cc_edge[comp_cnt] = (double) (n->data.blength);
        }
        marktype_free(&im);
        multifree(bin_flip, 2);
        comp_cnt++;
        n = n->next;
    }

}

void calc_white_cc(
        marklistptr list,       /* i : conncected component list */
        std::vector<double>& white_cc_pxl_rate,
        /* o : # of white pixels embedded in black cc */
        std::vector<unsigned int>& cnt_white, /* o : # of white cc */
        unsigned int height,    /* i : original image height */
        unsigned int width,     /* i : original image width */
        unsigned char ***input_img,   /* i : input image (color) */
        unsigned char **bin_msk       /* i : binary mask */
)
{
/*********************************************************************
 *     Calculate # of pixels of white connected components embedded
 *     in black cc
 *********************************************************************/
    marklistptr n, n2;
    marktype im;
    int Startx, Starty, axis_i, axis_j;
    int i, j, x, y;
    marklistptr wlist;
    unsigned char **bin_flip;
    unsigned int comp_cnt, b_pixel_cnt, w_pixel_cnt, tmp_pxl_cnt, w_cc_cnt;
    char break_flg;

    n = list;
    comp_cnt = 0;
    while (n != nullptr)
    {
        b_pixel_cnt = 0;

        /* Flip binary image */
        bin_flip = (unsigned char **) alloc_img(n->data.h,
                                                n->data.w, sizeof(unsigned char));
        for (i = 0; i < n->data.h; i++)
        {
            for (j = 0; j < n->data.w; j++)
            {
                if (pbm_getpixel(n->data.bitmap, j, i) == 0)
                {
                    bin_flip[i][j] = 1;
                }
                else
                {
                    b_pixel_cnt++;
                }
            }
        }


        /* Extract white connected components */

        marktype_alloc(&im, n->data.w, n->data.h);
        /* Conversion from 8bpp to 1bpp (pbm) */
        for (i = 0; i < n->data.h; i++)
        {
            for (j = 0; j < n->data.w; j++)
            {
                pbm_putpixel(im.bitmap, j, i, bin_flip[i][j]);
            }
        }

        /* Extract connected-component (nested) */
        im.imagew = n->data.w;
        im.imageh = n->data.h;
        wlist = nullptr;
        wlist = extract_all_marks(wlist, im, 1, 8);

        /* Count # of components */
        n2 = wlist;
        w_pixel_cnt = 0;
        w_cc_cnt = 0;
        while (n2 != nullptr)
        {
            Startx = n2->data.xpos; /* column */
            Starty = n2->data.ypos; /* row */
            break_flg = FLG_OFF;
            tmp_pxl_cnt = 0;
            for (x = 0; x < n2->data.h; x++)
            {
                for (y = 0; y < n2->data.w; y++)
                {
                    if (pbm_getpixel(n2->data.bitmap, y, x) == 1)
                    {
                        tmp_pxl_cnt++;
                        axis_i = Starty + x;
                        axis_j = Startx + y;
                        if (axis_i == 0 || axis_j == 0 ||
                            axis_i == n->data.h - 1 || axis_j == n->data.w - 1)
                        {
                            break_flg = FLG_ON;
                            break;
                        }
                    }
                }
                if (break_flg == FLG_ON)
                {
                    break;
                }
            }
            if (break_flg == FLG_OFF &&
                tmp_pxl_cnt < (b_pixel_cnt / RED_WHITECC_RATIO))
            {
                w_pixel_cnt += tmp_pxl_cnt;
                w_cc_cnt++;
            }
            n2 = n2->next;
        }

        white_cc_pxl_rate[comp_cnt] = (double) w_pixel_cnt / b_pixel_cnt;
        cnt_white[comp_cnt] = w_cc_cnt;
        marktype_free(&im);
        multifree(bin_flip, 2);
        comp_cnt++;
        n = n->next;
    }
}

void reverse_cc(
        unsigned char **input_bin,        /* i : cc binary mask */
        unsigned char **bin_img,          /* i : org binary mask */
        unsigned int height,                /* i : height */
        unsigned int width,                 /* i : width */
        unsigned char **flip_bin,         /* o : reversed binary mask */
        unsigned char **rem_bin           /* o : reversed binary mask */
)
{
/*********************************************************************
 *     Reverse the input binary mask.
 *********************************************************************/
    int i, j;
    CC_pixel seed;
    unsigned int b_pxl_cnt = 0;
    unsigned int cnt;
    unsigned char **tmp_bin;

    /* Count black pixel number */
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            if (input_bin[i][j] == 1)
            {
                b_pxl_cnt++;
            }
        }
    }

    /* Flip the original binary image */
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            if (bin_img[i][j] == 0)
            {
                flip_bin[i][j] = 1;
            }
            else
            {
                flip_bin[i][j] = 0;
            }
        }
    }

    /* Detect boundary pixels by region-growing */
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            if (i != 0 && i != height - 1 && j != 0 && j != width - 1)
            {
                continue;
            }
            /* Pick a seed */
            if (input_bin[i][j] == 1)
            {
                continue;
            }

            seed.m = i;
            seed.n = j;

            Region_growing(seed, input_bin, rem_bin, width, height);
        }
    }

    /* Detect large redundancy by region-growing */
    tmp_bin = (unsigned char **) alloc_img(height, width,
                                           sizeof(unsigned char));
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            tmp_bin[i][j] = rem_bin[i][j];
        }
    }

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            if (rem_bin[i][j] == 0)
            {
                /* Pick a seed */
                if (input_bin[i][j] == 0)
                {
                    seed.m = i;
                    seed.n = j;
                    Region_growing_cnt(seed, input_bin, tmp_bin, width, height, &cnt);
                    if (cnt > (b_pxl_cnt / RED_WHITECC_RATIO))
                    {
                        Region_growing(seed, input_bin, rem_bin, width, height);
                    }
                }
            }
        }
    }
    multifree(tmp_bin, 2);

}


void Region_growing
        (
                CC_pixel s,              /* i : seed pixels */
                unsigned char **bin_msk,        /* i : input binary mask */
                unsigned char **out_msk,        /* o : detected region map */
                int width,
                int height
        )
{
    CC_clist *pstart;
    CC_clist *pend;
    CC_clist data;
    CC_clist tmp[4];
    CC_pixel neigh[4];
    int M, i;

    /* Initialization */
    pstart = NULL;
    pend = NULL;

    data.pixels.m = s.m;
    data.pixels.n = s.n;
    data.pnext = NULL;

    /* Add a seed pixel to cc check list */
    Addtail(data, &pstart, &pend);

    out_msk[data.pixels.m][data.pixels.n] = 1;

    while (pstart)
    {
        /* Initialize # of neigboring pixels */
        M = 0;

        /* Get a pixel from waiting list */
        data = Removehead(&pstart, &pend);

        /* Find neighboring pixels */
        ConnectedNeighbors_UCHAR(data.pixels, bin_msk, width, height,
                                 &M, neigh);

        for (i = 0; i < M; i++)
        {
            tmp[i].pixels = neigh[i];
            tmp[i].pnext = NULL;

            /* If neighboring pixel is not inspected yet */
            if (out_msk[tmp[i].pixels.m][tmp[i].pixels.n] == 0)
            {
                /* Add pixel to cc check list */
                Addtail(tmp[i], &pstart, &pend);
                out_msk[tmp[i].pixels.m][tmp[i].pixels.n] = 1;
            }
        }
    }/* while */
}

void ConnectedNeighbors_UCHAR
        (
                CC_pixel s,
                unsigned char **bin_msk,
                int width,
                int height,
                int *M,
                CC_pixel c[4]
        )
{
    int count = 0;
    int s1, s2, i, pixel_i, pixel_j;
    CC_pixel neighbor[4];

    /* current pixel */
    s1 = s.m;
    s2 = s.n;

    if ((s1 < 0) || (s1 > height) || (s2 < 0) || (s2 > width))
    {
        fprintf(stderr, "Arguments error\n");
        fprintf(stderr, "Pixel s is out of range.\n");
    }

    neighbor[0].m = s1 - 1;
    neighbor[0].n = s2;
    neighbor[1].m = s1 + 1;
    neighbor[1].n = s2;
    neighbor[2].m = s1;
    neighbor[2].n = s2 - 1;
    neighbor[3].m = s1;
    neighbor[3].n = s2 + 1;

    for (i = 0; i < 4; i++)
    {
        pixel_i = neighbor[i].m;
        pixel_j = neighbor[i].n;
        if ((pixel_i >= 0) && (pixel_i <= height - 1)
            && (pixel_j >= 0) && (pixel_j <= width - 1))
        {
            if (bin_msk[pixel_i][pixel_j] == bin_msk[s1][s2])
            {
                c[count] = neighbor[i];
                count++;
            }
        }
    }

    *M = count;
}

void record_corner(
        marklistptr n,           /* i : connected component list */
        Corner_info *corner_info /* io: updated corner information */
)
{
    int lowerrightrow, lowerrightcol;

    if (corner_info->first_flag == FLG_ON)
    {
        corner_info->upperleft.row = n->data.ypos;
        corner_info->upperleft.col = n->data.xpos;
        corner_info->lowerright.row = n->data.ypos + n->data.h - 1;
        corner_info->lowerright.col = n->data.xpos + n->data.w - 1;
        corner_info->first_flag = FLG_OFF;
    }
    else
    {
        if (corner_info->upperleft.row > n->data.ypos)
        {
            corner_info->upperleft.row = n->data.ypos;
        }
        if (corner_info->upperleft.col > n->data.xpos)
        {
            corner_info->upperleft.col = n->data.xpos;
        }

        lowerrightrow = n->data.ypos + n->data.h - 1;
        lowerrightcol = n->data.xpos + n->data.w - 1;

        if (lowerrightrow > corner_info->lowerright.row)
        {
            corner_info->lowerright.row = lowerrightrow;
        }
        if (lowerrightcol > corner_info->lowerright.col)
        {
            corner_info->lowerright.col = lowerrightcol;
        }
    }

}


void flip_reversed_cc(
        unsigned char ***input_img,
        unsigned char **bin_msk,
        unsigned int height,
        unsigned int width
)
{
    unsigned char **bin_msk_r, **bin_flip, **bin_input, **bin_img, **rem_bin;
    marktype im;
    marklistptr list = NULL;
    marklistptr n;
    unsigned int i, j;
    double **vector;
    unsigned int comp_num, comp_cnt;
    short startx, starty;

    /******************************************************************/
    /*             Connected component extraction                     */
    /******************************************************************/
    /* Bitmap allocation (pbm) for connected component extraction */
    marktype_alloc(&im, width, height);

    /* Conversion from 8bpp to 1bpp (pbm) */
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            pbm_putpixel(im.bitmap, j, i, bin_msk[i][j]);
        }
    }

    /* Extract connected-component (nested) */
    im.imagew = width;
    im.imageh = height;
    list = extract_all_marks(list, im, 1, 4);

    /* Count # of components */
    n = list;
    comp_num = 0;
    while (n != nullptr)
    {
        comp_num++;
        n = n->next;
    }

    /******************************************************************/
    /*       Classify text components into regular and flipped        */
    /******************************************************************/
    /******************************************************************/
    /*             Feature vector calculation                         */
    /******************************************************************/
    /* Allocate memories for feature vector */
    vector = (double **) alloc_img(comp_num, 4, sizeof(double));

    std::vector<double> white_cc_edge_list(comp_num), black_cc_edge_list(comp_num), area_list(comp_num);
    calc_white_edge(list, white_cc_edge_list, black_cc_edge_list, area_list,
                    height, width, input_img, bin_msk, FLG_BOUND);

    for (i = 0; i < comp_num; i++)
    {
        vector[i][0] = white_cc_edge_list[i] / black_cc_edge_list[i];
        vector[i][1] = area_list[i];
    }

    /* Extract white connected component embedded in black connected component */
    std::vector<double> white_cc_pxl_list(comp_num);
    std::vector<unsigned int> cnt_white(comp_num);
    calc_white_cc(list, white_cc_pxl_list, cnt_white,
                  height, width, input_img, bin_msk);

    for (i = 0; i < comp_num; i++)
    {
        vector[i][2] = white_cc_pxl_list[i];
        vector[i][3] = (double) cnt_white[i];
    }

    /******************************************************************/
    /*             Feature vector classification                      */
    /******************************************************************/
    std::vector<int> clus(comp_num);

    for (i = 0; i < comp_num; i++)
    {
/*
    if ( (  vector[i][0] > 1.0 && vector[i][1] < 0.1 && vector[i][2] < 0.5 
         &&  vector[i][3] > 5 ) )
*/
        if ((vector[i][2] < 0.5
             && vector[i][3] > 8))
        {
            clus[i] = 0;
        }
        else
        {
            clus[i] = 1;
        }
    }
    multifree(vector, 2);


    /******************************************************************/
    /*             Reverese flipped text                              */
    /******************************************************************/
    bin_msk_r = (unsigned char **) alloc_img(height, width, sizeof(char));
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            bin_msk_r[i][j] = 0;
        }
    }

    comp_cnt = 0;
    n = list;
    while (n != nullptr)
    {
        startx = n->data.xpos;
        starty = n->data.ypos;
        if (clus[comp_cnt] == 1)
        {
            for (i = 0; i < n->data.h; i++)
            {
                for (j = 0; j < n->data.w; j++)
                {
                    if (pbm_getpixel(n->data.bitmap, j, i) == 1)
                    {
                        bin_msk_r[starty + i][startx + j] = 1;
                    }
                }
            }
        }
        n = n->next;
        comp_cnt++;
    }

    comp_cnt = 0;
    n = list;
    while (n != NULL)
    {
        startx = n->data.xpos;
        starty = n->data.ypos;
        if (clus[comp_cnt] == 0)
        {

            /* Flip text component */
            bin_flip = (unsigned char **) alloc_img(n->data.h, n->data.w,
                                                    sizeof(unsigned char));
            bin_input = (unsigned char **) alloc_img(n->data.h, n->data.w,
                                                     sizeof(unsigned char));
            bin_img = (unsigned char **) alloc_img(n->data.h, n->data.w,
                                                   sizeof(unsigned char));
            rem_bin = (unsigned char **) alloc_img(n->data.h, n->data.w,
                                                   sizeof(unsigned char));

            for (i = 0; i < n->data.h; i++)
            {
                for (j = 0; j < n->data.w; j++)
                {
                    bin_img[i][j] = bin_msk[starty + i][startx + j];
                }
            }

            for (i = 0; i < n->data.h; i++)
            {
                for (j = 0; j < n->data.w; j++)
                {
                    if (pbm_getpixel(n->data.bitmap, j, i) == 1)
                    {
                        bin_input[i][j] = 1;
                    }
                }
            }

            reverse_cc(bin_input, bin_img, n->data.h, n->data.w, bin_flip, rem_bin);
            multifree(bin_input, 2);
            multifree(bin_img, 2);

            for (i = 0; i < n->data.h; i++)
            {
                for (j = 0; j < n->data.w; j++)
                {
                    if (rem_bin[i][j] == 0)
                    {
                        bin_msk_r[starty + i][startx + j] = bin_flip[i][j];
                    }
                }
            }

            multifree(bin_flip, 2);
            multifree(rem_bin, 2);
        }
        n = n->next;
        comp_cnt++;
    }
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            bin_msk[i][j] = bin_msk_r[i][j];
        }
    }

    multifree(bin_msk_r, 2);
    marktype_free(&im);
}


void make_feat(
        marklistptr list,           /* i : cc info */
        unsigned int comp_num,      /* i : # of component */
        unsigned int height,
        unsigned int width,
        unsigned char ***input_img, /* i : input image */
        unsigned char **bin_msk,    /* i : binary mask */
        double **vector             /* o : feature vector */
)
{
    unsigned char ***inner_edge, ***outer_edge;
    unsigned int i;

    /* Count length of boundary for each component */
    std::vector<int> bound_list(comp_num);
    cnt_boundary_length(list, height, width, bound_list);

    /* Calculate edgeness along boundary */
    alloc_edge_memory(bound_list, comp_num, &inner_edge);
    alloc_edge_memory(bound_list, comp_num, &outer_edge);

    calc_boundary(list, bound_list, inner_edge, outer_edge,
                  height, width, (unsigned char ***) input_img, bin_msk);

    std::vector<double> edge_depth(comp_num), edge_std(comp_num),
            edge_std2(comp_num), edge_max(comp_num), edge_min(comp_num);
    calc_edge(bound_list, comp_num, inner_edge, outer_edge, edge_depth,
              edge_std, edge_std2, edge_max, edge_min);

    for (i = 0; i < comp_num; i++)
    {
        vector[i][0] = edge_depth[i];
        vector[i][1] = edge_max[i] - edge_min[i];
        vector[i][2] = edge_std[i];
        vector[i][3] = edge_std2[i];
    }

    free_edge_memory(bound_list, comp_num, inner_edge);
    free_edge_memory(bound_list, comp_num, outer_edge);
}


unsigned int find_percentile
        (
                std::vector<unsigned int>& data_array,
                int num,
                unsigned int percent
        )
{
    unsigned int thres;

    thres = num * percent / 100;
    return (data_array[thres]);
}

/**
 * Function Name : QuickSort
 *
 * Function Description :
 * Get color information of the biggest connected-component for DEBUG
 *
 * Input/Output  : array, input array
 * Input/Output  : index, index array
 * Input       : p, starting # of element (e.g. 0)
 *             : r, ending # of element (e.g. n-1)
 * Version : 1.0
 */

void QuickSort(
        std::vector<unsigned int>& array,  /* io : input array */
        std::vector<unsigned int>& index,  /* io : index array */
        unsigned int p,   /* i  : starting # of element (e.g. 0) */
        unsigned int r    /* i  : ending   # of element (e.g. n-1) */
)
{
    unsigned int q;
    if (p < r)
    {
        q = Partition(array, index, p, r);
        QuickSort(array, index, p, q);
        QuickSort(array, index, q + 1, r);
    }
}

/**
 * Function Name : Partition
 *
 * Function Description :
 * Partition
 *
 * Input/Output  : array, input array
 * Input/Output  : index, index array
 *               : p, starting # of element (e.g. 0)
 *               : r, ending # of element (e.g. n-1)
 * Output : position for target
 * Version : 1.0
 */

unsigned int Partition
        (
                std::vector<unsigned int>& array,  /* io : input array */
                std::vector<unsigned int>& index,  /* io : index array */
                unsigned int p,      /* i : starting # of element */
                unsigned int r       /* i : ending # of element */
        )
{
    unsigned int x, tempf;
    unsigned int i, j;
    unsigned int tempi;

    x = array[p];
    i = p - 1;
    j = r + 1;
    while (true)
    {
        do
        { j--; }
        while (array[j] > x);
        /* reverse the ineq signs for descending order */

        do
        { i++; }
        while (array[i] < x);
        /* reverse the ineq signs for descending order */

        if (i < j)
        {
            tempf = array[i];
            tempi = index[i];
            array[i] = array[j];
            index[i] = index[j];
            array[j] = tempf;
            index[j] = tempi;
        }
        else
        {
            return j;
        }
    }
}

void Region_growing_cnt
        (
                CC_pixel s,              /* i : seed pixels */
                unsigned char **bin_msk,        /* i : input binary mask */
                unsigned char **out_msk,        /* o : detected region map */
                int width,
                int height,
                unsigned int *count          /* o : number of pixels */
        )
{
    CC_clist *pstart;
    CC_clist *pend;
    CC_clist data;
    CC_clist tmp[4];
    CC_pixel neigh[4];
    int M, i;
    unsigned int cnt = 0;

    /* Initialization */
    pstart = NULL;
    pend = NULL;

    data.pixels.m = s.m;
    data.pixels.n = s.n;
    data.pnext = NULL;

    /* Add a seed pixel to cc check list */
    Addtail(data, &pstart, &pend);
    cnt++;

    out_msk[data.pixels.m][data.pixels.n] = 1;

    while (pstart)
    {
        /* Initialize # of neigboring pixels */
        M = 0;

        /* Get a pixel from waiting list */
        data = Removehead(&pstart, &pend);

        /* Find neighboring pixels */
        ConnectedNeighbors_UCHAR(data.pixels, bin_msk, width, height,
                                 &M, neigh);

        for (i = 0; i < M; i++)
        {
            tmp[i].pixels = neigh[i];
            tmp[i].pnext = NULL;

            /* If neighboring pixel is not inspected yet */
            if (out_msk[tmp[i].pixels.m][tmp[i].pixels.n] == 0)
            {
                /* Add pixel to cc check list */
                Addtail(tmp[i], &pstart, &pend);
                cnt++;
                out_msk[tmp[i].pixels.m][tmp[i].pixels.n] = 1;
            }
        }
    }/* while */
    *count = cnt;
}

