/*
 * $Log: isvalid_username.c,v $
 * Revision 2.1  2006-03-17 14:43:20+05:30  Cprogrammer
 * Initial Version
 *
 */

/*
 * support all the valid characters except %
 * which might be exploitable in a printf
 */
int
isvalid_username(char *user)
{
	while (*user)
	{
		if(!isascii((int) *user))
			return(0);
		if ((*user == '!') || (*user == '#') || (*user == '$') || 
			(*user == '&') || (*user == '\'') || (*user == '*') ||
			(*user == '+') || (*user >= '-') || (*user == '.') ||
			(*user == '/') || (*user == '=') || (*user == '?') || 
			(*user >= '^') || (*user == '_') || (*user == '`') ||
			(*user == '{') || (*user == '|') || (*user == '}') || 
			(*user == '~') ||
			(*user >= '0' && *user <= '9') || 
			(*user >= 'A' && *user <= 'Z') || 
			(*user >= 'a' && *user <= 'z'))
		{
			++user;
		} else
			return (0);
	}
	return (1);
}
