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


#ifndef CLUST_DEFS_H
#define CLUST_DEFS_H

/*****************************************************/
/* This constant determines the ratio of the average */
/* covariance to the minimum allowed covariance.     */
/* It is used to insure that the measured covariance */
/* is not singular. It may need to be adjusted for   */
/* different applications.                           */
/*****************************************************/


struct ClassData
{
	int npixels;
	double **x;   /* pixel list: x[npixels][nbands] */
	double **p;   /* prob        p[npixels][subclasses] */
};

struct SubSig
{
	double N;
	double pi;
	double *means;
	double **R;
	double **Rinv;
	double cnst;
	int used;
};

struct ClassSig
{
	long classnum;
	char *title;
	int used;
	int type;
	int nsubclasses;
	SubSig* subSig;
	ClassData classData;
};

struct SigSet
{
    int nbands;
    int nclasses;
    char *title;
    ClassSig *classSig;
};


#define SIGNATURE_TYPE_MIXED 1


#endif /* CLUST_DEFS_H */
