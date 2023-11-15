.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -g -W -I./include/
LDFLAGS =
x = .exe
o = .o

OBJS=les/parsemkb$o les/kbclear$o les/kbcopy$o \
     les/les$o les/lesinit$o

all: kbtest$x lestest$x

lestest$x: tests/lestest$o $(OBJS)
	$(CC) $(CFLAGS) -o lestest$x tests/lestest$o $(OBJS) $(LDFLAGS)

kbtest$x: tests/kbtest$o $(OBJS)
	$(CC) $(CFLAGS) -o kbtest$x tests/kbtest$o $(OBJS) $(LDFLAGS)

.SUFFIXES: .c $o

.c$o:
	$(CC) $(CFLAGS) -o $@ -c $<

les/les.c: include/les/expert.h include/les/knowbase.h
les/lesinit.c: include/les/expert.h include/les/knowbase.h
les/parsemkb.c: include/les/knowbase.h
les/kbclear.c: include/les/knowbase.h
les/kbcopy.c: include/les/knowbase.h

clean:
	rm -f tests/*$o les/*$o ./kbtest$x ./lestest$x
	
