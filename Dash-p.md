# termread -p \<str>

Request Custom

Sends the supplied string, and recieves back whatever the terminal
might respond with (octal escaping control characters) in a
Bourne shell compatible variable string in the format:

```
READ='string'; export READ;
```

## Send \<str>

The supplied string will deal with the following Backslash escapes:

| \\\\ | \\  | (\\0134) |
| \\a  | BEL | (\\007)  |
| \\b  | BS  | (\\008)  |
| \\e  | ESC | (\\033)  |
| \\f  | FF  | (\\014)  |
| \\n  | NL  | (\\012)  |
| \\n  | CR  | (\\015)  |
| \\t  | TAB | (\\011)  |
| \\v  | VT  | (\\013)  |
| \\0NNN | Octal Expansion (1 to 3 digits) |
| \\xXX | Hex Expansion (1 or 2 characters) |

This does NOT do `%` positionals (yet?).

## Return

If anything is returned it will be returned like this:

```
READ='<sequence>'; export READ;
```

READ is the default for `-p`, the --var argument can change this.
