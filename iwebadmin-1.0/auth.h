/*
 * $Id: auth.h,v 1.1.2.1 2004/11/20 01:10:41 tomcollins Exp $
 */

void auth_system (const char *ip_addr, struct passwd *pw);
void auth_user_domain (const char *ip_addr, struct passwd *pw);
void set_admin_type();

