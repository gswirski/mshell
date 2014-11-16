#include <stdio.h>
#include <unistd.h>
#include "config.h"

static char buff[MAX_LINE_LENGTH+1];
static int buff_len;
static int buff_pos;

static char line[MAX_LINE_LENGTH+1];
static int line_len;
static int line_overflow;

char * get_line(int *err) {
  *err = line_overflow;
  line[line_len] = 0;
  line_len = 0;
  line_overflow = 0;
  return line;
}

char * read_line(int *err, int *done) {
  *done = 0;
  while (buff_pos < buff_len && buff[buff_pos] != '\n') {
    if (line_len >= MAX_LINE_LENGTH) {
      line_overflow = 1;
    } else {
      line[line_len++] = buff[buff_pos];
    }
    buff_pos++;
  }

  if (buff_pos < buff_len && buff[buff_pos] == '\n') {
    buff_pos++;
    return get_line(err);
  }

  buff_len = read(STDIN_FILENO, buff, MAX_LINE_LENGTH+1);
  buff_pos = 0;

  if (buff_len == 0) {
    *done = 1;
    return get_line(err);
  }

  return read_line(err, done);
}
