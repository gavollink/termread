# termread -t

Request Terminal ID (primary Device Attributes)

This was introduced with the VT100 series.
Every terminal emulator I've tested responds to this.

Sends two control strings to the terminal, and recieves back identifying
information.

Returns whatever is returned (octal escaping control characters) in a
bourne shell compatible variable string in the format:

```
TERMID='string'; export TERMID; 
```

Exits silently if the terminal does not respond.

## TermRead, the TERM environment variable and `-t`

TermRead does check the TERM environment variable when it runs.
First, there are a subset of `terminfo` entries that claim to respond
to *DECID* and not the much more common *Primary DA*.

For these TERM entries (hard-coded at compile time), TermRead will
send *DECID*, and wait for a response (see --delay).

For all TERM entries (hard-coded at compile time) that claim to
respond to *Primary DA*, TermRead will send both *Primary DA*
and `ENQ`.

For any unknown term entry, TermRead will print an error to stderr.
The `!` argument can be used to force *Primary DA* capable treatment.

My own testing has shown that it is harmless to send *Primary DA*
and *ENQ* in one string.  Doing this has the benefit of sniffing out
PuTTY (which has many quirks of its own), and `rxvt` which will
essentially respond twice.

## Sends

| DEC Name   | Sequence     | As shown in docs |
| :-----     |  ----        | -----            |
| DECID      |  `\033Z`     | ESC Z            |
| Primary DA | `\033[c\005` | CSI c ENQ        |

## Returns

```
TERMID='<sequence>'; export TERMID; 
```

Where sequence MAY be on of:

| Sequence                        | Observed Terminal             |
|:--------------------------------|:------------------------------|
| `\033/Z`                        | xterm in vt52 emulation mode  |
| `\033[?1;2c`                    | Apple Terminal.app (Catalina) |
| `\033[?1;0c`                    | Microsoft Terminal (store)    |
| `\033[?65;1;9c`                 | Gnome Terminal                |
| `\033[?64;1;2;6;9;15;18;21;22c` | xterm in vt420 default mode   |
| `\033[?1;2c\033[?1;2c`          | rxvt (Linux)                  |
| `\033[?6cPuTTY`                 | PuTTY (Windows)               |
| `\033[?6c`                      | xvt (Linux)                   |
| `\033[?6c`                      | Linux console (Ubuntu 20.02)  |

*NOTE* That at least under recent releases, `xterm` is claiming to emulate
the features of a VT400 series terminal with 8 available options, see below.
An `xterm` can be complied to emulate other options and other terminal
types.

# Returned Code Meanings

## VT Terminals

The following is scraped out of old DEC manuals and xterm source code.

With the exception of VT50 (which only responded to the DECID sequence,
never to *primary DA* sequence, the return code starts with these two bytes
( called *CSI* in the DEC docs ):

```
    \033[       # ESC + [
```

See [Observed Output](./Observed_Output.md) for more known return codes.

## VT100 through VT125 Terminals

After *CSI*, is `position 0`, which identifies the terminal type itself.

```
    ?1      vt100 or 101
    ?6      vt102
    ?12     vt125
```

Options Bitfield; `position 1`

| Bit Position | Meaning |
|----|----|
| 4 ( 0100 ) | STP |
| 2 ( 0010 ) | AVO |
| 1 ( 0001 ) | GPO / ReGIS |

Examples:

| Code | Definition |
|----|----|
| `\033[?1;2c` | VT100 terminal: noSTP, AVO, noGPO |
| `\033[?1;0c` | VT101 terminal: always had no options |
| `\033[?6c`   | VT102 terminal: always had no options, or option positon |
| `\033[?12;3;0c` | VT125 terminal... |
|                 | ... (position 2): bool: printer present |

Note: In xterm VT125 emultion does NOT emulate the printer codes,
and always leaves this option OFF.

## VT220 Terminals and Newer

After *CSI*, `position 0` identifies the terminal itself.
Any subsequent positions describe the options available for that terminal.

```
    ?62     VT200 series terminal
    ?63     VT300 series terminal
    ?64     VT400 series terminal
    ?65     VT500 series terminal
```

Subsequent positions, separated by semicolon (;), are output in
numeric order and each number always has the same meaning.

### VT200 and More Recent Option Codes

| Availility | Option Number | Definition |
|----|----|----|
| from 220   | `1` | 132 columns |
| from 220   | `2` | Printer port |
| 240 .. 340 | `3` | ReGIS graphics |
| 240 .. 382 | `4` | sixel graphics I/O |
| from 220   | `6` | Selective erase |
| from 300   | `7` | DRCS |
| from 220   | `8` | UDK (User Defined Keys) |
| from 220   | `9` | 7-bit national replacement characters avail. |
| VT520      | `12` | Serbo-Croation character set (SCS) |
| VT300      | `13` | Local Editing Mode |
| from 220   | `15` | Technical character set |
| from 300   | `16` | Locator Device Port |
| from 440   | `17` | Terminal state interrogation |
| from 300   | `18` | Windowing capability |
| from 300   | `19` | Dual Sessions |
| from 420   | `21` | Horizontal scrolling |
| from 520   | `22` | ANSI color |
| VT520      | `23` | Greek |
| VT520      | `24` | Turkish |
| ? xterm ?  |  28  | rectangular editing |
| ? xterm ?  |  29  | ANSI text locator |
| VT520      | `42` | ISO Latin-2 character set |
| VT520      | `44` | PCTerm |
| VT520      | `45` | Soft key map |
| VT520      | `46` | ASCII emulation |

See [VT Device Attributes, References](./VT_Device_Attributes.md#References)

# Some Technical History

Even before computers were a thing, Teleprinters hooked up to
telegraph lines were a thing, and a feature of some of these early
devices was to respond to a certain character (called `ENQ`) with a
customizable (per device) with a 20 or 22 character string.

How devices got there is some [Teleprinter History](./TP_History.md)
that I had to include.  Mostly, there was a need in multipoint
networks to verify that a reciever was on and have it identify itself.

## DEC Video Terminals (VT)

While researching this program (but mostly the documentation),
I did a lot of searching through archives of manuals on various
serial terminals produced by Digital Equipment Corp (DEC), and
took notes, which appear below.  The info that dosn't fit here
ended up on the [VT history](VT_History.md) page.

When DEC started building Video Terminals (VT), the circuitry to
respond to `ENQ` was considered too slow, so this signal was
ignored (VT05, VT50, etc), and the first DEC VT to respond to
`ENQ` with a customizable string was the VT100 model.

The first DEC Video Terminal, the VT05, did not have any feature or
function that would auto-respond back to a host computer in any way.
However, just as Teleprinter manufactures of the early 20th century
had figured out, this basic feature was important.

When DEC did introduce a similar feature into their next model,
the VT50, instead of using `ENQ`, DEC instead returned a
non-customizable per-model response to a two-character
sequence, '\033Z`, called *DECID*.  See [Observed Output](./Observed_Output.md) for specifics.

**Jumping to modern terminal emulators**, Simon Tatham's PuTTY has
a programmable response to `ENQ`, which defaults to `PuTTY` if left
unmodified.  The program iTerm2 on macOS also has a programmable
response, and its default is empty, just like a Teleprinter when
shipped new.  The terminal `rxvt` also responds to `ENQ`, but not
with a customizable response, but with a *Primary DA*
response (see below).

The three character sequence, `\033[c`, *Device Attributes* (DA),
was added by Digital Equipment Corp for its VT100 line of terminals.
A response to this sequence is fairly standard for *soft terminals*.
DEC terminals from the VT100 series, still replied to `\033Z` *DECID*,
but from the VT200 series on, it would only respond to *DECID* if it
were specifically in VT50 compatibilty mode.  The VT200 series
introduced *Secondary DA*, and renamed Device Attributes
to *Primary DA*.

Every terminal emulator I have tested responds to *Primary DA* with
a string that looks like a valid DEC *Primary DA* response.
The terminal emulator package `rxvt` responds to `ENQ` with its
*Primary DA* response.  Responses and meaning are above.

