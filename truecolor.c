/****************************************************************************
 * truecolor.c
 *
 * Output terminal codes for truecolor in the 2 styles...
 *
 * FOREGROUND:
 * wezterm-direct: "\033[38:2::%d:%d:%dm", red, green, blue
 *   xterm-direct: "\033[38;2;%d;%d;%dm", red, green, blue
 *
 * FOREGROUND:
 * wezterm-direct: "\033[48:2::%d:%d:%dm", red, green, blue
 *   xterm-direct: "\033[48;2;%d;%d;%dm", red, green, blue
 *
 * CURSOR POSITION:
 * "\033[6n" should return "\033[[%d;%dR"
 *
 *    TODO   TODO   TODO
 * -t -- term env override needs doco
 * Try NOT using the terminal open for the prints
 *
 * LICENSE: Embedded at bottom...
 */
#define VERSION "0.2.0"
#define C_YEARS "2024"
#define IDENT "truecolor"

#define AT "@"
#define DOT "."
#define AUTHOR1 "Gary Allen Vollink"
#define CONTACT1 "gary" AT "vollink" DOT "com"
#define AUTHORS "   " AUTHOR1 " <" CONTACT1 ">\n"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

const char xt_eraseline[] = "\033[9D\033[2K";

static struct termios orig_term, new_term;

struct tc_options_s {
    long int delay;
    long int stat_d_first;
    long int stat_d_inter;
    long int stat_d_final;
    int      tests;
    int      verbosity;
    int      wantlicense;
    int      wanthelp;
    int      needhelp;
    int      fg_set;
    int      bg_set;
    int      std_out;
    int      no_input;
    int      reset;
    int      text_output;
    int      mode;
    int      red[2];
    int      green[2];
    int      blue[2];
    char   **printables;
    FILE    *termfh;
    char    *tty;
    char    *env_term;
    char    *env_tm_truemode;
    char    *env_debug_entry;
    char    *me;
};

struct tc_options_s *Opts;

void printlicense();
int  my_exit(int);
int  term_close();
int  term_write( const char * );

void
dohelp( struct tc_options_s *opts )
{
    FILE *out = stderr;
    if ( opts->wanthelp ) {
        out = stdout;
    }
    fprintf(out, IDENT " Version " VERSION "\n\n" );
    fprintf(out, "Copyright (c) " C_YEARS ", by\n" AUTHORS "\n" );

    fprintf(out, "Interprets a true color value from command line,\n" );
    fprintf(out, "and prints the true color control codes.\n" );
    fprintf(out, "Short options can be bundled.\n" );
    fprintf(out, "\n" );
    fprintf(out, "ENVIRONMENT VARIABLES:\n" );
    fprintf(out, "\n" );
    fprintf(out, "    TERM=\n" );
    fprintf(out, "        True color support is differnt between xterm\n" );
    fprintf(out, "        and nsterm, so kitty, wezterm and ms-terminal\n" );
    fprintf(out, "        values here will set output to the nsterm\n" );
    fprintf(out, "        format.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    _TM_TRUEMODE=\n" );
    fprintf(out, "        If set to \"c*\", output will be in colon format.\n" );
    fprintf(out, "        Default: semicolon format.\n");
    fprintf(out, "\n" );
    fprintf(out, "    %s=%d\n", Opts->env_debug_entry, Opts->verbosity );
    fprintf(out, "        If set, output verbosity will be set.\n" );
    fprintf(out, "        The env var is based on the name of this exec.\n" );
    fprintf(out, "\n" );
    fprintf(out, "OPTIONS:\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --\n" );
    fprintf(out, "        No more options, only arguments.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --help | -h\n" );
    fprintf(out, "        This help screen.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --license | -L\n" );
    fprintf(out, "        Print this software's license.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --verbose | -v\n" );
    fprintf(out, "        More output (turns on debug).\n" );
    fprintf(out, "\n" );
    fprintf(out, "    -q\n" );
    fprintf(out, "        Less output.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --mode | -m\n" );
    fprintf(out,
        "        Terminal's support truecolor via: colon or semicolon.\n");
    fprintf(out, "        If not set, checks $_TM_TRUEMODE.\n");
    fprintf(out, "        If neither are set, tries to guess based on $TERM\n");
    fprintf(out, "        Default: semicolon\n");
    fprintf(out, "\n" );
    fprintf(out, "    --stdout\n" );
    fprintf(out,
        "        Instead of opening the current TTY, output direct to stdout\n"
    );
    fprintf(out, "\n" );
    fprintf(out, "    --reset | -s\n" );
    fprintf(out, "        Terminal `ansi` reset will print at the end.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    -i\n" );
    fprintf(out, "        Toggle between foreground and background for next\n");
    fprintf(out, "            supplied color.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --foreground | -fg <nnnnnnnn>\n" );
    fprintf(out, "        Use truecolor to set foreground.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --background | -bg <nnnnnnnn>\n" );
    fprintf(out, "        Use truecolor to set background.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --color | -c <nnnnnnnn>\n" );
    fprintf(out, "        Combined RGB color\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --red | -r <nnn>\n" );
    fprintf(out, "        Red channel of color\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --green | -g <nnn>\n" );
    fprintf(out, "        Green channel of color\n" );
    fprintf(out, "\n" );
    fprintf(out, "    --blue | -b <nnn>\n" );
    fprintf(out, "        Blue channel of color\n" );
    fprintf(out, "\n" );
    fprintf(out, "    AFTER all option arguments have been found, any extra\n");
    fprintf(out, "    arguments will be printed by the program.\n" );
    fprintf(out, "\n" );
    fprintf(out, "TIPS:\n" );
    fprintf(out, "\n" );
    fprintf(out, "    All arguments to options (colors, color channels) can\n");
    fprintf(out, "    be interpreted as:\n");
    fprintf(out, "      Binary (start with 0b or 0B),\n" );
    fprintf(out, "      Octal (start with 0o, 0O, or 0),\n" );
    fprintf(out, "      Hexidecimal (start with 0x, x, or #)\n");
    fprintf(out, "      Base 10 is default (can be forced 0i or 0I).\n");
    fprintf(out, "\n" );
    fprintf(out, "    It seems convenient to use '#', but remember it is\n" );
    fprintf(out, "    a comment character so needs to be escaped or quoted.\n");
    fprintf(out, "    or the shell will eat it.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    Flag options can be negative.\n" );
    fprintf(out, "    -s sets reset on; +s sets reset off.\n" );
    fprintf(out, "    --reset sets reset on; --no-reset turns it off.\n" );
    fprintf(out, "    Yes, the 'meaning' of + and - are swapped.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    +s-igrb forces reset off, turns on background, and\n");
    fprintf(out, "    expects green, red, and blue values (in that order).\n");
    fprintf(out, "\n" );
    fprintf(out, "    If no options are present, the first argument will\n" );
    fprintf(out, "    be taken as a full color value.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    If a color argument has been satisfied, any other\n" );
    fprintf(out, "    arguments will be printed 'echo -e' style.\n" );
    fprintf(out, "\n" );
    fprintf(out, "    To capture a TRUECOLOR escape code via pipe, use\n" );
    fprintf(out, "    the --stdout option (this turns off tty checks).\n" );
    fprintf(out, "\n" );
}

void
dousage( struct tc_options_s *opts )
{
    FILE *out = stderr;
    if ( opts->wanthelp ) {
        out = stdout;
    }
    fprintf(out, "%s [-i] [-s] [-c <nnnnnn>] [-r <nn>] [-g <nn>] [-b <nn>]\n",
          opts->me );
    fprintf(out, "\tOR\n" );
    fprintf(out, "%s --help\n",
          opts->me );
}

int
my_exit(int val)
{
    if ( Opts ) {
        if ( Opts->termfh ) {
            if ( Opts->reset ) {
                term_write( "\033[0m" );
            }
            if ( Opts->text_output ) {
                term_write( "\n" );
            }
            fflush(Opts->termfh);
            term_close();
        }
        /* Free memory! */
        if ( Opts->printables ) {
            free( Opts->printables );
        }
        if ( Opts->env_debug_entry ) {
            free( Opts->env_debug_entry );
        }
        free( Opts );
    }
    exit(val);
}

void
fatal( char *str, ... )
{
    va_list vlist;
    fprintf(stderr, "# FATAL: ");
    va_start(vlist, str);
    vfprintf(stderr, str, vlist );
    va_end(vlist);
    my_exit(9);
}

void
error( char *str, ... )
{
    if (   ( str )
        && ( strlen( str ) )
        && ( Opts )
        && ( Opts->verbosity )
        && ( -1 <= Opts->verbosity ) )
    {
        va_list vlist;
        fprintf(stderr, "# ERROR: ");
        va_start(vlist, str);
        vfprintf(stderr, str, vlist );
        va_end(vlist);
    }
}

void
warning( char *str, ... )
{
    if (   ( str )
        && ( strlen( str ) )
        && ( Opts )
        && ( Opts->verbosity )
        && ( 0 <= Opts->verbosity ) )
    {
        va_list vlist;
        fprintf(stderr, "# WARN: ");
        va_start(vlist, str);
        vfprintf(stderr, str, vlist );
        va_end(vlist);
    }
}

void
info( char *str, ... )
{
    if (   ( str )
        && ( strlen( str ) )
        && ( Opts )
        && ( Opts->verbosity )
        && ( 1 <= Opts->verbosity ) )
    {
        va_list vlist;
        fprintf(stderr, "# INFO: ");
        va_start(vlist, str);
        vfprintf(stderr, str, vlist );
        va_end(vlist);
    }
}

void
debug( char *str, ... )
{
    if (   ( str )
        && ( strlen( str ) )
        && ( Opts )
        && ( Opts->verbosity )
        && ( 2 <= Opts->verbosity ) )
    {
        va_list vlist;
        fprintf(stderr, "# DEBUG: ");
        va_start(vlist, str);
        vfprintf(stderr, str, vlist );
        va_end(vlist);
    }
}

void
vdebug( char *str, ... )
{
    if (   ( str )
        && ( strlen( str ) )
        && ( Opts )
        && ( Opts->verbosity )
        && ( 3 <= Opts->verbosity ) )
    {
        va_list vlist;
        fprintf(stderr, "# VDEBUG: ");
        va_start(vlist, str);
        vfprintf(stderr, str, vlist );
        va_end(vlist);
    }
}

int
term_open()
{
    FILE * fh = Opts->termfh;
    if ( ! Opts->termfh ) {
        if ( Opts->std_out ) {
            vdebug("term_open() using stdout.\n");
            Opts->termfh = stdout;
        }
        else {
            int fp = open( Opts->tty, O_WRONLY|O_NOCTTY );
            if ( 0 <= fp ) {
                fh = fdopen( fp, "w" );
                if ( fh ) {
                    Opts->termfh = fh;
                } else {
                    fprintf( stderr, "Unable to open '%s': %s\n",
                        Opts->tty, strerror(errno) );
                    my_exit(1);
                }
            } else {
                fprintf( stderr, "Unable to open '%s': %s\n",
                    Opts->tty, strerror(errno) );
                my_exit(1);
            }
        }
    }
    if ( ( Opts->termfh ) && ( ( Opts->tty ) || ( Opts->std_out ) ) ) {
        return 1;
    }
    return my_exit(1);
}

int
term_write( const char * out )
{
    FILE * fh;
    if (term_open()) {
        fh = Opts->termfh;
    } else {
        return 0;
    }

    int ret = 0;
    ret = fprintf(fh, "%s", out );
    fflush( fh );
    return ret;
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
        fatal( "Unable to set terminal attributes: %s\n", strerror(errno) );
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
term_close()
{
    int chk = 0;
    if ( Opts->termfh ) {
        if ( stdout != Opts->termfh) {
            chk = fclose(Opts->termfh);
        }
        Opts->termfh = NULL;
    }
    if ( 0 == chk ) {
        return 1;
    }
    return 0;
}


int
readInput( int bufsz, char *buf )
{
    char     c;
    size_t   bufln          = 0;
    int      first          = 0;
    long int wait_for_first = ( 2 * Opts->delay );
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
                Opts->stat_d_first = delay;
                cur_wait = wait_for_more;
            } else if ( delay > Opts->stat_d_inter ) {
                Opts->stat_d_inter = delay;
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
    Opts->stat_d_final = delay;
    return ( bufln );
}

/***************************************
 * This function is VERY specific to doing matches on
 * command line options.
 * haystack should be the option, like "--help"
 * needle should be an argv[count].
 * min is the minimum number of characters acceptable as a
 * match.
 *
 * ( "--help", {"--he"}, 3 ) will match successful returning 4
 * ( "--help", {"--he="}, 3 ) will also match returning 5
 * ( "--help", {"--help="}, 3 ) will also match returning 7
 * ( "--help", {"--hef"}, 3 ) will fail completely returning 0
 * ( "--help", {"--helpq"}, 3 ) will fail completely returning 0
 *
 * For all matches,
 * byte of argv[count][returned_value]
 * either points to the end-of-string or the first byte after the =.
 */
int
_countmatch( const char *haystack, const char *needle, int min )
{
    int ret = 0;
    int max = strlen( haystack );
    if ( ( !min ) || ( min > max ) ) {
        min = max;
    }
    int cx;
    for ( cx = 0; ( haystack[cx] && needle[cx] ); cx++ ) {
        if ( haystack[cx] == needle[cx] ) {
            ret++;
        } else {
            break;
        }
    }
    if ( ( 0 == haystack[ret] ) && ( 0 == needle[ret] ) ) {
        return ret;
    }
    else if ( ( ret >= min ) && ( 0 == needle[ret] ) ) {
        return ret;
    }
    else if ( ( ret >= min ) && ( '=' == needle[ret] ) ) {
        return ret + 1;
    }
    if ( 2 < Opts->verbosity ) {
        vdebug( "_countmatch(): "
            "Failed on '%s' to match '%s', @ count=%d (%c == %c)\n",
            needle, haystack, ret,
            needle[cx], haystack[cx]
            );
    }
    return 0;
}

int
_means_yes( const char *thing, int null_is_yes )
{
    switch ( thing[0] ) {
        case 'y':       // yes
        case 'Y':       // Yes
        case 'a':       // affirmative
        case 'A':       // Affirmative
        case '1':       // Traditional binary yes
        case '2':       // Non-zero numbers count
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return 1;
            break;
        case 0:         // Not char '0', but the null string-end byte
            if ( null_is_yes ) {
                return 1;
            }
            break;
    }
    return 0;
}

size_t
_is_bg_opt( const char *needle )
{
    size_t argindex = 0;
    if ( ( argindex = _countmatch( "--bg", needle, 3 ) ) ) {
        return argindex;
    }
    else if ( ( argindex = _countmatch( "--background", needle, 3 ) ) ) {
        return argindex;
    }
    return 0;
}

size_t
_is_no_bg_opt( const char *needle )
{
    size_t argindex = 0;
    if ( ( argindex = _countmatch( "--no-bg", needle, 6 ) ) ) {
        if ( ! _means_yes(needle+argindex, 1) ) {
            return argindex;
        }
    }
    else if ( ( argindex = _countmatch( "--no-background", needle, 6 ) ) ) {
        if ( ! _means_yes(needle+argindex, 1) ) {
            return argindex;
        }
    }
    return 0;
}

size_t
_is_fg_opt( const char *needle )
{
    size_t argindex = 0;
    if ( ( argindex = _countmatch( "--fg", needle, 3 ) ) ) {
        return argindex;
    }
    else if ( ( argindex = _countmatch( "--foreground", needle, 3 ) ) ) {
        return argindex;
    }
    return 0;
}

size_t
_is_no_fg_opt( const char *needle )
{
    size_t argindex = 0;
    if ( ( argindex = _countmatch( "--no-fg", needle, 6 ) ) ) {
        if ( ! _means_yes(needle+argindex, 1) ) {
            return argindex;
        }
    }
    else if ( ( argindex = _countmatch( "--no-foreground", needle, 6 ) ) ) {
        if ( ! _means_yes(needle+argindex, 1) ) {
            return argindex;
        }
    }
    return 0;
}

int
hextobin ( const unsigned char h )
{
    switch ( h )
    { /* No break, everything returns */
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
    }
    return 0;
}

long
_read_value_l( const char *in, long *out, char **end )
{
    int known_base  = 0;
    int seen_number = 0;
    int explicit    = 0;
    int jump = 0;
    for ( int cx = 0; in[cx]; cx++ ) {
        switch( in[cx] ) {
            case '#':
                if ( 0 == cx ) {
                    explicit   = 1;
                    known_base = 16;
                    jump       = 1;
                }
                break;
            case '0':
                if ( 0 == cx ) {
                    // Implied
                    known_base = 8;
                }
                seen_number = 1;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                seen_number = 1;
                break;
            case '8':
            case '9':
                if ( ( ! explicit ) && ( 8 >= known_base ) ) {
                    known_base = 10;
                }
                seen_number = 1;
                break;
            case 'a':
            case 'c':
            case 'e':
            case 'f':
            case 'A':
            case 'C':
            case 'E':
            case 'F':
                if ( ( ! explicit ) && ( 10 >= known_base ) ) {
                    known_base  = 16;
                    seen_number = 1;
                } else if ( 16 >= known_base ) {
                    seen_number = 1;
                }
                break;
            case 'b':
            case 'B':
                if ( ( 1 == cx ) && ( '0' == in[0] ) ) {
                    explicit    = 1;
                    known_base  = 2;
                    jump        = 2;
                    seen_number = 0;
                }
                else if ( ( ! explicit ) && ( 10 >= known_base ) ) {
                    known_base  = 16;
                    seen_number = 1;
                } else if ( 16 >= known_base ) {
                    seen_number = 1;
                }
                break;
            case 'd':
            case 'D':
                if ( ( 1 == cx ) && ( '0' == in[0] ) ) {
                    explicit    = 1;
                    known_base  = 10;
                    jump        = 2;
                    seen_number = 0;
                }
                else if ( ( ! explicit ) && ( 10 >= known_base ) ) {
                    known_base  = 16;
                    seen_number = 1;
                } else if ( 16 >= known_base ) {
                    seen_number = 1;
                }
                break;
            case 'o':
            case 'O':
                if ( 0 == cx ) {
                    explicit   = 1;
                    known_base = 8;
                    jump       = 1;
                }
                else if ( ( 1 == cx ) && ( '0' == in[0] ) ) {
                    explicit    = 1;
                    known_base  = 8;
                    jump        = 2;
                    seen_number = 0;
                }
                break;
            case 'i':
            case 'I':
            case 't':
            case 'T':
                if ( 0 == cx ) {
                    explicit   = 1;
                    known_base = 10;
                    jump = 1;
                }
                else if ( ( 1 == cx ) && ( '0' == in[0] ) ) {
                    explicit    = 1;
                    known_base  = 10;
                    jump        = 2;
                    seen_number = 0;
                }
                break;
            case 'x':
            case 'X':
                if ( 0 == cx ) {
                    known_base = 16;
                    jump       = 1;
                }
                if ( ( 1 == cx ) && ( '0' == in[0] ) ) {
                    seen_number = 0;
                    known_base  = 16;
                    jump        = 2;
                }
                break;
        }
    }
    if (seen_number) {
        long l = strtol( in + jump, end, known_base );
        if ( out ) {
            out[0] = l;
        }
        return l;
    }
    return 0;
}

int
_mincmp( const char *haystack, const char *needle, int min ) {
    int max = strlen( haystack );
    if ( ( !min ) || ( min > max ) ) {
        min = max;
    }
    int res = strncmp( haystack, needle, min );
    if ( !res ) {
        return res;
    }
    if ( min != max ) {
        res = strncmp( haystack, needle, max );
    }
    if ( !res ) {
        return res;
    }
    if ( 0 == haystack[max+1] ) {
        return res;
    }
    else if ( '=' == haystack[max+1] ) {
        return res;
    } else {
        return -1;
    }
}

int
c_shift( char *in )
{
    int ret = in[0];
    if ( ret ) {
        for ( int dx = 0; (( dx < BUFSIZ-1 ) && ( in[dx]) ); dx++ ) {
            if (in[dx+1]) {
                in[dx] = in[dx+1];
            } else {
                in[dx] = 0;
            }
        }
    }
    return ret;
}

int
c_push( char *in, int val )
{
    int cx;
    for ( cx = 0; (( cx < BUFSIZ-1 ) && ( in[cx]) ); cx++ );
    if ( cx < BUFSIZ-1 ) {
        in[cx] = (char)val;
        in[cx+1] = 0;
        return val;
    }
    return 0;
}

int
c_poppeek( char *in )
{
    int cx;
    for ( cx = 0; (( cx < BUFSIZ-1 ) && ( in[cx]) ); cx++ );
    if ( in[cx-1] ) {
        return in[cx-1];
    }
    return 0;
}

int
c_last_skip( char *in, int val )
{
    char *idx = rindex( in, (char)val );
    int   ret = *idx;
    if ( idx ) {
        *idx = 'k';
        return ret;
    }
    return 0;
}

int
doprint( char* out )
{
    int buildval = 0;
    int retval = 0;
    FILE * fh;
    if (term_open()) {
        fh = Opts->termfh;
    } else {
        return 0;
    }
    for ( int cx = 0; cx < strlen( out ); cx++ ) {
        if ( 1 ) {
            if ( '\\' == out[cx] ) {
                if ( 0 == strncmp ( "\\\\", &out[cx], 2 ) ) {
                    putc('\\', fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\a", &out[cx], 2 ) ) { // BEL
                    putc(7, fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\b", &out[cx], 2 ) ) { // BS
                    putc(8, fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\c", &out[cx], 2 ) ) {
                    /* TO match `bash` built-in, I'm leaving this out...
                    if ( 0 == ( opts & NO_NEWLINE ) ) {
                        putc('\n', fh);
                    } **** */
                    my_exit(0);
                }
#ifndef MAC_BROKEN_BASH
                else if ( 0 == strncmp ( "\\e", &out[cx], 2 ) ) { // ESC
                    /* Escape Character */
                    putc( 0x1b, fh);
                    retval++;
                    cx++;
                }
#elif defined DEBUG
else if ( 0 == strncmp ( "\\e", &out[cx], 2 ) ) { // ESC
    fprintf( stderr, "MAC_BROKEN_BASH defined behavior, '\\e' does nothing.\n");
}
#endif
                else if ( 0 == strncmp ( "\\f", &out[cx], 2 ) ) { // FF
                    putc( 0x0c, fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\n", &out[cx], 2 ) ) { // NL
                    putc( 0x0a, fh);
                    Opts->text_output = 0;
                    retval = 0;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\r", &out[cx], 2 ) ) { // CR
                    putc( 0x0d, fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\t", &out[cx], 2 ) ) { // HT
                    putc( '\t', fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\v", &out[cx], 2 ) ) { // VT
                    putc( 0x0b, fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == strncmp ( "\\x", &out[cx], 2 ) ) { // HEX
                    if ( isxdigit( out[cx+2] ) ) {
                        if ( isxdigit( out[cx+3] ) ) {
                            buildval = 16 * hextobin( out[cx+2] );
#ifdef DEBUG
    fprintf( stderr, "HEX BUILD pos 1: '%c', Now: %d \\x%02x \\0%03o\n",
            out[cx+2], buildval, buildval, buildval );
#endif
                            buildval = buildval + hextobin( out[cx+3] );
#ifdef DEBUG
    fprintf( stderr, "HEX BUILD pos 2: '%c', Final: %d \\x%02x \\0%03o\n",
            out[cx+2], buildval, buildval, buildval );
#endif
                            cx = cx + 3;
                        } else {
                            buildval = hextobin( out[cx+2] );
#ifdef DEBUG
    fprintf( stderr, "HEX BUILD pos 1: '%c', Final: %d \\x%02x \\0%03o\n",
            out[cx+2], buildval, buildval, buildval );
#endif
                            cx = cx + 2;
                        }
                        if ( buildval ) {
                            putc( buildval, fh);
                            retval++;
                        }
                        buildval = 0;
                    } else {
#ifdef DEBUG
    fprintf( stderr, "HEX ABORT on 'no hexadecimal digit'.\n");
#endif
                        putc( '\\', fh);
                        retval++;
                    }
                }
                else if ( 0 == strncmp ( "\\0", &out[cx], 2 ) ) { // OCTAL
                    if ( '0' <= out[cx+2] && '7' >= out[cx+2] ) {
                        cx++;
                        for ( int dx = 0; dx < 3; dx++ ) {
                            if ( '0' <= out[cx+1] && '7' >= out[cx+1] ) {
                                buildval =
                                    (buildval * 8) + hextobin( out[cx+1] );
#ifdef DEBUG
    fprintf( stderr, "OCTAL BUILD pos %d: '%c', Now: %d \\x%02x \\0%03o\n",
            dx, out[cx+1], buildval, buildval, buildval );
#endif
                                cx++;
                            } else {
#ifdef DEBUG
    fprintf( stderr, "OCTAL STOP pos %d: '%c', Final: %d \\x%02x \\0%03o\n",
            dx, out[cx+1], buildval, buildval, buildval );
#endif
                                dx=3;
                            }
                        }
                        if ( buildval ) {
                            putc( buildval, fh);
                            retval++;
                        }
                        buildval = 0;
                    } else {
#ifdef DEBUG
    fprintf( stderr, "OCTAL ABORT on 'no octal digit'.\n");
#endif
                        putc( '\\', fh);
                        retval++;
                    }
                } else {
                    putc( '\\', fh);
                    retval++;
                }
            } else {
                putc( out[cx], fh);
                retval++;
            }
        } else {
            putc( out[cx], fh);
            retval++;
        }
    }
    return retval;
}

int
read_color_i( char *in, int *out )
{
    if ( NULL == out ) {
        return 1;
    }
    if ( NULL == in ) {
        return 1;
    }
    if ( 0 == in[0] ) {
        return 1;
    }
    long number = -1;
    _read_value_l( in, &number, NULL );
    if ( -1 == number ) {
        return 0;
    }
    if ( out ) {
        out[0] = number;
    }
    return 0;
}

int
read_color_l( char *in, long *out )
{
    if ( NULL == out ) {
        return 1;
    }
    if ( NULL == in ) {
        return 1;
    }
    if ( 0 == in[0] ) {
        return 1;
    }
    long number = -1;
    _read_value_l( in, &number, NULL );
    if ( -1 == number ) {
        return 0;
    }
    if ( out ) {
        out[0] = number;
    }
    return 0;
}

int
split_color_i(
        struct tc_options_s *opts,
        char *in,
        int *r,
        int *g,
        int *b
        )
{
    if ( NULL == in ) {
        return 1;
    }
    long color = -1;
    _read_value_l( in, &color, NULL );
    if ( -1 == color ) {
        fprintf(stderr,
                "Color input [%s] number interpretation failed.\n", in );
        return 1;
    } else {
        if ( r ) {
            r[0] = (( color & 0xFF0000 ) >> 16);
        }
        if ( g ) {
            g[0] = (( color & 0xFF00 ) >> 8 );
        }
        if ( b ) {
            b[0] = (( color & 0xFF ));
        }
        info(
            "Color %ld (0x%06X): "
            "red %d (0x%02X), green %d (0x%02X), blue %d (0x%02X)\t[%s]\n",
            color, color,
            r[0], r[0],
            g[0], g[0],
            b[0], b[0], in   );
        return 0;
    }
    return 1;
}

/***************************************
 * Aggressively seek TTY NAME
 *
 * Command line `tty` only checks STDIN_FILENO and gives up, this checks all.
 */
int
get_tty( char **store_tty )
{
    int which = -1;
    int try[4] = { STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO, 0 };
    for ( int cx = 0; ((NULL==*store_tty)&&(cx < 3)); cx++ ) {
        *store_tty = ttyname( try[cx] );
        if ( *store_tty ) {
            which = try[cx];
            break;
        }
    }
    if ( NULL == *store_tty ) {
        info( "get_tty(): Not a TTY.\n" );
        Opts->std_out = 1;
        Opts->no_input = 1;
        return 1;
    } else {
        if ( 0 != which ) {
            Opts->no_input = 1;
        }
        debug( "TTY '%s' Found at FILENO %d\n", *store_tty, which );
    }
    return 0;
}

struct tc_options_s *
getOpts( char **argv, char **env )
{
    /* First, allocate my options storage */
    struct tc_options_s *opts = calloc(1, sizeof(struct tc_options_s));
    if ( NULL == opts ) {
        fatal( "Out of memory. %s:%d\n", __FILE__, __LINE__-2 );
    }
    Opts = opts;

    opts->printables = calloc(1, BUFSIZ);
    if ( NULL == opts->printables ) {
        fatal( "Out of memory. %s:%d\n", __FILE__, __LINE__-2 );
    }

    /* opt->me is going to be my first "option" set */
    char *self = NULL;

    if ( strlen( argv[0] ) ) {
        char *slash = rindex( argv[0], '/' );
        if ( slash ) {
            self = slash + 1;
        } else {
            self = argv[0];
        }
    }
    if ( ! strlen( self ) ) {
        self = IDENT;
    }

    char wantoption[BUFSIZ];
    memset( wantoption, 0, BUFSIZ );

    /* COPY SELF to use as base to search for {ME}_DEBUG */
    strncpy( wantoption, self, BUFSIZ-1 );
    opts->me = self;
    for ( int cx = 0; wantoption[cx] != 0; cx++ ) {
        if ( ( 'a' <= wantoption[cx] ) && ( 'z' >= wantoption[cx] ) ) {
            // This is naive, but I don't care?
            wantoption[cx] = wantoption[cx] - 0x20;
        }
    }

    strncpy( wantoption + strlen(wantoption), "_DEBUG", BUFSIZ - strlen(wantoption) );
    char *savewant = calloc( 1, 2+strlen(wantoption) );
    if (NULL == savewant ) {
        fatal( "Out of memory. %s:%d\n", __FILE__, __LINE__-2 );
    }
    strcpy( savewant, wantoption );
    opts->env_debug_entry = savewant;

    int argseek = (1 | 2 | 4);
    int argindex = 0;

    for ( int cx = 0; NULL != env[cx]; cx++ )
    {
        if ( ( argindex = _countmatch( wantoption, env[cx], 0 ) ) ) {
            argseek = ( argseek & ( argseek ^ 1 ) );
            if ( env[cx][argindex] ) {
                int test = _read_value_l( env[cx]+argindex, NULL, NULL );
                if ( test ) {
                    opts->verbosity += test;
                    debug( "envloop: %s verbosity set to %d.\n", env[cx], opts->verbosity);
                }
                else if ( _means_yes( env[cx]+argindex, 1 ) ) {
                    opts->verbosity += 1;
                    debug( "envloop: %s verbosity set to %d.\n", env[cx], opts->verbosity);
                }
            }
            debug( "argseek = 0x%04X\n", argseek );
        }
        else if ( ( argindex = _countmatch( "TERM", env[cx], 0 ) ) ) {
            argseek = ( argseek & ( argseek ^ 4 ) );
            if ( env[cx][argindex] ) {
                opts->env_term = env[cx]+argindex;
                debug("envloop: TERM '%s'\n", opts->env_term );
            }
            debug( "argseek = 0x%04X\n", argseek );
        }
        else if ( ( argindex = _countmatch( "_TM_TRUEMODE", env[cx], 0 ) ) ) {
            argseek = ( argseek & ( argseek ^ 4 ) );
            if ( env[cx][argindex] ) {
                opts->env_tm_truemode = env[cx]+argindex;
                debug("envloop: _TM_TRUEMODE '%s'\n", opts->env_tm_truemode );
            }
            debug( "argseek = 0x%04X\n", argseek );
        }
        if ( ! argseek ) {
            debug("envloop: break found all envs.\n" );
            break;
        }
    }

    //opts->tty   = ttyname(STDOUT_FILENO);
    if ( get_tty(&opts->tty) ) {
        if ( opts->tests ) {
            error( "Not on tty, cannot run tests.\n" );
            opts->tests = 0;
        }
    }
    opts->delay = 500;
    int fg_bg_idx = 0;

    /* From here, wantoption is an array of argument types for options. */
    memset( wantoption, 0, BUFSIZ );
    argindex = 0;
    argseek = 1;
    for ( int cx = 1; NULL != argv[cx]; cx++ )
    {
        if ( argseek && ( 0 == strncmp( argv[cx], "--", 3 ) ) ) {
            argseek = 0;
        }
        else if ( argseek && ( 0 == strncmp( argv[cx], "--", 2 ) ) ) {
            if ( ( argindex = _countmatch( "--help", argv[cx], 3 ) ) ) {
                opts->wanthelp = _means_yes(argv[cx]+argindex, 1);
                debug( "Help option '%s' set to %d\n",
                        argv[cx], opts->wanthelp );
            }
            else
            if ( ( argindex = _countmatch( "--no-help", argv[cx], 6 ) ) ) {
                opts->wanthelp = ( ! _means_yes(argv[cx]+argindex, 1) );
                debug( "Help option '%s' set to %d\n",
                        argv[cx], opts->wanthelp );
            }
            else
            if ( ( argindex = _countmatch( "--license", argv[cx], 6 ) ) ) {
                opts->wantlicense = ( _means_yes(argv[cx]+argindex, 1) );
                debug( "License option '%s' set to %d\n",
                        argv[cx], opts->wantlicense );
            }
            else
            if ( ( argindex = _countmatch( "--no-license", argv[cx], 6 ) ) ) {
                opts->wantlicense = ( ! _means_yes(argv[cx]+argindex, 1) );
                debug( "License option '%s' set to %d\n",
                        argv[cx], opts->wantlicense );
            }
            else
            if ( ( argindex = _countmatch( "--stdout", argv[cx], 4 ) ) ) {
                opts->std_out = ( _means_yes(argv[cx]+argindex, 1) );
                debug( "Standard Out option '%s' set to %d\n",
                        argv[cx], opts->std_out );
            }
            else
            if ( ( argindex = _countmatch( "--no-stdout", argv[cx], 7 ) ) ) {
                opts->std_out = ( ! _means_yes(argv[cx]+argindex, 1) );
                debug( "Standard Out option '%s' set to %d\n",
                        argv[cx], opts->std_out );
            }
            else
            if ( ( argindex = _is_fg_opt( argv[cx] ) ) ) {
                fg_bg_idx = 0;
                if ( argv[cx][argindex] ) {
                    split_color_i( opts, argv[cx]+argindex,
                            &opts->red[0],
                            &opts->green[0],
                            &opts->blue[0] );
                    debug( "Foreground option '%s' set from [%s]\n",
                        argv[cx], argv[cx]+argindex );
                } else {
                    vdebug( "Foreground check 'c' against '%c'.\n",
                            c_poppeek( wantoption ) );
                    if ( 'c' != c_poppeek( wantoption ) ) {
                        strncat( wantoption, "x", BUFSIZ-1 );
                        debug( "Foreground option '%s' waits for argument.\n",
                            argv[cx] );
                    } else {
                        debug( "Foreground option '%s' redundant.\n",
                            argv[cx] );
                    }
                }
            }
            else
            if ( ( argindex = _is_no_fg_opt( argv[cx] ) ) ) {
                fg_bg_idx      = 1;
                opts->fg_set   = 0;
                opts->red[0]   = 0;
                opts->green[0] = 0;
                opts->blue[0]  = 0;
                if ( c_last_skip( wantoption, 'c' ) ) {
                    debug( "NO Foreground option '%s', previous fg reset.\n",
                        argv[cx] );
                } else {
                    debug( "NO Foreground option '%s' reset.\n",
                        argv[cx] );
                }
            }
            else
            if ( ( argindex = _is_bg_opt( argv[cx] ) ) ) {
                fg_bg_idx = 1;
                if ( argv[cx][argindex] ) {
                    split_color_i( opts, argv[cx]+argindex,
                            &opts->red[1],
                            &opts->green[1],
                            &opts->blue[1] );
                    opts->bg_set = 1;
                } else {
                    if ( 'C' != c_poppeek( wantoption ) ) {
                        strncat( wantoption, "X", BUFSIZ-1 );
                        debug( "Background option '%s' waits for argument.\n",
                            argv[cx] );
                    } else {
                        debug( "Background option '%s' redundant.\n",
                            argv[cx] );
                    }
                }
            }
            else
            if ( ( argindex = _is_no_bg_opt( argv[cx] ) ) ) {
                fg_bg_idx      = 0;
                opts->bg_set   = 0;
                opts->red[1]   = 0;
                opts->green[1] = 0;
                opts->blue[1]  = 0;
                if ( c_last_skip( wantoption, 'C' ) ) {
                    debug( "NO Background option '%s', previous bg reset.\n",
                        argv[cx] );
                } else {
                    debug( "NO Background option '%s' reset.\n",
                        argv[cx] );
                }
            }
            else
            if ( ( argindex = _countmatch( "--reset", argv[cx], 6 ) ) ) {
                opts->reset = _means_yes(argv[cx]+argindex, 1);
                debug( "Reset option '%s' set to %d\n",
                        argv[cx], opts->reset );
            }
            else
            if ( ( argindex = _countmatch( "--no-reset", argv[cx], 9 ) ) ) {
                opts->reset = ( ! _means_yes(argv[cx]+argindex, 1) );
                debug( "Reset option '%s' set to %d\n",
                        argv[cx], opts->reset );
            }
            else
            if ( ( argindex = _countmatch( "--mode", argv[cx], 3 ) ) ) {
                if ( argv[cx][argindex] ) {
                    if ( 'c' == argv[cx][argindex] ) {
                        // Mode == colon
                        opts->mode = 'c';
                    } else {
                        opts->mode = 's';
                    }
                }
                else {
                    strncat( wantoption, "m", BUFSIZ-1 );
                }
            }
            else
            if ( ( argindex = _countmatch( "--red", argv[cx], 5 ) ) ) {
                if ( argv[cx][argindex] ) {
                    read_color_i( argv[cx]+argindex, &opts->red[fg_bg_idx] );
                    if ( fg_bg_idx ) {
                        opts->bg_set = 1;
                    } else {
                        opts->fg_set = 1;
                    }
                    debug( "Red '%s' set to '%d'.\n", argv[cx], opts->red[fg_bg_idx] );
                } else {
                    char askfor[2] = { fg_bg_idx?'R':'r', 0 };
                    char colorltr  = fg_bg_idx?'X':'x';
                    if ( colorltr == c_poppeek( wantoption ) ) {
                        c_last_skip( wantoption, colorltr );
                        if ( fg_bg_idx ) {
                            debug("Red option '%s' wait for argument "
                                    "overrides previous --bg wait.\n",
                                argv[cx] );
                        } else {
                            debug("Red option '%s' wait for argument "
                                    "overrides previous --fg wait.\n",
                                argv[cx] );
                        }
                    } else {
                        debug("Red option '%s' waits for argument.\n",
                                argv[cx] );
                    }
                    strncat( wantoption, askfor, BUFSIZ-1 );
                }
            }
            else
            if ( ( argindex = _countmatch( "--green", argv[cx], 3 ) ) ) {
                if ( argv[cx][argindex] ) {
                    read_color_i( argv[cx]+argindex, &opts->green[fg_bg_idx] );
                    if ( fg_bg_idx ) {
                        opts->bg_set = 1;
                    } else {
                        opts->fg_set = 1;
                    }
                    debug( "Green '%s' set to '%d'.\n", argv[cx], opts->green[fg_bg_idx] );
                } else {
                    char askfor[2] = { fg_bg_idx?'G':'g', 0 };
                    char colorltr  = fg_bg_idx?'X':'x';
                    if ( colorltr == c_poppeek( wantoption ) ) {
                        c_last_skip( wantoption, colorltr );
                        if ( fg_bg_idx ) {
                            debug("Green option '%s' wait for argument "
                                    "overrides previous --bg wait.\n",
                                argv[cx] );
                        } else {
                            debug("Green option '%s' wait for argument "
                                    "overrides previous --fg wait.\n",
                                argv[cx] );
                        }
                    } else {
                        debug("Green option '%s' waits for argument.\n",
                                argv[cx] );
                    }
                    strncat( wantoption, askfor, BUFSIZ-1 );
                }
            }
            else
            if ( ( argindex = _countmatch( "--blue", argv[cx], 3 ) ) ) {
                if ( argv[cx][argindex] ) {
                    read_color_i( argv[cx]+argindex, &opts->blue[fg_bg_idx] );
                    if ( fg_bg_idx ) {
                        opts->bg_set = 1;
                    } else {
                        opts->fg_set = 1;
                    }
                    debug( "Blue '%s' set to '%d'.\n", argv[cx], opts->blue[fg_bg_idx] );
                }
                else
                {
                    char askfor[2] = { fg_bg_idx?'B':'b', 0 };
                    char colorltr  = fg_bg_idx?'X':'x';
                    if ( colorltr == c_poppeek( wantoption ) ) {
                        c_last_skip( wantoption, colorltr );
                        if ( fg_bg_idx ) {
                            debug("Blue option '%s' wait for argument "
                                    "overrides previous --bg wait.\n",
                                argv[cx] );
                        } else {
                            debug("Blue option '%s' wait for argument "
                                    "overrides previous --fg wait.\n",
                                argv[cx] );
                        }
                    } else {
                        debug("Blue option '%s' waits for argument.\n",
                                argv[cx] );
                    }
                    strncat( wantoption, askfor, BUFSIZ-1 );
                }
            }
            else
            if ( ( argindex = _countmatch( "--color", argv[cx], 3 ) ) ) {
                if ( argv[cx][argindex] ) {
                    split_color_i( opts, argv[cx]+argindex,
                            &opts->red[fg_bg_idx],
                            &opts->green[fg_bg_idx],
                            &opts->blue[fg_bg_idx] );
                    if ( fg_bg_idx ) {
                        opts->bg_set = 1;
                    } else {
                        opts->fg_set = 1;
                    }
                } else {
                    char askfor[2] = { fg_bg_idx?'C':'c', 0 };
                    char colorltr  = fg_bg_idx?'X':'x';
                    if ( colorltr != c_poppeek( wantoption ) ) {
                        strncat( wantoption, askfor, BUFSIZ-1 );
                        debug("Color option '%s' waits for argument.\n",
                                argv[cx] );
                    } else {
                        debug("Color option '%s' wait for argument "
                            "redundant to previous --%cg wait.\n",
                            argv[cx], fg_bg_idx?'b':'f' );
                    }
                }
            }
            else {
                opts->needhelp = 1;
                fprintf(stderr, "Unknown option '%s'\n", argv[cx] );
            }
        }
        else if (  ( argseek )
                &&  (   ( '+' == argv[cx][0] )
                     || ( '-' == argv[cx][0] )
                    )
                )
        {
            int value = ('+'==argv[cx][0])?0:1;
            for ( int dx = 1; argv[cx][dx]; dx++ ) {
                switch( argv[cx][dx] ) {
                    case 'L':
                        opts->wantlicense = value;
                        debug( "License option '%c' in '%s'\n",
                                argv[cx][dx], argv[cx] );
                        break;
                    case 'h':
                        opts->wanthelp = value;
                        debug( "Help option '%c' in '%s'\n",
                                argv[cx][dx], argv[cx] );
                        break;
                    case 'r':
                        if ( '=' == argv[cx][dx+1] ) {
                            dx++;
                            if ( argv[cx][dx+1] ) {
                                read_color_i( argv[cx]+dx+1,
                                        &opts->red[fg_bg_idx] );
                                if ( fg_bg_idx ) {
                                    opts->bg_set = 1;
                                } else {
                                    opts->fg_set = 1;
                                }
                            }
                        } else {
                            char askfor[2] = { fg_bg_idx?'R':'r', 0 };
                            char colorltr  = fg_bg_idx?'X':'x';
                            if ( colorltr == c_poppeek( wantoption ) ) {
                                c_last_skip( wantoption, colorltr );
                                if ( fg_bg_idx ) {
                                    debug("Red option '%s' wait for argument "
                                            "overrides previous --bg wait.\n",
                                        argv[cx] );
                                } else {
                                    debug("Red option '%s' wait for argument "
                                            "overrides previous --fg wait.\n",
                                        argv[cx] );
                                }
                            } else {
                                debug("Red option '%s' waits for argument.\n",
                                        argv[cx] );
                            }
                            strncat( wantoption, askfor, BUFSIZ-1 );
                        }
                        break;
                    case 'g':
                        if ( '=' == argv[cx][dx+1] ) {
                            dx++;
                            if ( argv[cx][dx+2] ) {
                                read_color_i( argv[cx]+dx+1, &opts->green[fg_bg_idx] );
                                if ( fg_bg_idx ) {
                                    opts->bg_set = 1;
                                } else {
                                    opts->fg_set = 1;
                                }
                            }
                        } else {
                            char askfor[2] = { fg_bg_idx?'G':'g', 0 };
                            char colorltr  = fg_bg_idx?'X':'x';
                            if ( colorltr == c_poppeek( wantoption ) ) {
                                c_last_skip( wantoption, colorltr );
                                if ( fg_bg_idx ) {
                                    debug("Green option '%s' wait for argument "
                                            "overrides previous --bg wait.\n",
                                        argv[cx] );
                                } else {
                                    debug("Green option '%s' wait for argument "
                                            "overrides previous --fg wait.\n",
                                        argv[cx] );
                                }
                            } else {
                                debug("Green option '%s' waits for argument.\n",
                                        argv[cx] );
                            }
                            strncat( wantoption, askfor, BUFSIZ-1 );
                        }
                        break;
                    case 'b':
                        if ( '=' == argv[cx][dx+1] ) {
                            dx++;
                            if ( argv[cx][dx+2] ) {
                                read_color_i( argv[cx]+dx+1,
                                        &opts->blue[fg_bg_idx] );
                                if ( fg_bg_idx ) {
                                    opts->bg_set = 1;
                                } else {
                                    opts->fg_set = 1;
                                }
                            }
                        } else {
                            char askfor[2] = { fg_bg_idx?'B':'b', 0 };
                            char colorltr  = fg_bg_idx?'X':'x';
                            if ( colorltr == c_poppeek( wantoption ) ) {
                                c_last_skip( wantoption, colorltr );
                                if ( fg_bg_idx ) {
                                    debug("Blue option '%s' wait for argument "
                                            "overrides previous --bg wait.\n",
                                        argv[cx] );
                                } else {
                                    debug("Blue option '%s' wait for argument "
                                            "overrides previous --fg wait.\n",
                                        argv[cx] );
                                }
                            } else {
                                debug("Blue option '%s' waits for argument.\n",
                                        argv[cx] );
                            }
                            strncat( wantoption, askfor, BUFSIZ-1 );
                        }
                        break;
                    case 'c':
                        if ( '=' == argv[cx][dx+1] ) {
                            dx++;
                            if ( argv[cx][dx+1] ) {
                                split_color_i(
                                    opts, argv[cx]+dx+1,
                                    &opts->red[fg_bg_idx],
                                    &opts->green[fg_bg_idx],
                                    &opts->blue[fg_bg_idx]
                                );
                                if ( fg_bg_idx ) {
                                    opts->bg_set = 1;
                                } else {
                                    opts->fg_set = 1;
                                }
                            }
                        } else {
                            char askfor[2] = { fg_bg_idx?'C':'c', 0 };
                            char colorltr  = fg_bg_idx?'X':'x';
                            if ( colorltr != c_poppeek( wantoption ) ) {
                                debug("color (-c) in option '%s' "
                                        "waits for argument.\n",
                                        argv[cx] );
                                strncat( wantoption, askfor, BUFSIZ-1 );
                            } else {
                                debug("Color (-c) in option '%s' wait for "
                                    "argument redundant to previous "
                                    "--%cg wait.\n",
                                    argv[cx], fg_bg_idx?'b':'f' );
                            }
                        }
                        break;
                    case 'm':
                        if ( '=' == argv[cx][dx+1] ) {
                            dx++;
                            vdebug( "-m%c\n", argv[cx][dx] );
                            if ( argv[cx][dx+1] ) {
                                dx++;
                                opts->mode = (int)argv[cx][dx];
                                while ( argv[cx][dx+1] ) {
                                    dx++;
                                }
                            }
                        } else {
                            strncat( wantoption, "m", BUFSIZ-1 );
                        }
                        break;
                    case 't':
                        if ( '=' == argv[cx][dx+1] ) {
                            dx++;
                            vdebug( "-t%c\n", argv[cx][dx] );
                            if ( argv[cx][dx+1] ) {
                                dx++;
                                vdebug( "-t=%s\n", argv[cx]+dx );
                                opts->env_term = argv[cx]+dx;
                                // - 1 (put it on the last letter)
                                dx += ( strlen( opts->env_term ) - 1 );
                            }
                        } else {
                            strncat( wantoption, "t", BUFSIZ-1 );
                        }
                    case 'i':
                        if ( fg_bg_idx ) {
                            fg_bg_idx = 0;
                        } else {
                            fg_bg_idx = 1;
                        }
                        break;
                    case 's':
                        opts->reset = value;
                        break;
                    case 'q':
                        if ( value ) {
                            opts->verbosity -= 1;
                        } else {
                            opts->verbosity += 1;
                        }
                        break;
                    case 'v':
                        if ( value ) {
                            opts->verbosity += 1;
                        } else {
                            opts->verbosity -= 1;
                        }
                        break;
                    case '+':
                        value=0;
                        break;
                    case '-':
                        value=1;
                        break;
                    default:
                        opts->needhelp = 1;
                        fprintf(stderr, "Unknown option '%c' in '%s'\n",
                                argv[cx][dx], argv[cx] );
                        break;
                }
            }
        }
        else {
            int nextopt = c_shift( wantoption );
            if ( 'k' == nextopt ) {
                nextopt = c_shift( wantoption );
            }
            switch ( nextopt ) {
                case 'r':
                    read_color_i( argv[cx], &opts->red[0] );
                    opts->fg_set = 1;
                    debug( "Red option '%s' set to %d\n",
                        argv[cx], opts->red[0] );
                    break;
                case 'g':
                    read_color_i( argv[cx], &opts->green[0] );
                    opts->fg_set = 1;
                    debug( "Green option '%s' set to %d\n",
                        argv[cx], opts->green[0] );
                    break;
                case 'b':
                    read_color_i( argv[cx], &opts->blue[0] );
                    opts->fg_set = 1;
                    debug( "Blue option '%s' set to %d\n",
                        argv[cx], opts->blue[0] );
                    break;
                case 'R':
                    read_color_i( argv[cx], &opts->red[1] );
                    opts->bg_set = 1;
                    debug( "Red option '%s' set to %d\n",
                        argv[cx], opts->red[1] );
                    break;
                case 'G':
                    read_color_i( argv[cx], &opts->green[1] );
                    opts->bg_set = 1;
                    debug( "Green option '%s' set to %d\n",
                        argv[cx], opts->green[1] );
                    break;
                case 'B':
                    read_color_i( argv[cx], &opts->blue[1] );
                    opts->bg_set = 1;
                    debug( "Blue option '%s' set to %d\n",
                        argv[cx], opts->blue[1] );
                    break;
                case 'm':
                    opts->mode = (int)argv[cx][0];
                    debug("Wantoption at %d '%s', MODE override (%c).\n",
                            cx, argv[cx], opts->mode );
                    break;
                case 't':
                    opts->env_term = argv[cx];
                    debug("Wantoption at %d '%s', TERM override (cis %d%d).\n",
                            cx, argv[cx], opts->fg_set, opts->bg_set);
                    break;
                case 'C':
                case 'X':
                    if ( 0 == split_color_i( opts, argv[cx],
                                &opts->red[1], &opts->green[1], &opts->blue[1] ) )
                    {
                        opts->bg_set = 1;
                    }
                    debug("Wantoption at %d '%s', bgcolor (cis %d%d).\n",
                            cx, argv[cx], opts->fg_set, opts->bg_set);
                    break;
                case 'c':
                case 'x':
                    if ( 0 == split_color_i( opts, argv[cx],
                                &opts->red[0], &opts->green[0], &opts->blue[0] ) )
                    {
                        opts->fg_set = 1;
                    }
                    debug("Wantoption at %d '%s', fgcolor (cis %d%d).\n",
                            cx, argv[cx], opts->fg_set, opts->bg_set);
                    break;
                case 'K':
                    // KILL, just ignore this position.
                    break;
                case 0:
                    debug("Wantoption empty at %d '%s', color (cis %d%d).\n",
                            cx, argv[cx], opts->fg_set, opts->bg_set);
                    if ( ( ! opts->fg_set ) && ( ! opts->bg_set ) ) {
                        if ( 0 == split_color_i( opts, argv[cx],
                                &opts->red[fg_bg_idx],
                                &opts->green[fg_bg_idx],
                                &opts->blue[fg_bg_idx] ) )
                        {
                            if ( fg_bg_idx ) {
                                opts->bg_set = 1;
                            } else {
                                opts->fg_set = 1;
                            }
                        }
                    } else {
                        for ( int dx = 0; dx < BUFSIZ - 1; dx++ ) {
                            if ( NULL == opts->printables[dx] ) {
                                opts->printables[dx] = argv[cx];
                                break;
                            }
                        }
                    }
                    break;
                default:
                    fatal(
"Prog error, unhandled option assignment '%c'. %s:%d\n",
                        wantoption[0], __FILE__, __LINE__-3 );
                    break;
            }
        }
    }

    if ( 'C' == opts->mode ) {
        opts->mode = 'c';
    }
    else if ( ( opts->mode ) && ( 'c' != opts->mode ) ) {
        opts->mode = 's';
    }
    return opts;
}

int
read_position( char *buf, size_t sz )
{
    int ret = 0;
    if  ( ( Opts->no_input ) || ( Opts->std_out ) ) {
        strncpy( buf, "Not a TTY", sz );
    }
    else {
        term_open();
        fprintf( Opts->termfh, "\033[6n" );
        fflush( Opts->termfh );
        if ( readInput( sz, buf ) ) {
            ret = 1;
        }
    }
    return ret;
}

int
termcap_match( const char *buf, const char *match )
{
    int size = strlen(match);
    if ( ( buf ) && ( match ) && (match[0] == buf[0] ) ) {
        if ( 0 == strncmp( match, buf, size - 1 ) ) {
            if (   (   0 == Opts->env_term[size] )
                || ( '-' == Opts->env_term[size] ) 
                || ( '+' == Opts->env_term[size] ) )
            {
                return 1;
            }
        }
    }
    return 0;
}

int
main( int argc, char **argv, char **env )
{
    Opts = getOpts( argv, env );

    int mode = 1;  // semicolon (default)
    if ( Opts->mode ) {
        // Was set via commandline.
        if ( 'c' == Opts->mode ) {
            mode = 0;
        }
    }
    else if ( Opts->env_tm_truemode ) {
        // If not commandline, check environment var
        info( "_TM_TRUEMODE is %s\n", Opts->env_tm_truemode );
        if ( 'c' == Opts->env_tm_truemode[0] ) {
            mode = 0;
        }
    }
    else if ( ( mode ) && ( Opts->env_term ) ) {
        /*****
         * Final check...
         * Terminals KNOWN to support colon.
         ** These do NOT seem to have any termcap entry:
         **     evilvte
         **     guake
         **     lilyterm
         **     lxterminal
         **     pangoterm
         **     pantheon
         **     roxterm
         **     sakura
         **     therm
         */
        if ( termcap_match( Opts->env_term, "alacritty" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "foot" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "gnome" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "iTerm2.app" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "iterm2" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "kitty" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "konsole" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "teraterm" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "terminator" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "vte" ) ) {
            mode = 0;
        }
        else if ( termcap_match( Opts->env_term, "wezterm" ) ) {
            mode = 0;
        }
        if ( 0 == mode ) {
            debug( "TERM is %s, setting TrueColor mode to colon.\n",
                    Opts->env_term );
        }
    }

    if ( Opts->wanthelp ) {
        dohelp(Opts);
    }
    else if ( Opts->needhelp ) {
        dousage(Opts);
    }
    if ( Opts->wantlicense ) {
        printlicense();
    }
    if ( Opts->wantlicense + Opts->wanthelp + Opts->needhelp ) {
        if ( Opts->wanthelp + Opts->wantlicense ) {
            my_exit(0);
        }
        my_exit(1);
    }

    term_open();

    // Foreground is 38
    // Background is 48
    char color[BUFSIZ];
    char grab[BUFSIZ];
    char grab2[BUFSIZ];
    memset(grab, 0, BUFSIZ);
    memset(grab2, 0, BUFSIZ);
    int exit_code = 0;

    if ( Opts->bg_set ) {
        if ( mode ) {
            snprintf( color, BUFSIZ-1, "[48;2;%d;%d;%dm",
                    Opts->red[1], Opts->green[1], Opts->blue[1] );
        } else {
            snprintf( color, BUFSIZ-1, "[48:2::%d:%d:%dm",
                    Opts->red[1], Opts->green[1], Opts->blue[1] );
        }
        info( "Setting background with \\e%s\n", color );

        if ( ! Opts->fg_set ) {
            read_position( grab, BUFSIZ );
        }
        fprintf( Opts->termfh, "\033%s", color );
        fflush( Opts->termfh );
    }
    if ( Opts->fg_set ) {
        if ( mode ) {
            snprintf( color, BUFSIZ-1, "[38;2;%d;%d;%dm",
                    Opts->red[0], Opts->green[0], Opts->blue[0] );
        } else {
            snprintf( color, BUFSIZ-1, "[38:2::%d:%d:%dm",
                    Opts->red[0], Opts->green[0], Opts->blue[0] );
        }
        info( "Setting foreground with \\e%s\n", color );

        read_position( grab, BUFSIZ );
        fprintf( Opts->termfh, "\033%s", color );
        fflush( Opts->termfh );
    }
    read_position( grab2, BUFSIZ );

    if ( Opts->fg_set + Opts->bg_set ) {
        if ( ! grab[0] ) {
            warning( "# Unable to read cursor position.\n" );
            exit_code = 2;
        }
        else
        if ( ! grab2[0] ) {
            warning( "# Unable to read cursor position 2.\n" );
            exit_code = 2;
        }
        else {
            debug( "Pos before color: %s\n", grab );
            debug( "Pos  after color: %s\n", grab2 );
            if ( strncmp( grab, grab2, BUFSIZ ) ) {
                fprintf( stderr,
                        "\n# ERROR: Cursor moved when printing color command.\n" );
                exit_code = 1;
            }
        }
    }

    if ( exit_code ) {
        my_exit( exit_code );
    }

    if ( ( Opts->printables ) && ( Opts->printables[0] ) ) {
        Opts->text_output += doprint( Opts->printables[0] );
        for (int cx = 1; Opts->printables[cx]; cx++) {
            Opts->text_output += term_write( " " );
            Opts->text_output += doprint( Opts->printables[cx] );
        }
    }

    my_exit(exit_code);
}

void
printlicense()
{
    puts("\n\
MIT License\n\
\n\
" IDENT " v " VERSION "\n\
Copyright (c) " C_YEARS ", by:\n\
" AUTHORS
"\n\
Permission is hereby granted, free of charge, to any person obtaining\n\
a copy of this software and associated documentation files\n\
(the \"Software\"), to deal in the Software without restriction,\n\
including without limitation the rights to use, copy, modify, merge,\n\
publish, distribute, sublicense, and/or sell copies of the Software,\n\
and to permit persons to whom the Software is furnished to do so,\n\
subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be\n\
included in all copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n\
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n\
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n\
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n\
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n\
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n\
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n");
}
// EOF truecolor.c
