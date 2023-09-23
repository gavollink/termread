# Observed Outputs

# Linux

| Terminal | -t | -2 | -bg | -c1 | Version Info |
|----------|----|----|-----|-----|--------------|
| Gnome Terminal | `\033[?65;1;9c` | `\033[>65;6003;1c` | YES | YES | VTE version 0.60.3 |
| lxterminal     | `\033[?65;1;9c` | `\033[>65;6003;1c` | YES | YES | VTE version 0.60.3 |
| xvt     | `\033[?6c` | `\033[?6c` | nil | nil | 2.1-20.3ubuntu2 |
| xterm   | `\033[?64;1;2;6;9;15;18;21;22c` | `\033[>41;353;0c` | YES | YES | XTerm(353) |
| console | `\033[?6c` | nil | ? | ? | Ubuntu 20.02  |
| xterm -ti vt52  | `\033/Z` | NO | NO | NO | XTerm(372) VT52 Emulation |

NOTE: xterm in vt52 mode responds with the string of a vt05 which must have
been a mistake.

# macOS

| Terminal | -t | -2 | -bg | -c1 | Version Info |
|----------|----|----|-----|-----|--------------|
| Terminal.app | `\033[?1;2c` | `\033[>1;95;0c` | YES | YES | Catalina 10.x |
| Terminal.app | `\033[?1;2c` | `\033[>1;95;0c` | YES | YES | Big Sur 11.x |
| Terminal.app | `\033[?1;2c` | `\033[>1;95;0c` | YES | YES | Ventura 13.x |
| KiTTY.app    | `\033[?62;c` | `\033[>1;4000;30c` | YES | YES | 0.30.0 |
| iTerm2.app   | `\033[?62;4c` | `\033[>0;95;0c` | YES | YES | 3.4.20 |

# Windows

It is disappointing that there is no difference in responses between
the default Windows Console Host and Windows Terminal as
Windows Terminal can display Emoji and has a larger set
of capabilities.

| Terminal             |  -t             |  -2           | -bg  |  -c1 | Version Info |
| ----------           | ----            | ----          |----- | -----|--------------|
| Windows Console Host | `\033[?1c`      | `\033[>0;10;1c` | nil | nil | Win 10 or 11 |
| Windows Terminal     | `\033[?1c`      | `\033[>0;10;1c` | nil | nil | 1.11.2921.0 |
| PuTTY                | `\033[?6cPuTTY` | `\033[>0;136;0c` | nil | nil | 0.70 -> 0.76 |

# Not-Observed But Expected Outputs

| Terminal | -t | -2 | -bg | -c1 | Version Info | NOTES |
|----------|----|----|-----|-----|--------------|-------|
| vt50     | `\033A` | nil | nil | nil | DECscope VT50 Ax (or Cx) | Displays Uppercase Only, 12 Lines x 80 Cols |
| vt50h    | `\033H` | nil | nil | nil | DECscope VT50 Hx | Displays Uppercase Only, 12 Lines x 80 Cols |
| vt50j    | `\033J` | nil | nil | nil | DECscope vt50 Jx (with copier) | Displays Uppercase Only, 12 Lines x C80 ols |
| vt52     | `\033K` | nil | nil | nil | DECscope vt52 Ax (or Cx) | 24 Lines x 80 Cols |
| vt52b    | `\033L` | nil | nil | nil | DECscope vt52 Bx (with copier) | 24 Lines x 80 Cols |
| vt55     | `\033E` | nil | nil | nil | DECscope vt55 | 24 Lines x 80 Cols |

vt50/52 outputs from https://vt100.net/dec/ek-vt5x-op-001.pdf (page 22).

# Tertiary DA (WIP)

Eventually this will show up in the tables above, but for now I'm
just grabbing the info.

| Terminal            | -3                     |
| ------------------- | ---------------------  |
| Windows Terminal    | `\033P!|00000000\033\` |

