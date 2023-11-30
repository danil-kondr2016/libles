.POSIX:
.SUFFIXES:

SYSTEM ?= windows

CFLAGS_linux = 
LDFLAGS_linux = 
LDFLAGS_windows =

CC ?= cc
CFLAGS += $(CFLAGS_$(SYSTEM)) -W -I./include/
LDFLAGS += -s $(LDFLAGS_$(SYSTEM))

x_windows = .exe
o_windows = .obj
a_windows = .lib
d_windows = .dll

x_linux = 
o_linux = .o
a_linux = .a
d_linux = .so

x := $(x_$(SYSTEM))
o := $(o_$(SYSTEM))
a := $(a_$(SYSTEM))
d := $(d_$(SYSTEM))

LIBOBJS=les/parsemkb$o les/kbclear$o les/kbcopy$o \
     les/les$o les/lesinit$o les/protocol$o \
     les/version$o

all: lesrun$x libles$d 

LIBLES_windows=libles$a
LIBLES_unix=libles$d
LIBLES := $(LIBLES_$(SYSTEM))

lesrun$x: lesrun/lesrun$o libles$d
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ \
		lesrun/lesrun$o $(LIBLES) $(LDLIBS)

LDFLAGS_libles_windows = -Wl,--out-implib,$(@:dll=lib)
LDFLAGS_libles_linux = -fvisibility=hidden
LDFLAGS_libles = $(LDFLAGS_libles_$(SYSTEM))

libles$d: $(LIBOBJS)
	$(CC) -shared \
		$(CFLAGS) $(LDFLAGS) $(LDFLAGS_libles) -o $@ $(LIBOBJS) $(LDLIBS)

.SUFFIXES: .c $o

.c$o:
	$(CC) $(CFLAGS) -o $@ -c $<

les/les$o: include/les/expert.h include/les/knowbase.h
les/lesinit$o: include/les/expert.h include/les/knowbase.h
les/parsemkb$o: include/les/knowbase.h
les/kbclear$o: include/les/knowbase.h
les/kbcopy$o: include/les/knowbase.h
les/protocol$o: include/les/protocol.h
les/version$o: include/les/version.h

clean:
	-rm -f les/*$o lesrun/*$o \
		./lesrun$x \
		libles$d $(LIBLES)
	
