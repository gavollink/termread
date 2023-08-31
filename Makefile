termread: termread.c Makefile
	gcc -Wall -O2 -std=gnu99 -o termread termread.c

clean:
	-rm -f termread termread.o
