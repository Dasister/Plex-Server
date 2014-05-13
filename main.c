
//Made by Avalank (GNU GPL v3)
//Designed by Alexander Litreev in St. Petersburg, Russia
//Working on Mac OS X 10.9

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>

#if defined(__APPLE__) || defined(__sun)
#include <libc.h>
#endif // OS X and Solaris

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>

#include "settings.h"

#define SERVER "Avalank PLEX/1.0"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define PORT 1234

char *get_mime_type(char *name)
{
    char *ext = strrchr(name, '.');
    if (!ext) return NULL;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".au") == 0) return "audio/basic";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0) return "video/mpeg";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    return NULL;
}

void send_headers(FILE *f, int status, char *title, char *extra, char *mime,
                  int length, time_t date)
{
    time_t now;
    char timebuf[128];
    
    fprintf(f, "%s %d %s\r\n", PROTOCOL, status, title);
    fprintf(f, "Server: %s\r\n", SERVER);
    now = time(NULL);
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    fprintf(f, "Date: %s\r\n", timebuf);
    if (extra) fprintf(f, "%s\r\n", extra);
    if (mime) fprintf(f, "Content-Type: %s\r\n", mime);
    if (length >= 0) fprintf(f, "Content-Length: %d\r\n", length);
    if (date != -1)
    {
        strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&date));
        fprintf(f, "Last-Modified: %s\r\n", timebuf);
    }
    fprintf(f, "Connection: close\r\n");
    fprintf(f, "\r\n");
}

void send_error(FILE *f, int status, char *title, char *extra, char *text)
{
    send_headers(f, status, title, extra, "text/html", -1, -1);
    fprintf(f, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\r\n", status, title);
    fprintf(f, "<BODY><H4>Avalank Plex | %d %s</H4>\r\n", status, title);
    fprintf(f, "%s\r\n", text);
    fprintf(f, "</BODY></HTML>\r\n");
}

void send_file(FILE *f, char *path, struct stat *statbuf)
{
    char data[4096];
    int n;
    
    FILE *file = fopen(path, "r");
    if (!file)
        send_error(f, 403, "403", NULL, "Access denied");
    else
    {
        int length = S_ISREG(statbuf->st_mode) ? statbuf->st_size : -1;
        send_headers(f, 200, "OK", NULL, get_mime_type(path), length, statbuf->st_mtime);
        
        while ((n = fread(data, 1, sizeof(data), file)) > 0) fwrite(data, 1, n, f);
        fclose(file);
    }
}

int process(FILE *f)
{
    char buf[4096];
    char buf2[4096];
    char *method;
    char *path;
    char *protocol;
    struct stat statbuf;
    char pathbuf[4096];
    int len;
    
    if (!fgets(buf, sizeof(buf), f)) return -1;
    
    strcpy(buf2,"1\n");
    
    while(strcmp(buf2,"\r\n")!=0){
        if (!fgets(buf2, sizeof(buf2), f)) return -1;
    }
    
    method = strtok(buf, " ");
    // Bad code. How to rewrite?
    path = strdup(strtok(NULL, " "));
    int maxlen = strlen(get_host_path()) + strlen(path) + 1;
    path = realloc(path, maxlen);
    // Dirty code. Need to rewrite.
    char *path_buf = strdup(path);
    snprintf(path, maxlen, "%s%s", get_host_path(), path_buf);
    free(path_buf);
    protocol = strtok(NULL, "\r");
    if (!method || !path || !protocol) return -1;
    
    fseek(f, 0, SEEK_CUR);
    
    if (strcmp(method, "GET") != 0)
        send_error(f, 501, "Not supported", NULL, "Method is not supported.");
    else if (stat(path, &statbuf) < 0)
        send_error(f, 404, "Not Found", NULL, "File not found.");
    else if (S_ISDIR(statbuf.st_mode))
    {
        len = strlen(path);
        if (len == 0 || path[len - 1] != '/')
        {
            snprintf(pathbuf, sizeof(pathbuf), "Location: %s/", path);
            send_error(f, 302, "Found", pathbuf, "Directories must end with a slash.");
        }
        else
        {
            snprintf(pathbuf, sizeof(pathbuf), "%sindex.html", path);
            if (stat(pathbuf, &statbuf) >= 0)
                send_file(f, pathbuf, &statbuf);
            else
            {
                DIR *dir;
                struct dirent *de;
                
                send_headers(f, 200, "OK", NULL, "text/html", -1, statbuf.st_mtime);
                fprintf(f, "<HTML><HEAD><TITLE>Index of %s</TITLE></HEAD>\r\n<BODY>", path);
                fprintf(f, "<H4>Index of %s</H4>\r\n<PRE>\n", path);
                fprintf(f, "Name Last Modified Size\r\n");
                fprintf(f, "<HR>\r\n");
                if (len > 1) fprintf(f, "<A HREF=\"..\">..</A>\r\n");
                
                dir = opendir(path);
		// ALWAYS CHECK THE RESULT OF SUCH FUNCTIONS! ALWAYS!
		if (dir != NULL) {
		  while ((de = readdir(dir)) != NULL)
		    {
		      char timebuf[32];
		      struct tm *tm;
		      
		      strcpy(pathbuf, path);
		      strcat(pathbuf, de->d_name);
		      
		      stat(pathbuf, &statbuf);
		      tm = gmtime(&statbuf.st_mtime);
		      strftime(timebuf, sizeof(timebuf), "%d-%b-%Y %H:%M:%S", tm);
		      
		      fprintf(f, "<A HREF=\"%s%s\">", de->d_name, S_ISDIR(statbuf.st_mode) ? "/" : "");
		      fprintf(f, "%s%s", de->d_name, S_ISDIR(statbuf.st_mode) ? "/</A>" : "</A> ");
		      if (de->d_reclen < 32) fprintf(f, "%*s", 32 - de->d_reclen, "");
		      
		      if (S_ISDIR(statbuf.st_mode))
                        fprintf(f, "%s\r\n", timebuf);
		      else
                        fprintf(f, "%s %10d\r\n", timebuf, statbuf.st_size);
		    }
		  closedir(dir);
		}
                
                fprintf(f, "</PRE>\r\n<HR>\r\n<ADDRESS>%s</ADDRESS>\r\n</BODY></HTML>\r\n", SERVER);
            }
        }
    }
    else
        send_file(f, path, &statbuf);
   
    // Free!
    free(path);
 
    return 0;
}

int main(int argc, char *argv[])
{
  init_settings();
  
  int sock;
  struct sockaddr_in sin;
  
  printf("Avalank Plex Server 1.0\n");
  
  if (!parse_commandline_parameters(argc, argv)) {
    return -1;
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(PORT);
  if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    printf("Socket binding error: %d\n", errno);
    return -1;
  }
  
  listen(sock, 5);
  printf("HTTP server listening on port %d at %s\n", PORT, inet_ntoa(sin.sin_addr));
  printf("Virtual server path: %s\n", get_host_path());
  
  while (1)
    {
      int s;
      FILE *f;
      
      s = accept(sock, NULL, NULL);
      if (s < 0) break;
      
      f = fdopen(s, "r+");
      process(f);
      fclose(f);
    }
  
  free_settings();
  close(sock);
  return 0;
}
