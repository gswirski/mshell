#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "reader.h"
#include "builtins.h"

static struct stat stdin_stat;
extern char **environ;

void print_prompt() {
  if (S_ISCHR(stdin_stat.st_mode)) {
    printf(PROMPT_STR);
    fflush(stdout);
  }
}

void replace_fd(int a, int b) {
  close(a);
  dup2(b, a);
  close(b);
}

int setup_file_descriptors(redirection *redirs[]) {
  int fd;
  redirection *redir;

  for (int i = 0; redirs[i]; i++) {
    redir = redirs[i];

    if (IS_RIN(redir->flags)) {
      fd = open(redir->filename, O_RDONLY);

      if (fd == -1) {
        fprintf(stderr, "%s: couldn't open file\n", redir->filename);
        return -1;
      }

      replace_fd(STDIN_FILENO, fd);
    } else {
      int mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

      if (IS_ROUT(redir->flags)) {
        fd = open(redir->filename, O_WRONLY|O_TRUNC|O_CREAT, mode);
      } else {
        fd = open(redir->filename, O_WRONLY|O_APPEND|O_CREAT, mode);
      }

      if (fd == -1) {
        fprintf(stderr, "%s: couldn't open file\n", redir->filename);
        return -1;
      }

      replace_fd(STDOUT_FILENO, fd);
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
  char * input;
  line * ln;
  command *cmd;
  pid_t pid;
  int status, err;

  fstat(STDIN_FILENO, &stdin_stat);

  repl:
  while (1) {
    print_prompt();

    input = read_line(&err, &status);

    if (status) { break; }

    if (err) {
      fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
      continue;
    }

    ln = parseline(input);
    cmd = pickfirstcommand(ln);

    if (!cmd || !cmd->argv || !cmd->argv[0]) {
      continue;
    }

    builtin_ptr builtin = builtin_lookup(cmd->argv[0]);
    if (builtin) {
      builtin(cmd->argv);
      continue;
    }

    pid = fork();
    if (pid == 0) {
      if (setup_file_descriptors(cmd->redirs) == -1) {
        exit(0); // probably should fail with errno
        // problem with error handling later
      }
      execvp(cmd->argv[0], cmd->argv);
      exit(errno);
    } else {
      wait(&status);

      if (WEXITSTATUS(status)) {
        fprintf(stderr, "%s: ", cmd->argv[0]);

        switch (WEXITSTATUS(status)) {
        case EACCES:
          fprintf(stderr, "permission denied\n");
          break;
        case ENOENT:
          fprintf(stderr, "no such file or directory\n");
          break;
        case ENOEXEC:
          fprintf(stderr, "exec error\n");
          break;
        }
      }
    }
  }

  return 0;
}
