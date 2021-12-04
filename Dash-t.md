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

| Sequence                        | Observed Terminal          |
|:--------------------------------|---------------------------:|
| `\033/1;2c`                     | Apple Terminal.app         |
| `\033[?1;0c`                    | Microsoft Terminal (store) |
| `\033[?65;1;9c`                 | Gnome Terminal             |
| "                               | lxterminal (Linux)         |
| `\033[?64;1;2;6;9;15;18;21;22c` | xterm (Linux)              |
| `\033[?1;2c\033[?1;2c`          | rxvt (Linux)               |
| `\033[?6cPuTTY`                 | PuTTY (Windows)            |
| `\033[?6c`                      | xvt (Linux)                |
| `\033[?6c`                      | Linux hw console           |

# Behind the scenes

The sequence, primary DA: `\033[c`, was added by Digital Equipment Corp for its vt220 (and later) line of terminals.  This sequence is fairly standard for Xterm compatible _soft terminals_.  Old DEC VT terminals, up to vt102, replied to `\033Z` (DECID), but VERY FEW soft terminals respond to this.

The character `\005` is ASCII ENQ (enquiry), and is not a vt control sequence.   PuTTY, by default, will respond to this with the string `PuTTY`, but this can be configured (in PuTTY) per-connection.

My own testing, so far, has shown that it is harmless to push them both at the same time.  It quickly singles out PuTTY (which has many quirks of its own), and so far, `rxvt` is the only other that responds to ENQ, with the same string as primary DA, so it just leaves the same string repeated twice.

# Actual Code Meanings

This is scraped out of old DEC manuals.

| Code | Definition |
|----|----|
| `\033[?1;2c` | VT-100 terminal |
| `\033[?1;0c` | VT-101 terminal |
| `\033[?6c` | VT-102 terminal |
| `\033[?62;...c` | VT-220 terminal with... |
| `\033[?62;1;...c` | ... 132 columns |
| `\033[?62;2;...c` | ... printer port |
| `\033[?62;6;...c` | ... selective erase |
| `\033[?62;7;...c` | ... DRCS |
| `\033[?62;8;...c` | ... UDK |
| `\033[?62;9;...c` | ... 7-bit national replacement characters avail. |
| `\033[?64;...c` | VT-550 terminal with... |
| `\033[?64;12;...c` | ... Yugoslavian (SCS) |
| `\033[?64;15;...c` | ... Technical character set |
| `\033[?64;18;...c` | ... Windowing capability |
| `\033[?64;21;...c` | ... Horizontal scrolling |
| `\033[?64;23;...c` | ... Greek |
| `\033[?64;24;...c` | ... Turkish |
| `\033[?64;42;...c` | ... ISO Latin-2 character set |
| `\033[?64;44;...c` | ... PCTerm |
| `\033[?64;45;...c` | ... Soft key map |
| `\033[?64;46;...c` | ... ASCII emulation |

VT-220 and VT-550 share capability codes up to 9, and I have to assume that 
xterm has done some co-opting of this in expanding the definitions in its
responses.

Try as I might, I've not found capability code 22 (yet?).
