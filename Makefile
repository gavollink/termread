CC:=gcc
CCFLAGS:=-Wall -O2
## NO DEFAULT PREFIX (not even /)
prefix:=
exec_prefix:=$(prefix)
bindir:=bin/
DESTDIR:=
INSTALLDIR:=$(DESTDIR)$(exec_prefix)$(bindir)
ITARGETS=termread truecolor ttguess.sh
CTARGETS=termread truecolor
MDEP=configure.mk

all: termread truecolor

termread: $(MDEP)
	$(MAKE) -f mk.skel SOURCE=termread.c FINAL=$@ $@

truecolor: $(MDEP)
	$(MAKE) -f mk.skel SOURCE=truecolor.c FINAL=$@ $@

$(MDEP): Makefile
	@touch $@
	@-grep -E '^CC:?=' $@ 2>&1 >/dev/null; \
	    if [ "0" != "$$?" ]; then \
			echo "CC:=$(CC)" >> $@; else true; fi
	@-grep -E '^CCFLAGS:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "CCFLAGS:=$(CCFLAGS)" >> $@; \
		else true; fi
	@# Important: -std=gnu99 is added, and needn't be defaulted
	@-grep -E '^CCFLAGS\+=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "CCFLAGS+=-std=gnu99" >> $@; \
		else true; fi
	@-grep -E '^prefix:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "prefix:=$(prefix)" >> $@; else true; fi
	@-grep -E '^exec_prefix:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "exec_prefix:=$(exec_prefix)" >> $@; else true; fi
	@-grep -E '^bindir:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "bindir:=$(bindir)" >> $@; else true; fi
	@-grep -E '^DESTDIR:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "DESTDIR:=$(DESTDIR)" >> $@; else true; fi
	@-grep -E '^INSTALLDIR:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "INSTALLDIR:=$(INSTALLDIR)" >> $@; else true; fi
	@-grep -E '^SIGNID:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "SIGNID:=$(SIGNID)" >> $@; else true; fi
	@-grep -E '^CHAIN:?=' $@ 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "CHAIN:=$(CHAIN)" >> $@; else true; fi

install: $(ITARGETS)
	$(MAKE) -f mk.skel ITARGETS="$(ITARGETS)" install

clean:
	$(MAKE) -f mk.skel ITARGETS="$(CTARGETS)" clean

distclean dist-clean:
	$(MAKE) -f mk.skel ITARGETS="$(CTARGETS)" distclean
	-rm -f configure.mk

.PHONY: all clean dist-clean distclean

# vim: ft=make syntax=make
# EOF Makefile
