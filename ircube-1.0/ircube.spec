#
#
%undefine _missing_build_ids_terminate_build
%global _unpackaged_files_terminate_build 1
%global debug_package %{nil}

%if %{defined _project}
# define if building on openSUSE build service
%global build_on_obs       1
%global reconfigure_mode   0
%else
%define _project           local
%global build_on_obs       0
%global reconfigure_mode   0
%global _hardened_build    1
%endif
%if %build_on_obs == 1
%global packager Manvendra Bhangui <manvendra@indimail.org>
%endif
%global fedorareview       0

%if 0%{?suse_version}
%global dist suse
%global disttag suse
%endif

%if 0%{?fedora_version}
%global dist %{?dist}
%global disttag fedora
%endif
%global _prefix            /usr
%global plugindir          %{_prefix}/share/roundcubemail
%global libexecdir         %{_prefix}/libexec/indimail

Name: ircube
Version: 1.0
Release: 1.1%{?dist}
Summary: A collection of plugins for RoubdCube Mail

%if %{undefined suse_version} && %{undefined sles_version}
Group: System Environment/Base
%else
Group: Productivity/Networking/Email/Servers
%endif
%if %build_on_obs == 1
License: GPL-3.0+
%else
License: GPLv3
%endif
URL: http://sourceforge.net/indimail
Source0: %{name}-%{version}.tar.gz

BuildRequires: coreutils
%if %build_on_obs == 1
BuildRequires: libidn-devel
%endif
Requires: roundcubemail
Requires: iwebadmin
Requires: indimail >= 2.0

%description
A collection of plugins for RoundCube Mail
This package includes RoundCube plugins that work with IndiMail

sauserprefs - Control Bogofilter settings from within Roundcube

markasjunk2 - Adds a new button to the mailbox toolbar to mark the
selected messages as Junk/Not Junk, optionally detaching original
messages from spam reports if the message is not junk and learning
junk/not junk using various methods (bogo-learn, etc.)

iwebadmin - This is a plugin that adds tabs to the Roundcube settings
page, allowing the user to set their vacation message or change their
password. Behind the scenes, it logs in to iwebadmin as that user to
make the changes. Both vacation and password tabs can be individually disabled.

Note: This screen-scrapes your iwebadmin installation, so currently only
works if iwebadmin is in English.
1. iwebadmin   - Change password and set vacation
2. sauserprefs - Set Bogofilter user preferences
3. markasjunk2 - Mark an email as spam or ham in bogofilter

%prep
%setup -q

%build
%configure --prefix=%{_prefix} --libexecdir=%{_prefix}/libexec/indimail --enable-plugindir=%{_prefix}/share/roundcubemail
make

%install
%make_install


%files
%attr(0755,root,root)      %{libexecdir}/bogo-learn
%dir %attr(0755,root,root) %{plugindir}/plugins
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/sauserprefs.js
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/composer.json
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/config.inc.php
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/README.md
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/localization
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/pl_PL.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/ru_RU.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/cs_CZ.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/hu_HU.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/en_GB.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/ro_RO.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/it_IT.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/de_CH.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/pt_BR.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/en_US.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/sv_SE.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/es_ES.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/de_DE.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/sk_SK.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/fr_FR.inc
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/localization/gl_ES.inc
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/skins
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/skins/larry
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/skins/larry/images
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/larry/images/icons.png
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/larry/images/listicons.png
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/larry/sauserprefs.css
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/skins/larry/templates
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/larry/templates/settingsedit.html
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/larry/templates/sauserprefs.html
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/larry/tabstyles.css
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/skins/classic
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/skins/classic/images
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/classic/images/icons.png
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/classic/images/sort.gif
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/classic/sauserprefs.css
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/skins/classic/templates
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/classic/templates/settingsedit.html
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/classic/templates/sauserprefs.html
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/skins/classic/tabstyles.css
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/config.inc.php.dist
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/sauserprefs.sql
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/.gitignore
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/CHANGELOG
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/lib
%dir %attr(0755,root,root) %{plugindir}/plugins/sauserprefs/lib/Roundcube
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/lib/Roundcube/rcube_sauserprefs_storage_sql.php
%attr(0644,root,root)      %{plugindir}/plugins/sauserprefs/sauserprefs.php
%dir %attr(0775,root,root) %{plugindir}/plugins/iwebadmin
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/config.inc.php
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/iwebadmin.php
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/README.md
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/onvacation.js
%dir %attr(0775,root,root) %{plugindir}/plugins/iwebadmin/localization
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/localization/pt_BR.inc
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/localization/en_US.inc
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/config.inc.php.dist
%attr(0664,root,root)      %{plugindir}/plugins/iwebadmin/ChangeLog
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2/drivers
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/drivers/cmd_learn.php
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/drivers/sa_blacklist.php
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/drivers/sa_detach.php
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/drivers/amavis_blacklist.php
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/drivers/edit_headers.php
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/drivers/dir_learn.php
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/drivers/email_learn.php
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/composer.json
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/config.inc.php
%attr(0755,root,root)      %{plugindir}/plugins/markasjunk2/markasjunk2.php
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/README.md
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2/localization
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/pl_PL.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/ru_RU.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/es_AR.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/bg_BG.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/cs_CZ.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/lv_LV.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/hu_HU.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/ja_JP.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/zh_TW.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/en_GB.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/ca_ES.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/tr_TR.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/ro_RO.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/it_IT.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/de_CH.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/pt_BR.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/en_US.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/es_ES.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/de_DE.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/sk_SK.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/fa_IR.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/fr_FR.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/nl_NL.inc
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/localization/gl_ES.inc
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2/skins
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2/skins/larry
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2/skins/larry/images
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/skins/larry/images/messageactions.png
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/skins/larry/images/mail_toolbar.png
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/skins/larry/markasjunk2.css
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2/skins/classic
%dir %attr(0755,root,root) %{plugindir}/plugins/markasjunk2/skins/classic/images
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/skins/classic/images/messageactions.png
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/skins/classic/images/mail_toolbar.png
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/skins/classic/markasjunk2.css
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/config.inc.php.dist
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/.gitignore
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/markasjunk2.js
%attr(0644,root,root)      %{plugindir}/plugins/markasjunk2/CHANGELOG
%dir /usr/libexec
%dir %{libexecdir}
%dir %{_prefix}/share/indimail
%dir %{_prefix}/share/indimail/doc
%dir %{plugindir}

%doc %{_prefix}/share/indimail/doc/README.ircube
%doc %{_prefix}/share/indimail/doc/COPYING.ircube

%changelog
* Sat Apr 13 2017 mbhangui@gmail.com 1-0
Release 1.1 Start 13/04/2017
1. Created package for roundcube plugins
2. Added INSTALL file
3. Added testssl.php
