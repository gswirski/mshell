#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2

typedef struct {
  char* name;
  int (*fun)(char**);
} builtin_pair;

typedef int (*builtin_ptr)(char**);

extern builtin_pair builtins_table[];

builtin_ptr builtin_lookup(char *);

#endif /* !_BUILTINS_H_ */
