#
# This file is in the Public Domain.
#

CFLAGS=		-std=gnu99 -O2 -g -W -Wextra -Werror -D_BSD_SOURCE
CFLAGS+=	-Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith
CFLAGS+=	-Wshadow -Wcast-qual -Wwrite-strings

ifeq ($(MAKECMDGOALS),tests)
DEBUG=		1
endif

ifeq ($(DEBUG),1)
CFLAGS+=	-O0 -DDEBUG -fno-omit-frame-pointer
else
CFLAGS+=	-DNDEBUG
endif

JAVA_HOME?=	/usr/lib/jvm/java/

CFLAGS+=	-I.. -I$(JAVA_HOME)/include
# Sadly, jni_md.h location is OS dependent
CFLAGS+=	-I$(shell dirname $(shell find $(JAVA_HOME) -name jni_md.h))

OBJS=		org_netbsd_liblpm_LPM.o ../lpm.o
LIB=		org_netbsd_liblpm_LPM

JAR_LIB=	liblpm.jar

JAR=		$(JAVA_HOME)/bin/jar
JAVA=		$(JAVA_HOME)/bin/java
JAVAH=		$(JAVA_HOME)/bin/javah
JAVAC=		$(JAVA_HOME)/bin/javac

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $<

$(LIB).so: $(OBJS)
	$(CC) $(LDFLAGS) -fPIC -shared -o $@ $(notdir $^)

org_netbsd_liblpm_LPM.o: org_netbsd_liblpm_LPM.h org_netbsd_liblpm_LPM.c

org_netbsd_liblpm_LPM.h: org/netbsd/liblpm/LPM.class
	$(JAVAH) -jni org.netbsd.liblpm.LPM

%.class: %.java
	$(JAVAC) $<

liblpm.jar: org/netbsd/liblpm/LPM.class $(LIB).so
	$(JAR) cf $(JAR_LIB) org/netbsd/liblpm/LPM.class $(LIB).so

.PHONY: test clean

test: liblpm.jar org/netbsd/liblpm/LPMTest.class
	$(JAVA) -classpath .:liblpm.jar org.netbsd.liblpm.LPMTest

clean:
	@ rm -rf *.so *.o \
	org_netbsd_liblpm_LPM.h org/netbsd/liblpm/*.class $(JAR_LIB)
