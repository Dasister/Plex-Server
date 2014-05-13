#include "string.h"

#include <string.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#else // __APPLE__
#include <malloc.h> 
#endif // Linux


string *make_string() {
  string *str = (string *)malloc(sizeof(string));
  if (str) {
    str->length = -1;
    str->data = NULL;
  }
  return str;
}

void string_concat(string *to, const char *what) {
  size_t size_from;
  size_t size_to;
#ifdef __APPLE__
  size_from = strlen(what) + 1;
  if (to->data)
    size_to = malloc_size(to->data);
  else
    size_to = 0;
#else // __APPLE__
  size_from = strlen(what);
  size_to = malloc_usable_size(to->data);
#endif // Linux
  if (size_from <= size_to + size_from) {
    // ToDo: Rewrite this shit.
    to->data = realloc(to->data, size_to + size_from);
  }
  strcat(to->data, what);
}

const char *get_data(const string *str) {
  // ToDo: Check "str" for NULL;
  return (const char *)str->data;
}

void free_string(string *str) {
  if (str) {
    if (str->data) {
      free(str->data);
      str->data = NULL;
      str->length = -1;
    }
    free(str);
    str = NULL;
  }
}


