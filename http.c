/* $NetBSD: http.c,v 1.02 2013/11/26 23:05:22 Lin Exp $ */
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
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

#include "net.h"

char *infoo[18] = {
  "200 OK",
  "201 Created",
  "202 Accepted",
  "204 No Content",
  "301 Moved Permanently",
  "302 Moved Temporarily",
  "304 Not Modified",
  "400 Bad Request",
  "401 Unauthorized",
  "403 Forbidden",
  "404 Not Found",
  "500 Internal Server Error",
  "501 Not Implemented",
  "502 Bad Gateway",
  "503 Service Unavailable",
  "505 Version Not Supported",
  "522 Connection Timed Out status",
  ""                          /* HTTP 0.9 void respond text*/
};

void
parsereq(unsigned char *buffer, ReqInfo *req_info)
{
  char *endptr = NULL, *tmp;
  int len;
  
  char *log = NULL;
  if ((log = (char*)malloc((1024)*sizeof(char))) == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n",
    strerror(errno));
    exit(1);
  }
  int fd;
  //int save_fd;
  fd = open("logfile", O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
  if(fd<0) {
	  perror("open");
	  exit(1);
  }
  //save_fd = dup(STDOUT_FILENO);
  dup2(fd, STDOUT_FILENO);
  close(fd);

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
	strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
    return;
  }
  else {
    req_info->status = BAD_REQUEST;       /* 400 Bad Request */
	strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
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
	strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
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
	  strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
      return;
    }
	else{
      req_info->status = BAD_REQUEST;       /* 400 Bad Request */
	  strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
      return;
    }
  }
  if (strncmp((char*)buffer, "HTTP/", 5) == 0) {
    buffer = buffer + 5;
    if (strncmp((char*)buffer, "1.0", 3) == 0) {
      req_info->type = FULL;
      req_info->status = OK;   /* 200 OK */
	  strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
      return;
    }
    else if (strncmp((char*)buffer, "0.9", 3) == 0) {
      req_info->type = FULL;
      req_info->status = OK;   /* 200 OK */
	  strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
    }
    else {
      req_info->type = FULL; 
      req_info->status = VERSION_NOT_SUPPORTED;  /* 505 Version Not Supported */
	  strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
      return;
    }
  }
  else {
    req_info->status = BAD_REQUEST;     /* 400 Bad Request */
	strcat(log,infoo[req_info->status]);
	write(STDOUT_FILENO, log, strlen(log));
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