<?php

/* Path to your iwebadmin installation */
$rcmail_config['iwebadmin_path'] = 'https://indimail.org/cgi-bin/iwebadmin/';

/* Allow users to configure their vacation message? true or false */
$rcmail_config['iwebadmin_allow_vacation'] = true;

/* Allow users to change their password? true or false */
$rcmail_config['iwebadmin_allow_password'] = true;

/* Min password lenght (defaults to 6)*/
/* Minimum password length for password changes. (Integer - defaults to 6) */
$rcmail_config['iwebadmin_password_min_length'] = 6;

/* Maximum password length for password changes. (Integer - defaults to 15) */
/* -- be carefull: vpopmail defaults to max 16! */
$rcmail_config['iwebadmin_password_max_length'] = 15;

/* Need to have lowercase letters ? */
$rcmail_config['iwebadmin_password_lower_need'] = false;

/* Need to have uppercase letters ? */
$rcmail_config['iwebadmin_password_upper_need'] = false;

/* Need to have numbers ? */
$rcmail_config['iwebadmin_password_number_need'] = true;

/* Need to have special chars ? */
$rcmail_config['iwebadmin_password_special_need'] = true;

/* special chars allowed (defaults to '@!#-_.') */
/* this string is limited to 20 chars for performance */
/* Be carefull: some special chars can cause erros (like $ & %) test them in your system before use */
$rcmail_config['iwebadmin_password_special_chars'] = '@!#-_.';

/* Show a warning message on each page when vacation is on? true or false */
$rcmail_config['iwebadmin_vacation_warning'] = true;

/* Charset used in iwebadmin ; defaults to 'ISO-8859-1' */
$rcmail_config['iwebadmin_charset'] = 'ISO-8859-1';

