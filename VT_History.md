# Serial Terminal History

# Before Screen-based Serial Terminals

Before computers (as modern English uses the term), there was telegraph
based communication.  More info on [that history](./TP_History.md) is
on the Teleprinter History Page, but typewriters that could communicate
keystrokes to a remote location and repeat keystrokes sent by a remote
location had been around since ~1910

When computers became complex enough to be able to interact on human
timescales, Teleprinter devices were already available and were able
to be utilized as a way to communicate in text to a computer, and get
output back from that computer.

This meant using paper for any output.  These TelePrinter devices,
most commonly made by the company, TeleType (as in the TTY device
type as found in UNIX and early Linux).

These devices would connect to a computer using a serial connection
(the ancestor of USB).  There was no true standard for the mapping of
data signals to the alphabet for the first 15 years of computing.
It was actually common to replace or move the punch keys on TeleType
machines to fit whatever a given applications needs might be, so even
the built-in Baudot-Murry standard that TeleTypes were built to use
were not actually treated as a standard.  Computers didn't really talk
to OTHER computers, so custom solutions were the obvious choice.
A standardization process, which would later be called ASCII started
in 1960, was first published in 1963, and did not look like the
standards we use now until 1978.

*The 1978 ASCII standard is even preserved in UNICODE.*

## Early Start

Digital Equipment Corporation (DEC) invented the concept
of using a Cathode Ray Tube (CRT) to display output from a computer,
with the release of the PDP-1 way back in 1959, and it's accessory
display, the _CRT Type 30_.

Since the PDP-1 could also be write output to standard Teleprinters,
not every PDP-1 came with a _CRT Type 30_.

This was the first computer monitor.  Just like modern monitors, it
relied entirely on the computer that was attached to it to tell it
exactly what to display.

# Screen-based Serial Terminals

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
terminal which they called the *Video Terminal*, Model *VT*05.
This terminal was capable of displaying 20 lines of 72 columns, or 1442
characters.

While dumb, there was one major innovation that this terminal brought
about, the concept of *CAD* (The Direct Cursor Address Code).
This used character `14` (ASCII `SO`) to put the terminal into a
'move the cursor' mode.  So, unlike a typewriter, it was possible to
change an area in the middle of the screen without retyping the entire
screen.

The manual for the VT05 says it complies with
`U.S. ANSI X3.15-1966` (standard for serial communication of data).
However, that standard also references `U.S. ANSI X3.4-1963` the first
version of ASCII.
Even though ASCII was still in flux during this time, there are some
outstanding characters this terminal uses as control characters that
have meanings that do not comply with the ASCII definition...

| Char | Action | ASCII definition |
|---|---| ---- |
| 9  | Move cursor down (No tab key) | HT, Horizontal Tab (1963+) |
| 14 | Cursor Addressing | SO, Shift Out (1963+) |
| 24 | Advance cursor right | CAN, Cancel (1963+) |
| 26 | Move cursor up | SUB, Substitute (1963+) |
| 27 | ALT (No Esc key) | ESC, Escape (1963+) |
| 30 | Erase from cursor to end of line | S6, Data Separator lv6 (1963), RS, Record Separator (1978+) |
| 31 | Erase screen | S7, Data Separator lv8 (1963), US, Unit Separator (1978+) |

Worse, though, any control that moved the cursor needed to be
followed by NULL padding characters (how many, depended on the
BAUD rate) because the circuitry couldn't do the operation and keep
up with an ongoing communication stream.

I'll note that the later VT50 docs throw shade at the need for NULL
padding which was common across all serial terminals at the time.

### The Loss of ENQ

Since the [ITU2](https://en.wikipedia.org/wiki/Baudot_code#ITA2)
standard of 1924, all standards have put aside an `ENQ` character that is
expected to be responded to by the recieving end automatically.
In ASCII and Unicode, this is code-point 5.  On Teleprinters this was often
a key labeled `WRU` (who are you) or `Here Is`.

The Computer Terminal Corporation's, Datapoint 3300 supported this feature
with the very first "glass terminal", but DEC decided that the cost of
implementation wasn't worth it, so DEC terminals would remain without the
ability to respond to Enquiries until the VT220.

## VT50 Series, Introducing ESC codes

Introduced late in 1975, VT50, 52, 55 and 62 terminals were the first
to interpret the ESC ( `\033` ) as the start of instructions to be
interpreted by the terminal instead of displayed directly to the user.
While the use of ESC to change settings at the recieving end was
put into 1963-ASCII by IBM Employee Robert Bemer, it was initially
expected to be used to swap a font carriage (for an international or
proprietary encoding), but DEC extended this to move all of the VT05
commands (like Cursor Direct Address) and a few new ones
(like Switch to Graphics Mode).

It is my belief that these innovations are why ESC has been the
start of reciever instructions for every terminal since, and was
later coded into the first ANSI Escape Codes standard,
`ANSI X3.64-1979` (though none of the VT05 or VT50 codes, themselves,
survived that process).

The VT50 line came up with an interesting twist on the missing `ENQ`
functionality by introducing a way to figure out the model of terminal
a computer is talking to.  This was a two byte response hard-coded into
the terminal, instead of the per-device settable `ENQ` response of
Teleprinters.  This came in the form of the DECID command ( `\033Z` )
which would respond with a terminal identifier.

For response strings,
see [VT Device Attributes](./VT_Device_Attributes.md).

The VT50 and VT50H did not support lowercase characters,
the backtick `` ` ``,
the braces ` { } `,
or the pipe ` | `, while the VT52 and later did.

That makes the VT52 the earliest 'modern' terminal.  Software written
for this terminal would have little issue talking with a modern
`xterm` (started in compatability mode).

That said, its capabilities were very small in relation to the
expectations of a modern user.

## VT100 series

August 1978, the VT100 was brought to market.
The VT100 was the first product to use the (not yet published)
`ANSI X3.64-1979` Control Codes standard.
This series proved to be very successful and created a HUGE number
of clones, making the VT100 the most emulated terminal in history.
On any modern system, most base capabilities of a VT100 are taken for
granted.

In this series, the DECID command is still supported, but is deprecated
in favor of the `Device Attributes` (DA) command: `\033[c`, where `\033[`
is the ANSI standard control start indicator.

Don't be fooled by the higher numbers, the VT101 and VT102 are reduced
cost and reduced feature versions of the VT100.

## VT200 series

Starting with the VT 220, the `Secondary Device Attributes`
or `Secondary DA` command was introduced: `\033[>c`

This has been embraced by most emulators (even ones that only claim VT100
capabilites).  With the advent of `Secondary DA` the original `DA` is 
referred to as `Primary DA`.

The VT200 series also brought back a per-device programmable ENQ response.
Few terminal emulators support this, though some, like PuTTY, and iTerm2, 
do support ENQ (Suffice to suggest, support is spotty).

## Primary DA and other terminals

Many other terminal types eventually picked up the general feature of
identifying themselves if a certain string is sent, though not all
respond to the DEC `Primary DA` string.  This idea of being able to identify
a terminal by literally asking it became popular enough that the `user9`
or `u9` string in curses (terminfo) is used to store this value.

# Sources

* [Computer History Museum](https://computerhistory.org/)
* [VT100.net](https://vt100.net/)
* [Wikipedia](https://wikipedia.org/)

