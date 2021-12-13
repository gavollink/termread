# TermRead

Very small C utility for reading information about a modern soft terminal.

In these modern times, almost every conceivable terminal sets the TERM
environment variable to '**xterm**'.
Thing is, with the exception of default starts of X11, I don't
actually use 'xterm' and the actual terminal I'm using may
have a better termcap on the system I'm attached to, and might
support colors, even though it doesn't say so.

This is a UNIX style (meaning very small) utility that is meant to
be used in a script (like a .bashrc) to help figure out what the
terminal can do.

Generally, if TERM is not 'xterm', my scripts won't call this at all.

*I have spent more time writing documentation about VT terminals than I
have spent writing the code for this.*

# Examples

```
$ termread -t
TERMID='\033[65;1;9c'; export TERMID; 
$ termread -b
TERMBG='\033]11;rgb:2e2e/3434/3636\033\'; export TERMBG; 
$ termread -c 231
COLOR='\033]4;231;rgb:ffff/ffff/ffff\07'; export COLOR; 
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

# Options

- [termread -t](Dash-t.md)
- [termread -2](Dash-2.md)
- [termread -b](Dash-b.md)
- `termread -c <n>`
    - Like -b, but ask for color number description)
- `termread --env MYSTR`
    - Use MYSTR instead of the default name in the output.
    - Only valid with other options.
- `termread ! ...`
    - Ignore TERM environment variable (try anyway)
    - Only valid with other options.

```
$ termread -c 231 --env CLR231
CLR231='\033]4;231;rgb:ffff/ffff/ffff\07'; export CLR231; 
$ eval `termread -c 231 --env CLR231`
$ echo $CLR231
\033]4;231;rgb:ffff/ffff/ffff\07
```

# Why

There is a shell built-in called 'read' that can do part of this,
but it only works for me on `bash` and only on some systems.
All of these systems can compile this simple code.

I find it useful to be able to query the terminal I'm on to try to figure
out what its capabilities are before my startup files (.profile) try to
set my default prompt.

Between `-t` and `-2`, I can usually get enough into environment variables
that I know which terminal program I happen to be using (or close enough).

## Decision to not support the VT52/55/62

This software doesn't bother with DECID because VT50 class terminals are
exceedingly rare, and I haven't found any soft terminals that
respond to DECID, but not *Primary DA*, with one very weird exception.

The X11 `xterm` software can be started in vt52 compatibility mode, and
in that case alone, the terminal type will ONLY be returned from a
DECID sequence.

That means this option would not illicit a response from `xterm` in that
one scenario.

Useful to note that `xterm` can _also_ be started in _Tektronix_ modes
which wouldn't be compatible with this at all.

# Just because

Check out the [VT History](VT_History.md) page.

