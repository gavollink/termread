# VT History

Digital Equipment Corporation (DEC) invented the concept
of using a Cathode Ray Tube (CRT) to display letters from a computer,
with the release of the PDP-1 way back in 1959, and it's accompanying
display the CRT Type 30.

## VT

VT stands for Video Terminal, which was a type of serial device that
displayed output characters onto a CRT (monitor) instead of a paper
teletype.

## VT05, dumb

The VT05 was introduced in 1970 while the ASCII standard was still
in flux (it didn't start to look like what we know now until 1977).

I haven't seen anything that knows how to talk to one. 
It doesn't even have an entry in the termcap database,
so modern systems are clearly not expected to see one.

I found a 'tiff' file of the documenation for one of these,
and it appears to be a dumb terminal, with no support for
escape codes.

Some characters do have unexpected meanings though:

| Char | Action |
|---|---|
| 9  | Move cursor down (No tab key) |
| 14 | Cursor Addressing |
| 24 | Advance cursor right |
| 26 | Move cursor up |
| 27 | ALT (No Esc key) |
| 30 | Erase from cursor to end of line |
| 31 | Erase screen |

Worse, these needed NULL padding after them (depending on the
BAUD rate) because the circuitry couldn't do the operation and keep
up with ongoing communication stream).

I'll note that the VT50 docs throw shade at the need for NULL padding
which was common across all serial terminals at the time.

## VT50 Series, Introducing ESC codes

*Not supported by this software*

Introduced late in 1975, VT50, 52, 55 and 62 terminals were the first 
to interpret the ESC as the start of instructions to be interpreted
by the terminal instead of displayed directly to the user.  It is my
belief that this innovation is why DEC terminal codes have been supported
in non-serial applications from DOS 2.0 (ansi.sys) up through modern
graphical systems, like Terminal.app in macOS.

ONLY responded to DECID ( `\033Z` ).
This software doesn't bother with DECID because VT50 terminals are
exceedingly rare, and I haven't found any soft terminals that
respond to DECID, but not *Primary DA*.

Exception: xterm itself CAN be forced into vt52 compatibility mode, and
in that case alone, this code will ONLY be returned from DECID.  That
means this option would not illicit a response from `xterm` in that odd
scenario.

| Action | Code | Definition |
|----|----|----|
| Send    | `\033Z`  | DECID sequence |
| Recieve | `\033/Z` | VT52 response |

The VT50 did not support lowercase characters, the backtick (\`),
the braces ({/}), or the pipe (|), while the VT52 and later did.


