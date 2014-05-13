#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Make subdirs.
#include "string.h"

string *host_path = NULL;

void init_settings() {
  host_path = make_string();
}

void free_settings() {
  if (host_path) {
    free_string(host_path);
  }
}

static void print_usage(const char *file_name) {
  printf("Usage: %s -p \"<Virtual Server Path>\"\n", file_name);
}

char parse_commandline_parameters(int argc, char *argv[]) {
  int i = 1;
  if (argc < 3) {
    print_usage(argv[0]);
    return 0;
  }
  while (i < argc) {
    if (strcmp(argv[i], "-p") == 0) {
      i++;
      string_concat(host_path, argv[i]);
      // ToDo: Check last symbol of string is "/" and replace it.
    }
    i++;
  }
  return 1;
}

const char *get_host_path() {
  return (const char*)host_path->data;
}
