/**
 * File name : prior_calc.h
 *
 * File Description : Header file for prior_training.c
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date : 7/8/2008
 * Version : 1.00
 *
 *
 */

#ifndef _PRIOR_CALC_H_
#define _PRIOR_CALC_H_

#define  NEAREST_K     6
#define  FEAT_DIM      4
#define  FEATAUG_DIM   6


//TODO: Review constants
#define  EXP_P         12.402208
#define  A_VAL         7.137982
#define  B_VAL         8.495436


/* internal structure definition */
typedef struct
{
    int  i;   
    int  j;  
} Pixel_pos;

typedef struct
{
    unsigned int  comp_num;   /* component # */
    double        dis;        /* distance */
    double        featdis;    /* feature distance */
} Nei_info;

typedef struct nei_list
{
    Nei_info  nei_info;       /* Neighbors info */
    struct nei_list *pnext;   /* Pointer to next data */
} Nei_list;

typedef struct
{
    Pixel_pos  cur_pos;
    Nei_list*  pstart;  
    Nei_list*  pend;  
    double     dist_avg;
    double     normfeat;
    unsigned int size;
} Nei_header;

typedef struct
{
    double  p_val;
    double  a_val;
    double  b_val;
} Dist_para;

/* external function declaration */
void find_neighbors(marklistptr list, Pixel_pos *hw, 
Nei_header *neighbors);
void init_neighbors(Nei_header **header,unsigned int comp_num);
void free_neighbors (Nei_header *header, unsigned int comp_num);
double calc_prior(unsigned int comp_cnt, int val, Nei_header *neighbors, const std::vector<int>& class_old, Dist_para *para);
void calc_featdis(unsigned int comp_num, marklistptr list, double **feat, Nei_header *neighbors);
void make_feataug(unsigned int comp_num, double **feat, Nei_header *neighbors, double **feataug);
void calc_sizecc(unsigned int comp_num, marklistptr list, Nei_header *neighbors);


#endif


