/****************************************************************************
 * termread.c
 *
 * Linux/MacOS untility to query VT compatible terminals for basic
 *  identifiers and capabilities.
 *
 * Internals of is_vtxx and is_vtxxx are from the
 * helper program generate_u9_matches.pl
 *
 * https://www.vt100.net/docs/vt100-ug/chapter3.html
 * https://vt100.net/docs/vt220-rm/chapter4.html
 *
 * LICENSE: Embedded at bottom...
 *
 */
#define VERSION "1.18"
#define C_YEARS "2021-2024"
#define IDENT "termread"
#define WEBHOME "https://gitlab.home.vollink.com/external/termread/"
#define AT "@"
#define DOT "."
#define AUTHOR1 "Gary Allen Vollink"
#define CONTACT1 "gary" AT "vollink" DOT "com"
#define AUTHORS "   " AUTHOR1 " <" CONTACT1 ">\n"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int term_cleanline();
void printlicense( void );

static struct termios orig_term, new_term;

struct sopt {
    int needhelp;   /* Help is needed */
    int wanthelp;   /* Help is explicitly requested */
    int wantversion;  /* Version is requested */
    int wantlicense;  /* License is requested */
    int background; /* VT100 background query */
    int getcolor;   /* VT100 color query */
    int termname;   /* VT* terminal caps query */
    int term2da;
    int term3da;
    int print;
    int justerase;  /* VT* terminal current line erase sequence */
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
    char * custom_print;
    char * var;
    char * term;
    char * buf;
    char * envterm;
    char * argv0;
} opt;

/***************************************
 * .control :
 * 'h' Help Output HEADER
 * 'H' Help Output HEADER only if opt.debug is set.
 * 'e' END of STRuCTURE
 * 'd' Help Output option only if apt.debug is set.
 * 'u' Suppress from `usage` output
 */
struct options_s {
    int   *int_dest;
    char  *header;
    char  *full[3];
    char  *want;
    char  *descr;
    char   indic;
    char  *helptext[3];
    char   ltr[3];
    char   control;
    char   is_action;
} Options[] = {
    {
        .control = 'h',
        .header = "ACTIONS\000\000",
    },
    {
        .is_action = 1,
        .full = { "term", NULL },
        .ltr  = { 't', '1', 0 },
        .int_dest = &opt.termname,
        .helptext = { "Ask terminal for 'Primary DA'.", NULL }
    },
    {
        .is_action = 1,
        .full = { "term2", NULL },
        .ltr  = { '2', 0 },
        .int_dest = &opt.term2da,
        .helptext = { "Ask terminal for 'Secondary DA'.", NULL }
    },
    {
        .is_action = 1,
        .full = { "term3", NULL },
        .ltr  = { '3', 0 },
        .int_dest = &opt.term3da,
        .helptext = { "Ask terminal for 'Tertiary DA'.", NULL }
    },
    {
        .is_action = 1,
        .full = { "bg", "background", NULL },
        .ltr  = { 'b', 0 },
        .int_dest = &opt.background,
        .helptext = { "Ask terminal for background color.", NULL }
    },
    {
        .is_action = 1,
        .full = { "color", "colour", NULL },
        .ltr  = { 'c', 0 },
        .want = "nnn",
        .descr = "color",
        .indic = 'c',
        .helptext = { "Ask terminal for background color.", NULL }
    },
    {
        .is_action = 1,
        .full = { "print", "printf", NULL },
        .ltr  = { 'p', 0 },
        .want = "string",
        .descr = "printf",
        .indic = 'p',
        .int_dest = &opt.print,
        .helptext = {
            "Ask terminal using string.",
            "String escapes will be expanded.",
            NULL
        }
    },
    {
        .control = 'd',
        .is_action = 1,
        .full = { "erase", NULL },
        .ltr  = { 0 },
        .int_dest = &opt.justerase,
        .helptext = {
            "Print 'erase current line' sequence,",
            "    and exit immediately.",
            NULL
        }
    },
    {
        .control = 'h',
        .header = "OPTIONS\000\000",
    },
    {
        .full = { NULL },
        .ltr  = { '!', 0 },
        .helptext = { "Ignore TERM env, asks as if TERM=xterm.", NULL }
    },
    {
        .full = { "delay", NULL },
        .ltr  = { 'd', 0 },
        .want = "ddd",
        .descr = "delay",
        .indic = 'd',
        .helptext = {
            "Milliseconds to wait for terminal reply.",
            "default: 500 ( 0.5 seconds ).",
            NULL
        }
    },
    {
        .ltr  = { 0 },
        .full = { "var", NULL },
        .want = "name",
        .descr = "variable name",
        .indic = 'Z',
        .helptext = {
            "Variable name for shell readable output.",
            "Only used for first output if multiple.",
            NULL
        }
    },
    {
        .ltr  = { 's', 0 },
        .full = { "stats", NULL },
        .int_dest = &opt.wantstat,
        .helptext = {
            "Print stats info after read response.",
            NULL
        }
    },
    {
        .ltr  = { 'v', 0 },
        .full = { "verbose", NULL },
        .int_dest = &opt.debug,
        .helptext = {
            "Print extra informational output.",
            NULL
        }
    },
    {
        .control = 'd',
        .ltr  = { 'o', 0 },
        .full = { "tty", NULL },
        .want = "/dev/tt...",
        .descr = "device name",
        .indic = 'o',
        .helptext = {
            "Open this instead of current stdin/stdout.",
            "Probably won't work.",
            NULL
        }
    },
    {
        // THIS NON-OPTION IS PULLED BEFORE the long/short arg checks, so
        // it doesn't need to be assigned to anything.
        // It is needed by prinhelp() though
        .full = { "", NULL },
        .descr = "options end",
        .helptext = {
            "Stop looking for options, force interpretation",
            "    of arguments to options.",
            NULL,
        }
    },
    {
        .control = 'h',
        .header = "ABOUT\000\000",
    },
    {
        .control = 'u',
        .is_action = 1,
        .full = { "version", NULL },
        .ltr  = { 'V', 0 },
        .int_dest = &opt.wantversion,
        .helptext = { "Print version number (" VERSION ") and exit.", NULL }
    },
    {
        .control = 'u',
        .is_action = 1,
        .full = { "license", NULL },
        .ltr  = { 'L', 0 },
        .int_dest = &opt.wantlicense,
        .helptext = { "Print license and exit.", NULL }
    },
    {
        .control = 'u',
        .is_action = 1,
        .full = { "help", NULL },
        .ltr  = { 'h', 0 },
        .int_dest = &opt.wanthelp,
        .helptext = { "This help.", NULL }
    },
    {
        .control = 'e',
    }
};

char background_var[] = "TERM_BG";
char termname_var[] = "TERMID";
char term2da_var[] = "TERM2DA";
char term3da_var[] = "TERM3DA";
char getcolor_var[] = "COLOR";
char print_var[] = "READ";
char default_var[] = "OUT";

const char vt_termreq[] = "\033Z";
const char vt_eraseline[] = "\033\015\033K";
// The xt_termreq sequence is based on Primary DA (name from
// vt220 manual).  However, this sequence includes a cheat-code.
// PuTTY responds to "\005" with "PuTTY", so by putting out both,
// PuTTY responsds with both outputs like so: '\033[?6cPuTTY'
// THIS IS NOT FOOLPROOF since, well, the setting is configurable
// by the user per connection (Under Terminal settings).
const char xt_termreq[] = "\033[c\005";
// This sequence is copied from the vt220 manual
//  where it is referred to as `Secondary DA`
// and is only supported by vt220 descendencts.
// Bascially, if a terminal doesn't respond to "xt_termreq"
// it won't respond to this.
// Note that many modern vt100 descendence (including xterm)
// respond to this anyway.
const char xt_term2da[] = "\033[>c";
const char xt_term3da[] = "\033[=c";
const char xt_colorbg[] = "\033]11;?\033\\";
const char xt_colorreq[] = "\033]4;%d;?\007";
const char xt_eraseline[] = "\033[9D\033[2K";

#define DEBUGOUT(str, ... ) if ( opt.debug ) \
    { \
        char *b = calloc(1024, 1); \
        sprintf( b, "# DEBUG: %s", str );\
        fprintf( stderr, b, __VA_ARGS__ ); \
        free(b); \
    };

void
prinversion(void)
{
    FILE *out = stderr;
    fprintf(out, "%s Version: %s\n",
                 opt.argv0, VERSION );

    return;
}

void
prinusage(void)
{
    FILE *out = stderr;
    if ( opt.wanthelp ) {
        out = stdout;
    }
    fprintf( out, "%s", opt.argv0 );
    for ( int cx = 0; ; cx++ ) {
        if ( ( Options[cx].control ) && ( 'e' == Options[cx].control ) ) {
            // 'e' END of STRuCTURE
            break;
        }
        else if ( ! Options[cx].control ) {
            // 'u' Suppress from `usage` output
            //     note how this isn't defined anywhere.
            if ( Options[cx].ltr[0] ) {
                if ( Options[cx].want ) {
                    fprintf( out,
                            " [-%c <%s>]",
                            Options[cx].ltr[0],
                            Options[cx].want
                           );
                } else {
                    fprintf( out, " [-%c]", Options[cx].ltr[0] );
                }
            }
        }
    }
    fprintf(out, "\n" );
    fprintf(out, "    OR\n");
    fprintf(out, "%s --help | --version | --license\n",
                 opt.argv0 );

    return;
}

int
sncmp( const char * a, const char * b, long int len )
{
    int match = 0;
    for ( long int cx = 0; cx < len; cx++ ) {
        if ( ( 0 == a[cx] ) && ( 0 == b[cx] ) ) {
            return( match );
        }
        else if ( 0 == a[cx] ) {
            match--;
            return( match );
        }
        else if ( 0 == b[cx] ) {
            match++;
            return( match );
        }
        match += ( a[cx] - b[cx] );
        if ( match ) {
            return( match );
        }
    }
    return match;
}

int is_matchlist ( const char * term, const char ** match )
{
    char *tbuf;
    tbuf = calloc( strlen(term)+2, 1 );
    strcpy( tbuf, term );
    /* If `term` is xterm-400, eliminate the -400 */
    char * found = index(tbuf, '-');
    if ( found ) {
        *found = 0;
    }
    /* If `term` is xterm+direct2, eliminate the +direct2 */
    found = index(tbuf, '+');
    if ( found ) {
        *found = 0;
    }
    int two = 0;
    if ( strlen(term) > strlen(tbuf) ) {
        two = 1;
    }
    for ( int cx = 0; 1; cx++ ) {
        if ( 0 == match[cx][0] ) {
            free( tbuf );
            return ( 1 );
        }
        /* Explicit request to match up to and including end-nulls */
        if ( 0 == sncmp( match[cx], term, strlen( match[cx] ) + 1 ) ) {
            free( tbuf );
            return (0);
        }
        if ( two ) {
            if ( 0 == sncmp( match[cx], tbuf, strlen( match[cx] ) + 1 ) ) {
                free( tbuf );
                return (0);
            }
        }
    }
    free( tbuf );
    return ( -1 );
}

int is_vtxx ( const char * term )
{
    /*******
     *   SELF ID ( user9 ) |\EZ| term list
     */
    const char *term_list[] = {
        "hz1552",
        "hz1552-rv",
        "linux+decid",
        "linux-m1",
        "linux-m1b",
        "linux-m2",
        "putty-m1",
        "putty-m1b",
        "putty-m2",
        "screen.linux-m1",
        "screen.linux-m1b",
        "screen.linux-m2",
        "screen.minitel1",
        "screen.minitel1-nb",
        "screen.minitel12-80",
        "screen.minitel1b",
        "screen.minitel1b-80",
        "screen.minitel1b-nb",
        "screen.minitel2-80",
        "screen.putty-m1",
        "screen.putty-m1b",
        "screen.putty-m2",
        "vt50",
        "vt50h",
        "vt52",
        // Not in termcap, but exists
        "vt52b",
        "vt55",
        "vt62",
        "\000"
    };
    return is_matchlist( term, term_list );
}

int is_vtxxx ( const char * term )
{
    /*******
     *   SELF ID ( user9 ) |\E[c| term list
     */
    const char *term_list[] = {
        "Apple_Terminal",
        "Eterm",
        "Eterm-256color",
        "Eterm-88color",
        "Eterm-color",
        "absolute",
        "alacritty",
        "alacritty+common",
        "alacritty-direct",
        "ansi",
        "ansi+enq",
        "ansi-color-2-emx",
        "ansi-color-3-emx",
        "ansi-emx",
        "aterm",
        "cit101e-rv",
        "crt",
        "crt-vt220",
        "cygwin",
        "cygwinDBG",
        "dec-vt220",
        "domterm",
        "foot+base",
        "gnome",
        "gnome-2007",
        "gnome-2008",
        "gnome-2012",
        "gnome-256color",
        "gnome-fc5",
        "gnome-rh62",
        "gnome-rh72",
        "gnome-rh80",
        "gnome-rh90",
        "hterm",
        "hterm-256color",
        "iTerm.app",
        "iTerm2.app",
        "iris-color",
        "iterm",
        "iterm2",
        "iterm2-direct",
        "jfbterm",
        "kitty",
        "kitty+common",
        "kitty-direct",
        "kon",
        "kon2",
        "konsole",
        "konsole-16color",
        "konsole-256color",
        "konsole-base",
        "konsole-direct",
        "konsole-linux",
        "konsole-solaris",
        "konsole-vt100",
        "konsole-vt420pc",
        "konsole-xf3x",
        "konsole-xf4x",
        "kterm",
        "kterm-co",
        "kterm-color",
        "kvt",
        "linux",
        "linux-16color",
        "linux-basic",
        "linux-c",
        "linux-c-nc",
        "linux-koi8",
        "linux-koi8r",
        "linux-lat",
        "linux-m",
        "linux-nic",
        "linux-s",
        "linux-vt",
        "linux2.2",
        "linux2.6",
        "linux2.6.26",
        "linux3.0",
        "mgt",
        "mintty",
        "mintty+common",
        "mintty-direct",
        "mlterm",
        "mlterm-256color",
        "mlterm-direct",
        "mlterm2",
        "mlterm3",
        "mrxvt",
        "mrxvt-256color",
        "ms-terminal",
        "ms-vt-utf8",
        "ms-vt100",
        "ms-vt100+",
        "ms-vt100-color",
        "ncsa",
        "ncsa-m",
        "ncsa-m-ns",
        "ncsa-ns",
        "ncsa-vt220",
        "ncsa-vt220-8",
        "netbsd6",
        "nsterm",
        "nsterm+7",
        "nsterm+acs",
        "nsterm+mac",
        "nsterm-16color",
        "nsterm-256color",
        "nsterm-7",
        "nsterm-7-c",
        "nsterm-7-c-s",
        "nsterm-7-m",
        "nsterm-7-m-s",
        "nsterm-7-s",
        "nsterm-acs",
        "nsterm-acs-c",
        "nsterm-acs-c-s",
        "nsterm-acs-m",
        "nsterm-acs-m-s",
        "nsterm-acs-s",
        "nsterm-bce",
        "nsterm-build309",
        "nsterm-build326",
        "nsterm-build343",
        "nsterm-build361",
        "nsterm-build400",
        "nsterm-c",
        "nsterm-c-7",
        "nsterm-c-acs",
        "nsterm-c-s",
        "nsterm-c-s-7",
        "nsterm-c-s-acs",
        "nsterm-direct",
        "nsterm-m",
        "nsterm-m-7",
        "nsterm-m-acs",
        "nsterm-m-s",
        "nsterm-m-s-7",
        "nsterm-m-s-acs",
        "nsterm-old",
        "nsterm-s",
        "nsterm-s-7",
        "nsterm-s-acs",
        "nwp-517",
        "nwp-517-w",
        "nwp517",
        "nwp517-w",
        "nxterm",
        "pccon",
        "pccon-m",
        "putty",
        "putty-256color",
        "putty-noapp",
        "putty-sco",
        "putty-screen",
        "putty-vt100",
        "rxvt-unicode",
        "rxvt-unicode-256color",
        "screen",
        "screen-16color",
        "screen-16color-bce",
        "screen-16color-bce-s",
        "screen-16color-s",
        "screen-256color",
        "screen-256color-bce",
        "screen-256color-bce-s",
        "screen-256color-s",
        "screen-bce",
        "screen-bce.Eterm",
        "screen-bce.gnome",
        "screen-bce.konsole",
        "screen-bce.linux",
        "screen-bce.mrxvt",
        "screen-bce.rxvt",
        "screen-bce.xterm-new",
        "screen-s",
        "screen-w",
        "screen.Eterm",
        "screen.gnome",
        "screen.konsole",
        "screen.konsole-256color",
        "screen.linux",
        "screen.linux-s",
        "screen.mlterm",
        "screen.mlterm-256color",
        "screen.mrxvt",
        "screen.nsterm",
        "screen.putty",
        "screen.putty-256color",
        "screen.rxvt",
        "screen.teraterm",
        "screen.vte",
        "screen.vte-256color",
        "screen.xterm-256color",
        "screen.xterm-new",
        "screen.xterm-r6",
        "screen.xterm-xfree86",
        "screen4",
        "screen5",
        "scrt",
        "securecrt",
        "st",
        "st-0.6",
        "st-0.7",
        "st-0.8",
        "st-16color",
        "st-256color",
        "st-direct",
        "stterm",
        "stterm-16color",
        "stterm-256color",
        "teken",
        "teraterm",
        "teraterm-256color",
        "teraterm2.3",
        "teraterm4.59",
        "teraterm4.97",
        "terminology",
        "terminology-1.8.1",
        "termite",
        "ti916",
        "ti916-132",
        "ti916-220-7",
        "ti916-220-8",
        "ti916-8",
        "ti916-8-132",
        "tmux",
        "tmux-256color",
        "tmux-direct",
        "uniterm",
        "uniterm49",
        "v200-nam",
        "v320n",
        "vs100",
        "vscode",
        "vscode-direct",
        "vt-utf8",
        "vt100+",
        "vt100+enq",
        "vt102+enq",
        "vt200",
        "vt200-w",
        "vt220",
        "vt220-base",
        "vt220-nam",
        "vt220-w",
        "vt300",
        "vt300-nam",
        "vt300-w",
        "vt300-w-nam",
        "vt320",
        "vt320-nam",
        "vt320-w",
        "vt320-w-nam",
        "vt320nam",
        "vt420",
        "vt420f",
        "vt420pc",
        "vt420pcdos",
        "vt510",
        "vt510pc",
        "vt510pcdos",
        "vt520",
        "vt520ansi",
        "vt525",
        "vte",
        "vte-2007",
        "vte-2008",
        "vte-2012",
        "vte-2014",
        "vte-2017",
        "vte-2018",
        "vte-256color",
        "vte-direct",
        "vtnt",
        "wsvt25",
        "wsvt25m",
        "x68k",
        "x68k-ite",
        "xfce",
        "xiterm",
        "xterm",
        "xterm+nofkeys",
        "xterm-1002",
        "xterm-1003",
        "xterm-1005",
        "xterm-1006",
        "xterm-16color",
        "xterm-24",
        "xterm-256color",
        "xterm-88color",
        "xterm-8bit",
        "xterm-basic",
        "xterm-bold",
        "xterm-color",
        "xterm-debian",
        "xterm-direct",
        "xterm-direct16",
        "xterm-direct2",
        "xterm-direct256",
        "xterm-hp",
        "xterm-kitty",
        "xterm-mono",
        "xterm-new",
        "xterm-nic",
        "xterm-noapp",
        "xterm-old",
        "xterm-pcolor",
        "xterm-r5",
        "xterm-r6",
        "xterm-sco",
        "xterm-sun",
        "xterm-utf8",
        "xterm-vt220",
        "xterm-x10mouse",
        "xterm-x11hilite",
        "xterm-x11mouse",
        "xterm-xf86-v32",
        "xterm-xf86-v33",
        "xterm-xf86-v333",
        "xterm-xf86-v40",
        "xterm-xf86-v43",
        "xterm-xf86-v44",
        "xterm-xfree86",
        "xterm-xi",
        "xterm.js",
        "xterm1",
        "xterms",
        "xterms-sun",
        "xwsh",
        "z340",
        "z340-nam",
        // Does not officially have u9, but does support:
        "vt100",
        "vt101",
        "vt102",
        // Not in terminfo.src, but does support:
        "ghostty",
        "xterm-ghostty",
        "wezterm",
        "\000"
    };
    return is_matchlist( term, term_list );
}

int
_arg_find_long( char *in )
{
    int ret = -1;

    for ( int cx = 0; ; cx++ ) {
        if ( 'e' == Options[cx].control ) {
            break;
        }
        for ( int dx = 0; ; dx++ ) {
            if ( NULL == Options[cx].full[dx] ) {
                break;
            }
            char *s = Options[cx].full[dx];
            if ( strlen( s ) ) {
                if ( 0 == sncmp(in, s, strlen(s)) ) {
                    ret = cx;
                }
            }
        }
    }

    return ret;
}

int
_arg_find_short( char in )
{
    int ret = -1;

    for ( int cx = 0; ; cx++ ) {
        if ( 'e' == Options[cx].control ) {
            break;
        }
        for ( int dx = 0; ; dx++ ) {
            if ( 0 == Options[cx].ltr[dx] ) {
                break;
            }
            if ( in == Options[cx].ltr[dx] ) {
                ret = cx;
            }
        }
    }

    return ret;
}

int
_arg_valid( int index, char *val )
{
    char  indic  = Options[index].indic;
    char *endptr = NULL;

    // NOTE: Apple clang doesn't allow variable create within a switch case
    //       they were all moved here instead.
    int   getcolor = 0;
    int   getdelay = 0;

    switch(indic) {
        case 'p':
            opt.custom_print = val;
            DEBUGOUT("Value '%s' for %s is set\n",
                    val,
                    ( Options[index].descr?
                        Options[index].descr:
                        Options[index].full[0] )
                    );
            break;
        case 'Z':
            opt.var = val;
            DEBUGOUT("Value '%s' for %s is set\n",
                    val,
                    ( Options[index].descr?
                        Options[index].descr:
                        Options[index].full[0] )
                    );
            break;
        case 'o':
            opt.term = val;
            DEBUGOUT("Value '%s' for %s is set\n",
                    val,
                    ( Options[index].descr?
                        Options[index].descr:
                        Options[index].full[0] )
                    );
            break;
        case 'c':
            getcolor = strtol( val, &endptr, 10 );
            if (   ( endptr != val )
                && ( endptr - val == strlen(val) )  )
            {
                opt.getcolor = 1;
                opt.color_num = getcolor;
                DEBUGOUT("--color [%i] ACTION requested.\n", opt.color_num);
            } else {
                opt.needhelp = 1;
                return 1;
            }
            break;
        case 'd':
            endptr = NULL;
            getdelay = strtol( val, &endptr, 10 );
            if (   ( endptr != val )
                && ( endptr - val == strlen(val) )  )
            {
                if ( 0 >= getdelay ) {
                    opt.needhelp = 1;
                    return 1;
                } else {
                    opt.delay = getdelay;
                    DEBUGOUT("--delay [%li].\n", opt.delay);
                }
            }
            break;
        default:
            return 1;
    }
    return 0;
}

void
prinhelp(void)
{
    FILE *out = stderr;
    if ( opt.wanthelp ) {
        out = stdout;
    }
    fprintf( out, IDENT " Version " VERSION "\n\n" );
    fprintf( out, "Copyright (c) " C_YEARS ", by\n" AUTHORS "\n" );
    prinusage();
    fprintf( out, "\nQueries Terminal for various settings.\n" );
    fprintf( out, "short options can be bundled.\n" );
    for ( int cx = 0; ; cx++ ) {
        if ( ( Options[cx].control ) && ( 'e' == Options[cx].control ) ) {
            // 'e' END of STRuCTURE
            break;
        }
        else if ( ( Options[cx].control )
               && ( 'd' == Options[cx].control )
               && ( ! opt.debug ) )
        {
            // 'd' Help Output option only if apt.debug is set.
            ;
        }
        else if ( ( Options[cx].control ) && ( 'H' == Options[cx].control ) ) {
            // 'H' Help Output HEADER only if opt.debug is set.
            if ( opt.debug ) {
                fprintf( out, "\n  %s:\n", Options[cx].header );
            }
        }
        else if ( ( Options[cx].control ) && ( 'h' == Options[cx].control ) ) {
            // 'h' Help Output HEADER
            fprintf( out, "\n  %s:\n", Options[cx].header );
        } else {
            fprintf( out, "\n" );
            for ( int dx = 0; ; dx++ ) {
                if ( 0 == Options[cx].ltr[dx] ) {
                    break;
                }
                if ( Options[cx].want ) {
                    fprintf( out, "    -%c <%s>\n",
                            Options[cx].ltr[dx],
                            Options[cx].want );
                } else {
                    fprintf( out, "    -%c\n", Options[cx].ltr[dx] );
                }
            }
            for ( int dx = 0; ; dx++ ) {
                if ( NULL == Options[cx].full[dx] ) {
                    break;
                }
                if ( Options[cx].want ) {
                    fprintf( out, "    --%s <%s>\n",
                            Options[cx].full[dx],
                            Options[cx].want );
                } else {
                    fprintf( out, "    --%s\n", Options[cx].full[dx] );
                }
            }
            for ( int dx = 0; ; dx++ ) {
                if ( NULL == Options[cx].helptext[dx] ) {
                    break;
                }
                fprintf( out, "        %s\n", Options[cx].helptext[dx] );
            }
        }
    }
    if ( opt.wanthelp ) {
        fprintf(out, "\n");
        if ( 0 != strcmp( getenv("TERM"), opt.envterm ) ) {
            fprintf(out, "  Effective TERM='%s'\n", opt.envterm);
        }
        fprintf(out, "Environment TERM='%s'\n", getenv("TERM") );
    }

    return;
}

int
zero_int( int *arry, int max )
{
    int ret = 0;
    while ( ret < max ) {
        arry[ret++] = 0;
    }
    return(ret);
}

int
push_int( int *arry, int next, int max )
{
    int ret = 0;
    while( 0 != arry[ret] ) {
        ret++;
        if ( ret > max ) {
            return -1;
        }
    }
    arry[ret] = next;
    int rest = 1 + ret;
    while ( rest < max ) {
        arry[rest++] = 0;
    }
    return( ret );
}

int
shift_int( int *arry, long max )
{
    int ret = 0;
    int go  = 1;

    if ( ( arry ) && ( 0 != arry[0] ) ) {
        ret = arry[0];
        for ( int cx = 1; cx < max; cx++ ) {
            if ( go ) {
                arry[cx - 1] = arry[cx];
                if ( 0 == arry[cx] ) {
                    go = 0;
                }
            }
            arry[cx] = 0;
        }
    }

    return( ret );
}

int
zero_ch( char *arry, int max )
{
    int ret = 0;
    while ( ret < max ) {
        arry[ret++] = 0;
    }
    return(ret);
}

int
push_ch( char *arry, char next, long max )
{
    int ret = strlen(arry);
    char put[2];
    put[0] = next;
    put[1] = 0;
    if ( ret < max ) {
        strncat( arry, put, max );
        ret++;
    } else {
        return -1;
    }
    return( ret );
}

char
shift_ch( char *arry, long max )
{
    char ret = 0;
    int  go  = 1;

    if ( ( arry ) && ( 0 != arry[0] ) ) {
        ret = arry[0];
        for ( int cx = 1; cx < max; cx++ ) {
            if ( go ) {
                arry[cx - 1] = arry[cx];
                if ( 0 == arry[cx] ) {
                    go = 0;
                }
            }
            arry[cx] = 0;
        }
    }

    return( ret );
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
    int needarg[BUFSIZ+1];
    int fromarg[BUFSIZ+1];
    zero_int(needarg, BUFSIZ+1 );
    zero_int(fromarg, BUFSIZ+1 );
    int  arg_process = 1;

    /* Walk the commandline */
    for ( int cx=1; cx < argc; cx++ ) {

        /* CHECK for stop processing        */
        if (   ( strlen("--") == strlen(argv[cx]) )
            && ( 0 == strncmp("--", argv[cx], strlen("--") + 1) ) )
        {
            arg_process = 0;
        }
        /* CHECK for standalone '!'         */
        else if (  ( arg_process )
                && ( 1 == strlen( argv[cx] ) )
                && ( '!' == argv[cx][0] ) )
        {
            opt.ignoreterm = 1;
            opt.envterm = "xterm";
            DEBUGOUT("! forcing TERM actions for [%s]\n", opt.envterm);
        }
        /* CHECK for long options           */
        else if ( ( arg_process )
               && ( strlen("--") < strlen(argv[cx]) )
               && ( 0 == strncmp("--", argv[cx], 2) ) )
        {
            int found = -1;
            if ( 0 <= ( found = _arg_find_long( argv[cx]+2 ) ) ) {
                if ( Options[found].int_dest ) {
                    *Options[found].int_dest = 1;
                }
                char *descr = ( Options[found].descr?
                                 Options[found].descr:
                                 Options[found].full[0] );
                if ( Options[found].is_action ) {
                    DEBUGOUT("%s --%s ACTION requested.\n",
                           descr, argv[cx]+2, NULL);
                    action_requested++;
                } else {
                    DEBUGOUT("%s --%s requested.\n",
                           descr, argv[cx]+2, NULL);
                }
                if ( Options[found].want ) {
                    char * eq  = index(argv[cx], '=');
                    if ( eq ) {
                        eq++;
                        if ( _arg_valid( found, eq ) ) {
                            fprintf(stderr,
                                    "Not a valid %s '%s' for option '%s'\n",
                                    ( Options[found].descr?
                                       Options[found].descr:
                                       Options[found].full[0] ),
                                    eq,
                                    argv[cx]
                                   );
                        }
                    } else {
                        if ( 0 > push_int( needarg, found, BUFSIZ ) ) {
                            fprintf( stderr,
"Too many value requiring options requested at arg %d, '%s'\n",
                                    cx, argv[cx] );
                            exit( 1 );
                        } else {
                            push_int( fromarg, cx, BUFSIZ );
                        }
                    }
                }
            } else {
                fprintf( stderr, "Unknown option %d: [%s]\n", cx, argv[cx] );
                opt.needhelp = 1;
            }
        } // END check for long options
        /* CHECK for short options           */
        else if ( ( arg_process )
               && ( 1 < strlen(argv[cx]) )
               && ( '-' == argv[cx][0] ) )
        {
            for ( int dx=1; dx < strlen(argv[cx]); dx++ ) {
                int found = -1;
                if ( 0 <= ( found = _arg_find_short( argv[cx][dx] ) ) ) {
                    if ( Options[found].int_dest ) {
                        *Options[found].int_dest = 1;
                    }
                    char *descr = ( Options[found].descr?
                                     Options[found].descr:
                                     Options[found].full[0] );
                    if ( Options[found].is_action ) {
                        DEBUGOUT("%s -%c ACTION requested.\n",
                                descr, argv[cx][dx], NULL);
                        action_requested++;
                    } else {
                        DEBUGOUT("%s -%c requested.\n",
                               descr, argv[cx][dx], NULL);
                    }
                    if ( Options[found].want ) {
                        if ( 0 > push_int( needarg, found, BUFSIZ ) ) {
                            fprintf( stderr,
"Too many value requiring options at arg %d, '%c' in '%s'\n",
                                cx, argv[cx][dx], argv[cx] );
                            exit( 1 );
                        } else {
                            push_int( fromarg, cx, BUFSIZ );
                        }
                    }
                } else {
                    fprintf( stderr, "Unknown option %d: [%s] char '%c'\n",
                            cx, argv[cx], argv[cx][dx] );
                    opt.needhelp = 1;
                }
            } // END walk single short argument
        } // END check for short options
        /* PULL anything left if I need arguments */
        else if ( 0 != needarg[0] ) {
            // yes, I need at least one argument!
            int next_arg = shift_int( needarg, BUFSIZ );
            int from_arg = shift_int( fromarg, BUFSIZ );
            if ( _arg_valid( next_arg, argv[cx] ) ) {
                fprintf(stderr,
                        "Not a valid %s '%s' for option '%s'\n",
                        ( Options[next_arg].descr?
                            Options[next_arg].descr:
                            Options[next_arg].full[0] ),
                        argv[cx],
                        argv[from_arg]
                    );
            }
        } else {
            fprintf( stderr, "Unknown option %d: [%s]\n", cx, argv[cx] );
            opt.needhelp = 1;
        }
    } //  END walk argv[cx] -- for ( int cx=1; cx < argc; cx++ )

    /* Complain about unfulfilled args */
    if ( 0 != needarg[0] ) {
        int next_arg = shift_int( needarg, BUFSIZ );
        int from_arg = shift_int( fromarg, BUFSIZ );
        fprintf( stderr, "Missing argument for option %d, '%s' (%c)",
                from_arg, argv[ from_arg ], next_arg );
        if ( 0 != needarg[1] ) {
            fprintf( stderr, " and more.\n" );
        } else {
            fprintf( stderr, ".\n" );
        }
        opt.needhelp = 1;
    }
    if ( ( NULL == opt.envterm ) || ( 0 == strlen( opt.envterm ) ) )
    {
        opt.envterm = "xterm";
        DEBUGOUT("TERM empty, forcing actions for [%s]\n", opt.envterm);
    }
    if ( opt.needhelp || opt.wanthelp || opt.wantversion || opt.wantlicense ) {
        // SIDE EFFECT -- NO DEBUG WARNING ABOUT --var BELOW.
        action_requested = 1;
    }
    /* Set default delay */
    if ( 0 == opt.delay ) {
        opt.delay = 500;      /* ~ 500 milliseconds or 0.5 seconds */
        DEBUGOUT("--delay defaulting to [%li]\n", opt.delay);
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
            DEBUGOUT("--var [%s] will only be used for --term\n", opt.var );
        }
        else if ( opt.term2da ) {
            DEBUGOUT("--var [%s] will only be used for --term2\n", opt.var );
        }
        else if ( opt.term3da ) {
            DEBUGOUT("--var [%s] will only be used for --term3\n", opt.var );
        }
        else if ( opt.getcolor ) {
            DEBUGOUT("--var [%s] will only be used for --color\n", opt.var );
        }
        else if ( opt.background ) {
            DEBUGOUT("--var [%s] will only be used for --bg %i\n", opt.var,
                    opt.color_num );
        }
        else if ( opt.print ) {
            DEBUGOUT("--var [%s] will only be used for --printf\n", opt.var );
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
        exit(1);
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
term_open()
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
                exit(1);
            }
        } else {
            fprintf( stderr, "Unable to open '%s': %s\n",
                opt.term, strerror(errno) );
            exit(1);
        }
    }
    if ( opt.termfh ) {
        return 1;
    }
    exit(1);
}

int
term_close()
{
    int chk = 0;
    if ( opt.termfh ) {
        chk = fclose(opt.termfh);
        opt.termfh = NULL;
    }
    if ( 0 == chk ) {
        return 1;
    }
    return 0;
}

int
term_cleanline()
{
    FILE * fh;
    if (term_open()) {
        fh = opt.termfh;
    } else {
        return 0;
    }

    int ret = 0;
    if ( 0 == is_vtxx( opt.envterm ) ) {
        ret = fprintf(fh, vt_eraseline );
    } else {
        ret = fprintf(fh, xt_eraseline );
    }
    fflush( fh );
    return ret;
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

#define NO_NEWLINE 0x2
#define INTERPRET_ESC 0x4

int
doprint( int opts, FILE *fh, char* out )
{
    int buildval = 0;
    int retval = 0;
    for ( int cx = 0; cx < strlen( out ); cx++ ) {
        if ( opts & INTERPRET_ESC ) {
            if ( '\\' == out[cx] ) {
                if ( 0 == sncmp ( "\\\\", &out[cx], 2 ) ) {
                    putc('\\', fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\a", &out[cx], 2 ) ) { // BEL
                    putc(7, fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\b", &out[cx], 2 ) ) { // BS
                    putc(8, fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\e", &out[cx], 2 ) ) { // ESC
                    /* Escape Character */
                    putc( 0x1b , fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\f", &out[cx], 2 ) ) { // FF
                    putc( 0x0c , fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\n", &out[cx], 2 ) ) { // NL
                    putc( 0x0a , fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\r", &out[cx], 2 ) ) { // CR
                    putc( 0x0d , fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\t", &out[cx], 2 ) ) { // HT
                    putc( '\t' , fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\v", &out[cx], 2 ) ) { // VT
                    putc( 0x0b , fh);
                    retval++;
                    cx++;
                }
                else if ( 0 == sncmp ( "\\x", &out[cx], 2 ) ) { // HEX
                    if ( isxdigit( out[cx+2] ) ) {
                        if ( isxdigit( out[cx+3] ) ) {
                            buildval = 16 * hextobin( out[cx+2] );
#ifdef DEBUG
    DEBUGOUT( "HEX BUILD pos 1: '%c', Now: %d \\x%02x \\0%03o\n",
            out[cx+2], buildval, buildval, buildval );
#endif
                            buildval = buildval + hextobin( out[cx+3] );
#ifdef DEBUG
    DEBUGOUT( "HEX BUILD pos 2: '%c', Final: %d \\x%02x \\0%03o\n",
            out[cx+2], buildval, buildval, buildval );
#endif
                            cx = cx + 3;
                        } else {
                            buildval = hextobin( out[cx+2] );
#ifdef DEBUG
    DEBUGOUT( "HEX BUILD pos 1: '%c', Final: %d \\x%02x \\0%03o\n",
            out[cx+2], buildval, buildval, buildval );
#endif
                            cx = cx + 2;
                        }
                        if ( buildval ) {
                            putc( buildval , fh);
                            retval++;
                        }
                        buildval = 0;
                    } else {
#ifdef DEBUG
    fprintf( stderr, "HEX ABORT on 'no hexadecimal digit'.\n");
#endif
                        putc( '\\' , fh);
                        retval++;
                    }
                }
                else if ( 0 == sncmp ( "\\0", &out[cx], 2 ) ) { // OCTAL
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
                            putc( buildval , fh);
                            retval++;
                        }
                        buildval = 0;
                    } else {
#ifdef DEBUG
    fprintf( stderr, "OCTAL ABORT on 'no octal digit'.\n");
#endif
                        putc( '\\' , fh);
                        retval++;
                    }
                } else {
                    putc( '\\' , fh);
                    retval++;
                }
            } else {
                putc( out[cx] , fh);
                retval++;
            }
        } else {
            putc( out[cx] , fh);
            retval++;
        }
    }
    fflush( fh );
    return retval;
}

int
term_write()
{
    FILE * fh;
    if (term_open()) {
        fh = opt.termfh;
    } else {
        return 0;
    }

    int ret = 0;
    if ( 1 == opt.termname ) {
        opt.termname = 0;
        if ( NULL == opt.var ) {
            opt.var = termname_var;
            DEBUGOUT("Set default --term var to %s\n", opt.var );
        }

        if ( 0 == is_vtxx( opt.envterm ) ) {
            // THIS IS VERY RARE, WILL PROBABLY NEVER BE USED...
            ret = fprintf(fh, vt_termreq );
        }
        else if ( 0 == is_vtxxx( opt.envterm ) ) {
            ret = fprintf(fh, xt_termreq );
        } else {
            fprintf( stderr,
                "# Current effective TERM='%s', does not support --term\n",
                opt.envterm );
            exit(1);
        }
        fflush( fh );
    }
    else if ( 1 == opt.term2da ) {
        opt.term2da = 0;
        if ( NULL == opt.var ) {
            opt.var = term2da_var;
            DEBUGOUT("Set default --term2 var to %s\n", opt.var );
        }

        if ( 0 == is_vtxxx( opt.envterm ) ) {
            ret = fprintf(fh, xt_term2da);
            fflush( fh );
        } else {
            fprintf( stderr,
                "# Current effective TERM='%s', does not support --term2\n",
                 opt.envterm);
            if ( 0 == is_vtxx( opt.envterm ) ) {
                fprintf( stderr, "# This term in a descendent of vt50, but 'Secondary DA'\n" );
                fprintf( stderr, "# is a feature of 'vt220', 'xterm' and descendents.\n" );
            }
        }
    }
    else if ( 1 == opt.term3da ) {
        opt.term3da = 0;
        if ( NULL == opt.var ) {
            opt.var = term3da_var;
            DEBUGOUT("Set default --term2 var to %s\n", opt.var );
        }

        if ( 0 == is_vtxxx( opt.envterm ) ) {
            ret = fprintf(fh, xt_term3da);
            fflush( fh );
        } else {
            fprintf( stderr,
                "# Current effective TERM='%s', does not support --term3\n",
                 opt.envterm);
            if ( 0 == is_vtxx( opt.envterm ) ) {
                fprintf( stderr, "# This term in a descendent of vt50, but 'Tertiary DA'\n" );
                fprintf( stderr, "# is a feature of 'vt510', 'xterm' and descendents.\n" );
            }
        }
    }
    else if ( 1 == opt.getcolor ) {
        opt.getcolor = 0;
        if ( NULL == opt.var ) {
            opt.var = getcolor_var;
            DEBUGOUT("Set default --color var to %s\n", opt.var );
        }

        if ( 0 == is_vtxxx( opt.envterm ) ) {
            ret = fprintf(fh, xt_colorreq, opt.color_num);
            fflush( fh );
        } else {
            fprintf( stderr,
                "# Current effective TERM='%s', does not support --color\n",
                opt.envterm);
            if ( 0 == is_vtxx( opt.envterm ) ) {
                fprintf( stderr, "# This term in a descendent of vt50, but color\n" );
                fprintf( stderr, "# is a feature of 'vt220' descendents.\n" );
            }
        }
    }
    else if ( 1 == opt.background ) {
        opt.background = 0;
        if ( NULL == opt.var ) {
            opt.var = background_var;
            DEBUGOUT("Set default --bg var to %s\n", opt.var );
        }

        if ( 0 == is_vtxxx( opt.envterm ) ) {
            ret = fprintf(fh, xt_colorbg );
            fflush( fh );
        } else {
            fprintf( stderr,
                "# Current effective TERM='%s', does not support --color\n",
                opt.envterm);
            if ( 0 == is_vtxx( opt.envterm ) ) {
                fprintf( stderr, "# This term in a descendent of vt50, but color\n" );
                fprintf( stderr, "# is a feature of 'vt220' descendents.\n" );
            }
        }
    }
    else if ( 1 == opt.print ) {
        opt.print = 0;
        if ( NULL == opt.var ) {
            opt.var = print_var;
            DEBUGOUT("Set default --printf var to %s\n", opt.var );
        }
        doprint( INTERPRET_ESC | NO_NEWLINE, fh, opt.custom_print );
    }
    else {
        if ( 0 == is_vtxx( opt.envterm ) ) {
            ret = fprintf(fh, vt_eraseline );
        } else {
            ret = fprintf(fh, xt_eraseline );
        }
        fflush( fh );
    }
    return ret;
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

    while (   opt.termname
            + opt.background
            + opt.getcolor
            + opt.term2da
            + opt.term3da
            + opt.print )
    {
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

    term_close();

    return 0;
}

int
main( int argc, char *argv[], char *env[] )
{
    args( argc, argv );

    if ( opt.wanthelp ) {
        prinhelp();
        exit( 0 );
    }
    else if ( opt.needhelp ) {
        prinusage();
        exit( 1 );
    }
    else if ( opt.wantlicense ) {
        printlicense();
        exit( 0 );
    }
    else if ( opt.wantversion ) {
        prinversion();
        exit( 0 );
    }
    else if ( opt.justerase ) {
        term_cleanline();
        exit( 0 );
    }

    do_term();

    return(0);
}

void
printlicense()
{
    puts("\n\
MIT License\n\
\n\
" IDENT " v " VERSION " -- " WEBHOME "\n\
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
/* EOF termread.c */
