# termread -b

Request Terminal Background Color

Sends the VT control strings, and recieves back the color of the terminal
background (sometimes).

Returns whatever is returned (octal escaping control characters) in a
bourne shell compatible variable string in the format:

```
TERM_BG='string'; export TERM_BG; 
```

## Sends

Send the following string to the terminal:

```
\033]11;?\033\
```

## Returns

If supported, the following will be returned.

```
TERM_BG=' <escaped sequence> '; export TERM_BG; 
```

Where escaped sequence looks like the following:

```
\033]11;rgb:RRRR/GGGG/BBBB\033\
```

|----|-----|
| RRRR | 2 hex digits for red, repeated once, like 2f2f
| GGGG | 2 hex digits for green, repeated
| BBBB | 2 hex digits for green, repeated

TERM_BG is the default for `-b`, --var can change this.

## Terminals Observed without support

* PuTTY
* Microsoft Terminal
* xvt (Ubuntu 18)

