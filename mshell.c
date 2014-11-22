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
#include "pipe.h"
#include "builtins.h"

#define SWAP(a, b) __typeof(a) c = b; b = a; a = c

static struct stat stdin_stat;

struct pid_queue {
  pid_t pid;
  struct pid_queue *next;
};

void print_prompt() {
  if (S_ISCHR(stdin_stat.st_mode)) {
    printf(PROMPT_STR);
    fflush(stdout);
  }
}

void replace_fd(int a, int b) {
  if (a == b) return;
  close(a);
  dup2(b, a);
  close(b);
}

int handle_error(char *filename) {
  if (errno == EACCES) {
    fprintf(stderr, "%s: permission denied\n", filename);
  } else {
    fprintf(stderr, "%s: no such file or directory\n", filename);
  }
  return -1;
}

int setup_cmd_redirs(redirection *redirs[]) {
  int fd;
  redirection *redir;

  for (int i = 0; redirs[i]; i++) {
    redir = redirs[i];

    if (IS_RIN(redir->flags)) {
      fd = open(redir->filename, O_RDONLY);

      if (fd == -1) {
        return handle_error(redir->filename);
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
        return handle_error(redir->filename);
      }

      replace_fd(STDOUT_FILENO, fd);
    }
  }

  return 0;
}

int setup_redirs(struct pipe *lft, command *cmd, struct pipe *rgt) {
  close_write(lft);
  close_read(rgt);
  replace_fd(STDIN_FILENO, lft->pipefd[0]);
  replace_fd(STDOUT_FILENO, rgt->pipefd[1]);
  return setup_cmd_redirs(cmd->redirs);
}

void handle_pipeline(command **cmd) {
  pid_t pid;
  struct pipe *lft, *rgt;
  struct pid_queue *X, *Q = NULL;

  if (!(*cmd) || !(*cmd)->argv || !(*cmd)->argv[0]) {
    return;
  }

  if (!(*(cmd+1))) {
    builtin_ptr builtin = builtin_lookup((*cmd)->argv[0]);
    if (builtin) {
      builtin((*cmd)->argv);
      return;
    }
  }

  lft = create_pipe();
  rgt = create_pipe();

  while (*cmd) {
    if (*(cmd+1)) {
      init_new_pipe(rgt);
    }

    pid = fork();
    if (pid == 0) {
      if (setup_redirs(lft, *cmd, rgt) == -1) {
        exit(1);
      }
      execvp((*cmd)->argv[0], (*cmd)->argv);

      fprintf(stderr, "%s: ", (*cmd)->argv[0]);

      switch (errno) {
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
      exit(errno);
    }

    X = (struct pid_queue *)malloc(sizeof(struct pid_queue));
    X->pid = pid;
    X->next = Q;
    Q = X;


    close_pipe(lft);
    SWAP(lft, rgt);
    init_default_pipe(rgt);

    cmd++;
  }

  close_pipe(lft);
  free(lft);

  close_pipe(rgt);
  free(rgt);

  int status;
  while (Q) {
    waitpid(Q->pid, &status, 0);

    X = Q;
    Q = Q->next;
    free(X);
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

    pipeline *pipln = ln->pipelines;
    while (*pipln) {
      handle_pipeline(*pipln);
      pipln++;
    }
  }

  return 0;
}
