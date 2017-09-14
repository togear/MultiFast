/*
 * pattern.c:
 * This file is part of multifast.
 *
    Copyright 2010-2015 Kamiar Kanani <kamiar.kanani@gmail.com>

    multifast is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    multifast is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with multifast.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "pattern.h"
#include "reader.h"
#include "strmm.h"
#include "multifast.h"

static STRMM_t strmem;      /* Holds strings in memory for easy display */
static AC_TRIE_t * trie;    /* Aho-Corasick trie */

extern struct program_config config;

void pattern_print (AC_PATTERN_t *patt);
void pattern_genrep (const char **id);
void pattern_makeacopy (const AC_ALPHABET_t **astrp, size_t len);
int  pattern_addtoac (AC_PATTERN_t *patt);

/* The search call-back function */
extern int match_handler (AC_MATCH_t *m, void *param);

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

int pattern_load (const char *infile, AC_TRIE_t **ptrie)
{
    FILE *fd;
    char *buffer = reader_init();
    struct token_s *mytok;
    int readcount, loopguard = 0;
    static enum token_type last_type = ENTOK_NONE;
    static AC_PATTERN_t last_pattern = {{NULL, 0}, {NULL, 0}, {{0}, 0}};
    
    if ((fd = fopen(infile, "r")) == NULL)
    {
        printf ("Error in reading the pattern file %s\n", infile);
        return -1;
    }

    /* Initialize string memory */
    strmm_init (&strmem);

    /* Initialize automata */
    trie = ac_trie_create ();

    /* Main loop to read patterns from pattern file */
    while ((readcount = fread((void*)buffer, 1, READ_BUFFER_SIZE, fd)) > 0)
    {
        reader_reset_buffer (readcount);

        while ((mytok = reader_get_next_token()))
        {
            if (mytok->type == ENTOK_EOBUF)
                break;

            switch (mytok->type)
            {
            case ENTOK_AX:
                if (last_type == ENTOK_PATTERN || 
                        last_type == ENTOK_REPLACEMENT)
                    pattern_addtoac (&last_pattern);
                last_pattern.id.u.stringy = NULL;
                break;
                
            case ENTOK_ID:
                if (mytok->length == 0)
                    pattern_genrep(&last_pattern.id.u.stringy);
                else
                    last_pattern.id.u.stringy = 
                            strmm_addstrid (&strmem, mytok->value);
                    /* mytok->value is null-terminated */
                break;
                
            case ENTOK_PATTERN:
                if (last_pattern.id.u.stringy == NULL)
                    pattern_genrep (&last_pattern.id.u.stringy);
                
                if (config.insensitive)
                    lower_case(mytok->value, mytok->length);
                
                last_pattern.ptext.astring = mytok->value;
                last_pattern.ptext.length = mytok->length;
                pattern_makeacopy (&last_pattern.ptext.astring, 
                        last_pattern.ptext.length);
                break;
                
            case ENTOK_REPLACEMENT:
                last_pattern.rtext.astring = mytok->value;
                last_pattern.rtext.length = mytok->length;
                pattern_makeacopy (&last_pattern.rtext.astring, 
                        last_pattern.rtext.length);
                break;
                
            case ENTOK_ERR:
                printf ("%s\n", mytok->value);
                loopguard = 1;
                break;
                
            case ENTOK_EOF:
                if (last_type == ENTOK_PATTERN || last_type==ENTOK_REPLACEMENT)
                    pattern_addtoac (&last_pattern);
                loopguard = 1;
                break;
                
            case ENTOK_NONE:
            case ENTOK_EOBUF:
                /* Not expected to come here */
                /* TODO: Warning */
                break;
            }
            last_type = mytok->type;
            if (loopguard)
                break;
        }

        if (loopguard)
            break;
    }

    if (last_type != ENTOK_EOF)
    {
        printf ("Unexpected end of pattern file\n");
        return -1;
    }
    
    /* Finalize the trie */
    ac_trie_finalize (trie);

    *ptrie = trie;

    fclose (fd);
    reader_release();

    return 0;
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

void pattern_makeacopy (const AC_ALPHABET_t **astrp, size_t len)
{
    /* Make a copy of pattern to the string memory */
    if (!strmm_add (&strmem, astrp, len))
    {
        printf("Fatal: Copy Failed\n");
        exit(1);
    }
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

int pattern_addtoac (AC_PATTERN_t *patt)
{
    /* Add pattern to automata */
    switch (ac_trie_add (trie, patt, 0))
    {
        case ACERR_DUPLICATE_PATTERN:
            printf("WARNINIG: Skip duplicate string: %s\n", 
                    patt->ptext.astring);
            break;
            
        case ACERR_LONG_PATTERN:
            printf("WARNINIG: Skip long string: %s\n", patt->ptext.astring);
            break;
            
        case ACERR_ZERO_PATTERN:
            printf("WARNINIG: Skip zero length string.\n");
            break;
            
        case ACERR_SUCCESS:
            if(config.verbosity)
            {
                printf ("Added successfully: %s - ", patt->id.u.stringy);
                pattern_print (patt);
                printf ("\n");
            }
            break;
            
        default:
            printf("WARNINIG: Skip adding string.\n");
            break;
    }

    return 0;
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

void pattern_release ()
{
    /* Release string memory */
    strmm_release (&strmem);
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

void pattern_print (AC_PATTERN_t *patt)
{
    #define DISPLAY_PATT_LEN 80
    
    int i, ishex = 0;
    int maxdisplay = (patt->ptext.length <= DISPLAY_PATT_LEN) ? 
        patt->ptext.length : DISPLAY_PATT_LEN;
    
    for (i = 0; i < maxdisplay; i++)
        if (!isprint(patt->ptext.astring[i]))
            ishex = 1;
    printf ("{");
    
    if (ishex)
    {
        for (i = 0; i < maxdisplay; i++)
            printf ("%s%02x", i ? " " : "", 
                    (unsigned char)(patt->ptext.astring[i]));
    }
    else
    {
        for (i = 0; i < maxdisplay; i++)
            printf ("%c", patt->ptext.astring[i]);
    }
    
    if (patt->ptext.length > DISPLAY_PATT_LEN)
        printf ("...");
    
    printf ("}");
}

/******************************************************************************
 * FUNCTION:
 *****************************************************************************/

void pattern_genrep (const char **id)
{
    /* Get automatic representative for none-representative patterns. */
    static char strid[64];
    static int item = 1;
    sprintf(strid, "p%06d", item++);
    *id = strmm_addstrid(&strmem, strid);
}
