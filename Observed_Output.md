# Observed Outputs

# Linux

| Terminal | -t | -2 | -bg | -c1 | Version Info |
|----------|----|----|-----|-----|--------------|
| Gnome Terminal | `\033[?65;1;9c` | `\033[>65;6003;1c` | YES | YES | VTE version 0.60.3 |
| lxterminal     | `\033[?65;1;9c` | `\033[>65;6003;1c` | YES | YES | VTE version 0.60.3 |
| xvt     | `\033[?6c` | `\033[?6c` | nil | nil | 2.1-20.3ubuntu2 |
| xterm   | `\033[?64;1;2;6;9;15;18;21;22c` | `\033[>41;353;0c` | YES | YES | XTerm(353) |
| console | `\033[?6c` | nil | ? | ? | Ubuntu 20.02  |

# macOS

| Terminal | -t | -2 | -bg | -c1 | Version Info |
|----------|----|----|-----|-----|--------------|
| Terminal.app | `\033/1;2c` | `\033[>1;95;0c` | YES | YES | Catalina 10.x |
| Terminal.app | `\033/1;2c` | `\033[>1;95;0c` | YES | YES | Big Sur 11.x |
| Terminal.app | `\033/1;2c` | `\033[>1;95;0c` | YES | YES | Ventura 13.x |

# Windows

It is disappointing that there is no difference in responses between
the default Windows Console Host and Windows Terminal as
Windows Terminal can display Emoji and has a larger set
of capabilities.

| Terminal | -t | -2 | -bg | -c1 | Version Info |
|----------|----|----|-----|-----|--------------|
| Win10 cmd        | `\033[?1c` | `\033[>0;10;1c` | nil | nil | Win 10 or 11 |
| Windows Terminal | `\033[?1c` | `\033[>0;10;1c` | nil | nil | 1.11.2921.0 |
| PuTTY      | `\033[?6cPuTTY` | `\033[>0;136;0c` | nil | nil | 0.70 |
