/*-
 * $Log: tai64.c,v $
 * Revision 2.2  2009-02-25 11:26:42+05:30  Cprogrammer
 * changed storage to static
 *
 * Revision 2.1  2003-09-08 17:32:04+05:30  Cprogrammer
 * tai routines for tcl
 *
 */
#include <time.h>
#include <string.h>
#include <tcl.h>

int
tai64nlocal(Tcl_Interp *interp, int argc, char **argv)
{
	time_t          secs;
	char           *ptr, *query = NULL;
	int             ch;
	unsigned long   nanosecs, u;
	static char     str[56], errormsg[128];
	struct tm      *t;

	if (argc <= 0 || argv[0] == NULL)
	{
		Tcl_SetResult(interp, "Usage: tai unix|local|version timestamp; please try again.", TCL_STATIC);
		return TCL_ERROR;
	}
#if UTF_ENCODING
	Tcl_DStringInit(&ds);
	Tcl_UtfToExternalDString(NULL, argv[0], strlen(argv[0]), &ds);
	query = Tcl_DStringValue(&ds);
#else
	query = argv[0];
#endif
	ch = *query;
	if (ch != '@')
	{
#if UTF_ENCODING
		Tcl_DStringFree(&ds);
#endif
		/*- Tcl_SetResult(interp, "", TCL_STATIC); -*/
		snprintf(errormsg, sizeof(errormsg), "Invalid string [%s]", query),
		Tcl_SetResult(interp, errormsg, TCL_STATIC);
		return TCL_ERROR;
	}
	secs = 0;
	nanosecs = 0;
	for (ptr = query + 1;*ptr;ptr++)
	{
		ch = *ptr;
		u = ch - '0';
		if (u >= 10)
		{
			u = ch - 'a';
			if (u >= 6)
				break;
			u += 10;
		}
		secs <<= 4;
		secs += nanosecs >> 28;
		nanosecs &= 0xfffffff;
		nanosecs <<= 4;
		nanosecs += u;
	}
	secs -= 4611686018427387914ULL;
	t = localtime(&secs);
	snprintf(str, sizeof(str), "%d-%02d-%02d %02d:%02d:%02d.%09ld", 
			t->tm_year + 1900, 1 + t->tm_mon, t->tm_mday, 
			t->tm_hour, t->tm_min, t->tm_sec, nanosecs);
	Tcl_SetResult(interp, str, TCL_STATIC);
#if UTF_ENCODING
	Tcl_DStringFree(&ds);
#endif
	return TCL_OK;
}

int
tai64nunix(Tcl_Interp *interp, int argc, char **argv)
{
	time_t          secs;
	char           *ptr, *query = NULL;
	int             ch;
	unsigned long   nanosecs, u;
	static char     str[56], errormsg[128];

	if (argc <= 0 || argv[0] == NULL)
	{
		Tcl_SetResult(interp, "Usage: tai unix|local|version timestamp; please try again.", TCL_STATIC);
		return TCL_ERROR;
	}
#if UTF_ENCODING
	Tcl_DStringInit(&ds);
	Tcl_UtfToExternalDString(NULL, argv[0], strlen(argv[0]), &ds);
	query = Tcl_DStringValue(&ds);
#else
	query = argv[0];
#endif
	ch = *query;
	if (ch != '@')
	{
#if UTF_ENCODING
		Tcl_DStringFree(&ds);
#endif
		/*- Tcl_SetResult(interp, "", TCL_STATIC); -*/
		snprintf(errormsg, sizeof(errormsg), "Invalid string [%s]", query),
		Tcl_SetResult(interp, errormsg, TCL_STATIC);
		return TCL_ERROR;
	}
	secs = 0;
	nanosecs = 0;
	for (ptr = query + 1;*ptr;ptr++)
	{
		ch = *ptr;
		u = ch - '0';
		if (u >= 10)
		{
			u = ch - 'a';
			if (u >= 6)
				break;
			u += 10;
		}
		secs <<= 4;
		secs += nanosecs >> 28;
		nanosecs &= 0xfffffff;
		nanosecs <<= 4;
		nanosecs += u;
	}
	secs -= 4611686018427387914ULL;
	snprintf(str, sizeof(str), "%ld.%09ld", secs, nanosecs);
	Tcl_SetResult(interp, str, TCL_STATIC);
#if UTF_ENCODING
	Tcl_DStringFree(&ds);
#endif
	return TCL_OK;
}

int
Tai64Cmd(ClientData data, Tcl_Interp *interp, int argc, char **argv)
{
	/*- any command specified */
	if (argc <= 1)
	{
		Tcl_SetResult(interp, "Usage: tai unix|local|version; please try again.", TCL_STATIC);
		return TCL_ERROR;
	}
	/*- check command list */
	if (strcmp(argv[1], "local") == 0)
		return tai64nlocal(interp, argc - 2, argv + 2);
	else
	if (strcmp(argv[1], "unix") == 0)
		return tai64nunix(interp, argc - 2, argv + 2);
	else
	if (strcmp(argv[1], "version") == 0)
	{
		Tcl_SetResult(interp, "TAI64 Interface for Tcl; version 1.0", TCL_STATIC);
		return TCL_OK;
	} 
	Tcl_SetResult(interp, "Usage: tai unix|local|version timestamp; please try again.", TCL_STATIC);
	return TCL_ERROR;
}

/*- define the TCL initialisation code to create the "tai64" command */
/*- haven't tested this yet (for building on Unix) */
#ifdef WINDOWS
extern
__declspec(dllexport)
int             Tai_Init(Tcl_Interp *interp);
#endif
int
Tai_Init(Tcl_Interp *interp)
{
	/*- create the main "tai64" command */
	Tcl_CreateCommand(interp, "tai64", (Tcl_CmdProc *) Tai64Cmd, (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);
	return (Tcl_PkgProvide(interp, "tai64", "1.0") == TCL_ERROR ? TCL_ERROR : TCL_OK);
}
