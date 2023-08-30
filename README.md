# TermRead

Very small C utility for reading information about a
modern soft terminal.

This is a UNIX style (meaning very small) utility that is meant to
be used in a script (like a .bashrc) to help figure out what 
terminal is actually attached.

*I have spent more time writing documentation about VT terminals than I
have spent writing the code for this.*

## Command Line (as of release 1.12)

### Actions

At least one action must be chosen

| [-t](./Dash-t.md) | Ask for terminal identity. |
| [-2](./Dash-2.md) | Ask for terminal version. |
| [-b](./Dash-b.md) | Ask terminal to respond with background color. |
| -c \<nnn> | Ask terminal for the color represented by supplied number |
| [-p \<str>](./Dash-p.md) | Send the terminal custom text |

### Options

| !  | Ignore the TERM environment variable, treat as VT100 or newer  |
| -d \<nnn> | Milliseconds to wait for the first character of a response |
| --var \<name> | Variable name for shell readable output. `*` |
| -s | Drop stats after each action. |
| -v | Verbose: extra output |

`*` If there are multiple actions, the --var can only be used on the
first (in Action Order above).  Argument position jockeying won't
change this.  Just call TermRead multiple times.

### About

| -L | Print that license and exit |
| -V | Print the version and exit |
| -h | Print some help and exit |

### Long Options

| -t | --term    |
| -2 | --term2   |
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
to escape codes invented by Digital Equipment Corporation *DEC* for
their Video Terminal line of terminals, starting with the basics set
up for the VT50 line of terminals.

When the American National Standards Institute looked to standardize
control codes, they chose codes that were a subset of the controls
for the DEC VT100 terminal.  This was even available in MS-DOS 5.0 by
loading ANSI.SYS.

The first UNIX graphical emulation of a terminal under 
the `X windows` system, xterm, carefully emulates a DEC VT terminal
(the exact emulated model has changed over the years).

These days, almost every software defined terminal sets
the default TERM environment variable to '**xterm**'.
That also means that the default terminfo entry for xterm, as shipped
by most vendors is, itself, a simplified entry, catering to the
lowest common denominator terminal features.

Thing is, with the exception of default starts of X11, I don't
actually use 'xterm' and the actual terminal I'm using may
have a better terminfo definition on the system that I'm attached to,
and might support colors, even though `xterm` might not say so.

There is a shell built-in called 'read' that can do what this does,
but it only works for me on `bash` and only on some systems I
regularly interact with.  Yet, all of these systems can compile this
simple code.

I find it useful to be able to query the terminal I'm on to try to
figure out what it actually is in the beginning of my startup
files (.profile).  I will then reset TERM to the best available
matching terminfo entry available on that system.

Between `-t` and `-2`, I usually have enough info that I know which
terminal program I happen to be using (or close enough).

## Support for TERM=

There's a helper program in here that creates C-Functions that are
just long lists of terminals sorted by the terminfo entry for
`user 9`.  Of these, I use the two lists that cover terminals that
only respond to *DECID*, and those that respond to *Primary DA*.

That itself is run on a system with the latest terminfo database
that ncurses has to offer, and even so, I've added a few entries
on top of each.

NOTE:
The first year + of this software, it did not support VT50/52/55/62,
as these require `DECID` send instead of `Primary DA` which all
modern terminals support.

However, adding this support was fairly trivial, so I finally made
the change...

Basically, if TERM is set to any of these four;
"vt50", "vt52", "vt55", or "vt62", the program will send a `DECID`
request for action `-t` and will print an error on `-2`, `-b`,
or `-c`.

### Actual DECID

I haven't found any soft terminals that respond to `DECID`,
but not `Primary DA`, with one very weird exception.

The X11 `xterm` software can be started in vt52 compatibility mode, and
in that case alone, the terminal type will ONLY be returned from a
`DECID` sequence where a `Primary DA` sequence will be ignored.

Useful to note that `xterm` can *also* be started in *Tektronix* modes
which wouldn't be compatible with this at all.

I have never encountered a program that only talks *Tektronix*, so I've
never considered trying to add such support here.

# Just because

Check out the [Teleprinter](./TP_History.md)
and [VT History](./VT_History.md) pages.

