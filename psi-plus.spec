%define rev 20110530svn3954
%define genericplugins attentionplugin autoreplyplugin birthdayreminderplugin captchaformsplugin chessplugin cleanerplugin clientswitcherplugin conferenceloggerplugin contentdownloaderplugin extendedmenuplugin extendedoptionsplugin gmailserviceplugin gomokugameplugin historykeeperplugin icqdieplugin imageplugin jabberdiskplugin juickplugin pepchangenotifyplugin qipxstatusesplugin screenshotplugin skinsplugin stopspamplugin storagenotesplugin translateplugin watcherplugin

%define unixplugins videostatusplugin

Summary:        Jabber client based on Qt
Name:           psi-plus
Version:        0.15
Release:        0.18.%{rev}%{?dist}
Epoch:          1
Packager:       Ivan Romanov <drizt@land.ru>

URL:            http://code.google.com/p/psi-dev/
License:        GPLv2+1
Group:          Applications/Internet
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Source0:        http://koji.russianfedora.ru/storage/psi-plus/%{name}-%{version}-20110530svn3954.tar.bz2
Source1:        iconsets.tar.bz2
Source2:        language_ru.tar.bz2
Source3:        skins.tar.bz2
Source4:        themes.tar.bz2

BuildRequires:  qt-devel
BuildRequires:  zlib-devel
BuildRequires:  qca2-devel
BuildRequires:  glib2-devel
BuildRequires:  qconf >= 1.4-2
BuildRequires:  enchant-devel
BuildRequires:  libXScrnSaver-devel
BuildRequires:  openssl-devel
BuildRequires:  qt-webkit-devel

Requires:       sox
Requires:       gnupg
# Required for SSL/TLS connections
Requires:       qca-ossl
# Required for GnuPG encryption
Requires:       qca-gnupg

%description
Psi+ - Psi IM Mod by psi-dev@conference.jabber.ru

%package        plugins
Summary:        Plugins pack for Psi+
Group: Applications/Internet
Requires: %{name} = %{epoch}:%{version}-%{release}


%description    plugins
Psi+ - Psi IM Mod by psi-dev@conference.jabber.ru

Attention Plugin
This plugin is designed to send and receive special messages such as
Attentions.

Autoreply Plugin
This plugin acts as an auto-answering machine.

Birthday Reminder Plugin
This plugin is designed to show reminders of upcoming birthdays.

Captcha Forms Plugin
This plugin is designed to pass of captcha directly from the Psi+.

Chess Plugin
This plugin allows you to play chess with your friends.
The plugin is compatible with a similar plugin for Tkabber.

Cleaner Plugin
This plugin is designed to clear the avatar cache, saved local copies
of vCards and history logs.

Client Switcher Plugin
This plugin is intended to spoof version of the Jabber client, the
name and type of operating system. It is possible to manually specify
the version of the client and the operating system or choose from a
predefined list.

Conference Logger Plugin
This plugin is designed to save conference logs in which the Psi+
user sits.

Content Downloader Plugin
This plugin can currently be used to download and install roster
iconsets and emoticons.

Extended Menu Plugin
This plugin adds roster submenu 'Extended Actions' to contact's
context menu. At the moment we have the following items: 'Copy JID',
'Copy the nickname', 'Copy the status message' and 'Ping'.

Extended Options Plugin
This plugin is designed to allow easy configuration of some advanced
options in Psi+. This plugin gives you access to advanced application
options, which do not have a graphical user interface.

Gmail Service Plugin
Shows notifications of new messages in your Gmailbox.

History Keeper Plugin
This plugin is designed to remove the history of selected contacts
when the Psi+ is closed.

ICQ Must Die Plugin
This plugin is designed to help you transfer as many contacts as
possible from ICQ to Jabber.

Image Plugin
This plugin is designed to send images to roster contacts.

Juick Plugin
This plugin is designed to work efficiently and comfortably with the
Juick microblogging service.

PEP Change Notify Plugin
The plugin is designed to display popup notifications on change of
moods, activities and tunes at the contacts of the roster. In the
settings you can choose which ones to include notification of events,
specify the time within which a notice will appear, as well as play a
sound specify.

Qip X-statuses Plugin
This plugin is designed to display X-statuses of contacts using the
QIP Infium jabber client.

Screenshot Plugin
This plugin allows you to make a snapshot (screenshot) of the screen,
edit the visible aria to make a screenshot and save the image to a
local drive or upload to HTTP/FTP server.

Stop Spam Plugin
This plugin is designed to block spam messages and other unwanted
information from Psi+ users.

Storage Notes Plugin
This plugin is an implementation of XEP-0049: Private XML Storage.
The plugin is fully compatible with notes saved using Miranda IM.
The plugin is designed to keep notes on the jabber server with the
ability to access them from anywhere using Psi+ or Miranda IM.

Translate Plugin
This plugin allows you to convert selected text into another language.

Video Status Changer Plugin
This plugin is designed to set the custom status when you see the
video in selected video player. Communication with players made by
D-Bus.

%package        icons
Summary:        Iconsets for Psi+
Group: Applications/Internet
Requires: %{name} = %{epoch}:%{version}-%{release}


%description    icons
Psi+ - Psi IM Mod by psi-dev@conference.jabber.ru

Iconsets for Psi+

%package        skins
Summary:        Iconsets for Psi+
Group:          Applications/Internet
Requires:       %{name} = %{epoch}:%{version}-%{release}


%description    skins
Psi+ - Psi IM Mod by psi-dev@conference.jabber.ru

Skins Plugin and skins for Psi+

Skins Plugin
This plugin is designed to create, store and apply skins to Psi+.

%package        themes
Summary:        Themes for Psi+
Group:          Applications/Internet
Requires:       %{name} = %{epoch}:%{version}-%{release}

%description    themes
Psi+ - Psi IM Mod by psi-dev@conference.jabber.ru

Contents themes for Psi+. Now only one theme. It is Lunna Cat.
Thank majik for greatest work.

%prep
%setup -q -n %{name}-%{version}-%{rev}

# Untar russian language
%{__tar} xjf %{SOURCE2} -C .

%build
unset QTDIR
qconf-qt4
./configure                        \
        --prefix=%{_prefix}        \
        --bindir=%{_bindir}        \
        --libdir=%{_libdir}        \
        --datadir=%{_datadir}      \
        --release                  \
        --no-separate-debug-info   \
        --enable-webkit            \
        --enable-plugins           \
        --enable-whiteboarding

make %{?_smp_mflags}

lrelease-qt4 psi_ru.ts

pushd src/plugins

# Make paths for generic plugins
allplugins=""
for dir in %{genericplugins}
do
  allplugins="${allplugins} generic/$dir"
done

# Make paths for unix plugins
for dir in %{unixplugins}
do
  allplugins="${allplugins} unix/$dir"
done

# Compile all plugins
for dir in ${allplugins}
do
  pushd $dir
  %{_qt4_qmake}
  make %{?_smp_mflags}
  popd
done
popd

%install
rm -rf $RPM_BUILD_ROOT

export INSTALL_ROOT=$RPM_BUILD_ROOT
make install

# Install russian
cp psi_ru.qm $RPM_BUILD_ROOT%{_datadir}/psi-plus/

# Install iconsets
%{__tar} xjf %{SOURCE1} -C $RPM_BUILD_ROOT%{_datadir}/psi-plus/

# Install skins
%{__tar} xjf %{SOURCE3} -C $RPM_BUILD_ROOT%{_datadir}/psi-plus/

# Install themes
%{__tar} xjf %{SOURCE4} -C $RPM_BUILD_ROOT%{_datadir}/psi-plus/

mkdir -p $RPM_BUILD_ROOT%{_libdir}/psi-plus/plugins

# Make paths for generic plugins
allplugins=""
for dir in %{genericplugins}
do
  allplugins="${allplugins} generic/$dir"
done

# Make paths for unix plugins
for dir in %{unixplugins}
do
  allplugins="${allplugins} unix/$dir"
done

pushd src/plugins

for dir in ${allplugins}
do
  cp $dir/*.so $RPM_BUILD_ROOT%{_libdir}/psi-plus/plugins/
done
popd

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
%dir %{_datadir}/psi-plus/
%{_datadir}/psi-plus/certs/
%{_datadir}/psi-plus/sound/
%{_datadir}/psi-plus/psi_ru.qm
%{_datadir}/psi-plus/iconsets/*/default/
%{_datadir}/psi-plus/themes/chatview/adium/
%{_datadir}/psi-plus/themes/chatview/util.js
%{_datadir}/psi-plus/themes/chatview/psi/classic/
%{_datadir}/psi-plus/themes/chatview/psi/adapter.js
%{_datadir}/psi-plus/README
%{_datadir}/psi-plus/COPYING
%dir %{_libdir}/psi-plus/

%files plugins
%defattr(-,root,root,-)
%{_libdir}/psi-plus/plugins/
%exclude %{_libdir}/psi-plus/plugins/libskinsplugin.so

%files icons
%defattr(-,root,root,-)
%{_datadir}/psi-plus/iconsets/
%exclude %{_datadir}/psi-plus/iconsets/*/default/

%files skins
%defattr(-,root,root,-)
%{_libdir}/psi-plus/plugins/libskinsplugin.so
%{_datadir}/psi-plus/skins/

%files themes
%defattr(-,root,root,-)
%{_datadir}/psi-plus/themes/chatview/psi/
%exclude %{_datadir}/psi-plus/themes/chatview/psi/adapter.js
%exclude %{_datadir}/psi-plus/themes/chatview/psi/classic/

%changelog
* Mon May 30 2011 Ivan Romanov <drizt@land.ru> - 0.15-0.18.20110530svn3954
- update to r3954
- now will be used only .bz2 archives insted .gz
- moved psimedia to standalone package
- added skipped %{?_smp_mflags} to plugins building
- removed unusual desktop-file-utils. Really .desktop file will be
  installed in make install stage
- removed clean stage
- added whiteboarding
- added themes subpackage
- new plugins: Client Switcher, Gomoku Game, Extended Menu,
  Jabber Disk, PEP Change Notify, Video Status
- dropped hint flags from Required

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

* Mon Nov 01 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.13.20101102svn3143
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

* Wed Aug 04 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.9.20100804svn2794
- update to r2794

* Mon Jul 26 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.8.20100726svn2752
- update to r2752

* Mon Jul 5 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.7.20100705svn2636
- fix for working with psimedia
- update to r2636

* Tue Jun 29 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.6.20100629svn2620
- update to r2620

* Fri Jun 04 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.6.20100603svn2507
- fix translations
- update to r2507

* Thu Jun 03 2010 Ivan Romanov <drizt@land.ru> - 0.15-0.5.20100603svn2500
- added skins
- update to r2500

* Thu May 20 2010 Arkady L. Shane <ashejn@yandex-team.ru> - 0.15-0.4.20100520svn2439
- new Ivan Romanov <drizt@land.ru> build

* Tue Mar 02 2010 Arkady L. Shane <ashejn@yandex-team.ru> - 0.15-0.3.20100122svn1671
- rebuilt with openssl

* Sat Jan 30 2010 Arkady L. Shane <ashejn@yandex-team.ru> - 0.15-0.20100122svn1671
- initial Psi+ build
