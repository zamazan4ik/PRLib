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


#include "clust_io.h"
#include "clust_util.h"


#define eq(a, b) strcmp(a,b)==0


int gettag(FILE* fd, char* tag);

void get_nbands(FILE* fd, struct SigSet* S);

void get_title(FILE* fd, struct SigSet* S);

void get_class(FILE* fd, struct SigSet* S);

void get_classnum(FILE* fd, struct ClassSig* C);

void get_classtype(FILE* fd, struct ClassSig* C);

void get_classtitle(FILE* fd, struct ClassSig* C);

void get_subclass(FILE* fd, struct SigSet* S, struct ClassSig* C);

void get_subclass_pi(FILE* fd, struct SubSig* Sp);

void get_subclass_means(FILE* fd, struct SubSig* Sp, int nbands);

void get_subclass_covar(FILE* fd, struct SubSig* Sp, int nbands);

void G_strip(char* buf);


void I_ReadSigSet(FILE* fd, struct SigSet* S)
{
    char tag[256];

    I_InitSigSet(S);

    while (gettag(fd, tag))
    {
        if (eq (tag, "title:"))
        { get_title(fd, S); }
        if (eq (tag, "nbands:"))
        { get_nbands(fd, S); }
        if (eq (tag, "class:"))
        { get_class(fd, S); }
    }
}

int gettag(FILE* fd, char* tag)
{
    if (fscanf(fd, "%s", tag) != 1)
    { return 0; }
    G_strip(tag);
    return 1;
}

void get_nbands(FILE* fd, struct SigSet* S)
{
    fscanf(fd, "%d", &S->nbands);
}

void get_title(FILE* fd, struct SigSet* S)
{
    char title[1024];

    *title = 0;
    fscanf(fd, "%[\n]", title);
    I_SetSigTitle(S, title);
}

void get_class(FILE* fd, struct SigSet* S)
{
    char tag[1024];
    ClassSig* C;

    C = I_NewClassSig(S);
    while (gettag(fd, tag))
    {
        if (eq(tag, "endclass:"))
        {
            break;
        }
        if (eq(tag, "classnum:"))
        {
            get_classnum(fd, C);
        }
        if (eq(tag, "classtype:"))
        {
            get_classtype(fd, C);
        }
        if (eq(tag, "classtitle:"))
        {
            get_classtitle(fd, C);
        }
        if (eq(tag, "subclass:"))
        {
            get_subclass(fd, S, C);
        }
    }
}

void get_classnum(FILE* fd, struct ClassSig* C)
{
    fscanf(fd, "%ld", &C->classnum);
}

void get_classtype(FILE* fd, struct ClassSig* C)
{
    fscanf(fd, "%d", &C->type);
}

void get_classtitle(FILE* fd, struct ClassSig* C)
{
    char title[1024];

    *title = 0;
    fscanf(fd, "%[\n]", title);
    I_SetClassTitle(C, title);
}

void get_subclass(FILE* fd, SigSet* S, struct ClassSig* C)
{
    SubSig* Sp;
    char tag[1024];

    Sp = I_NewSubSig(S, C);

    while (gettag(fd, tag))
    {
        if (eq(tag, "endsubclass:"))
        {
            break;
        }
        if (eq(tag, "pi:"))
        {
            get_subclass_pi(fd, Sp);
        }
        if (eq(tag, "means:"))
        {
            get_subclass_means(fd, Sp, S->nbands);
        }
        if (eq(tag, "covar:"))
        {
            get_subclass_covar(fd, Sp, S->nbands);
        }
    }
}


void get_subclass_pi(FILE* fd, struct SubSig* Sp)
{
    fscanf(fd, "%lf", &Sp->pi);
}


void get_subclass_means(FILE* fd, struct SubSig* Sp, int nbands)
{
    int i;

    for (i = 0; i < nbands; i++)
    {
        fscanf(fd, "%lf", &Sp->means[i]);
    }
}


void get_subclass_covar(FILE* fd, struct SubSig* Sp, int nbands)
{
    int i, j;

    for (i = 0; i < nbands; i++)
    {
        for (j = 0; j < nbands; j++)
        {
            fscanf(fd, "%lf", &Sp->R[i][j]);
        }
    }
}


void G_strip(char* buf)
{
    char* a, * b;

    /* remove leading white space */
    for (a = b = buf; *a == ' ' || *a == '\t'; a++)
    {}
    if (a != b)
    {
        while (*a)
        {
            *b = *a;
            b++;
            a++;
        }
    }

    /* remove trailing white space */
    for (a = buf; *a; a++)
    {}
    if (a != buf)
    {
        for (a--; *a == ' ' || *a == '\t'; a--)
        {}
        a++;
        *a = 0;
    }
}


