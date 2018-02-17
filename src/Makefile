#
# This file is in the Public Domain.
#

CFLAGS=		-std=c99 -O2 -g -W -Wextra -Werror
CFLAGS+=	-D_POSIX_C_SOURCE=200809L
CFLAGS+=	-D_GNU_SOURCE -D_DEFAULT_SOURCE

#
# Extended warning flags.
#
CFLAGS+=	-Wno-unknown-warning-option # gcc vs clang

CFLAGS+=	-Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith
CFLAGS+=	-Wmissing-declarations -Wredundant-decls -Wnested-externs
CFLAGS+=	-Wshadow -Wcast-qual -Wcast-align -Wwrite-strings
CFLAGS+=	-Wold-style-definition
CFLAGS+=	-Wsuggest-attribute=noreturn -Wjump-misses-init

# New GCC 6/7 flags:
#CFLAGS+=	-Wduplicated-cond -Wmisleading-indentation -Wnull-dereference
#CFLAGS+=	-Wduplicated-branches -Wrestrict

ifeq ($(MAKECMDGOALS),tests)
DEBUG=		1
endif

ifeq ($(DEBUG),1)
CFLAGS+=	-Og -DDEBUG -fno-omit-frame-pointer
else
CFLAGS+=	-DNDEBUG
endif

ifeq ($(MAKECMDGOALS),lua)
# Lua module
CFLAGS+=	$(shell pkg-config --cflags lua) -fPIC
LDFLAGS+=	$(shell pkg-config --cflags --libs lua)
LDFLAGS+=	-L.libs -llpm
OBJS=		lpm_lua.o
LIB=		lpm
else
# C library
INCS=		lpm.h
OBJS=		lpm.o
LIB=		liblpm
endif

$(LIB).la:	LDFLAGS+=	-rpath $(LIBDIR)
install/%.la:	ILIBDIR=	$(DESTDIR)/$(LIBDIR)
install:	IINCDIR=	$(DESTDIR)/$(INCDIR)/
#install:	IMANDIR=	$(DESTDIR)/$(MANDIR)/man3/

obj: $(OBJS)

lib: $(LIB).la

lua: $(LIB).so

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(LIB).so: $(OBJS)
	$(CC) $(LDFLAGS) -fPIC -shared -o $@ $(notdir $^)

%.lo: %.c
	libtool --mode=compile --tag CC $(CC) $(CFLAGS) -fPIC -c $<

$(LIB).la: $(shell echo $(OBJS) | sed 's/\.o/\.lo/g')
	libtool --mode=link --tag CC $(CC) $(LDFLAGS) -o $@ $(notdir $^)

install/%.la: %.la
	mkdir -p $(ILIBDIR)
	libtool --mode=install install -c $(notdir $@) $(ILIBDIR)/$(notdir $@)

install: $(addprefix install/,$(LIB).la)
	libtool --mode=finish $(LIBDIR)
	mkdir -p $(IINCDIR) && install -c $(INCS) $(IINCDIR)
	#mkdir -p $(IMANDIR) && install -c $(MANS) $(IMANDIR)

lua_install:
	mkdir -p $(DESTDIR)/$(LIBDIR)
	install -c lpm.so $(DESTDIR)/$(LIBDIR)

tests: $(OBJS) t_lpm.o
	$(CC) $(CFLAGS) $^ -o t_lpm
	MALLOC_CHECK_=3 ./t_lpm

clean:
	libtool --mode=clean rm
	rm -rf .libs *.so *.o *.lo *.la t_lpm

.PHONY: all obj lib lua install tests clean
