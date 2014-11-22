#ifndef _PIPE_H_
#define _PIPE_H_

struct pipe {
  int pipefd[2];
};

void close_read(struct pipe *p);
void close_write(struct pipe *p);
void close_pipe(struct pipe *p);
struct pipe * create_pipe();
void init_new_pipe(struct pipe *res);
void init_default_pipe(struct pipe *res);

#endif /* !_PIPE_H_ */
