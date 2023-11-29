.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -Os -g -W -I./include/
LDFLAGS = -s
x = .exe
o = .o

LIBOBJS=les/parsemkb$o les/kbclear$o les/kbcopy$o \
     les/les$o les/lesinit$o les/protocol$o \
     les/version$o

all: kbtest$x lestest$x libles.lib

lestest$x: tests/lestest$o libles.lib
	$(CC) $(CFLAGS) $(LDFLAGS) -o lestest$x tests/lestest$o libles.dll $(LDLIBS)

kbtest$x: tests/kbtest$o libles.lib
	$(CC) $(CFLAGS) $(LDFLAGS) -o kbtest$x tests/kbtest$o libles.dll $(LDLIBS)

libles.lib: libles.dll

libles.dll: $(LIBOBJS)
	$(CC) -shared -Wl,--out-implib,$(@:dll=lib) \
		$(CFLAGS) $(LDFLAGS) -o $@ $(LIBOBJS) $(LDLIBS)

.SUFFIXES: .c $o

.c$o:
	$(CC) $(CFLAGS) -o $@ -c $<

les/les.c: include/les/expert.h include/les/knowbase.h
les/lesinit.c: include/les/expert.h include/les/knowbase.h
les/parsemkb.c: include/les/knowbase.h
les/kbclear.c: include/les/knowbase.h
les/kbcopy.c: include/les/knowbase.h
les/protocol.c: include/les/protocol.h
les/version.c: include/les/version.h

clean:
	rm -f tests/*$o les/*$o ./kbtest$x ./lestest$x libles.dll libles.lib
	
