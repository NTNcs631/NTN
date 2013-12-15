/* $NetBSD: http.c,v 1.05 2013/12/15 04:41:22 Lin Exp $ */
/* $NetBSD: http.c,v 1.03 2013/12/06 20:00:02 Qihuang Exp $ */
 
/* Copyright (c) 2013, NTNcs631
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in 
 *    the documentation and/or other materials provided with the distribution.
 * 3. The names of its contributors may be used to endorse or promote 
 *    products derived from this software without specific prior written 
 *    permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDERS AND CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <magic.h>

#include "net.h"

extern char *req_time;
extern char *first_line;
extern char *log_length;

#define GMT_LENGTH 24

void
parsereq(unsigned char *buffer, ReqInfo *req_info)
{
  char *endptr = NULL, *tmp;
  int len, i;
  if (buffer[1] == '\n')
    return; 
  if (req_info->text) {
    len = strlen((char *)buffer);
    if ((tmp = malloc((len+strlen(req_info->text)+1)*sizeof(char))) == NULL) {
      fprintf(stderr, "Unable to allocate memory: %s\n",
              strerror(errno));
      exit(1);
    }
    strcpy(tmp,req_info->text);
    strcat(tmp, (char*)buffer);
    free (req_info->text);
    req_info->text = tmp;
  }
  else {
    len = strlen((char*)buffer);
    if ((req_info->text = (char*)malloc((len+1)*sizeof(char))) == NULL) {
      fprintf(stderr, "Unable to allocate memory: %s\n",
              strerror(errno));
      exit(1);
    }
    strcpy(req_info->text, (char*)buffer);
    for (i=0; i<strlen(req_info->text); i++)
      if (req_info->text[i] == '\n' || req_info->text[i] == '\r')
        break;
    if ((first_line = (char*)malloc((i+3)*sizeof(char))) == NULL) {
      fprintf(stderr, "Unable to allocate memory: %s\n",
              strerror(errno));
      exit(1);
    }
    strcpy(first_line, "\"");
    strncat(first_line, req_info->text, i);
	strcat(first_line, "\"");
  }
  if (req_info->type == FULL)
    return;
  if (strncmp((char*)buffer, "GET ", 4) == 0) {
    req_info->method = GET;
    buffer = buffer + 4;
  }
  else if (strncmp((char*)buffer, "HEAD ", 5) == 0) {
    req_info->method = HEAD;
    buffer = buffer + 5;
  }
  else if (strncmp((char*)buffer, "POST ", 5) == 0) {
    req_info->method = POST;
    buffer = buffer + 5;
    req_info->status = NOT_IMPLEMENTED;      /* 501 Not Implemented */
    return;
  }
  else {
    req_info->status = BAD_REQUEST;       /* 400 Bad Request */
    return;
  }
  while (*buffer && isspace(*buffer))
    buffer++;
  endptr = strchr((char*)buffer, ' ');
  if (endptr == NULL) {
    req_info->type = SIMPLE;
    len = strlen((char*)buffer)-2; /* not include \r\n */
  }
  else
    len = endptr - (char*)buffer;
  if (len == 0) {
    req_info->status = BAD_REQUEST;       /* 400 Bad Request */
    return;
  }
  if ((req_info->resource = (char*)malloc((len+1)*sizeof(char))) == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n",
            strerror(errno));
    exit(1);
  }
  strncpy(req_info->resource, (char*)buffer, len);
  req_info->resource[len] = '\0';
  buffer = buffer + len;
  while (*buffer && isspace(*buffer))
    buffer++;
  if (!*buffer) {
    req_info->type = SIMPLE;
    if (req_info->method == GET) {
      req_info->status = SIMPLE_RESPONSE ;       /* simple response */
      return;
    }
	else{
      req_info->status = BAD_REQUEST;       /* 400 Bad Request */
      return;
    }
  }
  if (strncmp((char*)buffer, "HTTP/", 5) == 0) {
    buffer = buffer + 5;
    if (strncmp((char*)buffer, "1.0", 3) == 0) {
      req_info->type = FULL;
      req_info->status = OK;   /* 200 OK */
      return;
    }
    else if (strncmp((char*)buffer, "0.9", 3) == 0) {
      req_info->type = FULL;
      req_info->status = OK;   /* 200 OK */
    }
    else {
      req_info->type = FULL; 
      req_info->status = VERSION_NOT_SUPPORTED;  /* 505 Version Not Supported */
      return;
    }
  }
  else {
    req_info->status = BAD_REQUEST;     /* 400 Bad Request */
    return;
  }
  /* Not reached */
  return;
}

void
initreq(ReqInfo * req_info)
{
  req_info->resource = NULL;
  req_info->text = NULL;
  req_info->method = INIT;
  req_info->status = BAD_REQUEST;
  req_info->type = INIT;
}

void
freereq(ReqInfo * req_info)
{
  if (req_info->resource)
    free(req_info->resource);
  if (req_info->text)
    free(req_info->text);
}

int
getgmttime(char ** gmt_str)
{
  time_t gmt_time;
  struct tm *gmt_tm;

  gmt_time = time(NULL);
  if ((gmt_tm = gmtime(& gmt_time)) == NULL) {
    fprintf(stderr, "Failed in gmtime(): %s\n", strerror(errno));
    return 1;
  }
  if ((*gmt_str = asctime(gmt_tm)) == NULL) {
    fprintf(stderr, "Failed in asctime().\n");
    return 1;
  }
  return 0;
}

int getmtime(char * pathname, char **mtime_str)
{
  struct stat mode;
  struct tm *gmt_mtime;

  if (stat(pathname, &mode) < 0) {
    fprintf(stderr, "Unable to stat: %s\n", strerror(errno));
    return 1;
  }
  if ((gmt_mtime = gmtime(& mode.st_mtime)) == NULL) {
    fprintf(stderr, "Failed in gmtime(): %s\n", strerror(errno));
    return 1;
  }
  if ((*mtime_str = asctime(gmt_mtime)) == NULL) {
    fprintf(stderr, "Failed in asctime().\n");
    return 1;
  }
  return 0;
}

int
gettype(char ** file_type, char *pathname)
{
  magic_t magic;

  if ((magic = magic_open(MAGIC_MIME_TYPE)) == NULL) {
    fprintf(stderr, "Failed in magic_open: %s\n", strerror(errno));
    return 1;
  }
  if (magic_load(magic, NULL) < 0) {
    fprintf(stderr, "Failed in magic_load: %s\n", strerror(errno));
    return 1;
  }
  if ((*file_type = (char*)magic_file(magic, pathname)) == NULL) {
    fprintf(stderr, "Failed in magic_file: %s\n", strerror(errno));
    return 1; 
  }
  magic_close(magic);
  return 0;
}

char *
getlength(char *pathname)
{
  struct stat mode;
  int i = 1, j;
  long tmp;
  char *file_length;

  if (stat(pathname, &mode) < 0) {
    fprintf(stderr, "Unable to stat: %s\n", strerror(errno));
    return NULL;
  }
  tmp = (long)mode.st_size;
  while ((tmp/10) > 0) {
    tmp = tmp/10;
    i++;
  }
  if ((file_length = (char*)malloc((i+1)*sizeof(char))) == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n",
            strerror(errno));
    exit(1);
  }
  tmp = (long)mode.st_size;
  for (j=i-1; j>=0; j--) {
    file_length[j] = (char)(tmp % 10 + 48);
    tmp = tmp / 10;
  }
  file_length[i] = '\0';
  return file_length;
}
int
clienthead(int clientsocket_fd, char *info[18], ReqInfo *req_info, char *pathname)
{
  char *gmt_str;
  char *mtime_str;
  char *file_type;
  char *file_length;
  char hostname[128];

  if (req_info->type != SIMPLE)
    if (write(clientsocket_fd, "HTTP/1.0 ", strlen("HTTP/1.0 ")) != 
        strlen("HTTP/1.0 ")) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
  if (write(clientsocket_fd, info[req_info->status], 
            strlen(info[req_info->status])) != strlen(info[req_info->status])) {
    fprintf(stderr, "Unable to write %s: %s\n",
            info[req_info->status], strerror(errno));
    return 1;
  }
  if (write(clientsocket_fd, "\n", 1) != 1) {
    fprintf(stderr, "Unable to write: %s\n", strerror(errno));
    return 1;
  }
  if (getgmttime(& gmt_str)) {
    fprintf(stderr, "Unable to get gmt time.\n");
    return 1;
  }
  if ((req_time = (char*)malloc((strlen(gmt_str)+1)*sizeof(char))) == NULL) {
      fprintf(stderr, "Unable to allocate memory: %s\n",
              strerror(errno));
      return 1;
  }
  strcpy(req_time, gmt_str);
  req_time[GMT_LENGTH] = '\0';
  if (req_info->type != SIMPLE) {
    if (write(clientsocket_fd, "Date: ", strlen("Date: ")) != strlen("Date: ")) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, gmt_str, strlen(gmt_str)) != strlen(gmt_str)) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (gethostname(hostname, sizeof(hostname)) < 0) {
      fprintf(stderr, "Unable to get hostname: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, "Server: ", strlen("Server: ")) != 
        strlen("Server: ")) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, hostname, strlen(hostname)) != 
        strlen(hostname)) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, "\n", 1) != 1) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
  }
  if (req_info->status != OK && req_info->status != SIMPLE_RESPONSE)
    return 0;
  if (req_info->type != SIMPLE) {
    if (getmtime(pathname, &mtime_str)) {
      fprintf(stderr, "Unable to get modify time.\n");
      return 1;
    }
    if (write(clientsocket_fd, "Last-Modified: ", strlen("Last-Modified: ")) != 
              strlen("Last-Modified: ")) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, mtime_str, strlen(mtime_str)) != 
        strlen(mtime_str)) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (gettype(& file_type, pathname)) {
      fprintf(stderr, "Unable to get type.\n");
      return 1;
    }
    if (write(clientsocket_fd, "Content-Type: ", strlen("Content-Type: ")) != 
        strlen("Content-Type: ")) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, file_type, strlen(file_type)) != 
        strlen(file_type)) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, "\n", 1) != 1) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
  }
  if ((file_length = getlength(pathname)) == NULL) {
    fprintf(stderr, "Unable to get type.\n");
    return 1;
  }
  if ((log_length = (char*)malloc((strlen(file_length)+1)*sizeof(char))) == NULL) {
      fprintf(stderr, "Unable to allocate memory: %s\n",
              strerror(errno));
      return 1;
  }
  strcpy(log_length, file_length);
  if (req_info->type != SIMPLE) {
    if (write(clientsocket_fd, "Content-Length: ", strlen("Content-Length: ")) != 
        strlen("Content-Length: ")) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, file_length, strlen(file_length)) != 
        strlen(file_length)) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, "\n", 1) != 1) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
    if (write(clientsocket_fd, "\n", 1) != 1) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      return 1;
    }
  }

  return 0;
}