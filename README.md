# TermRead

Very small C utility for reading information about a
modern soft terminal.

This is a UNIX style (meaning very small) utility that is meant to
be used in a script (like a .bashrc) to help figure out what 
terminal is actually attached.

*I have spent more time writing documentation about VT terminals than I
have spent writing the code for this.*

## Code Home

    Primary Development: https://gitlab.home.vollink.com/external/termread
    Backup Location:     https://github.com/gavollink/termread

## Command Line (as of release 1.12)

### Actions

At least one action must be chosen

| Opt | Desc |
| --- | --- |
| [-t](./Dash-t.md) | Ask for terminal identity/attributes. |
| [-2](./Dash-2.md) | Ask for terminal version. |
| [-3](./Dash-3.md) | Ask for terminal unique ID. |
| [-b](./Dash-b.md) | Ask terminal to respond with background color. |
| -c \<nnn> | Ask terminal for the color represented by supplied number |
| [-p \<str>](./Dash-p.md) | Send the terminal custom text |

### Options

| Opt | Desc |
| --- | --- |
| !  | Ignore the TERM environment variable, treat as VT100 or newer  |
| -d \<nnn> | Milliseconds to wait for the first character of a response |
| --var \<name> | Variable name for shell readable output. `*` |
| -s | Drop stats after each action. |
| -v | Verbose: extra output |

`*` If there are multiple actions, the --var can only be used on the
first (in Action Order above).  Argument position jockeying won't
change this.  Just call TermRead multiple times.

### About

| Opt | Desc |
| --- | --- |
| -L | Print that license and exit |
| -V | Print the version and exit |
| -h | Print some help and exit |

### Long Options

| Short | Long |
| --- | --- |
| -t | --term    |
| -2 | --term2   |
| -3 | --term3   |
| -b | --bg , --background |
| -c | --color   |
| -p | --printf , --print |
| !  |           |
| -d | --delay   |
|    | --var     |
| -s | --stats   |
| -v | --verbose |
| -L | --license |
| -V | --version |
| -h | --help    |

# Examples

```
$ termread -t
TERMID='\033[?1;2c'; export TERMID;
$ termread -b
TERMBG='\033]11;rgb:2e2e/3434/3636\033\'; export TERMBG;
$ termread -c 231
COLOR='\033]4;231;rgb:ffff/ffff/ffff\07'; export COLOR;
$ termread -t -2
TERMID='\033[?1;2c'; export TERMID;
TERM2DA='\033[>1;95;0c'; export TERM2DA;
$ termread -p "\e[6n"
READ='\033[24;1R'; export READ;
$ termread -c 231 --var CLR231
CLR231='\033]4;231;rgb:ffff/ffff/ffff\07'; export CLR231;
$ eval `termread -c 231 --var CLR231`
$ echo $CLR231
\033]4;231;rgb:ffff/ffff/ffff\07
```

In practice, each of these would be wrapped in an eval:

```
$ eval `termread -t`
$ echo $TERMID
\033[65;1;9c
```

# Observed Output

Though not complete, I put together a page of how the terminals
I have tested react to **termread**.

- [Observed Output](Observed_Output.md)

# Why

In these modern times, every text interface to a computer, responds
to some subset of the escape codes specified in ANSI X3.64-1979,
which were first used by Digital Equipment Corporation *DEC* for their
Video Terminal *VT* line of terminals, starting with the VT100,
released 1978 (before the standard had been finalized).  While there
were MANY other types of terminals, later, **DEC**'s early adoption of
the ANSI escape codes, (as well as cost advantages that I won't get into
here) gave them a lead in general develer acceptance.

It is important to note here that the ANSI X3.64 escape sequence standard
explicitly states that there is NO expectation that any implementation
attempt to implement all of the specified features.
The DEC VT100 implemented many of these features, but not all of them.
Another sub-set of the ANSI Escape Sequence standard was even available
in MS-DOS 2.0 (1983) and more recent by loading ANSI.SYS at boot, which
would later be used by BBS dial-up software to support ASCII art.

The first UNIX graphical emulation of a terminal under 
the `X windows` system, xterm, carefully emulates a DEC VT terminal
(the exact emulated model has changed over the years), though it has
always been able to even emulate a pre-ANSI VT52 terminal (1974),
that had proprietary control sequences.

## The Actual Problem

Background covered, these days, almost every software defined terminal
sets the default TERM environment variable to '**xterm**'.
That also means that the default terminfo entry for xterm, as shipped
by most vendors is, itself, a simplified entry, catering to the
lowest common denominator terminal features that all terminal
emulators are likely to support.

Because the `xterm` entry is often a lowest common denominator catch-all,
there is probably a more complete terminal definition for whatever
terminal emulator I am using (even xterm itself).

To help my login start-up files figure out which terminal emulator
I'm using, I started to send three identity queries to the terminal.
There is a bash shell built-in called 'read' that can do what this does,
but it only works for me on some systems I regularly interact with.  Yet,
all of these systems can compile the simple termread code.

Between `-t` and `-2`, I usually have enough info that I know which
terminal program I happen to be using (or close enough).
Once I've figured out which terminal emulator the session is actually
connected to, I can then reset the TERM environment variable to the best
available matching terminfo entry available on that system.

(Even if it falls back to xterm anyway).

## Support for various TERM=

There's a helper program in here that creates C-Functions that are
just long lists of terminals sorted by the terminfo entry
`user 9` (this is where the primary identity string resides).
Of these, I use the two lists that cover terminals that
only respond to *DECID*, and those that respond to *Primary DA*.

That itself is run on a system with the latest terminfo database
that ncurses has to offer, and even so, I've added a few entries
on top of each.

NOTE:
The first year + of this software, it did not support VT52/55/62,
as these require `DECID` send instead of `Primary DA` which all
modern terminals support.

However, adding this support was fairly trivial, so I finally made
the change...

Basically, if TERM is set to any of the terminal definitions descended
from the "vt52 the program will send a `DECID` request for action `-t`
and will print an error on `-2`, `-b`, or `-c`.

### Actual DECID

I haven't found any soft terminals that respond to `DECID`,
but not `Primary DA`, with one very weird exception.

The X11 `xterm` software can be started in vt52 compatibility mode, and
in that case alone, the terminal type will ONLY be returned from a
`DECID` sequence where a `Primary DA` sequence will be ignored.

Useful to note that `xterm` can *also* be started in *Tektronix* modes
which wouldn't be compatible with this at all, either.

I have never encountered a program that only talks *Tektronix*, so I've
never considered trying to add such support here.

# ttguess.sh Utility

ttguess.sh is the tool I use in my startup scripts to try to figure
out which Terminal Emulator I'm logging in with.
This script uses termread internally.

This will export a number of environment variables about the terminal
(obiously, requires `source ttguess.sh` or `. ttguess.sh`), though
not all of these have complete coverage.

`_TM_KITTY`
`_TM_ITERM2`

Each of these have extended terminal capabilities not covered by any
`termcap` database, so these are set separately (and both are set
if `wezterm` is detected).

`_TM_COLORS` is the known color count (up to 256)

`_TM_EMOJI` if the terminal is known to be able to print EMOJI characters.

`_TM_EMOJISPACE` if the terminal prints fat emoji, obscuring the next char.

`_TM_TRUECOLOR` if the terminal is known to support truecolor (even if the
termcap doesn't know about it).

`_TM_TRUEMODE` if the terminal is known to support truecolor, this is set to
`colon` or `semi`.

`TERM` is set to the recommended value (as printed in the output).

# truecolor Utility

This will print truecolor escape codes, using environment `$_TM_TRUEMODE` to
switch `colon` from the default of `semi`.  This does NO checking for whether
or not truecolor is actually supported, so this can end up printing junk
if a terminal doesn't support it.

```
$ ./truecolor --help
$ ./truecolor -c xff8210 -s TEST OUTPUT
```

# Just because

Check out the [Teleprinter](./TP_History.md)
and [VT History](./VT_History.md) pages.

