%define version	%(cat %{_topdir}/version.txt)
%define luaver 5.1
%define lualibdir %{_libdir}/lua/%{luaver}

Name:		liblpm
Version:	%{version}
Release:	1%{?dist}
Summary:	Longest Prefix Match (LPM) library
Group:		System Environment/Libraries
License:	BSD
URL:		https://github.com/rmind/liblpm
Source0:	liblpm.tar.gz

BuildRequires:	make
BuildRequires:	libtool
BuildRequires:	lua >= %{luaver}, lua-devel >= %{luaver}
Requires:	lua >= %{luaver}

%description
Longest Prefix Match (LPM) library supporting IPv4 and IPv6.

%prep
%setup -q -n src

%check
make tests

%build
make %{?_smp_mflags} lib LIBDIR=%{_libdir}
make %{?_smp_mflags} -C lua lib LIBDIR=%{lualibdir}

%install
make install \
    DESTDIR=%{buildroot} \
    LIBDIR=%{_libdir} \
    INCDIR=%{_includedir} \
    MANDIR=%{_mandir}
make -C lua install \
    DESTDIR=%{buildroot} \
    LUA_LIBDIR=%{lualibdir}

%files
%{_libdir}/liblpm.*

%{_includedir}/*
#%{_mandir}/*

#
# Lua module.
#
%package lua
Summary: Lua bindings for the LPM library
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description lua
Lua bindings for the LPM library.

%files lua
%{lualibdir}/*

%changelog
* Wed Jun 1 2016 Mindaugas Rasiukevicius <rmind@noxt.eu> 0.2.0-1
- Initial release of the liblpm library.
