#ifndef GENERIC_CONF_H
#define GENERIC_CONF_H

#include <sys/types.h>
#include "buffer.h"

extern void init(const char *,const char *);

extern void makedir(const char *);
extern void makelink(const char *, const char *, const char *);

extern void start(const char *);
extern void outs(const char *);
extern void out(const char *,unsigned int);
extern void copyfrom(buffer *);
extern void finish(void);

extern void perm(mode_t);
extern void owner(uid_t,gid_t);
extern void makelog(const char *,const char *,const char *,uid_t,gid_t);

#endif
