/*
 * multifast.c:
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "pattern.h"
#include "walker.h"
#include "multifast.h"

#define STREAM_BUFFER_SIZE 4096

/* Program configuration */
struct program_config config = 
    {0, WORKING_MODE_SEARCH, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

char *get_outfile_name (const char *dir, const char *file);
int mkpath(const char *path, mode_t mode);

char *output_file_name = NULL;

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

int main (int argc, char **argv)
{
    int i;
    int clopt; /* Command line option */
    AC_TRIE_t *trie; /* Aho-Corasick trie pointer */
    char *infpath, *outfpath;
    
    if(argc < 4)
    {
        print_usage (argv[0]);
        exit(1);
    }

    /* Read Command line options */
    while ((clopt = getopt(argc, argv, "P:R:lndxrpfivh")) != -1)
    {
        switch (clopt)
        {
        case 'P':
            config.pattern_file_name = optarg;
            break;
        case 'R':
            config.w_mode = WORKING_MODE_REPLACE;
            config.output_dir = optarg;
            break;
        case 'l':
            config.lazy_replace = 1;
            break;
        case 'n':
            config.output_show_item = 1;
            break;
        case 'd':
            config.output_show_dpos = 1;
            break;
        case 'x':
            config.output_show_xpos = 1;
            break;
        case 'r':
            config.output_show_reprv = 1;
            break;
        case 'p':
            config.output_show_pattern = 1;
            break;
        case 'f':
            config.find_first = 1;
            break;
        case 'i':
            config.insensitive = 1;
            break;
        case 'v':
            config.verbosity = 1;
            break;
        case '?':
        case 'h':
        default:
            print_usage (argv[0]);
            exit(1);
        }
    }
    config.input_files = argv + optind;
    config.input_files_num = argc - optind;
    
    /* Correct and normalize the command-line options */
    
    if (config.pattern_file_name == NULL || config.input_files[0] == NULL)
    {
        print_usage (argv[0]);
        exit(1);
    }
    
    if (!(config.output_show_item || config.output_show_dpos ||
            config.output_show_xpos || config.output_show_reprv
            || config.output_show_pattern))
    {
        config.output_show_dpos = 1;
        config.output_show_pattern = 1;
    }
    
    if (config.lazy_replace && config.w_mode != WORKING_MODE_REPLACE)
    {
        fprintf (stderr, "Switch -l is not applicable. "
                "It operates in replace mode. Use switch -R\n");
        exit(1);
    }
    
    /* Show the configuration file */
    if(config.verbosity)
    {
        printf("Loading Patterns From '%s'\n", config.pattern_file_name);
    }
    
    /* Load patterns */
    if (pattern_load (config.pattern_file_name, &trie))
        exit(1);
    
    if(config.verbosity)
        printf("Total Patterns: %lu\n", trie->patterns_count);
    
    if (config.w_mode == WORKING_MODE_SEARCH)
    {
        if (trie->patterns_count == 0)
        {
            printf ("No pattern to search!\n");
            return 1;
        }
        
        /* Search */
        if (opendir(config.input_files[0])) /* if it is a directory */
        {
            if (config.verbosity)
                printf("Searching directory %s:\n", config.input_files[0]);
            walker_find (config.input_files[0], trie);
        } 
        else /* if it is not a directory */
        {
            if (config.verbosity)
                printf("Searching %ld files\n", config.input_files_num);
            
            for (i = 0; i < config.input_files_num; i++)
                search_file (config.input_files[i], trie);
        }
    }
    else if (config.w_mode == WORKING_MODE_REPLACE)
    {
        /* Replace Mode */
        if (trie->repdata.has_replacement == 0)
        {
            printf ("No pattern was specified for replacement "
                    "in the pattern file!\n");
            return 1;
        }
        
        for (i = 0; i < config.input_files_num; i++)
        {
            infpath = config.input_files[i];
            outfpath = get_outfile_name (config.output_dir, infpath);
            
            if (!replace_file (trie, infpath, outfpath))
                printf("Successfully replaced: %s >> %s\n", infpath, outfpath);
        }
    }
    
    /* Release */
    pattern_release ();
    ac_trie_release (trie);
    free (output_file_name);
    
    return 0;
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

int search_file (const char *filename, AC_TRIE_t *trie)
{
    int fd_input; /* Input file descriptor */
    static AC_TEXT_t intext; /* input text */
    static AC_ALPHABET_t in_stream_buffer[STREAM_BUFFER_SIZE];
    static struct match_param mparm; /* Match parameters */
    ssize_t num_read; /* Number of byes read from input file */
    int keep = 0;
    
    intext.astring = in_stream_buffer;
    
    /* Open input file */
    if (!strcmp(config.input_files[0], "-"))
    {
        fd_input = 0; /* read from stdin */
    }
    else if ((fd_input = open(filename, O_RDONLY|O_NONBLOCK)) == -1)
    {
        fprintf(stderr, "Cannot read from input file '%s'\n", filename);
        return -1;
    }
    
    /* Reset the parameter */
    mparm.item = 0;
    mparm.total_match = 0;
    mparm.fname = fd_input ? (char *)filename : NULL;
    
    /* loop to load and search the input file repeatedly, chunk by chunk */
    do
    {
        /* Read a chunk from input file */
        num_read = read (fd_input, 
                (void *)in_stream_buffer, STREAM_BUFFER_SIZE);
        
        if (num_read < 0)
        {
            fprintf(stderr, "Error while reading from '%s'\n", filename);
            return -1;
        }
        
        intext.length = num_read;

        /* Handle case sensitivity */
        if (config.insensitive)
            lower_case(in_stream_buffer, intext.length);

        /* Break loop if call-back function has done its work */
        if (ac_trie_search (trie, &intext, keep, match_handler, &mparm))
            break;
        
        keep = 1;
        
    } while (num_read == STREAM_BUFFER_SIZE);

    close (fd_input);

    return 0;
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

int replace_file (AC_TRIE_t *trie, const char *infile, const char *outfile)
{
    int fd_input; /* Input file descriptor */
    int fd_output; /* output file descriptor */
    static AC_TEXT_t intext; /* input text */
    static AC_ALPHABET_t in_stream_buffer[STREAM_BUFFER_SIZE];
    static struct match_param uparm; /* user parameters */
    ssize_t num_read; /* Number of byes read from input file */
    struct stat file_stat;
    MF_REPLACE_MODE_t rpmod = MF_REPLACE_MODE_DEFAULT;
    
    intext.astring = in_stream_buffer;

    /* Open input file */
    if (!strcmp(config.input_files[0], "-"))
    {
        fd_input = 0; /* read from stdin */
    }
    else if ((fd_input = open(infile, O_RDONLY|O_NONBLOCK)) == -1)
    {
        fprintf(stderr, "Cannot read from input file '%s'\n", infile);
        return -1;
    }
    
    if (fstat(fd_input, &file_stat))
    { 
        fprintf(stderr, "Cannot get file stat for '%s'\n", infile);
        close(fd_input);
        return -1; 
    }
    
    if (S_ISDIR(file_stat.st_mode))
    {
        fprintf(stderr, "Directories is not supported in replace mode: "
                "skipped '%s'\n", infile);
        close(fd_input);
        return -1;
    }
    
    /* Open output file */
    if (outfile)
    {
        if ((fd_output = open(outfile, 
                O_WRONLY|O_CREAT|O_TRUNC, file_stat.st_mode)) == -1)
        {
            fprintf(stderr, "Cannot open '%s' for writing\n", outfile);
            return -1;
        }
    }
    else
    {
        fd_output = 1; /* sent output to stdout */
    }
    
    /* Reset the parameter */
    uparm.item = 0;
    uparm.total_match = 0;
    uparm.fname = NULL; /* note used */
    uparm.out_file_d = fd_output;
    
    /* loop to load and search the input file repeatedly, chunk by chunk */
    do
    {
        /* Read a chunk from input file */
        num_read = read (fd_input, 
                (void *)in_stream_buffer, STREAM_BUFFER_SIZE);
        
        if (num_read < 0)
        {
            fprintf(stderr, "Error while reading from '%s'\n", infile);
            return -1;
        }
        
        if (num_read == 0)
            break;
        
        intext.length = num_read;

        /* Handle case sensitivity */
        if (config.insensitive)
            lower_case(in_stream_buffer, num_read);

        if (config.lazy_replace)
            rpmod = MF_REPLACE_MODE_LAZY;
        
        if (multifast_replace (trie, &intext, rpmod, 
                replace_listener, &uparm))
            /* Break loop if call-back function has done its work */
            break;
        
    } while (1);
    
    multifast_rep_flush (trie, 0);

    close (fd_input);
    close (fd_output);

    return 0;
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

void lower_case (char *s, size_t l)
{
    size_t i;
    for(i = 0; i < l; i++)
        s[i] = tolower(s[i]);
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

int mkpath(const char *path, mode_t mode)
{
    char *p = (char*)path;

    /* Do mkdir for each slash until end of string or error */
    while (*p != '\0')
    {
        /* Skip first character */
        p++;

        /* Find first slash or end */
        while(*p != '\0' && *p != '/') p++;

        /* Remember value from p */
        char v = *p;

        /* Write end of string at p */
        *p = '\0';

        /* Create folder from path to '\0' inserted at p */
        if(mkdir(path, mode) == -1 && errno != EEXIST)
        {
            *p = v;
            return -1;
        }

        /* Restore path to it's former glory */
        *p = v;
    }

    return 0;
}

#define PATH_BUFFER_LENGTH 1024

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

char *get_outfile_name (const char *dir, const char *inpath)
{
    static size_t bufsize = 0;
    size_t dirlen = 0, pathlen = 0;
    struct stat st;
    char *fname;
    
    __mode_t md = S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH; /* Default mode */
        
    if (dir == NULL || *dir == '\0' || !strcmp(dir, "-"))
        return NULL; /* Send to the standard output */
    
    if (inpath && *inpath=='\0')
        return NULL; /* unexpected: assert */
    
    /* Manage buffer allocation
     * the lifetime of the buffer equals to the program life time */
    if (output_file_name == NULL)
    {
        bufsize = PATH_BUFFER_LENGTH;
        output_file_name = (char *) malloc(bufsize * sizeof(char));
    }
    dirlen = strlen(dir);
    pathlen = dirlen + strlen(inpath) + 1;
    
    /* Grow memory if needed */
    if (bufsize < pathlen)
    {
        bufsize = ((pathlen / PATH_BUFFER_LENGTH) + 1) * PATH_BUFFER_LENGTH;
        output_file_name = realloc (output_file_name, bufsize);
    }
    
    *output_file_name = '\0';
    strcpy(output_file_name, dir);
    if (output_file_name[dirlen - 1] != '/') {
        output_file_name[dirlen++] = '/';
        output_file_name[dirlen] = '\0';
    }
    
    if ((fname = rindex (inpath, (int)'/')))
    {
        *fname = '\0';
        if (*inpath != '\0') {
            stat(inpath, &st);
            md = st.st_mode;
        }
        strcat (output_file_name, *inpath == '/' ? inpath + 1 : inpath);
    }
    
    mkpath (output_file_name, md);
    
    if (fname != NULL)
    {
        *fname = '/';
        strcat (output_file_name, fname);
    }
    else
    {
        strcat (output_file_name, *inpath == '/' ? inpath + 1 : inpath);
    }
    
    return output_file_name;
}

#define STRINGIFY(s) #s
#define XSTRINGIFY(s) STRINGIFY(s)

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

void print_usage (char *progname)
{
    printf("MultiFast v%s Usage:\n%s "
            "-P pattern_file [-R out_dir [-l] | -n[d|x]rpvfi] [-h] "
            "file1 [file2 ...]\n", 
            XSTRINGIFY(MF_VERSION_NUMBER), progname);
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

int match_handler (AC_MATCH_t *m, void *param)
{
    unsigned int j;
    struct match_param *mparm = (struct match_param *)param;
    
    for (j=0; j < m->size; j++)
    {
        /* if (mparm->item == 0) */
        if (mparm->fname)
            printf ("%s: ", mparm->fname);
        
        if (config.output_show_item)
            printf("#%ld ", ++mparm->item);
        
        if (config.output_show_dpos)
            printf("@%ld ", m->position - m->patterns[j].ptext.length + 1);
        
        if (config.output_show_xpos)
            printf("@%08X ", (unsigned int)
                    (m->position - m->patterns[j].ptext.length + 1));
        
        if (config.output_show_reprv)
            printf("%s ", m->patterns[j].id.u.stringy);
        
        if (config.output_show_pattern)
            pattern_print (&m->patterns[j]);
        
        printf("\n");
    }
    
    mparm->total_match += m->size;
    
    if (config.find_first)
        return 1; /* Find First Match */
    else
        return 0; /* Find all matches */
}

/******************************************************************************
 * FUNCTION
 *****************************************************************************/

void replace_listener (AC_TEXT_t *text, void *user)
{
    write (((struct match_param *)user)->out_file_d, 
            text->astring, text->length);
}
