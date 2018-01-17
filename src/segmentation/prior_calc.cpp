/**
 * File name : prior_calc.c
 *
 * File Description : Calculate updated prior for MRF model
 *
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date : 7/8/2008
 * Version : 1.00
 *
 *
 */


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <vector>

#include "TIFF_RW.h"
#include "Main_def.h"

#include "marklist.h"
#include "boundary.h"

#include "allocate.h"
#include "getopt.h"
#include "prior_calc.h"

#define boundary8_j(x) ((((x)==7)||((x)==0)||((x)==1))?1:((((x)==3)||((x)==4)||((x)==5))?-1 : 0))
#define boundary8_i(x) ((((x)==1)||((x)==2)||((x)==3))?1:((((x)==5)||((x)==6)||((x)==7))?-1 : 0))
#define boundary16_i(x) ((((x)==2)||((x)==3)||((x)==4)||((x)==5)||((x)==6))?-2:((((x)==10)||((x)==11)||((x)==12)||((x)==13)||((x)==14))?2:((((x)==1)||((x)==7))?-1:((((x)==15)||((x)==7))?1:0))))
#define boundary16_j(x) ((((x)==14)||((x)==15)||((x)==0)||((x)==1)||((x)==2))?2:((((x)==6)||((x)==7)||((x)==8)||((x)==9)||((x)==10))?-2:((((x)==3)||((x)==13))?1:((((x)==5)||((x)==11))?-1:0))))

const double inv_C[FEATAUG_DIM][FEATAUG_DIM] = {\
  {0.0018, -0.0002, -0.0005, 0.0013, -0.0000, -0.0000}, \
  {-0.0002, 0.0195, -0.0008, -0.0583, 0.0001, 0.0000}, \
  {-0.0005, -0.0008, 0.0085, -0.0032, -0.0000, -0.0000}, \
  {0.0013, -0.0583, -0.0032, 0.2098, -0.0005, -0.0000}, \
  {-0.0000, 0.0001, -0.0000, -0.0005, 0.0007, -0.0000}, \
  {-0.0000, 0.0000, -0.0000, -0.0000, -0.0000, 0.0008}};

/* Internal function declaration */
void Addtail(Nei_list newlist, Nei_header* neighbors,
                    unsigned int comp_cnt);

static Nei_list Removehead(Nei_list** pstart, Nei_list** pend);

void Removeall(Nei_list* pstart);

double mahal_dis(double* vec_a, double* vec_b);

void find_neighbors(
        marklistptr list,        /* i : connected component list */
        Pixel_pos* hw,          /* i : height width */
        Nei_header* neighbors    /* io : neighborhood que */
)
{
    unsigned int** loc_map;
    int x, y, i, j, k, f, ci;
    int height, width;
    unsigned int cur_comp_num, nei_num, comp_cnt, cur_size;
    Nei_list data;
    unsigned int search_len;
    marklistptr n;
    unsigned int i_mark, j_mark;
    int startx, starty, ti, tj, dir;
    double dist;
    char check_flg;
    double feat_dis;
    double tmp, tmp2;

    height = hw->i;
    width = hw->j;
    loc_map = (unsigned int**) alloc_img(height, width, sizeof(unsigned int));
    n = list;
    comp_cnt = 0;
    while (n != NULL)
    {
        /* Put a marker at centroid of cc */
        startx = n->data.xpos;
        starty = n->data.ypos;
        i_mark = n->data.ycen + starty;
        j_mark = n->data.xcen + startx;

        /* Label at the center of mass of connected component */
        if (loc_map[i_mark][j_mark] == 0)
        { /* empty */
            loc_map[i_mark][j_mark] = comp_cnt + 1; /* Label number */
            neighbors[comp_cnt].cur_pos.i = i_mark;
            neighbors[comp_cnt].cur_pos.j = j_mark;
            neighbors[comp_cnt].size = (unsigned int) n->data.h;
        }
        else
        {
            check_flg = FLG_OFF;
            for (dir = 0; dir < 8; dir++)
            {
                ti = i_mark + boundary8_i(dir);
                tj = j_mark + boundary8_j(dir);
                if (ti < 0 || ti >= height || tj < 0 || tj >= width)
                {
                    continue;
                }
                if (loc_map[ti][tj] == 0)
                { /* empty */
                    check_flg = FLG_ON;
                    loc_map[ti][tj] = comp_cnt + 1; /* Label number */
                    neighbors[comp_cnt].cur_pos.i = ti;
                    neighbors[comp_cnt].cur_pos.j = tj;
                    neighbors[comp_cnt].size = (unsigned int) n->data.h;
                    break;
                }
            }
            if (check_flg == FLG_OFF)
            {
                for (dir = 0; dir < 16; dir++)
                {
                    ti = i_mark + boundary16_i(dir);
                    tj = j_mark + boundary16_j(dir);
                    if (ti < 0 || ti >= height || tj < 0 || tj >= width)
                    {
                        continue;
                    }
                    if (loc_map[ti][tj] == 0)
                    { /* empty */
                        check_flg = FLG_ON;
                        loc_map[ti][tj] = comp_cnt + 1; /* Label number */
                        neighbors[comp_cnt].cur_pos.i = ti;
                        neighbors[comp_cnt].cur_pos.j = tj;
                        break;
                    }
                }
                if (check_flg == FLG_OFF)
                {
                    printf("FATAL ERROR: Cannot locate the centroid\n");
                    exit;
                }
            }
        }
        comp_cnt++;
        n = n->next;
    }

    for (ci = 0; ci < comp_cnt; ci++)
    {
        /* Get current component location */
        x = neighbors[ci].cur_pos.i;
        y = neighbors[ci].cur_pos.j;

        if (height < width)
        {
            search_len = (unsigned int) floor((double) height / 2) - 1;
        }
        else
        {
            search_len = (unsigned int) floor((double) width / 2) - 1;
        }

        /* Get current component number */
        cur_comp_num = loc_map[x][y] - 1;
        cur_size = neighbors[ci].size;

        /* Search k-th surrounding square box */
        nei_num = 0;
        for (k = 1; k < search_len; k++)
        {
            for (i = x - k; i <= x + k; i += (k + k))
            {
                for (j = y - k; j <= y + k; j++)
                {
                    if (i < 0 || i >= height || j < 0 || j >= width)
                    {
                        continue;
                    }
                    if (loc_map[i][j] != 0)
                    { /* if neighbor is found */
                        if (neighbors[loc_map[i][j] - 1].size <= cur_size / 4.0)
                        {
                            continue;
                        }

                        /* Add the neighbor to que */
                        data.nei_info.comp_num = loc_map[i][j] - 1;

                        dist = sqrt((i - x) * (i - x) + (j - y) * (j - y));
                        data.nei_info.dis = dist;
                        Addtail(data, neighbors, cur_comp_num);
                        nei_num++;

                        /* mutual check */
                        data.nei_info.comp_num = cur_comp_num;
                        data.nei_info.dis = dist;
                        Addtail(data, neighbors, loc_map[i][j] - 1);
                    }
                }
            }
            for (j = y - k; j <= y + k; j += (k + k))
            {
                for (i = x - k + 1; i <= x + k - 1; i++)
                {
                    if (i < 0 || i >= height || j < 0 || j >= width)
                    {
                        continue;
                    }
                    if (loc_map[i][j] != 0)
                    { /* if neighbor is found */
                        if (neighbors[loc_map[i][j] - 1].size <= cur_size / 4.0)
                        {
                            continue;
                        }

                        /* Add the neighbor to que */
                        data.nei_info.comp_num = loc_map[i][j] - 1;

                        dist = sqrt((i - x) * (i - x) + (j - y) * (j - y));
                        data.nei_info.dis = dist;
                        Addtail(data, neighbors, cur_comp_num);
                        nei_num++;

                        /* mutual check */
                        data.nei_info.comp_num = cur_comp_num;
                        data.nei_info.dis = dist;
                        Addtail(data, neighbors, loc_map[i][j] - 1);
                    }
                }
            }
            if (nei_num >= NEAREST_K)
            {
                break;
            }
        }
    }
    multifree(loc_map, 2);

}

/**
 * Function Name : Addtail
 *
 * Function Description :
 * Add a new node to chain
 *
 * Input/Output: newlist, new node
 *             : neighbors, starting address of chain
 *             : comp_cnt,  current cc # 
 * Version : 1.0
 */

void Addtail(
        Nei_list newlist,
        Nei_header* neighbors,
        unsigned int comp_cnt
)
{
    Nei_list* ptr, * nptr;
    Nei_list* pstart, * pend;

    pstart = neighbors[comp_cnt].pstart;
    pend = neighbors[comp_cnt].pend;

    /* Pre-check */
    nptr = pstart;
    while (nptr != NULL)
    {
        /* if already exists */
        if (nptr->nei_info.comp_num == newlist.nei_info.comp_num)
        {
            return;
        }
        nptr = nptr->pnext;
    }

    /* Memory allocation */
    ptr = (Nei_list*) malloc(sizeof(Nei_list));

    if (pend)
    {/* If que is not empty */

        pend->pnext = ptr;
    }
    else
    {
        pstart = ptr;
    }
    pend = ptr;

    *pend = newlist;
    pend->pnext = NULL;

    neighbors[comp_cnt].pstart = pstart;
    neighbors[comp_cnt].pend = pend;
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

Nei_list Removehead
        (
                Nei_list** pstart,
                Nei_list** pend
        )
{
    Nei_list ret;

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
 * Function Name : Removeall
 *
 * Function Description :
 * Remove all of the node from chain
 *
 * Input       : pstart, starting address of chain
 * Version : 1.0
 */

void Removeall
        (
                Nei_list* pstart
        )
{
    Nei_list* p;
    Nei_list* pnext;

    for (p = pstart; p; p = pnext)
    {
        pnext = p->pnext;
        free(p);
    }
}

void init_neighbors(Nei_header** header,    /* io :  pointer to neighborhood info header */
                    unsigned int comp_num   /* i : component # */)
{
    Nei_header* header_ptr;
    unsigned int i;

    header_ptr = (Nei_header*) alloc_array(comp_num, sizeof(Nei_header));
    for (i = 0; i < comp_num; i++)
    {
        header_ptr[i].pstart = NULL;
        header_ptr[i].pend = NULL;
    }
    *header = header_ptr;
}

void free_neighbors(
        Nei_header* header,    /* i :  pointer to neighborhood info header */
        unsigned int comp_num  /* i : component # */
)
{
    for (int i = 0; i < comp_num; ++i)
    {
        Removeall(header[i].pstart);
    }
    multifree(header, 1);
}

double calc_prior(
        unsigned int comp_cnt,  /* i : component # */
        int val,                /* i : class value for specified cc # */
        Nei_header* neighbors,  /* i : neighborhood que */
        int* class_old,             /* i : class */
        Dist_para* para         /* i : parameters for distance func */
)
{
    Nei_list* nptr;
    double distfunc, energy, t, feat;

    /* Calculate updated prior */

    nptr = neighbors[comp_cnt].pstart;
    energy = 0.0;
    while (nptr != nullptr)
    {

        feat = nptr->nei_info.featdis;

        if (val == class_old[nptr->nei_info.comp_num])
        {
            t = 0.0;
        }
        else
        {
            t = 1.0;
        }

        distfunc = para->b_val / (std::pow(feat, para->p_val) + para->a_val);
        energy += t * distfunc;

        nptr = nptr->pnext;
    }
    return energy;
}

void calc_featdis(
        unsigned int comp_num,    /* i : # of connected component */
        marklistptr list,        /* i : connected component list */
        double** feat_list,   /* i : feature vector list */
        Nei_header* neighbors    /* io : neighborhood que */
)
{
    int j;
    unsigned int cnt;
    Nei_list* nptr;
    double feat, feat_i, feat_j, dist_avg;

    for (int i = 0; i < comp_num; i++)
    {
        /* Calculate normalization factor for feature distance */
        nptr = neighbors[i].pstart;
        dist_avg = 0.0;
        cnt = 0;
        while (nptr != nullptr)
        {
            dist_avg += mahal_dis(feat_list[i],
                                           feat_list[nptr->nei_info.comp_num]);
            nptr = nptr->pnext;
            cnt++;
        }
        dist_avg = dist_avg / cnt;
        neighbors[i].normfeat = dist_avg;
    }

    for (int i = 0; i < comp_num; i++)
    {
        nptr = neighbors[i].pstart;

        while (nptr != nullptr)
        {
            j = nptr->nei_info.comp_num;
            feat_i = neighbors[i].normfeat;
            feat_j = neighbors[j].normfeat;
            feat = (double) (mahal_dis(feat_list[i], feat_list[j]));
            feat = 2.0 * feat / (feat_i + feat_j);
            nptr->nei_info.featdis = feat;
            nptr = nptr->pnext;
        }
    }
}

double mahal_dis(double* vec_a,     /* i : vector a */ double* vec_b      /* i : vector b */)
{
    std::vector<double> vec_diff(FEATAUG_DIM), tmp(FEATAUG_DIM);

    for (int i = 0; i < vec_diff.size(); ++i)
    {
        vec_diff[i] = vec_a[i] - vec_b[i];
    }

    for (int j = 0; j < tmp.size(); ++j)
    {
        tmp[j] = 0.0;
        for (int i = 0; i < vec_diff.size(); ++i)
        {
            tmp[j] += vec_diff[i] * inv_C[i][j];
        }
    }
    double dis = 0.0;
    for (int i = 0; i < FEATAUG_DIM; i++)
    {
        dis += tmp[i] * vec_diff[i];
    }
    return std::sqrt(dis);
}


void make_feataug(
        unsigned int comp_num,    /* i : # of connected component */
        double** feat,        /* i : feature vector */
        Nei_header* neighbors,   /* i : neighborhood que */
        double** feataug      /* o : feature vector with augments */
)
{
    for (int i = 0; i < comp_num; i++)
    {
        for (int j = 0; j < FEAT_DIM; j++)
        {
            feataug[i][j] = feat[i][j];
        }
        feataug[i][FEAT_DIM] = (double) neighbors[i].cur_pos.i;
        feataug[i][FEAT_DIM + 1] = (double) neighbors[i].cur_pos.j;
    }
}



