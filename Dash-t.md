# termread -t

Sends two control strings to the terminal, and recieves back identifying
information, though the string may not be unique.

## Sends

So long as the TERM environment variable identifies as
vt*, xt*, or pu* it will send the following string to the terminal:

```
\033[c\005
```

## Returns

```
TERMID=' <escaped sequence> '; export TERMID; 
```

Where sequence MAY be on of:

| Sequence | Observed Terminal |
| \033/1;2c | Apple Terminal.app |
| \033[?1;0c | Microsoft Terminal (store) |
| \033[?65;1;9c | Gnome Terminal |
| \033[?64;1;2;6;9;15;18;21;22c | xterm (Linux) |
| \033[?6c | xvt (Linux) |

