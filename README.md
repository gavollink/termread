# TermRead

Very small C utility for reading information about a modern soft terminal.

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

# Options

- [termread -t](Dash-t.md)
- [termread -2](Dash-2.md)
- [termread -b](Dash-b.md)
- `termread -c <n>` -- Like -b, but ask for color number description)
    - Like -b, but ask for color number description)
- `termread --env MYSTR`
    - Use MYSTR instead of the default name in the output.

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

