/****************************************************************************
 * termread.c
 *
 * Author: Gary Allen Vollink
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int is_next( int argc, int cx );

static struct termios orig_term, new_term;

struct sopt {
    int needhelp;   /* Help is needed */
    int wanthelp;   /* Help is explicitly requested */
    int background; /* VT100 background query */
    int getcolor;   /* VT100 color query */
    int termname;   /* VT100 + terminal caps query */
    int term2da;
    int ignoreterm; /* Ignore $TERM */
    int wantstat;
    int color_num;
    int debug;
    long int delay;
    long int stat_d_first;
    long int stat_d_inter;
    long int stat_d_final;
    size_t bufsz;
    FILE * termfh;
    char * var;
    char * term;
    char * buf;
    char * envterm;
    char * argv0;
} opt;

char background_var[] = "TERM_BG";
char termname_var[] = "TERMID";
char term2da_var[] = "TERM2DA";
char getcolor_var[] = "COLOR";
char default_var[] = "OUT";

const char vt_termreq[] = "\033Z";
const char xt_termreq[] = "\033[c\005";
const char xt_term2da[] = "\033[>c";
const char xt_colorbg[] = "\033]11;?\033\\";
const char xt_colorreq[] = "\033]4;%d;?\007";
const char xt_eraseline[] = "\033[9D\033[2K";

#define DEBUG(str, ... ) if ( opt.debug ) \
    { \
        char *b = calloc(1024, 1); \
        sprintf( b, "# DEBUG: %s", str );\
        fprintf( stderr, b, __VA_ARGS__ ); \
        free(b); \
    };

void
prinusage(void)
{
    FILE *out = stderr;
    if ( opt.wanthelp ) {
        out = stdout;
    }
    fprintf(out, "\n");
    fprintf(out, "%s [!] [-t] [-2] [-b] [-c <nnn>] [-d <nnnn>] [-s]\n",
                 opt.argv0 );
    fprintf(out, "%s --help\n",
                 opt.argv0 );

    return;
}

void
prinhelp(void)
{
    FILE *out = stderr;
    if ( opt.wanthelp ) {
        out = stdout;
    }
    fprintf(out, "\n");
    fprintf(out, "  ACTIONS:\n");
    fprintf(out, "\n");
    fprintf(out, "    -t\n");
    fprintf(out, "    --term    Ask for terminal ident 'primary DA'\n");
    fprintf(out, "\n");
    fprintf(out, "    -2\n");
    fprintf(out, "    --term2   Ask for terminal ident 'secondary DA'\n");
    fprintf(out, "\n");
    fprintf(out, "    -b\n");
    fprintf(out, "    --bg      Ask for terminal background color\n");
    fprintf(out, "\n");
    fprintf(out, "    -c <nnn>\n");
    fprintf(out, "    --color <nnn>\n");
    fprintf(out, "              Ask for the RGB of a color by number.\n");
    fprintf(out, "\n");
    fprintf(out, "  OPTIONS:\n");
    fprintf(out, "\n");
    fprintf(out, "    !         Ignore TERM env, asks as if xterm.\n");
    fprintf(out, "\n");
    fprintf(out, "    -d <nnnn>\n");
    fprintf(out, "    --delay <nnnn>\n");
    fprintf(out, "              Milliseconds to wait for first reply.\n");
    fprintf(out, "              default: 500 ( 0.5 seconds ).\n");
    fprintf(out, "\n");
    fprintf(out, "    --var <name>\n");
    fprintf(out, "              Variable name for shell readable output.\n");
    fprintf(out, "              Only used for first output if multiple.\n");
    fprintf(out, "\n");
    fprintf(out, "    -s\n");
    fprintf(out, "    --stats   Print stats info after read response.\n");
    fprintf(out, "\n");
    fprintf(out, "    -v\n");
    fprintf(out, "    --verbose Extra output.\n");
    fprintf(out, "\n");
    fprintf(out, "    -h\n");
    fprintf(out, "    --help    This help.\n");
    if ( opt.debug ) {
        fprintf(out, "\n");
        fprintf(out, "    -o </dev/tt...>\n");
        fprintf(out, "    --tty   </dev/tt...>\n");
        fprintf(out, "              Open this instead of current term.\n");
        fprintf(out, "              Probably won't work.\n");
    }
    if ( opt.wanthelp ) {
        fprintf(out, "\n");
        fprintf(out, "Effective TERM='%s'\n", opt.envterm);
    }

    return;
}

int
args( int argc, char *argv[] )
{
    int action_requested = 0;

    /* Default all options */
    struct sopt* memopt = &opt;
    memset( memopt, 0, sizeof(struct sopt) );
    opt.envterm = getenv("TERM");
    opt.argv0   = argv[0];
    opt.term    = ttyname(STDIN_FILENO);

    /* Walk the commandline */
    for ( int cx=1; cx < argc; cx++ ) {
        if (
            (      ( ( strlen("-h") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-h", argv[cx]) ) )
            ) || (
                   ( ( strlen("--help") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--help", argv[cx]) ) )
            ) )
        {
            opt.wanthelp = 1;
            action_requested++;
            DEBUG("--help ACTION requested.\n", NULL);
        }
        else if ( 0 == strcmp("!", argv[cx] ) ) {
            opt.ignoreterm = 1;
            opt.envterm = "xterm";
            DEBUG("! forcing TERM actions for [%s]\n", opt.envterm);
        }
        else if (
            (      ( ( strlen("-b") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-b", argv[cx]) ) )
            ) || (
                   ( ( strlen("--bg") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--bg", argv[cx]) ) )
            ) || (
                   ( ( strlen("--background") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--background", argv[cx]) ) )
            ) )
        {
            opt.background = 1;
            action_requested++;
            DEBUG("--bg ACTION requested.\n", NULL);
        }
        else if (
            (      ( ( strlen("-t") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-t", argv[cx]) ) )
            ) || (
                   ( ( strlen("--term") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--term", argv[cx]) ) )
            ) )
        {
            opt.termname = 1;
            action_requested++;
            DEBUG("--term ACTION requested.\n", NULL);
        }
        else if (
            (      ( ( strlen("-2") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-2", argv[cx]) ) )
            ) || (
                   ( ( strlen("--term2") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--term2", argv[cx]) ) )
            ) )
        {
            opt.term2da = 1;
            action_requested++;
            DEBUG("--term2 ACTION requested.\n", NULL);
        }
        else if (
            (      ( ( strlen("-s") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-s", argv[cx]) ) )
            ) || (
                   ( ( strlen("--stat") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--stat", argv[cx]) ) )
            ) || (
                   ( ( strlen("--stats") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--stats", argv[cx]) ) )
            ) )
        {
            opt.wantstat = 1;
            DEBUG("--stats requested.\n", NULL);
        }
        else if (
            (      ( ( strlen("-c") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-c", argv[cx]) ) )
            ) || (
                   ( ( strlen("--color") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--color", argv[cx]) ) )
            ) || (
                   ( ( strlen("--colour") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--colour", argv[cx]) ) )
            ) )
        {
            int getcolor = 0;
            int gc_ok    = 0;
            if ( is_next( argc, cx ) ) {
                char *endptr = NULL;
                getcolor = strtol( argv[1 + cx], &endptr, 10 );
                if (   ( endptr != argv[1 + cx] )
                    && ( endptr - argv[1 + cx] == strlen(argv[1+cx]) )  )
                {
                    gc_ok = 1;
                    opt.getcolor = 1;
                    opt.color_num = getcolor;
                    cx++;
                    DEBUG("--color [%i] ACTION requested.\n", opt.color_num);
                }
            }
            if ( ! gc_ok ) {
                fprintf(stderr,
                    "Unable to read color num after option '%s'\n",
                    argv[cx]);
                opt.needhelp = 1;
            }
            action_requested++;
        }
        else if (
            (      ( ( strlen("-d") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-d", argv[cx]) ) )
            ) || (
                   ( ( strlen("--delay") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--delay", argv[cx]) ) )
            ) )
        {
            int getdelay = 0;
            int gd_ok    = 0;
            if ( is_next( argc, cx ) ) {
                char *endptr = NULL;
                getdelay = strtol( argv[1 + cx], &endptr, 10 );
                if (   ( endptr != argv[1 + cx] )
                    && ( endptr - argv[1 + cx] == strlen(argv[1+cx]) )  )
                {
                    if ( 0 >= getdelay ) {
                        fprintf(stderr,
                            "Not a valid delay '%s' for option '%s'\n",
                            argv[cx+1], argv[cx]);
                        opt.needhelp = 1;
                    } else {
                        opt.delay = getdelay;
                        DEBUG("--delay [%li].\n", opt.delay);
                    }
                    cx++;
                    gd_ok = 1; /* Even if not, error already handled! */
                }
            }
            if ( ! gd_ok ) {
                fprintf(stderr,
                    "Unable to read delay after option '%s'\n",
                    argv[cx]);
                opt.needhelp = 1;
            }
        }
        else if ( (
                   ( ( strlen("--var") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--var", argv[cx]) ) )
            ) )
        {
            if ( is_next( argc, cx ) ) {
                opt.var = argv[++cx];
                DEBUG("--var OVERRIDE [%s].\n", opt.var);
            } else {
                fprintf(stderr,
                    "Unable to read outvar after option '%s'\n",
                    argv[cx]);
                opt.needhelp = 1;
            }
        }
        else if (
            (      ( ( strlen("-v") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-v", argv[cx]) ) )
            ) || (
                   ( ( strlen("--ve") <= strlen(argv[cx]) )
                && ( 0 == strncmp("--ve", argv[cx], strlen("--ve")) ) )
            ) )
        {
            opt.debug = 1;
            DEBUG("--verbose DEBUGGING ON.\n", NULL);
        }
        else if (
            (      ( ( strlen("-o") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-o", argv[cx]) ) )
            ) || (
                   ( ( strlen("--tty") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--tty", argv[cx]) ) )
            ) )
        {
            if ( is_next( argc, cx ) ) {
                cx++;
                opt.term = argv[cx];
                DEBUG("OPEN OVERRIDE [%s] (probably won't work).\n", opt.term);
            } else {
                fprintf(stderr,
                    "Unable to read output term after option '%s'\n",
                    argv[cx]);
                opt.needhelp = 1;
            }
        }
        else {
            fprintf( stderr, "Unknown option %d: [%s]\n", cx, argv[cx] );
            opt.needhelp = 1;
        }
    }
    if ( NULL == opt.envterm )
    {
        opt.envterm = "xterm";
        DEBUG("TERM empty, forcing actions for [%s]\n", opt.envterm);
    }
    if ( opt.needhelp || opt.wanthelp ) {
        // SIDE EFFECT -- NO DEBUG WARNING ABOUT --var BELOW.
        action_requested = 1;
    }
    /* Set default delay */
    if ( 0 == opt.delay ) {
        opt.delay = 500;      /* ~ 500 milliseconds or 0.5 seconds */
        DEBUG("--delay defaulting to [%li]\n", opt.delay);
    }
    /* Warn about no action */
    if ( 0 == action_requested ) {
        fprintf( stderr, "No ACTION requested.\n");
        opt.needhelp = 1;
    }
    else if ( ( opt.debug )
            && ( opt.var)
            && ( 1 < action_requested )
            )
    {
        if ( opt.termname ) {
            DEBUG("--var [%s] will only be used for --term\n", opt.var );
        }
        else if ( opt.term2da ) {
            DEBUG("--var [%s] will only be used for --term2\n", opt.var );
        }
        else if ( opt.getcolor ) {
            DEBUG("--var [%s] will only be used for --color\n", opt.var );
        }
        else if ( opt.background ) {
            DEBUG("--var [%s] will only be used for --bg %i\n", opt.color_num );
        }
    }
    return 1;
}

void
initTermios(int echo)
{
    fcntl(0, F_SETFL, O_NONBLOCK);
    tcgetattr(0, &orig_term);      /* grab original terminal i/o settings */
    new_term = orig_term;  /* make new settings same as original settings */
    new_term.c_lflag &= ~ICANON;                  /* disable buffered i/o */
    new_term.c_lflag &= echo ? ECHO : ~ECHO;             /* set echo mode */
    int ret = tcsetattr(0, TCSANOW, &new_term);
    if ( -1 == ret ) {
        fprintf(stderr, "Unable to set terminal attributes: %s\n",
            strerror(errno) );
    }
}

/* Restore original terminal i/o settings */
void
resetTermios(void)
{
    int ret = tcsetattr(0, TCSANOW, &orig_term);
    if ( -1 == ret ) {
        fprintf(stderr, "Unable to reset terminal attributes: %s\n",
            strerror(errno) );
    }
    ret = fcntl(0, F_SETFL, 0);
    if ( -1 == ret ) {
        fprintf(stderr, "Unable to reset terminal NBLOCK status: %s\n",
            strerror(errno) );
    }
}

int
term_cleanline()
{
    FILE * fh = opt.termfh;
    if ( ! opt.termfh ) {
        int fp = open( opt.term, O_WRONLY|O_NOCTTY );
        if ( 0 <= fp ) {
            fh = fdopen( fp, "w" );
            if ( fh ) {
                opt.termfh = fh;
            } else {
                fprintf( stderr, "Unable to open '%s': %s\n",
                    opt.term, strerror(errno) );
                return 0;
            }
        } else {
            fprintf( stderr, "Unable to open '%s': %s\n",
                opt.term, strerror(errno) );
            return 0;
        }
    }

    int ret = 0;
    ret = fprintf(fh, xt_eraseline );
    fflush( fh );
    return ret;
}

int
term_write()
{
    FILE * fh = opt.termfh;
    if ( ! opt.termfh ) {
        int fp = open( opt.term, O_WRONLY|O_NOCTTY );
        if ( 0 <= fp ) {
            fh = fdopen( fp, "w" );
            if ( fh ) {
                opt.termfh = fh;
            } else {
                fprintf( stderr, "Unable to open '%s': %s\n",
                    opt.term, strerror(errno) );
                return 0;
            }
        } else {
            fprintf( stderr, "Unable to open '%s': %s\n",
                opt.term, strerror(errno) );
            return 0;
        }
    }

    int ret = 0;
    if ( 1 == opt.termname ) {
        opt.termname = 0;
        if ( NULL == opt.var ) {
            opt.var = termname_var;
            DEBUG("Set default --term var to %s\n", opt.var );
        }

        if ( ( 4 == strlen( opt.envterm ) )
            && (   ( 0 == strncmp( "vt5", opt.envterm, 3 )
                || ( 0 == strncmp( "vt6", opt.envterm, 3 ) ) )
            ) )
        {
            fprintf( stderr, "TERM env is '%s' (-t not supported)\n",
                opt.envterm );
            exit(1);
        }
        else if ( ( 3 <= strlen( opt.envterm ) )
            && (
                 ( 0 == strncmp( "vt", opt.envterm, 2 ) )
              || ( 0 == strncmp( "xt", opt.envterm, 2 ) )
              || ( 0 == strncmp( "pu", opt.envterm, 2 ) )
              || ( 0 == strncmp( "co", opt.envterm, 2 ) )
              || ( 0 == strncmp( "nst", opt.envterm, 3 ) )
              || ( 0 == strncmp( "kon", opt.envterm, 3 ) )
            )   )
        {
            ret = fprintf(fh, xt_termreq );
        } else {
            fprintf( stderr, "TERM env is '%s' (-t not supported)\n",
                opt.envterm );
            exit(1);
        }
        fflush( fh );
    }
    else if ( 1 == opt.term2da ) {
        opt.term2da = 0;
        if ( NULL == opt.var ) {
            opt.var = term2da_var;
            DEBUG("Set default --term2 var to %s\n", opt.var );
        }

        ret = fprintf(fh, xt_term2da);
        fflush( fh );
    }
    else if ( 1 == opt.getcolor ) {
        opt.getcolor = 0;
        if ( NULL == opt.var ) {
            opt.var = getcolor_var;
            DEBUG("Set default --color var to %s\n", opt.var );
        }

        ret = fprintf(fh, xt_colorreq, opt.color_num);
        fflush( fh );
    }
    else if ( 1 == opt.background ) {
        opt.background = 0;
        if ( NULL == opt.var ) {
            opt.var = background_var;
            DEBUG("Set default --bg var to %s\n", opt.var );
        }

        ret = fprintf(fh, xt_colorbg );
        fflush( fh );
    }
    else {
        ret = fprintf(fh, xt_eraseline );
        fflush( fh );
    }
    return ret;
}

int
is_next( int argc, int cx )
{
    if ( 1 + cx >= argc ) {
        return 0;
    } else {
        return 1;
    }
}

int
readInput( int bufsz, char * buf )
{
    if ( 0 == bufsz ) {
        if ( opt.bufsz ) {
            bufsz = opt.bufsz;
        }
        else {
            bufsz = 80;
            if ( buf ) {
                free( buf );
                buf = opt.buf;
            }
        }
    }
    if ( NULL == buf ) {
        if ( opt.buf ) {
            buf = opt.buf;
        } else {
            buf = calloc( bufsz, 1 );
            if ( NULL == buf ) {
                fprintf(stderr, "%s\n", strerror(errno) );
                exit ( 1 );
            }
        }
    } else {
        memset( buf, 0, bufsz );
    }
    opt.bufsz = bufsz;
    opt.buf = buf;
    char     c;
    size_t   bufln          = 0;
    int      first          = 0;
    long int wait_for_first = ( 2 * opt.delay );
    /* Translates to ~ 10 milliseconds or 0.01 seconds ...                */
    long int wait_for_more  = 20;
    long int cur_wait       = wait_for_first;
    long int delay          = 0;
    int      ret            = 0;

    initTermios(0);
    while (cur_wait > delay) {
        ret = read(0, &c, 1);
        if ( 1 == ret ) {
            if ( 0 == first ) {
                first = 1;
                opt.stat_d_first = delay;
                cur_wait = wait_for_more;
            } else if ( delay > opt.stat_d_inter ) {
                opt.stat_d_inter = delay;
            }
            delay = 0;
            if ( 0x20 > c ) {
                if ( 3 == c ) {
                    // ETX: End of Text
                    break;
                }
                else if ( 4 == c ) {
                    // EOT: End of Transmission
                    break;
                }
                else if ( 10 == c ) {
                    // LF: Line Feed
                    break;
                }
                else if ( 16 == c ) {
                    // DLE: Data Link Escape
                    break;
                }
                else if ( 23 == c ) {
                    // ETB: End Transmission Block
                    break;
                }
                else {
                    char oct_c[10];
                    snprintf(oct_c, 10, "\\0%o", c);
                    for ( int cx = 0; cx < strlen(oct_c); cx++ ) {
                        buf[bufln++] = oct_c[cx];
                    }
                }
            } else {
                buf[bufln++] = c;
                c = 0;
            }
            if ( bufsz <= (bufln + 1) ) {
                break;
            }
        } else {
            delay++;
            usleep(500);          /* 0.000 500 of a second */
            // nanosleep(500000); /* 0.000 500 000 of a second */
        }
    }
    resetTermios();
    opt.stat_d_final = delay;
    return ( bufln );
}

int
do_term()
{
    int bsz = 1024;
    char *in;
    long int got = 0;

    in = calloc( bsz, 1 );
    if ( NULL == in ) {
        exit ( 1 );
    }

    while ( opt.termname + opt.background + opt.getcolor + opt.term2da ) {
        term_write();

        got = readInput(bsz, in);

        term_cleanline();

        if ( got ) {
            printf( "%s='%s'; export %s; \n", opt.var, in, opt.var );
        }
        opt.var = NULL;

        if ( opt.wantstat ) {
            printf("#### STATS \n" );
            printf("# length : %ld", got );
            if ( got ) {
                printf(", first_delay : %.3f s",
                        ((float)opt.stat_d_first/2000) );
            }
            if ( opt.stat_d_inter ) {
                printf(", longest interchar delay : %.3f s",
                    ((float)opt.stat_d_inter/2000) );
            }
            printf("\n" );
            if ( opt.debug ) {
                printf("# tty : %s\n", opt.term );
            }
        }
    }

    return 0;
}

int
main( int argc, char *argv[] )
{
    args( argc, argv );

    if ( opt.wanthelp ) {
        prinhelp();
        exit( 0 );
    }
    if ( opt.needhelp ) {
        prinusage();
        exit( 1 );
    }

    do_term();

    fclose(opt.termfh);

    exit(0);
}
