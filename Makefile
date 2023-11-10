.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -g -W -I.
EXE = .exe

all: kbtest$(EXE)

kbtest$(EXE): tests/kbtest.o les/kbparse.o les/kbdestroy.o \
	lib/sds/sds.o lib/stb/stb_ds.o
	$(CC) $(CFLAGS) -o kbtest$(EXE) tests/kbtest.o \
		les/kbparse.o les/kbdestroy.o \
		lib/sds/sds.o lib/stb/stb_ds.o

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f tests/*.o les/*.o lib/sds/*.o lib/stb/*.o
	
