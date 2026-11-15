#include "helper.h"
#include "shell.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void setPWD(void) {
  char path[PATH_BUFFER];
  if (getcwd(path, sizeof(path)) == NULL) {
    perror("getcwd");
    setenv("PWD", "InvalidPATH", 1);
    return;
  }
  setenv("PWD", path, 1);
}

// Handling Case where cd is called
// returns status -1 if failed, 0 success
int internal_command_cd(const struct command *command) {
  int status = 0;
  char path[PATH_BUFFER];

  switch (command->argc) {
  case 1: {
    const char *home = getenv("HOME");
    if (home == NULL) {
      fprintf(stderr, "cd: HOME is not set\n");
      return -1;
    } else if (chdir(home) == -1) {
      return -1;
    } else {
      setPWD();
      return 0;
    }
  }
  case 2:
    if (chdir(command->argv[1]) == -1)
      return -1;
    setPWD();
    return 0;
  default:
    errno = E2BIG;
    return -1;
  }
}

void handle_sigchld(int signal) {
  int saved_errno = errno;

  if (signal == SIGCHLD)
    reap_bg_jobs();

  errno = saved_errno;
}

int change_signal_handler(struct sigaction *sigact, void (*handler)(int),
                          int signal) {
  memset(sigact, 0, sizeof(*sigact));
  sigact->sa_handler = handler;
  sigemptyset(&sigact->sa_mask);
  if (signal == SIGCHLD)
    sigact->sa_flags = SA_RESTART;
  if (sigaction(signal, sigact, NULL) == -1) {
    return -1;
  }

  return 0;
}
// returns 0 if success , returns number of opened till failure
size_t initialize_pipes(int (*pipes)[2], size_t num_of_pipes) {
  for (size_t i = 0; i < num_of_pipes; i++) {
    if (pipe(pipes[i]) == -1)
      return i;
  }
  return num_of_pipes;
}

void close_pipes(int (*pipes)[2], size_t num_of_pipes) {
  for (size_t i = 0; i < num_of_pipes; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }
}

int check_redirections(const struct command *command) {
  for (size_t i = 0; i < command->redirect_count; i++) {
    const struct redirect *red = &command->redirects[i];
    switch (red->type) {
      int fd;
    case REDIRECT_INPUT:
      fd = open(red->path, O_RDONLY);
      if (fd == -1)
        return -1;
      if (dup2(fd, STDIN_FILENO) == -1) {
        close(fd);
        return -1;
      }
      if (fd != STDIN_FILENO)
        close(fd);
      break;
    case REDIRECT_OUTPUT:
      fd = open(red->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd == -1)
        return -1;

      if (dup2(fd, STDOUT_FILENO) == -1) {
        close(fd);
        return -1;
      }
      if (fd != STDOUT_FILENO)
        close(fd); // we do not need fid any more (we can access the file
                   // through 1).
      break;
    case REDIRECT_APPEND:
      fd = open(red->path, O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (fd == -1)
        return -1;

      if (dup2(fd, STDOUT_FILENO) == -1) {
        close(fd);
        return -1;
      }
      if (fd != STDOUT_FILENO)
        close(fd); // we do not need fid any more (we can access the file
                   // through 1).
      break;
    default:
      return -1;
    }
  }
  return 0;
}

void execute_command(const struct command *command) {
  if (command->redirect_count > 0) {
    if (check_redirections(command) == -1) {
      perror("minish:redirection");
      _exit(EXIT_FAILURE);
    }
  }

  if (execvp(command->argv[0], command->argv) == -1)
    perror("execvp:");
  _exit(EXIT_FAILURE);
}

void pipeline_child_process(const struct pipeline *pipeline,
                            size_t command_index, int (*pipeID)[2]) {
  // checking case where there is no pipe
  if (pipeline->command_count == 1) {
    execute_command(&pipeline->commands[command_index]);
  }

  // Changing the file descriptors for the pipe
  if (command_index == 0) {
    if (dup2(pipeID[command_index][1], STDOUT_FILENO) == -1) {
      perror("dup2");
      _exit(EXIT_FAILURE);
    }
  } else if (command_index == pipeline->command_count - 1) {
    if (dup2(pipeID[command_index - 1][0], STDIN_FILENO) == -1) {
      perror("dup2");
      _exit(EXIT_FAILURE);
    }
  } else {
    if (dup2(pipeID[command_index][1], STDOUT_FILENO) == -1 ||
        dup2(pipeID[command_index - 1][0], STDIN_FILENO) == -1) {
      perror("dup2");
      _exit(EXIT_FAILURE);
    }
  }
  close_pipes(pipeID, pipeline->command_count - 1);

  // execute_commands handles redirections and the final execution
  // of the command
  execute_command(&pipeline->commands[command_index]);
}
