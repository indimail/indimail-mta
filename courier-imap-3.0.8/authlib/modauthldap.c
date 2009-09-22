/*
 * modauthldap.c - Define the authentification ldap module
 * 
 * courier-imap - 
 *
 * Copyright 1999 Luc Saillard <luc.saillard@alcove.fr>.
 *
 * This module use a server LDAP to authenticate user.
 * See the README.ldap
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to 
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#define	MODULE	auth_ldap
#define	MODNAME	"authldap"
#include	"mod.h"
