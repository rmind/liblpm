PROJ=	lpm

all: rpm

prepare:
	mkdir -p SOURCES && tar czpvf SOURCES/lib$(PROJ).tar.gz ../src

rpm: prepare
	rpmbuild -ba -v --define "_topdir ${PWD}" SPECS/lib$(PROJ).spec

clean:
	rm -rf BUILD BUILDROOT RPMS SOURCES SRPMS

.PHONY: all prepare rpm clean
