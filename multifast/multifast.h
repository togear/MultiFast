/*
 * multifast.h:
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

#ifndef _MULTIFAST_H_
#define _MULTIFAST_H_

enum working_mode
{
    WORKING_MODE_SEARCH = 0,
    WORKING_MODE_REPLACE
};

struct program_config
{
    char *pattern_file_name;
    enum working_mode w_mode;
    char **input_files;
    long input_files_num;
    char *output_dir;
    short find_first;
    short verbosity;
    short insensitive;
    short lazy_replace;         /* Lazy replace mode */
    short output_show_item;     /* Item number */
    short output_show_dpos;     /* Start position (decimal) */
    short output_show_xpos;     /* Start position (hex) */
    short output_show_reprv;    /* Representative */
    short output_show_pattern;  /* Pattern */
};

void lower_case (char *s, size_t l);
void print_usage (char *progname);
int  search_file (const char *filename, AC_TRIE_t *trie);
int  replace_file (AC_TRIE_t *trie, const char *infile, const char *outfile);
int  match_handler (AC_MATCH_t *m, void *param);
void replace_listener (AC_TEXT_t *, void *);

/* Parameter to match_handler */
struct match_param
{
    unsigned long total_match;
    unsigned long item;
    char *fname;
    int out_file_d;
};

#endif /* _MULTIFAST_H_ */
