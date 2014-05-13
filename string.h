#ifndef __STRING_H
#define __STRING_H

#define min(a, b) ((a) > (b) ? (b) : (a))
#define max(a, b) ((a) > (b) ? (a) : (b))

struct string {
  char *data;
  int length;
};

typedef struct string string;

string *make_string();

void string_concat(string */*to*/, const char */*what*/);

const char *get_data(const string */*str*/);

void free_string(string */*str*/);

#endif // __STRING_H
