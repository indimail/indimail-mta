use RoundCube_db;
CREATE TABLE if not exists `userpref` (
  `username` varchar(100) NOT NULL DEFAULT '',
  `preference` varchar(50) NOT NULL DEFAULT '',
  `value` varchar(100) NOT NULL DEFAULT '',
  `prefid` int(11) NOT NULL auto_increment,
   PRIMARY KEY (prefid),
   KEY username (username)
);

INSERT INTO userpref (username, preference, value) VALUES ('@GLOBAL', 'required_hits', '4.0');

CREATE TABLE if not exists awl (
  Username varchar (100) NOT NULL default '',
  Email varchar (255) NOT NULL default '',
  Ip varchar (40) NOT NULL default '',
  Count int (11) NOT NULL default '0',
  Totscore float NOT NULL default '0',
  Signedby varchar (255) NOT NULL default '',
  PRIMARY KEY (username, email, signedby, ip));

CREATE TABLE if not exists bayes_expire (
  Id int (11) NOT NULL default '0',
  Runtime int (11) NOT NULL default '0',
  KEY bayes_expire_idx1 (id));

CREATE TABLE if not exists bayes_global_vars (
  Variable varchar (30) NOT NULL default '',
  Value varchar (200) NOT NULL default '',
  PRIMARY KEY (variable));

INSERT INTO bayes_global_vars VALUES ('VERSION', '3');

CREATE TABLE if not exists bayes_seen (
  Id int (11) NOT NULL default '0',
  Msgid varchar (200) binary NOT NULL default '',
  Flag char (1) NOT NULL default '',
  PRIMARY KEY (id, msgid));

CREATE TABLE if not exists bayes_token (
  Id int (11) NOT NULL default '0',
  Token char (5) NOT NULL default '',
  Spam_count int (11) NOT NULL default '0',
  Ham_count int (11) NOT NULL default '0',
  Atime int (11) NOT NULL default '0',
  PRIMARY KEY (id, token), INDEX bayes_token_idx1 (id, atime));

CREATE TABLE if not exists bayes_vars (
  Id int (11) NOT NULL AUTO_INCREMENT,
  Username varchar (200) NOT NULL default '',
  Spam_count int (11) NOT NULL default '0',
  Ham_count int (11) NOT NULL default '0',
  Token_count int (11) NOT NULL default '0',
  Last_expire int (11) NOT NULL default '0',
  Last_atime_delta int (11) NOT NULL default '0',
  Last_expire_reduce int (11) NOT NULL default '0',
  Oldest_token_age int (11) NOT NULL default '2147483647',
  Newest_token_age int (11) NOT NULL default '0',
  PRIMARY KEY (id),
  UNIQUE bayes_vars_idx1 (username));
