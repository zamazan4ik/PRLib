
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


void error(char* prog, char* message, char* extra)
{
    fprintf(stderr, "%s: %s %s\n", prog, message, extra);
    exit(1);
}

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
    while (true);

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
    fgoodgets_space = nullptr;

    return p;
}