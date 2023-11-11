.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -g -W -I.
x = .exe
o = .o

all: kbtest$x

kbtest$x: tests/kbtest$o les/knowbase$o
	$(CC) $(CFLAGS) -o kbtest$x tests/kbtest$o \
		les/knowbase$o

.SUFFIXES: .c $o

.c$o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f tests/*$o les/*$o ./kbtest$x
	
