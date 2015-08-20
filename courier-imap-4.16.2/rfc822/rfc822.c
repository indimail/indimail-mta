/*
** Copyright 1998 - 2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

/*
*/
#include	"rfc822.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>

static void tokenize(const char *p, struct rfc822token *tokp, int *toklen,
	void (*err_func)(const char *, int, void *), void *voidp)
{
const char *addr=p;
int	i=0;
int	inbracket=0;

	*toklen=0;
	while (*p)
	{
		if (isspace((int)(unsigned char)*p))
		{
			p++;
			i++;
			continue;
		}

#define SPECIALS "<>@,;:.[]()%!\"\\?=/"

		switch (*p)	{
		int	level;

		case '(':
			if (tokp)
			{
				tokp->token='(';
				tokp->ptr=p;
				tokp->len=0;
			}
			level=0;
			for (;;)
			{
				if (!*p)
				{
					if (err_func) (*err_func)(addr, i,
								  voidp);
					if (tokp) tokp->token='"';
					++*toklen;
					return;
				}
				if (*p == '(')
					++level;
				if (*p == ')' && --level == 0)
				{
					p++;
					i++;
					if (tokp)	tokp->len++;
					break;
				}
				if (*p == '\\' && p[1])
				{
					p++;
					i++;
					if (tokp)	tokp->len++;
				}

				i++;
				if (tokp)	tokp->len++;
				p++;
			}
			if (tokp)	++tokp;
			++*toklen;
			continue;

		case '"':
			p++;
			i++;

			if (tokp)
			{
				tokp->token='"';
				tokp->ptr=p;
			}
			while (*p != '"')
			{
				if (!*p)
				{
					if (err_func) (*err_func)(addr, i,
								  voidp);
					++*toklen;
					return;
				}
				if (*p == '\\' && p[1])
				{
					if (tokp)	tokp->len++;
					p++;
					i++;
				}
				if (tokp)	tokp->len++;
				p++;
				i++;
			}
			++*toklen;
			if (tokp)	++tokp;
			p++;
			i++;
			continue;
		case '\\':
		case ')':
			if (err_func) (*err_func)(addr, i, voidp);
			++p;
			++i;
			continue;

		case '=':

			if (p[1] == '?')
			{
				int j;

			/* exception: =? ... ?= */

				for (j=2; p[j]; j++)
				{
					if (p[j] == '?' && p[j+1] == '=')
						break;

					if (p[j] == '?' || p[j] == '=')
						continue;

					if (strchr(RFC822_SPECIALS, p[j]) ||
					    isspace(p[j]))
						break;
				}

				if (p[j] == '?' && p[j+1] == '=')
				{
					j += 2;
					if (tokp)
					{
						tokp->token=0;
						tokp->ptr=p;
						tokp->len=j;
						++tokp;
					}
					++*toklen;

					p += j;
					i += j;
					continue;
				}
			}
			/* FALLTHROUGH */

		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '.':
		case '[':
		case ']':
		case '%':
		case '!':
		case '?':
		case '/':

			if ( (*p == '<' && inbracket) ||
				(*p == '>' && !inbracket))
			{
				if (err_func) (*err_func)(addr, i, voidp);
				++p;
				++i;
				continue;
			}

			if (*p == '<')
				inbracket=1;

			if (*p == '>')
				inbracket=0;

			if (tokp)
			{
				tokp->token= *p;
				tokp->ptr=p;
				tokp->len=1;
				++tokp;
			}
			++*toklen;

			if (*p == '<' && p[1] == '>')
					/* Fake a null address */
			{
				if (tokp)
				{
					tokp->token=0;
					tokp->ptr="";
					tokp->len=0;
					++tokp;
				}
				++*toklen;
			}
			++p;
			++i;
			continue;
		default:

			if (tokp)
			{
				tokp->token=0;
				tokp->ptr=p;
				tokp->len=0;
			}
			while (*p && !isspace((int)(unsigned char)*p) && strchr(
				SPECIALS, *p) == 0)
			{
				if (tokp)	++tokp->len;
				++p;
				++i;
			}
			if (i == 0)	/* Idiot check */
			{
				if (err_func) (*err_func)(addr, i, voidp);
				if (tokp)
				{
					tokp->token='"';
					tokp->ptr=p;
					tokp->len=1;
					++tokp;
				}
				++*toklen;
				++p;
				++i;
				continue;
			}
			if (tokp)	++tokp;
			++*toklen;
		}
	}
}

static void parseaddr(struct rfc822token *tokens, int ntokens,
		struct rfc822addr *addrs, int *naddrs)
{
int	flag, j, k;

	*naddrs=0;

	while (ntokens)
	{
	int	i;

		/* atoms (token=0) or quoted strings, followed by a : token
		is a list name. */

		for (i=0; i<ntokens; i++)
			if (tokens[i].token && tokens[i].token != '"')
				break;
		if (i < ntokens && tokens[i].token == ':')
		{
			++i;
			if (addrs)
			{
				addrs->tokens=0;
				addrs->name=i ? tokens:0;
				for (j=1; j<i; j++)
					addrs->name[j-1].next=addrs->name+j;
				if (i)
					addrs->name[i-1].next=0;
				addrs++;
			}
			++*naddrs;
			tokens += i;
			ntokens -= i;
			continue;  /* Group=phrase ":" */
		}

		/* Spurious commas are skipped, ;s are recorded */

		if (tokens->token == ',' || tokens->token == ';')
		{
			if (tokens->token == ';')
			{
				if (addrs)
				{
					addrs->tokens=0;
					addrs->name=tokens;
					addrs->name->next=0;
					addrs++;
				}
				++*naddrs;
			}
			++tokens;
			--ntokens;
			continue;
		}

		/* If we can find a '<' before the next comma or semicolon,
		we have new style RFC path address */

		for (i=0; i<ntokens && tokens[i].token != ';' &&
				tokens[i].token != ',' &&
					tokens[i].token != '<'; i++)
			;

		if (i < ntokens && tokens[i].token == '<')
		{
		int	j;

			/* Ok -- what to do with the stuff before '>'???
			If it consists exclusively of atoms, leave them alone.
			Else, make them all a quoted string. */

                        for (j=0; j<i && (tokens[j].token == 0 ||
                                          tokens[j].token == '('); j++)
                                ;

			if (j == i)
			{
				if (addrs)
				{
					addrs->name= i ? tokens:0;
					for (k=1; k<i; k++)
						addrs->name[k-1].next=addrs->name+k;
					if (i)
						addrs->name[i-1].next=0;
				}
			}
			else	/* Intentionally corrupt the original toks */
			{
				if (addrs)
				{
					tokens->len= tokens[i-1].ptr
							+ tokens[i-1].len
							- tokens->ptr;
					/* We know that all the ptrs point
					to parts of the same string. */
					tokens->token='"';
						/* Quoted string. */
					addrs->name=tokens;
					addrs->name->next=0;
				}
			}

		/* Any comments in the name part are changed to quotes */

			if (addrs)
			{
			struct rfc822token *t;

				for (t=addrs->name; t; t=t->next)
					if (t->token == '(')
						t->token='"';
			}

			/* Now that's done and over with, see what can
			be done with the <...> part. */

			++i;
			tokens += i;
			ntokens -= i;
			for (i=0; i<ntokens && tokens[i].token != '>'; i++)
				;
			if (addrs)
			{
				addrs->tokens=i ? tokens:0;
				for (k=1; k<i; k++)
					addrs->tokens[k-1].next=addrs->tokens+k;
				if (i)
					addrs->tokens[i-1].next=0;
				++addrs;
			}
			++*naddrs;
			tokens += i;
			ntokens -= i;
			if (ntokens)	/* Skip the '>' token */
			{
				--ntokens;
				++tokens;
			}
			continue;
		}

		/* Ok - old style address.  Assume the worst */

		/* Try to figure out where the address ends.  It ends upon:
		a comma, semicolon, or two consecutive atoms. */

		flag=0;
		for (i=0; i<ntokens && tokens[i].token != ',' &&
			tokens[i].token != ';'; i++)
		{
			if (tokens[i].token == '(')	continue;
					/* Ignore comments */
			if (tokens[i].token == 0 || tokens[i].token == '"')
								/* Atom */
			{
				if (flag)	break;
				flag=1;
			}
			else	flag=0;
		}
		if (i == 0)	/* Must be spurious comma, or something */
		{
			++tokens;
			--ntokens;
			continue;
		}

		if (addrs)
		{
			addrs->name=0;
		}

		/* Ok, now get rid of embedded comments in the address.
		Consider the last comment to be the real name */

		if (addrs)
		{
			struct	rfc822token	save_token;

			memset(&save_token, 0, sizeof(save_token));

			for (j=k=0; j<i; j++)
			{
				if (tokens[j].token == '(')
				{
					save_token=tokens[j];
					continue;
				}
				tokens[k]=tokens[j];
				k++;
			}

			if (save_token.ptr)
			{
				tokens[i-1]=save_token;
				addrs->name=tokens+i-1;
				addrs->name->next=0;
			}
			addrs->tokens=k ? tokens:NULL;
			for (j=1; j<k; j++)
				addrs->tokens[j-1].next=addrs->tokens+j;
			if (k)
				addrs->tokens[k-1].next=0;
			++addrs;
		}
		++*naddrs;
		tokens += i;
		ntokens -= i;
	}
}

static void print_token(const struct rfc822token *token,
		void (*print_func)(char, void *), void *ptr)
{
const char *p;
int	n;

	if (token->token == 0 || token->token == '(')
	{
		for (n=token->len, p=token->ptr; n; --n, ++p)
			(*print_func)(*p, ptr);
		return;
	}

	if (token->token != '"')
	{
		(*print_func)(token->token, ptr);
		return;
	}

	(*print_func)('"', ptr);
	n=token->len;
	p=token->ptr;
	while (n)
	{
		if (*p == '"' || (*p == '\\' && n == 1)) (*print_func)('\\', ptr);
		if (*p == '\\' && n > 1)
		{
			(*print_func)('\\', ptr);
			++p;
			--n;
		}
		(*print_func)(*p++, ptr);
		--n;
	}
	(*print_func)('"', ptr);
}

void rfc822tok_print(const struct rfc822token *token,
		void (*print_func)(char, void *), void *ptr)
{
int	prev_isatom=0;
int	isatom;

	while (token)
	{
		isatom=rfc822_is_atom(token->token);
		if (prev_isatom && isatom)
			(*print_func)(' ', ptr);
		print_token(token, print_func, ptr);
		prev_isatom=isatom;
		token=token->next;
	}
}

static void rfc822_prname_int(const struct rfc822addr *addrs,
			      void (*print_func)(char, void *),
			      void *ptr)

{
	struct rfc822token *i;
	int n;
	int	prev_isatom=0;
	int	isatom=0;

	for (i=addrs->name; i; i=i->next, prev_isatom=isatom)
	{
		isatom=rfc822_is_atom(i->token);
		if (isatom && prev_isatom)
			(*print_func)(' ', ptr);

		if (i->token == '"')
		{
			for (n=0; n<i->len; n++)
			{
				if (i->ptr[n] == '\\' &&
				    n + 1 < i->len)
					++n;
				(*print_func)(i->ptr[n], ptr);
			}
			continue;
		}

		if (i->token != '(')
		{
			print_token(i, print_func, ptr);
			continue;
		}

		for (n=2; n<i->len; n++)
			(*print_func)(i->ptr[n-1], ptr);
	}
}

static void rfc822_print_common_nameaddr_cntlen(char c, void *p)
{
	++ *(size_t *)p;
}

static void rfc822_print_common_nameaddr_saveaddr(char c, void *p)
{
	char **cp=(char **)p;

	*(*cp)++=c;
}

static int rfc822_print_common_nameaddr(const struct rfc822addr *addrs,
					char *(*decode_func)(const char *,
							     const char *, int),
					const char *chset,
					void (*print_func)(char, void *),
					void *ptr)
{
	size_t n=1;
	char *addrbuf, *namebuf;
	char *p, *q;
	int print_braces=0;

	if (addrs->tokens)
		rfc822tok_print(addrs->tokens,
				rfc822_print_common_nameaddr_cntlen, &n);


	p=addrbuf=malloc(n);

	if (!addrbuf)
		return -1;

	if (addrs->tokens)
		rfc822tok_print(addrs->tokens,
				rfc822_print_common_nameaddr_saveaddr, &p);

	*p=0;

	n=1;

	rfc822_prname_int(addrs,
			  rfc822_print_common_nameaddr_cntlen, &n);

	p=namebuf=malloc(n);

	if (!p)
	{
		free(addrbuf);
		return -1;
	}

	rfc822_prname_int(addrs,
			  rfc822_print_common_nameaddr_saveaddr, &p);

	*p=0;

	p=(*decode_func)(namebuf, chset, 0);

	free(namebuf);
	if (!p)
	{
		free(addrbuf);
		return -1;
	}

	for (namebuf=p; *p; p++)
	{
		print_braces=1;
		(*print_func)(*p, ptr);
	}
	free(namebuf);

	p=(*decode_func)(addrbuf, chset, 1);
	free(addrbuf);

	if (!p)
		return -1;

	if (print_braces)
		(*print_func)(' ', ptr);

	for (q=p; *q; ++q)
		if (*q != '.' && *q != '@' && strchr(RFC822_SPECIALS, *q))
		{
			print_braces=1;
			break;
		}

	if (print_braces)
		(*print_func)('<', ptr);

	for (addrbuf=p; *p; p++)
		(*print_func)(*p, ptr);

	if (print_braces)
		(*print_func)('>', ptr);

	free(addrbuf);
	return (0);
}

int rfc822_print(const struct rfc822a *rfcp, void (*print_func)(char, void *),
	void (*print_separator)(const char *s, void *), void *ptr)
{
	return rfc822_print_common(rfcp, 0, 0, print_func, print_separator, ptr);
}

int rfc822_print_common(const struct rfc822a *rfcp,
			char *(*decode_func)(const char *, const char *, int),
			const char *chset,
			void (*print_func)(char, void *),
			void (*print_separator)(const char *, void *),
			void *ptr)
{
const struct rfc822addr *addrs=rfcp->addrs;
int naddrs=rfcp->naddrs;

	while (naddrs)
	{
		if (addrs->tokens == 0)
		{
			rfc822tok_print(addrs->name, print_func, ptr);
			++addrs;
			--naddrs;
			if (addrs[-1].name && naddrs)
			{
			struct	rfc822token *t;

				for (t=addrs[-1].name; t && t->next; t=t->next)
					;

				if (t && (t->token == ':' || t->token == ';'))
					(*print_separator)(" ", ptr);
			}
			continue;
		}
		else if (addrs->name && addrs->name->token == '(')
		{	/* old style */

			if (!decode_func)
			{
				rfc822tok_print(addrs->tokens, print_func, ptr);
				(*print_func)(' ', ptr);
				rfc822tok_print(addrs->name, print_func, ptr);
			}
			else
			{
				if (rfc822_print_common_nameaddr(addrs,
								 decode_func,
								 chset,
								 print_func,
								 ptr) < 0)
					return -1;
			}
		}
		else
		{
			if (!decode_func)
			{
				int	print_braces=0;

				if (addrs->name)
				{
					rfc822tok_print(addrs->name,
							print_func, ptr);
					(*print_func)(' ', ptr);
					print_braces=1;
				}
#if 1
				else
				{
					struct rfc822token *p;

					for (p=addrs->tokens; p && p->next; p=p->next)
						if (rfc822_is_atom(p->token) &&
						    rfc822_is_atom(p->next->token))
							print_braces=1;
				}
#endif

				if (print_braces)
					(*print_func)('<', ptr);

				rfc822tok_print(addrs->tokens, print_func, ptr);

				if (print_braces)
					(*print_func)('>', ptr);
			}
			else
			{
				if (rfc822_print_common_nameaddr(addrs,
								 decode_func,
								 chset,
								 print_func,
								 ptr) < 0)
					return -1;
			}
		}
		++addrs;
		--naddrs;
		if (naddrs)
			if (addrs->tokens || (addrs->name &&
				rfc822_is_atom(addrs->name->token)))
				(*print_separator)(", ", ptr);
	}
	return 0;
}

void rfc822t_free(struct rfc822t *p)
{
	if (p->tokens)	free(p->tokens);
	free(p);
}

void rfc822a_free(struct rfc822a *p)
{
	if (p->addrs)	free(p->addrs);
	free(p);
}

void rfc822_deladdr(struct rfc822a *rfcp, int index)
{
int	i;

	if (index < 0 || index >= rfcp->naddrs)	return;

	for (i=index+1; i<rfcp->naddrs; i++)
		rfcp->addrs[i-1]=rfcp->addrs[i];
	if (--rfcp->naddrs == 0)
	{
		free(rfcp->addrs);
		rfcp->addrs=0;
	}
}

struct rfc822t *rfc822t_alloc_new(const char *addr,
	void (*err_func)(const char *, int, void *), void *voidp)
{
struct rfc822t *p=(struct rfc822t *)malloc(sizeof(struct rfc822t));

	if (!p)	return (NULL);
	memset(p, 0, sizeof(*p));

	tokenize(addr, NULL, &p->ntokens, err_func, voidp);
	p->tokens=p->ntokens ? (struct rfc822token *)
			calloc(p->ntokens, sizeof(struct rfc822token)):0;
	if (p->ntokens && !p->tokens)
	{
		rfc822t_free(p);
		return (NULL);
	}
	tokenize(addr, p->tokens, &p->ntokens, NULL, NULL);
	return (p);
}

struct rfc822a *rfc822a_alloc(struct rfc822t *t)
{
struct rfc822a *p=(struct rfc822a *)malloc(sizeof(struct rfc822a));

	if (!p)	return (NULL);
	memset(p, 0, sizeof(*p));

	parseaddr(t->tokens, t->ntokens, NULL, &p->naddrs);
	p->addrs=p->naddrs ? (struct rfc822addr *)
			calloc(p->naddrs, sizeof(struct rfc822addr)):0;
	if (p->naddrs && !p->addrs)
	{
		rfc822a_free(p);
		return (NULL);
	}
	parseaddr(t->tokens, t->ntokens, p->addrs, &p->naddrs);
	return (p);
}
