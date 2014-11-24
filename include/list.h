#ifndef _LIST_H_
#define _LIST_H_

/* PID recycling DOES NOT work */

struct task {
  int status;
  pid_t pid;
  int exit_code;
  struct task *prev, *next;
};

struct task_list {
  struct task *head;
};

#define TBACKGROUND 1
#define TFOREGROUND 2

void task_add(struct task_list *list, pid_t pid, int status);
struct task * task_find(struct task_list *list, pid_t pid);
void task_delete(struct task_list *list, struct task *del);

#endif /* !_LIST_H_ */
