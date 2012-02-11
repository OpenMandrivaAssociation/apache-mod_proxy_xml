#Module-Specific definitions
%define mod_name mod_proxy_xml
%define mod_conf A69_%{mod_name}.conf
%define mod_so %{mod_name}.so

Summary:	DSO module for the apache web server
Name:		apache-%{mod_name}
Version:	0.1
Release:	%mkrel 15
Group:		System/Servers
License:	GPL
URL:		http://apache.webthing.com/mod_proxy_xml/
# there is no official tar ball
# http://apache.webthing.com/svn/apache/filters/xmlns/
Source0:	http://apache.webthing.com/svn/apache/filters/xmlns/mod_proxy_xml.c
Source1:	README.mod_proxy_xml
Source2:	%{mod_conf}
Requires(pre): rpm-helper
Requires(postun): rpm-helper
Requires(pre):	apache-conf >= 2.2.0
Requires(pre):	apache >= 2.2.0
Requires(pre):	apache-mod_proxy >= 2.2.0
Requires(pre):	apache-mod_xmlns
Requires:	apache-conf >= 2.2.0
Requires:	apache >= 2.2.0
Requires:	apache-mod_proxy >= 2.2.0
Requires:	apache-mod_xmlns
BuildRequires:	apache-devel >= 2.2.0
BuildRequires:	apache-mod_xmlns-devel
BuildRequires:	file
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
mod_proxy_xml is an output filter to rewrite links in a proxy situation, to
ensure that links work for users outside the proxy. It serves the same purpose
as Apache's ProxyPassReverse directive does for HTTP headers, and is an
essential component of a reverse proxy.

%prep

%setup -q -c -T -n %{mod_name}-%{version}

cp %{SOURCE0} mod_proxy_xml.c
cp %{SOURCE1} README
cp %{SOURCE2} %{mod_conf}

# strip away annoying ^M
find . -type f|xargs file|grep 'CRLF'|cut -d: -f1|xargs perl -p -i -e 's/\r//'
find . -type f|xargs file|grep 'text'|cut -d: -f1|xargs perl -p -i -e 's/\r//'

%build
%{_sbindir}/apxs -c %{mod_name}.c

%install
[ "%{buildroot}" != "/" ] && rm -rf %{buildroot}

install -d %{buildroot}%{_sysconfdir}/httpd/modules.d
install -d %{buildroot}%{_libdir}/apache-extramodules

install -m0755 .libs/*.so %{buildroot}%{_libdir}/apache-extramodules/
install -m0644 %{mod_conf} %{buildroot}%{_sysconfdir}/httpd/modules.d/%{mod_conf}

%post
if [ -f %{_var}/lock/subsys/httpd ]; then
 %{_initrddir}/httpd restart 1>&2;
fi

%postun
if [ "$1" = "0" ]; then
 if [ -f %{_var}/lock/subsys/httpd ]; then
	%{_initrddir}/httpd restart 1>&2
 fi
fi

%clean
[ "%{buildroot}" != "/" ] && rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc README
%attr(0644,root,root) %config(noreplace) %{_sysconfdir}/httpd/modules.d/%{mod_conf}
%attr(0755,root,root) %{_libdir}/apache-extramodules/%{mod_so}
