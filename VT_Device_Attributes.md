# Introduction

I have read a LOT of manuals for Digital Equipment Corporation for
clues about why things work the way they work.  This is a summary
of official DEC documentation for responses to the question,
Who Are You?

# VT50 Series (actual): DECID Responses

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

# VT100 and later (in VT52 compatible mode): DECID Response

| Terminal        | Reponse  | Version Info   | NOTES |
| --------------- |----------|----------------|-------|
| VT100 and newer | `\033/Z` | Emulation Mode | 24 Lines x 80 Cols |

    > NOTE: `xterm` in VT52 emulation mode also responds with this
    same string.  This is correct, to mean "not a real VT52, but a
    later device in compatability mode".  There are places where this
    response is claimed as being the actual response for a VT52
    terminal, which is misleading and is one of the things that led
    me to read every DEC Video Terminal Manual I could find.

# VT100 Series: Device Attributes and DECID Responses

| Terminal | Response        |
|----------|-----------------|
| VT100    | `\033[?1;<Ps>c` |
| VT132    | `\033[?4;<Ps>c` |
| VT102    | `\033[?6c`      |

## VT100 Options \<Ps>

The \<Ps> is descibed as

| Number | Option Desciption  |
|--------| ------------------ |
| 1      | Standard Terminal Port processor option (STP) |
| 2      | Advanced Video Option (AVO)    |
| 3      | STP and AVO                    |
| 4      | Graphics Processor Option (GO) |
| 5      | STP and GO                     |
| 6      | AVO and GO                     |
| 7      | AVO, STP and GO                |
| 11     | VT|XX-AC                       |
| 15     | VT|XX-AC and Graphics Processor |

    > NOTE: A true VT100 with NO OPTIONS would respond
    with `\033[?1;0c.

    > NOTE: The LA120, a Teleprinter product from DEC
    responds to *Device Attributes* with `\033[2c`.

# VT200 Series: Device Attributes

| Terminal | Response        |
|----------|-----------------|
| VT220    | `\033[?62;<Ps>;<...>c` |

# VT200 Options \<Ps>

Unlike the VT100 in which all options were represented by a single
position, the VT200 (and newer) series continue listing numeric
feature codes in ascending order, separated by semicolons ` ; `.

| Number | Feature Desciption  |
|--------| ------------------- |
| 1      | 132 column mode     |
| 2      | Printer Port        |
| 6      | Selective Erase     |
| 7      | DCRS                |
| 8      | UDK                 |

    > Note the VT220 introduces 8-Bit native for the first time

# References

**VT5x DECID**

* VT50 -> VT52 responses found in
`DECscope User's Manual`, publication `EK-VT5X-OP-001` (March 1977)
on page 22 (26 in PDF).

    * https://vt100.net/dec/ek-vt5x-op-001.pdf

* VT55 response found in the
`VT55-E, F, H, J DECgraphic Scope Users' Manual`,
publication `EK-VT55E-TM-001` (1976) on page 5-19 (65 in PDF)

    * https://vt100.net/dec/ek-vt55e-tm-001.pdf

**VT100 in VT52 Compatible Mode**

* `VT100 Series Technical Manual` (which covers VT100 through VT135),
publication `EK-VT100-TM-002` (Sept 1980) on page A-16 (308 in PDF).

    * https://vt100.net/dec/ek-vt100-tm-002.pdf

**VT100 Series: Device Attributes and DECID Responses**

* `VT100 Series Technical Manual` (which covers VT100, VT101 and
    VT135), publication `EK-VT100-TM-002` (Sept 1980) on page
    A-18 (310 in PDF)

    * https://vt100.net/dec/ek-vt100-tm-002.pdf

* `VT1XX-AC User Guide` (covers the serial printer interface option
    for the VT100, VT101 and VT132) `EK-VT1AC-UG-002` (Oct 1980) on
    page 48 (58 in PDF)

    * https://vt100.net/dec/ek-vt1ac-ug-002.pdf

* `VT102 Video Terminal User Guide` (specific to the VT102)
    publication `EK-VT102-UG-003` (June 1982) on
    page 202 (236 in PDF)

    * https://archive.org/details/h42_HP_VT102_Video_Terminal_User_Guide

* `LA120 Technical Manual`
publication `EK-LA120-TM-001` on page 3-5 (49 in PDF)

* https://vt100.net/dec/ek-la120-tm-001.pdf

**VT200 Series: Device Attributes**

* `VT220 Programmer Reference Manual` publication `VT220-RM-001` on
    page 4-48 (88 in PDF)

    * https://www.vt100.net/dec/ek-vt220-rm-001.pdf

