# termread -t

Request Terminal ID (primary DA)

Sends two control strings to the terminal, and recieves back identifying
information (sometimes).

Returns whatever is returned (octal escaping control characters) in a
bourne shell compatible variable string in the format:

```
TERMID='string'; export TERMID; 
```

## Sends

So long as the TERM environment variable identifies as
vtXX, xtXX, or puXX, but NOT vt52,
it will send the following string to the terminal:

```
\033[c\005
```

## Returns

```
TERMID=' <escaped sequence> '; export TERMID; 
```

Where sequence MAY be on of:

| Sequence                        | Observed Terminal             |
|:--------------------------------|:------------------------------|
| `\033/1;2c`                     | Apple Terminal.app (Catalina) |
| `\033[?1;0c`                    | Microsoft Terminal (store)    |
| `\033[?65;1;9c`                 | Gnome Terminal                |
| `\033[?65;1;9c`                 | lxterminal (Linux)            |
| `\033[?64;1;2;6;9;15;18;21;22c` | xterm (Linux)                 |
| `\033[?1;2c\033[?1;2c`          | rxvt (Linux)                  |
| `\033[?6cPuTTY`                 | PuTTY (Windows)               |
| `\033[?6c`                      | xvt (Linux)                   |
| `\033[?6c`                      | Linux console (Ubuntu 20.02)  |

*NOTE* That at least under recent releases, `xterm` is claiming to emulate
the features of a VT400 series terminal with 8 available options, see below.
An `xterm` can be complied to emulate other options and other terminal
types.

# Some Technical History

Even before computers were a thing, as far back as 1835, engineers were
working on what would become telegraphs.  Samuel Morse patented a machine
capable of recording recieving letters over a closed circuit.  Over the next 
80 years, refinements were made until there was a Teleprinter capable of
sending and receiving characters without the need to manually fine-tune 
clockwork timing mechanisms on both sides.

Versions of this from the 1850s and on were used to transmit news between
cities, and they printed everything onto thin strips of paper, all in a row,
but other places can talk about "ticker tape" better than I can.

So 1908 brings the first Teleprinter capable of automatic synchronization 
using start and stop signals between characters.  These became relatively 
common for use between offices of large companies and governments by 1930.

Notably, the company born from the start/stop signalling would later rename
themselves Teletype, which much like Kleenex, would become the word that
got used to describe all Teleprinter devices (and is still used as the
name of a device on a computer that allows textual interaction).

1924 brought the first common communication standard to be put in actual 
common use, the [ITU2](https://en.wikipedia.org/wiki/Baudot_code#ITA2) 
standard came into existence.  And the version of this used in Europe 
included a spot for a control character called `ENQ` (the US variant, 
called `US TTY` was based on the same standard would print a "$" when
recieved).

Some Teleprinters included a key or button with WRU (Who are You) or "Here Is"
where WRU was typically printed on and a varient of the `E` key.  This was
put on teleprinters as a way to find if the remote side was on and listening,
and each machine could be "programmed" by removing tabs from a rotating drum.
Manufacturers had either 20 or 22 characters of customizable response.
The `ENQ` function ended up in ASCII since its 1963 introduction.

When DEC started building Video Terminals (VT), the circuitry to respond to 
`ENQ` was considered too slow, so this signal was ignored (VT05, VT50, etc),
and the first DEC VT to respond to `ENQ` with a customizable string was the
VT100 model.

**Jumping to modern terminal emulators**, Simon Tatham's PuTTY has
a programmable response to `ENQ`, which defaults to `PuTTY` out of the box.
iTerm2 on macOS also has a programmable response, and its default is empty,
just like a Teleprinter when shipped new.  The terminal `rxvt` also responds
to `ENQ`, but not with a customizable response (more below).

The first DEC Video Terminal, the VT05, did not have any feature or function
that would auto-respond back to a host computer in any way.  However, just
as Teleprinter manufactures of the early 20th century had figured out, this
simple idea was important.  When they did introduce this idea into their
next model, the VT50, instead of using `ENQ`, DEC instead did a 
non-customizable per-model response to a two-character sequence, '\033Z`, 
called *DECID*.  See [Observed Output](./Observed_Output.md) for specifics.

The three character sequence, `\033[c`, *Device Attributes* (DA):  was added 
by Digital Equipment Corp for its VT100 line of terminals.  This sequence is 
fairly standard for *soft terminals*.  DEC terminals from the VT100 through 
the VT102, still replied to `\033Z` *DECID*, but from the VT200 series on, 
it would only respond to *DECID* if it were specifically in VT50 
compatibilty mode.  The VT200 series introduced *Secondary DA*, and renamed
Device Attributes to *Primary DA*.

Every terminal emulator I have tested responds to *Primary DA* with a string
that looks like a valid DEC *Primary DA* response.  The terminal emulator
package `rxvt` responds to `ENQ` with its *Primary DA* response.

## Term Read and `-t`

My own testing has shown that it is harmless to send *Primary DA* and *ENQ* 
in one string.  Doing this has the benefit of sniffing out PuTTY (which has 
many quirks of its own), and `rxvt` which will essentially respond twice.

# Actual Code Meanings

While researching this program (but mostly the documentation),
I did a lot of searching through archives of manuals on various
serial terminals produced by Digital Equipment Corp (DEC), and
took notes, which appear below.  The info that dosn't fit here
ended up on the [VT history](VT_History.md) page.

# VT Terminals

The following is scraped out of old DEC manuals and xterm source code.

With the exception of VT50 (which only responded to the DECID sequence,
never to *primary DA* sequence, the return code starts with these two bytes
( called *CSI* in the DEC docs ):

```
    \033[       # ESC + [
```

See [Observed Output](./Observed_Output.md) for return codes.

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

### Options

| Availility | Option Number | Definition |
|----|----|----|
| from 220   | `1` | 132 columns |
| from 220   | `2` | printer port |
| 240 .. 340 | `3` | ReGIS graphics |
| 240 .. 382 | `4` | Sixel graphics |
| from 220   | `6` | selective erase |
| VT550 (note) | `7` | DRCS |
| from 220   | `8` | UDK (User Defined Keys) |
| from 220   | `9` | 7-bit national replacement characters avail. |
| VT550      | `12` | Yugoslavian (SCS) |
| from 220   | `15` | Technical character set |
| from 440   | `17` | Terminal state interrogation |
| from 440   | `18` | Windowing capability |
| from 440   | `21` | Horizontal scrolling |
| from 525   | `22` | ANSI color |
| VT550      | `23` | Greek |
| VT550      | `24` | Turkish |
| ? xterm ?  |  28  | rectangular editing |
| ? xterm ?  |  29  | ANSI text locator |
| VT550      | `42` | ISO Latin-2 character set |
| VT550      | `44` | PCTerm |
| VT550      | `45` | Soft key map |
| VT550      | `46` | ASCII emulation |

*NOTE* VT550 means options found in the DEC manual for VT550, but
could have been present in other terminals before or after.

