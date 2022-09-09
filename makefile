CC=gcc
CFLAGS=-Wall

DEBUG=-g
RELEASE=-s

ANSI=-ansi
ISOSTD=

SRCMODULES=file/file_database.c user/user_database.c
HDRMODULES=$(SRCMODULES:.c=.h)
OBJMODULES=$(SRCMODULES:.c=.o)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $< -c -o $@

main: main.c $(OBJMODULES)
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $^ -o $@
