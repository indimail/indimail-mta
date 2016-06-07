global Help
# aboutBox --
#
# Pops up a message box with an "about" message
#
set Help(About) {
	heading {About "IndiMail Administration"}

	text {Copyright (c) 2002-2003 IndiMail Limited}
	text {$Id: IndiMailHelp.tcl,v 2.5 2016-06-07 22:25:17+05:30 Cprogrammer Exp mbhangui $}
	text {}
	text {This is Indium Release 1.0 released on 01 Apr 2003 on All Fools day.}
	text {Indium itself and the associated command line utilities are covered by GPLv3 License.}
	text {}
	text {Report bugs to postmaster@indi.com.}
	text {If you find this program useful, a note to the author would be appreciated.}
	paragraph
	text {This code has been written to run only on reliable Operating Systems like unix}
	text {This code aborts if your OS has anything to do with words like 'Bill' or 'Gates'}
	text {If you have got this error, it means that you should either}
	text {}
	text {1. Let experts at Indi manage your mailing requirements}
	text {2. Change to a better Operating System}
	text {3. Sell your soul and downgrade your Mail Platform to something which doesn't run on unix}
	text {}
	text {The above choices are in the increasing order of complexity and is entirely left to your discretion}
	text {}
	text {}
	text {Have a good day}
}

#
# Pops up a message box with a "Help for Add Domain" message
#
set Help(adddomain) {
   heading {Help for "Add Domain"}
   text { Add Domain option enables an administrator to add a new domain on a particular server.The domain addition utility comes with a variety of configurable options}
   text {}
   text {}
   bold {Domain}
   text {      		Enter the fully qualified domain name you wish to create.}
   text {			example - test.com}
   text {}
   bold {Server}
   text {     		Choose the server on which the domain is to be created.}
   text {			example - 10.1.1.120}
   text {}
   bold {Password}
   text {    		Set a password for the postmaster user of the domain. The }
   text {			postmaster user will act as the}
   text {			administrator for the domain.}
   text {}
   bold {UserID}
   text {    		Choose a system(/etc/passwd) user from the drop }
   text {			down.The default user is indimail.}
   text {}
   bold {Uid}
   text {			This will be the unique identifier for the.}
   text {			userID chosen.The value can be chosen from the drop down.}
   text {			It is recommended that the uid be a value above 1.}
   text {}
   bold {Gid}
   text {			The value will be the unique identifier of the}
   text {			group to which the userid will belong.}
   text {}
   bold {Email for bounce}
   text {	The email address to which the default bounces for the }
   text {			domain goes, will be the postmaster. To choose a }
   text {			different email ID Click the EmailID}
   text {			button under the label "Bounce handling".}
   text {}
   bold {ETRN IP address}
   text {	The IP address of the server where the mails are}
   text {			queued and on demand the client can connect to the}
   text {			server and issue the ETRN command to download the mails.}
   text {}
   bold {Postmaster Quota}
   text {	The mailbox size of the postmaster ID can be set.}
   text {			The quota can also be set by using the sliders provided.}
   text {}
   bold {Domain Type}
   text {		Cluster Non Cluster.The default is a non cluster domain.}
   text {}
   bold {Bounce Handling}
   text {	Refer the "Email for Bounce" section above}
   text {}
   bold {Enable ETRN}
   text {		Refer the "ETRN IP address" section above.}
   text {			The ETRN feature is not enabled by default.}
   text {}
   bold {Enable Vfilter}
   text {		Enable mail filtering capabilities for the domain.}
   text {			By Default the feature is enabled for all domains.}
   text {}
   text {______________________________________________________________________}
   text {}
   text {          command vadddomain+     }
   text {}
   text {We recommend that all domains be setup as virtualdomains. This is done through the progam vadddomain.}

   text {}
   bold {SYNOPSYS}
   text {}
   text {}
   text {vadddomain [options] virtual domain [postmaster password]}
   text {}
   bold {DESCRIPTION}
   text {}
   text {}
   text {Add a new virtual domain.}
   text {All qmail files and system are automatically updated. It also adds the RFC821 required postmaster account.The command carries out the following steps to create the domain.}
   text {  i)   Create the domains directory (/var/indimail/domains/domain_name)}
   text {  ii)  Create the .qmail-default file in the domains directory.}
   text {  iii) Add the domain to qmail assign file.}
   text {  iv)  Add the domain to rcpthosts, virtualdomains (and etrnhosts}
   text {       for domains with ETRN/AUTOTURN support).}
   text {  v)   Create table domain_name if indimail has been configured}
   text {       as a large site. For a small site, creates table indimail}
   text {       and indibak if used for the first time.}
   text {  vi)  Sends SIGHUP to qmail-send.}
   text {  vii) Create the postmaster account for a non-clustered domain.}
   text {       For a clustered domain, the postmaster account is}
   text {       created on the first host on which the domain is created.}
   text {}
   bold {OPTIONS}
   text {}
   text {}
   text {-V   print version}
   text {-v   verbose}
   text {-q   quota_in_bytes (sets the quota for postmaster account)}
   text {-b       (bounces all mail that doesn't match a user, default)}
   text {-e   email_address (forwards all non matching user to this address)}
   text {-u   user (sets the uid/gid based on a user in /etc/passwd).}
   text {      Default is user indimail.}
   text {-d   dir (sets the dir to use for this domain)}
   text {-i   uid (sets the uid to use for this domain)}
   text {-g   gid (sets the gid to use for this domain)}
   text {-a   sets the account to use APOP, default is POP}
   text {-O   optimize adding, for bulk adds set this for all}
   text {     except the last one}
   text {-f   sets the domain to use vfilter mechanism for mail filtering.}
   text {-t   Enable ETRN support for the domain.}
   text {-T <ip_address> Enable AUTOTURN Support form IP ip_address}
   text {}
   text {[virtual domain] Name of the new virtual domain}
   text {}
   text {[postmaster password] The password for the RFC required}
   text {      postmaster account. If the password is not supplied on the}
   text {      command line then vadddomain will prompt for the password twice.}
   text {}
   bold {RETURN VALUE}
   text {}
   text {}
   text {0 if all steps were successful, non-zero otherwise.}
   text {If any of the steps fail, a diagnostic message is printed.}

}
#
# Pops up a message box with a "Help for Modify Domain" message
#
set Help(moddomain) {
	heading {Help for "Modify Domain"}
	text { A domain and its enabled features can be modified with the capability of setting previously disabled features}
   	text {}
   	text {}
	bold {Domain}
	text {		Choose the domain you wish to modify from the}
	text {			drop down list.}
	text {}
	bold {Server}
	text {			Choose from the drop down list, the server IP address}
	text {			to which the domain to be modified belongs.}
	text {}
	bold {Domain Info}
	text {		Provides valuable information about the domain.}
	text {			Same as the domain info option.}
	text {}
	bold {Rename Domain}
	text {	The chosen domain can be renamed with this option.}
	text {}
	bold {Add Alias Domain}
	text {	An alias for the chosen domain can be created.}
	text {}
	bold {Delete Domain}
	text {		The chosen domain can be delted from the server}
	text {}
	text {}
	text {_____________________________________________________________________}
	text {}
	text {		command 	vaddaliasdomain+}
	text {}
	bold {SYNOPSYS}
	text {}
	text {}
	text {vaddaliasdomain new_domain old_domain}
	text {}
	bold {DESCRIPTION}
	text {}
	text {}
	text {Take an existing domain and link a new domain to all the}
	text {accounts on the existing domain.}
	text {}
	text {The command does the following}
	text {}
	text {	i)   Insert into table aliasdomain on the Control Host (if old_domain is }
	text {		 clustered).}
	text {	ii)  Create a symbolic link new_domain linked}
	text {		 to old_domain in the domains directory.}
	text {	iii)  Add entry to qmail assign file.}
	text {	iv)  Send SIGHUP to qmail-send.}
	text {	v)   Add entry to .aliasdomains in the domains directory for }
	text {		 the domain old_domain.}
	text {}
	bold {OPTIONS}
	text {}
	text {}
	text {new_domain   The desired name for the new alias domain.}
	text {}
	text {old_domain   The name of an existing real domain to which }
	text {			   the domain new_domain needs to be aliased.}
	text {}
	bold {RETURN VALUE}
	text {}
	text {}
	text {0 if all steps were successful, non-zero otherwise.}
	text {If any of the steps fail, a diagnostic message is printed.}
	text {}
	text {}
	text {_____________________________________________________________________}
	text {}
	text {		command 	vdeldomain+}
	text {}
	bold {SYNOPSYS}
	text {}
	text {}
	text {vdeldomain [-vV] domain_name}
	text {}
	bold {DESCRIPTION}
	text {}
	text {}
	text {Completely removes a virtual domain from the system. Including updating all qmail files and removing all email users and their directories. For a clustered domain, vdeldomain should be run on the host having the postmaster account.}
	text {}
	text {For real domain, it does the following}
	text {}
	text {	i)    Delete all domains aliased to the real domain being removed}
	text {		  (from the file .aliasdomains)}
	text {	ii)   Remove entry for real domain in qmail assign file.}
	text {	iii)   Recursively remove directories in all filesystems}
	text {		  pertaining to the real domain.}
	text {	iv)   Remove entries in dir_control tables pertaining to}
	text {		  the real domain.}
	text {	v)    Remove the Domain directory}
	text {	vi)   Remove all entries for domain in tables indimail}
	text {		  and indibak.}
	text {	vii)  Remove entries for domain in rcpthosts,}
	text {		  morercpthosts, virtualdomains.}
	text {	viii) Send SIGHUP to qmail-send}
	text {}
	text {For alias domains, it does the following}
	text {}
	text {	i)    Delete entry from aliasdomain table on the Control Host.}
	text {	ii)   Remove entry from .aliasdomains.}
	text {	iii)  Remove entry for alias domain in qmail assign file.}
	text {	vi)   Recursively remove directories in all filesystems}
	text {		  pertaining to the real domain.}
	text {	v)    Remove entries in dir_control tables pertaining}
	text {		  to the alias  domain.}
	text {	vi)   Remove the Domain directory (symbolic link)}
	text {	vii)  Remove all entries for domain in tables}
	text {		  indimail and indibak.}
	text {	viii) Remove entries for domain in rcpthosts,}
	text {		  morercpthosts, virtualdomains}
	text {	ix)   Send SIGHUP to qmail-send}
	text {}
	text {For domain with AUTOTURN support, it does the following}
	text {i)    Recursively remove Maildir for the domain}
	text {ii)   Remove entries for domain in rcpthosts,}
	text {		morercpthosts, virtualdomains and etrnhosts.}
	text {iii)  Send SIGHUP to qmail-send}
	text {}
	bold {OPTIONS}
	text {}
	text {}
	text {-t  Deletes a domain enabled with AUTOTURN support.}
	text {-v  Print verbose messages on the screen.}
	text {-V  Prints version numbers.}
	text {}
	text {domain_name The domain name to remove.}
	text {}
	bold {RETURN VALUE}
	text {}
	text {0 if all steps were successful, non-zero otherwise.}
	text {If any of the steps fail, a diagnostic message is printed.}
	text {}
	text {}
	text {_____________________________________________________________________}
	text {}
	text {		command 	vdominfo}
	text {}
	bold {SYNOPSYS}
	text {}
	text {}
	text {vdominfo [domain_name]}
	text {}
	bold {DESCRIPTION}
	text {}
	text {}
	text {vdominfo displays domain information for all domains (or for a }
	text {particular domain given on the command line).The information is given as below}
	text {}
	text {Domain  :    the domain name (the real domain in}
	text {			   case of an alias domain).}
	text {uid     :    uid of the virtualdomain}
	text {gid     :    gid of the virtualdomain}
	text {H Id    :    hostid for the host where the domain is created.}
	text {Ip      :    IP Address of the host.}
	text {Dir     :    domain directory for the domain.}
	text {Users   :    number of users in this domain. (Not}
	text {			   displayed for an alias domain)}
	text {}
	bold {OPTIONS}
	text {}
	text {}
	text {[domain_name]    The name of the virtual domain.}
	text {}
	bold {RETURN VALUE}
	text {}
	text {}
	text {0 for success, 1 for any failure.}
}
#
# Pops up a message box with a "Help for Add User" message
#
set Help(adduser) {
	heading {Help for "Add User"}
	text {The "user add" utiliy enables an administrator to add a user to a particular domain.}
	text {}
	bold {Username}
	text {		Enter the username to be added to the domain}
	text {			example : tom_jerry}
	text {}
	bold {Password}
	text {		Set a password for the user not exceeding 8 characters}
	text {			example : mail!@#}
	text {}
	bold {Domain}
	text {		Choose a domain from the drop down list to}
	text {			which the user should belong to}
	text {}
	bold {Server}
	text {			Choose the server from the drop down}
	text {			on which the user's mail directory should reside}
	text {}
	bold {Comment}
	text {		Add a comment for the user}
	text {}
	bold {Mail Quota}
	text {		Set the mailbox size for the user. The}
	text {			same can also be set using the sliders provided.}
	text {}
	bold {Generate Password}
	text {	Generate a random password for the user}
	text {}
	bold {Password Type}
	text {	Choose the way the password is to be stored}
	text {}
	bold {Filesystem balance}
	text {}
	text {}
	text {________________________________________________________________}
	text {}
	text {		command		vadduser+ }
	text {}
	bold {SYNOPSYS}
	text {}
	text {}
	text {vadduser [options] email_address password}
	text {}
	bold {DESCRIPTION}
	text {}
	text {}
	text {Adds a new user to a virtual domain. No additional modifications to the system are required.The account created is inactive. On login (IMAP4 or POP3), the account becomes active immediately. The username should be either alpha-numeric or have the characters '.', '-', '_' . The username, gecos and domain component cannot have the ':' character. The case is changed to lower case before adding to the database. For a clustered domain, the user is also added to the table hostcntrl on the Control Host. The Directory Structure Layout's hashed component (See Below) is incremented by one.}
	text {}
	text {The user's home dirctory has four components}
	text {		i)   Base Path}
	text {		ii)  Directory Prefix}
	text {		iii) Domain Name}
	text {		iv)  Hashed Component}
	text {}
	text {The Base Path is defined by the environment variable ISOCOR_BASE_PATH. If this variable is not set, the value is taken from BASE_PATH defined in indimail.h. If vfstab (-b option) is run periodically, -b option in vadduser can be used to balance optimally user creation across filesystems to distribute the load.}
	text {}
	text {The Directory Prefix depends on the first character of the username. It can have one of the five values}
	text {}
	text {A2E: 		First character of username is a alphabet including and }
	text {	   	lying in between 'a' and 'e'}
	text {}
	text {F2K: 		First character of username is a alphabet including and }
	text {	   	lying in between 'f' and 'k'}
	text {}
	text {L2P: 		First character of username is a alphabet including and }
	text {	   	lying in between 'l' and 'p'}
	text {}
	text {Q2S: 		First character of username is a alphabet including and }
	text {	   	lying in between 'q' and 's'}
	text {}
	text {T2Zsym: 	First character of username is a alphabet including }
	text {		  and lying in between 't' and 'z' or starts with a }
	text {		  non alphabetic character.}
	text {}
	text {The Domain Name component is derived from the virtual domain (domain component) of the username.}
	text {}
	text {The Hashed Component is constructed using an adaptive directory structure which is automatically managed by the core indimail api functions "vadduser" and "vdeluser". This structure is known as Directory Structure Layout. For sites with 100 users or less, all user directories are stored in the virtual domain directory. For sites that go above 100 users the adaptive directory structure goes into effect. The basic idea is to break up the user Maildir directories across multiple directories and sub directories so that there are never more than 100 user directories in a single directory.}
	text {}
	text {The default directory setup allows for 62 directories in 3 levels and 100 user directories per directory. The total number of user directories is equal to 100 + (62 * 100) + (62 * 62 * 100) + (62 * 62 * 62 * 100) = over 24 million directories. This should be more than sufficient for any site and probably goes beyond the technology of directory structures.}
	text {}
	text {If you are going to be storing large numbers of user directories, make sure you set your file system to have a higher than normal percentage of inodes.}                                                                 text {vadduser will automatically create these directories and sub directories as needed and populate each directory with up to 100 user accounts. As soon as a directory reaches 100 users it will create the next directory or sub directory and store the new users directory there.}
	text {}
	text {Over a period of time, due to large no of deletions of users, the Directory Structure Layout  for all users can be regenerated using the program vreorg.}
	text {}
	bold {OPTIONS}
	text {}
	text {}
	text {-V		Display Version Numbers}
	text {}
	text {-v		Set verbose mode.}
	text {}
	text {-c Comment	Set Comment (Sets the gecos comment field)}
	text {}
	text {-d		Create the directory for the user.If this option is }
	text {		not given, the home directory is not created. It gets }
	text {		created when the user logs in either through IMAP4 }
	text {		or POP3 protocol.}
	text {}
	text {-e passwd	Expects Standard Encryption Password }
	text {		to be given on command line.}
	text {}
	text {-r		Generates Random Password. Need not give password }
	text {		on command line.}
	text {}
	text {-a		Set the account to use APOP instead of the default POP.}
	text {}
	text {-b		Balances users across filesystems listed in fstab }
	text {		table as they are created. This option should be used if}
	text {		vfstab (with -b option) is enabled in cron.}
	text {}
	text {-q [quota in bytes] 	Set the hard quota limit for the user.}
	text {			If not supplied then the default system hard quota limit}
	text {			is set. The default limit is either 50M or what ever is set }
	text {			via --enable-hardquota. If set to NOQUOTA then }
	text {			the user will have no quota limit.}
	text {}
	text {-h <MailDeliveryHost>	For a clustered domain, this option }
	text {			can be used to create the user on a specific cluster.}
	text {}
	text {email_address		The new email address of the user.}
	text {			Requires the domain name as well as the user name.}
	text {			For example: user@domain.com. If the domain name }
	text {			is not specified the user is added to the default domain.}
	text {}
	text {password		Set the password for the user. If the password}
	text {			is not supplied on the commandline then vadduser}
	text {			will prompt standard in for the password. The password }
	text {			must be typed in twice.}
	text {}
	text {The user is added to the inactive table indibak (except for RFC ids postmaster and abuse) and is treated as an inactive user until the user logs in, upon which the user record is moved to the active table indimail.}
	text {}
	bold {RETURN VALUE}
	text {}
	text {}
	text {0 if all steps were successful, non-zero otherwise.}
	text {If any of the steps fail, a diagnostic message is printed.}
	text {}

}
#
# Pops up a message box with a "Help for Modify User" message
#
set Help(moduser) {
	heading {Help for "Modify User"}
	text {}
	text {}

	text {The "Modify User" utility enables the administrator to modify the attributes of an email account}
	text {}

	bold {Username}
        text {	Enter the username for which the attributes needs to}
        text {		be modified}
        text {		example:tom_jerry}
	text {}
	bold {Go}
        text {		Existing information for the user can be obtained on}
        text {		clicking the "Go" button}
	text {}

	bold {Trash}
        text {		The user will be deleted on clicking the Trash symbol}
	text {}

	bold {Domain}
        text {	Choose the domain to which the user to be modified belongs}
	text {}

	bold {Server}
        text {		Choose the server on which the user ID is present}
        text {		example: 10.1.1.201}
	text {}

	bold {Password}
        text {	Enter the Password to be modified for the selected user}
        text {		either in clear text or crypted format and click on}
        text {		"Modify" button for the changes to take place.}
	text {}

	bold {UserID}
        text {		Choose a system(/etc/passwd) user from the drop down}
        text {		the default is indimail}
	text {}

	bold {Comments}
        text {		Changes the gecos (comment) field for the user}
	text {}

	bold {Directory}
       	text {		Select the directory path where the user's inbox}
        text {			needs to be created the default is /mail/}
	text {}

	bold {Mail Quota}
      	text {		Changes the malbox quota for the selected user}
	text {}

	bold {Inactive}
        text {		Toggles between Inactive and Active state for the }
        text {			selected user}
	text {}

	bold {No Dialup}
       	text {		If this Flag is set for the selected user, the dialup access }
       	text {			will be disabled}
	text {}

	bold {No Password Change}
      	text {		If this flag is set for the selected user, the user will} 
	text {				not be able to change the password}
	text {}

	bold {No POP3 Access}
  	text {		If this flag is set for the selected user, the pop3 }
  	text {			access will be disabled}
	text {}

	bold {No IMAP Access}
  	text {		If this flag is set for the selected user, the imap }
  	text {			access will be disabled}
	text {}
	bold {No WEB Access}
   	text {		If this flag is set for the selected user, the web }
   	text {			access will be disabled}
	text {}

	bold {Bounce Flag}
     	text {		If this flag is set for the selected user, mails sent} 
	text {			to the selected user will get bounced back to the sender}
	text {}

	bold {No Relay}
        text {		If this flag is set for the selected user, the user will} 
	text {			be allowed to relay without pop-before-smtp}
	text {}

	bold {Admin Privileges}
        text {		If this flag is set for the selected user, the user }
        text {			gains adminsitator privileges}
	text {}

	bold {Reset Privilege}
 	text {		If this flag is set On, it will clear }
 	text {			all the flags for the selected user}
	text {}

	bold {Alias}
        text {		This column displays the aliases/forwarding addresses created}
	text {          for the selected user. To add/modify/delete an alias/forwarding for}
	text {          a specific user, clickon and hold down the right mouse button anywhere}
	text {          in the space provided below the "alias" column and choose the appropriate}
	text {          option from the menu displayed.}
	text {		For example: to add an alias, enter the alias/forwarding address as} 
	text {	        &tom_jerry@indi.com (Please note that there is an "&" prefixed to the email ID}	
	text {}
	text {}
text {---------------------------------------------------------------}
text {}
text {}

        text {command         		vmoduser}
text {}

text {}
bold {SYNOPSYS}
text {}
text {}

text {vmoduser[options] email_addr}
text {}
text {}

bold {DESCRIPTION}
text {}
text {}
text {           vmoduser modifies attributes of an email account}
text {      	 by modifying fields in the authentication tables}
text {  	 indimail or indibak}
text {}
text {}

bold {OPTIONS}
text {}
text {}
text {-V}
text {Print Version numbers}
text {}

text {-v}
text {Sets verbose mode}
text {}

text {-n}
text {Toggles between Inactive and Active state for the user i.e. Moves}
text {the user between indimail and indibak}
text {}

text {-q quota}
text {Changes the quota for the user. Changes the field pw_shell in}
text {indimail or indibak.}
text {}

text {-c comment}
text {Changes the gecos (comment) field for the user. Changes the field}
text {pw_gecos in indimail or indibak}
text {}

text {-P clear_text}
text {Changes the password for the user after encrypting the clear text}
text {password given on the command line. Changes the field pw_passwd in}
text {indimail or indibak.}
text {}

text {-e encrypted_passwd}
text {Changes the encrypted password for the user as given after '-e'}
text {argument (without performing any encryption). It is expected to}
text {supply a crypted password using crypt(3) library call. Changes}
text {the field pw_passwd in indimail or indibak}
text {}

text {-l vacation_messsage_file}
text {Sets up autoresponder for the user. It creates .qmail file in}
text {user's homedir containing path to the autoresponder program vacation.}
text {If vacation_message_file is specified as '-', autoresponder is}
text {removed. If it is specified as '+', the text for autoresponder is}
text {taken from STDIN. Any other value is taken as a file containing}
text {text for the autoresponder}
text {}

text {The following options are bit flags in the gid int field}
text {(pw_gid in indimail or indibak)}
text {}

text {-u      Set no dialup flag.}
text {}

text {-d      Set no password changing flag for the user}
text {}

text {-p      Set no POP3 access flag.}
text {}

text {-w      Set no web mail access flag.}
text {}

text {-i      Set no IMAP4 access flag.}
text {}
text {-b      Set bounce mail flag.}
text {}

text {-r      Set no external relay flag.}
text {}

text {-a      Grant qmailadmin administrator privileges.}
text {}

text {-0}
text {}
text {-1}
text {}
text {-2}
text {}
text {-3}
text {}
text {Reserved for future use.}
text {}

text {-x Clear all flags}
text {}
text {}

text {The values of pw_gid corresponding to the above options are as below.}
text {}

text {Option                  Value}
text {}
text {NO_PASSWD_CHNG          0x01}
text {NO_POP                  0x02}
text {NO_WEBMAIL              0x04}
text {NO_IMAP                 0x08}
text {BOUNCE_MAIL             0x10}
text {NO_RELAY                0x20}
text {NO_DIALUP               0x40}
text {QA_ADMIN                0x80}
text {V_USER0                 0x100}
text {V_USER1                 0x200}
text {V_USER2                 0x400}
text {V_USER3                 0x800}
text {V_USER4                 0x2000}
text {V_USER6                 0x8000}
text {}
text {}

bold {RETURN VALUES}
text {}
text {}
text {0 in case of success and non-zero in case of any failure.}
text {}
text {}

}

set Help(deluser) {
	heading {Help for "Delete User"}
	text {}
	text {}

        text {The "Delete user" utility enables an administrator to delete a}
        text {user id permanently from the system.}
	text {}

	bold {Username}
        text {	Enter the name of the user whose email id needs to}
        text { 		be deleted from the system after clicking on}
        text {		"Delete" button.}
	text {}

	bold {Domain}
        text {	Select the Domain Name from the drop down}
        text {		list to which the userid belongs.}
	text {}

	bold {Server}  
        text {		Choose from the drop down list, the server IP address}
        text {		to which the domain to be modified belongs.}
	text {}
	text {}


text {-----------------------------------------------------------}
text {}
text {}

text { command 		vdeluser+}
text {}
text {}

bold {SYNOPSYS}
text {}
text {}
text {vdeluser [options] email address}
text {}
text {}

bold {DESCRIPTION}
text {}
text {}
                text {Completely removes a virtual domain email}
                text {account from the system. Takes effect immediately.}
                text {All files and directories for the user are removed}
                text {from disk. All forwarding, aliases, entries in}
                text {database are also removed. The Directory Structure}
                text {Layout's Hashed Component is decremented by one.}
                text {For a clustered domain, the user will be deleted}
                text {only if the user lies on the host where vdeluser}
                text {is being run.}
text {}
text {}

bold {OPTIONS}
text {}
text {}
text {-V}
text {Display Version Numbers}
text {}
text {-v}
text {Set verbose mode.}
text {}

text {email address}
text {Fully qualified email account. For example: user@domain.com.}
text {If no domain is specified then the account is removed from the}
text {default virtual domain.}
text {}

bold {RETURN VALUE}
text {}
text {}
text {0 if all steps were successful, non-zero otherwise. If any of}
text {the steps fail, a diagnostic message is printed.}
text {}
text {}

}

set Help(DBmgmt) {
	heading {Help for "Database management"}
	text {}
	bold {SMTP}
        text { 		This sets up artificial routes for a clustered domain}
	text { 		which helps to deliver mails to the mailstore}
	text {}
	bold {Domain}
	text { 	Select the domain from the drop down list and click on the}
	text { 		"Submit" button to view the artificial routes set for that}
	text { 		particular domain. If there is no information displayed}
	text { 		then it indicates that the domain is not a clustered }
	text { 		domain}
	text {}
	bold {Server}
	text { 	Select the IP/host from the drop down list where the }
	text { 		domain exists}
	text {}
	bold {HostID}
	text { 	This displays the hostid to IP address mapping for }
	text { 		clustered domains when clicked on "Submit" button.}
	text {}
	bold {Dbinfo}
	text { 	dbinfo displays the status of all sql servers used}
	text { 		by Mail listed in the sql file}
	text {}
	bold {Domain}
	text { 	Select the domain from the drop down list for which the}
	text { 		dbinfo is to be displayed and click on the "Submit" button} 
	text { 		Right click on the entries displayed to add/modify/delete}
	text {}
	bold {Server}
	text { 	Select the server IP on which the sql file resides}
	text {}
	bold {MCD File}
	text { 	Select the path to the control file sql}
	text { 		default : /var/qmail/control/sql}
	text {}
	bold {Clustered}
	text {  Indicates whether the domain is a clustered domain or not}
	text {  If it is in released state, it indicates that the domain is}
	text {  not  clustered otherwise it is a clustered domain }
	text {}
	bold {FSTAB}
	text {          vfstab is a utility for maintaining the FSTAB table}
	text {          (Filesystem table).The FSTAB table maintains the filesystems}
	text {          which can be used for serving the user mailboxes. Normally}
	text {          when creating an user account (adduser utility), the}
	text {          environment variable ISOCOR_BASE_PATH is used. This is}
	text {          however not optimal when having multiple filesystems.}
	text {          In such case, the administrator can use the -b option}
	text {          in vadduser to distribute the users across multiple filesystems,}
	text {          taking into account the load of the different filesystems}
	text {          Right click anywhere within the space provided above the}
	text {          "Submit" button to add/modify/delete a filesystem}
	text {}
	text {}
	bold {Domain}
	text {}
	text { Select the domain from the drop down list and click on} 
	text { the "submit" button to view the FSTAB entries}
	text {}
	bold {Server}
	text {}
	text { Select the server IP where the domain exists} 
	text {}
	bold {MDA Host}
	text {}
	text { Represents the IP of the server where the user's mailboxes are stored}
	text {}
	bold {IPMAP}
	text {}
	text { vipmap maps an ip address on a mailstore to a virtual domain.}
	text { Select the domain and the IP of the server and click on the }
	text { "Submit" button to view the existing IP mappings for the }
	text { selected virtual domain}
	text { Right click on the space available above the "Submit" button}
	text { to add/modify/delete a IP map}
	text {}
	bold {Domain}
	text {}
	text { Select the domain from the drop down list.}
	text {}
	bold {Server}
	text {}
	text { Select the Server IP on which the domain exists.}
	text {}
	bold {Ports}
	text {}
	text { This utility helps in managing the attributes of an admin user)}
	text { like change password, add admin user and delete an admin userr)}
	text { The admin user  is used to connect to indisrvr.Indisrvr is an }
	text { administration server for administering Mail's clustered domains.} 
	text {}
	bold {Domain}
	text {}
	text { Select the domain from the drop down list for which the admin user}
	text { attributes needs to be changed and click on "Info" button to disply}
	text { the attributes}
	text {}
	bold {Filter}
	text {}
	text { This utility helps creating, modifying and}
	text { deleting Mail's powerful filters which are used to }
	text { block or take some action on the mails received }
	text { Currently, filter option allows maximum of 255 filters to be created.}
	text { Select the domain name,the user name and choose "info" under options}
	text { then click on "info" button to view the filters for the selected user}
	text {}
	bold {Domain}
	text {}
	text {Select the domain to which the user belongs}
	text {}
	bold {Server}
	text {}
	text {iSelect the server on which the domain resides}
	text {}
	bold {Username}
	text {}
	text { Select the user for whom you wish to add the filters}
	text {}
	bold {Filter Name}
	text {}
	text { Enter a name for the filter you are going to create}
	text { ex : test_filter1}
	text {}
	bold {Header}
	text {}
	text { Select a header from the drop down list}
	text { ex : From}
	text {}
	bold {Comparison}
	text {}
	text { Select a comparison from the drop down list}
	text { ex : Equals}
	bold {Keyword}
	text {}
	text { Type the keyword that you wish to match the comparison}
	text { ex : porn}
	text {}
	text {}
	bold {Destination}
	text {}
	text { Type the destination which is a folder that gets created}
	text { when the filter matches with the mail received}
	text {}
	bold {Forwarding}
	text {}
	text { The address to which the mail will get forwarded }
	text { to if a filter match occurs}
	text {}
text {--------------------------------------------------------------------}
text {}
text {}
text {command 		vsmtp}
text {}
text {}
bold {SYNOPSYS}
text {}
text {}
text {vsmtp [options] [host@domain_name -m mta | -s domain_name]}
text {}
bold {DESCRIPTION}
text {}
text {}
text {          vsmtp sets up artificial routes for qmail-remote }
text {          to be able to deliver mails to the mailstore on }
text {          host host and domain domain_name. This route is }
text {          used by qmail-remote for clustered domains. }
text {          For all other domains the control/smtproutes }
text {          is consulted. In case, the Control Host is }
text {          unavailable due to any reason, qmail-remote }
text {          fall backs to control/smtproutes and expects the }
text {          MDA on the mailstore to re-route the mail to the }
text {          correct destination.}
text {}
text {}
bold {OPTIONS}
text {}
text {}
text {-V }
text {Print Version Numbers}
text {}
text {-v}
text {Sets Verbose option.}
text {}
text {-s}
text {Show smtp ports for a virtual domain or SMTP Route }
text {between a MTA host and a mailstore having the domain domain_name}
text {}
text {-d}
text {Delete SMTP Route between a MTA host and a mailstore.}
text {}
text {-i port}
text {Add SMTP Route between a MTA host and a mailstore}
text {}
text {-u port}
text {Change SMTP Route between a MTA host and a mailstore}
text {}
text {-m mta_ipaddress}
text {The ip address of the MTA which needs a SMTP route to the mailstore.}
text {}
text {host@domain}
text {The name of the mailstore followed by '@' sign and the domain_name.}
text {}
text {domain_name}
text {The name of the virtual domain.}
text {}
bold {RETURN VALUES}
text {}
text {}
text {0 for success, 1 for any failure.}
text {}
text {}
text {---------------------------------------------------------------}
text {}
text {}
text {  command 		vhostid}
text {}
text {}
bold {SYNOPSYS}
text {}
text {}
text {vhostid [options] [HostId]}
text {}
bold {DESCRIPTION}
text {}
text {}
text {          vhostid displays the hostid to IP address mapping}
text {          used my indimail for clustered domains. Each host}
text {          in a clustered setup is assigned a unique hostid}
text {          with the help of vhostid. Subscquently, vhostid}
text {          can be used to display, insert, update or delete}
text {          these mappings}
text {}
bold {OPTIONS}
text {}
text {}
text {-V}
text {Print Version Numbers}
text {}
text {-v}
text {Sets Verbose Option}
text {}
text {-s}
text {Display hostid Mappings for all hosts}
text {(if hostid is not given on command line.)}
text {Else displays the hostid to IP}
text {Address mapping only for the host with id HostId.}
text {}
text {-d HostId}
text {Delete IP address Mapping for HostId}
text {}
text {-i Ipaddr}
text {Maps IP address Ipaddr to hostid HostId}
text {}
text {-u Ipaddr}
text {(Map HostId to New_Ipaddr)}
text {Changes IP address for hostid Hostid }
text {to IP address Ipaddr}
text {}
bold {RETURN VALUE}
text {}
text {}
text {0 if successful, 1 for any failure.}
text {}
text {}
text {------------------------------------------------------------------------------------}
text {}
text {}
text {  command dbinfo}
text {}
text {}
bold {SYNOPSYS}
text {}
text {}
text {dbinfo [sql_file]}
text {}
bold {DESCRIPTION}
text {}
text {}
text {          dbinfo displays the status of all sql servers used}
text {          by IndiMail listed in the sql file. If sql_file}
text {          is not given, the environment variable SQLFILE}
text {          is used. If environment variable is not set, the}
text {          static definition of SQLFILE in indimail.h is used}
text {          The information displayed is as follows}
text {}
text {Domain            satyam.net.in   Non clustered}
text {sqlserver[003]    202.144.76.7}
text {mda host  	202.144.76.48}
text {TCP/IP Port       3306}
text {database  	indimail}
text {user              indimail}
text {password  	ssh-1.-5-}
text {fd                4}
text {}
text {}
text {Mysql Stat        Uptime: 6105  Threads: 2 Questions: 15  Slow queries: 0 }
text {          	Opens: 7  Flush tables: 1 }
text {          	Open tables: 1  Queries per second avg: 0.002}
text {}
bold {OPTIONS}
text {}
text {}
text {[sql_file]}
text {Name of a file containing Mysql, Mda configuration for IndiMail}
text {}
bold {RETURN VALUE}
text {}
text {}
text {0 for success, 1 for any failure.}
text {}
text {}
text {----------------------------------------------------------------------------}
text {}
text {}
text {  command 	vfstab}
text {}
text {}
text {}
bold {SYNOPSYS}
text {}
text {}
text {vfstab -d | -i | -l | -u | -o | -n user_quota -q size_quota -m mdaHost Filesystem}
text {vfstab -s [-m mdaHost]}
text {vfstab -b}
text {}
text {}
text {DESCRIPTION}
text {}
text {}
text {          vfstab is a utility for maintaining the FSTAB table}
text {          The FSTAB table maintains the filesystems which can}
text {          be used for serving the user mailboxes. Normally}
text {          when creating an user account (vadduser), the}
text {          environment variable ISOCOR_BASE_PATH is used}
text {          This however is not optimal when having multiple}
text {          filesystems. In such case, the administrator can}
text {          use the -b option in vadduser to distribute the}
text {          users across multiple filesystems, taking into}
text {          account the load of the different filesystems}
text {          The load factor is computed by running vfstab}
text {          with -b option periodically through cron(1)}
text {}
bold {OPTIONS}
text {}
text {}
text {-V}
text {Print version numbers}
text {}
text {-v}
text { Sets verbose option}
text {}
text {-d }
text {Delete local/remote filesystem}
text {}
text {-i}
text {Insert local/remote filesystem}
text {}
text {-u}
text {Update local/remote filesystem}
text {}
text {-o}
text {Make   local/remote filesystem offline/online}
text {}
text {-l}
text {Add     local filesystem. If -n and -q options are not given,}
text {  the values user_quota and size_quota are automatically}
text {  calculated from the free disk space available and by}
text {  assuming an average size of the mailbox to be the}
text {  value of the environment variable AVG_USER_QUOTA}
text {}
text {-n}
text {Max number of users to be added on this filesystem.}
text {Max number of users to be added on this filesystem.}
text {}
text {-q}
text {Max size of filesystem}
text {}
text {-m}
text {Mail Delivery Host. The host on which the user's mailbox lie.}
text {}
text {-s}
text {Show filesystems configured in FSTAB table. }
text {If -m option is given, displays the filesystem }
text {only for the specified Mail Delivery Host.}
text {}
text {-b}
text {Balance filesystems by computing load.}
text {}
text {Filesystem}
text {Name of a valid filesystem.}
text {}
bold {RETURN VALUE}
text {Returns 0 for success, 1 for any failure.}
text {}
text {----------------------------------------------------------------}
text {  command vcfilter}
text {}
text {SYNOPSYS}
text {vcfilter [options] emailid [mailing_list1..mailing_listn]}
text {vcfilter -m emailid [options] [mailing_list1..mailing_listn]}
text {}
text {DESCRIPTION}
text {          vcfilter is a utility for creating, modifying, }
text {          deleting IndiMail's powerful filter called as vfilter. }
text {          There are two options; one for administration of filters }
text {          and another for administration of mailing lists }
text {          subscribed by the user. For mailing lists special }
text {          action needs to be performed by vfilter, so that }
text {          valid mails do not get rejected. Currently, }
text {          vcfilter allows maximum of 255 filters to be created.}
text {}
text {          The first form of usage is primararily for creating }
text {          filters (though some amount of mailing list updation }
text {          is also possible). The second form of usage is for }
text {          mailing list updation only.}
text {}
text {OPTIONS  (for filters)}
text {-v}
text {Sets verbose option}
text {}
text {-s}
text {Show Filters}
text {}
text {-i}
text {add filter}
text {}
text {-d filter_no}
text {delete filter(s)}
text {}
text {-u filter_no}
text {update filter}
text {}
text {-t filter_name}
text {Textual name for the filter}
text {}
text {-h }
text {header value. A number denoting the following}
text {-1 ~V If comparision (-c option) is 5 or 6}
text {}
text {0 ~V Return-Path}
text {1 ~V From}
text {2 ~V Subject}
text {3 ~V To}
text {4 ~V Cc}
text {5 ~V Bcc}
text {6 ~V Reply-To}
text {7 ~V Sender        }
text {8 ~V List-Id}
text {9 ~V Mailing-List}
text {10 ~V X-Sender}
text {11 ~V X-Mailing-List}
text {12 ~V X-ML-Name}
text {13 ~V X-List   }
text {14 ~V X-Loop}
text {15 ~V X-BeenThere}
text {16 ~V X-Sequence}
text {17 ~V X-Mailer}
text {18 ~V Precedence}
text {}
text { }
text {-c}
text {Comparision with the headers to make. This could }
text {be one of the following number.}
text {}
text {0 ~V Equals}
text {1 ~V Contains}
text {2 ~V Does not contain}
text {3 ~V Starts with}
text {4 ~V Ends with}
text {5 ~V Sender not in address book}
text {6 ~V My ID not in To, Cc, Bcc}
text {7 ~V RegExp}
text {Note: Mailing List can be specified for Comparision 5 or 6}
text {}
text {-k keyword}
text {A keyword or string which will be used in comparision }
text {with the header value (depending on -c option). The }
text {string should be a NULL "" string if -c option is either }
text {5 or 6. The keyword can also be any Regular Expression }
text {(i.e. *, ? [], etc) if comparision equals 7.}
text {}
text {-f Folder}
text {Folder to which the mail should be delivered if a filter }
text {match occurs. This can be the Trash folder in case the }
text {mail needs to be rejected. It can be /NoDeliver if the }
text {mail needs to be discarded completely.}
text {}
text {-b bounce_action}
text {0.One of the four numbers}
text {1.Do not bounce}
text {2.Bounce mail to sender with the following custom message}
text {"Hi. This is the IndiMail MDA for <domain_name>.}
text {I'm afraid I cannot accept your message as my SPAM filter has decided}
text {to reject this email.}
text {Please refrain from sending such mail in future."}
text {3.Forward mail to address following the number. }
text {An ampersand '&' sign must separate the number }
text {and the address. i.e. 3&editor@indi.comwill }
text {forward all rejected mails to the mailbox editor@indi.com}
text {4.Same as above but bounce as well.}
text {}
text {mailing_list1...mailing_listn}
text {List of mailing lists which should be recognized by vfilter }
text {(if the filter comparision (-c option) is 5 or 6. This }
text {could also be a NULL "" string in which case, }
text {vfilter will intelligently try to figure out mails }
text {from mailing lists.}
text {}
text {OPTIONS  (for mailing lists)}
text {-v}
text {Sets verbose option}
text {}
text {-m}
text {Show Mailing list which should be allowed in case filter rule rejects the mail.}
text {}
text {-i}
text {add mailing lists}
text {}
text {-D }
text {delete mailing lists}
text {}
text {-U old_mailing_list_name new_mailing_list_name}
text {Change name of an existing mailing list to another name.}
text {}
text {-o option (used only for comparision 5 or 6)}
text {Change behavior when figuring out mailing lists for }
text {comparision 5 or 6. Following are the valid options}
text {}
text {0 ~V Do not consider mailing lists in filtering}
text {1 ~V Consider mailing lists while filtering intelligently}
text {2 ~V Consider only mailing lists specified in the filter when filtering.}
text {}
text {mailing_list1...mailing_listn}
text {List of mailing lists to be added or deleted.}
text {RETURN VALUE}
text {Returns 0 for success, 1 for any failure.}
text {}
text {}
text {-------------------------------------------------------------------------}
text {}
text {}
text {Command vipmap}
text {}
bold {SYNOPSYS}
text {}
text {}
text {vipmap [options] ip_address domain_name}
text {}
bold {DESCRIPTION}
text {}
text {}
text {          vipmap maps an ip address on a mailstore }
text {          to a virtual domain. In case a host has }
text {          multiple ip addresses, individual ip addresses }
text {          can be mapped to individual virtual domains. }
text {          This can be used instead of the environment }
text {          variable DEFAULT_DOMAIN or VPOPMAIL_DOMAIN. }
text {          The environment variable VPOPMAIL_DOMAIN takes precedence.}
text {}
bold {OPTIONS}
text {}
text {}
text {-d ipaddr domain_name}
text {Deletes mapping for ip address ipaddr and domain domain_name}
text {}
text {-a ipaddr domain_name}
text {Adds a mapping between IP address ipaddr and domain domain_name}
text {}
text {-p}
text {Prints mapping between all IP addresses and virtual domains.}
text {}
text {-V}
text {Prints Version Numbers}
text {}
text {-v}
text {Sets verbose option.}
text {}
bold {RETURN VALUES}
text {}
text {}
text {0 for success, 1 for any failure.}
text {}
text {}
text {----------------------------------------------------------------------------}
text {}
text {}
}


set Help(svcmgmt) {

 	heading {Help For "Service Management"}
	text {}
	text {}
	text {This utility helps the administrator to monotor and as well configure the}
	text {variuos services like smtp,imap, pop3 etc.}
	text {}

	bold {Monitor}
        text {	This tab option helps the administrator to monitor and}
        text { 		control the services}
	text {}

	bold {Service Directory}
	text {}
	text {}
	text { 		Click on the folder symbol displayed against the}
        text { 		this option to select the directory where all the}
        text { 		mail services are configured.}
        text { 		For example: /service1}
	text {}

	bold {Start Services}
	text {}
	text {}
  	text { 		This option helps in starting a service}
        text { 		select the service from the drop down and click on the "up"}
        text { 		arrow to bring up the service}
        text { 		The green signal indicates that all the services are}
        text { 		up and running}
        text { 		The signal changes to red if any one of the configured}
        text { 		services goes down}
	text {}

	bold {Stop Services}
	text {}
	text {}
   	text {		This option helps in stopping a service}
	text {}

	bold {StartAll}
	text {}
	text {}
        text { 		When clicked on this button, it starts all the}
        text { 		configured services}
	text {}

	bold {StopAll}
	text {}
	text {}
        text {		when clicked on this button, it stops all the}
        text {		configured services}
	text {}

	bold {Config}
	text {}
	text {}
        text {		This tab option helps in configuring various services}
        text {   		listed in the drop down menu.}
	text {}

	bold {Service Name}
	text {}
	text {}
    	text {		Select the name of the service to be created and configured}
        text {		from the drop down}
        text {		example : smtp}
	text {}

	bold {Service Dir}
	text {}
	text {}
     	text {		The name and path of the directory under which the service}
        text {		startup files need to be created}
        text {		ex : /service1}
	text {}

	bold {Control Dir}
	text {}
	text {}
     	text {		Select the path to the control directory where all the}
        text {		control files are stored}
        text {		default : /var/qmail/control}
	text {}

	bold {SMTP port}
	text {}
	text {}
       	text {		TCP/IP port number for smtp}
        text {		default : 25}
	text {}

	bold {Queue Base}
	text {}
	text {}
      	text {		The base directory where all the queues are installed}
        text {		ex: /queue}
	text {}

	bold {Min Free}
	text {}
	text {}
        text {		Minimum Disk Space to maintain in queue after which Temporary }
	text {		error will be returned}
	text {}

	bold {Check User}
	text {}
	text {}
      	text {		Perform a User Status query before accepting mails (qmail-smtpd)}
	text {}

	bold {Check Relay}
	text {}
	text {}
     	text {		Perform a Relay Status query before sending out mails (qmail-smtpd)}
	text {}

	bold {Dynamic Routing}
	text {}
	text {}
 	text {		Perform Dynamic routing (instead of static smtproutes)}
	text {}

	bold {RBL Check}
	text {}
	text {}
       	text {		Deploy RBL lookups}
	text {}

	bold {Skip Send}
	text {}
	text {}
       	text {		Skip creation of send script}
	text {}
	text {}

text {-----------------------------------------------------------------------}
text {}
text {}
	
text {command 		svctool}
text {}
text {}

bold {SYNOPSYS}
text {}
text {}
text {svctool [OPTION]}
text {}

bold {OPTIONS}
text {}
text {}

text {--smtp=port --qbase=queue_path --qcount=N --qstart=I --servicedir=service_path }
text {[--chkusr --chkrelay] --cntrldir=cntrl_path [--routes --min-free=M --rbl --skipsend]}
text {}

text { Installs a new queue with a SMTP Listener}
text {}

text {  N            - No of Queues}
text {}
text {  I            - Numeral Prefix of first queue (i.e 1 for /var/qmail/queue1)}
text {}
text {  M            - Minimum Disk Space to maintain in queue after }
text {                 which Temporary error will be returned}
text {}
text {  Port         - Bind on this port for SMTP}
text {}
text {  service_path - Path where supervise scripts will be installed}
text {}
text {  queue_path   - Path where the queues are installed. If this is different }
text {                 from /var/qmail}
text {}
text {  appropriate links will be created in /var/qmail}
text {}
text {  cntrl_path   - Path where Qmail control files are stored}
text {}
text {  chkusr       - Perform a User Status query before accepting mails (qmail-smtpd)}
text {}
text {  chkrelay     - Perform a Relay Status query before sending out mails (qmail-smtpd)}
text {}
text {  routes       - Perform Dynamic routing (instead of static smtproutes)}
text {}
text {  rbl          - Deploy RBL lookups}
text {}
text {  skipsend     - Skip creation of send script}
text {}
text {}

text {--imap=port --servicedir=service_path [--nolastauth] [--proxy=destport]}
text { --maxdaemons=m --maxperip=i --ldapaccess}
text {}
text {}

text {  Installs a new IMAP4 Listner}
text {}
text {  Port         - Bind on this port for IMAP}
text {}
text {  service_path - Path where supervise scripts will be installed}
text {}
text {  nolastauth   - Do not update lastauth}
text {}
text {  proxy        - Install as a proxy}
text {}
text {  m            - Concurrency of connections to allow}
text {}
text {  i            - Concurrency of connections to be allowed from a single ip address}
text {}
text {  ldapaccess   - Use ldap for access control}
text {}
text {}

text {--pop3=port --servicedir=service_path [--nolastauth] [--proxy=destport]}
text { --maxdaemons=m --maxperip=i --ldapaccess}
text {}
text {}

text {  Installs a new POP3 Listner}
text {}
text {  Port         - Bind on this port for POP3}
text {}
text {  service_path - Path where supervise scripts will be installed}
text {}
text {  nolastauth   - Do not update lastauth}
text {}
text {  proxy        - Install as a proxy}
text {}
text {  m            - Concurrency of connections to allow}
text {}
text {  i            - Concurrency of connections to be allowed from a single ip address}
text {}
text {  ldapaccess   - Use ldap for access control}
text {}
text {}

text {--qmail=DIR}
text {}

text {Installs a Qmail Binaries in DIR}
text {}

text {--daemontools=DIR}
text {}

text {  Installs a Daemontoos Binaries in DIR}
text {}

text {--ucspi-tcp=DIR}
text {}

text {  Installs a UCSPI-TCP Binaries in DIR}
text {}

text {--showctl=DIR}
text {}

text {  Shows Qmail Control Information for Control Directory at DIR}
text {}

text {--queuefix=queue_path}
text {}

text {  Fix Qmail Queue where queue_path is the absolute path of a queue}
text {}

text {--rmsvc=service --servicedir=service_path}
text {}

text {  Disable supervise scripts for service}
text {}

text {--ensvc=service --servicedir=service_path}
text {}

text {  Enable supervise scripts for service}
text {}

text {--dbserver=fifo_path --threads=N --ldaphost=host --ldappass=passwd --servicedir=service_path}
text {}

text {  Installs a new Fifo Server}
text {}
text {  fifo_path    - Install fifo specified by this path (e.g. /var/qmail/dbquery/dbfifo)}
text {}
text {  N            - No of parallel dbserver threads to spawn}
text {}
text {  ldaphost     - Host Serving LDAP requests}
text {}
text {  ldappass     - Passwd required for ldap_bind}
text {}
text {  service_path - Path where supervise scripts will be installed}
text {}

text {--indisrvr=port --mysqlhost=host --mysqluser=user --mysqlpass=passwd --avguserquota=quota}
text {}
text {       --hardquota=quota --isocorbasepath=path --maxdaemons=m --servicedir=service_path}
text {}
text {  Installs a new Indi Admin Server}
text {}
text {  port           - TCP/IP port on which to bind}
text {}
text {  mysqlhost      - Mysql Host having authentication tables}
text {}
text {  mysqluser      - Username for connecting to mysql}
text {}
text {  mysqlpass      - Passwd for connecting to mysql}
text {}
text {  avguserquota   - Average Usage per user in Bytes}
text {}
text {  hardqutoa      - Max Quota for a user}
text {}
text {  isocorbasepath - Default Filesystem Path for user mailbox creation}
text {}
text {  m              - Concurrency of connections to allow}
text {}
text {  service_path   - Path where supervise scripts will be installed}
text {}

text {}
text {--mysql=mysql_port --basedir=path --datadir=path --config=conf_file --servicedir=service_path}
text {}

text {  Installs a new Mysql Server}
text {}
text {  mysql_port   - Install Mysql to listen on this ort}
text {}
text {  basedir      - Base Installation Directory for mysql}
text {}
text {  datadir      - Directory containing the Database}
text {}
text {  config       - Mysql Configuration file (e.g. /etc/my.cnf)}
text {}
text {  service_path - Path where supervise scripts will be installed}
text {}

text {--fetchmail --qcount=N --qstart=I --servicedir=service_path}
text {}

text {  Install Fetchmail Server}
text {}

text {--folderdaemon=port --servicedir=service_path --maxdaemons=m [--nolastauth]}
text {}

text {  Installs a new Folder Daemon}
text {}
text {  Port         - Bind on this port for POP3}
text {}
text {  service_path - Path where supervise scripts will be installed}
text {}
text {  nolastauth   - Do not update lastauth}
text {}
text {  m            - Concurrency of connections to allow}
text {}
text {  i            - Concurrency of connections to be allowed from a single ip address}
text {}

text {}
text {--help}
text {}
text {		display this help and exit}
text {}

text {--version}
text {}
text {}
text {}

text {--------------------------------------------------------------------}
text {}
text {}
text {        command svc}
text {}

text {       svc - controls services monitored by supervise(8).}
text {}

bold {SYNOPSIS}
text {}
text {}

text {svc [ -udopchaitkx ] services}
text {}

bold {DESCRIPTION}
text {}
text {}
text {                services consists of any number of arguments,}
text {                each argument naming a directory used by supervise(8).}

text {}
text {                svc applies all the options to each service in turn.}

text {}
bold {OPTIONS}
text {}
text {}
text {-u     Up. If the service is not running, start it. If the service stops,}
text {       restart it.}
text {}
text {-d     Down. If the service is running, send it a TERM signal and then}
text {       a CONT signal. After it stops, do not restart it.}
text {       If the script shutdown exists, run shutdown to terminate the service}
text {}
text {-o     Once. If the service is not running, start it. Do not restart it if it stops.}
text {}
text {-p     Pause. Send the service a STOP signal.}
text {}
text {-c     Continue. Send the service a CONT signal.}
text {}
text {-h     Hangup. Send the service a HUP signal.}
text {}
text {-a     Alarm. Send the service an ALRM signal.}
text {}
text {-i     Interrupt. Send the service an INT signal.}
text {}
text {-t     Terminate. Send the service a TERM signal.}
text {}
text {-k     Kill. Send the service a KILL signal.}
text {}
text {-x     Exit. supervise(8) will exit as soon as the service is down.}
text {}
text {       If you use this option on a  stable  system,  you're}
text {       doing something wrong; supervise(8) is designed to run forever.}
text {}
text {}
text {----------------------------------------------------------------------}
text {}
text {   command supervise}
text {}

text {    supervise - starts and monitors a service.}
text {}

bold {SYNOPSIS}
text {}
text {}
text {       supervise s}
text {}

bold {DESCRIPTION}
text {}
text {}
text {          supervise  switches  to  the directory named}
text { 		s and starts ./run. It restarts ./run if ./run exits.}
text {      	It pauses for a second after starting ./run, so that}
text {   	it does not loop too quickly if ./run exits immediately.}
text {}

text { 		If the file s/down exists, supervise does not start ./run}
text {   	immediately. You can use svc(8) to  start  ./run  and  to  give}
text { 		other commands to supervise.}
text {}

text {  	supervise  maintains  status  information  in a binary format}
text {    	inside the directory s/supervise, which must be writable to}
text { 		supervise.  The status information can be read by svstat(8).}
text {}

text {	   	supervise may exit immediately after startup if it cannot}
text {  	find the files it needs in s or if another copy of supervise is}
text {   	already running in s.  Once supervise is successfully running,}
text { 		it will not exit unless it is killed or specifically asked}
text { 		to exit. You can use svok(8) to check whether supervise is}
text {  	successfully running. You can use svscan(8) to reliably  start}
text {   	a collection of supervise processes}
text {}
text {}
text {-------------------------------------------------------------------------------}
text {}
text {}
text {   command 		svstat}
text {}
text {}

text {  svstat - prints the status of services monitored by supervise(8).}
text {}

bold {SYNOPSIS}
text {}
text {}
text {svstat services}
text {}

bold {DESCRIPTION}
text {}
text {}
text {           services  consists  of  any number of arguments,}
text {      	 each argument naming a directory.  svstat prints one}
text {  	 human-readable line for each directory, saying whether}
text {           supervise(8) is successfully running in  that  directory,}
text {   	 and  reporting  the  status information maintained by supervise(8).}
text {}
text {}

}

set Help(editcontrol) {

	heading {Help on "Control Files"}
	text {}
	text {}

	text {This section contains information you will need to configure}
	text {Mail server to make it work the way you want it to}
	text {}
	text {}
	bold {Control Directory}
	text {}
	text {}
	text {			Select the directory where all the control}
        text {			files are stored ex : /var/qmail/control}

	bold {Control File}
	text {}
	text {}
	text {			Select the control file you wish to modify}
        text {			and click on the "Edit" button}
	text {}
	text {}
	text {---------------------------------------------------------------}
	text {}
	text {}

	bold {badmailfrom}
	text {}
	text {}
	text {This control file is used by qmail-smtpd.}
	text {This file contains blacklisted "From" addresses}
	text {Sample entries :  cubangirls@list.escortenterprises.com} 
	text { 			*@jackpot101.com}
	text {}

	bold {badmailpatterns}
	text {}
	text {}
	text {This control file is used by qmail-smtpd.}
	text {This file contains blacklisted "From" addresses and  wildcard}
	text {patterns can be given. For example: *porn*}
	text {}

	bold {badrcptto}
	text {}
	text {}
	text {This control file is used by qmail-smtpd.}
	text {This file contains blacklisted "To" addresses}
	text {}

	bold {badrcptpatterns}
	text {}
	text {}
	text {This control file is used by qmail-smtpd.}
	text {This file contains blacklisted "To" addresses it can}
	text {contain wildcard patterns}
	text {}

	bold {bouncefrom}
	text {}
	text {}
	text {The default value is MAILER-DAEMON}
	text {This control file is used by qmail-send}
	text {This file contains username of bounce sender}
	text {}
	text {}

	bold {bouncehost}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-send}
	text {This file contains hostname of bounce sender}
	text {}

	bold {bouncesubject}
	text {}
	text {}
	text {The default value is  "failure notice"}
	text {This control file is used by qmail-send}
	text {This file contains subject for failure notices}
	text {}

	bold {bouncemessage}
	text {}
	text {}
	text {The default value is QSBMF}
	text {This control file is used by qmail-send}
	text {This file contains the bouncemessage text }
	text {}

	bold {doublebouncesubject}
	text {}
	text {}
	text {The default value is "failure notice"}
	text {This control file is used by qmail-send}
	text {This file contains subject for double bounce}
	text {}

	bold {doublebouncemessage}
	text {}
	text {}
	text {The default value is QSBMF}
	text {This control file is used by qmail-send}
	text {This file contains text for double bounce}
	text {}

	bold {bouncemaxbytes}
	text {}
	text {}
	text {This control file is used by qmail-send}
	text {If bounce text is greater than this size, the bounce is truncated}
	text {}

	bold {concurrencylocal}
	text {}
	text {}
	text {The default value is 10}
	text {This control file is used by qmail-send}
	text {This file contains max simultaneous local deliveries}
	text {}

	bold {concurrencyremote}
	text {}
	text {}
	text {The default value is 20}
	text {This control file is used by qmail-send}
	text {This file contains  max simultaneous remote deliveries}
	text {}

	bold {defaultdomain}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-inject}
	text {This file contains  default domain name}
	text {}

	bold {defaulthost}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-inject}
	text {This file contains  default host name}
	text {}

	bold {databytes}
	text {}
	text {}
	text {The default value is 0}
	text {This control file is used by qmail-smtpd, qmail-inject, sendmail}
	text {This file contains Max number of bytes in message (0=no limit)}
	text {}

	bold {maxhops}
	text {}
	text {}
	text {The default value is 100}
	text {This control file is used by qmail-smtpd}
	text {This file contains Messages with hops greater than this number is rejected}
	text {}

	bold {doublebouncehost}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-send}
	text {This file contains host name of double bouncesender}
	text {}

	bold {doublebounceto}
	text {}
	text {}
	text {The default value is postmaster}
	text {This control file is used by qmail-send}
	text {This file contains user to receivedouble bounces}
	text {}

	bold {envnoathost}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-send}
	text {This file contains default domain for addresses without "@"}
	text {}

	bold {helohost}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-remote}
	text {This file contains host name used in SMTP HELO command}
	text {}

	bold {idhost}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-inject}
	text {This file contains host name for Message-ID's}
	text {}

	bold {localiphost}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-smtpd}
	text {This file contains name substituted for local IP address}
	text {}

	bold {outgoingip}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-remote}
	text {This file contains Outgoing IP to use for delivering remote mails.}
	text {Used with hosts having multiple ethernet interfaces}
	text {}

	bold {locals}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-send}
	text {This file contains domains that we deliver locally}
	text {}

	bold {me}
	text {}
	text {}
	text {The default value is FQDN of system}
	text {This control file is used by various}
	text {This file contains default for many control files}
	text {}

	bold {morercpthosts}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains secondary  rcpthosts database}
	text {}

	bold {percenthack}
	text {}
	text {}
	text {This control file is used by qmail-send}
	text {This file contains domains that can use "%"-style relaying}
	text {}

	bold {nodnscheck}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains Domains excluded from domain name validification}
	text {}

	bold {plusdomain}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-inject}
	text {This file contains domain substituted for trailing "+"}
	text {}

	bold {qmqpservers}
	text {}
	text {}
	text {This control file is used by qmail-qmqpc}
	text {This file contains IP addresses of QMQP servers}
	text {}

	bold {queuelifetime}
	text {}
	text {}
	text {The default value is 604800}
	text {This control file is used by qmail-send}
	text {This file contains seconds a message can remain in queue}
	text {}

	bold {rcpthosts}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains domains that we accept mail for}
	text {}

	bold {smtpgreeting}
	text {}
	text {}
	text {The default value is me}
	text {This control file is used by qmail-smtpd}
	text {This file contains SMTP greeting message}
	text {}

	bold {smtproutes}
	text {}
	text {}
	text {This control file is used by qmail-remote}
	text {This file contains artificial SMTP routes}
	text {}

	bold {timeoutconnect}
	text {}
	text {}
	text {The default value is 60}
	text {This control file is used by qmail-remote}
	text {This file contains how long, in seconds, to wait for SMTP connection}
	text {}

	bold {timeoutremote}
	text {}
	text {}
	text {The default value is 1200}
	text {This control file is used by qmail-remote}
	text {This file contains how long, in seconds, to wait for remote server}
	text {}

	bold {timeoutsmtpd}
	text {}
	text {}
	text {The default value is 1200}
	text {This control file is used by qmail-smtpd}
	text {This file contains how long, in seconds, to wait for SMTP client}
	text {}

	bold {timeoutread}
	text {}
	text {}
	text {The default value is 4}
	text {This control file is used by qmail-smtpd}
	text {This file contains How long, in seconds, to wait for query from dbserver}
	text {}

	bold {timeoutwrite}
	text {}
	text {}
	text {The default value is 4}
	text {This control file is used by dbquery}
	text {This file contains How long will dbquery wait for writing to client fifo}
	text {}

	bold {virtualdomains}
	text {}
	text {}
	text {This control file is used by qmail-send}
	text {This file contains virtual domains and users}
	text {}

	bold {etrnhosts}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains domains for which etrn is enabled.}
	text {}

	bold {chkusrdomains}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains domains for which 'User Status' checking is enabled.}
	text {}

	bold {relayclients}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains Hosts allowed to relay}
	text {}

	bold {relaydomains}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains Domains allowed to relay}
	text {}

	bold {relaymailfrom}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains Relaying allowed for mails having this envelope sender}
	text {}

	bold {tarpitcount}
	text {}
	text {}
	text {This control file is used by qmail-smtpd}
	text {This file contains No of RCPT Tos to accept before starting tarpitting.}
	text {}

	bold {tarpitdelay}
	text {}
	text {}
	text {The default value is 5}
	text {This control file is used by qmail-smtpd}
	text {This file contains No of seconds of delay to introduce after each}
	text {subsequent RCPT TO once tarpitcount has been reached}
	text {}
	text {For more information about a particular control file, Refer the manual page}
	text {man -M /var/qmail/man qmail-control.}
	text {}
	text {}
}

#
# Pops up a message box with a "Help for Bulletin" message
#
set Help(bulletin) {
	heading {Help for Bulletin}
	text { The bulletin feature allows to deliver emails to users inbox by}
	text { copying the mail to the maildir of the user or by creating a link in the maildir.}
	text {}
	bold {Email File}
	text {		Choose the file containing the message that needs to }
	text {			be delivered.}
	text {}
	bold {Exclude File}
	text {		Choose the file which contains the users to be}
	text {			excluded from receiving the mail.}
	text {}
	bold {Subscriber List}
	text {	Choose the list of users who are to receive the mail.}
	text {}
	bold {Domain}
	text {		The domain to which the users who are to receive }
	text {			the mail belong.}
	text {}
	bold {Server}
	text {			The server on which the domain resides.}
	text {}
	bold {Email}
	text {			Enable the "Dry run" check box.The email can}
	text {			be sent to a single ID (usually an administrators email}
	text {			address) to test the message delivery before actually }
	text {			delivering the message to the users.}
	text {}
	bold {Bulkmail}
	text {}
	text {}
	bold {Dry run}
	text {		Refer the "Email" section described above.}
	text {}
	bold {Instant Bulletin}
	text {	Enable the instant bulletin option to deliver}
	text {			a mail to many users without wasting system resources.}
	text {}
	bold {Copy file}
	text {		Create a copy of the message in the maildirs of }
	text {			all users receiving the mail.}
	text {}
	bold {Symb link}
	text {		Create a soft link of the message in the maildirs }
	text {			of all users receiving the mail without actually }
	text {			copying the file to the maildir.}
	text {}
	bold {Hard link}
	text {		Create a hard link of the message to the users maildir.}
	text {}
	text {}
	text {___________________________________________________________________}
	text {}
	text {		command		vpopbull+	}
	text {}
	text {}
	bold {SYNOPSYS}
	text {}
	text {}
	text {vpopbull [-f filename ] [-e exclude email addresses file] [-v] [-n] [-c] [-h] [-s] 
	[-S subscriber_list_file] [virtual domain ...]}
	text {}
	bold {DESCRIPTION}
	text {}
	text {}
	text {vpopbull implements instant bulletin for IndiMail. It does this by copying an email file in the user's inbox (Maildir). The various options given on the command line decide how this copying is to be done.}
	text {}
	bold {OPTIONS}
	text {}
	text {}
	text {[-v]	 	Verbose mode. Prints out each email address it is sending to.}
	text {}
	text {[-f filename]   	File containing the email message to be posted.}
	text {}
	text {[-e filename]	File containing a list of email addresses }
	text {	 	to exclude from posting.}
	text {}
	text {[-n]        Don't actually mail it. -v and -n can be used to }
	text {	list out all virtual domain email accounts.}
	text {}
	text {[-c]        Default, copy message to users directory.}
	text {}
	text {[-h]        Make a hard link from email file to virtual users }
	text {	directory. Email file must be on the same physical device }
	text {	as the virtual users directories. This will save disk space.}
	text {}
	text {[-s]        Make a soft link from the email file to the virtual }
	text {	users directory. This will save on disk space but will not }
	text {	remove the file when all users read it. If the original file }
	text {	is deleted, users will not be able to read the message.}
	text {}
	text {[-S subscriber_list_file]	Take the list of subscriber list from }
	text {			a file and populates the bulkmail table. The IMAP4/POP3 }
	text {			daemon query this table when a user logs in and creates a }
	text {			link in the user's inbox (Maildir) to the email_file. This }
	text {			causes instant delivery of the bulletin without using any }
	text {			disk space. For a clustered domain, vpopbull can be safely }
	text {			run on all the hosts giving the same subscriber_list file.}
	text {			Vpopbull takes care of identifying the subscribers listed in}
	text {			subscriber_list file present on the host and those which are}
	text {			present on some other host.}
	text {}
	text {[virtual domain ... ]}
	text {}
	text {		List of domains to send the message to. If this is not}
	text {		supplied then the message is sent to all virtual domains.}
	text {}
	bold {RETURN VALUE}
	text {}
	text {}
	text {0 if all steps were successful, non-zero otherwise.}
	text {If any of the steps fail, a diagnostic message is printed.}
	text {}
}

set Help(addgroup) {
	heading {Help for "Add Group"}
	text {}
	text {The "Add Group" Utility enables administrator to add groups in a domain}
	text {}
	bold {Group Name}
	text {}
	text {          Group Name is the name of the group to be added}
	text {}
	bold {Domain Name}
	text {}
	text {          Domain Name is the name of the domain where the }
	text {group would be listed.}
	text {}
	bold {Email}
	text {}
	text {          Email represents the  group's mailing Address}
	text {}
	bold {Member Mailid}
	text {}
	text {          Lists all memebers in a group}
	text {}
	bold {Options}
	text {}
	bold {group List}
	text {}
	text {Lists all groups }
	text {}
	bold {Add Group}
	text {}
	text {          Add Group tab enables administrator to add a group.}
	text {          Field required for the same are Email,group Name}
	text {}
	bold {Modify Group}
	text {}
	text { Pls. key in the help}
	text {}
	bold {Delete Group}
	text {}
	text {          Delete Group tab enables administrator to delete the group after }
	text {          confirmation. Group name is the required field for delete group.}
	text {}
	bold {Add Member}
	text {}
	text {          Add Member tab enables administrator to add an email account to}
	text {          the group.Member Email is the required field for the Add member option}
	text {}
	bold {Modify Member}
	text {}
	text {          Modify Member option enables administrator to change an existing }
	text {          member of a group.Choose the domain name,Server,Member email id }
	text {          and corresponding member to be modified.Member Email is the required }
	text {          field for the Add member option}
	text {}
	bold {Delete Member}
	text {}
	text {          Delete Member tab enables administrator to delete a member from}
	text {          a group.Choose the domain name,Server,Member email id and }
	text {          corresponding member to be deleted.Click on delete to remove }
	text {          the Member email id.}
	text {}
	text {-----------------------------------------------------------------}
	text {                  Command  vgroup }
	text {}
	text {}
	text {usage1: vgroup -a [-h] [-c] [-q] [-v] [-V] group_email_address [password]}
	text {usage2: vgroup [-i] [-d] [-u] [-v] [-V] group_email_address}
	text {}
	bold {options}
	text {          -V (print version number)}
	text {         -v (verbose)}
	text {         -a (Add new group)}
	text {         -h Mail_Delivery_Host (host on which the account needs to be created)}
	text {         -c comment (sets the gecos comment field)}
	text {         -q quota_in_bytes (sets the users quota)}
	text {         -i member_email_address (insert member to group)}
	text {         -d member_email_address (delete member from group)}
	text {         -u new_member_email_address -o old_member_email_address (update member with a new address)}
	text {}
	text {}
}


set Help(queuefix) {
heading {Help for "Queue-Fix"}
	text {}
	text {This command fixes damaged qmail queue directory.}
	text {Note  The Queue-Fix program can take a significant }
	text {amount of time to run on a large database}
	text {}
	bold {Queue Directory }
	text {}
	text {                  This specifies the Queue directory upn which the }
	text {                  program should run which fixes up all corrupt file systems}
	text {}
	bold {Single Queue Architecture }
	text {}
	text {                  In a single Queue Architecture the field would be ideally}
	text {                  set to /var/qmail/queue which signifies the qmail queue system}
	text {}
	bold {Multi Queue Architectute}
	text {}
	text {                  In a multi queue Achitecure the field could be set to }
	text {                  /var/qmail/queueN where integer "N" signifies the queue }
	text {                  directory on the qmail queue system}
	text {}
	text {                  Condsider an qmail queue system with "5" queue architecture}
	text {                  to fix the third queue,field can be set to}
	text {                  /var/qmail/queue3}
	text {}
	bold {Run}
	text {}
	text {                  Run tab repairs the corrupt queue directory}
	text {}
	text {-----------------------------------------------------------------}
	text { command   queue-fix}
	text {}
	text {}
	bold {NAME}
	text {       queue-fix - Repairs the corrupt qmail queue system}
	text {}
	text {}
	bold {NAME}
	text {       queue-fix - Repairs the corrupt qmail queue system}
	text {}
	bold {SYNOPSIS}
	text {  queue-fix [-i | -N | -v] queue_dir}
	text {}
	text {  -i - Interactive Mode}
	text {  -N - Test Mode}
	text {  -v - Verbose Mode}
	text {}
	text {}
	bold {DESCRIPTION}
	text {      }
	text {  queue-fix fixes and repairs the corrupt qmail queue system}
	text {}
	text {        queue-fix must be run either as root or with group id}
	text {        qmail.}
	text {}
}


set Help(queuestat) {
	heading {HELP for "QUEUE STATUS"}
	text {}
	text {The "QUEUE STATUS" utility enables administrator to summarise information on mails in }
	text {the queue.}
	text {}
	text {}
	text {-------------------------------------------------------------------------------------}
	bold {Command qmail-qread}
	text {}
	text {}
	text {}
	bold {NAME}
	text {       qmail-qread - summarize status of mail queue}
	text {}
	bold {SYNOPSIS}
	text {       qmail-qread}
	text {}
	bold {DESCRIPTION}
	text {          qmail-qread gives a human-readable breakdown of the number}
	text {          of messages at various spots in the mail queue.}
	text {}
	text {          qmail-qread must be run either as root or  with  group  id}
	text {          qmails.}
	text {}
}

set Help(mysqlclient) {
	
		heading {HELP for "Mysql Client"}
	text {}
	text {The utility helps the administrator to connect to the mysql database}
	text { directly and run the SQL quieries }
	text { Select the domain and the server then click on the "run"}
	text { button to connect to the mysql database}
	text { For a detailed help on SQL queries, please refer the URL}
	text {http://www.mysql.com/documentation/mysql/bychapter/manual_Tutorial.html#Tuto}
	text {}
	bold {Domain}
	text {}
	text { 		Select the domain that you wish to administer from the}
	text { 		drop down list}
	text {}
	bold {Server}
	text {}
	text {       	Select the server IP on which the domain resides}
	text {}
}

set Help(addrbook) {
		heading {Help for "Address Book"}
	text {}
	text {}
	text {Address book option allows the administrator to }
	text {view, add, delete and modify fields of an  address book entry.}
	bold {Domain}
	text {}
	text {		Choose the domain from the drop down list}
	text {  	for which address book information must be listed.}
	text {}
	bold {Address  list}
	text {}
	text {		Provides address book information for the selected domain.}
	text {}
	bold {Add  Address}
	text {}
	text {          An address book entry can be added.}
	text {}
	bold {Modify  Address}
	text {}
	text {		Chosen Address book entry can be modified.}
	text {}
	bold {Delete  Address}
	text {}
	text {Chosen emailid can be deleted from the address  book.}
	text {}
}

set Help(userlist) {
		heading {Help for "User List"}
	text {}
	text {}
	text {This utility helps the administrator to check periodically}
	text {the number of active and inactive users in a selected domain}
	bold {Domain}
	text {}
	text {		Choose the domain from the drop down list}
	text {  	for which the active/inactive users list is to be generated }
	text {}
	bold {server}
	text {}
	text {		Select the server on which the domain exists}
	text {}
	bold {starting with}
	text {}
	text {          Select an alphabet from the drop down }
	text {}
	text {}
}

set Help(cmdline) {
		heading {Help for "Command Line Utility"}
	text {}
	text {}
	text { This utility helps the administrator to run all }
	text { mail commands by simply selecting the name of the comand from}
	text { the drop down list and supply the suitable arguments}
	text { click on "Run" button to execute it}
	text { If no arguments are given, the command help is}
	text { displayed for reference}
	text {}
	bold {Program Name}
	text {}
	text {		Choose the name of the program from the drop down list}
	text {}
	bold {Command Line}
	text {}
	text {		Tyepe the appropriate arguments for the selected program}
	text {}
	text {}
}
#box with a "Unknown" message
#
set Help(unknown) {
	text {      Unknown}
	text {IndiMail Administration Tool}
	text {Help not found for this page}
	text { Plz mail your queries to customercare@indicorp.com}
	text {}
}
