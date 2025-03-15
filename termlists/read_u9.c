#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "uthash/utarray.h"

char Filename[BUFSIZ] = "terminfo.src.latest";
UT_array *TermDefinesPDA;
UT_array *TermDefinesDA1;

int
args(int argc, char *argv[])
{
    int  arg_process = 1;
    char want[64];
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
            if ( ( 0 == strncmp("--file", argv[cx], strlen("--filex") ) )
                || ( 0 == strncmp("-f", argv[cx], strlen("-fx") ) ) )
            {
                strncat(want, "f", lenwant--);
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
                need = want[last-1];
                want[last-1] = (char)0;
            }
            switch (need) {
                case 'f':
                    strncpy(Filename, argv[cx], BUFSIZ-1);
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

    struct stat *filestat = calloc(1, sizeof(struct stat));

    if ( stat( Filename, filestat ) ) {
        fprintf(stderr,
                "ERROR: Checking '%s': %s\n", Filename, strerror(errno));
        exit(1);
    }
    /* This is ~20% smaller than the current file. */
    if ( 920000 > filestat->st_size ) {
        fprintf(stderr,
                "ERROR: Checking '%s': Size (%lu) too small.\n",
                Filename, filestat->st_size);
        exit(1);
    }
    free(filestat);

    return 0;
}

int
clean_line(char *curline)
{
    while ( ( ' ' == curline[strlen(curline)-1] )
        || ( '\t' == curline[strlen(curline)-1] )
        || ( '\r' == curline[strlen(curline)-1] )
        || ( '\n' == curline[strlen(curline)-1] )
        )
    {
        curline[strlen(curline)-1] = (char)0;
    }
    int cx = 0;
    int onlywhitespace = 1;
    for ( ; ( cx < strlen(curline) && cx < BUFSIZ-1); cx++ ) {
        if ( ( ' ' == curline[cx] ) || ( '\t' == curline[cx] ) ) {
            ;
        }
        else {
            onlywhitespace = 0;
        }
        if ( '#' == curline[cx] ) {
            if ( onlywhitespace ) {
                curline[0] = (char)0;
            } else {
                curline[cx] = (char)0;
            }
            break;
        }
    }
    return 0;
}

int
seek_u9(char *fn)
{
    FILE *fh = NULL;

    if ( NULL == ( fh = fopen( fn, "r") ) ) {
        fprintf(stderr,
                "ERROR: Opening '%s': %s\n", Filename, strerror(errno));
        exit(1);
    }

    char curline[BUFSIZ];
    char workline[BUFSIZ];
    char curtype[BUFSIZ];
    memset(curline, 0, BUFSIZ);

    while ( 0 < fgets(curline, BUFSIZ-1, fh ) ) {
        clean_line(curline);
        if ( (char)0 == curline[0] ) {
            goto READY_NEXT_LINE;
        }
        memset(workline, 0, BUFSIZ);
        strncpy(workline, curline, BUFSIZ-1);

        if (
                ( ( 'a' <= workline[0] ) && ( 'z' >= workline[0] ) )
            || ( ( 'A' <= workline[0] ) && ( 'Z' >= workline[0] ) )
            || ( ( '0' <= workline[0] ) && ( '9' >= workline[0] ) )
            )
        {
            // THIS LINE IS A TERMINAL DEFINITION...
            // New terminal definitions start at pos 0
            memset(curtype, 0, BUFSIZ);
            // fprintf(stderr, "RAW   term: [%s]\n", workline);
            char * dx = rindex(workline, '|');
            if ( dx ) {
                // Anything PAST the last '|' is a 
                // comment ABOUT the terminal
                *dx = (char)0;
            }
            // fprintf(stderr, "Found term: [%s]\n", workline);
            strncpy(curtype, workline, BUFSIZ-1);
            goto READY_NEXT_LINE;
        }
        char *start = strstr(workline, "u9=\\E[c,");
        char *forut = curtype;
        if ( start ) {
            utarray_push_back(TermDefinesDA1, &forut);
            goto READY_NEXT_LINE;
        }
        start = strstr(workline, "u9=\\EZ,");
        if ( start ) {
            utarray_push_back(TermDefinesPDA, &forut);
            goto READY_NEXT_LINE;
        }

        char **list = NULL;
        char newsearch[BUFSIZ];
        int found = 0;
        while( (list=(char**)utarray_next(TermDefinesDA1,list)) ) {
            sprintf(newsearch, "use=%s,", *list);
            char *start = strstr(workline, newsearch);
            if ( start ) {
                found = 1;
                break;
            }
        }
        if (found) {
            utarray_push_back(TermDefinesDA1, &forut);
            found = 0;
            goto READY_NEXT_LINE;
        }
        list = NULL;
        while( (list=(char**)utarray_next(TermDefinesPDA,list)) ) {
            sprintf(newsearch, "use=%s,", *list);
            char *start = strstr(workline, newsearch);
            if ( start ) {
                found = 1;
            }
        }
        if (found) {
            utarray_push_back(TermDefinesPDA, &forut);
            found = 0;
            goto READY_NEXT_LINE;
        }
READY_NEXT_LINE:
        memset(curline, 0, BUFSIZ);
    }

    fclose(fh);
    return 0;
}

int
main(int argc, char *argv[])
{
    args(argc, argv);

    utarray_new(TermDefinesPDA, &ut_str_icd);
    utarray_new(TermDefinesDA1, &ut_str_icd);

    seek_u9(Filename);

    char **list = NULL;
    char *inpipe = NULL;
    while( (list=(char**)utarray_next(TermDefinesPDA,list)) ) {
        inpipe = index(*list, '|');
        if ( inpipe ) {
            char *start = *list;
            while ( inpipe ) {
                *inpipe = (char)0;
                inpipe++;
                fprintf(stdout, "Has U9 like vt50 : [%s] |\n", start );
                start = inpipe;
                inpipe = index(start, '|');
            }
            fprintf(stdout, "Has U9 like vt50 : [%s] -\n", start );
        } else {
            fprintf(stdout, "Has U9 like vt50 : [%s]\n", *list );
        }
    }
    list = NULL;
    while( (list=(char**)utarray_next(TermDefinesDA1,list)) ) {
        inpipe = index(*list, '|');
        if ( inpipe ) {
            char *start = *list;
            while ( inpipe ) {
                *inpipe = (char)0;
                inpipe++;
                fprintf(stdout, "Has U9 like vt102: [%s] |\n", start );
                start = inpipe;
                inpipe = index(start, '|');
            }
            fprintf(stdout, "Has U9 like vt102: [%s] -\n", start );
        } else {
            fprintf(stdout, "Has U9 like vt102: [%s]\n", *list );
        }
    }

    utarray_free(TermDefinesDA1);
    utarray_free(TermDefinesPDA);

    exit(0);
}
