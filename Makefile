CC=gcc
CCFLAGS=-Wall -O2
CCFLAGS+=-std=gnu99
ifdef DEBUG
	# Maintainer stuff only, you don't want to see this.
CCFLAGS+=-DDEBUG
endif
#CCFLAGS+=-ggdb

FINAL=termread
SOURCE=termread.c
X_DEPS=Makefile

SYS=$(shell uname -s)
ARCH=$(shell uname -m)
MAJ_VER=$(shell uname -r | awk -F. '{print $$1}')
# OS X 11, Big Sur, is the earliest macOS that supported arm64
TARGET_ARM64=arm64-apple-macos11
# OS X 10.10 is Yosemite, released 2014, which supports
# all hardware 2009 or more recent.
TARGET_X86_64=x86_64-apple-macos10.10

ifeq ($(shell test "Darwin" = $(SYS) -a $(MAJ_VER) -ge 20; echo $$?), 0)
# Darin 20 is Big Sur:
# Big Sur is the earliest macOS that supported arm64.
# Anything (x86_64 or arm64) will be able to target
# a universal binary with arm64-Big Sur and x86_64-Yosemite
INTERIM=universal
else ifeq ($(shell test "Darwin" = $(SYS) -a $(MAJ_VER) -lt 14; echo $$?), 0)
# Darwin 14 is Yosemite:
# Yosemite is the earlies target macOS I've been able to test,
# so if Darwin is from Mavericks or older, we'll treat this like
# Linux, and just take compiler defaults (whatever they are)
INTERIM=default
else ifeq ($(shell test "Darwin" = $(SYS); echo $$?), 0)
# x86_64 from Yosemite to Catalina will target Yosemite
# arm64 always targets Big Sur (while it's supported, I guess?).
INTERIM=$(ARCH)
else
INTERIM=default
endif

$(FINAL): $(INTERIM).$(FINAL)
	cp $(INTERIM).$(FINAL) $(FINAL)

universal.$(FINAL): x86_64.$(FINAL) arm64.$(FINAL)
	lipo -create -output universal.$(FINAL) x86_64.$(FINAL) arm64.$(FINAL)

x86_64.$(FINAL): $(SOURCE) $(X_DEPS)
	$(CC) $(CCFLAGS) -target $(TARGET_X86_64) -o x86_64.$(FINAL) $(SOURCE)

arm64.$(FINAL): $(SOURCE) $(X_DEPS)
	$(CC) $(CCFLAGS) -target $(TARGET_ARM64) -o arm64.$(FINAL) $(SOURCE)

default.$(FINAL): termread.c Makefile
	$(CC) $(CCFLAGS) -o default.$(FINAL) termread.c

clean:
	-rm -f $(FINAL) x86_64.$(FINAL) arm64.$(FINAL) universal.$(FINAL)
	@-rm *.o *.obj *.$(FINAL)
