                                 Shared folders

   This document describes how shared folders are implemented by
   Courier-IMAP, and SqWebMail.

   Courier-IMAP and SqWebMail actually have two different shared folder
   implementations, for two situations: A) Filesystem permissions-based
   shared folders, for systems with traditional shell login accounts, and
   mailboxes; and B) virtual shared folders, for closed systems that provide
   mail access only, with all mailboxes using the same system userid/groupid,
   and no end-user shell login access.

   This documentation is also applicable to the Courier mail server, which
   includes Courier-IMAP and SqWebMail as its components. There are some
   minor implementation differences between the standalone Courier-IMAP and
   SqWebMail packages, and Courier; namely the different locations of several
   key configuration files. Aside from that, both implementation are
   equivalent.

   Table Of Contents
   Virtual shared folders
      Terminology
      Technical Overview
      Functional Overview
      Combining Courier-IMAP's and SqWebMail index files
      IMAP Access Control List and account groups
    
   Filesystem permissions-based shared folders
      Terminology
      Technical Overview
      Functional Overview
      Accessing shared folders
      Subscribing to a shared folder
      Unsubscribing to a shared folder
      Opening a shared folder

                             Virtual shared folders

Terminology

   Virtual shared folders are implemented in a completely different manner.
   In a virtual setup, all mailboxes use the same system userid and groupid;
   there is no shell login access to the mail server, and all access is
   managed by setting up access control lists. Each individual user may
   voluntarily grant access to a folder to another user or group of user.
   Access control lists permit fine-grained control. It is possible to
   specify what different users are permitted to do to the folder or its
   contents. Since there is no shell login access, all access to mail folders
   occurs through an IMAP connection to the server, and the IMAP server
   observes the defined access control lists on each folder.

     NOTE:

     The references to IMAP in this documentation equally apply to SqWebMail
     as well. This documentation file is included in Courier-IMAP's and
     SqWebMail's tarballs. When reading this documentation file in the
     SqWebMail tarball, mentally replace all references to IMAP with webmail.
     SqWebMail does not use IMAP, the server reads the maildirs directory;
     but it uses the same shared library as Courier-IMAP, which explains the
     similar implementation.

   For more information on how to set up access control lists, see the
   maildiracl(1) manual page.

Technical Overview

   After logging in, the server process runs in the logged in account's
   maildir. As the INSTALL file describes in great detail, virtual mail
   accounts may be maintained via a wide variety of back-end databases,
   including MySQL, PostgresSQL, or an LDAP directory. If now the server
   needs to check the access controls on another account's mail folder, to
   determine if the user has access to it, the server needs to now locate
   where the other account is located.

   However, there are several reasons why the server cannot directly go back
   to the authentication database. The first reason is that after logging in
   the server process no longer has root privileges; and database
   configuration files normally require root privileges (since they may
   contain administrative passwords). Although there might be ways to hack
   around that, the second reason is the show-stopper: IMAP's design permits
   clients to enumerate all available shared mail accounts, or arbitrary
   subsets of them. And, in fact, many of them do just that; mainly because
   IMAP's design leaves no other alternative, in some situations.

   Clearly, it is not feasible to have hundreds of clients hammering at the
   database server, repeatedly downloading huge lists of available shared
   folders (which may number in tens of thousands, on a busy server).
   Therefore, the following approach is used.

   One or more flat text files are set up, which contain an index of all
   virtual mail folders. The list is, essentially, downloaded from the
   authentication database. The text file contain a subset of the data in the
   authentication database; only the minimum amount of fields necessary to
   specify the account's name, and where the account lives. The server
   process quickly reads the text file(s) in order to locate another
   account's folder, and to check its access control list.

   The system administrator will need to set up a regularly-scheduled process
   that rebuilds the shared folder index. Yes, that means that a new
   account's shared folders will not be accessible, by other accounts, until
   the shared folder index gets rebuilt (but the account still has immediate
   access to its own folders, after creation). The shared folder index will
   be updated in a manner that allows the update to occur live, without
   requiring any system downtime.

   On moderately-sized systems it might be feasible to invoke the shared
   index update script as part of the process that adds or removes an
   account; thereby the shared folder index files will always be kept up to
   date.

   In its most simple form, the shared index is a single file, called
   SYSCONFDIR/shared/index, where "SYSCONFDIR" is the server's configuration
   directory. The following table provides the default locations of the
   configuration file:

   Courier-IMAP:                   /usr/lib/courier-imap/etc/shared/index
   Courier:                        /usr/lib/courier/etc/shared/index
   Courier (Red Hat/Fedora build): /etc/courier/shared/index
   SqWebMail (standalone build):   /usr/local/share/sqwebmail/shared/index

     NOTE:

     If the "shared" directory doesn't exist, just create it.

   Blank lines in this file are ignored, as well as lines that begin with a
   single "#" character, which are arbitrary comments. Each remaining line
   specifies the location of an account's maildir. The line contains the
   following fields, separated by a single tab character:

    1. name
    2. system userid
    3. system groupid
    4. virtual home directory
    5. maildir path, relative to the virtual home directory

   "name" SHOULD be the account's login name (which in some rare
   configurations may not actually match the IMAP login used by the client;
   this is the name that's logged to syslog when the account successfully
   logs in, which is usually the same as the IMAP login id). In most
   situations all accounts will have the same system userid and groupid. The
   server doesn't actually do anything with these fields at this time, but
   may do so in the future.

   The virtual account's home directory is taken verbatim from the
   authentication database. The maildir path field is optional. If missing,
   it defaults to "./Maildir". Combining the home directory, and the maildir
   path, results in the location where the specified account's maildir
   folders may be found.

   Inaccessible accounts are silently ignored. This allows the shared folder
   index to be itself shared between multiple servers even though that
   accounts themselves are not shared. Each server will only see the accounts
   it can access.

   As mentioned previously, IMAP clients will often want to obtain a complete
   list of shared folders. If the mail server has more than a a couple of
   hundred accounts, a single index file may be inefficient, and the mail
   accounts should be segregated into different groups. An account group
   entry in the index file looks like this:

    1. group name
    2. *
    3. filename

   When the second tab-delimited field is a single asterisk, the first field
   is taken to be a name of an account group; and "filename" is the name of
   another, separate, index file that lists all accounts that belong in this
   group. The second file should also be installed in SYSCONFDIR/shared.

   When it's time to use account groups, SYSCONFDIR/shared/index will usually
   contain only group entries, with the accounts themselves dispersed in
   other files in the same directory.

   Account names and group names use the UTF-8 character set. This is not
   usually an issue for account names, which are almost always limited to the
   Latin character set. Group names may be arbitrary, though, and include
   non-Latin characters, using UTF-8.

   Courier does not allow IMAP folder names to contain the "." or the "/"
   characters. The shared index file may include those characters in account
   or group names, and they will be automatically replaced by spaces (which
   are allowed).

   This is not normally an issue, unless there exists two separate accounts
   or groups whose names are the same, except that one name contains spaces,
   and the other name contains the forbidden characters. This is obviously
   not something that is likely to occur, but if it did the end result would
   be that one of the names will have its punctuation characters replaced by
   spaces, and now two virtual folders will exist with the same name.

   If this rather unlikely scenario somehow materializes the solution is to
   enable the IMAP_SHAREDMUNGENAMES setting in Courier-IMAP's configuration
   file. When enabled, this setting replaces the "." and "/" with a two
   character sequence: "\:" and "\;". Any the existing backslashes are
   doubled, thus preserving folder name uniqueness. This is not something
   you'd want to do unless you have no other choice.

   The equivalent setting for SqWebMail (or Courier's webmail server) is the
   SQWEBMAIL_SHAREDMUNGENAMES. When using SqWebMail at the same time as
   Courier-IMAP, and if IMAP_SHAREDMUNGENAMES is set, the
   SQWEBMAIL_SHAREDMUNGENAMES variable must also be set in order for everyone
   to be in sync. This can be done in one of two different ways:

      1. Set this variable before running the sqwebmaild start command:

 SQWEBMAIL_SHAREDMUNGENAMES=1
 export SQWEBMAIL_SHAREDMUNGENAMES
 sqwebmaild start

      2. Set this environment variable in the web server's configuration
         files. With Apache, for example:

 SetEnv SQWEGBMAIL_SHAREDMUNGENAMES 1

  Many Shared Accounts

   When the number of mail accounts begins to exceed a couple of thousand (or
   even less, depending on the mail server), even account groups will not be
   enough. If a single mail server has tens of thousands of accounts, an
   individual IMAP client may potentially access hundreds of thousands of
   folders.

   It is a sad fact of life that there are poorly-designed IMAP clients that
   insist on searching for every accessible folder, every time. They will
   stubornly scan the entire IMAP folder namespace, recursively reading each
   folder hierarchy, to compile a list of available folders. Even though
   there may only be a few folders shared by their owners, the server still
   has to check whether the IMAP client has access to each folder, in order
   to compile the list of accessible folders. No mail server will like an
   IMAP client that insists on reading the access control lists of hundreds
   of thousand of folders.

   This problem is solved by setting the "sharedgroup" option for each
   account. The INSTALL file contains a more specific description of how to
   initialize account options. Normally, the shared folder index file is
   "SYSCONFDIR/shared/index"; however, if the "sharedgroup" option is set,
   the value of "sharedgroup" is appended to the shared folder index
   filename. So, if user1's account has the "sharedgroup=12" option, as far
   as this account is concerned the shared folder index file is
   "SYSCONFDIR/shared/index12". Note that SYSCONFDIR/shared/index12 may still
   contain account group entries that point to other shared index files.

   This enables the ability to partition all accounts into smaller
   "universes". The list of accounts is broken up into individual universe.
   Accounts within a universe can only see other accounts in the same
   universe. Even if a given folder's access control lists permit global
   access, only accounts in the same universe will be able to access it.

   Also note that trans-universal wormholes are possible. Two, or more,
   top-level index files may list the same set of account groups, and those
   accounts will be visible from both (or more) universes.

   IMPORTANT: Under no circumstances should you install circular wormholes
   (index file A lists index file B as a group, and index file B lists index
   file A as a group). The consequences would be disastrous for both
   universes.

Functional Overview

   As mentioned in the previous section, it is necessary to set up a
   regularly-scheduled system process that updates the shared folder index
   files. The procedure for doing so is site-specific. Individual sites will
   need to put together a set of custom scripts that create the shared folder
   index files in SYSCONFDIR/shared. This is likely to be a highly
   site-specific code; however Courier installs several generic shell scripts
   that can be used as a working starting point.

   The most important step in the process is actually the final step of
   installing new shared folder index files in SYSCONFDIR/shared. They must
   be installed in a way that can be done live, without shutting down the
   system, and without affecting any existing processes. This can be done
   using the “sharedindexinstall” script, which may be found in the sbin
   directory. To use sharedindexinstall, first create the shared index files
   in a temporary directory called SYSCONFDIR/shared.tmp, then run the script
   to move all the files in this directory to SYSCONFDIR/shared.

   So a typical script that updates shared index files will generally look
   like this (this example uses Courier-IMAP, for SqWebMail the directory
   locations will be different):

   --------------------------------------------------------------------------

 #!/bin/sh
 sysconfdir="/usr/lib/courier-imap/etc"  # Or /etc/courier, or whatever...
 sbindir="/usr/lib/courier-imap/sbin"    # Or, wherever it actually is...

 rm -rf $sysconfdir/shared.tmp
 mkdir $sysconfdir/shared.tmp || exit 1

 #
 # A magical process creates updated shared index files right about now...
 #

 $sbindir/sharedindexinstall

   --------------------------------------------------------------------------

    NOTE

     Existing IMAP server processes may continue to use cached shared folder
     index data for some time, after sharedindexinstall. This will not cause
     any problems.

  Using authenumerate

   In most cases, systems that use a single shared index file are likely to
   need to only run the “authenumerate” program in order to build the shared
   folder index. As long as Courier's authentication modules are properly
   configured (and authdaemond is running) authenumerate will download the
   list of accounts from the configured authentication module, and generate a
   suitably-formatted list on standard output. So the complete shared folder
   index update script will look like this:

   --------------------------------------------------------------------------

 #!/bin/sh
 sysconfdir="/usr/lib/courier-imap/etc"
 sbindir="/usr/lib/courier-imap/sbin"

 rm -rf $sysconfdir/shared.tmp
 mkdir $sysconfdir/shared.tmp || exit 1

 $sbindir/authenumerate -s >$sysconfdir/shared.tmp/index || exit 1

 $sbindir/sharedindexinstall

   --------------------------------------------------------------------------

   The functionality to enumerate accounts is new to Courier-IMAP 3.0. When
   upgrading from an earlier versions, systems that are configured to use a
   custom MySQL, PostgreSQL, or LDAP queries will need to enter a new
   configuration setting in the appropriate configuration file. The update
   process will add a new variable to the configuration file, which must be
   initialized to contain the custom query that reads the account list
   accordingly. In most cases the query only needs to be a slight variation
   on the existing query that checks the password of a specific account
   that's requesting authentication.

   Systems that do not use the custom query option should not need to make
   any additional setting, as the standard query authentication variables
   contain all the information that's needed to obtain a complete list of
   accounts.

   If only a small proportion of your users are entitled to use shared
   mailboxes, then it helps scalability enormously if you restrict the shared
   index file to contain just those accounts. The -s flag to authenumerate
   implements this, by discarding all accounts which have the option
   disableshared set to a non-zero value. Further efficiency can be gained by
   customising your database query so that the database itself returns only
   the relevant accounts. Use option MYSQL_ENUMERATE_CLAUSE,
   PGSQL_ENUMERATE_CLAUSE or LDAP_ENUMERATE_FILTER as appropriate.

   Note also that you can set DEFAULTOPTIONS="disableshared=1" in
   authdaemonrc to make sharing disabled for everyone, and then set option
   disableshared=0 only on permitted accounts.

    NOTE

     authenumerate tries not to download the complete results of each query
     into a memory buffer, but it may be that this still happens due to
     circumstances out of its control (e.g. older versions of MySQL or
     PostgreSQL client libraries may force this to happen). If so, it's
     possible that authenumerate's memory requirements may be large enough to
     affect the running system. In this case you will need to come up with an
     alternative mechanism to obtain the list of accounts, in some other way.

    NOTE

     The PAM library does not have a function that obtains a list of
     available accounts. Therefore, on all systems that use the authpwd,
     authshadow, or authpam modules, authenumerate works in exactly the same
     way: by using the getpwent() function. Systems that use certain PAM
     modules, such as ones that authenticate against a MySQL, a PostgreSQL,
     or an LDAP database, will not be able to use authenumerate, and must
     come up with a suitable replacement.

  Using sharedindexsplit

   As mentioned in the technical overview, a single index file may not be
   feasible if the number of accounts is more than a thousand, or so. At that
   point it becomes necessary to split the shared folder index into multiple
   files.

   The sharedindexsplit script reads a list of all accounts, formatted as a
   single shared folder index (which, incidentally, matches authenumerate's
   output format as well) and splits it into multiple files. The first
   argument to sharedindexsplit is the name of the directory where the output
   files are going to be created. The directory must be empty.

   sharedindexsplit splits the index into multiple files, based on either the
   'sharedgroup' account option, or the first character or characters of the
   account's name. Use the optional second argument to sharedindexsplit to
   specify a number, if splitting the account list based on initial
   characters is desired. If splitting based on the 'sharedgroup' account
   option then use the -o flag to authenumerate to get it to include the
   account options in its output.

   Perl 5.8.0, or greater, must be installed if account names include
   non-Latin characters. For best results, the shared folder index input to
   sharedindexsplit should already be sorted, but it doesn't have to be.

   Therefore, the following scripts should produce good results on a system
   with a large, but a moderate number of accounts. Split on 'sharedgroup' if
   you have a number of separate 'universes' where sharing is only permitted
   within each universe; otherwise split on the account name if all users can
   potentially access all shared mailboxes.

   --------------------------------------------------------------------------

 #!/bin/sh
 sysconfdir="/usr/lib/courier-imap/etc"
 sbindir="/usr/lib/courier-imap/sbin"

 rm -rf $sysconfdir/shared.tmp
 mkdir $sysconfdir/shared.tmp || exit 1

 # split on the 'sharedgroup' option
 $sbindir/authenumerate -s -o >$sysconfdir/shared.tmp/.tmplist || exit 1
 $sbindir/sharedindexsplit $sysconfdir/shared.tmp <$sysconfdir/shared.tmp/.tmplist  || exit 1
 rm -f $sysconfdir/shared.tmp/.tmplist
 $sbindir/sharedindexinstall

   --------------------------------------------------------------------------

 #!/bin/sh
 sysconfdir="/usr/lib/courier-imap/etc"
 sbindir="/usr/lib/courier-imap/sbin"

 rm -rf $sysconfdir/shared.tmp
 mkdir $sysconfdir/shared.tmp || exit 1

 # split on the first 1 character of the username
 $sbindir/authenumerate -s >$sysconfdir/shared.tmp/.tmplist || exit 1
 $sbindir/sharedindexsplit $sysconfdir/shared.tmp 1 <$sysconfdir/shared.tmp/.tmplist  || exit 1
 rm -f $sysconfdir/shared.tmp/.tmplist
 $sbindir/sharedindexinstall

   --------------------------------------------------------------------------

   authenumerate is saved to a temporary file, instead of being piped
   directly, is so that its exit code can be checked. We want to abort the
   entire process if authenumerate terminates with a non-zero exit code. If
   authenumerate pipes its output directly to sharedindexsplit, and aborts
   with an error, then sharedindexsplit will read an empty shared folder
   index, consequently the output directory will be empty, and then
   sharedindexinstall will obligingly install an empty list of shared
   folders.

   As mentioned previously, at some point authenumerate's memory requirements
   may become too much to handle, and an alternative mechanism will need to
   be improvised. sharedindexsplit's memory requirements are not dependent on
   the number of accounts, so this script can still be used even with very
   many accounts.

   When it's time to use account groups, SYSCONFDIR/shared/index will usually
   contain only group entries, with the accounts themselves dispersed in
   other files in the same directory.

Combining Courier-IMAP's and SqWebMail index files

   The information in this section is applicable only when running both
   Courier-IMAP and SqWebMail packages. This is not applicable when running
   Courier, which includes both the IMAP server and the webmail server. In
   the Courier build, both servers are set up to use the same shared index
   file.

   But, if both standalone builds are installed, Courier-IMAP will want to
   use /usr/lib/courier-imap/etc/shared/index as its shared index file, while
   SqWebMail will read /usr/local/share/sqwebmail/shared/index.

   To make both servers use the same shared folder index, create a soft link:

 ln -s /usr/lib/courier-imap/etc/shared /usr/local/share/sqwebmail/shared

   It's important to create a soft link to the entire "shared" directory,
   otherwise index group files will not work.

IMAP Access Control List and account groups

   Even after setting up shared folders indexes correctly, the mail client
   will show an empty list of shared folders. This is because even though
   each account's folders may be shared, they will not be visible to other
   accounts until the mail account's owner explicitly grants public access to
   a folder. This can be done using an IMAP mail client that understands
   access control lists, using Courier's SqWebMail, or the maildiracl
   command. The maildiracl manual page includes an overview of access control
   lists and how to control who gets what kind of rights on a folder. See the
   maildiracl manual page for more information.

                  Filesystem permissions-based shared folders

Terminology

     * Sharable Maildir - a maildir that contains folders that can be shared.
     * Sharable folder - one or more folders in a sharable Maildir that can
       be shared.

Technical Overview

     * They are a somewhat logical extension of the base Maildir format.
     * Older versions of Courier-IMAP and SqWebMail will completely ignore
       shared folders.
     * Messages in shared folders do not count towards any individual maildir
       quotas (see README.maildirquota.html).
     * Shared folders are implemented as additional, Maildir-based folders.
       The messages will be soft links to the messages in the sharable
       folder. Soft links will be used instead of hard links in order to be
       able to have a folder containing large messages, and then be able to
       reclaim space immediately when the messages are purged, instead of
       waiting for everyone to sync up and delete their hard link. The flip
       side is that you can expect the usual sorts of errors and increased
       confusion if someone attempts to read a message that has just been
       removed, but their client (Courier-IMAP or SqWebMail) doesn't know
       that yet. That's unavoidable. You'll probably have to set some kind of
       a policy in order to keep the expected insanity to a minimum.
     * Sharable folders are subscribed to by creating a separate maildir-type
       directory within the main Maildir. It's created in a separate
       subdirectory that is ignored by other Maildir clients.
     * The new directory will contains soft links to the messages in the
       sharable folder. "Synchronization" code will be used to synchronize
       the contents of the sharable folder, and the copy of it in the regular
       Maildir. Once links to the messages are created, they can be
       manipulated like regular messages in a Maildir. This procedure will be
       transparently performed by Courier-IMAP or SqWebMail behind the
       scenes.
     * Access to shared folders is controlled by a combination of an explicit
       configuration, plus regular filesystem permissions. If account owners
       have shell access to the server, they can create shared folders, and
       link their mailbox to other accounts' shared folders. Then, the actual
       access is controlled by regular filesystem permissions. The owner of
       the shared folder will use the regular filesystem permission to give
       read and/or write access to either everyone else, or everyone else who
       uses the same system group ID. Read access allows people to read
       messages from a shared folder. Write access allows people to add and
       delete messages in the shared. The owner of the shared folder can
       remove any message, everyone else will be able to remove only their
       own messages.

Functional Overview

   Generally speaking, shared folders are configured using a feature-enhanced
   maildirmake command as follows:

     * make install will install a maildirmake command that has a bunch of
       funny options. The modified maildirmake command is installed into
       Courier-IMAP's or SqWebMail's installation directory.
     * Somebody, maybe you, will use some of these options to create a
       maildir with modified permissions. The same somebody will run
       maildirmake again, with a few other options, to create folders in that
       maildir, also with modified permissions. Those permissions allows
       global read (and optionally write) access to those folders.
     * Do NOT use this maildir as the primary mailbox, INBOX, for an account.
       Instead, you must create this maildir separately, perhaps as
       $HOME/Maildir-shared, then set it up as one of your sharable maildirs
       (see below), and access it in shared mode. Because you own it, you
       have unlimited read/write access to it. The previously mentioned
       options will select whether or not access permissions are given to
       everyone else, and they do not apply to you.
     * Everyone else will run maildirmake with even more funny options. Those
       options install a link from their own maildir to a maildir that
       contains sharable folders. A given maildir may contain links to more
       than one sharable maildir. As long as their system user/group
       permissions allow them to access messages, they can SUBSCRIBE via
       their IMAP client to the folder, or use the SUBSCRIBE function in
       SqWebmail.
     * If people do not have shell access, the system administrator will have
       to run maildirmake on their behalf, in order to create the links to
       the sharable maildirs.
     * People with write access can add messages to a sharable folder.
       Messages from a sharable folder can be removed only by the owner of
       the shared folder, or by the owner of each message.
     * This sharable maildir implementation cannot use maildir soft-quotas.
       There cannot be a soft-quota installed in a sharable maildir. If you
       need quota support, you will have to use filesystem-based quotas.
       There are some sticky issues with updating quotas on a sharable
       maildir.

   Also, note that anyone, not just the superuser, can create a sharable
   maildir in their account, and invite anyone else to access it, as long as
   their system user/group permissions allow them access.

   To summarize:

     * Use the -S option to maildirmake to create a maildir that can contain
       sharable folders.
     * Use the -s and -f options to maildirmake to create sharable folders.
       The same sharable maildir may contain multiple sharable folders with
       different access permissions, as selected by the -s option.
     * Use the --add option to install a link from a regular maildir to a
       sharable maildir. Afterwards, IMAP clients will be able to subscribe
       to folders in the sharable maildir.
     * Use the --del option to remove a link.

   For more information on these options, see the maildirmake manual page
   that will be installed.

  More on additional options for the maildirmake command

   The '-S' option to maildirmake to create a maildir that will contain
   shared folders. The -S option gives group and world read permissions on
   the maildir itself (where group/world permissions are normally not set for
   a regular maildir). This allows access to any folders in the shared
   maildir, and that's why you should not use this Maildir directly as your
   primary mailbox.

   The "new" and "cur" subdirectories will not be used or shared, although
   they will still be created. Both SqWebMail and Courier-IMAP create their
   auxiliary files in the main Maildir with the group and world permissions
   turned off, so this maildir can, theoretically, still be used as the
   primary INBOX, but I don't recommend that.

   The -S option is not limited to the system administrator. In fact, anyone
   can use the -S option, to create shared maildirs that they maintain.

   Shared folders are created like any other folder, using the -f option to
   maildirmake. However, that normally creates a folder that is not sharable,
   because it will not have any group or world permissions. Therefore,
   maildirmake will take the following options to create a sharable folder:

     * -s read will create a "moderated" folder. The folder will have group
       and world read permissions, letting anyone read messages, but only the
       owner of the sharable Maildir can add messages to the sharable folder.
     * -s write will create an "unmoderated" folder. The folder will have
       group and world read/write permissions, letting anyone read and add
       messages to the folder. The folder will be created with the sticky bit
       set, like the /tmp directory, preventing people from removing someone
       else's messages.
     * -s read,group/-s write,group append a comma and the word "group" to
       modify the semantics of moderated and unmoderated folders only on the
       group level, not the group and world level, as far as permissions go.
       This restricts sharing the folder only to members of the same system
       group as the owner of the sharable maildir.

   It's worth noting that it is perfectly permissible for folders in the same
   sharable maildir to have different access levels.

   Also, this is driven entirely by filesystem permissions, so theoretically
   it's possible to create a folder that has write permissions for the group,
   and read permissions for everyone else. However, I'm too lazy to actually
   do it. Feel free to patch maildirmake to add this functionality, then send
   me the patch.

Accessing shared folders

   The rest of the document consists of technical implementation notes.

   Accessing a collection of shared folders is implemented by a new file that
   is installed in the primary maildir (usually $HOME/Maildir), and a new
   subdirectory hierarchy underneath the primary maildir, which are hereby
   declared.

  shared-maildirs

   This file must be created by the administrator or by the maildir owner,
   manually. This file lists all available sharable maildirs that this
   maildir can access in shared mode (confused yet?). This file contains one
   or more lines of the following format:

   name /path/to/shared/maildir

   "name" is an arbitrary name that's given to this collection of shared
   folders. The name may not contain slashes, periods, or spaces. This is
   followed by a pathname to the maildir containing shared folders. Note that
   this is NOT the sharable folder itself, but the maildir that contains one
   or more sharable folders. The maildir client will be able to selectively
   subscribe to any sharable folder in that maildir.

  shared-folders

   This subdirectory forms the root of all the shared folders that are
   subscribed to. Let's say that shared-maildirs has the following contents:

   firmwide /home/bigcheese/Maildir-shared tech /home/gearhead/Maildir-shared

   Subscribing to folders 'golf' and 'boat' in bigcheese's shared Maildir,
   and to the folder 'maintenance' in gearhead's shared Maildir involves
   creating the following subdirectories: shared-folders/firmwide/.golf,
   shared-folders/firmwide/.boat, and shared-folders/tech/.maintenance.

  shared

   This is a soft link that's automatically created in shared-folders/name.
   It is a soft link that points to the sharable maildir, which contains the
   sharable folders.

Subscribing to a shared folder

     * Read shared-maildirs.
     * Read the shared-folders hierarchy to determine which folders are
       already subscribed to.
     * Read the folders in each maildir listed in shared-folders, and ignore
       the ones that are already subscribed to. Make sure to stat() each
       folder to make sure that you can read it.
     * If necessary, create shared-folders/name, and create the
       shared-folders/name/shared soft link.
     * Create shared-folders/name/.foldername, then create the standard tmp,
       new, and cur subdirectories. All the directories are created without
       any group or world permissions.

Unsubscribing

     * Delete everything in shared-folders/name/foldername.
     * If this is there are no more subscribed folders in this sharable
       maildir, delete shared-folders/name for good measure as well.

Opening a shared folder

   The process of "opening" a shared folder involves basically "syncing" the
   contents of shared-folders/name/.foldername with the contents of the
   sharable folder in the original sharable maildir.

     * Do the usual processing on the tmp subdirectory.
     * Read the contents of shared-folders/name/foldername/shared/new. If you
       find anything in there, rename it to ../cur, ignoring any errors from
       the rename. The sharable maildir owner can arrange for mail to be
       delivered directly to this folder, and the first one to open it will
       put the message into cur.
     * Read the contents of shared-folder/name/foldername/shared/cur. Call
       this directory "A", for short.
     * Read the contents of shared-folder/name/foldername/cur. Call this
       directory "B", for short.
     * stat() A and B to see if they live on different devices.
     * Remove all messages in B that do not exist in A. You want to compare
       the filenames up to the : character.
     * Create soft links from messages that exist in A and do not exist in B.
       The name in B should not have any flags after the :, so it shows up as
       a new message.
     * You can now do whatever you normally do with a maildir. Note that new
       is completely ignored, though. Moving messages IN and OUT of the
       shared folder involves copying the message as if it were a new
       message. Copying the message INTO the shared folder means copying the
       message into the underlying sharable folder's tmp/cur directory, and
       it will show up after the next sync.

References

   Visible links
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#newshared
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#newsharedterm
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#newsharedtech
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#newsharedfunc
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#combo
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#imapacl
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldshared
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldsharedterm
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldsharedtech
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldsharedfunc
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldsharedaccess
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldsharedsub
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldsharedunsub
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#oldsharedopen
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/maildiracl
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.sharedfolders.html#newsharedtech
   . file:///home/mrsam/src/courier.git/courier-imap/libs/maildir/README.maildirquota.html
