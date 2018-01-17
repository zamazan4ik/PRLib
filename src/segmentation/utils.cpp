
/*
 *
 *   Author:   Stuart Inglis     <singlis@internz.co.nz>
 *
 *
 * This file contains functions which are common utilities
 *
 */
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include <ctype.h>
#include <math.h>


int isEOF(FILE* fp)
{
    int ch;

    ch = getc(fp);
    ungetc(ch, fp);
    if (ch == EOF)
    { return 1; }
    return 0;
}


unsigned short magic_getshort(FILE* fp)
{
    int ch1, ch2;
    ch1 = getc(fp);
    if (ch1 == EOF)
    {
        ungetc(ch1, fp);
        return 0;
    }
    ch2 = getc(fp);
    if (ch2 == EOF)
    {
        ungetc(ch2, fp);
        return 0;
    }
    ungetc(ch2, fp);
    ungetc(ch1, fp);
    return ((unsigned short) ch1 << 8) | (unsigned short) ch2;
}

unsigned short magic_popshort(FILE* fp)
{
    int ch1, ch2;
    ch1 = getc(fp);
    if (ch1 == EOF)
    { return 0; }
    ch2 = getc(fp);
    if (ch2 == EOF)
    { return 0; }
    return (ch1 << 8) | ch2;
}

unsigned char magic_getbyte(FILE* fp)
{
    int ch1;
    ch1 = getc(fp);
    if (ch1 == EOF)
    {
        ungetc(ch1, fp);
        return 0;
    }
    ungetc(ch1, fp);
    return ((unsigned char) ch1);
}

unsigned char magic_popbyte(FILE* fp)
{
    int ch1;
    ch1 = getc(fp);
    if (ch1 == EOF)
    { return 0; }
    return ((unsigned char) ch1);
}


/* 4 bytes of of pushback must be guaranteed */
unsigned long magic_getlong(FILE* fp)
{
    int ch1, ch2, ch3, ch4;
    ch1 = getc(fp);
    if (ch1 == EOF)
    {
        ungetc(ch1, fp);
        return 0;
    }
    ch2 = getc(fp);
    if (ch2 == EOF)
    {
        ungetc(ch2, fp);
        return 0;
    }
    ch3 = getc(fp);
    if (ch3 == EOF)
    {
        ungetc(ch3, fp);
        return 0;
    }
    ch4 = getc(fp);
    if (ch4 == EOF)
    {
        ungetc(ch4, fp);
        return 0;
    }
    ungetc(ch4, fp);
    ungetc(ch3, fp);
    ungetc(ch2, fp);
    ungetc(ch1, fp);

    return ((unsigned long) ch1 << 24) | ((unsigned long) ch2 << 16) | ((unsigned long) ch3 << 8) |
           ((unsigned long) ch4);
}

unsigned long magic_poplong(FILE* fp)
{
    int ch1, ch2, ch3, ch4;
    ch1 = getc(fp);
    if (ch1 == EOF)
    { return 0; }
    ch2 = getc(fp);
    if (ch2 == EOF)
    { return 0; }
    ch3 = getc(fp);
    if (ch3 == EOF)
    { return 0; }
    ch4 = getc(fp);
    if (ch4 == EOF)
    { return 0; }
    return ((unsigned long) ch1 << 24) | ((unsigned long) ch2 << 16) | ((unsigned long) ch3 << 8) |
           ((unsigned long) ch4);
}


void magic_write(FILE* fp, unsigned long magic_num)
{
    if (fwrite(&magic_num, sizeof(magic_num), 1, fp) != 1)
    {
        error("magic num", "Couldn't write magic number.", "");
    }
}


void magic_check(FILE* fp, unsigned long magic_num)
{
    unsigned long magic;
    if (fread(&magic, sizeof(magic), 1, fp) != 1 || magic != magic_num)
    {
        error("magic num", "Incorrect magic number.", "");
    }
}


unsigned long magic_popnamed(char fn[], int* err)
{
    FILE* fp;
    unsigned long magic;

    if ((fp = fopen(fn, "rb")) != NULL)
    {
        magic = magic_poplong(fp);
        fclose(fp);
        *err = 0;
        return magic;
    }
    else
    {
        *err = 1;
/*	error("magic_popnamed","can't open file",fn);*/
        return 0;
    }
}


void error(char* prog, char* message, char* extra)
{
    fprintf(stderr, "%s: %s %s\n", prog, message, extra);
    exit(1);
}

void warn(char* prog, char* message, char* extra)
{
    fprintf(stderr, "%s: %s %s\n", prog, message, extra);
}



/*void readline(char str[], FILE *fp)
{
  int i=0,ch;

  while(((ch=fgetc(fp))!='\n') && (!feof(fp)))
    str[i++]=ch;
  str[i]='\0';
}*/


#define GOOD_GAP 1024

/*
 * fgoodgets(FILE *fp) and sgoodgets(char *&str)
 * 
 * Both of these functions consume the input parameter.
 * *fp is modified with a normal fgetc(), while *&str is
 * incremented each time a character is read.
 *
 * Input               Output
 * 'a'                 'a'
 * 'b\n'               'b'
 * 'c\r\n'             'c'
 * ''                  ''
 * '\n'                ''
 * '\r\n'              ''
 * '\n\r'              '' '' (two lines)
 *
 * Both functions return a pointer to newly allocated memory.
 * On EOF or EOS, NULL is returned.
 * You must free the space when you are finished with it.
 *
 */

char* fgoodgets(FILE* fp)
{
    static int fgoodgets_len = GOOD_GAP;
    static char* fgoodgets_space = NULL;
    char* p = NULL;
    int i = 0, empty, ch = EOF;

    if (fgoodgets_space == NULL)
    {
        fgoodgets_space = (char*) malloc(sizeof(char) * fgoodgets_len);
    }

    if ((fgoodgets_space == NULL) || (fp == NULL))
    { return NULL; }

    empty = 1;
    do
    {
        ch = (int) fgetc(fp);
        if (ch == EOF)
        {
            break;
        }
        else if (ch == '\n')
        {
            empty = 0;
            break;
        }
        else
        {
            if (i >= fgoodgets_len)
            {
                fgoodgets_len += GOOD_GAP;
                fgoodgets_space = (char*) realloc(fgoodgets_space,
                                                  sizeof(char) * fgoodgets_len);
            }
            empty = 0;
            fgoodgets_space[i++] = (char) ch;
        }
    }
    while (1);

    while ((i >= 1) && (fgoodgets_space[i - 1] == 13))
    {
        i--;
    }

    if ((i >= 0) && (!empty))
    {
        fgoodgets_space[i] = 0;
        p = (char*) malloc(sizeof(char) * (i + 1));
        strcpy(p, fgoodgets_space);
    }

    free(fgoodgets_space);
    fgoodgets_space = NULL;

    return p;
}


int getint(FILE* fp)
{
    register char ch;
    register unsigned int i = 0;

    do
    {
        ch = getc(fp);
        if (feof(fp))
        { return EOF; }
    }
    while (ch < '0' || ch > '9');

    do
    {
        i = i * 10 + ch - '0';
        ch = getc(fp);
        if (feof(fp))
        { return EOF; }
    }
    while (ch >= '0' && ch <= '9');

    return i;
}

/* get_header_int i.e. get an integer from a PNM header ; basically  
   the same as
   above except for comment checking ; above kept because of lower  
   overhead */


int isinteger(char s[])
{
    int i = 0;

    for (i = 0; s[i] != '\0'; i++)
    {
        if (!isdigit(s[i]))
        { return 0; }
    }
    return 1;
}


int isfloat(char s[])
{
    int i = 0;

    for (i = 0; s[i] != '\0'; i++)
    {
        if ((s[i] != '.') && (!isdigit(s[i])))
        { return 0; }
    }
    return 1;
}



