-include configure.mk
CC:=gcc
CCFLAGS:=-Wall -O2
bindir:=bin
CTARGETS=termread truecolor
ITARGETS=$(CTARGETS) ttguess.sh
MDEP=configure.mk mk.skel Makefile

all: termread truecolor

termread: $(MDEP) termread.c
	$(MAKE) -f mk.skel SOURCE=termread.c FINAL=$@ $@

truecolor: $(MDEP) truecolor.c
	$(MAKE) -f mk.skel SOURCE=truecolor.c FINAL=$@ $@

configure.mk: configure.dist
	@if [ -e "$@" ]; then \
		echo "##############################################################"; \
		echo "####  Distribution's configure.dist is newer than $@"; \
		echo "####   please compare and update $@ manually."; \
		echo "##############################################################"; \
		false; \
	else \
		cp $< $@; \
	fi

install: $(ITARGETS)
	$(MAKE) -f mk.skel ITARGETS="$(ITARGETS)" install

clean:
	$(MAKE) -f mk.skel ITARGETS="$(CTARGETS)" clean

distclean dist-clean:
	$(MAKE) -f mk.skel ITARGETS="$(CTARGETS)" distclean
	@if [ -e "configure.mk" ]; then \
		diff "configure.mk" "configure.dist" 2>&1 >/dev/null; \
		if [ "0" != "$$?" ]; then \
			echo "##########################################################"; \
			echo "#### Modified configure.mk exists, remove manually..."; \
			echo "#### rm configure.mk"; \
			echo "##########################################################"; \
			false; \
		else \
			rm "configure.mk"; \
		fi \
	fi

.PHONY: all clean dist-clean distclean

# vim: ft=make syntax=make
# EOF Makefile
