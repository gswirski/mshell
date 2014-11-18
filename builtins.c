#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "builtins.h"

int _two_args(char *argv[]) {
  return argv[1] && argv[2];
}

int lexit(char *argv[]) {
  exit(0);
}

int lecho(char * argv[]) {
  int i = 1;

  if (argv[i]) {
    printf("%s", argv[i++]);
  }

  while (argv[i]) {
    printf(" %s", argv[i++]);
  }

  printf("\n");
  fflush(stdout);
  return 0;
}

int lcd(char *argv[]) {
  char *path = argv[1];
  if (!path) {
    path = getenv("HOME");
  }

  int status = _two_args(argv) || chdir(path);
  if (status) {
    fprintf(stderr, "Builtin lcd error.\n");
  }

  return status;
}

int lkill(char *argv[]) {
  if (!_two_args(argv)) {
    return fprintf(stderr, "Builtin lkill error.\n");
  }
  int sig = strtol(argv[1] + 1, 0, 10);
  int pid = strtol(argv[2], 0, 10);

  return kill(pid, sig);
}

int lls(char *argv[]) {
  struct dirent *entry;
  DIR *directory = opendir(".");

  while ((entry = readdir(directory))) {
    if (entry->d_name[0] != '.') {
      printf("%s\n", entry->d_name);
    }
  }

  closedir(directory);
  return 0;
}

builtin_pair builtins_table[]={
  {"exit",	&lexit},
  {"lecho",	&lecho},
  {"lcd",		&lcd},
  {"lkill",	&lkill},
  {"lls",		&lls},
  {NULL,NULL}
};

builtin_ptr builtin_lookup(char *name) {
  builtin_pair *ptr = builtins_table;
  while (ptr->name && strcmp(ptr->name, name)) {
    ptr++;
  }
  return ptr->fun;
}
