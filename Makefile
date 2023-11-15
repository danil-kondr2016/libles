.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -g -W -I./include/
x = .exe
o = .o

all: kbtest$x

kbtest$x: tests/kbtest$o les/parsemkb$o les/kbclear$o
	$(CC) $(CFLAGS) -o kbtest$x tests/kbtest$o \
		les/parsemkb$o les/kbclear$o les/kbcopy$o

.SUFFIXES: .c $o

.c$o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f tests/*$o les/*$o ./kbtest$x
	
