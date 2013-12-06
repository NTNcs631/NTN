/* $NetBSD: net.c,v 1.07 2013/11/25 22:42:00 Weiyu Exp $ */
/* $NetBSD: net.c,v 1.08 2013/11/25 23:08:33 Lin Exp $ */
 
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
  ""                          /* HTTP 0.9 void respond text*/
};

extern int flag_host_ipv6;
extern int p_port;
extern char *c_dir;
extern char *sws_dir;
extern char *i_address;

int clientsocket_fd;
long total_time = 0;

#define MAX_TIMEOUT 15
#define BUFFSIZE 64

void
clienttimer(int signal) {
  total_time ++;
  if (total_time > MAX_TIMEOUT) {
    if (write(clientsocket_fd, info[16], 
              strlen(info[16])) != strlen(info[16])) {
      fprintf(stderr, "Unable to write %s: %s\n",
              info[16], strerror(errno));
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
clientresponse(int newsocket_fd)
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
  do {
    memset(buffer, 0, strlen((char *)buffer));
    if (recv(clientsocket_fd, buffer, bufsize, 0) < 0) {
      fprintf(stderr, "Failed to receive Client Request: %s\n",
              strerror(errno));
      exit(1);
    }
    total_time = 0;
    parsereq(buffer, & req_info);
  } while(req_info.type != SIMPLE && req_info.status != 7 &&
          buffer[0] != '\n' && buffer[0] != '\r');
  
  printf("\n-----------------------");
  printf(" INFO ");
  printf("-----------------------\n");
  
  /* Client Info */
  if (client_address.ss_family == AF_INET) {
    struct sockaddr_in *addr = (struct sockaddr_in *)&client_address;

    printf("Client: %s:%d\n", 
           inet_ntop(AF_INET, &addr->sin_addr, ipstr, sizeof ipstr), 
           ntohs(addr->sin_port));
  } else { // AF_INET6
    struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&client_address;

    printf("Client: %s:%d\n", 
           inet_ntop(AF_INET6, &addr->sin6_addr, ipstr, sizeof ipstr), 
           ntohs(addr->sin6_port));
}

  /* HTTP0.9/1.0 */
  if (req_info.text)
    printf("%s\n", req_info.text);
  if (clientwrite(clientsocket_fd, & req_info))
    return 1;
  // close(clientsocket_fd);
  free(buffer);
  freereq(& req_info);
  return 0;    
}

int
clientwrite(int clientsocket_fd, ReqInfo * req_info)
{
  int clientsource_fd, n;
  char *pathname;
  char buf[BUFFSIZE];
  if (req_info->status == 0 || req_info->status == 17) {
    if (strcmp(req_info->resource,"/") == 0) {
      free (req_info->resource);
      req_info->resource = "/index.html";
    }
    if ((pathname = (char*)malloc((strlen(sws_dir)+
                                   strlen(req_info->resource)+1)*sizeof(char))) == NULL) {
      fprintf(stderr, "Unable to allocate memory: %s\n",
              strerror(errno));
      return 1;
    }
    strcpy(pathname, sws_dir);
    strcat(pathname, req_info->resource);
    if ((clientsource_fd = open(pathname,O_RDONLY)) == -1)
      switch (errno) {
	  case EACCES:
        req_info->status = 9;  /* 403 Forbidden */
        break;
	  case ENOENT:
        req_info->status = 10; /* 404 Not Found */
        break;
      default:
        req_info->status = 10; /* 404 Not Found(temporary) */
        break;
      }
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
  switch (req_info->status) {
  case 0:    /* "200 OK" */
  case 17:   /* HTTP 0.9 simple request*/
    switch (req_info->method) {
    case GET:
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
startsws(void)
{    
  int socket_fd, newsocket_fd;
  socklen_t addrlen;
  pid_t pid;
  struct sockaddr_in address;
  struct sockaddr_in6 address6;
  // int status;
 
  printf("\n-----------Starting Sever-----------\n");
  /* 
   * Some IPv6 socket cannot turn off IPV6_V6ONLY, 
   * so set them separately.
   */
  if (flag_host_ipv6) {
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
  if (flag_host_ipv6) {
    /* i_address: IPv6 format */
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

  if (flag_host_ipv6) {
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
  
   
  while (1) {    
    /* 
     * Block process until a client connects to the server. 
     */   
    if (flag_host_ipv6) {
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
      if (clientresponse(newsocket_fd) > 0)
        printf("Client Response Error. PID: %d\n", pid);
      exit(0);    /* exit child process */
    }
    else
      close(newsocket_fd);
    // sleep(2);
    // wait(&status);
 }
}