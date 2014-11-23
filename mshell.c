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
#include "list.h"

#define SWAP(a, b) __typeof(a) c = b; b = a; a = c

static struct stat stdin_stat;

static volatile int fgn = 0;
static volatile struct task_list running;

void print_prompt() {
  if (S_ISCHR(stdin_stat.st_mode)) {
    printf(PROMPT_STR);
    fflush(stdout);
  }
}

void print_exited_process(struct task *task) {
  if (S_ISCHR(stdin_stat.st_mode)) {
    printf("Background process %d terminated. ", task->pid);
    if (WIFSIGNALED(task->exit_code)) {
      printf("(killed by signal %d)\n", WTERMSIG(task->exit_code));
    } else {
      printf("(exited with status %d)\n", WEXITSTATUS(task->exit_code));
    }
  }
}

void print_exited_processes() {
  struct task *task;
  for (int i = 0; i < running.tasks; i++) {
    task = &running.list[i];
    if (task->status == 0) {
      print_exited_process(task);
      task_delete(&running, task);
      i--;
    }
  }
}

void replace_fd(int a, int b) {
  if (a == b) return;
  close(a);
  dup2(b, a);
  close(b);
}

int handle_error(int no, char *subject) {
  if (!no) { return 0; }

  switch (errno) {
  case EACCES:
    fprintf(stderr, "%s: permission denied\n", subject);
    break;
  case ENOENT:
    fprintf(stderr, "%s: no such file or directory\n", subject);
    break;
  case ENOEXEC:
    fprintf(stderr, "%s: exec error\n", subject);
    break;
  }

  return no;
}

int setup_cmd_redirs(redirection *redirs[]) {
  int fd;
  redirection *redir;

  for (int i = 0; redirs[i]; i++) {
    redir = redirs[i];

    if (IS_RIN(redir->flags)) {
      fd = open(redir->filename, O_RDONLY);

      if (fd == -1) {
        return handle_error(errno, redir->filename);
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
        return handle_error(errno, redir->filename);
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

void handle_pipeline(command **cmd, int bg) {
  pid_t pid;
  struct pipe *lft, *rgt;

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

    if (!bg) {
      fgn++;
    }

    pid = fork();
    if (pid == 0) {
      if (bg) {
        pid_t pid = setsid();
      }

      if (setup_redirs(lft, *cmd, rgt) != 0) {
        exit(1);
      }
      execvp((*cmd)->argv[0], (*cmd)->argv);
      handle_error(errno, (*cmd)->argv[0]);
      exit(errno);
    }

    task_add(&running, pid, 2-bg);

    close_pipe(lft);
    SWAP(lft, rgt);
    init_default_pipe(rgt);

    cmd++;
  }

  close_pipe(lft);
  free(lft);

  close_pipe(rgt);
  free(rgt);
}

void sigchld_handler(int signo) {
  int status;
  pid_t pid;
  struct task *chld;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    chld = task_find(&running, pid);
    if (chld == NULL) {
      continue;
    }
    if (chld->status == TFOREGROUND) {
      fgn--;
      task_delete(&running, chld);
    } else {
      chld->status = 0;
      chld->exit_code = status;
    }
  }
}

void sigint_handler(int signo) {
}

int validate_line(line *ln) {
  pipeline *pipln = ln->pipelines;
  command **cmd;

  while (*pipln) {
    cmd = *pipln;
    if (cmd && *cmd && *(cmd+1)) {
      while (*cmd) {
        if (!(*cmd)->argv || !(*cmd)->argv[0]) {
          return 0;
        }
        cmd++;
      }
    }
    pipln++;
  }

  return 1;
}

int main(int argc, char *argv[]) {
  if (signal(SIGCHLD, &sigchld_handler) == SIG_ERR) {
    exit(1);
  }

  if (signal(SIGINT, &sigint_handler) == SIG_ERR) {
    exit(1);
  }

  char * input;
  line * ln;
  int status, err;

  fstat(STDIN_FILENO, &stdin_stat);

  while (1) {
    print_exited_processes();
    print_prompt();

    input = read_line(&err, &status);

    if (status) { break; }

    if (err) {
      fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
      continue;
    }

    ln = parseline(input);
    if (!validate_line(ln)) {
      fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
      continue;
    }

    pipeline *pipln = ln->pipelines;
    while (*pipln) {
      handle_pipeline(*pipln, ln->flags);
      pipln++;
    }

    while (fgn) {
      pause();
    }
  }

  return 0;
}
