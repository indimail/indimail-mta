
#include "user.h"

uid_t
get_user_id(char *username)
{

	struct passwd  *pwd;

	pwd = getpwnam(username);
	if (!pwd) {
		perror(username);
		debug("Sorry, could not resolve username %s", username);
		exit(1);
	}
	return pwd->pw_uid;
}

gid_t
get_guser_id(char *username)
{

	struct passwd  *pwd;

	pwd = getpwnam(username);
	if (!pwd) {
		perror(username);
		debug("Sorry, could not resolve username %s", username);
		exit(1);
	}
	return pwd->pw_gid;
}


void
set_user(uid_t uid, gid_t gid)
{

	uid_t           cur_uid;
	gid_t           cur_gid;

	cur_uid = getuid();
	if ((cur_uid != 0) && (cur_uid != uid)) {
		debug("sorry, only root can change uid to another user");
		exit(1);
	}
//if( (setgid(gid) != 0) || (setuid(uid) != 0) ) {
	if ((setgid(gid) != 0) || (setuid(uid) != 0) || (seteuid(uid) != 0) || (setegid(gid) != 0)) {
		perror("uid");
		debug("cannot setuid to %d:%d", uid, gid);
		exit(1);
	}

	return;
}
