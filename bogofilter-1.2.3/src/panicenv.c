/** \file panicenv.c - Set Berkeley DB environment's PANIC state
 * \author Matthias Andree
 * \date 2005
 *
 * GNU General Public License v2 */

#include <db.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((noreturn))
static void barf(const char *tag, const int e)
{
    fprintf(stderr, "%s: %s\n", tag, db_strerror(e));
    exit(EXIT_FAILURE);
}

__attribute__((noreturn))
static void usage(int code)
{
    fprintf(stderr, "Usage: panicenv <ENVDIR>\n");
    exit(code);
}

int main(int argc, char **argv)
{
    DB_ENV *env;
    int e;

    if (argc != 2)
	usage(EXIT_FAILURE);

    e = db_env_create(&env, 0);
    if (e) barf("db_env_create", e);

    e = env->set_flags(env, DB_NOPANIC, 1);
    if (e) barf("DB_ENV->set_flags/DB_NOPANIC", e);

    e = env->set_flags(env, DB_NOMMAP, 1);
    if (e) barf("DB_ENV->set_flags/DB_NOMMAP", e);

    e = env->open(env, argv[1], DB_JOINENV, 0);
    if (e) barf("DB_ENV->open", e);

    e = env->set_flags(env, DB_PANIC_ENVIRONMENT, 1);
    if (e) barf("DB_ENV->set_flags/DB_PANIC_ENVIRONMENT", e);

    e = env->close(env, 0);
    if (e) barf("DB_ENV->close", e);

    return EXIT_SUCCESS;
}
