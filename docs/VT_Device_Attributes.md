# Device Attributes / DA1 / Primary Device Attributes

## Introduction

Most simply, Device Attributes is a way for a running program to ask a
terminal what it is capable of displaying or doing.  It started out
much simpler than this, but that is the start.

## The DEC VT05

There was no query to a DEC VT05 that would give any clue as to what
device was at the end of the line.

According to a reference to the VT05 from the PDP-8/e documentation,
the VT05 is "fully compatible with the Teletype Model 33", however,
the VT05 did not have a programmable response to the `ENQ` key (verified
from the Reference Manual).

The VT05 would safely ignore the `ENQ` character, just like a
Teletype Model 33 fresh out of the box with its response string
left unset.  Interestingly, it would ignore `ENQ` *faster* than an
actual unmodified `TTY3 3`.

## VT50 Series (actual): DECID Responses

The DECID Sequence, as described in the VT5x and VT55 manuals
are as follows:

| Terminal | Response | Version Info | NOTES |
| -------- | -------- |--------------|-------|
| VT50     |  `\033/A` | DECscope VT50 Ax (or Cx) | Displays Uppercase Only, 12 Lines x 80 Cols |
| VT50h    |  `\033/H` | DECscope VT50 Hx | Displays Uppercase Only, 12 Lines x 80 Cols |
| VT50j    |  `\033/J` | DECscope VT50 Jx (with copier) | Displays Uppercase Only, 12 Lines x 80 Cols |
| VT52     |  `\033/K` | DECscope VT52 Ax (or Cx) | 24 Lines x 80 Cols |
| VT52b    |  `\033/L` | DECscope VT52 Bx (with copier) | 24 Lines x 80 Cols |
| VT55     |  `\033/C` | VT55 response (s1) |
| VT55E    |  `\033/E` | DECgraphic Scope VT55 | 24 Lines x 80 Cols |

### VT52 Compatibility Mode in Later Terminals

VT50 was never emulated, as the difference between them (mostly) was that
the 52 was the first to track and display the English 26 lowercase AND
uppercase letters.  Nobody was asking to emulate uppercase only.

| Terminal        | Reponse  | Version Info        | NOTES |
| --------------- |----------|---------------------|-------|
| VT100 and newer | `\033/Z` | VT52 Emulation Mode | 24 Lines x 80 Cols |

> **NOTE**: `xterm` in VT52 emulation mode also responds with this
  same string.  This is correct, to mean "not a real VT52, but a
  later device in compatability mode".  There are places where this
  response is claimed as being the actual response for a VT52
  terminal, which is misleading and is one of the things that led
  me to read every DEC Video Terminal Manual I could find.

## VT100 Series: Device Attributes and DECID Responses

    NOTE: No (xterm compatible) should actually respond as a VT1xx,
    but many do anyway.  At least VT100/VT132 allow an option list, BUT
    the VT1xx series options are NOT compatible with the VT200 and later
    which, because of 8-bit compatability and color, every current
    soft terminal actually is, even if their featureset is incomplete.

| Terminal | Response        | Notes |
|----------|-----------------|-------|
| VT100    | `\033[?1;<Ps>c` | First full feature terminal |
| VT131    | `\033[?4;<Ps>c` | Block Mode Varient |
| VT102    | `\033[?6c`      | Minimal Feature Set (cheap version) |

### VT100 Compatible Mode in (much) Later Terminals

```
\033[?61;<Ps>c
```

Claims to be VT100 compatible while sharing the VT200 and
later capability list.  (First found in the VT1000/VT1200 Manual).

> **NOTE**: the Ps list of a `?61` response does NOT come from the list
  immedately below, but the lists from VT200 and further down.

## VT100 Options \<Ps>

The \<Ps> is descibed as

| Number | Option Desciption  |
|--------| ------------------ |
| 1      | Standard Terminal Port processor option (STP) |
| 2      | Advanced Video Option (AVO)      |
| 3      | STP and AVO                      |
| 4      | Graphics Processor Option (GO)   |
| 5      | STP and GO                       |
| 6      | AVO and GO                       |
| 7      | AVO, STP and GO                  |
| 11     | VT\|XX-AC                        |
| 15     | VT\|XX-AC and Graphics Processor |

> **NOTE**: A true VT100 with NO OPTIONS would respond
  with `\033[?1;0c`, (`EK-VT100-UG-002`)

> **NOTE**: The LA120, a Teleprinter product from DEC
  responds to *Device Attributes* with `\033[?2c`.
  (`EK-LA120-UG-003`)

# VT200 Series: Device Attributes

| Terminal | Response        |
|----------|-----------------|
| VT220    | `\033[?62;<Ps>;<...>c` |

## VT200 Options

> **NOTE**: For a list of all capabilities in one table, see the
  [Primary Device Attributes Master List](./PrimaryDeviceAttributes_Master.md)

Unlike the VT100 in which all options were represented by a single
position, the VT200 (and newer) series continue listing numeric
feature codes in ascending order, separated by semicolons ` ; `.

| Number | Feature Desciption  |
|--------| ------------------- |
| 1      | 132 column mode     |
| 2      | Printer Port        |
| 6      | Selective Erase     |
| 7      | DCRS                |
| 8      | UDK (User Defined Keys) |
| 9      | National Replacement Character Set |

- The VT220 introduces 8-Bit native for the first time
- Option 9 is ONLY mentioned in the VT240 Programmer Reference Manual
  ('EK-VT240-RM-002`)

# VT300 Series: Device Attributes

| Terminal | Response        |
|----------|-----------------|
| VT330    | `\033[?63;<Ps>;<...>c` |

## VT300 Options

| Number | Feature Desciption  |
|--------| ------------------- |
| 1      | 132 column mode     |
| 2      | Printer Port        |
| 3      | Regis Graphics      |
| 4      | Sixel Graphics      |
| 6      | Selective Erase     |
| 7      | DCRS                |
| 8      | UDKs (User Defined Keys) |
| 9      | National Replacement Character Set |
| 13     | Local Editing Mode  |
| 15     | Technical Character Set |
| 16     | Locator Device Port |
| 18     | Windowing           |
| 19     | Dual Sessions       |

# VT400 Series: Device Attributes

| Terminal | Response        |
|----------|-----------------|
| VT420    | `\033[?64;<Ps>;<...>c` |

## VT400 Options

| Number | Feature Desciption  |
|--------| ------------------- |
| 1      | 132 column mode     |
| 2      | Printer Port        |
| 6      | Selective Erase     |
| 7      | Soft Character Set  |
| 8      | UDKs (User Defined Keys) |
| 9      | National Replacement Character Set |
| 15     | Technical Character Set |
| 18     | Windowing           |
| 19     | Dual Sessions       |
| 21     | Horizontal Scrolling |

# VT500 Series: Device Attributes

| Terminal | Response        |
|----------|-----------------|
| VT520    | `\033[?65;<Ps>;<...>c` |

## VT500 Options

Source: VT510 Video Terminal Programmer Information `EK-VT510-RM`

Also checked against the VT520/525 manual `EK-VT520-RM` (which is a subset
of the VT510 attributes)

| Number | Feature Desciption  |
|--------| ------------------- |
| 1       | 132 columns |
| 2       | Printer port |
| 4       | Sixel extension |
| 6       | Selective erase [^nr] |
| 7       | Soft character set (DRCS) |
| 8       | User-defined keys (UDKs) [^nr] |
| 9       | National replacement character sets (NRCS) |
| 12      | Serbo-Croatian (SCS) |
| 15      | Technical character set [^nr] |
| 18      | Windowing capability |
| 19      | Sessions |
| 21      | Horizontal scrolling |
| 22      | ANSI (Indexed) Color |
| 23      | Greek |
| 24      | Turkish |
| 42      | ISO Latin-2 |
| 44      | PCTerm |
| 45      | Soft key mapping |
| 46      | ASCII terminal emulation |

[^nr] Option exists, but is not reported.

# VT1000

The VT1000 was an X11 terminal, specifically meant to
run an X11R3 desktop session from a host server.
It could also still act as a character based terminal.
The VT1000, itself, did not have a unique `?6x` number,
but instead could emulate other DEC terminals.

This seems to be the only official DEC use of the `?61`
to refer to VT100 compatible using VT200+ capabilities.

| Number  | Feature Desciption  |
|---------|---------------------|
| 1       | 132 columns                     |
| 2       | Printer port                    |
| 3       | ReGIS graphics                  |
| 4       | Sixel graphics                  |
| 6       | Selective erase [^nr]           |
| 7       | Soft character set (DRCS)       |
| 8       | User-defined keys (UDKs) [^nr]  |
| 9       | National replacement character sets (NRC) |
| 11      | 25th Status Line                |
| 14      | 8-bit architecture              |
| 15      | DEC technical set               |
| 16      | ReGIS locator                   |
| 17      | Terminal state reports          |
| 29      | ANSI text locator               |
| 39      | page memory extension           |


## DEC Internal Standards Document, 1992

Level, here, suggests that these features should not be available
for terminals below, or above the described range.  **NOTE**
Exceptions to this are marked `(imp)` for implied for compatability
for terminals reporting as above the range.

Note that 39 - 46, used in the VT5xx and VT1000 series are not
reflected in the 1992 standards doc.

Note that 10 and 11 are overloaded, having been used as Greek and
Turkish for country specific models of the VT2xx and VT3xx series.

| Number  | Level   | Feature Desciption                         |
|---------|---------|--------------------------------------------|
| 1       | 61 - 64 | 132 columns                                |
| 2       | 61 - 64 | Printer port                               |
| 3       | 61 - 64 | ReGIS graphics                             |
| 4       | 61 - 64 | Sixel graphics                             |
| 5       | 61 - 63 | Katakana                                   |
| 6       | 62 - 64 | Selective erase                            |
| 7       | 62 - 64 | Dynamically Redefinalble Character Sets (DRCS) |
| 8       | 62 - 64 | User-defined keys (UDKs)                   |
| 9       | 61 - 64 | National replacement character sets (NRCS) |
| 10      | 61 - 63 | Kanji                                      |
| 11      | 62      | Status Display `(imp)`                     |
| 12      | 61 - 63 | Serbo-Croatian                             |
| 13      | 61 - 63 | Block Mode                                 |
| 14      | 62      | 8-bit interface architecture `(imp)`       |
| 15      | 62 - 64 | Technical Character Set                    |
| 16      | 61 - 64 | Locator Port                               |
| 17      | 62      | Terminal State Interrogation `(imp)`       |
| 18      | 62 - 64 | Windowing                                  |
| 19      | 62 - 64 | Multiple Sessions (TD/SMP)                 |
| 20      | 61 - 64 | APL                                        |
| 21      | 62 - 64 | Horizontal Scrolling                       |
| 22      | 62 - 64 | Color Text                                 |
| 23      | 61 - 63 | Greek                                      |
| 24      | 61 - 63 | Turkish                                    |
| 25      | 62 - 63 | Arabic Bilingual Mode 1                    |
| 26      | 62 - 63 | Arabic Bilingual Mode 2                    |
| 27      | 62 - 63 | Arabic Bilingual Mode 3                    |
| 28      | 62 - 63 | Rectangular Editing                        |
| 29      | 62 - 64 | Text Locator                               |
| 30      | 62 - 63 | Hanzi                                      |
| 31      |         |         (reserved?)                        |
| 32      | 62 - 64 | Text Macros                                |
| 33      | 62 - 63 | Hangul and Hanja                           |
| 34      | 62 - 63 | Icelandic                                  |
| 35      | 62 - 63 | Arabic Bilingual with Text Controls        |
| 36      | 62 - 63 | Arabic Bilingual with no Text Controls     |
| 37      | 62 - 63 | Thai                                       |
| 38      | 62 - 63 | Character Outlining                        |


## Non-Official, Software Terminal Capabilities

| Number | Feature Desciption  |
|--------| ------------------- |
| 52     | Can interact with system clipboard [MS term] |
| 314    | Screen Capture [Contour-Terminal] |

- github user microsoft, project terminal (52):
  `src/terminal/adapter/adaptDispatch.cpp`,
  function `AdaptDispatch::DeviceAttributes()`
  - 52 is ALSO used by iterm2, ghostty, but I couldn't find definitive docs
- github user contour-terminal, project contour:
  `metainfo.xml`: "Adds VT sequence to capture the current screen buffer"...

## META Search Keyword Matching

- DA1
- Device Attributes
- Primary Device Attributes
- Primary DA
- Terminal Capabilities Report

## References

**VT05**

- `VT05 alphanumeric display terminal reference manual`
  publication `DEC-00-H4AC-D`
  - https://www.vt100.net/dec/vt05 (converted to HTML)

Another source of information on the VT05 is from the PDP-8/e
Small Computer Manual, Ch-7, Pg 52.  Only a FAX (tiff) scan is available.

- http://highgate.comm.sfu.ca/pdp8/8ehandbook/8ech7-p1.tif

***NOTE***: The source says the VT05 is fully compatible with the Teletype 33, which implies that receiving an ENQ will not harm the VT functions, but does NOT actually prove that it would have any response.

**VT5x DECID**

- VT50 -> VT52 responses found in
  `DECscope User's Manual`, publication `EK-VT5X-OP-001` (March 1977)
  on page 22 (26 in PDF).
  - https://vt100.net/dec/ek-vt5x-op-001.pdf

- VT55 response found in the
  `VT55-E, F, H, J DECgraphic Scope Users' Manual`,
  publication `EK-VT55E-TM-001` (1976) on page 5-19 (65 in PDF)
  - https://vt100.net/dec/ek-vt55e-tm-001.pdf

**VT100 in VT52 Compatible Mode**

- `VT100 Series Technical Manual` (which covers VT100 through VT135),
   publication `EK-VT100-TM-002` (Sept 1980) on page A-16 (308 in PDF).
  - https://vt100.net/dec/ek-vt100-tm-002.pdf

**VT100 Series: Device Attributes and DECID Responses**

- `VT100 Series Technical Manual` (which covers VT100, VT101 and
  VT135), publication `EK-VT100-TM-002` (Sept 1980) on page
  A-18 (310 in PDF)
  - https://vt100.net/dec/ek-vt100-tm-002.pdf

- `VT1XX-AC User Guide` (covers the serial printer interface option
  for the VT100, VT101 and VT132) `EK-VT1AC-UG-002` (Oct 1980) on
  page 48 (58 in PDF)
  - https://vt100.net/dec/ek-vt1ac-ug-002.pdf

- `VT102 Video Terminal User Guide` (specific to the VT102)
  publication `EK-VT102-UG-003` (June 1982) on
  page 202 (236 in PDF)
  - https://archive.org/details/h42_HP_VT102_Video_Terminal_User_Guide

- `LA120 Technical Manual`
  publication `EK-LA120-TM-001` on page 3-5 (49 in PDF)
  - https://vt100.net/dec/ek-la120-tm-001.pdf

**VT200 Series: Device Attributes**

- `VT220 Programmer Reference Manual` publication `VT220-RM-001` on
  page 4-48 (88 in PDF)
  - https://www.vt100.net/dec/ek-vt220-rm-001.pdf

* `VT240 Programmer Reference Manual` publication `VT240-RM-002` on
  page 101 (120 in PDF)
  - https://www.vt100.net/dec/ek-vt220-rm-001.pdf

**VT300 Series: Device Attributes**

- `VT320/VT340 Programmer Reference Manual, Vol 1: Text Programming`
   publication `EK-VT3XX-TP-002` on page 198 (212 in PDF)
   - https://vt100.net/dec/ek-vt3xx-tp-002.pdf

**VT400 Series: Device Attributes**

- `VT420 Programmer Reference Manual`
  publication `EK-VT420-RM.002` on page 230 (252 in PDF)
  - https://manx-docs.org/collections/mds-199909/cd3/term/vt420rm2.pdf

**VT500 Series: Device Attributes**

- `VT520/VT525 Programmer Information` publication `EK-VT520-RM.A01`
  on page 5-10 (164 in PDF)
  - https://vt100.net/mirror/mds-199909/cd3/term/vt520rma.pdf

**VT1000 Series: Device Attributes**

- `Installing and Using the VT1000 Video Terminal`
  publication `EK-VT1000-UG-002` on page 180-181 (190 in PDF)
  - https://vt100.net/mirror/mds-199909/cd3/term/v1000ug2.pdf

**DEC Internal Video Systems Reference Manual (1991)**

- `DEC STD 070 Video System Referenec Manual`
  publication `A-MN-ELSM070-00-0000 Rev H, 03-Dec-1991`
  on page 04-19 (223 in PDF)
  - https://archive.org/details/bitsavers_decstandar0VideoSystemsReferenceManualDec91_74264381
