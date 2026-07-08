# termread -e

Enquiry

This is one of the oldest "who are you?" characters, and comes from early
TeleType machines.

## Sends

```
\005
```

## Returns

In modern days, the ENQ character is often ignored by terminals.
Some terminals send the Primary DA [-t](./Dash-t.md) response.

Simon G Tatham's PuTTY sends 'PuTTY' by default. This can be
configured to send any string in the settings, under the "Terminal"
category under the setting `Answerback to ^E`.

## Historic

Some vintage TeleType machines had a programmable drum that could hold
a 20 or 22 character response, which would often be the name of the office
where a TeleType was located.  There were often dedicated keys on these
devices that would show "Here is", which would transmit the programmed
ENQ reply or "WRU" (Who Are You), which would transmit the ENQ character
to the remote TeleType.

Terminals by DEC also had a programmable response since the VT200 series.
