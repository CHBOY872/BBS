CC=gcc
CFLAGS=-Wall

DEBUG=-g
RELEASE=-s

ANSI=-ansi
ISOSTD=

SRCMODULES=
OBJMODULES=$(SRCMODULES:.c=:.o)
HEDMODULES=$(SRCMODULES:.c=:.h)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $< -c -o $@

main: main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $^ -o $@
