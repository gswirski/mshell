#ifndef _LIST_H_
#define _LIST_H_

/* PID recycling DOES NOT work */

struct task {
  int status;
  pid_t pid;
  int exit_code;
  //int d;
  //int chld;
  //struct task *sub[10];
};

struct task_list {
  int tasks;
  struct task list[1000];
};

#define TBACKGROUND 1
#define TFOREGROUND 2

void task_add(struct task_list *head, pid_t pid, int status) {
  struct task *task = &head->list[head->tasks++];
  task->status = status;
  task->pid = pid;
  task->exit_code = 0;
}

struct task * task_find(struct task_list *head, pid_t pid) {
  for (int i = 0; i < head->tasks; i++) {
    if (head->list[i].pid == pid) {
      return &head->list[i];
    }
  }
  return NULL;
}

void task_delete(struct task_list *head, struct task *del) {
  if (del == NULL) { return; }
  head->tasks--;
  del->status = head->list[head->tasks].status;
  del->pid = head->list[head->tasks].pid;
  del->exit_code = head->list[head->tasks].exit_code;
}

//void task_init(struct task *head) {
  //memset(head, 0, sizeof(struct task));
//}

//void task_add(struct task *head, pid_t pid, int status) {
  //char buff[10];
  //sprintf(buff, "%d", pid);
  //for (int i = 0; buff[i]; i++) {
    //if (head->sub[i] == NULL) {
      //head->chld++;
      //head->sub[i] = malloc(sizeof(struct task));
      //task_init(head->sub[i]);
      //head->sub[i]->d = buff[i];
    //}
    //head = head->sub[i];
  //}
  //head->status = status;
  //head->pid = pid;
//}

//struct task * task_find(struct task *head, pid_t pid) {
  //char buff[10];
  //sprintf(buff, "%d", pid);
  //for (int i = 0; buff[i]; i++) {
    //if (head->sub[i] == NULL) {
      //return NULL;
    //}
    //head = head->sub[i];
  //}
  //if (head->status == 0) {
    //return NULL;
  //}
  //return head;
//}

//void task_delete(struct task *head, struct task *t) {
  //if (t->chld) {
    //t->pid = t->status = t->exit_code = 0;
    //return;
  //}
//}

#endif /* !_LIST_H_ */
