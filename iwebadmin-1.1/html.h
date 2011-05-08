/*
 * $Id: html.h,v 1.1 2010-04-26 12:07:51+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 1999-2006 Inter7 Internet Technologies, Inc.
 * Copyright (C) 2006 Tom Logic LLC
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

#define HTML_MENU_START			"[&nbsp;"
#define HTML_MENU_SEP			"&nbsp;|&nbsp;"
#define HTML_MENU_END			"&nbsp;]"

#define HTML_EMPTY_TD			"<td> </td>"

#define HTML_BTN_GENERIC		"<img border=\"0\" src=\"" IMAGEURL "/delete.png\">"
#define HTML_BTN_DELETE			"<img border=\"0\" src=\"" IMAGEURL "/trash.png\">"

#define HTML_ALIAS_ERROR		"<tr><td colspan=\"4\">%s %s</td></tr>\n"

#define HTML_ALIAS_ROW_START	"<tr>"
#define HTML_ALIAS_ROW_END		"</tr>\n"

#define HTML_ALIAS_NAME			"<td align=left>%H</td>"

// start of the list of destination addresses
#define HTML_ALIAS_DEST_START	"<td align=left>"
#define HTML_ALIAS_DEST_END	"</td>"

// Possible destinations in list
// BLACKHOLE uses text string 303 (All mail deleted)
#define HTML_ALIAS_BLACKHOLE	"<i>%s</i>"
// LOCAL is for local addresses (no domain shown)
#define HTML_ALIAS_LOCAL		"%H"
// INVALID is for local addresses that no longer exist (error condition)
#define HTML_ALIAS_INVALID		"<font color=\"red\"><b>%H</b></font>"
// REMOTE is for remote addresses (domain is shown)
#define HTML_ALIAS_REMOTE		"%H"

#define HTML_ALIAS_SHOWPAGE	"<a href=\"%s&page=%d\">%s</a>"
#define HTML_ALIAS_DOSEARCH	"<a href=\"%s&searchuser=%C\">%s</a>"

#define HTML_ALIAS_FOOTER_START	"<tr><td align=\"right\" colspan=\"4\">"
#define HTML_ALIAS_FOOTER_END		"</td></tr>"

// HTML from the "modify alias" page
#define HTML_ALIAS_MOD_ROW_START	"<tr>"
#define HTML_ALIAS_MOD_ROW_END	"</tr>"
#define HTML_ALIAS_MOD_NAME		"<td align=\"center\" valign=\"top\"><b>%H</b></td>"
#define HTML_ALIAS_MOD_DEST		"<td align=\"center\" valign=\"top\">%s</td>\n"
#define HTML_ALIAS_MOD_DELETE		"<td align=\"center\" valign=\"top\">" \
	"<a href=\"%s&modu=%C&linedata=%C&action=delentry\">" HTML_BTN_DELETE "</a></td>\n"
