# Teleprinters - An Abbreviated History

I wrote this while writing about this program's use of
ASCII `ENQ` (005), which has it's origins from long before
computers were a common thing.

## Side Quest: Early Telegraphy

Late 18th century, France

Nephews of a noted astronomer, Claude Chappe and his brothers, all
unemployed at the time - as many people were in the lead-up to the
French Revolution, experimented with long distance communication
using telescopes.

Later, during the period of the French Revolution, his brother Ignace
Chappe was a member of the newly established Legislative Assembly, and
called on Claude to demonstrate their efforts.  The Legislature
approved construction of 15 relay stations to be built, some up to
25KM (20 miles) apart spanning 193KM (about 120 miles) between Paris
and Lille.  This was done and successfully used for the first time
in 1792.

Claude called this a Tachygraph (fast writer), but the customer, the
French Army, preferred the word Telegraph (far writer).  This optical
telegraph was capable of 196 distinguishable settings (or characters),
and is the first telegraph system known to be able to carry written
words.

This was adopted across much of Europe during the Napoleonic wars.
The first electrical (closed circuit) telegraph was invented in 1833.
By 1850 the electrical telegraph was adopted for land use everywhere.

## Building Machines to Augment Telegraphy

Samuel Morse was working on the concept of electrical telegraphy from
1832 until his patent application in 1938.  While not the first, this
invention was notable for several reasons.  First, it included 
Morse Code, a rhythm based method of transmitting letters which is 
still in common use today.  It was able to work over a single wire,
which was far cheaper to deploy than the alternatives, but most
notable for this discussion is that The Morse Telegraph included a
reel of paper tope to record the recieved transmission signals.
While a discussion about ticker tape (the ticking of a telegraph) is
best done by someone else, this initial idea of having a physical
record of a transmission is important.

There were several machines in the second half of the 19th century 
that attempted to be able to transmit and recieve letters without
a human operator listening and translating clicks.  All of these
required precise timing which needed to be manually adjusted, so 
were never universally accepted.  Similarly, the paper transmission
tape was able to be used to automatically repeat a telegraph signal
from one machine to another.  During this period there were
some commercial telegraph equipment where letters were etched onto
piano style keys.

Standardization efforts for digital character transmission were
also started.  The French Baudot code from the 1870s, had some
acceptance across Europe.  This reserved two of the five bit values as
*control characters* to switch between Letters (A-Z) and
Symbols (1-9, and punctuation).  Even in Europe, though, the UK and
the rest of Europe differeed slightly in how they
used this standard.

## The First Teleprinter

A New Zealander named Donald Murray, had an idea about
combining ticker tape operated telegraph with a typewriter.
First make a machine that could read the Baudot Code directly from
a telegraph and punch those letters (five wide) as they are recieved.
Second allow the typewriter to type Baudot Code onto telegraph tape to
be sent after the finished message is composed.

He moved to New York City looking for, and recieved financial backing
for his idea in 1901.  In the course of creating this new idea, he
added Line Feed and Delete *control characters* and rearranged the
Baudot code entirely.

## Start and Stop Bits

1908 brings the first Teleprinter capable of automatic
synchronization using start and stop signals between characters.
This drastically reduces the timing errors in transmissions.

This was work started by Frank Pearne and Joy Morton (of the Salt
company) and finished by Mortan and engineer Charles Krum.  The
resulting product was sold be their joint company Morkrum.
These became relatively common for use between offices of large
companies and governments by 1930.

Notably, the Morkrum company would later rename themselves Teletype,
which much like Kleenex, would become the word that got used to refer
to all Teleprinter devices.

## Finally, A Standard (almost)

1924 brought the first communication standard to be put in actual 
common use, the [ITU2](https://en.wikipedia.org/wiki/Baudot_code#ITA2) 
standard.  And the version of this used in Europe included a spot for a
control character called `ENQ` (the US variant, called `US TTY` was
based on the same standard but would print a "$" when recieved).

Some Teleprinter equipment included a key or button with
`WRU` (Who are You) or `Here Is` [^1] where `WRU` was typically printed on
the `E` key.  When recieved, the device would automatically send back
a response, as a way for a remote to know a reciever was on and
listening, and each machine could be "programmed" by removing tabs
from a rotating drum.  Manufacturers supporting this feature had
either 20 or 22 characters of customizable response.
The `ENQ` function was included in ASCII since its 1963 introduction.

Even ASCII was only a US standard, and while common, it was not capable
of printing European alphabets.

# End of Line

Eventually, the Teleprinter fell out of fashion as "glass teletypes"
or computer terminals became common in the 1970s.

The last manufactured Teletype machines were manufactured in the
early 1980s.

[^1] [Model 28 Article on kb8ojh.net](https://kb8ojh.net/station/teletype)
