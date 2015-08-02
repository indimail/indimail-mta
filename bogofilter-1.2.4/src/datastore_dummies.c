
#include "common.h"
#include "datastore.h"
#include "datastore_db.h"

void *dbe_init(bfpath *p) {
    (void)p;
    return (void *)~0;
}

void dsm_init(bfpath *p) {
    (void)p;
}

void *db_get_env(void *vhandle) {
    (void)vhandle;
    return NULL;
}
