#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "uthash/utarray.h"

#define SP4 "    "
#define SP8 "        "
#define SP12 "            "
#define SP16 "                "

char TermSrcFN[BUFSIZ] = "terminfo.src.latest";
char TermCustomFN[BUFSIZ] = "ignorelist.txt";

UT_array *SrcLines       = NULL;
UT_array *CustomLines    = NULL;
UT_array *EveryTerm      = NULL;
UT_array *TermHasDA1     = NULL;
UT_array *TermHasDID     = NULL;
UT_array *TermNotANSI    = NULL;

int
args(int argc, char *argv[])
{
    int  arg_process = 1;
    char want[64];
    char wantfrom[64];
    char lenwant = 63;

    memset(want, 0, 64);

    for ( int cx=1; cx < argc; cx++ ) {
        /* CHECK for stop processing        */
        if (   ( strlen("--") == strlen(argv[cx]) )
            && ( 0 == strncmp("--", argv[cx], strlen("--") + 1) ) )
        {
            arg_process = 0;
        }
        /* Processing Arguments */
        else if ( ( arg_process ) && ( '-' == argv[cx][0] ) ) {
            if (   ( 0 == strncmp("--input", argv[cx], strlen("--input^") ) )
                || ( 0 == strncmp("--file", argv[cx], strlen("--file^") ) )
                || ( 0 == strncmp("-i", argv[cx], strlen("-i^") ) ) )
            {
                wantfrom[strlen(wantfrom)] = (char)cx;
                strncat(want, "i", lenwant);
                lenwant--;
            }
            else if (
                ( 0 == strncmp("--custom", argv[cx], strlen("--custom^") ) )
             || ( 0 == strncmp("-c", argv[cx], strlen("-c^") ) )
            ) {
                wantfrom[strlen(wantfrom)] = (char)cx;
                strncat(want, "c", lenwant);
                lenwant--;
            }
            else
            {
                fprintf(stderr,
                    "ERROR: Spare command-line arg[%d]: '%s'\n",
                    cx, argv[cx]);
                exit(1);
            }
        }
        else /* Processing over */
        {
            int last = strlen(want);
            char need = '-';
            if ( last ) {
                need = want[0];
                int pad = 0;
                for ( int cx = 1; cx <= last; cx++ ) {
                    if ((char)0 == want[cx]) {
                        want[cx-1] = (char)0;
                        wantfrom[cx-1] = (char)0;
                        pad = 1;
                    }
                    else if ( pad ) {
                        want[cx-1] = (char)0;
                        wantfrom[cx-1] = (char)0;
                    } else {
                        want[cx-1] = want[cx];
                        wantfrom[cx-1] = wantfrom[cx];
                    }
                }
            }
            switch (need) {
                case 'i':
                    strncpy(TermSrcFN, argv[cx], BUFSIZ-1);
                    break;
                case 'c':
                    strncpy(TermCustomFN, argv[cx], BUFSIZ-1);
                    break;
                default:
                    fprintf(stderr,
                        "ERROR: Spare command-line arg[%d]: '%s'\n",
                        cx, argv[cx]);
                    fprintf(stderr, "ERROR: need %c\n", need);
                    exit(1);
                    break;
            }
        }
    }
    if (strlen(want)) {
        fprintf(stderr, "ERROR: %s without value.\n",
                argv[(int)wantfrom[0]]);
        exit(1);
    }

    struct stat *filestat = calloc(1, sizeof(struct stat));

    if ( stat( TermSrcFN, filestat ) ) {
        fprintf(stderr,
                "ERROR: Checking '%s': %s\n", TermSrcFN, strerror(errno));
        exit(1);
    }
    /* This is ~20% smaller than the current file. */
    if ( 920000 > filestat->st_size ) {
        fprintf(stderr,
                "ERROR: Checking '%s': Size (%lu) too small.\n",
                TermSrcFN, filestat->st_size);
        exit(1);
    }
    memset(filestat, 0, sizeof(struct stat));
    if ( stat( TermCustomFN, filestat ) ) {
        // Just remove the value.
        memset(TermCustomFN, 0, BUFSIZ);
    }
    free(filestat);

    return 0;
}

uint32_t
read_clean_src(char *fn, UT_array *dest)
{
    FILE *fh = NULL;
    uint32_t linecx = 0;

    if ( NULL == ( fh = fopen( fn, "r") ) ) {
        fprintf(stderr,
                "ERROR: Opening '%s': %s\n", TermSrcFN, strerror(errno));
        return 0;
    }

    char curline[BUFSIZ];
    char *for_ut = curline;
    memset(curline, 0, BUFSIZ);
    while ( 0 < fgets(curline, BUFSIZ-1, fh ) ) {
        int cx = 0;
        int onlywhitespace = 1;

        // Eliminate lines with only comments (#)
        for ( ; ( cx < strlen(curline) && cx < BUFSIZ-1); cx++ ) {
            if ( ( ' ' == curline[cx] ) || ( '\t' == curline[cx] ) ) {
                ;
            }
            else if ( '#' == curline[cx] ) {
                if ( 0 == cx ) {
                    curline[0] = (char)0;
                }
                else if ( onlywhitespace ) {
                    curline[0] = (char)0;
                }
                else if (
                        ( curline[cx-1] == ' ' ) 
                     || ( curline[cx-1] == '\t' ) 
                ) {
                    curline[cx] = (char)0;
                }
                goto CLEAN_READ_LINEEND;
            } else {
                onlywhitespace = 0;
            }
        }
CLEAN_READ_LINEEND:
        // Eliminate whitespace at the end of the lines
        //   ( even if before a comment )
        while (
               ( strlen(curline) )
            && (   ( ' '  == curline[strlen(curline)-1] )
                || ( '\t' == curline[strlen(curline)-1] )
                || ( '\r' == curline[strlen(curline)-1] )
                || ( '\n' == curline[strlen(curline)-1] )
               )
            )
        {
            curline[strlen(curline)-1] = (char)0;
        }
        if (0 != curline[0]) {
            linecx++;
            utarray_push_back(dest, &for_ut);
        }

        memset(curline, 0, BUFSIZ);
    }

    fclose(fh);
    return linecx;
}

char *
push_if_uniq(UT_array *target, char *entry)
{
    char **ut_line = NULL;
    char *ret = entry;
    while( (ut_line=(char**)utarray_next(target,ut_line)) ) {
        if (   ( strlen(*ut_line) == strlen(entry) )
            && ( 0 == strcmp(*ut_line, entry) )  )
        {
            ret = NULL;
            break;
        }
    }
    if ( ret ) {
        utarray_push_back(target, &entry);
    }
    return ret;
}

char *
get_termline(char *dest, int dsz, const char *line)
{
    char workline[BUFSIZ];
    char curtype[BUFSIZ];
    char *ret = NULL;
    memset(workline, 0, BUFSIZ);

    strncpy(workline, line, BUFSIZ-1);

    if (
            ( ( 'a' <= workline[0] ) && ( 'z' >= workline[0] ) )
        || ( ( 'A' <= workline[0] ) && ( 'Z' >= workline[0] ) )
        || ( ( '0' <= workline[0] ) && ( '9' >= workline[0] ) )
        )
    {
        // THIS LINE IS A TERMINAL DEFINITION...
        // New terminal definitions start at pos 0
        // fprintf(stderr, "RAW   term: [%s]\n", workline);
        memset(curtype, 0, BUFSIZ);
        strncpy(curtype, workline, BUFSIZ-1);
        char * dx = index(curtype, ',');
        if ( dx ) {
            // Anything PAST the any ',' is a different field.
            *dx = (char)0;
        }
        dx = rindex(curtype, '|');
        if ( dx ) {
            // Anything PAST the last '|' is a 
            // comment ABOUT the terminal
            *dx = (char)0;
        }

        if (dest && (strlen(curtype))) {
            int pad = 0;
            for (int cx = 0; cx < dsz; cx++) {
                if ( pad ) {
                    dest[cx] = (char)0;
                }
                else if ( curtype[cx] ) {
                    dest[cx] = curtype[cx];
                    dest[cx+1] = (char)0;
                } else {
                    pad = 1;
                    dest[cx] = (char)0;
                }
            }
            if ( strlen(dest) ) {
                ret = dest;
            }
        }
    }

    return ret;
}

int
start_func(const char *name) {
    FILE *fh = stdout;
    char line[BUFSIZ];
    memset(line, 0, BUFSIZ);

    line[0] = '/';
    for( int cx = 1; cx < 77; cx++ ) {
        line[cx] = '*';
    }
    fprintf(fh, "%s\n", line);
    fprintf(fh, " * check for `term` in embedded list.\n */\n");
    fprintf(fh, "\nint is_matchlist(const char * term,");
    fprintf(fh, " const char ** match);\n\n");
    fprintf(fh, "int\nis_not_%s( const char *term )\n{\n", name);
    fprintf(fh, "%sconst char *term_list[] = {\n", SP4);

    return 0;
}

int
end_func(void)
{
    FILE *fh = stdout;

    fprintf(fh, "        \"\\000\"\n");
    fprintf(fh, "    };\n");
    fprintf(fh, "    return is_matchlist( term, term_list );\n}\n");

    return 0;
}

int
create_notlist(void)
{
    char **everylist = NULL;
    char **DEClist   = NULL;
    int    notfound  = 1;

    while( (everylist=(char**)utarray_next(EveryTerm,everylist)) ) {
        notfound = 1;
        DEClist = NULL;
        while( (DEClist=(char**)utarray_next(TermHasDA1,DEClist)) ) {
            if (   ( strlen(*everylist) == strlen(*DEClist) )
                && ( 0 == strcmp(*everylist, *DEClist) )  )
            {
                notfound = 0;
                goto EVERY_BREAKOUT;
            }
        }
        DEClist = NULL;
        while( (DEClist=(char**)utarray_next(TermHasDID,DEClist)) ) {
            if (   ( strlen(*everylist) == strlen(*DEClist) )
                && ( 0 == strcmp(*everylist, *DEClist) )  )
            {
                notfound = 0;
                goto EVERY_BREAKOUT;
            }
        }
        DEClist = NULL;
EVERY_BREAKOUT:
        if (notfound) {
            utarray_push_back(TermNotANSI, everylist);
        }
    }
    return 0;
}

int
loop_find_type(UT_array *src, UT_array *target)
{
    int total_found  = 0;
    int reject_count = 0;
    int loop_count   = 0;
    int find_count   = 1;
    while (find_count) {
        char **ut_line = NULL;
        char workline[BUFSIZ];
        char curtype[BUFSIZ];
        char newsearch[BUFSIZ];
        char *for_ut = curtype;
        find_count = 0;

        while( (ut_line=(char**)utarray_next(src,ut_line)) ) {
            int found   = 0;
            char **list = NULL;
            memset(workline, 0, BUFSIZ);
            strncpy(workline, *ut_line, BUFSIZ-1);

            get_termline(curtype, BUFSIZ, workline);

            while( (list=(char**)utarray_next(target,list)) ) {
                memset(newsearch, 0, BUFSIZ);
                snprintf(newsearch, BUFSIZ-1, "use=%s,", *list);
                char *start = strstr(workline, newsearch);
                if ( start ) {
                    found = 1;
                    goto LOOP_FOUND;
                }
            }
LOOP_FOUND:
            if (found) {
                if ( push_if_uniq(target, for_ut) ) {
                    find_count++;
                    total_found++;
                } else {
                    reject_count++;
                }
            }
        } // end ** target while
        loop_count++;
    } // while ** finding new items
    fprintf(stderr, "DEBUG: recursive search took %d loops.\n", loop_count);
    fprintf(stderr, "DEBUG: recursive search caught %d duplicates.\n",
            reject_count);
    return( total_found++ );
}

int
find_def(UT_array *src, UT_array *dest, const char *seek, UT_array *alltype)
{
    char **ut_line = NULL;
    char workline[BUFSIZ];
    char curtype[BUFSIZ];
    char *for_ut = curtype;
    int   da1_cx = 0;

    while( (ut_line=(char**)utarray_next(src,ut_line)) ) {
        memset(workline, 0, BUFSIZ);
        strncpy(workline, *ut_line, BUFSIZ-1);

        if ( get_termline(curtype, BUFSIZ, workline) ) {
            if (alltype) {
                // fprintf(stderr, "Found term: [%s]\n", workline);
                utarray_push_back(alltype, &for_ut);
            }
        }

        char *start = strstr(workline, seek);
        if ( start ) {
            utarray_push_back(dest, &for_ut);
            da1_cx++;
        }
    }
    return da1_cx;
}
// find_def(UT_array *src, UT_array *dest, const char *seek, UT_array *alltype)

int
find_all_def(UT_array *src, UT_array *dest, const char *seek, UT_array *alltype)
{
    UT_array *scratch = NULL;
    utarray_new(scratch, &ut_str_icd);
    int pass1 = find_def(src, scratch, seek, alltype);
    fprintf(stderr, "DEBUG: Found %u definitions for '%s'\n", pass1, seek);
    utarray_concat(dest, scratch);
    utarray_free(scratch);
    int pass2 = loop_find_type(SrcLines, dest);
    fprintf(stderr, "DEBUG: Recursed %u 'use='ers for '%s'\n", pass2, seek);
    return( utarray_len(dest) );
}

int
find_DA1(UT_array *src)
{
    // find_all_def(UT_array *src, UT_array *dest, const char *seek, UT_array *alltype)
    return(
        find_all_def(
            src,
            TermHasDA1,
            "u9=\\E[c,",
            NULL
        )
    );
}

int
find_DID(UT_array *src)
{
    // find_all_def(UT_array *src, UT_array *dest, const char *seek, UT_array *alltype)
    return(
        find_all_def(
            src,
            TermHasDID,
            "u9=\\EZ,",
            EveryTerm
        )
    );
}

void
debug_dump_utarray(UT_array *src)
{
    char **list = NULL;
    while( (list=(char**)utarray_next(src,list)) ) {
        fprintf(stderr, "%s\n", *list );
    }
}

void
debug_dump_utarray_term(UT_array *src, const char *type)
{
    char **list = NULL;
    char *inpipe = NULL;
    while( (list=(char**)utarray_next(src,list)) ) {
        inpipe = index(*list, '|');
        if ( inpipe ) {
            char *start = *list;
            while ( inpipe ) {
                *inpipe = (char)0;
                inpipe++;
                fprintf(stderr, "Has U9 like %-9s: [%s] |\n", type, start );
                start = inpipe;
                inpipe = index(start, '|');
            }
            fprintf(stderr, "Has U9 like %-9s: [%s] -\n", type, start );
        } else {
            fprintf(stderr, "Has U9 like %-9s: [%s]\n", type, *list );
        }
    }
}

int
main(int argc, char *argv[])
{
    args(argc, argv);

    utarray_new(SrcLines, &ut_str_icd);

    uint32_t linecx = read_clean_src(TermSrcFN, SrcLines);
    fprintf(stderr, "DEBUG: Read %u lines from '%s'\n", linecx, TermSrcFN);

    // All DEC ID '\eZ'
    utarray_new(TermHasDID, &ut_str_icd);
    utarray_new(EveryTerm, &ut_str_icd);
    if (( linecx = find_DID(SrcLines) )) {
        fprintf(stderr, "DEBUG: Found %u definitions for u9=\\EZ\n", linecx);
        debug_dump_utarray_term(TermHasDID, "vt50");
    }

    utarray_new(TermHasDA1, &ut_str_icd);
    if (( linecx = find_DA1(SrcLines) )) {
        fprintf(stderr, "DEBUG: Found %u definitions for u9=\\E[c\n", linecx);
        // debug_dump_utarray_term(TermHasDA1, "'u9=\\E[c'");
    }
    utarray_free(SrcLines);

    utarray_new(TermNotANSI, &ut_str_icd);
    create_notlist();

    utarray_new(CustomLines, &ut_str_icd);
    if ( TermCustomFN[0] ) {
        linecx = read_clean_src(TermCustomFN, CustomLines);
        fprintf(stderr, "DEBUG: Read %u lines from '%s'\n",
                linecx, TermCustomFN);
        debug_dump_utarray(CustomLines);
    }

    start_func("not_ecma");
    char **list = NULL;
    char *inpipe = NULL;
    while( (list=(char**)utarray_next(TermNotANSI,list)) ) {
        inpipe = index(*list, '|');
        if ( inpipe ) {
            fprintf(stdout, "%s// Has aliases [%s].\n", SP16, *list);
            char *start = *list;
            while ( inpipe ) {
                *inpipe = (char)0;
                inpipe++;
                fprintf(stdout, "%s\"%s\",\n", SP8, start);
                start = inpipe;
                inpipe = index(start, '|');
            }
            fprintf(stdout, "%s\"%s\",  // last alias.\n", SP8, start);
        } else {
            fprintf(stdout, "%s\"%s\",\n", SP8, *list);
        }
    }
    list = NULL;
    if ( utarray_len(CustomLines) ) {
        fprintf(stdout, "%s// *******************\n", SP16);
        fprintf(stdout, "%s// User Supplied Additions from '%s'\n",
                SP16, TermCustomFN);
        fprintf(stdout, "%s// *******************\n", SP16);
        while( (list=(char**)utarray_next(CustomLines,list)) ) {
            fprintf(stdout, "%s\"%s\",\n", SP8, *list);
        }
    }
    
    utarray_free(CustomLines);

    utarray_free(EveryTerm);
    exit(0);
}


