# Primary Device Attributes Master List

This is the list of attributes collected from all sources.

See [VT Device Attributes](./VT_Device_Attributes.md) for more in
depth explanation.

## Basic Anatomy

| CSI     | ? | VT Line | ; | Ps  | c |
|---------|---|---------|---|-----|---|
| `\033[` | ? | 65      | ; | 1;2 | c |

## Capabilities (Ps)

| Origin | Number | Description                             |
|--------|--------|-----------------------------------------|
| VT200  | 1      | 132 column mode                         |
| VT200  | 2      | Printer Port                            |
| VT300  | 3      | ReGIS graphics                          |
| VT300  | 4      | Sixel graphics                          |
| DS1991 | 5      | Katakana                                |
| VT200  | 6      | Selective Erase                         |
| VT200  | 7      | DCRS                                    |
| VT200  | 8      | UDK (User Defined Keys)                 |
| VT200  | 9      | National Replacement Character Set      |
| (note) | 10     | Kanji                                   |
| (note) | 11     | Status Display                          |
| DS1991 | 12     | Serbo-Croatian                          |
| VT300  | 13     | Local Editing Mode                      |
| VT1000 | 14     | 8-bit interface architecture            |
| VT300  | 15     | Technical Character Set                 |
| VT300  | 16     | Locator Device Port                     |
| VT1000 | 17     | Terminal State Interrogation            |
| VT300  | 18     | Windowing                               |
| VT300  | 19     | Dual Sessions                           |
| DS1991 | 20     | APL   # (note)                          |
| VT400  | 21     | Horizontal Scrolling                    |
| VT500  | 22     | ANSI (Indexed) Color                    |
| VT500  | 23     | Greek                                   |
| VT500  | 24     | Turkish                                 |
| DS1991 | 25     | Arabic Bilingual Mode 1                 |
| DS1991 | 26     | Arabic Bilingual Mode 2                 |
| DS1991 | 27     | Arabic Bilingual Mode 3                 |
| DS1991 | 28     | Rectangular Editing                     |
| VT1000 | 29     | Text Locator                            |
| DS1991 | 30     | Hanzi                                   |
| DS1991 | 31     |         (reserved?)                     |
| DS1991 | 32     | Text Macros                             |
| DS1991 | 33     | Hangul and Hanja                        |
| DS1991 | 34     | Icelandic                               |
| DS1991 | 35     | Arabic Bilingual with Text Controls     |
| DS1991 | 36     | Arabic Bilingual with no Text Controls  |
| DS1991 | 37     | Thai                                    |
| DS1991 | 38     | Character Outlining                     |
| VT1000 | 39     | page memory extension                   |
| VT500  | 42     | ISO Latin-2                             |
| VT500  | 44     | PCTerm                                  |
| VT500  | 45     | Soft key mapping                        |
| VT500  | 46     | ASCII terminal emulation                |
| ??     | 52     | Can interact with system clipboard      |
| Contour | 314   | Screen Capture                          |

**DS1991** is an internal DEC Document for future VT development that
has been available on the Internet for a while.  Many feature codes were
never observed in terminals, but had been documented here.
That document notes that 10 and 11 are overloaded, having been used as
Greek and Turkish for country specific models of the VT2xx and VT3xx
series, which I could not otherwise find documentation for.

**APL** I can only assume this is some special version that supported
the special characters needed by `A Programming Language` (APL), but
there is zero explanation, nor have I found any product that actually
shipped with this flag.  It is documented, thus it is here.

**??** 52 showed up in `iTerm2`, and `Microsoft Terminal`, maybe others.

**Contour Terminal** so far this is the only one I've run into that
implements or reports on Screen Capture capability.

## Ever Incomplete List

Terminal emulator developers are catching on that these number are fluid,
and really, there aren't that many unique terminal programs out there
(so many use a common library, like Konsole or VTE), so
add a number, and someone else is likely to pick it up too.

Anyway, I have heard tell of several other numbers, BUT, never with any
pointers to exactly what terminal actually uses it, and I'm very
wary of the circular reference problem on the Internet.

You know when some guy on Reddit says he saw it on vt100.net that has
a footnote suggesting that terminal.computer said it, and when pressed,
they think they saw it on Reddit.

Anyway, feel free to reach out if you have a pointer to a number I
don't have with an actual reference, even if it is in source code.

