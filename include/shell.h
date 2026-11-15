#ifndef SHELL_SOMETHING
#define SHELL_SOMETHING

#include "parser.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_SIZE 4096
#define MAX_JOBS 128
#define PATH_BUFFER 4096

struct job {
  pid_t pid;
  int active;
};

static struct job jobs[MAX_JOBS];

static void print_prompt(void);
static int read_line(char *buffer, size_t size);
static int execute_pipeline(const struct pipeline *pipeline);
static void init_bg_jobs(struct job *jobs);
static void add_bg_job(const pid_t pid);
static void remove_bg_job(int pid);
void reap_bg_jobs(void);

#endif
