#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
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

int main(int argc, char *argv[]) {
  char * input;
  line * ln;
  command *cmd;
  pid_t pid;
  int status, err;

  fstat(STDIN_FILENO, &stdin_stat);

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
