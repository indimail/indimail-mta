/*
 * $Id: auth.h,v 1.1 2010-04-26 12:07:27+05:30 Cprogrammer Exp mbhangui $
 */

void            auth_system(const char *ip_addr, struct passwd *pw);
void            auth_user_domain(const char *ip_addr, struct passwd *pw);
void            set_admin_type();
