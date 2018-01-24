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


#include "clust_util.h"
#include "alloc_util.h"


void I_InitSigSet(struct SigSet* S)
{
    S->nbands = 0;
    S->nclasses = 0;
    S->classSig = nullptr;
    S->title = nullptr;
}


ClassSig*
I_NewClassSig(struct SigSet* S)
{
    struct ClassSig* Sp;
    if (S->nclasses == 0)
    {
        S->classSig = (ClassSig*) G_malloc(sizeof(ClassSig));
    }
    else
    {
        S->classSig = (ClassSig*) G_realloc((char*) S->classSig,
                                                   sizeof(ClassSig) * (S->nclasses + 1));
    }

    Sp = &S->classSig[S->nclasses++];
    Sp->classnum = 0;
    Sp->nsubclasses = 0;
    Sp->used = 1;
    Sp->type = SIGNATURE_TYPE_MIXED;
    Sp->title = nullptr;
    Sp->classData.x = nullptr;
    Sp->classData.p = nullptr;

    return Sp;
}


SubSig* I_NewSubSig(SigSet* S, ClassSig* C)
{
    struct SubSig* Sp;
    int i;

    if (C->nsubclasses == 0)
    {
        C->subSig = (SubSig*) G_malloc(sizeof(SubSig));
    }
    else
    {
        C->subSig = (SubSig*) G_realloc((char*) C->subSig,
                                               sizeof(SubSig) * (C->nsubclasses + 1));
    }

    Sp = &C->subSig[C->nsubclasses++];
    Sp->used = 1;
    Sp->R = (double**) G_calloc(S->nbands, sizeof(double*));
    Sp->R[0] = (double*) G_calloc(S->nbands * S->nbands, sizeof(double));
    for (i = 1; i < S->nbands; i++)
    {
        Sp->R[i] = Sp->R[i - 1] + S->nbands;
    }
    Sp->Rinv = (double**) G_calloc(S->nbands, sizeof(double*));
    Sp->Rinv[0] = (double*) G_calloc(S->nbands * S->nbands, sizeof(double));
    for (i = 1; i < S->nbands; i++)
    {
        Sp->Rinv[i] = Sp->Rinv[i - 1] + S->nbands;
    }
    Sp->means = (double*) G_calloc(S->nbands, sizeof(double));
    Sp->pi = 0;
    Sp->cnst = 0;
    return Sp;
}


void
I_SetSigTitle(struct SigSet* S, char* title)
{
    if (title == nullptr)
    { title = ""; }
    if (S->title)
    { free(S->title); }
    S->title = G_malloc(strlen(title) + 1);
    strcpy(S->title, title);
}

void
I_SetClassTitle(struct ClassSig* C, char* title)
{
    if (title == nullptr)
    { title = ""; }
    if (C->title)
    { free(C->title); }
    C->title = G_malloc(strlen(title) + 1);
    strcpy(C->title, title);
}
