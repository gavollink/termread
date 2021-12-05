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
    int background; /* VT102 background query */
    int getcolor;   /* VT102 color query */
    int termname;   /* VTxx terminal caps query */
    int term2da;
    int wantstat;
    int color_num;
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

void
prinhelp(void)
{
    FILE *out = stderr;
    if ( opt.wanthelp ) {
        out = stdout;
    }
    fprintf(out, "\n");
    fprintf(out, "%s [-t] [-b] [-d <nnnn>] [-h]\n", opt.argv0 );
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
    fprintf(out, "    -d <nnnn>\n");
    fprintf(out, "    --delay <nnnn>\n");
    fprintf(out, "              Milliseconds to wait for first reply.\n");
    fprintf(out, "              default: 1000 ( 1.0 seconds ).\n");
    fprintf(out, "\n");
    fprintf(out, "    -s\n");
    fprintf(out, "    --stats   Print stats info after read response.\n");
    fprintf(out, "\n");
    fprintf(out, "    -h\n");
    fprintf(out, "    --help    This help.\n");
    if ( opt.wanthelp ) {
        fprintf(out, "\n");
        fprintf(out, "env TERM='%s'\n", opt.envterm);
    }
}

int
args( int argc, char *argv[], struct sopt* opt )
{
    /* Default all options */
    memset( opt, 0, sizeof(struct sopt) );
    opt->envterm = getenv("TERM");
    opt->argv0   = argv[0];
    opt->term    = ttyname(STDIN_FILENO);

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
            opt->needhelp = 1;
            opt->wanthelp = 1;
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
            opt->background = 1;
        }
        else if (
            (      ( ( strlen("-t") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-t", argv[cx]) ) )
            ) || (
                   ( ( strlen("--term") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--term", argv[cx]) ) )
            ) )
        {
            opt->termname = 1;
        }
        else if (
            (      ( ( strlen("-2") <= strlen(argv[cx]) )
                && ( 0 == strcmp("-2", argv[cx]) ) )
            ) || (
                   ( ( strlen("--term2") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--term2", argv[cx]) ) )
            ) )
        {
            opt->term2da = 1;
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
            opt->wantstat = 1;
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
            if ( is_next( argc, cx ) ) {
                getcolor = atoi( argv[1 + cx] );
                if ( 0 <= getcolor ) {
                    opt->getcolor = 1;
                    opt->color_num = getcolor;
                    cx++;
                }
            }
            if ( ! getcolor ) {
                fprintf(stderr,
                    "Unable to read color num after option '%s'\n",
                    argv[cx]);
                opt->needhelp = 1;
            }
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
            if ( is_next( argc, cx ) ) {
                getdelay = atoi( argv[1 + cx] );
                if ( 0 < getdelay ) {
                    opt->delay = getdelay;
                    cx++;
                }
            }
            if ( ! getdelay ) {
                fprintf(stderr,
                    "Unable to read delay after option '%s'\n",
                    argv[cx]);
                opt->needhelp = 1;
            }
        }
        else if ( (
                   ( ( strlen("--var") <= strlen(argv[cx]) )
                && ( 0 == strcmp("--var", argv[cx]) ) )
            ) )
        {
            if ( is_next( argc, cx ) ) {
                opt->var = argv[++cx];
            } else {
                fprintf(stderr,
                    "Unable to read outvar after option '%s'\n",
                    argv[cx]);
                opt->needhelp = 1;
            }
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
                opt->term = argv[cx];
            } else {
                fprintf(stderr,
                    "Unable to read output term after option '%s'\n",
                    argv[cx]);
                opt->needhelp = 1;
            }
        }
        else {
            fprintf( stderr, "Unknown option %d: [%s]\n", cx, argv[cx] );
        }
    }
    if ( 1 < ( opt->termname + opt->background + opt->getcolor ) ) {
        fprintf( stderr, "Only 1 query per command.\n" );
        opt->needhelp = 1;
    }
    if ( NULL == opt->var ) {
        if ( opt->termname ) {
            opt->var = termname_var;
        }
        else if ( opt->term2da ) {
            opt->var = term2da_var;
        }
        else if ( opt->background ) {
            opt->var = background_var;
        }
        else if ( opt->getcolor ) {
            opt->var = getcolor_var;
        } else {
            opt->var = default_var;
        }
    }
    /* Set default delay */
    if ( 0 == opt->delay ) {
        opt->delay = 1000;      /* ~ 1000 milliseconds or 1.0 seconds */
    }
    if ( NULL == opt->envterm ) {
        opt->envterm = "xterm-256color";
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
        if ( ( 4 == strlen( opt.envterm ) ) 
            && (   ( 0 == strncmp( "vt5", opt.envterm, 3 )
                || ( 0 == strncmp( "vt6", opt.envterm, 3 ) ) )
            ) )
        {
            fprintf( stderr, "TERM env is '%s' (-t not supported)\n",
                opt.envterm );
            exit(1);
        }
        else if ( ( 2 <= strlen( opt.envterm ) ) 
            && (
                 ( 0 == strncmp( "vt", opt.envterm, 2 ) )
              || ( 0 == strncmp( "xt", opt.envterm, 2 ) )
              || ( 0 == strncmp( "pu", opt.envterm, 2 ) )
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
        ret = fprintf(fh, xt_term2da);
        fflush( fh );
    }
    else if ( 1 == opt.getcolor ) {
        ret = fprintf(fh, xt_colorreq, opt.color_num);
        fflush( fh );
    }
    else if ( 1 == opt.background ) {
        ret = fprintf(fh, xt_colorbg );
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
    size_t   in_ln          = 0;
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
            in_ln++;
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
main( int argc, char *argv[] )
{
    int bsz = 80;
    char *in;
    long int got = 0;

    args( argc, argv, &opt );

    in = calloc( bsz, 1 );
    if ( NULL == in ) {
        exit ( 1 );
    }
    if ( opt.needhelp ) {
        prinhelp();
        exit( 0 );
    }

    term_write();

    got = readInput(bsz, in);

    if ( got ) {
        printf( "%s='%s'; export %s; \n", opt.var, in, opt.var );
    }
    if ( opt.wantstat ) {
        printf("#### STATS \n" );
        printf("# length : %ld", got );
        if ( got ) {
            printf(", first_delay : %.3f s", ((float)opt.stat_d_first/2000) );
        }
        if ( opt.stat_d_inter ) {
            printf(", longest interchar delay : %.3f s",
                ((float)opt.stat_d_inter/2000) );
        }
        printf("\n" );
        // printf("# tty : %s\n", opt.term );
    }
}
