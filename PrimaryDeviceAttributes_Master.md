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
| xterm  | 28     | Rectangluar Editing              |
| ??     | 32     | Text macros                      |
| ??     | 52     | Can interact with system clipboard |
| Contour | 314   | Screen Capture                   |

