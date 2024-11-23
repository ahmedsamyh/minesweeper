CC ?=gcc
CLFAGS ?=-Wall -Wextra -ggdb -pedantic

mine: mine.c
	$(CC) $(CFLAGS) -o mine mine.c $(LIBS)
