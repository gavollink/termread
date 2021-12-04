# termread -t

Sends two control strings to the terminal, and recieves back identifying
information (sometimes).

Resturns whatever is returned (octal escaping control characters) in a
bourne shell compatible variable string in the format:

```
TERMID='string'; export TERMID; 
```

## Sends

So long as the TERM environment variable identifies as
vtXX, xtXX, or puXX it will send the following string to the terminal:

```
\033[c\005
```

## Returns

```
TERMID=' <escaped sequence> '; export TERMID; 
```

Where sequence MAY be on of:

| Sequence                        | Observed Terminal          |
|:--------------------------------|---------------------------:|
| `\033/1;2c`                     | Apple Terminal.app         |
| `\033[?1;0c`                    | Microsoft Terminal (store) |
| `\033[?65;1;9c`                 | Gnome Terminal             |
| `\033[?65;1;9c`                 | lxterminal (Linux)         |
| `\033[?64;1;2;6;9;15;18;21;22c` | xterm (Linux)              |
| `\033[?1;2c\033[?1;2c`          | rxvt (Linux)               |
| `\033[?6cPuTTY`                 | PuTTY (Windows)            |
| `\033[?6c`                      | xvt (Linux)                |
| `\033[?6c`                      | Linux hw console           |

*NOTE* That at least under Ubuntu, `xterm` is claiming to emulate
the features of a VT400 series terminal with 8 available options, see below.
An `xterm` can be complied to emulate other options and other terminal
types.

# Behind the scenes

The sequence, *primary DA*: `\033[c`, was added by Digital Equipment Corp for its vt100 (and later) line of terminals.  This sequence is fairly standard for Xterm compatible _soft terminals_.  Old DEC VT terminals, up to vt102, replied to `\033Z` (DECID), but VERY FEW soft terminals respond to this.

The character `\005` is ASCII ENQ (enquiry), and is not a vt control sequence.   PuTTY, by default, will respond to this with the string `PuTTY`, but this can be configured (in PuTTY) per-connection.

My own testing, so far, has shown that it is harmless to push them both at the same time.  It quickly singles out PuTTY (which has many quirks of its own), and so far, `rxvt` is the only other that responds to ENQ, with the same string as *primary DA*, so it just leaves the same string repeated twice.

# Actual Code Meanings

# VT Terminals

The following is scraped out of old DEC manuals and xterm source code.

With the exception of VT50 (which only responded to the DECID sequence,
never to *primary DA* sequence, the return code starts with these two bytes
( called *CSI* in the DEC docs ):

```
    \033[       # ESC + [
```

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


## VT50 Series, the VT exception

*Not supported by this software*

VT50, 52, 55 and 62 terminals ONLY responded to DECID ( `\033Z` ).
This option doesn't use DECID because VT52 terminals are exceedingly rare,
and I haven't found any soft terminals that respond to DECID,
but not *Primary DA*.

Exception: xterm itself CAN be forced into vt52 compatibility mode, and
in that case alone, this code will ONLY be returned from DECID.  That
means this option would not illicit a response from `xterm` in that odd
scenario.

| Action | Code | Definition |
|----|----|----|
| Send    | `\033Z`  | DECID sequence |
| Recieve | `\033/Z` | VT52 response |

## VT05, dumb

Introduced in 1970, before ASCII had full adoption, 
I haven't seen anything that knows how to talk to one. 
It doesn't even have an entry in the termcap database, so modern
systems are clearly not expected to see one.

I found a 'tiff' file of the documenation for one of these, and it appears
to be a dumb terminal, with no support for escape codes.

Some characters do have ... unexpected meanings though:

| Char | Action |
|---|---|
| 9  | Move cursor down (No tab key?) |
| 14 | Cursor Addressing |
| 24 | Advance cursor right |
| 26 | Move cursor up |
| 27 | ALT (No Esc key?) |
| 30 | Erase from cursor to end of line |
| 31 | Erase screen |

Worse, most of these need NULL padding after them (depending on the
BAUD rate) because the circuitry couldn't do the operation and keep
up with ongoing communication stream).

