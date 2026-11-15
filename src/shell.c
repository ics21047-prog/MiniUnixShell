/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/*
 * Minish shell.
 *
 * Add The name of the first team member here in the form Surname Lastname,
 * iis... Add The name of the first team member here in the form Surname
 * Lastname, iis...
 */

#include "shell.h"
#include "errno.h"
#include "helper.h"
#include "parser.h"

// Add the prototypes of helper funcitons here..
/////HELPER FUNCTION AT helper.c
/**
 * @brief Run the minish command interpreter.
 *
 * @return Process exit status.
 */
int main(void) {
  char line[INPUT_SIZE];
  struct pipeline pipeline;
  int error;
  int result;

  // Install the required signal handlers here.
  // Add your code here.
  struct sigaction chld_action, int_action;

  // implementing the signal handler for termination of background jobs
  if (change_signal_handler(&chld_action, handle_sigchld, SIGCHLD) == -1) {
    perror("sigaction");
    return -1;
  }
  // implementing the signal handler for SIGINT
  if (change_signal_handler(&int_action, SIG_IGN, SIGINT) == -1) {
    perror("sigaction");
    return -1;
  }

  // The main shell loop.
  for (;;) {
    // Print the prompt.
    print_prompt();

    // Read a command line from the user.
    result = read_line(line, sizeof(line));

    // Handle read errors.
    if (result == 0)
      break;

    if (result < 0) {
      if (errno == EINTR)
        continue;

      perror("minish: Cannot read the command line.");
      return EXIT_FAILURE;
    }

    // Parse the line.
    error = parse_line(line, &pipeline);

    // An error occurred during parsing.
    if (error != PARSE_SUCCESS) {
      fprintf(stderr, "minish: %s\n", parse_error_string(error));
      continue;
    }

    // The user just pressed enter. We have nothing to execute at this time.
    if (pipeline.command_count == 0) {
      pipeline_free(&pipeline);
      continue;
    }

    // Execute the command pipeline.
    if ((result = execute_pipeline(&pipeline)) < 0)
      perror("minish");

    // Clear the pipeline.
    pipeline_free(&pipeline);
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Print the interactive shell prompt.
 */
static void print_prompt(void) {
  if (isatty(STDIN_FILENO) != 0) {
    (void)printf("%s@minish$ ", getenv("PWD"));
    (void)fflush(stdout);
  }
}

/**
 * @brief Read one command line from standard input.
 *
 * @param[out] buffer Destination buffer.
 * @param[in] size Size of the destination buffer.
 *
 * @return 1 when a line was read, 0 on EOF, or -1 on error.
 */
static int read_line(char *buffer, size_t size) {
  if (fgets(buffer, size, stdin) == NULL) {
    if (feof(stdin) != 0)
      return 0;

    return -1;
  }

  buffer[strcspn(buffer, "\n")] = '\0';

  return 1;
}

void init_bg_jobs(struct job *jobs) {
  int i;

  if (!jobs)
    return;

  for (i = 0; i < MAX_JOBS; i++) {
    jobs[i].pid = -1;
    jobs[i].active = 0;
  }
}

void add_bg_job(const pid_t pid) {
  int i;

  for (i = 0; i < MAX_JOBS; i++) {
    // Find the first position in the array with active == 0 and add the
    // new job there. Then call break to exit the loop.
    // Add your code here...
    if (jobs[i].active == 0) {
      jobs[i].pid = pid;
      jobs[i].active = 1;
      return;
    }
  }
}

void remove_bg_job(int pid) {
  int i;
  // Find the first position in the array where its pid matches with the
  // pid parameter of the function and then clean it by setting pid to
  // -1 and active to 0.
  // Add your code here...
  for (i = 0; i < MAX_JOBS; i++) {
    if (pid == jobs[i].pid) {
      jobs[i].pid = -1;
      jobs[i].active = 0;
    }
  }
}

// The function below should be called from a signal handler.
void reap_bg_jobs(void) {
  pid_t pid;
  int status;
  int i;

  for (i = 0; i < MAX_JOBS; i++) {
    // Call pid = waitpid() system call properly for each array slot.
    // If this call returns a positive pid, then this means that we just
    // found a terminated background process. Call remove_bg_job()
    // properly to remove this process from the jobs array.

    // Add your code here...
    if (!jobs[i].active)
      continue;
    pid = waitpid(jobs[i].pid, &status, WNOHANG);
    if (pid > 0) {
      jobs[i].pid = -1;
      jobs[i].active = 0;
    }
  }
}

/**
 * @brief Execute all commands in a parsed pipeline.
 *
 * @param[in] pipeline Parsed pipeline.
 *
 * @return 0 on success, -1 on error, -2 if exit was requested.
 */
static int execute_pipeline(const struct pipeline *pipeline) {
  pid_t *pids;
  int result = 0;

  // First, we have to handle the internal commands. To do this, we need to
  // check if our command count is one, and if the name of the command matches
  // an internal command. Remember that the command name is always stored in
  // argv[0].

  // Add your code here...
  if (pipeline->command_count == 1) {
    if (!strncmp(pipeline->commands[0].argv[0], "exit", 5)) {
      pipeline_free((struct pipeline *)pipeline);
      exit(0);
    } else if (!strncmp(pipeline->commands[0].argv[0], "cd", 3)) {
      result = internal_command_cd(&pipeline->commands[0]);
      return result;
    }
  }

  // No internal command found. We have to execute a new program for each
  // command in the pipeline.

  // Allocate an array to store the PID of each child process.
  // You may use the array below, if you find it helpful.
  pids = calloc(pipeline->command_count, sizeof(*pids));
  if (pids == NULL)
    return -1;

  // Process each command in the pipeline. Remember that the command_count
  // tells us the number of commands in the pipeline.
  // E.g., cmd1 | cmd2 : 2 commands.
  // E.g., cmd1 : 1 command.

  // Add your code here... This implementation can execute only one command for
  // now
  /// Initialize the pipes
  size_t num_of_pipes = pipeline->command_count - 1;
  int (*pipeID)[2] = malloc(num_of_pipes * sizeof(*pipeID));

  for (size_t i = 0; i < num_of_pipes; i++) {
    pipe(pipeID[i]);
  }
  for (size_t command_index = 0; command_index < pipeline->command_count;
       command_index++) {

    pids[command_index] = fork();
    if (pids[command_index] == -1) {
      return -1;
    }

    if (pids[command_index] == 0) {
      // Changing the SIGINT handler for background
      if (pipeline->background != 1) {
        struct sigaction dfl;
        if (change_signal_handler(&dfl, SIG_DFL, SIGINT) == -1) {
          perror("sigaction");
          _exit(EXIT_FAILURE);
        }
      }

      // checking case where there is no pipe
      if (pipeline->command_count == 1) {
        execute_command(&pipeline->commands[command_index]);
      }

      // Changing the file descriptors for the pipe
      if (command_index == 0) {
        close(pipeID[command_index][0]);
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
      for (size_t i = 0; i < num_of_pipes; i++) {
        close(pipeID[i][0]);
        close(pipeID[i][1]);
      }

      // execute_commands handles redirections and the final execution
      // of the command
      execute_command(&pipeline->commands[command_index]);
    }
    // Adding the background process to jobs array
    // If the if the pipeline is not a background job we need to wait
    if (pipeline->background == 1) {
      add_bg_job(pids[command_index]);
    }
  }
  // closing all the pipes
  for (size_t i = 0; i < num_of_pipes; i++) {
    close(pipeID[i][0]);
    close(pipeID[i][1]);
  }
  // if the pipeline is not in the background the the parent needs to wait
  // for the children to finish
  if (pipeline->background != 1) {
    int child_status = 0;
    for (size_t i = 0; i < pipeline->command_count; i++) {
      if (waitpid(pids[i], &child_status, 0) == -1)
        result = -1;
      if (child_status == EXIT_FAILURE)
        result = -1;
    }
  }
  free(pipeID);
  free(pids);
  return result;
}
