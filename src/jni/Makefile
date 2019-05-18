#
# This file is in the Public Domain.
#

CFLAGS+=	-std=c99 -O2 -g -Wall -Wextra -Werror
CFLAGS+=	-D_POSIX_C_SOURCE=200809L
CFLAGS+=	-D_GNU_SOURCE -D_DEFAULT_SOURCE

CFLAGS+=	-Wno-unused-parameter

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
CFLAGS+=	-I$(shell dirname $(shell find -L $(JAVA_HOME) -name jni_md.h))

OBJS=		org_netbsd_liblpm_LPM.o ../lpm.o
LIB=		org_netbsd_liblpm_LPM

JAR_LIB=	liblpm.jar

JAR=		$(JAVA_HOME)/bin/jar
JAVA=		$(JAVA_HOME)/bin/java
JAVAC=		$(JAVA_HOME)/bin/javac

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $<

$(LIB).so: $(OBJS)
	$(CC) $(LDFLAGS) -fPIC -shared -o $@ $(notdir $^)

org_netbsd_liblpm_LPM.o: org_netbsd_liblpm_LPM.h org_netbsd_liblpm_LPM.c

org_netbsd_liblpm_LPM.h org/netbsd/liblpm/LPM.class: org/netbsd/liblpm/LPM.java
	$(JAVAC) -h . $<

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
