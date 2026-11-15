#ifndef HELPER
#define HELPER

#include "parser.h"
#include <signal.h>

void handle_sigchld(int);
int change_signal_handler(struct sigaction *sigact, void (*handler)(int),
                          int signal);
void setPWD(void);
int internal_command_cd(const struct command *command);
int check_redirections(const struct command *command);
void execute_command(const struct command *command);

#endif
