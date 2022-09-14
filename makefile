CC=gcc
CFLAGS=-Wall

DEBUG=-g
RELEASE=-s

ANSI=-ansi
ISOSTD=

SRCMODULES=file/file_database.c user/user_database.c server/server.c
HDRMODULES=$(SRCMODULES:.c=.h)
OBJMODULES=$(SRCMODULES:.c=.o)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $< -c -o $@

_server: _server.c $(OBJMODULES)
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $^ -o $@

_client: _client.c
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $< -o $@

all:
	make _server
	make _client