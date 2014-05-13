#ifndef __SETTINGS_H
#define __SETTINGS_H

void init_settings();
void free_settings();

char parse_commandline_parameters(int argc, char *argv[]);

void set_host_path(const char * /* path */);
const char *get_host_path();


#endif //__SETTINGS_H
