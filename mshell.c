#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "config.h"
#include "siparse.h"
#include "utils.h"

int
main(int argc, char *argv[])
{
  ssize_t input_len;
  char * input;
  line * ln;
  command *com;
  pid_t pid;
  int status;

  input = (char *)malloc(MAX_LINE_LENGTH+1);

  while (1) {
    printf(PROMPT_STR);
    fflush(stdout);

    input_len = read(STDIN_FILENO, input, MAX_LINE_LENGTH);
    input[input_len] = 0;

    if (input_len == 0) { printf("\n"); break; }
    if (input_len == 1) { continue; }

    ln = parseline(input);

    if (ln) {
      com = pickfirstcommand(ln);

      pid = fork();
      if (pid == 0) {
        execvp(com->argv[0], com->argv);
      } else {
        waitpid(pid, &status, WUNTRACED
#ifdef WCONTINUED
        | WCONTINUED
#endif
        );
      }
    }
  }

  return 0;
}
