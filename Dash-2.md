# termread -2

Request Terminal Version (secondary DA)

Sends one control string to the Terminal.

## Sends

```
\033[<c
```

## Returns

```
TERM2DA=' <escaped sequence > '; export TERM2DA; 
```

Sequence is expected to look like the following:

```
\033[>77;8888;9c
```

The first position (77;) is meant to match position one of **primary DA**,
though this is not reliable.

The second position (8888;) is the version of the terminal firmware.

The third position (9) is meant to be an option marker.

Sequence MAY be one of:

| Response sequence  | Observed Terminal             | Version info   |
|:-------------------|:------------------------------|:---------------|
| `\033[>1;95;0c`    | Apple Terminal.app (Catalina) | 2.10 (433)  |
| `\033[>0;10;1c`    | Windows Terminal (Windows 10) | 1.11.2921.0 |
| `\033[>65;6003;1c` | Gnome Terminal                | VTE version 0.60.3 |
| `\033[>65;6003;1c` | lxterminal                    | VTE version 0.60.3 |
| `\033[>41;353;0c`  | xterm                         | XTerm(353) |
| `\033[>0;136;0c`   | PuTTY (Windows)               | 0.70 |
| (no response)      | Linux hw console              | Ubuntu 20.04 |
| `\033[?6c`         | xvt (Ubuntu 18.04)            | 2.1-20.3ubuntu2 |


### response notes

* xvt responds to -2 and -t with the same string.

