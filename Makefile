CC=gcc
CFLAGS= -W -Wall -pedantic -g
LDFLAGS=-lncurses -lcaca
EXEC=tc8e

all: $(EXEC)

tc8e: tc8e.o display.o keyboard.o
	$(CC) -o $@ $^ $(LDFLAGS)

tc8e.o: display.h keyboard.h

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o *.*~ *~

mrproper: clean
	rm -rf $(EXEC)
