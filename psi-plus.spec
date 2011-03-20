%define rev 20110119svn3559
%define allplugins attentionplugin autoreplyplugin birthdayreminderplugin chessplugin cleanerplugin conferenceloggerplugin extendedoptionsplugin gmailserviceplugin historykeeperplugin icqdieplugin imageplugin juickplugin qipxstatusesplugin screenshotplugin skinsplugin stopspamplugin storagenotesplugin translateplugin watcherplugin contentdownloaderplugin captchaformsplugin

Summary:        Jabber client based on Qt
Summary(ru):    Джаббер клиент основанный на Qt
Name:           psi-plus
Version:        0.15
Release:        0.17.%{rev}%{?dist}
Epoch:          1
Packager:       Ivan Romanov <drizt@land.ru>

URL:        http://code.google.com/p/psi-dev/
License:    GPLv2+
Group:      Applications/Internet
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Source0:    http://koji.russianfedora.ru/storage/psi-plus/%{name}-%{version}-20110119svn3559.tar.bz2
Source1:    iconsets.tar.gz
Source2:    language_ru.tar.gz
Source3:    skins.tar.gz
Source4:    psi-plus-psimedia.tar.gz
Source5:    psi-plus-psimedia.desktop

BuildRequires:  qt-devel
BuildRequires:  zlib-devel
BuildRequires:  desktop-file-utils
BuildRequires:  qca2-devel
BuildRequires:  glib2-devel
BuildRequires:  qconf
BuildRequires:  enchant-devel
BuildRequires:  libXScrnSaver-devel
BuildRequires:  openssl-devel
%if 0%{?fedora} >= 14
BuildRequires:  qt-webkit-devel
%endif

Requires:     sox
Requires:     gnupg
# Required for SSL/TLS connections
Requires:     qca-ossl
# Required for GnuPG encryption
Requires:     qca-gnupg

%description
Psi+ - Psi IM Mod by psi-dev@conference.jabber.ru

%description -l ru
Psi+ - модификация клиента Psi от команды конференции
psi-dev@conference.jabber.ru.

%package        plugins
Summary:        Plugins pack for Psi+
Summary(ru):    Пакет плагинов для Psi+
Group: Applications/Internet
Requires: %{name} = %{epoch}:%{version}-%{release}


%description plugins
Supports Attention, Autoreply, Birthday Reminder, Chess,
Cleaner, Conference Logger, Extended Options, GMail Notification,
History Keeper, ICQ Must Die, Image, Juick, Qip X-statuses,
Screenshot, Stop Spam, Storage Notes, Translate,
Watcher, Content Downloader, Captcha Forms plugins

%description -l ru plugins
Включает Attention, Autoreply, Birthday Reminder, Chess,
Cleaner, Conference Logger, Extended Options, GMail Notification,
History Keeper, ICQ Must Die, Image, Juick, Qip X-statuses,
Screenshot, Stop Spam, Storage Notes, Translate,
Watcher, Content Downloader, Captcha Forms plugins

%package        icons
Summary:        Iconsets for Psi+
Summary(ru):    Наборы иконок для Psi+
Group: Applications/Internet
Requires: %{name} = %{epoch}:%{version}-%{release}


%description icons
Iconsets for Psi+

%description -l ru icons
Наборы иконок для Psi+

%package        skins
Summary:        Iconsets for Psi+
Summary(ru):    Наборы иконок для Psi+
Group:          Applications/Internet
Requires:       %{name} = %{epoch}:%{version}-%{release}


%description skins
Skins Plugin and skins for Psi+

%description -l ru skins
Skins Plugin и скины для Psi+

%package        psimedia
Summary:        Audio and video RTP services for Psi+
Summary(ru):    Аудио и видевызовы для Psi+
Group:          Applications/Internet
Requires:       %{name} = %{epoch}:%{version}-%{release}
BuildRequires:  gstreamer-plugins-base-devel >= 0.10.22
BuildRequires:  liboil-devel >= 0.3
BuildRequires:  speex-devel

%description psimedia
PsiMedia is a thick abstraction layer for providing audio and
video RTP services to Psi-like IM clients. The implementation
is based on GStreamer.

%description -l ru psimedia
PsiMedia это слой абстракции предоставляющий службы аудио и 
видеовызовов для Psi-подобных IM клиентов. Его реализация
основана на GStreamer.

%prep
%setup -q -n %{name}-%{version}-%{rev}

# Untar russian language
%{__tar} xzf %{SOURCE2} -C .

# Untar psimedia
%{__rm} -fr ../psi-plus-psimedia
%{__tar} xzf %{SOURCE4} -C ..

%build
unset QTDIR
qconf
./configure                        \
        --prefix=%{_prefix}        \
        --bindir=%{_bindir}        \
        --libdir=%{_libdir}        \
        --datadir=%{_datadir}      \
        --release                  \
        --no-separate-debug-info   \
        --enable-webkit            \
        --enable-plugins

make %{?_smp_mflags}

lrelease-qt4 psi_ru.ts

pushd src/plugins/generic

for dir in %{allplugins}
do
  cd $dir
  qmake-qt4 -makefile -unix ${dir}.pro
  make
  cd ..
done
popd

# Make psimedia
cd ../psi-plus-psimedia
qconf
./configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT

export INSTALL_ROOT=$RPM_BUILD_ROOT
make install

# Install russian
cp psi_ru.qm $RPM_BUILD_ROOT%{_datadir}/psi-plus/

# Install iconsets
%{__tar} xzf %{SOURCE1} -C $RPM_BUILD_ROOT%{_datadir}/psi-plus/

# Install skins
%{__tar} xzf %{SOURCE3} -C $RPM_BUILD_ROOT%{_datadir}/psi-plus/

# Menu
desktop-file-install \
       --dir $RPM_BUILD_ROOT%{_datadir}/applications \
       --delete-original \
       $RPM_BUILD_ROOT%{_datadir}/applications/psi-plus.desktop

mkdir -p $RPM_BUILD_ROOT%{_libdir}/psi-plus/plugins

pushd src/plugins/generic

for dir in %{allplugins}
do
  cp $dir/*.so $RPM_BUILD_ROOT%{_libdir}/psi-plus/plugins/
done
popd

# Install psimedia
cd ../psi-plus-psimedia
install -D -m 755 gstprovider/libgstprovider.so $RPM_BUILD_ROOT%{_libdir}/psi-plus/plugins/libgstprovider.so
install -D -m 755 demo/demo $RPM_BUILD_ROOT%{_bindir}/psi-plus-psimedia
desktop-file-install --dir=${RPM_BUILD_ROOT}%{_datadir}/applications %{SOURCE5}

%clean
rm -rf $RPM_BUILD_ROOT


%post
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
    %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi


%postun
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
    %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi


%files
%defattr(-,root,root,-)
%doc README COPYING
%{_bindir}/psi-plus
%{_datadir}/applications/psi-plus.desktop
%{_datadir}/icons/hicolor/*/apps/psi-plus.png
%{_datadir}/psi-plus/
%exclude %{_datadir}/psi-plus/iconsets/
%{_datadir}/psi-plus/iconsets/*/default/
%exclude %{_datadir}/psi-plus/skins/

%files plugins
%defattr(-,root,root,-)
%{_libdir}/psi-plus/plugins/
%exclude %{_libdir}/psi-plus/plugins/libskinsplugin.so
%exclude %{_libdir}/psi-plus/plugins/libgstprovider.so

%files icons
%defattr(-,root,root,-)
%{_datadir}/psi-plus/iconsets/
%exclude %{_datadir}/psi-plus/iconsets/*/default/

%files skins
%defattr(-,root,root,-)
%{_libdir}/psi-plus/plugins/libskinsplugin.so
%{_datadir}/psi-plus/skins/

%files psimedia
%defattr(-,root,root,-)
%{_bindir}/psi-plus-psimedia
%{_libdir}/psi-plus/plugins/libgstprovider.so
%{_datadir}/applications/psi-plus-psimedia.desktop

%changelog
* Wed Jan 19 2011 Ivan Romanov <drizt@land.ru> - 0.15-0.17.20110119svn3559
- all 'psi' dirs and files renamed to 'psi-plus'
- removed conflicts tag
- added psimedia sub-package
- update to r3559

* Sun Jan 09 2011 Ivan Romanov <drizt@land.ru> - 0.15-0.16.20110110svn3465
- some a bit fixes
- update to r3465

* Sat Dec 18 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.15.20101218svn3411
- update to r3411

* Tue Nov 16 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.14.20101116svn3216
- update to r3216
- removed libproxy from reques

* Mon Nov 1 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.13.20101102svn3143
- update to r3143
- split main package to psi-plus-skins and psi-plus-icons

* Wed Oct 06 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.12.20101006svn3066
- update to r3066
- removed obsoletes tags
- psi-plus now conflicts with psi

* Fri Sep 10 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.11.20100919svn3026
- update to r3026
- added to obsoletes psi-i18n
- added Content Downloader Plugin
- added Captcha Plugin
- remove smiles.

* Thu Aug 12 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.10.20100812svn2812
- update to r2812

* Wed Aug 4 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.9.20100804svn2794
- update to r2794

* Mon Jul 26 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.8.20100726svn2752
- update to r2752

* Mon Jul 5 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.7.20100705svn2636
- fix for working with psimedia
- update to r2636

* Tue Jun 29 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.6.20100629svn2620
- update to r2620

* Fri Jun  4 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.6.20100603svn2507
- fix translations
- update to r2507

* Thu Jun  3 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.5.20100603svn2500
- added skins
- update to r2500

* Thu May 20 2010 Arkady L. Shane <ashejn@yandex-team.ru> - 0.15-0.4.20100520svn2439
- new Ivan Romanov <drizt@land.ru> build

* Tue Mar  2 2010 Arkady L. Shane <ashejn@yandex-team.ru> - 0.15-0.3.20100122svn1671
- rebuilt with openssl

* Sat Jan 30 2010 Arkady L. Shane <ashejn@yandex-team.ru> - 0.15-0.20100122svn1671
- initial Psi+ build
