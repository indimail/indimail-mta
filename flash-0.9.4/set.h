/*
 * $Log: set.h,v $
 * Revision 1.2  2008-06-09 15:32:59+05:30  Cprogrammer
 * changed name to flash
 *
 * Revision 1.1  2002-12-16 01:55:44+05:30  Manny
 * Initial revision
 *
 * 
 * Set variables that are use in the rest of the code
 * 
 * - mailnotify [x]      - notifys user of new mail every x seconds
 * [-1 = not notify] or on a screen change
 * 
 * - noclobber           - left arrow will not exit flash on main menu    
 * 
 * - background x        - load file x into flash's background. x can be a ':' 
 * seperated list of files to overlay as background.
 * 
 * - logging             - turn on exec logging
 * 
 * - execpager           - pager to use when P flag specified in menu item
 * 
 * - lockscreensaver [t] - turn on the (inappropriatly titled) lock screensaver
 * with update time t
 * 
 * - lockbackdoor [x]    - Enable (encrypted) backdoor password [x] into the 
 * lock screen.         
 * 
 * - barclock            - Enable clock in top bar
 * 
 * - notimeout           - Disable timeouts
 * 
 */

#define SF_SET     0x0001
#define SF_NORESET 0x0002

struct set_node
{
	char           *variable;
	char           *value;
	int             flags;
};

int             find_variable(char *, char **);
void            set_variable(char *, char *, int);
void            unset_variable(char *);
void            global_variables(void);
