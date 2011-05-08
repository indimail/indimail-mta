/*
 * $Id: printh.h,v 1.1 2010-04-26 12:07:58+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 2004 Tom Logic LLC 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdlib.h>
#include <stdarg.h>

int             vsnprinth(char *buffer, size_t size, const char *format, va_list ap);
int             snprinth(char *buffer, size_t size, const char *format, ...);
int             printh(const char *format, ...);
