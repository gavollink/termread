#############################################################################
# Makefile for Updating Termlists...
####
CC := gcc
CCFLAGS += -Wall

default: known_terminals.c

known_terminals.c: read_u9 do_pull_terminfo

read_u9: read_u9.c uthash/utarray.h Makefile
	$(CC) $(CCFLAGS) read_u9.c -o "$@"

.DUMMY: do_pull_terminfo
do_pull_terminfo:
	./pull_terminfo.sh

uthash/utarray.h:
	@if [ -r "$@" ]; then \
		echo "Target exists : $@"; \
		true; \
	 else \
	    echo "$@ is not found, this needs Troy D Hansen's UTHash" 1>&2; \
	    echo "package.  A soft link will do." 1>&2; \
		false; \
	 fi
