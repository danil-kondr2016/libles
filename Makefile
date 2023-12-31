.POSIX:
.SUFFIXES:

SYSTEM ?= windows

CFLAGS_linux = 
LDFLAGS_linux = 
LDFLAGS_windows =

CC ?= cc
AR ?= ar
RANLIB ?= ranlib
CFLAGS += $(CFLAGS_$(SYSTEM)) -W -I./include/
LDFLAGS += -s $(LDFLAGS_$(SYSTEM))

x_windows = .exe
o_windows = .obj
a_windows = .a
d_windows = .dll
i_windows = .lib

x_linux = 
o_linux = .o
a_linux = .a
d_linux = .so
i_linux = .a

x := $(x_$(SYSTEM))
o := $(o_$(SYSTEM))
a := $(a_$(SYSTEM))
d := $(d_$(SYSTEM))
i := $(i_$(SYSTEM))

LIBOBJS=les/parsemkb$o les/kbclear$o les/kbcopy$o \
     les/les$o les/lesinit$o les/protocol$o \
     les/version$o les/kbinit$o

all: libles$d libles$a lesrun$x lesruns$x

lesrun$x: cmd/lesrun/lesrun$o libles$d
	$(CC) $(CFLAGS) -D_LES_DLL $(LDFLAGS) -o $@ \
		cmd/lesrun/lesrun$o libles$d $(LDLIBS)

lesruns$x: cmd/lesrun/lesrun$o libles$a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ \
		cmd/lesrun/lesrun$o libles$a $(LDLIBS)

LDFLAGS_libles_windows = -Wl,--out-implib,$(@:$d=$i)
LDFLAGS_libles_linux = -fvisibility=hidden
LDFLAGS_libles = $(LDFLAGS_libles_$(SYSTEM))

libles$d: $(LIBOBJS)
	$(CC) -shared \
		$(CFLAGS) -D_LES_DLL -D_LES_EXPORT \
		$(LDFLAGS) $(LDFLAGS_libles) \
		-o $@ $(LIBOBJS) $(LDLIBS)

libles$i: libles$d

libles$a: $(LIBOBJS)
	$(AR) rcs $@ $(LIBOBJS)
	$(RANLIB) $@

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
	-rm -f les/*$o cmd/lesrun/*$o \
		./lesrun$x \
		libles$d libles$a libles$i \
		./lesruns$x
	
