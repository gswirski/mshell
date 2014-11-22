#include <stdlib.h>
#include <unistd.h>
#include "pipe.h"

void close_read(struct pipe *p) {
  if (p->pipefd[0] > 2) {
    close(p->pipefd[0]);
    p->pipefd[0] = STDIN_FILENO;
  }
}

void close_write(struct pipe *p) {
  if (p->pipefd[1] > 2) {
    close(p->pipefd[1]);
    p->pipefd[1] = STDOUT_FILENO;
  }
}

void close_pipe(struct pipe *p) {
  close_read(p);
  close_write(p);
}

struct pipe * create_pipe() {
  struct pipe *res = (struct pipe *)malloc(sizeof(struct pipe));
  init_default_pipe(res);
  return res;
}

void init_new_pipe(struct pipe *res) {
  pipe(res->pipefd);
}

void init_default_pipe(struct pipe *res) {
  res->pipefd[0] = STDIN_FILENO;
  res->pipefd[1] = STDOUT_FILENO;
}
