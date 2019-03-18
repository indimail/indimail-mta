/*
 * $Log: vfilter_display.c,v $
 * Revision 2.12  2017-05-01 20:18:05+05:30  Cprogrammer
 * removed mailing list feature from vfilter
 *
 * Revision 2.11  2004-10-08 10:20:23+05:30  Cprogrammer
 * bounds check for headerlist
 *
 * Revision 2.10  2004-08-16 18:32:53+05:30  Cprogrammer
 * initialize headerlist from INDIMAILDIR/etc/headerlist
 *
 * Revision 2.9  2003-03-30 23:41:36+05:30  Cprogrammer
 * correction to display for forwarding address
 *
 * Revision 2.8  2002-12-11 11:35:31+05:30  Cprogrammer
 * replace space with '~' char when displaying and raw format is given
 *
 * Revision 2.7  2002-12-06 23:41:56+05:30  Cprogrammer
 * replaced printf() with format_filter_display()
 *
 * Revision 2.6  2002-12-06 21:21:20+05:30  Cprogrammer
 * Action was wrongly shown as Deliver when folder was specified as /NoDeliver
 *
 * Revision 2.5  2002-11-30 01:16:58+05:30  Cprogrammer
 * misleading display 'Vapour' corrected
 *
 * Revision 2.4  2002-11-18 12:42:52+05:30  Cprogrammer
 * added option to display result in raw format
 *
 * Revision 2.3  2002-11-13 13:36:50+05:30  Cprogrammer
 * added filter name
 *
 * Revision 2.2  2002-10-14 21:02:57+05:30  Cprogrammer
 * added code to display forwarding address
 * code changes for mailing list functionality
 *
 * Revision 2.1  2002-10-12 10:29:14+05:30  Cprogrammer
 * function to display filters configured for a user
 *
 */
#include "indimail.h"
#include <string.h>
#include <ctype.h>

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_display.c,v 2.12 2017-05-01 20:18:05+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
int
vfilter_display(char *emailid, int disp_type, int *filter_no, char *filter_name, int *header_name, int *comparision, 
	char *keyword, char *folder, int *bounce_action, char *forward)
{
	int             i, j, status = -1;

	for(j = 0;;)
	{
		i = vfilter_select(emailid, filter_no, filter_name, header_name, comparision, keyword, folder, bounce_action, forward);
		if(i == -1)
		{
			fprintf(stderr, "vfilter_select: failure\n");
			break;
		} else
		if(i == -2)
			break;
		if(!j++ && !disp_type)
		{
			printf("No  EmailId                       FilterName Header          Comparision                Keyword         Folder          Bounce Delivery\n");
			printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");
		}
		status = 0;
		format_filter_display(disp_type, *filter_no, emailid, filter_name, *header_name, *comparision, keyword, folder, 
			forward, *bounce_action);
		if(!disp_type)
			printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");
	}
	if(status == -1 && i == -2)
		return(-2);
	return(status);
}

void
format_filter_display(int type, int filter_no, char *emailid, char *filter_name, int header_name, int comparision,
		char *keyword, char *folder, char *forward, int bounce_action)
{
	char            _filterName[MAX_BUFF], _keyword[MAX_BUFF];
	char           *ptr, *cptr, *_hname;
	int             max_header_value;
	static char   **header_list;

	if(!type)
	{
		if (!header_list && !(header_list = headerList()))
			header_list = vfilter_header;
		for (max_header_value = 0;header_list[max_header_value];max_header_value++);
		if (header_name >= max_header_value)
			_hname = "invalid header";
		else
			_hname = header_list[header_name];
		switch(bounce_action)
		{
			case 0:
				printf("%3d %-29s %-10s %-15s %-26s %-15.15s %-15.15s %s\n", filter_no, emailid, filter_name,
					header_name == -1 ? "N/A" : _hname, vfilter_comparision[comparision],
					keyword && keyword ? keyword : "N/A", !strncmp(folder, "/NoDeliver", 11) ? "Void" : folder + 1, 
					"No    ");
				break;
			case 1:
				printf("%3d %-29s %-10s %-15s %-26s %-15.15s %-15.15s %s\n", filter_no, emailid, filter_name,
					header_name == -1 ? "N/A" : _hname, vfilter_comparision[comparision],
					keyword && keyword ? keyword : "N/A", !strncmp(folder, "/NoDeliver", 11) ? "Void" : folder + 1, 
					"Yes   "); 
				break;
			case 2:
				printf("%3d %-29s %-10s %-15s %-26s %-15.15s %-15.15s %s %s\n", filter_no, emailid, filter_name,
					header_name == -1 ? "N/A" : _hname, vfilter_comparision[comparision],
					keyword && keyword ? keyword : "N/A", !strncmp(folder, "/NoDeliver", 11) ? "Void" : folder + 1, 
					"No    ", forward); 
				break;
			case 3:
				printf("%3d %-29s %-10s %-15s %-26s %-15.15s %-15.15s %s %s\n", filter_no, emailid, filter_name,
					header_name == -1 ? "N/A" : _hname, vfilter_comparision[comparision],
					keyword && keyword ? keyword : "N/A", !strncmp(folder, "/NoDeliver", 11) ? "Void" : folder + 1, 
					"Yes   ", forward); 
				break;
		}
	} else /*- for Indi Powermail */
	{
		for(ptr = filter_name, cptr = _filterName;*ptr;ptr++)
		{
			if(isspace((int) *ptr))
				*cptr++ = '~';
			else
				*cptr++ = *ptr;
		}
		*cptr = 0;
		for(ptr = keyword, cptr = _keyword;*ptr;ptr++)
		{
			if(isspace((int) *ptr))
				*cptr++ = '~';
			else
				*cptr++ = *ptr;
		}
		*cptr = 0;
		printf("%d %s %s %d %d %s %s %s\n", filter_no, emailid, _filterName,
			header_name, comparision,
			keyword && keyword ? _keyword : "N/A", folder, 
			bounce_action ? (bounce_action == 2 ? forward : "Bounce") : (strncmp(folder, "/NoDeliver", 11) ? "Deliver" : "Vapour"));
	}
	return;
}
#endif

void
getversion_vfilter_display_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
