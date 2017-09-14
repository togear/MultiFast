/*
 * example2.c: Describes the _replace()/_rep_flush() function pair of the 
 * ahocorasick library
 * 
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
#include "ahocorasick.h"

#define PATTERN(p,r)    {{p,sizeof(p)-1},{r,sizeof(r)-1},{{0},0}}
#define CHUNK(c)        {c,sizeof(c)-1}

AC_PATTERN_t patterns[] = {
    PATTERN("city", "[S1]"),    /* Replace "simplicity" with "[S1]" */
    PATTERN("the ", ""),        /* Replace "the " with an empty string */
    PATTERN("and", NULL),       /* Do not replace "and" */
    PATTERN("experience", "[S2]"),
    PATTERN("exp", "[S3]"),
    PATTERN("simplicity", "[S4]"),
    PATTERN("ease", "[S5]"),
};
#define PATTERN_COUNT (sizeof(patterns)/sizeof(AC_PATTERN_t))

AC_TEXT_t input_chunks[] = {
    CHUNK("experience "),
    CHUNK("the ease "),
    CHUNK("and simpli"),
    CHUNK("city of multifast"),
};
#define CHUNK_COUNT (sizeof(input_chunks)/sizeof(AC_TEXT_t))

/* Define a call-back function of type MF_REPLACE_CALBACK_f */
void listener (AC_TEXT_t *text, void *user);

/* The call-back function is called when:
 *      1. the replacement buffer is full
 *      2. the _rep_flush() is called
 * 
 * Replacement buffer size is determined by the MF_REPLACEMENT_BUFFER_SIZE 
 * macro
 */

int main (int argc, char **argv)
{
    unsigned int i;
    AC_TRIE_t *trie;
    
    /* Get a new trie */
    trie = ac_trie_create ();
    
    /* Add patterns to the trie */
    for (i = 0; i < PATTERN_COUNT; i++)
    {
        if (ac_trie_add (trie, &patterns[i], 0) != ACERR_SUCCESS)
            printf("Failed to add pattern \"%.*s\"\n", 
                    (int)patterns[i].ptext.length, patterns[i].ptext.astring);
    }
    
    /* Finalize the trie */
    ac_trie_finalize (trie);
    
    printf("Normal replace mode:\n");

    for (i = 0; i < CHUNK_COUNT; i++)
        /* Replace */
        multifast_replace (trie, 
                &input_chunks[i], MF_REPLACE_MODE_NORMAL, listener, 0);
    
    /* Flush the buffer */
    multifast_rep_flush (trie, 0);
    
    /* After the last chunk you must call the rep_flush() function in order to 
     * receive the final result in your call-back function. The last rep_flush
     * call must be done with 0 in its second parameter.
     * 
     * It is possible to receive intermediate results by calling rep_flush with
     * a non-zero value in the second parameter. The intermediate results may
     * not be as you expect, Because the replacement algorithm keeps the 
     * prefixes in a backlog buffer.
     */
    
    printf("\nLazy replace mode:\n");
    
    /* There are two replacement modes:
     *      1. Normal
     *      2. Lazy
     * 
     * In the normal mode:
     *      - Any pattern occurrence is replaced
     *      - Factor patterns are ignored
     * 
     * In the lazy mode:
     *      - The first occurrence is replaced
     *      - If the first occurrence has a common factor with a successor 
     *        pattern, then the successor is ignored
     * 
     * Example:
     * 
     * Patterns and replacements:
     *      abc -> x
     *      cb -> y
     *      b -> z
     * 
     * Input text and replacement result:
     * 
     *  - Normal mode:
     *      abc => x
     *      abcb => xy
     * 
     *  - Lazy mode:
     *      abc => azc
     *      abcb => azy
     * 
     */
    
    for (i = 0; i < CHUNK_COUNT; i++)
        /* Replace */
        multifast_replace (trie, 
                &input_chunks[i], MF_REPLACE_MODE_LAZY, listener, 0);
    
    /* Flush the buffer */
    multifast_rep_flush (trie, 0);
    
    printf("\n");
    
    /* Release the trie */
    ac_trie_release (trie);
    
    return 0;
}

void listener (AC_TEXT_t *text, void *user)
{
    printf ("%.*s", (int)text->length, text->astring);
}
