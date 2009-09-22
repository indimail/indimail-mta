/*
** Copyright 2000-2002 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: big5.c,v 1.14 2004/05/23 14:28:24 mrsam Exp $
*/

#include "big5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIG5_HKSCS_EXTENSION	1

static const unicode_char * const big5fwdlo[]= {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	big5_88_lo,
	big5_89_lo,
	big5_8a_lo,
	big5_8b_lo,
	big5_8c_lo,
	big5_8d_lo,
	big5_8e_lo,
	big5_8f_lo,
	big5_90_lo,
	big5_91_lo,
	big5_92_lo,
	big5_93_lo,
	big5_94_lo,
	big5_95_lo,
	big5_96_lo,
	big5_97_lo,
	big5_98_lo,
	big5_99_lo,
	big5_9a_lo,
	big5_9b_lo,
	big5_9c_lo,
	big5_9d_lo,
	big5_9e_lo,
	big5_9f_lo,
	big5_a0_lo,
	big5_a1_lo,
	big5_a2_lo,
	big5_a3_lo,
	big5_a4_lo,
	big5_a5_lo,
	big5_a6_lo,
	big5_a7_lo,
	big5_a8_lo,
	big5_a9_lo,
	big5_aa_lo,
	big5_ab_lo,
	big5_ac_lo,
	big5_ad_lo,
	big5_ae_lo,
	big5_af_lo,
	big5_b0_lo,
	big5_b1_lo,
	big5_b2_lo,
	big5_b3_lo,
	big5_b4_lo,
	big5_b5_lo,
	big5_b6_lo,
	big5_b7_lo,
	big5_b8_lo,
	big5_b9_lo,
	big5_ba_lo,
	big5_bb_lo,
	big5_bc_lo,
	big5_bd_lo,
	big5_be_lo,
	big5_bf_lo,
	big5_c0_lo,
	big5_c1_lo,
	big5_c2_lo,
	big5_c3_lo,
	big5_c4_lo,
	big5_c5_lo,
	big5_c6_lo,
	big5_c7_lo,
	big5_c8_lo,
	big5_c9_lo,
	big5_ca_lo,
	big5_cb_lo,
	big5_cc_lo,
	big5_cd_lo,
	big5_ce_lo,
	big5_cf_lo,
	big5_d0_lo,
	big5_d1_lo,
	big5_d2_lo,
	big5_d3_lo,
	big5_d4_lo,
	big5_d5_lo,
	big5_d6_lo,
	big5_d7_lo,
	big5_d8_lo,
	big5_d9_lo,
	big5_da_lo,
	big5_db_lo,
	big5_dc_lo,
	big5_dd_lo,
	big5_de_lo,
	big5_df_lo,
	big5_e0_lo,
	big5_e1_lo,
	big5_e2_lo,
	big5_e3_lo,
	big5_e4_lo,
	big5_e5_lo,
	big5_e6_lo,
	big5_e7_lo,
	big5_e8_lo,
	big5_e9_lo,
	big5_ea_lo,
	big5_eb_lo,
	big5_ec_lo,
	big5_ed_lo,
	big5_ee_lo,
	big5_ef_lo,
	big5_f0_lo,
	big5_f1_lo,
	big5_f2_lo,
	big5_f3_lo,
	big5_f4_lo,
	big5_f5_lo,
	big5_f6_lo,
	big5_f7_lo,
	big5_f8_lo,
	big5_f9_lo,
	big5_fa_lo,
	big5_fb_lo,
	big5_fc_lo,
	big5_fd_lo,
	big5_fe_lo};

static const unicode_char * const big5fwdhi[]= {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	big5_88_hi,
	big5_89_hi,
	big5_8a_hi,
	big5_8b_hi,
	big5_8c_hi,
	big5_8d_hi,
	big5_8e_hi,
	big5_8f_hi,
	big5_90_hi,
	big5_91_hi,
	big5_92_hi,
	big5_93_hi,
	big5_94_hi,
	big5_95_hi,
	big5_96_hi,
	big5_97_hi,
	big5_98_hi,
	big5_99_hi,
	big5_9a_hi,
	big5_9b_hi,
	big5_9c_hi,
	big5_9d_hi,
	big5_9e_hi,
	big5_9f_hi,
	big5_a0_hi,
	big5_a1_hi,
	big5_a2_hi,
	big5_a3_hi,
	big5_a4_hi,
	big5_a5_hi,
	big5_a6_hi,
	big5_a7_hi,
	big5_a8_hi,
	big5_a9_hi,
	big5_aa_hi,
	big5_ab_hi,
	big5_ac_hi,
	big5_ad_hi,
	big5_ae_hi,
	big5_af_hi,
	big5_b0_hi,
	big5_b1_hi,
	big5_b2_hi,
	big5_b3_hi,
	big5_b4_hi,
	big5_b5_hi,
	big5_b6_hi,
	big5_b7_hi,
	big5_b8_hi,
	big5_b9_hi,
	big5_ba_hi,
	big5_bb_hi,
	big5_bc_hi,
	big5_bd_hi,
	big5_be_hi,
	big5_bf_hi,
	big5_c0_hi,
	big5_c1_hi,
	big5_c2_hi,
	big5_c3_hi,
	big5_c4_hi,
	big5_c5_hi,
	big5_c6_hi,
	big5_c7_hi,
	big5_c8_hi,
	big5_c9_hi,
	big5_ca_hi,
	big5_cb_hi,
	big5_cc_hi,
	big5_cd_hi,
	big5_ce_hi,
	big5_cf_hi,
	big5_d0_hi,
	big5_d1_hi,
	big5_d2_hi,
	big5_d3_hi,
	big5_d4_hi,
	big5_d5_hi,
	big5_d6_hi,
	big5_d7_hi,
	big5_d8_hi,
	big5_d9_hi,
	big5_da_hi,
	big5_db_hi,
	big5_dc_hi,
	big5_dd_hi,
	big5_de_hi,
	big5_df_hi,
	big5_e0_hi,
	big5_e1_hi,
	big5_e2_hi,
	big5_e3_hi,
	big5_e4_hi,
	big5_e5_hi,
	big5_e6_hi,
	big5_e7_hi,
	big5_e8_hi,
	big5_e9_hi,
	big5_ea_hi,
	big5_eb_hi,
	big5_ec_hi,
	big5_ed_hi,
	big5_ee_hi,
	big5_ef_hi,
	big5_f0_hi,
	big5_f1_hi,
	big5_f2_hi,
	big5_f3_hi,
	big5_f4_hi,
	big5_f5_hi,
	big5_f6_hi,
	big5_f7_hi,
	big5_f8_hi,
	big5_f9_hi,
	big5_fa_hi,
	big5_fb_hi,
	big5_fc_hi,
	big5_fd_hi,
	big5_fe_hi};

static unicode_char *c2u_doconv(const struct unicode_info *u,
			 const char *cp, int *err, int compat)
{
	size_t i, cnt;
	unicode_char *uc;

	if (err)
		*err= -1;

	/*
	** Count the number of potential unicode characters first.
	*/

	for (i=cnt=0; cp[i]; i++)
	{
		if ((int)(unsigned char)cp[i] < 0x88 ||
		    (int)(unsigned char)cp[i] > 0xFE ||
		    cp[i+1] == 0)
		{
			++cnt;
			continue;
		}

		++i;
		++cnt;
	}

	uc=malloc((cnt+1)*sizeof(unicode_char));
	if (!uc)
		return (NULL);

	i=cnt=0;
	while (cp[i])
	{
		unsigned int a=(int)(unsigned char)cp[i], b;

		/* 2-byte Character */
		if ((unsigned)0x88 <= a && a <= (unsigned)0xFE && cp[i+1])
		{
			unicode_char ucv;
			b=(int)(unsigned char)cp[i+1];

			/* ranges extended by HKSCS */
			if (!(compat & BIG5_HKSCS_EXTENSION)
			    && (a < (unsigned)0xA1
			    || (a == (unsigned)0xC6
			    && (unsigned)0xBF <= b && b <= (unsigned)0xD7)))
				ucv = (unicode_char)0xFFFD;
			/* 0xXX40-0xXX7E */
			else if (0x40 <= b && b <= 0x7E
			    && big5fwdlo[a-0x81]
			    && (ucv=big5fwdlo[a-0x81][b-0x40]))
				;
			/* 0xXXA1-0xXXFE */
			else if ((unsigned)0xA1 <= b && b <= (unsigned)0xFE
			    && big5fwdhi[a-0x81]
			    && (ucv=big5fwdhi[a-0x81][b-0xA1]))
				;
			/* Not found */
			else
				ucv = (unicode_char)0xFFFD;

			/* mapped to PUA by HKSCS extension */
			if (!(compat & BIG5_HKSCS_EXTENSION)
			    && (unicode_char)0xE000 <= ucv
			    && ucv <= (unicode_char)0xF8FF)
				ucv = (unicode_char)0xFFFD;

			if (ucv == (unicode_char)0xFFFD && err)
			{
				*err = i;
				free(uc);
				return NULL;
			}
			uc[cnt++] = ucv;
			i += 2;
		}
		/* US-ASCII */
		else if (a < (unsigned)0x80)
		{
			uc[cnt++]=a;
			i += 1;
		}
		/* Not Found */
		else if (err)
		{
			*err=i;
			free(uc);
			return (NULL);
		}
		else
		{
			uc[cnt++] = (unicode_char)0xFFFD;
			i += 1;
		}
	}
	uc[cnt]=0;

	return (uc);
}

static unicode_char *c2u_eten(const struct unicode_info *u,
				const char *cp, int *err)
{
	return c2u_doconv(u, cp, err, 0);
}

static unicode_char *c2u_hkscs(const struct unicode_info *u,
				const char *cp, int *err)
{
	return c2u_doconv(u, cp, err, BIG5_HKSCS_EXTENSION);
}

static unsigned revlookup(unicode_char c)
{
	unsigned j;
	unsigned bucket;
	unsigned uc;

	bucket=c % big5_revhash_size;
	uc=0;

	for (j=big5_revtable_index[ bucket ];
	     j < sizeof(big5_revtable_uc)/sizeof(big5_revtable_uc[0]);
	     ++j)
	{
		unicode_char uuc=big5_revtable_uc[j];

		if (uuc == c)
			return (big5_revtable_octets[j]);

		if ((uuc % big5_revhash_size) != bucket)
			break;
	}
	return (0);
}

static char *u2c_doconv(const struct unicode_info *u,
		 const unicode_char *cp, int *err, int compat)
{
	size_t cnt, i;
	char *s;

	if (err)
		*err= -1;
	/*
	** Figure out the size of the octet string.  Unicodes < 0x7f will
	** map to a single byte, unicodes >= 0x80 will map to two bytes.
	*/

	for (i=cnt=0; cp[i]; i++)
	{
		if (cp[i] > 0x7f)
			++cnt;
		++cnt;
	}

	s=malloc(cnt+1);
	if (!s)
		return (NULL);
	cnt=0;

	for (i=0; cp[i]; i++)
	{
		unsigned uc;

		/* US-ASCII */
		if (cp[i] < (unicode_char)0x0080)
		{
			s[cnt++]= (char)cp[i];
			continue;
		}
		/* PUA by HKSCS */
		if (!(compat & BIG5_HKSCS_EXTENSION)
		    && (unicode_char)0xE000 <= cp[i]
		    && cp[i] <= (unicode_char)0xF8FF)
		{
			if (err)
			{
				*err=i;
				free(s);
				return (NULL);
			}
			s[cnt++] = '?';
			continue;
		}

		uc=revlookup(cp[i]);

		if (!uc
		    || (!(compat & BIG5_HKSCS_EXTENSION)
		    && (uc < (unsigned)0xA140
		    || ((unsigned)0xC6BF <= uc && uc <= (unsigned)0xC6D7))))
		{
			if (err)
			{
				*err=i;
				free(s);
				return (NULL);
			}
			s[cnt++] = '?';
		}
		else
		{
			s[cnt++]= (char)(uc >> 8);
			s[cnt++]= (char)(uc & 0x00FF);
		}
	}
	s[cnt]=0;
	return (s);
}

static char *u2c_eten(const struct unicode_info *u,
			const unicode_char *cp, int *err)
{
	return u2c_doconv(u, cp, err, 0);
}

static char *u2c_hkscs(const struct unicode_info *u,
			const unicode_char *cp, int *err)
{
	return u2c_doconv(u, cp, err, BIG5_HKSCS_EXTENSION);
}

static char *toupper_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=(*u->c2u)(u, cp, ip);
	char *s;

	unsigned i;

	if (!uc)
		return (NULL);

	for (i=0; uc[i]; i++)
	{
		unicode_char c=unicode_uc(uc[i]);

		if (revlookup(c))
			uc[i]=c;
	}

	s=(*u->u2c)(u, uc, NULL);
	free(uc);
	return (s);
}

static char *tolower_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=(*u->c2u)(u, cp, ip);
	char *s;

	unsigned i;

	if (!uc)
		return (NULL);

	for (i=0; uc[i]; i++)
	{
		unicode_char c=unicode_lc(uc[i]);

		if (revlookup(c))
			uc[i]=c;
	}

	s=(*u->u2c)(u, uc, NULL);
	free(uc);
	return (s);
}

static char *totitle_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=(*u->c2u)(u, cp, ip);
	char *s;

	unsigned i;

	if (!uc)
		return (NULL);

	for (i=0; uc[i]; i++)
	{
		unicode_char c=unicode_tc(uc[i]);

		if (revlookup(c))
			uc[i]=c;
	}

	s=(*u->u2c)(u, uc, NULL);
	free(uc);
	return (s);
}

const struct unicode_info unicode_BIG5_ETEN = {
	"BIG5",
	UNICODE_MB | UNICODE_REPLACEABLE | UNICODE_USASCII |
	UNICODE_HEADER_BASE64 | UNICODE_BODY_BASE64,
	c2u_eten,
	u2c_eten,
	toupper_func,
	tolower_func,
	totitle_func};

const struct unicode_info unicode_BIG5_HKSCS = {
	"BIG5-HKSCS",
	UNICODE_MB | UNICODE_REPLACEABLE | UNICODE_USASCII |
	UNICODE_HEADER_BASE64 | UNICODE_BODY_BASE64,
	c2u_hkscs,
	u2c_hkscs,
	toupper_func,
	tolower_func,
	totitle_func};

#if 0

int main()
{
	FILE *fp=popen("gunzip -cd <Unihan-3.2.0.txt.gz", "r");
	char buf[4000];
	unicode_char *uc;
	char *s, *p;
	int dummyi;

	if (!fp)
		return (0);

	while (fgets(buf, sizeof(buf), fp))
	{
		unsigned a, b, c;
		int dummy;

		if (sscanf(buf, "U+%4x kBigFive %4x", &b, &a) != 2)
			continue;
		printf("0x%04x 0x%04x: ", a, b);

		buf[0]= a / 256;
		buf[1]= a % 256;
		buf[2]=0;

		uc=c2u(buf, &dummy);
		if (!uc)
		{
			printf("c2u failure: %d\n", dummy);
			return (1);
		}
		if (uc[0] != b || uc[1])
		{
			printf("c2u failure: got 0x%04x, expected 0x%04x\n",
			       (unsigned)uc[0], (unsigned)b);
			return (1);
		}
		s=u2c(uc, &dummy);
		if (s == NULL && uc[0] == 0xfffd)
		{
			free(uc);
			printf("Ok\n");
			continue;	/* Unmapped */
		}
		free(uc);
		if (!s)
		{
			printf("u2c failure: %d\n", dummy);
			return (1);
		}

		c=0;
		if (!s[0] || !s[1] || s[2] ||
		    (c=(int)(unsigned char)s[0] * 256 +
		     (unsigned char)s[1]) != a)
		{
			printf("u2c failure: got 0x%04x, expected 0x%04x\n",
			       c, a);
			return (1);
		}

		p=toupper_func(s, NULL);
		if (!p)
		{
			printf("toupper failure\n");
			return (1);
		}
		if (strcmp(p, s))
			printf("toupper ");
		free(p);

		p=tolower_func(s, NULL);
		if (!p)
		{
			printf("tolower failure\n");
			return (1);
		}
		if (strcmp(p, s))
			printf("tolower ");
		free(p);

		p=totitle_func(s, NULL);
		if (!p)
		{
			printf("totitle failure\n");
			return (1);
		}
		if (strcmp(p, s))
			printf("totitle ");
		free(p);

		free(s);
		printf("ok\n");
	}
	fclose(fp);

	buf[0]=0x40;
	buf[1]=0;
	uc=c2u(buf, NULL);

	if (!uc)
	{
		printf("us-ascii c2u failure\n");
		return (1);
	}
	s=u2c(uc, NULL);
	free(uc);
	if (!s)
	{
		printf("us-ascii u2c failure\n");
		return (1);
	}
	free(s);

	buf[0]=0xA2;
	buf[1]=0x40;
	buf[2]=0;

	uc=c2u(buf, NULL);
	if (!uc)
	{
		printf("fallback failed\n");
		return (1);
	}
	printf("fallback: %04x %04x\n", (unsigned)uc[0],
	       (unsigned)uc[1]);

	s=u2c(uc, NULL);
	free(uc);

	if (!s)
	{
		printf("fallback-reverse failed\n");
		return (1);
	}
	printf("fallback: %02x %02x\n", (int)(unsigned char)s[0],
	       (int)(unsigned char)s[1]);
	free(s);

	buf[0]=0xA2;
	buf[1]=0x40;
	buf[2]=0;

	uc=c2u(buf, &dummyi);

	if (uc)
	{
		printf("abort failed\n");
		return (1);
	}

	printf("aborted at index %d\n", dummyi);

	{
		static unicode_char testing[]={0x0040, 0x1000, 0};

		uc=testing;

		s=u2c(uc, NULL);
		
		if (!s)
		{
			printf("abort-fallback failed\n");
			return (1);
		}
		printf("abort-fallback: %02x %02x\n", (int)(unsigned char)s[0],
		       (int)(unsigned char)s[1]);
		free(s);

		uc=testing;
	}

	s=u2c(uc, &dummyi);

	if (s)
	{
		printf("abort-abort failed\n");
		return (1);
	}

	printf("abort-aborted: index %d\n", dummyi);
	return (0);
}
#endif
