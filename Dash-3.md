# termread -3

Request Terminal Unit Identification.

This was introduced with the VT400 series,
and was not present before, and many terminal
emulators ignore it.

## Sends

```
\033[=c
```

## Returns

```
TERM3DA=' <escaped sequence> '; export TERM3DA
```

Sequence is expected to look like the following:

```
\033P!|00010205\033\
```

According to the VT400 Programmers Manual,
after the pipe (|) there are 8 digits, to be
read in 4 sets of 2 each.

The VT520 Programmers Manual specifies that
these are HEX DIGITS, so expect HEX when
programming.

The first two are the "site of manufacture".

The next three are a unit unique identifier.
The example above (from that VT420 doc) says,

> The terminal was
> manufactured at site 00 and
> has a unique ID number of
> 125.

The example above (from that VT520 doc) says,

> The terminal was
> manufactured at site 00
> and has a unique ID number
> of 010205.

# References

* `VT420 Programmer Reference Manual`
  publication `EK-VT420-RM.002` on page 234 (256 in PDF)

    * https://manx-docs.org/collections/mds-199909/cd3/term/vt420rm2.pdf

* `VT520/VT525 Programmer Informaion` publication `EK-VT520-RM.A01`
  on page 5-12 (166 in PDF)

    * https://vt100.net/mirror/mds-199909/cd3/term/vt520rma.pdf
