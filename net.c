/* $NetBSD: net.c,v 1.11 2013/12/13 21:11:02 Weiyu Exp $ */
/* $NetBSD: net.c,v 1.14 2013/12/15 00:44:33 Lin Exp $ */
/* $NetBSD: net.c,v 1.13 2013/12/14 22:32:13 Qihuang Exp $ */

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
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>

#include "net.h"

char *info[18] = {
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
  "" /* HTTP 0.9 void respond text*/
};

int clientsocket_fd;
long total_time = 0;

#define MAX_TIMEOUT 15
#define BUFFSIZE 64


int
clientls(char *pathname, int clientsocket_fd)
{
  struct dirent **namelist;
  int i, file_number;
  if ((file_number = scandir(pathname, &namelist, 0, alphasort)) < 0) {
     fprintf(stderr, "Unable to scan directory: %s\n", strerror(errno));
     return 1;
  }
  for (i=0; i<file_number; i++)
    if (namelist[i]->d_name[0] == '.')
      free(namelist[i]);
    else {
      if (write(clientsocket_fd, namelist[i]->d_name, strlen(namelist[i]->d_name)) != 
          strlen(namelist[i]->d_name)) {
        fprintf(stderr, "Unable to write: %s\n", strerror(errno));
        return 1;
      }
      if (write(clientsocket_fd, "\n", 1) != 1) {
        fprintf(stderr, "Unable to write: %s\n", strerror(errno));
        return 1;
      }
      free(namelist[i]);
    }
  free(namelist);
  return 0;
}
void
clienttimer(int signal)
{
  total_time ++;
  if (total_time > MAX_TIMEOUT) {
    if (write(clientsocket_fd, info[TIME_OUT], 
              strlen(info[TIME_OUT])) != strlen(info[TIME_OUT])) {
      fprintf(stderr, "Unable to write %s: %s\n",
              info[TIME_OUT], strerror(errno));
      exit(1);
   }
    if (write(clientsocket_fd, "\n", 1) != 1) {
      fprintf(stderr, "Unable to write: %s\n", strerror(errno));
      exit(1);
    }
	exit(1);
  }
  return;
}

void 
clienttimerinit(void)
{
  struct itimerval value, ovalue;
  /* set signal : Alarm clock */
  if (signal(SIGALRM, clienttimer) < 0) {
    fprintf(stderr, "Failed to set Alarm Clock signal: %s\n",
            strerror(errno));
    exit(1);
  }
  value.it_value.tv_sec = 1; 
  value.it_value.tv_usec = 0; 
  value.it_interval.tv_sec = 1; 
  value.it_interval.tv_usec = 0;
  /* set value of interval timer */
  if (setitimer(ITIMER_REAL, &value, &ovalue) < 0) {
    fprintf(stderr, "Failed to set timer: %s\n",
            strerror(errno));
    exit(1);
  }
  return ;
}

int
clientresponse(int newsocket_fd, char *sws_dir, char *c_dir)
{
  clientsocket_fd = newsocket_fd;
  int bufsize = 1024;
  unsigned char *buffer;
  ReqInfo req_info;
  socklen_t client_addrlen;
  struct sockaddr_storage client_address;
  char ipstr[INET6_ADDRSTRLEN];

  /* Set a timer to detect timeout */
  clienttimerinit();
  if ((buffer = (unsigned char*)malloc(bufsize*sizeof(char))) == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n",
            strerror(errno));
    return 1;
  }

  /* Get Client IP Address */
  client_addrlen = sizeof(client_address);
  if (getpeername(clientsocket_fd, (struct sockaddr *) &client_address, &client_addrlen) == -1) {
    fprintf(stderr, "Get client socket name failed: %s\n",
            strerror(errno));
    return 1;
  }
  
  /* HTTP0.9/1.0 */
  initreq(& req_info);
  while(1) {
    memset(buffer, 0, strlen((char *)buffer));
    if (recv(clientsocket_fd, buffer, bufsize, 0) < 0) {
      fprintf(stderr, "Failed to receive Client Request: %s\n",
              strerror(errno));
      exit(1);
    }
    total_time = 0;
    parsereq(buffer, & req_info);
    if (req_info.type == SIMPLE)
      break;
    if (req_info.status == BAD_REQUEST)
      break;
    if ((buffer[0] == '\n') || (buffer[0] == '\r'))
      break;
    if (strstr((char *)buffer,"\r\n\r\n"))
      break;
  }
  
  printf("\n-----------------------");
  printf(" INFO ");
  printf("-----------------------\n");
  
  /* Client Info */
  if (client_address.ss_family == AF_INET) {
    struct sockaddr_in *addr = (struct sockaddr_in *)&client_address;

    printf("Client: %s:%d\n", 
           inet_ntop(AF_INET, &addr->sin_addr, ipstr, sizeof(ipstr)), 
           ntohs(addr->sin_port));
  } 
  else { /* AF_INET6 */
    struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&client_address;
    printf("Client: %s:%d\n", 
           inet_ntop(AF_INET6, &addr->sin6_addr, ipstr, sizeof(ipstr)), 
           ntohs(addr->sin6_port));
  }

  /* HTTP0.9/1.0 */
  if (req_info.text)
    printf("%s\n", req_info.text);
  if (clientwrite(clientsocket_fd, & req_info, sws_dir, c_dir))
    return 1;
  // close(clientsocket_fd);
  free(buffer);
  freereq(& req_info);
  return 0;    
}

int
clientwrite(int clientsocket_fd, ReqInfo * req_info, char *sws_dir, char *c_dir)
{
  int clientsource_fd, n, need_ls = 0;
  char *cur_dir = sws_dir;
  char *pathname;
  char *tmp_pathname;
  char buf[BUFFSIZE];
  struct stat mode;

  if (req_info->status == OK || req_info->status == SIMPLE_RESPONSE) {
    if (strncmp(req_info->resource, "/cgi-bin", strlen("/cgi-bin")) == 0 && c_dir) {
      cur_dir = c_dir;
      req_info->resource = req_info->resource + strlen("/cgi-bin");
    }
    if ((pathname = (char*)malloc((strlen(cur_dir)+
                                   strlen(req_info->resource)+1)*sizeof(char))) == NULL) {
      fprintf(stderr, "Unable to allocate memory: %s\n",
              strerror(errno));
      return 1;
    }
    strcpy(pathname, cur_dir);
    strcat(pathname, req_info->resource);
    if (stat(pathname, &mode) < 0) {
      switch (errno) {
      case EACCES:
        req_info->status = FORBIDDEN;  /* 403 Forbidden */
        break;
      case ENOENT:
        req_info->status = NOT_FOUND; /* 404 Not Found */
        break;
      default:
        req_info->status = NOT_FOUND; /* 404 Not Found(temporary) */
        break;
      }
    }
    else {
      if (S_ISDIR(mode.st_mode)) {
        if ((tmp_pathname = (char*)malloc(strlen(pathname)+strlen("/index.html")+1)) == NULL) {
          fprintf(stderr, "Unable to allocate memory: %s\n",
                  strerror(errno));
          return 1;
        }
        strcpy(tmp_pathname, pathname);
        strcat(tmp_pathname, "/index.html");
        if (stat(tmp_pathname, &mode) < 0) {
          switch (errno) {
          case EACCES:
            req_info->status = FORBIDDEN;  /* 403 Forbidden */
            break;
          case ENOENT:
            need_ls = 1;
            break;
          default:
            req_info->status = NOT_FOUND; /* 404 Not Found(temporary) */
            break;
          }
        }
		else {
          free(pathname);
          pathname = tmp_pathname;
        }
      }
      if (need_ls)
        ;
      else {
        if ((clientsource_fd = open(pathname,O_RDONLY)) == -1) {
          switch (errno) {
          case EACCES:
            req_info->status = FORBIDDEN;  /* 403 Forbidden */
            break;
          case ENOENT:
            req_info->status = NOT_FOUND; /* 404 Not Found */
            break;
          default:
            req_info->status = NOT_FOUND; /* 404 Not Found(temporary) */
            break;
          }
        }
      }
    }
  }
  if (req_info->type == FULL) {
    if (clienthead(clientsocket_fd, info, req_info, pathname) < 0) {
      fprintf(stderr, "Unable to write head: %s\n",
              strerror(errno));
      return 1;
    }
  }
  else {
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
  }
  switch (req_info->status) {
  case OK:    /* "200 OK" */
  case SIMPLE_RESPONSE:   /* HTTP 0.9 simple request*/
    switch (req_info->method) {
    case GET:
      if (need_ls) {
        if (clientls(pathname,clientsocket_fd) < 0) {
          fprintf(stderr, "Unable to list files: %s\n",
                  strerror(errno));
          return 1;
        }
      }
	  else
        while ((n = read(clientsource_fd, buf, BUFFSIZE)) > 0) {
          if (write(clientsocket_fd, buf, n) != n) {
            fprintf(stderr, "Unable to write: %s\n",
                    strerror(errno));
            return 1;
          }
        }
      return 0;
      break;
/*	case HEAD:  */
/*	case POST:  */
    }
    break;
  }
  return 0;
}

void
startsws(char *i_address, int p_port, char *sws_dir, char *c_dir, int host_ipv, int flag_d)
{
  int socket_fd, newsocket_fd;
  socklen_t addrlen;
  pid_t pid;
  struct sockaddr_in address;
  struct sockaddr_in6 address6;
  int status;
 
  printf("\n-----------Starting Sever-----------\n");
  printf("host_ipv: %d\n", host_ipv);
  /* 
   * Some IPv6 socket cannot turn off IPV6_V6ONLY, 
   * so set them separately.
   */
  if ((host_ipv == IPADDR_V6) || (host_ipv == INIT)) {
    /* socket: IPv6 */
    if ((socket_fd = socket(AF_INET6, SOCK_STREAM, 0)) > 0) {
      printf("Socket created: %d\n", socket_fd);
      printf("Binding socket ...\n");
    }
    else {
      fprintf(stderr, "Unable to create socket: %s\n",
              strerror(errno));
      exit (1);
    }
  }
  else {
    /* socket: IPv4 */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0) {
      printf("Socket created: %d\n", socket_fd);
      printf("Binding socket ...\n");
    }
    else {
      fprintf(stderr, "Unable to create socket: %s\n",
              strerror(errno));
      exit (1);
    }
  }
  
  /* SET socket address/host machine/port number */ 
  if ((host_ipv == IPADDR_V6) || (host_ipv == INIT)) {
    /* i_address: IPv6 format */
    if (host_ipv == INIT) 
      address6.sin6_addr = in6addr_any;
    else 
      (void)inet_pton(AF_INET6, i_address, address6.sin6_addr.s6_addr);
    
    address6.sin6_family = AF_INET6;
    address6.sin6_port = htons(p_port);
  }
  else {
    /* i_address: IPv4 format */
    address.sin_addr.s_addr = inet_addr(i_address);
    address.sin_family = AF_INET;
    address.sin_port = htons(p_port);
  }

  if ((host_ipv == IPADDR_V6) || (host_ipv == INIT)) {
    /* IPv6 Host */
    if (bind(socket_fd, (struct sockaddr *) &address6, 
            sizeof(address6)) == 0) {
      printf("Socket Binding Completed:\nSocket: %d | Port: %d | Address: %s \n\n",
             socket_fd, p_port, i_address);
    }
    else {
      fprintf(stderr, "Unable to create bind %d: %s\n",
              socket_fd, strerror(errno));
      exit (1);
    }
  }
  else {
    /* IPv4 Host */
    if (bind(socket_fd, (struct sockaddr *) &address, 
            sizeof(address)) == 0) {
      printf("Socket Binding Completed:\nSocket: %d | Port: %d | Address: %s \n\n",
             socket_fd, p_port, i_address);
    }
    else {
      fprintf(stderr, "Unable to create bind %d: %s\n",
              socket_fd, strerror(errno));
      exit (1);
    }
  }
  /* 
   * Listen on the socket for connections.
   */
  if (listen(socket_fd, 5) < 0) {    
    perror("server: listen");    
    exit(1);
  }

  if(!flag_d && daemon(1, 0) < 0) {
    perror("daemon");
    exit(1);
  }
  
   
  while (1) {    
    /* 
     * Block process until a client connects to the server. 
     */   
    if ((host_ipv == IPADDR_V6) || (host_ipv == INIT)) {
      if ((newsocket_fd=accept(socket_fd, (struct sockaddr *) &address6,
                            &addrlen)) < 0) {    
        perror("server: accept");    
        exit(1);
      }
    }
    else {
      if ((newsocket_fd=accept(socket_fd, (struct sockaddr *) &address,
                            &addrlen)) < 0) {    
        perror("server: accept");    
        exit(1);
      }
    }
    /*
     * Client Connection Response
     */
    if ((pid = fork()) < 0)
      printf("Failed to fork process to response client request.\n");

    if (pid == 0) {             /* Child Process */
      close(socket_fd);
      if (clientresponse(newsocket_fd, sws_dir, c_dir) > 0)
        printf("Client Response Error. PID: %d\n", pid);
      exit(0);    /* exit child process */
    }
    else
      close(newsocket_fd);

    // sleep(2);
    if (flag_d)
      wait(&status);
 }
}