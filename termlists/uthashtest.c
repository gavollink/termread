#include <stdio.h>
#include "uthash/utarray.h"

int main() {
  UT_array *strs;
  char *s, **p;
  char st[BUFSIZ];

  utarray_new(strs,&ut_str_icd);

  strcpy(st, "hello");
  //s = "hello";
  s = st;
  utarray_push_back(strs, &s);
  strcpy(st, "world");
  // s = "world";
  s = st;
  utarray_push_back(strs, &s);
  p = NULL;
  while ( (p=(char**)utarray_next(strs,p))) {
    printf("%s\n",*p);
  }

  utarray_free(strs);

  return 0;
}
