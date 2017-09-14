/*
 * example4.c: Just another example that shows how to use Aho-Corasick library
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

char input_file [] =
    "ACAAGATGCCATTGTCCCCCGGCCTCCTGCTGCTGCTGCTCTCCGGGGCCACGGCCACCGCTGCCCTGCC"
    "CCTGGAGGGTGGCCCCACCGGCCGAGACAGCGAGCATATGCAGGAAGCGGCAGGAATAAGGAAAAGCAGC"
    "CTCCTGACTTTCCTCGCTTGGTGGTTTGAGTGGACCTCCCAGGCCAGTGCCGGGCCCCTCATAGGAGAGG"
    "AAGCTCGGGAGGTGGCCAGGCGGCAGGAAGGCGCACCCCCCCAGCAATCCGCGCGCCGGGACAGAATGCC"
    "CTGCAGGAACTTCTTCTGGAAGACCTTCTCCTCCTGCAAATAAAACCTCACCCATGAATGCTCACGCAAG"
    "TTTAATTACAGACCTGAA";

#define BUFFER_SIZE 64
char buffer[BUFFER_SIZE];

#define PATTERN(p)  {p,sizeof(p)-1}

AC_PATTERN_t sample_patterns[] =
{
    {PATTERN("TGGAGGGT"),       {0, 0}, {{"iris"}, AC_PATTID_TYPE_STRING}},
    {PATTERN("GTGCCGGGCCC"),    {0, 0}, {{"lily"}, AC_PATTID_TYPE_STRING}},
    {PATTERN("TTCT"),           {0, 0}, {{"daisy"}, AC_PATTID_TYPE_STRING}},
    {PATTERN("GGGCCC"),         {0, 0}, {{"rose"}, AC_PATTID_TYPE_STRING}},
    {PATTERN("AACTTCTT"),       {0, 0}, {{"violet"}, AC_PATTID_TYPE_STRING}},
    {PATTERN("TCCCCC"),         {0, 0}, {{"poppy"}, AC_PATTID_TYPE_STRING}},
    {PATTERN("CTT"),            {0, 0}, {{"sunflower"}, AC_PATTID_TYPE_STRING}}
};
#define PATTERN_COUNT (sizeof(sample_patterns)/sizeof(AC_PATTERN_t))

struct parameter
{
    size_t match_count;
    size_t position;
};

int match_handler (AC_MATCH_t *m, void *param);


int main (int argc, char **argv)
{
    unsigned int i;
    AC_TEXT_t intext;
    AC_TRIE_t *trie = ac_trie_create ();
    AC_PATTERN_t *patt;
    AC_STATUS_t status;
    struct parameter my_param = {0, 0};
        
    for (i = 0; i < PATTERN_COUNT; i++)
    {
        patt = &sample_patterns[i];
        status = ac_trie_add (trie, patt, 0);
        
        switch (status)
        {
            case ACERR_DUPLICATE_PATTERN:
                printf ("Add pattern failed: ACERR_DUPLICATE_PATTERN: %s\n", 
                        patt->ptext.astring);
                break;
            case ACERR_LONG_PATTERN:
                printf ("Add pattern failed: ACERR_LONG_PATTERN: %s\n", 
                        patt->ptext.astring);
                break;
            case ACERR_ZERO_PATTERN:
                printf ("Add pattern failed: ACERR_ZERO_PATTERN: %s\n", 
                        patt->ptext.astring);
                break;
            case ACERR_TRIE_CLOSED:
                printf ("Add pattern failed: ACERR_AUTOMATA_CLOSED: %s\n", 
                        patt->ptext.astring);
                break;
            case ACERR_SUCCESS:
                printf ("Pattern Added: %s\n", patt->ptext.astring);
                break;
        }
    }

    ac_trie_finalize (trie);
    
    /* Here we want to show how to search a big text chunk by chunk.
     * The input buffer size is 64 and input file is pretty bigger than that. 
     * In such a case searching usually is done inside a loop. The loop 
     * continues until it consumed all input file.
     */
    
    printf ("Trie finalized.\n\nSearching...\n");

    char *chunk_start = input_file;
    char *end_of_file = input_file + sizeof(input_file);
    intext.astring = buffer;

    while (chunk_start < end_of_file)
    {
        intext.length = (chunk_start < end_of_file) ? 
            sizeof(buffer) : (sizeof(input_file) % sizeof(buffer));
        strncpy (buffer, chunk_start, intext.length);
        
        if (ac_trie_search (trie, 
                &intext, 1, match_handler, (void *)(&my_param)))
            /* if the search stopped in the middle (returned 1) we should break 
             * the loop */
            break;
        
        chunk_start += sizeof(buffer);
    }
    
    printf ("Found %d occurrence in the beginning %d bytes\n", 
        (int)my_param.match_count, (int)my_param.position);
    
    /* TODO: do the same search with _settext/_findnext interface
     * TODO: do a replace with the same trie
     * TODO: show a thread example
     */
    
    ac_trie_release (trie);

    return 0;
}

int match_handler (AC_MATCH_t *m, void *param)
{
    unsigned int j;
    struct parameter *par = (struct parameter *)param;
    
    printf ("@ %2lu : ", m->position);
    
    for (j=0; j < m->size; j++)
        printf ("%s (%s), ", 
                m->patterns[j].id.u.stringy, 
                m->patterns[j].ptext.astring);
    
    printf ("\n");
    
    par->match_count += m->size;
    par->position = m->position;
    
    /* In this case we only needs the first 5 occurrence */
    if (par->match_count >= 5)
        return 1;
    
    return 0;
}
