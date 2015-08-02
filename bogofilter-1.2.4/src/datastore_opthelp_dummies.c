#include "common.h"
#include "datastore_db.h"

/* help messages and option processing */

const char **dsm_help_bogofilter(void)
{
    static const char *help_text[] = {
	NULL
    };
    return &help_text[0]; 
}

const char **dsm_help_bogoutil(void)
{
    static const char *help_text[] = {
	NULL
    };
    return &help_text[0]; 
}

bool dsm_options_bogofilter(int option, const char *name, const char *val)
{
    (void) option;
    (void) name;
    (void) val;
    return false;
}

bool dsm_options_bogoutil(int option, cmd_t *flag, int *count, const char **ds_file, const char *name, const char *val)
{
    (void) option;
    (void) flag;
    (void) count;
    (void) ds_file;
    (void) name;
    (void) val;
    return false;
}
