Qmailadmin plugin for Roundcube
August 28, 2011 by davidc - http://www.davidc.net/miscellany/qmailadmin-plugin-roundcube
February, 1st, 2016 by internero - https://github.com/internero/roundcube-qmailadmin

This is a plugin that adds tabs to the Roundcube settings page, allowing the user
to set their vacation message or change their password. Behind the scenes, it logs
in to qmailadmin as that user to make the changes. Both vacation and password tabs
 can be individually disabled.

Note: This screen-scrapes your qmailadmin installation, so currently only works if
qmailadmin is in English.

Installation
============

Extract the zip file into the plugins directory of your Roundcube installation. Under
plugins, you should have a directory "qmailadmin", containing "qmailadmin.php" and the
other files.

Edit the file config.inc.php, making sure to change the path to your own qmailadmin
installation. The configuration options are documented within this file.

You will then find 'Vacation Message' and 'Change Password' tabs on the user settings page.

Vacation Message, Change Password
How it works
=================================

Rather than try to modify qmail files or call vpasswd directly, it uses qmailadmin which is already setuid.

It logs in to qmailadmin using the current user's credentials (stored encrypted in their session),
retrieves the current settings, modifies them, and saves them again. As qmailadmin has no useful API,
it does this by screenscraping the HTML form and pulling out all the INPUT/TEXTAREA elements. It expects
the "Modify User" screen after giving the current user's credentials. It does not work if the current
user is postmaster, in which case you get a diffrent menu and not the "Modify User" menu expected by
this plugin. You will get 1001 error if your try to change password when logged in as 'postmaster'

After changing the password, it stores the new password on the user's session, so they don't have to
log back in to continue reading their e-mail.

This plugin has been modified for iwebadmin for qmailadmin by Manvendra Bhangui <manvendra@indimail.org>

