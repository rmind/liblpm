Name:		liblpm
Version:	0.1
Release:	1%{?dist}
Summary:	Longest Prefix Match (LPM) library
Group:		System Environment/Libraries
License:	BSD
URL:		https://github.com/rmind/liblpm
Source0:	liblpm.tar.gz

BuildRequires:	make
BuildRequires:	libtool

%description
Longest Prefix Match (LPM) library supporting IPv4 and IPv6.

%prep
%setup -q -n src

%build
make %{?_smp_mflags} LIBDIR=%{_libdir}

%install
make install \
    DESTDIR=%{buildroot} \
    LIBDIR=%{_libdir} \
    INCDIR=%{_includedir} \
    MANDIR=%{_mandir}

%files
%{_libdir}/*
%{_includedir}/*
#%{_mandir}/*

%changelog
