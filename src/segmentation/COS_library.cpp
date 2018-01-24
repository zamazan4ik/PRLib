/**
 * File name : COS_library.c
 *
 * File Description : Cost Optimized segmentation library
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date : 12/21/2008
 * Version : 1.10
 *
 *
 */

#include <array>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


#include "Main_def.h"
#include "allocate.h"
#include "segmentation.h"
#include "COS_library.h"
#include "COS_library_itnl.h"

/**
 * Function Name : COS_segment
 *
 * Function Description :
 * COS (Cost Optimized Segmentation) 
 *
 * Input       : img, input color image
 * Input/Output: seg_para, segmentation parameters and outputs 
 * Version : 1.0
 */

void COS_segment(
        unsigned char*** img,       /* i : input image */
        Seg_parameter* seg_para     /* io : segmentation parameters */
)
{
    /* Note : Cost Optimized Segmentation with dynamic programming         */
    /*        (1) Divide an image into overlappig blocks                   */
    /*        (2) Thresholding each overlapping block using mmse           */
    /*        (3) Cost optimization segmentation using dynamic programming */

    unsigned int new_height, new_width;
    unsigned int block, height, width, nh, nw;
    unsigned char**** O_b;
    unsigned char**** C_b;
    double** var_b, ** gamma_b, ** cnt_1_b;
    unsigned char** bin_msk_pad;

    /*** (1) Make an over-lapping block sequence from original image ***/
    /***     with padding                                            ***/


    /* Padding so that image height and width can be multiples of block size */
    block = seg_para->cur_block;
    height = seg_para->height;
    width = seg_para->width;
    new_height = ((height - 1) / block + 1) * block;
    new_width = ((width - 1) / block + 1) * block;

    /* calculate block height & block width */
    nh = seg_para->cur_nh;
    nw = seg_para->cur_nw;

    /* Make an overlapping block sequence & calculate its variance */
    O_b = (unsigned char****) alloc_vols(nh, nw, block, block,
                                         sizeof(unsigned char));
    var_b = (double**) alloc_img(nh, nw, sizeof(double));
    make_blkseq_c(img, height, width, nh, nw, block, O_b, var_b);

    /*** (2) Threshold each block to get blockwise segmentation ***/

    C_b = (unsigned char****) alloc_vols(nh, nw, block, block,
                                         sizeof(unsigned char));
    gamma_b = (double**) alloc_img(nh, nw, sizeof(double));
    cnt_1_b = (double**) alloc_img(nh, nw, sizeof(double));
    thres_mmse(O_b, nh, nw, block, C_b, gamma_b, cnt_1_b);
    multifree(O_b, 4);

    /*** (3) Dynamic programming segmentation ***/

    bin_msk_pad = (unsigned char**) alloc_img(new_height, new_width,
                                              sizeof(unsigned char));
    dynamic_seg(C_b, gamma_b, var_b, cnt_1_b, nh, nw, seg_para, bin_msk_pad);
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            seg_para->binmsk.at<uchar>({i, j}) = bin_msk_pad[i][j];
        }
    }
    multifree(bin_msk_pad, 2);

    /* Free memories */
    multifree(C_b, 4);
    multifree(gamma_b, 2);
    multifree(cnt_1_b, 2);
    multifree(var_b, 2);
}

void make_blkseq_c(
        unsigned char*** img,      /* i : input color image */
        unsigned int org_height,  /* i : original Image height */
        unsigned int org_width,   /* i : original Image width */
        unsigned int nh,          /* i : block height */
        unsigned int nw,          /* i : block width */
        unsigned int block,       /* i : block size */
        unsigned char**** O_b,   /* o : output overlapping block sequence (4-D) */
        double** var_b        /* o : variance for each block  */
)
{
    /* Image is divided into a set of overlapping blocks.            */
    /* For each overlapping block, choose the color axis             */
    /* with the largest variance among the three color axes (RGB).   */
    /* Then, set the selected color data to                          */
    /* nh*nw*block*block format                                      */

    unsigned int half_blk;
    unsigned int x, y;
    int index;
    double mean_x, mean_x_2;
    unsigned int block_size;
    double total, total_2, val;
    std::array<double, 3> array;
    //unsigned int cnt1, cnt2, cnt3;

    block_size = block * block;
    half_blk = block / 2;
    for (size_t i = 0, I = 0; i < nh; i++, I += half_blk)
    {
        for (size_t j = 0, J = 0; j < nw; j++, J += half_blk)
        {
            for (size_t c = 0; c < 3; c++)
            {
                total = 0;
                total_2 = 0;
                for (size_t k = 0; k < block; k++)
                {
                    for (size_t l = 0; l < block; l++)
                    {
                        x = I + k;
                        y = J + l;
                        if ((x < org_height) && (y < org_width))
                        {
                            val = (double) img[c][x][y];
                        }
                        else
                        {
                            val = 0.0;
                        } /* padding */
                        total += val;
                        total_2 += val * val;
                    }
                }
                /* Calculate variance for each overlapping block in RGB */
                mean_x = total / block_size;
                mean_x_2 = total_2 / block_size;
                array[c] = mean_x_2 - mean_x * mean_x;
            }
            /* Find color which has the largest variance in RGB */
            index = 0;
            if (array[1] > array[0])
            {
                index = 1;
            }
            if (array[2] > array[index])
            {
                index = 2;
            }

            var_b[i][j] = array[index];

            /* Set the selected color data to overlapping block sequence O_b */
            for (size_t k = 0; k < block; k++)
            {
                for (size_t l = 0; l < block; l++)
                {
                    x = I + k;
                    y = J + l;
                    if ((x < org_height) && (y < org_width))
                    {
                        O_b[i][j][k][l] = img[index][x][y];
                    }
                    else
                    {
                        O_b[i][j][k][l] = 0;
                    } /* padding */
                }
            }
        }
    }
}

void thres_mmse(
        unsigned char**** O_b,    /* i : input image */
        unsigned int nh,          /* i : block height */
        unsigned int nw,          /* i : block width */
        unsigned int block,       /* i : block size */
        unsigned char**** C_b,    /* o : output image */
        double** gamma_b,      /* o : gamma */
        double** cnt_1_b       /* o : # of 1's in each block */
)
{
    /* Each pixel in each overlapping block is segmented into         */
    /* 0 (Background) or 1 (Foreground) using a clustering procedure. */
    /* The clustering procedure uses partition to minimize            */
    /* the total variance of sub-groups.                              */

    int i, j, k, l, thres;

    for (int i = 0; i < nh; i++)
    {
        for (j = 0; j < nw; j++)
        {

            /* Calculate smallest gamma */
            gamma_b[i][j] = calc_min_gamma(O_b[i][j], block, &thres, &(cnt_1_b[i][j]));

            /* Segmentation for each overlapping block */
            for (k = 0; k < block; k++)
            {
                for (l = 0; l < block; l++)
                {
                    if (O_b[i][j][k][l] > thres)
                    {
                        C_b[i][j][k][l] = 1;
                    }
                    else
                    {
                        C_b[i][j][k][l] = 0;
                    }
                }
            }

            /* Calculate smallest gamma */
            gamma_b[i][j] = calc_gamma(O_b[i][j], C_b[i][j], block, &(cnt_1_b[i][j]));

        }
    }
}

double calc_min_gamma(
        unsigned char** O_b,      /* i : data block */
        unsigned int block,      /* i : block size */
        int* thres,     /* o : threshold */
        double* cnt_1      /* o : number of 1's within a block */
)
{
    /* Calculate gamma for all possible thresholds and output the threshold */
    /* with the smallest gamma.                                             */
    /* -- Definition of gamma                                               */
    /*      gamma = (N_0*var_0+N_1*var_1)/(N_0+N_1)                         */
    /*      N_0   : Number of pixels of '0' by threshold t (G1)             */
    /*      N_1   : Number of pixels of '1' by threshold t (G2)             */
    /*      var_0 : Variance of pixels of '0'              (G1)             */
    /*      var_1 : Variance of pixels of '1'              (G2)             */

    std::array<unsigned int , 256> bak;
    std::array<unsigned char, 256> z;
    std::array<unsigned int , 256> c;
    double G1_s1, G1_s2, G2_s1, G2_s2, total_s1, total_s2;
    int m;
    double total_num, G1_num, G2_num;
    double gamma, min_gamma;
    int min_index;

    /* Initialization */
    for (int n = 0; n < 256; n++)
    {
        bak[n] = 0;  /* buffer for backet sort */
        z[n] = 0;    /* Image value of histgram */
        c[n] = 0;    /* # of pixels of histgram */
    }

    /* backet sort */
    for (int k = 0; k < block; k++)
    {
        for (int l = 0; l < block; l++)
        {
            bak[O_b[k][l]]++;
        }
    }
    /* Create histogram */
    m = 0;
    for (int n = 0; n < 256; n++)
    {
        if (bak[n] != 0)
        {
            z[m] = n;
            c[m] = bak[n];
            m++;
        }
    }

    total_s1 = 0;   /* Total sum of value */
    total_s2 = 0;   /* Total sum of squared value */
    total_num = 0;  /* Total number */

    /* Calculate total sum and squared sum */
    for (int i = 0; i < m; i++)
    {
        total_s1 = total_s1 + z[i] * c[i];
        total_s2 = total_s2 + z[i] * z[i] * c[i];
        total_num = total_num + c[i];
    }

    G1_s1 = 0;   /* Sum of data in group1 (G1) */
    G1_s2 = 0;   /* Sum of squared data in group1 (G1) */
    G1_num = 0;  /* Total number of group1 (G1) */

    for (int t = 0; t < m; t++)
    { /* For all possible thresholds */
        /* Update the total sum of G1 and G2 */
        G1_s1 = G1_s1 + z[t] * c[t];
        G1_s2 = G1_s2 + (z[t] * z[t]) * c[t];
        G1_num = G1_num + c[t];
        G2_s1 = total_s1 - G1_s1;
        G2_s2 = total_s2 - G1_s2;
        G2_num = total_num - G1_num;

        /* calculate gamma */
        if (G1_num == 0)
        {
            gamma = (G2_s2 - G2_s1 * G2_s1 / G2_num) / total_num;
        }
        else if (G2_num == 0)
        {
            gamma = (G1_s2 - G1_s1 * G1_s1 / G1_num) / total_num;
        }
        else
        {
            gamma = ((G1_s2 - G1_s1 * G1_s1 / G1_num) + (G2_s2 - G2_s1 * G2_s1 / G2_num)) / total_num;
        }

        /* Select min gamma */
        if (t == 0)
        {
            min_gamma = gamma;
            min_index = 0;
            *cnt_1 = (double) G2_num;
        }
        else if (gamma < min_gamma)
        {
            min_gamma = gamma;
            min_index = t;
            *cnt_1 = (double) G2_num;
        }
    }
    *thres = z[min_index];
    return min_gamma;
}

void dynamic_seg(
        unsigned char**** C_b,         /* i : block binary image */
        double** gamma_b,           /* i : gamma for each block */
        double** var_b,             /* i : variance for each block */
        double** cnt_1_b,           /* i : # of 1's in each block */
        unsigned int nh,               /* i : block height */
        unsigned int nw,               /* i : block width */
        Seg_parameter* seg_para,       /* i : weight cofficient info */
        unsigned char** bin_msk        /* o : output binary mask */
)
{
    /* Cost Optimized Segmentation with dynamic programming.                */
    /* Dynamic programming is performed line by line and repeated by N      */
    /* iterations. In the 1st iteration, only previous line is used. From   */
    /* the 2nd iteration, two lines above and below the current line are    */
    /* used to optimize the cost in the vertical direction.                 */

    unsigned int block, dynamic_itr_num;
    Pre_dynm_para pre_dynm_comp;
    char change_flg = FLG_OFF;

    /* Read parameter info */
    block = seg_para->cur_block;
    dynamic_itr_num = seg_para->dynamic_itr_num;

    /* Set pre-computed values */
    pre_dynm_comp.gamma_b = gamma_b;
    pre_dynm_comp.var_b = var_b;
    pre_dynm_comp.cnt_1_b = cnt_1_b;
    pre_dynm_comp.C_b = C_b;

    /* Dynamic programming */
    calc_overlap_bet_layer(seg_para, &pre_dynm_comp, block, nh, nw);
    calc_overlap_pxl(&pre_dynm_comp, block, nh, nw);
    for (int K = 0; K < dynamic_itr_num; K++)
    {
        if (K == 0)
        {
            horizontal_dynamic_seg(&pre_dynm_comp, nh, nw, seg_para, FLG_FIRST, &change_flg);
        }
        else
        {
            change_flg = FLG_OFF;
            horizontal_dynamic_seg(&pre_dynm_comp, nh, nw, seg_para, FLG_NONFIRST, &change_flg);
            if (change_flg == FLG_OFF)
            {
                break;
            }
        }
    }
    free_overlap_pxl(&pre_dynm_comp);
    free_overlap_bet_layer(&pre_dynm_comp);
    decide_binmsk(C_b, block, nh, nw, seg_para->S_b, bin_msk);
}

void horizontal_dynamic_seg
        (
                Pre_dynm_para* pre_dynm_comp,  /* i : precomputed values */
                unsigned int nh,              /* i : block height */
                unsigned int nw,              /* i : block width */
                Seg_parameter* seg_para,       /* io: segmentation parameters */
                char first_flg,                /* i : FIRST or NONFIRST */
                char* change_flg               /* o : status changed or not */
        )
{
    /* Horizontal dynamic programming                   */
    /*  -- Procedures                                                          */
    /*     (1) Define 4 candidates of segmentation for each overlapping block  */
    /*         0 : Original                                                    */
    /*         1 : Reversed                                                    */
    /*         2 : All 0's                                                     */
    /*         3 : All 1's                                                     */
    /*     (2) One candidate is chosen so that the cost function is minimized  */
    /*     (3) Cost minimization is performed by dynamic programming method.   */
    /*         Dynamic programming is performed row by row.                    */

    unsigned int overlap, block;
    unsigned int i, j, x, y, m, coarse_i, coarse_j;
    double block_num;
    double lambda1, lambda2, lambda3, lambda4;
    double cost_Vb2, cost_MSE, cost_Vb3, cost_Vb5;
    unsigned char** prev_stat;
    double** sum_cost;
    std::array<double, 4> cost, cost_Vb1;
    unsigned char sb, prev_sb;
    unsigned int** H_b, ** V_b, ** R_b, ** L_b, ** T_b, ** B_b;
    double** gamma_b, ** var_b, ** cnt_1_b;
    int class_old, index;
    unsigned char old_class;

    /* Read parameter info */
    block = seg_para->cur_block;
    lambda1 = seg_para->lambda[seg_para->cur_lyr_itr][0];
    lambda2 = seg_para->lambda[seg_para->cur_lyr_itr][1];
    lambda3 = seg_para->lambda[seg_para->cur_lyr_itr][2];
    lambda4 = seg_para->lambda[seg_para->cur_lyr_itr][3];

    block_num = block * block;
    overlap = block * (block / 2);

    /* Read pre-computed values */
    gamma_b = pre_dynm_comp->gamma_b;
    var_b = pre_dynm_comp->var_b;
    cnt_1_b = pre_dynm_comp->cnt_1_b;

    /* Horizontal overlapping */
    H_b = pre_dynm_comp->H_b;
    V_b = pre_dynm_comp->V_b;
    R_b = pre_dynm_comp->R_b;
    L_b = pre_dynm_comp->L_b;
    T_b = pre_dynm_comp->T_b;
    B_b = pre_dynm_comp->B_b;

    /* Optimal path for dynamic programming */
    prev_stat = (unsigned char**) alloc_img(nw, 4, sizeof(unsigned char));
    /* Total cost for dynamic programming */
    sum_cost = (double**) alloc_img(nw, 4, sizeof(double));


    if (first_flg == FLG_FIRST)
    {
        if (seg_para->prev_S_b != nullptr)
        {
            for (i = 0; i < nh; i++)
            {
                for (j = 0; j < nw; j++)
                {
                    coarse_i = i / MULTI_LAYER_RATE;
                    coarse_j = j / MULTI_LAYER_RATE;
                    if (coarse_i >= seg_para->prev_nh)
                    {
                        coarse_i = seg_para->prev_nh - 1;
                    }
                    if (coarse_j >= seg_para->prev_nw)
                    {
                        coarse_j = seg_para->prev_nw - 1;
                    }
                    seg_para->S_b[i][j] = seg_para->prev_S_b[coarse_i][coarse_j];
                }
            }
        }
    }

    for (i = 0; i < nh; i++)
    {
        /* initialization for row dynamic programming resource */
        for (x = 0; x < nw; x++)
        {
            for (y = 0; y < 4; y++)
            {
                prev_stat[x][y] = 0;
                sum_cost[x][y] = 0;
            }
        }
        for (j = 0; j < nw; j++)
        {
            /* initialization */
            for (x = 0; x < 4; x++)
            {
                cost[x] = 0;
                cost_Vb1[x] = 0;
            }
            for (sb = 0; sb < 4; sb++)
            { /* For 4 current classes */
                /* Cost3: Number of 1's in the current block */
                if (sb == 0)
                {
                    cost_Vb3 = cnt_1_b[i][j];
                }
                else if (sb == 1)
                {
                    cost_Vb3 = block_num - cnt_1_b[i][j];
                }
                else if (sb == 2)
                {
                    cost_Vb3 = 0;
                }
                else if (sb == 3)
                {
                    cost_Vb3 = block_num;
                }
                cost_Vb3 = cost_Vb3 / block_num;

                /* Cost2: # of mismatches in vertical overlap region */
                if (i > 0)
                {
                    cost_Vb2 = calc_Vb2(sb, seg_para, V_b, B_b, T_b, var_b,
                                        overlap, i, j, first_flg);
                }
                else if (i == 0)
                {
                    cost_Vb2 = 0;
                }

                /* Cost_MSE: Variance */
                cost_MSE = calc_MSE(sb, gamma_b[i][j], var_b[i][j]);
                /* cost_MSE = cost_MSE/256.0; */

                /* Cost_Vb5: # of mismatches with previous layer */
                cost_Vb5 = calc_Vb5(pre_dynm_comp, sb, seg_para, i, j, block_num);

                for (prev_sb = 0; prev_sb < 4; prev_sb++)
                {
                    /* For 4 previous classes */
                    cost[prev_sb] = 0;
                    cost_Vb1[prev_sb] = 0;
                    if (j > 0)
                    {
                        /* Cost1 : # of mismatches in horizontal overlap region */
                        cost_Vb1[prev_sb] = calc_Vb1(prev_sb, sb, H_b[i][j], R_b[i][j - 1],
                                                     L_b[i][j], overlap);
                        /* Total Cost */
                        cost[prev_sb] = sum_cost[j - 1][prev_sb]
                                        + lambda1 * cost_Vb1[prev_sb] + lambda2 * cost_Vb2
                                        + lambda3 * cost_Vb3 + cost_MSE
                                        + lambda4 * cost_Vb5;
                    }
                    else if (j == 0)
                    {
                        cost[prev_sb] = lambda2 * cost_Vb2 + lambda3 * cost_Vb3
                                        + cost_MSE + lambda4 * cost_Vb5;
                    }
                }
                /* Find min cost & optimal path */
                index = 0;
                for (m = 1; m <= 3; m++)
                {
                    if (cost[m] <= cost[index])
                    {
                        index = m;
                    }
                }

                /* Record path */
                prev_stat[j][sb] = index;
                sum_cost[j][sb] = cost[index];
            }
        }
        /* Procedure at the end of row */
        /* Find an optimal set of classes with minimum cost in current row */
        class_old = 0;
        for (m = 1; m <= 3; m++)
        {
            if (sum_cost[nw - 1][m] < sum_cost[nw - 1][class_old])
            {
                class_old = m;
            }
        }
        old_class = seg_para->S_b[i][nw - 1];
        seg_para->S_b[i][nw - 1] = class_old;
        if (old_class != seg_para->S_b[i][nw - 1])
        {
            *change_flg = FLG_ON;
        }
        for (x = nw - 1; x > 0; x--)
        {
            old_class = seg_para->S_b[i][x - 1];
            seg_para->S_b[i][x - 1] = prev_stat[x][class_old];
            if (old_class != seg_para->S_b[i][x - 1])
            {
                *change_flg = FLG_ON;
            }
            class_old = prev_stat[x][class_old];
        }
    }
    multifree(sum_cost, 2);
    multifree(prev_stat, 2);
}

void decide_binmsk
        (
                unsigned char**** C_b,            /* i : block binary image */
                unsigned int block,               /* i : block size */
                unsigned int nh,                  /* i : block height */
                unsigned int nw,                  /* i : block width */
                unsigned char** S_b,              /* i : segmentation class */
                unsigned char** bin_msk           /* o : binary mask */
        )
{
    unsigned int i, j, k, l;
    unsigned int BlockStart, BlockStop, BlockHalf, BlockClass;

    /* To decide the final segmentation,
       pick center region of each overlapping block */

    BlockStart = block / 4;
    BlockStop = 3 * (block / 4);
    BlockHalf = block / 2;
    for (i = 0; i < nh; i++)
    {
        for (j = 0; j < nw; j++)
        {
            BlockClass = S_b[i][j];
            if (BlockClass == 0)
            {
                for (k = BlockStart; k < BlockStop; k++)
                {
                    for (l = BlockStart; l < BlockStop; l++)
                    {
                        bin_msk[i * BlockHalf + k][j * BlockHalf + l] = C_b[i][j][k][l];
                    }
                }
            }
            else if (BlockClass == 1)
            {
                for (k = BlockStart; k < BlockStop; k++)
                {
                    for (l = BlockStart; l < BlockStop; l++)
                    {
                        bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1 - C_b[i][j][k][l];
                    }
                }
            }
            else if (BlockClass == 2)
            {
                for (k = BlockStart; k < BlockStop; k++)
                {
                    for (l = BlockStart; l < BlockStop; l++)
                    {
                        bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 0;
                    }
                }
            }
            else if (BlockClass == 3)
            {
                for (k = BlockStart; k < BlockStop; k++)
                {
                    for (l = BlockStart; l < BlockStop; l++)
                    {
                        bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1;
                    }
                }
            }
        }
    }

    /* Boundary for i=0 */
    i = 0;
    for (j = 0; j < nw; j++)
    {
        BlockClass = S_b[i][j];
        if (BlockClass == 0)
        {
            for (k = 0; k < BlockStart; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[k][j * BlockHalf + l] = C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 1)
        {
            for (k = 0; k < BlockStart; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[k][j * BlockHalf + l] = 1 - C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 2)
        {
            for (k = 0; k < BlockStart; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[k][j * BlockHalf + l] = 0;
                }
            }
        }
        if (BlockClass == 3)
        {
            for (k = 0; k < BlockStart; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[k][j * BlockHalf + l] = 1;
                }
            }
        }
    }
    /* Boundary for i=nh-1 */
    i = nh - 1;
    for (j = 0; j < nw; j++)
    {
        BlockClass = S_b[i][j];
        if (BlockClass == 0)
        {
            for (k = BlockStop; k < block; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 1)
        {
            for (k = BlockStop; k < block; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1 - C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 2)
        {
            for (k = BlockStop; k < block; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 0;
                }
            }
        }
        if (BlockClass == 3)
        {
            for (k = BlockStop; k < block; k++)
            {
                for (l = BlockStart; l < BlockStop; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1;
                }
            }
        }
    }
    /* Boundary for j=0 */
    j = 0;
    for (i = 0; i < nh; i++)
    {
        BlockClass = S_b[i][j];
        if (BlockClass == 0)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = 0; l < BlockStart; l++)
                {
                    bin_msk[i * BlockHalf + k][l] = C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 1)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = 0; l < BlockStart; l++)
                {
                    bin_msk[i * BlockHalf + k][l] = 1 - C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 2)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = 0; l < BlockStart; l++)
                {
                    bin_msk[i * BlockHalf + k][l] = 0;
                }
            }
        }
        if (BlockClass == 3)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = 0; l < BlockStart; l++)
                {
                    bin_msk[i * BlockHalf + k][l] = 1;
                }
            }
        }
    }
    /* Boundary for j=nw-1 */
    j = nw - 1;
    for (i = 0; i < nh; i++)
    {
        BlockClass = S_b[i][j];
        if (BlockClass == 0)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = BlockStop; l < block; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 1)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = BlockStop; l < block; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1 - C_b[i][j][k][l];
                }
            }
        }
        else if (BlockClass == 2)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = BlockStop; l < block; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 0;
                }
            }
        }
        if (BlockClass == 3)
        {
            for (k = BlockStart; k < BlockStop; k++)
            {
                for (l = BlockStop; l < block; l++)
                {
                    bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1;
                }
            }
        }
    }

    /* Boundary for i=0 & j=0 */
    i = 0;
    j = 0;
    BlockClass = S_b[i][j];
    if (BlockClass == 0)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[k][l] = C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 1)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[k][l] = 1 - C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 2)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[k][l] = 0;
            }
        }
    }
    if (BlockClass == 3)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[k][l] = 1;
            }
        }
    }

    /* Boundary for i=0 & j=nw-1 */
    i = 0;
    j = nw - 1;
    BlockClass = S_b[i][nw - 1];
    if (BlockClass == 0)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[k][j * BlockHalf + l] = C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 1)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[k][j * BlockHalf + l] = 1 - C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 2)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[k][j * BlockHalf + l] = 0;
            }
        }
    }
    if (BlockClass == 3)
    {
        for (k = 0; k < BlockStart; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[k][j * BlockHalf + l] = 1;
            }
        }
    }

    /* Boundary for i=nh-1 & j=0 */
    i = nh - 1;
    j = 0;
    BlockClass = S_b[i][j];
    if (BlockClass == 0)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[i * BlockHalf + k][l] = C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 1)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[i * BlockHalf + k][l] = 1 - C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 2)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[i * BlockHalf + k][l] = 0;
            }
        }
    }
    else if (BlockClass == 3)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = 0; l < BlockStart; l++)
            {
                bin_msk[i * BlockHalf + k][l] = 1;
            }
        }
    }

    /* Boundary for i=nh-1 & j=nw-1 */
    i = nh - 1;
    j = nw - 1;
    BlockClass = S_b[i][j];
    if (BlockClass == 0)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[i * BlockHalf + k][j * BlockHalf + l] = C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 1)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1 - C_b[i][j][k][l];
            }
        }
    }
    else if (BlockClass == 2)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 0;
            }
        }
    }
    else if (BlockClass == 3)
    {
        for (k = BlockStop; k < block; k++)
        {
            for (l = BlockStop; l < block; l++)
            {
                bin_msk[i * BlockHalf + k][j * BlockHalf + l] = 1;
            }
        }
    }

}

void cnt_ext_neighbor(
        unsigned char**** C_b,    /* i : Block binary image */
        unsigned int nh,          /* i : Block height */
        unsigned int nw,          /* i : Block width */
        unsigned int block,       /* i : Block size */
        unsigned int** H_b,       /* o : # of mismatches in horizontal overlap */
        unsigned int** V_b        /* o : # of mismatches in vertical overlap */
)
{
    /* For each block, count # of mismatches between a neighboring block and  */
    /* current block in horizontal & vertical direction                       */

    int i, j, k, l;

    /* Count # of mismatches in horizontal overlapping region */
    for (i = 0; i < nh; i++)
    {
        H_b[i][0] = 0;
    }
    for (i = 0; i < nh; i++)
    {
        for (j = 1; j < nw; j++)
        {
            H_b[i][j] = 0;
            for (k = 0; k < block; k++)
            {
                for (l = 0; l < block / 2; l++)
                {
                    H_b[i][j] = H_b[i][j] + (C_b[i][j - 1][k][block / 2 + l] != C_b[i][j][k][l]);
                }
            }
        }
    }

    /* Count # of mismatches in vertical overlapping region */
    for (j = 0; j < nw; j++)
    {
        V_b[0][j] = 0;
    }
    for (i = 1; i < nh; i++)
    {
        for (j = 0; j < nw; j++)
        {
            V_b[i][j] = 0;
            for (k = 0; k < block / 2; k++)
            {
                for (l = 0; l < block; l++)
                {
                    V_b[i][j] = V_b[i][j] + (C_b[i - 1][j][block / 2 + k][l] != C_b[i][j][k][l]);
                }
            }
        }
    }
}

void cnt_one_edge(
        unsigned char**** C_b,      /* i : block binary image */
        unsigned int nh,            /* i : block height */
        unsigned int nw,            /* i : block width */
        unsigned int block,         /* i : block size */
        unsigned int** R_b,         /* o : # of 1's of right  half */
        unsigned int** L_b,         /* o : # of 1's of left   half */
        unsigned int** T_b,         /* o : # of 1's of top    half */
        unsigned int** B_b          /* o : # of 1's of bottom half */
)
{
    /* For each block, count # of 1's pixels in right, */
    /* left, top and bottom half                       */

    /* Count # of 1's pixels in Right half and Left half */
    for (int i = 0; i < nh; i++)
    {
        for (int j = 0; j < nw; j++)
        {
            for (int k = 0; k < block; k++)
            {
                for (int l = block / 2; l < block; l++)
                {
                    R_b[i][j] = R_b[i][j] + (C_b[i][j][k][l] == 1);
                }
                for (int l = 0; l < block / 2; l++)
                {
                    L_b[i][j] = L_b[i][j] + (C_b[i][j][k][l] == 1);
                }
            }
        }
    }

    /* Count # of 1's pixels in Bottom half */
    for (int i = 0; i < nh; i++)
    {
        for (int j = 0; j < nw; j++)
        {
            for (int k = block / 2; k < block; k++)
            {
                for (int l = 0; l < block; l++)
                {
                    B_b[i][j] = B_b[i][j] + (C_b[i][j][k][l] == 1);
                }
            }
            for (int k = 0; k < block / 2; k++)
            {
                for (int l = 0; l < block; l++)
                {
                    T_b[i][j] = T_b[i][j] + (C_b[i][j][k][l] == 1);
                }
            }
        }
    }
}

double calc_Vb1(
        unsigned char sb_prev,   /* i : previous class */
        unsigned char sb_cur,    /* i : current class */
        unsigned int H_b,        /* i : # of mismatches in horizontal direction */
        unsigned int R_b,        /* i : # of 1's in right half */
        unsigned int L_b,        /* i : # of 1's in left  half */
        unsigned int num         /* i : # of overlap */
)
{
    /* Calculate Horizontal mismatches of selected classes  */
    /* between previous block and current block             */

    double cost = 0.0;

    if ((sb_prev == 0 && sb_cur == 0) || (sb_prev == 1 && sb_cur == 1))
    {
        cost = H_b;
    }
    else if ((sb_prev == 0 && sb_cur == 1) || (sb_prev == 1 && sb_cur == 0))
    {
        cost = num - H_b;
    }
    else if ((sb_prev == 0 && sb_cur == 2) || (sb_prev == 1 && sb_cur == 3))
    {
        cost = R_b;
    }
    else if ((sb_prev == 0 && sb_cur == 3) || (sb_prev == 1 && sb_cur == 2))
    {
        cost = num - R_b;
    }
    else if ((sb_prev == 2 && sb_cur == 0) || (sb_prev == 3 && sb_cur == 1))
    {
        cost = L_b;
    }
    else if ((sb_prev == 3 && sb_cur == 0) || (sb_prev == 2 && sb_cur == 1))
    {
        cost = num - L_b;
    }
    else if ((sb_prev == 2 && sb_cur == 2) || (sb_prev == 3 && sb_cur == 3))
    {
        cost = 0;
    }
    else if ((sb_prev == 2 && sb_cur == 3) || (sb_prev == 3 && sb_cur == 2))
    {
        cost = num;
    }

    return cost / num;
}

double calc_Vb2(
        unsigned char sb_cur,        /* i : current class */
        Seg_parameter* seg_para,     /* i : previous class */
        unsigned int** V_b,           /* i : # of mismatches in vertical direction */
        unsigned int** B_b,           /* i : # of 1's in bottom half */
        unsigned int** T_b,           /* i : # of 1's in top half */
        double** var_b,           /* i : variance of previous block */
        unsigned int num,             /* i : # of overlap */
        unsigned int i,               /* i : ith row in block */
        unsigned int j,               /* i : jth column in block */
        char first_flg                 /* i : FIRST or NONFIRST */
)
{
    /* Calculate Vertical mismatches of selected classes  */
    /* between previous block and current block           */

    double cost_above = 0.0;
    double cost_below = 0.0;
    double cost = 0.0;
    unsigned int cnt = 0;
    unsigned char sb_prev_above, sb_prev_below;
    unsigned char** S_b;

    S_b = seg_para->S_b;
    if (i >= 1)
    {
        sb_prev_above = S_b[i - 1][j];

        if ((sb_prev_above == 0 && sb_cur == 0) || (sb_prev_above == 1 && sb_cur == 1))
        {
            cost_above = V_b[i][j];
        }
        else if ((sb_prev_above == 0 && sb_cur == 1) || (sb_prev_above == 1 && sb_cur == 0))
        {
            cost_above = num - V_b[i][j];
        }
        else if ((sb_prev_above == 0 && sb_cur == 2) || (sb_prev_above == 1 && sb_cur == 3))
        {
            cost_above = B_b[i - 1][j];
        }
        else if ((sb_prev_above == 0 && sb_cur == 3) || (sb_prev_above == 1 && sb_cur == 2))
        {
            cost_above = num - B_b[i - 1][j];
        }
        else if ((sb_prev_above == 2 && sb_cur == 0) || (sb_prev_above == 3 && sb_cur == 1))
        {
            cost_above = T_b[i][j];
        }
        else if ((sb_prev_above == 3 && sb_cur == 0) || (sb_prev_above == 2 && sb_cur == 1))
        {
            cost_above = num - T_b[i][j];
        }
        else if ((sb_prev_above == 2 && sb_cur == 2) || (sb_prev_above == 3 && sb_cur == 3))
        {
            cost_above = 0;
        }
        else if ((sb_prev_above == 2 && sb_cur == 3) || (sb_prev_above == 3 && sb_cur == 2))
        {
            cost_above = num;
        }

        cost += cost_above;
        cnt++;
    }

    if (i < seg_para->cur_nh - 1)
    {
        if (first_flg == FLG_NONFIRST || seg_para->prev_S_b != nullptr)
        {
            sb_prev_below = S_b[i + 1][j];

            if ((sb_prev_below == 0 && sb_cur == 0) || (sb_prev_below == 1 && sb_cur == 1))
            {
                cost_below = V_b[i + 1][j];
            }
            else if ((sb_prev_below == 0 && sb_cur == 1) || (sb_prev_below == 1 && sb_cur == 0))
            {
                cost_below = num - V_b[i + 1][j];
            }
            else if ((sb_prev_below == 0 && sb_cur == 2) || (sb_prev_below == 1 && sb_cur == 3))
            {
                cost_below = T_b[i + 1][j];
            }
            else if ((sb_prev_below == 0 && sb_cur == 3) || (sb_prev_below == 1 && sb_cur == 2))
            {
                cost_below = num - T_b[i + 1][j];
            }
            else if ((sb_prev_below == 2 && sb_cur == 0) || (sb_prev_below == 3 && sb_cur == 1))
            {
                cost_below = B_b[i][j];
            }
            else if ((sb_prev_below == 3 && sb_cur == 0) || (sb_prev_below == 2 && sb_cur == 1))
            {
                cost_below = num - B_b[i][j];
            }
            else if ((sb_prev_below == 2 && sb_cur == 2) || (sb_prev_below == 3 && sb_cur == 3))
            {
                cost_below = 0;
            }
            else if ((sb_prev_below == 2 && sb_cur == 3) || (sb_prev_below == 3 && sb_cur == 2))
            {
                cost_below = num;
            }

            cost += cost_below;
            cnt++;
        }
    }
    cost = cost / (double) cnt;
    return cost / num;
}

double calc_MSE(
        unsigned char sb_cur,  /* i : current class */
        double gam_b,        /* i : gamma */
        double var_b         /* i : variance */
)
{
    /* Calculate MSE  */

    double cost = 0.0;

    if (sb_cur == 0 || sb_cur == 1)
    {
        cost = gam_b;
    }
    else if (sb_cur == 2)
    {
        cost = var_b;
    }
    else if (sb_cur == 3)
    {
        cost = var_b;
    }
    return std::sqrt(cost);
}

double calc_gamma(
        unsigned char** O_b,       /* i : data block */
        unsigned char** C_b,       /* i : binary block */
        unsigned int block,      /* i : block size */
        double* cnt_1       /* o : number of 1's within a block */
)
{
    /* Calculate gamma using the input binary mask             */
    /* -- Definition of gamma                                  */
    /*    gamma = (N_0*var_0+N_1*var_1)/(N_0+N_1)              */
    /*    N_0   : Number of pixels of '0'   (G1)               */
    /*    N_1   : Number of pixels of '1'   (G2)               */
    /*    var_0 : Variance of pixels of '0' (G1)               */
    /*    var_1 : Variance of pixels of '1' (G2)               */

    double G1_s1, G1_s2, G2_s1, G2_s2;
    double total_num, G1_num, G2_num;
    double gamma;

    total_num = block * block;  /* Total number */

    G1_s1 = 0.0;   /* Sum of data in group1 (G1) */
    G1_s2 = 0.0;   /* Sum of squared data in group1 (G1) */
    G1_num = 0.0;  /* Total number of group1 (G1) */
    G2_s1 = 0.0;   /* Sum of data in group1 (G2) */
    G2_s2 = 0.0;   /* Sum of squared data in group2 (G2) */
    G2_num = 0.0;  /* Total number of group1 (G2) */

    for (int i = 0; i < block; ++i)
    {
        for (int j = 0; j < block; ++j)
        {
            if (C_b[i][j] == 0)
            {
                G1_s1 += O_b[i][j];
                G1_s2 += O_b[i][j] * O_b[i][j];
                G1_num++;
            }
            else
            {
                G2_s1 += O_b[i][j];
                G2_s2 += O_b[i][j] * O_b[i][j];
                G2_num++;
            }
        }
    }
    /* calculate gamma */
    if (G1_num == 0)
    {
        gamma = (G2_s2 - G2_s1 * G2_s1 / G2_num) / total_num;
    }
    else if (G2_num == 0)
    {
        gamma = (G1_s2 - G1_s1 * G1_s1 / G1_num) / total_num;
    }
    else
    {
        gamma = ((G1_s2 - G1_s1 * G1_s1 / G1_num) + (G2_s2 - G2_s1 * G2_s1 / G2_num)) / total_num;
    }

    *cnt_1 = G2_num;
    return gamma;
}

double calc_Vb5(
        Pre_dynm_para* pre_dynm_comp,  /* i : precomputed values */
        unsigned char sb_cur,        /* i : current class */
        Seg_parameter* seg_para,     /* i : segmentation parameter */
        unsigned int i,               /* i : ith row in block */
        unsigned int j,               /* i : jth column in block */
        double blocksize               /* i : block size */
)
{
    /* Calculate mismatches between current layer and
       previous layer segmentation   */

    unsigned int cost;

    if (seg_para->prev_binmsk.total() == 0)
    {
        return 0.0;
    }

    if (sb_cur == 0)
    {
        cost = pre_dynm_comp->lyr_mismatch[i][j];
    }
    else if (sb_cur == 1)
    {
        cost = pre_dynm_comp->rev_lyr_mismatch[i][j];
    }
    else if (sb_cur == 2)
    {
        cost = pre_dynm_comp->prev_cnt_1[i][j];
    }
    else if (sb_cur == 3)
    {
        cost = blocksize - pre_dynm_comp->prev_cnt_1[i][j];
    }

    return ((double) cost / blocksize);

}

void calc_overlap_pxl
        (
                Pre_dynm_para* pre_dynm_comp,  /* io : precomputed values */
                unsigned int block,
                unsigned int nh,              /* i : block height */
                unsigned int nw               /* i : block width */
        )
{
    unsigned int** H_b, ** V_b, ** R_b, ** L_b, ** T_b, ** B_b;
    unsigned char**** C_b;

    /* Read pre-computed values */
    C_b = pre_dynm_comp->C_b;

    /* memory allocation */
    /* Horizontal overlapping */
    H_b = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));
    /* Vertical   overlapping */
    V_b = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));
    /* # of 1's in Right  half */
    R_b = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));
    /* # of 1's in Left   half */
    L_b = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));
    /* # of 1's in Top    half */
    T_b = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));
    /* # of 1's in Bottom half */
    B_b = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));

    /* Pre-compute H_b, V_b, R_b, L_b, T_b, B_b */
    cnt_ext_neighbor(C_b, nh, nw, block, H_b, V_b);
    cnt_one_edge(C_b, nh, nw, block, R_b, L_b, T_b, B_b);

    pre_dynm_comp->H_b = H_b;
    pre_dynm_comp->V_b = V_b;
    pre_dynm_comp->R_b = R_b;
    pre_dynm_comp->L_b = L_b;
    pre_dynm_comp->T_b = T_b;
    pre_dynm_comp->B_b = B_b;
}

void free_overlap_pxl(Pre_dynm_para* pre_dynm_comp   /* i : precomputed values */)
{
    /* free memory */
    multifree(pre_dynm_comp->H_b, 2);
    multifree(pre_dynm_comp->V_b, 2);
    multifree(pre_dynm_comp->R_b, 2);
    multifree(pre_dynm_comp->L_b, 2);
    multifree(pre_dynm_comp->T_b, 2);
    multifree(pre_dynm_comp->B_b, 2);
}

void calc_overlap_bet_layer
        (
                Seg_parameter* seg_para,       /* i : segmentation parameters */
                Pre_dynm_para* pre_dynm_comp,  /* io : precomputed values */
                unsigned int block,
                unsigned int nh,              /* i : block height */
                unsigned int nw               /* i : block width */
        )
{
    unsigned int** lyr_mismatch, ** rev_lyr_mismatch, ** prev_cnt_1;
    unsigned char**** C_b;

    /* Read pre-computed values */
    C_b = pre_dynm_comp->C_b;

    /* # of mismatched pixels (1->0 and 1->1) between layers */
    lyr_mismatch = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));
    rev_lyr_mismatch = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));
    prev_cnt_1 = (unsigned int**) alloc_img(nh, nw, sizeof(unsigned int));

    /* Pre-compute */
    cnt_mismatch_bet_layer(seg_para, C_b, nh, nw, lyr_mismatch, rev_lyr_mismatch, prev_cnt_1);

    pre_dynm_comp->lyr_mismatch = lyr_mismatch;
    pre_dynm_comp->rev_lyr_mismatch = rev_lyr_mismatch;
    pre_dynm_comp->prev_cnt_1 = prev_cnt_1;
}

void free_overlap_bet_layer(Pre_dynm_para* pre_dynm_comp   /* i : precomputed values */)
{
    /* free memory */
    multifree(pre_dynm_comp->lyr_mismatch, 2);
    multifree(pre_dynm_comp->rev_lyr_mismatch, 2);
    multifree(pre_dynm_comp->prev_cnt_1, 2);
}

void cnt_mismatch_bet_layer(
        Seg_parameter* seg_para,
        unsigned char**** C_b,    /* i : Block binary image */
        unsigned int nh,          /* i : Block height */
        unsigned int nw,          /* i : Block width */
        unsigned int** lyr_mis,   /* o : # of mismatches between layers (1->0) */
        unsigned int** rev_lyr_mis, /* o : # of mismatches (1->1) */
        unsigned int** prev_cnt_1   /* o : # of 1 pixels on previous layer */
)
{
    double cost, mismatch, rev_mismatch, cnt_one;
    unsigned int half_block, block;
    unsigned int prev_value, pos_i, pos_j;

    /* Calculate mismatches between current layer and
       previous layer segmentation   */

    if (seg_para->prev_binmsk.total() == 0)
    {
        return;
    }
    block = seg_para->cur_block;
    half_block = block / 2;

    for (int i = 0, I = 0; i < nh; i++, I += half_block)
    {
        for (int j = 0, J = 0; j < nw; j++, J += half_block)
        {
            mismatch = 0;
            rev_mismatch = 0;
            cnt_one = 0;
            for (int m = 0; m < block; m++)
            {
                for (int n = 0; n < block; n++)
                {
                    pos_i = I + m;
                    pos_j = J + n;
                    if (pos_i >= seg_para->height || pos_j >= seg_para->width)
                    {
                        prev_value = 0;
                    }
                    else
                    {
                        prev_value = seg_para->prev_binmsk.at<uchar>({pos_i, pos_j});
                    }
                    if (prev_value == 1)
                    {
                        if (C_b[i][j][m][n] == 0)
                        {
                            mismatch++;
                        }
                        if (C_b[i][j][m][n] == 1)
                        {
                            rev_mismatch++;
                        }

                        cnt_one++;
                    }
                }
            }
            lyr_mis[i][j] = mismatch;
            rev_lyr_mis[i][j] = rev_mismatch;
            prev_cnt_1[i][j] = cnt_one;
        }
    }
}
