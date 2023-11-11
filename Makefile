.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -g -W -I.
EXE = .exe

all: kbtest$(EXE)

kbtest$(EXE): tests/kbtest.o les/knowbase.o
	$(CC) $(CFLAGS) -o kbtest$(EXE) tests/kbtest.o \
		les/knowbase.o

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f tests/*.o les/*.o ./kbtest$(EXE)
	
