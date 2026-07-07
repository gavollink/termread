# Primary Device Attributes Master List

This is the list of attributes collected from all sources.

See [VT Device Attributes](./VT_Device_Attributes.md) for more in
depth explanation.

## Basic Anatomy

| CSI     | ? | VT Line | ; | Ps  | c |
|---------|---|---------|---|-----|---|
| `\033[` | ? | 65      | ; | 1;2 | c |

## Capabilities (Ps)

| Origin | Number | Description |
|--------|--------|-------------|
| VT200  | 1      | 132 column mode                  |
| VT200  | 2      | Printer Port                     |
| VT200  | 6      | Selective Erase                  |
| VT200  | 7      | DCRS                             |
| VT200  | 8      | UDK (User Defined Keys)          |
| VT200  | 9      | National Replacement Character Set |
| VT300  | 13     | Local Editing Mode               |
| VT300  | 15     | Technical Character Set          |
| VT300  | 16     | Locator Device Port              |
| VT300  | 18     | Windowing                        |
| VT300  | 19     | Dual Sessions                    |
| VT400  | 21     | Horizontal Scrolling             |
| VT500  | 22     | ANSI (Indexed) Color             |
| VT500  | 23     | Greek                            |
| VT500  | 24     | Turkish                          |
| VT500  | 42     | ISO Latin-2                      |
| VT500  | 44     | PCTerm                           |
| VT500  | 45     | Soft key mapping                 |
| VT500  | 46     | ASCII terminal emulation         |
| ??     | 28     | Rectangluar Editing              |
| ??     | 32     | Text macros                      |
| ??     | 52     | Can interact with system clipboard |
| Contour | 314   | Screen Capture                   |

## Ever Incomplete List

More and more, terminals are catching on that these number are fluid,
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

