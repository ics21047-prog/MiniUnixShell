#ifndef HELPER
#define HELPER

#include "parser.h"
#include <signal.h>
#include <stddef.h>

void handle_sigchld(int);
int change_signal_handler(struct sigaction *sigact, void (*handler)(int),
                          int signal);
void setPWD(void);
int internal_command_cd(const struct command *command);

size_t initialize_pipes(int (*pipes)[2], size_t num_of_pipes);
void close_pipes(int (*pipes)[2], size_t num_of_pipes);

int check_redirections(const struct command *command);
void execute_command(const struct command *command);
void pipeline_child_process(const struct pipeline *pipeline,
                            size_t command_index, int (*pipeID)[2]);

#endif
