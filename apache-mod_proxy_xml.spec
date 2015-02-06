#Module-Specific definitions
%define mod_name mod_proxy_xml
%define mod_conf A69_%{mod_name}.conf
%define mod_so %{mod_name}.so

Summary:	DSO module for the apache web server
Name:		apache-%{mod_name}
Version:	0.1
Release:	16
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
%{_bindir}/apxs -c %{mod_name}.c

%install

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

%files
%doc README
%attr(0644,root,root) %config(noreplace) %{_sysconfdir}/httpd/modules.d/%{mod_conf}
%attr(0755,root,root) %{_libdir}/apache-extramodules/%{mod_so}


%changelog
* Sat Feb 11 2012 Oden Eriksson <oeriksson@mandriva.com> 0.1-15mdv2012.0
+ Revision: 772746
- rebuild

* Tue May 24 2011 Oden Eriksson <oeriksson@mandriva.com> 0.1-14
+ Revision: 678400
- mass rebuild

* Sun Oct 24 2010 Oden Eriksson <oeriksson@mandriva.com> 0.1-13mdv2011.0
+ Revision: 588046
- rebuild

* Mon Mar 08 2010 Oden Eriksson <oeriksson@mandriva.com> 0.1-12mdv2010.1
+ Revision: 516163
- rebuilt for apache-2.2.15

* Sat Aug 01 2009 Oden Eriksson <oeriksson@mandriva.com> 0.1-11mdv2010.0
+ Revision: 406634
- rebuild

* Tue Jan 06 2009 Oden Eriksson <oeriksson@mandriva.com> 0.1-10mdv2009.1
+ Revision: 326219
- rebuild

* Mon Jul 14 2008 Oden Eriksson <oeriksson@mandriva.com> 0.1-9mdv2009.0
+ Revision: 235069
- rebuild

* Thu Jun 05 2008 Oden Eriksson <oeriksson@mandriva.com> 0.1-8mdv2009.0
+ Revision: 215620
- fix rebuild

* Sun Mar 09 2008 Oden Eriksson <oeriksson@mandriva.com> 0.1-7mdv2008.1
+ Revision: 182830
- rebuild

* Mon Feb 18 2008 Thierry Vignaud <tv@mandriva.org> 0.1-6mdv2008.1
+ Revision: 170741
- rebuild
- fix "foobar is blabla" summary (=> "blabla") so that it looks nice in rpmdrake
- kill re-definition of %%buildroot on Pixel's request

  + Olivier Blin <blino@mandriva.org>
    - restore BuildRoot

* Sat Oct 13 2007 Oden Eriksson <oeriksson@mandriva.com> 0.1-5mdv2008.1
+ Revision: 97987
- bunzip the sources

* Sat Sep 08 2007 Oden Eriksson <oeriksson@mandriva.com> 0.1-4mdv2008.0
+ Revision: 82660
- rebuild


* Sat Mar 10 2007 Oden Eriksson <oeriksson@mandriva.com> 0.1-3mdv2007.1
+ Revision: 140729
- rebuild

* Thu Nov 09 2006 Oden Eriksson <oeriksson@mandriva.com> 0.1-2mdv2007.0
+ Revision: 79482
- Import apache-mod_proxy_xml

* Tue Jul 18 2006 Oden Eriksson <oeriksson@mandriva.com> 0.1-2mdv2007.0
- fix summary and deps

* Tue Jul 18 2006 Oden Eriksson <oeriksson@mandriva.com> 0.1-1mdv2007.0
- initial Mandriva package

