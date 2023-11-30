.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -Os -fvisibility=hidden -g -W -I./include/
LDFLAGS = -s

LIBOBJS=les/parsemkb.o les/kbclear.o les/kbcopy.o \
     les/les.o les/lesinit.o les/protocol.o \
     les/version.o

all: kbtest lestest libles.so

lestest: tests/lestest.o libles.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o lestest \
		tests/lestest.o libles.so $(LDLIBS)

kbtest: tests/kbtest.o libles.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o kbtest \
		tests/kbtest.o libles.so $(LDLIBS)

libles.so: $(LIBOBJS)
	$(CC) -shared \
		$(CFLAGS) $(LDFLAGS) -o $@ $(LIBOBJS) $(LDLIBS)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

les/les.o: include/les/expert.h include/les/knowbase.h
les/lesinit.o: include/les/expert.h include/les/knowbase.h
les/parsemkb.o: include/les/knowbase.h
les/kbclear.o: include/les/knowbase.h
les/kbcopy.o: include/les/knowbase.h
les/protocol.o: include/les/protocol.h
les/version.o: include/les/version.h

clean:
	rm -f tests/*.o les/*.o \
		./kbtest ./lestest \
		libles.so
	
