/**
 * File name : COS_library_itnl.h
 *
 * File Description : This is a internal header file for COS_library.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _COS_LIBRARY_ITNL_H_
#define _COS_LIBRARY_ITNL_H_
#endif


/***************************************************/
/*  Internal structure Definition                  */
/***************************************************/
typedef struct
{
    unsigned char ****C_b;    
    double     **gamma_b;
    double     **var_b;
    double     **cnt_1_b;
    unsigned int **H_b;
    unsigned int **V_b;
    unsigned int **R_b;
    unsigned int **L_b;
    unsigned int **T_b;
    unsigned int **B_b;
    unsigned int **lyr_mismatch;
    unsigned int **rev_lyr_mismatch;
    unsigned int **prev_cnt_1;
    
} Pre_dynm_para;

/***************************************************/
/*  Internal variable Definition                   */
/***************************************************/
#define FLG_FIRST      1
#define FLG_NONFIRST   2


/***************************************************/
/*  Internal function declarations                 */
/***************************************************/
static void make_blkseq_c( unsigned char ***img, unsigned int org_height, unsigned int org_width, unsigned int NH, unsigned int NW, unsigned int block, unsigned char ****O_b, double **var_b);

static void thres_mmse( unsigned char ****O_b, unsigned int nh, unsigned int nw, unsigned int block, unsigned char ****C_b, double **gamma_b, double **cnt_1_b);

static double calc_min_gamma(unsigned char **O_b, unsigned int block, int *thres, double *cnt_1);

static void dynamic_seg(unsigned char ****C_b, double **gamma_b, double **var_b, double **cnt_1_b, unsigned int nh, unsigned int nw, Seg_parameter *seg_para, unsigned char **bin_msk_n);

static void cnt_ext_neighbor( unsigned char ****C_b, unsigned int nh, unsigned int nw, unsigned int block, unsigned int **H_b, unsigned int **V_b);

static void cnt_one_edge( unsigned char ****C_b, unsigned int nh, unsigned int nw, unsigned int block, unsigned int **R_b, unsigned int **L_b, unsigned int **T_b, unsigned int **B_b);

static double calc_Vb1( unsigned char sb_prev, unsigned char sb_cur, unsigned int H_b, unsigned int R_b, unsigned int L_b, double var_prev, double var_cur, unsigned int num);

static double calc_Vb2(unsigned char sb_cur, Seg_parameter *seg_para, unsigned int **V_b, unsigned int **B_b, unsigned int **T_b, double **var_b, unsigned int num, unsigned int i, unsigned int j, char first_flg);

static double calc_MSE( unsigned char sb_cur, double gam_b, double var_b);

static void horizontal_dynamic_seg(Pre_dynm_para *pre_dynm_comp, unsigned int  nh, unsigned int  nw, Seg_parameter *seg_para, char first_flg, char *change_flg);

static void decide_binmsk(unsigned char  ****C_b, unsigned int block, unsigned int nh, unsigned int nw, unsigned char **S_b, unsigned char **bin_msk);

static double calc_gamma(unsigned char **O_b, unsigned char **C_b, unsigned int block, double *cnt_1);

static void calc_blocksize(unsigned int org_height, unsigned int org_width, Seg_parameter *seg_para);

static double calc_Vb5( Pre_dynm_para *pre_dynm_comp, unsigned char sb_cur,
Seg_parameter *seg_para, unsigned int  i, unsigned int  j, double  blocksize);


static void calc_overlap_pxl(Pre_dynm_para *pre_dynm_comp, unsigned int block, unsigned int  nh, unsigned int  nw);
static void free_overlap_pxl(Pre_dynm_para *pre_dynm_comp);

static void calc_overlap_bet_layer( Seg_parameter *seg_para,
 Pre_dynm_para *pre_dynm_comp, unsigned int  block, unsigned int  nh,
  unsigned int  nw);

static void free_overlap_bet_layer(Pre_dynm_para *pre_dynm_comp);
static void cnt_mismatch_bet_layer(Seg_parameter *seg_para, 
unsigned char  ****C_b, unsigned int  nh, unsigned int  nw,
unsigned int  **lyr_mis, unsigned int  **rev_lyr_mis,
unsigned int  **prev_cnt_1);


