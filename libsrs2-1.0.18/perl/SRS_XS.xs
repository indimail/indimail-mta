#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#undef VERSION	/* From XS command line */
#include "../libsrs2/srs2.h"

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

typedef srs_t *Mail__SRS_XS;

#define SRS_SET(s, x, v) do { \
		ret = srs_set_ ## x(s, v); \
		if (ret != SRS_SUCCESS) { \
			srs_free(srs); \
			croak("Error from SRS library: %s", srs_strerror(ret)); \
		} \
	} while(0)

#define EXPORT_INTEGER(x) do { \
				newCONSTSUB(stash, #x, newSViv(x)); \
				av_push(export, newSVpv(#x, strlen(#x))); \
			} while(0)


MODULE = Mail::SRS_XS		PACKAGE = Mail::SRS_XS

PROTOTYPES: ENABLE

BOOT:
{
	HV	*stash;
	AV	*export;

	stash = gv_stashpv("Mail::SRS_XS", TRUE);
	export = get_av("Mail::SRS_XS::EXPORT_OK", TRUE);

	EXPORT_INTEGER(SRS_ERRTYPE_NONE);
	EXPORT_INTEGER(SRS_ERRTYPE_CONFIG);
	EXPORT_INTEGER(SRS_ERRTYPE_INPUT);
	EXPORT_INTEGER(SRS_ERRTYPE_SYNTAX);
	EXPORT_INTEGER(SRS_ERRTYPE_SRS);

	EXPORT_INTEGER(SRS_SUCCESS);
	EXPORT_INTEGER(SRS_ENOTSRSADDRESS);

	EXPORT_INTEGER(SRS_ENOSECRETS);
	EXPORT_INTEGER(SRS_ESEPARATORINVALID);

	EXPORT_INTEGER(SRS_ENOSENDERATSIGN);
	EXPORT_INTEGER(SRS_EBUFTOOSMALL);

	EXPORT_INTEGER(SRS_ENOSRS0HOST);
	EXPORT_INTEGER(SRS_ENOSRS0USER);
	EXPORT_INTEGER(SRS_ENOSRS0HASH);
	EXPORT_INTEGER(SRS_ENOSRS0STAMP);
	EXPORT_INTEGER(SRS_ENOSRS1HOST);
	EXPORT_INTEGER(SRS_ENOSRS1USER);
	EXPORT_INTEGER(SRS_ENOSRS1HASH);
	EXPORT_INTEGER(SRS_EBADTIMESTAMPCHAR);
	EXPORT_INTEGER(SRS_EHASHTOOSHORT);

	EXPORT_INTEGER(SRS_ETIMESTAMPOUTOFDATE);
	EXPORT_INTEGER(SRS_EHASHINVALID);
}

Mail::SRS_XS
new(class, args)
	SV*	class
	HV*	args
	PREINIT:
		SV		**svp;
		srs_t	 *srs;
		int		  ret;
	CODE:
		svp = hv_fetch(args, "Secret", 6, FALSE);
		if (!SvPOK(*svp))
			croak("Usage: new( { Secret => 'secret', ... } )");
		srs = srs_new();
		srs_add_secret(srs, SvPV_nolen(*svp));

		svp = hv_fetch(args, "Separator", 9, FALSE);
		if (svp && SvPOK(*svp))
			SRS_SET(srs, separator, (SvPV_nolen(*svp))[0]);

		svp = hv_fetch(args, "MaxAge", 6, FALSE);
		if (svp && SvIOK(*svp))
			SRS_SET(srs, maxage, (SvIV(*svp)));

		svp = hv_fetch(args, "HashLength", 9, FALSE);
		if (svp && SvIOK(*svp))
			SRS_SET(srs, hashlength, (SvIV(*svp)));

		svp = hv_fetch(args, "HashMin", 7, FALSE);
		if (svp && SvIOK(*svp))
			SRS_SET(srs, hashmin, (SvIV(*svp)));

		RETVAL = srs;
	OUTPUT:
		RETVAL

void
DESTROY(srs)
	Mail::SRS_XS	srs
	CODE:
		srs_free(srs);

const char *
forward(srs, sender, alias)
	Mail::SRS_XS	srs
	const char *	sender
	const char *	alias
	PREINIT:
		char	buf[BUFSIZ];
		int		ret;
	CODE:
		ret = srs_forward(srs, buf, BUFSIZ, sender, alias);
		if (ret != SRS_SUCCESS)
			croak(srs_strerror(ret));
		else
			RETVAL = buf;
	OUTPUT:
		RETVAL

const char *
reverse(srs, sender)
	Mail::SRS_XS	srs
	const char *	sender
	PREINIT:
		char	buf[BUFSIZ];
		int		ret;
	CODE:
		ret = srs_reverse(srs, buf, BUFSIZ, sender);
		if (ret != SRS_SUCCESS)
			croak(srs_strerror(ret));
		else
			RETVAL = buf;
	OUTPUT:
		RETVAL

const char *
get_secret(srs, idx)
	Mail::SRS_XS	srs
	int				idx
	CODE:
		if (idx >= srs->numsecrets)
			croak("No such secret %d", idx);
		RETVAL = srs_get_secret(srs, idx);
	OUTPUT:
		RETVAL

const char *
timestamp_create(srs, now)
	Mail::SRS_XS	srs
	long			now
	PREINIT:
		char	buf[64];
	CODE:
		srs_timestamp_create(srs, buf, (time_t)now);
		RETVAL = buf;
	OUTPUT:
		RETVAL

SV *
timestamp_check(srs, stamp)
	Mail::SRS_XS	srs
	const char *	stamp
	PREINIT:
		int			ret;
	CODE:
		ret = srs_timestamp_check(srs, stamp);
		if (ret == SRS_SUCCESS)
			RETVAL = &PL_sv_yes;
		else
			RETVAL = &PL_sv_no;
	OUTPUT:
		RETVAL

const char *
strerror(srs, code)
	Mail::SRS_XS	srs
	int				code
	CODE:
		RETVAL = srs_strerror(code);
	OUTPUT:
		RETVAL
