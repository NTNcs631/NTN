/* $NetBSD: http.c,v 1.02 2013/11/26 23:05:22 Lin Exp $ */
 
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
 * THIS SOFTWARE IS PROVIDED BY WEIYU XUE "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL WEIYU XUE BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "net.h"

#define OK 0
#define CREATED 1
#define ACCEPTED 2
#define No_CONTENT 3
#define MOVED_PERMANENTLY 4
#define MOVED_TEMPORARILY 5
#define NOT_MODIFIED 6
#define BAD_REQUEST 7
#define UNAUTHORIZED 8
#define FORBIDDEN 9
#define NOT_FOUND 10
#define INTERNAL_SERVER_ERROR 11
#define NOT_IMPLEMENTED 12
#define BAD_GATEWAY 13
#define SERVICE_UNAVAILABLE 14
#define VERSION_NOT_SUPPORTED 15
#define TIME_OUT 16

void
parsereq(unsigned char *buffer, ReqInfo *req_info)
{
  char *endptr = NULL, *tmp;
  int len;
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
      req_info->status = 17;       /* simple response */
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
      req_info->status = 0;   /* 200 OK */
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
  req_info->method = 0;
  req_info->status = 7;
  req_info->type = -1;
}

void
freereq(ReqInfo * req_info)
{
  if (req_info->resource)
    free(req_info->resource);
  if (req_info->text)
    free(req_info->text);
}