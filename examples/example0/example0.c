/*
 * example0.c It explains how to use the _settext/_findnext function pair of
 * the ahocorasick library
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
#include <string.h>
#include "ahocorasick.h"

AC_ALPHABET_t *sample_patterns[] = {
    "city",
    "clutter",
    "ever",
    "experience",
    "neo",
    "one",
    "simplicity",
    "utter",
    "whatever",
};
#define PATTERN_COUNT (sizeof(sample_patterns)/sizeof(AC_ALPHABET_t *))

AC_ALPHABET_t *chunk1 = "experience the ease and simplicity of multifast";
AC_ALPHABET_t *chunk2 = "whatever you are be a good one";
AC_ALPHABET_t *chunk3 = "out of clutter, find simplicity";

void print_match (AC_MATCH_t *m);

int main (int argc, char **argv)
{
    unsigned int i;
    AC_TRIE_t *trie;
    AC_PATTERN_t patt;
    AC_TEXT_t chunk;
    AC_MATCH_t match;
    
    /* Get a new trie */
    trie = ac_trie_create ();
    
    for (i = 0; i < PATTERN_COUNT; i++)
    {
        /* Fill the pattern data */
        patt.ptext.astring = sample_patterns[i];
        patt.ptext.length = strlen(patt.ptext.astring);
        
        /* The replacement pattern is not applicable in this program, so better 
         * to initialize it with 0 */
        patt.rtext.astring = NULL;
        patt.rtext.length = 0;
        
        /* Pattern identifier is optional */
        patt.id.u.number = i + 1;
        patt.id.type = AC_PATTID_TYPE_NUMBER;
        
        /* Add pattern to automata */
        ac_trie_add (trie, &patt, 0);
        
        /* We added pattern with copy option disabled. It means that the 
         * pattern memory must remain valid inside our program until the end of 
         * search. If you are using a temporary buffer for patterns then you 
         * may want to make a copy of it so you can use it later. */
    }
    
    /* Now the preprocessing stage ends. You must finalize the trie. Remember 
     * that you can not add patterns anymore. */
    ac_trie_finalize (trie);
    
    /* Finalizing the trie is the slowest part of the task. It may take a 
     * longer time for a very large number of patters */
    
    /* Display the trie if you wish */
    // ac_trie_display (trie);
    
    printf ("Searching: \"%s\"\n", chunk1);
    
    chunk.astring = chunk1;
    chunk.length = strlen (chunk.astring);
    
    /* Set the input text */
    ac_trie_settext (trie, &chunk, 0);
    
    /* The ownership of the input text belongs to the caller program. I.e. the
     * API does not make a copy of that. It must remain valid until the end
     * of search of the given chunk. */
    
    /* Find matches */
    while ((match = ac_trie_findnext(trie)).size)
    {
        print_match (&match);
    }
    
    printf ("Searching: \"%s\"\n", chunk2);
    
    chunk.astring = chunk2;
    chunk.length = strlen (chunk.astring);
    
    /* Set the input text for a new search (keep = 0) */
    ac_trie_settext (trie, &chunk, 0);
    
    while ((match = ac_trie_findnext(trie)).size)
    {        
        print_match (&match);
    }
    
    printf ("Searching: \"%s\"\n", chunk3);
    
    chunk.astring = chunk3;
    chunk.length = strlen (chunk.astring);
    
    /* Set the input text as the successor chunk of the previous one */
    ac_trie_settext (trie, &chunk, 1);
    
    /* When the keep option (3rd argument) in set, then the automata considers 
     * that the given text is the next chunk of the previous one. To see the 
     * difference try it with 0 and compare the result */
    
    while ((match = ac_trie_findnext(trie)).size)
    {        
        print_match (&match);
    }
    
    /* You may release the automata after you have done with it. */
    ac_trie_release (trie);
    
    return 0;
}

void print_match (AC_MATCH_t *m)
{
    unsigned int j;
    AC_PATTERN_t *pp;
    
    printf ("@%2lu found: ", m->position);
    
    for (j = 0; j < m->size; j++)
    {
        pp = &m->patterns[j];
        
        printf("#%ld \"%.*s\", ", pp->id.u.number,
            (int)pp->ptext.length, pp->ptext.astring);
        
        /* CAUTION: the AC_PATTERN_t::ptext.astring pointers, point to the 
         * sample patters in our program, since we added patterns with copy 
         * option disabled.
         */        
    }
    
    printf ("\n");
}
