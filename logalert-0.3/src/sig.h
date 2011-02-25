#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifndef SIGNAL_H
#define SIGNAL_H

void sig_handle_abort(int sig);
void sig_handle_timer(uint timeout);
void sig_init();

#endif
