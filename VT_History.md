# Serial Terminal History

# Before Screen-based Serial Terminals

Computer output used to be sent into a computer directly to a typewriter 
that was hooked up to press keys on specific output signals.
This meant using paper for any output.  These, so called, TeleType
machines (TTY) eventually became capable of being input devices as well.

These devices would connect to a computer using a serial connection
(the ancestor of USB).  There was no standard for the mapping of data
signals to the alphabet for the first 15 years of computing.
A standardization process, which would later be called ASCII started
in 1963, and did not look like the standard we use now until 1978.

*The ASCII standard is even preserved in UNICODE.*

## Early Start

Digital Equipment Corporation (DEC) invented the concept
of using a Cathode Ray Tube (CRT) to display output from a computer,
with the release of the PDP-1 way back in 1959, and it's accessory
display, the _CRT Type 30_.

Since the PDP-1 could also be write output to standard Teletypes,
not every PDP-1 came with a _CRT Type 30_.

This was the first computer monitor.  Just like modern monitors, it relied
entirely on the computer that was attached to it to tell it exactly what to
display.

## Screen-based Serial Terminals

The initial point of a screen based serial terminal is to replace
reams of paper that a teletype would produce with an ephemeral
display that could infinitely change without dealing with all the
paper.

A screen based serial terminal needed to have onboard capabilities
to display a font of characters onto the CRT, just like a TTY,
without a computer having to explain where each "dot" of a character
needs to be individually drawn.

It also needs to be able to scroll lines up the screen as new input 
is received.  This means that these terminals needed some memory to
be able to redraw the information that was on the rest of the screen.

Despite the great start of creating the concept of using a
screen with a computer, DEC was not the first company 
to create a CRT based serial terminal.  The earliest I've been able
to find is 1969's Datapoint 3300, by Computer Terminal Corporation.

## DEC VT05, Very dumb by today's standards

Only a year later, in 1970, DEC introduced their screen based serial
terminal which they called the _Video Terminal_, Model _VT_05.
This terminal was capable of displaying 20 lines of 72 columns, or 1442
characters.

While dumb, there was one major innovation that this terminal brought
about, the concept of _CAD_ (The Direct Cursor Address Code).
This used character `14` to put the terminal into a 'move the cursor'
mode.  So, unlike a typewriter, it was possible to change an
area in the middle of the screen without retyping the entire screen.

I noted that ASCII was still in flux during this time, so here are some
outstanding characters that I found that have non-ASCII meanings.

| Char | Action |
|---|---|
| 9  | Move cursor down (No tab key) |
| 14 | Cursor Addressing |
| 24 | Advance cursor right |
| 26 | Move cursor up |
| 27 | ALT (No Esc key) |
| 30 | Erase from cursor to end of line |
| 31 | Erase screen |

Worse, though, any control that moved the cursor needed to be
followed by NULL padding characters (how many, depended on the
BAUD rate) because the circuitry couldn't do the operation and keep
up with an ongoing communication stream.

I'll note that the VT50 docs throw shade at the need for NULL padding
which was common across all serial terminals at the time.

## VT50 Series, Introducing ESC codes

Introduced late in 1975, VT50, 52, 55 and 62 terminals were the first 
to interpret the ESC ( `\033` ) as the start of instructions to be
interpreted by the terminal instead of displayed directly to the user.
It is my belief that this innovation is why DEC terminal codes
have been supported in non-serial applications from DOS 2.0
(ansi.sys) up through modern graphical systems,
like Terminal.app in macOS.

This line also introduced the idea of sending data back to the computer 
that didn't come from user input.  This came in the form of the
DECID command ( `\033Z` ) which would respond with ( `\033/Z` ).

| Action | Code | Definition |
|----|----|----|
| Send    | `\033Z`  | DECID sequence |
| Recieve | `\033/Z` | VT52 response |

The VT50 did not support lowercase characters, the backtick (\`),
the braces ({/}), or the pipe (|), while the VT52 and later did.

That makes the VT52 the earliest 'modern' terminal.  Software written
for this terminal would have little issue talking with a modern
`xterm`, `Microsoft Terminal` or `Terminal.app`.

That said, its capabilities were very small in relation to the 
expectations of a modern user.

### Decision to not support the VT52/55/62

This software doesn't bother with DECID because VT50 terminals are
exceedingly rare, and I haven't found any soft terminals that
respond to DECID, but not *Primary DA*, with one very weird exception.

The X11 `xterm` software can be started in vt52 compatibility mode, and
in that case alone, the terminal type will ONLY be returned from a
DECID sequence.

That means this option would not illicit a response from `xterm` in that
one scenario.

Useful to note that `xterm` can _also_ be started in _Tektronix_ modes
which wouldn't be compatible with this at all.

# Sources

* [Computer History Museum](https://computerhistory.org/)
* [VT100.net](https://vt100.net/)
* [Wikipedia](https://wikipedia.org/)
