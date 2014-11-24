#include <stdlib.h>
#include <unistd.h>

#include "list.h"

void task_add(struct task_list *list, pid_t pid, int status) {
  struct task *task = malloc(sizeof(struct task));

  task->status = status;
  task->pid = pid;
  task->exit_code = 0;
  task->prev = NULL;
  task->next = list->head;

  if (list->head) {
    list->head->prev = task;
  }

  list->head = task;
}

struct task * task_find(struct task_list *list, pid_t pid) {
  struct task *task = list->head;
  while (task) {
    if (task->pid == pid) {
      return task;
    }
    task = task->next;
  }
  return NULL;
}

void task_delete(struct task_list *list, struct task *del) {
  if (del == NULL) { return; }
  if (del->prev) {
    del->prev->next = del->next;
  }
  if (del->next) {
    del->next->prev = del->prev;
  }
  if (del == list->head) {
    list->head = del->next;
  }
}
