/**
 * File name : classify.h
 *
 * File Description : This is a header file for classify.c 
 *
 * Author : Eri Haneda (haneda@purdue.edu), Purdue University
 * Created Date :
 * Version : 1.00
 *
 */

#ifndef _CLASSIFY_H_
#define _CLASSIFY_H_

#include <vector>

int classify(double **vector, int num, double **loglikelihood, std::vector<int>& clus,
int dim, const char *filename, double text_cost, double non_text_cost);

#endif

