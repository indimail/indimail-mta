#include <stdio.h>
#include "menu.h"
#include <time.h>

char           *progname = "";
time_t          start_time = 0l;
struct menu_items **GlobalHotKeys = NULL;
int             login_shell = 0;
