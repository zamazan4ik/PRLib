/*
* All questions regarding the software should be addressed to
* 
*       Prof. Charles A. Bouman
*       Purdue University
*       School of Electrical and Computer Engineering
*       1285 Electrical Engineering Building
*       West Lafayette, IN 47907-1285
*       USA
*       +1 765 494 0340
*       +1 765 494 3358 (fax)
*       email:  bouman@ecn.purdue.edu
*       http://www.ece.purdue.edu/~bouman
* 
* Copyright (c) 1995 The Board of Trustees of Purdue University.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement is
* hereby granted, provided that the above copyright notice and the following
* two paragraphs appear in all copies of this software.
*
* IN NO EVENT SHALL PURDUE UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE
* USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF PURDUE UNIVERSITY HAS
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* PURDUE UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS,
* AND PURDUE UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
* UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <exception>
#include <stdexcept>

#include "clust_defs.h"

#include "clust_io.h"
#include "clust_util.h"
#include "alloc_util.h"

#include "Main_def.h"

const double PI = 3.141592654;

/* Subroutine in eigen.c */
void eigen(
        double** M,     /* Input matrix */
        std::vector<double>& lambda, /* Output eigenvalues */
        int n           /* Input matrix dimension */
);


static void LogLikelihood_init(struct SigSet* S);

int G_ludcmp(double** a, int n, int* indx, double* d);

int invert(
        double** a, /* input/output matrix */
        int n  /* dimension */
);

void LogLikelihood(
        double* vector,
        std::vector<double>& ll,          /* log likelihood, ll[class] */
        struct SigSet* S,    /* class signatures */
        double text_cost,
        double non_text_cost
);


int classify(
        double** vector,        /* i : feature vector */
        int num,            /* i : # of the vector */
        double** loglikelihood, /* o : loglikelihood */
        std::vector<int>& clus,           /* o : ML estimation */
        int dim,            /* i : vector dimension */
        const char* filename,       /* i : parameter file name */
        double text_cost,      /* i : cost toward text */
        double non_text_cost
)
{
    FILE* fp;
    SigSet S;
    const char* paramfname;
    int NDataVectors;
    double** data;
    int maxindex;
    double maxval;

    /* Read File name : Please modify the path */
    paramfname = filename;

    /* Read SigSet from parameter file */
    if ((fp = fopen(paramfname, "r")) == nullptr)
    {
        throw std::invalid_argument("Can't open parameter file");
    }
    I_ReadSigSet(fp, &S);
    fclose(fp);

    /* Parameter check */
    if (S.nbands != dim)
    {
        throw std::invalid_argument("Error: Dimension of Input feature vector is wrong.");
    }

    /* Single vector input*/
    NDataVectors = num;


    /* Read data */
    data = G_alloc_matrix(NDataVectors, S.nbands);
    for (int i = 0; i < NDataVectors; i++)
    {
        for (int j = 0; j < S.nbands; j++)
        {
            data[i][j] = vector[i][j];
        }
    }

    /* Initialize constants for Log likelihood calculations */
    LogLikelihood_init(&S);

    /* Compute Log likelihood for each class*/
    std::vector<double> ll(S.nclasses);

    for (int i = 0; i < NDataVectors; i++)
    {
        LogLikelihood(data[i], ll, &S, text_cost, non_text_cost);

        maxval = ll[0];
        maxindex = 0;
        for (int j = 0; j < S.nclasses; j++)
        {
            loglikelihood[i][j] = ll[j];
            if (ll[j] > maxval)
            {
                maxval = ll[j];
                maxindex = j;
            }
        }
        /* Final output */
        clus[i] = maxindex;
    }

    G_free_matrix(data);
    return 0;
}


void LogLikelihood(
        double* vector,
        std::vector<double>& ll,          /* log likelihood, ll[class] */
        struct SigSet* S,    /* class signatures */
        double text_cost,    /* cost */
        double non_text_cost
)
{
    //int m;               /* class index */
    int k;               /* subclass index */
    int b1, b2;           /* spectral index */
    int max_nsubclasses; /* maximum number of subclasses */
    int nbands;          /* number of spectral bands */
    //double* subll;        /* log likelihood of subclasses */
    double maxlike;
    double subsum;
    ClassSig* C;
    SubSig* SubS;

    nbands = S->nbands;

    /* determine the maximum number of subclasses */
    max_nsubclasses = 0;
    for (int m = 0; m < S->nclasses; m++)
    {
        if (S->classSig[m].nsubclasses > max_nsubclasses)
        {
            max_nsubclasses = S->classSig[m].nsubclasses;
        }
    }

    /* allocate memory */
    std::vector<double> diff(nbands), subll(max_nsubclasses);

    /* Compute log likelihood for each class */

    /* for each class */
    for (int m = 0; m < S->nclasses; m++)
    {
        C = &(S->classSig[m]);

        /* compute log likelihood for each subclass */
        for (k = 0; k < C->nsubclasses; k++)
        {
            SubS = &(C->subSig[k]);
            subll[k] = SubS->cnst;
            for (b1 = 0; b1 < nbands; b1++)
            {
                diff[b1] = vector[b1] - SubS->means[b1];
                subll[k] -= 0.5 * diff[b1] * diff[b1] * SubS->Rinv[b1][b1];
            }
            for (b1 = 0; b1 < nbands; b1++)
            {
                for (b2 = b1 + 1; b2 < nbands; b2++)
                {
                    subll[k] -= diff[b1] * diff[b2] * SubS->Rinv[b1][b2];
                }
            }
        }

        /* shortcut for one subclass */
        if (C->nsubclasses == 1)
        {
            ll[m] = subll[0];
        }
            /* compute mixture likelihood */
        else
        {
            /* find the most likely subclass */
            for (k = 0; k < C->nsubclasses; k++)
            {
                if (k == 0)
                { maxlike = subll[k]; }
                if (subll[k] > maxlike)
                { maxlike = subll[k]; }
            }

            /* Sum weighted subclass likelihoods */
            subsum = 0;
            for (k = 0; k < C->nsubclasses; k++)
            {
                subsum += std::exp(subll[k] - maxlike) * C->subSig[k].pi;
            }

            ll[m] = std::log(subsum) + maxlike;
            /* Cost added by Eri */
            /* log p(y|0) - log p(y|1) < or >  log ( pi_1/pi_0 ) */
            /* log p(y|0) + log pi_0 - log p(y|1) - log pi_1 < or > 0 */
            ll[1] += text_cost;  /* More text */
            ll[0] += non_text_cost;  /* More non-text */
        }
    }
}


void LogLikelihood_init(struct SigSet* S)
{
    int nbands;
    ClassSig* C;
    SubSig* SubS;

    nbands = S->nbands;
    /* allocate scratch memory */
    std::vector<double> lambda(nbands);

    /* invert matrix and compute constant for each subclass */

    /* for each class */
    for (int m = 0; m < S->nclasses; m++)
    {
        C = &(S->classSig[m]);

        /* for each subclass */
        for (int i = 0; i < C->nsubclasses; i++)
        {
            SubS = &(C->subSig[i]);

            /* Test for symetric  matrix */
            for (int b1 = 0; b1 < nbands; b1++)
            {
                for (int b2 = 0; b2 < nbands; b2++)
                {
                    if (SubS->R[b1][b2] != SubS->R[b2][b1])
                    {
                        fprintf(stderr, "\nWarning: nonsymetric covariance for class %d ", m + 1);
                        fprintf(stderr, "Subclass %d\n", i + 1);
                    }
                    SubS->Rinv[b1][b2] = SubS->R[b1][b2];
                }
            }

            /* Test for positive definite matrix */
            eigen(SubS->Rinv, lambda, nbands);
            for (int b1 = 0; b1 < nbands; b1++)
            {
                if (lambda[b1] <= 0.0)
                {
                    fprintf(stderr, "Warning: nonpositive eigenvalues for class %d", m + 1);
                    fprintf(stderr, "Subclass %d\n", i + 1);
                }
            }

            /* Precomputes the cnst */
            SubS->cnst = (-nbands / 2.0) * std::log(2 * PI);
            for (int b1 = 0; b1 < nbands; b1++)
            {
                SubS->cnst += -0.5 * log(lambda[b1]);
            }

            /* Precomputes the inverse of tex->R */
            invert(SubS->Rinv, nbands);
        }
    }
}

