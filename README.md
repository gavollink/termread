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

* [termread -t](Dash-t.md)
* [termread -b](Dash-b.md)

# Why

I use a lot of different computers, and not all of them have a modern
bash shell, but all of them can compile this simple code.
I find it useful to be able to query the terminal I'm on to try to figure
out what its capabilities are before my startup files (.profile) try to
set my default prompt.

If I happen to be connecting from PuTTY, that lets me set a bunch of
defaults quickly, since it has more features than it's willing to report.
