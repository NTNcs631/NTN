/* $NetBSD: net.c,v 1.03 2013/11/20 19:53:33 Weiyu Exp $ */
/* $NetBSD: net.c,v 1.05 2013/11/22 21:00:15 Lin Exp $ */
 
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

#include "net.h"

char *info[17] = {
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
  "522 Connection Timed Out status"
};

int clientsocket_fd;
long total_time = 0;
#define MAX_TIMEOUT 20

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
  signal(SIGALRM, clienttimer);
  value.it_value.tv_sec = 1; 
  value.it_value.tv_usec = 0; 
  value.it_interval.tv_sec = 1; 
  value.it_interval.tv_usec = 0;
  /* set value of interval timer */
  if (setitimer(ITIMER_REAL, &value, &ovalue) <0 ) {
    fprintf(stderr, "Fail to set timer: %s\n",
            strerror(errno));
    exit(1);
  }
  return ;
}
int
ClientResponse(int newsocket_fd)
{
  clientsocket_fd = newsocket_fd;
  int bufsize = 1024;
  unsigned char *buffer;
  ReqInfo req_info;
  /* Set a timer to detect timeout */
  clienttimerinit();
  if ((buffer = (unsigned char*)malloc(bufsize*sizeof(char))) == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n",
            strerror(errno));
    return 1;
  }
  
  memset(buffer, 0, strlen((char *)buffer));
  initreq(& req_info);
  recv(clientsocket_fd, buffer, bufsize, 0);
  parsereq((char *)buffer, & req_info);
  printf("-----------------------");
  printf("Client~ INFO");
  printf("-----------------------\n");
  printf("%s\n", buffer);
  if (write(clientsocket_fd, info[req_info.status], 
            strlen(info[req_info.status])) != strlen(info[req_info.status])) {
    fprintf(stderr, "Unable to write %s: %s\n",
            info[req_info.status], strerror(errno));
    return 1;
  }
  if (write(clientsocket_fd, "\n", 1) != 1) {
    fprintf(stderr, "Unable to write: %s\n", strerror(errno));
    return 1;
  }
  // close(clientsocket_fd);
  free(buffer);

  return 0;    
}

void
startsws(char *i_address, int p_port)
{    
  int socket_fd, newsocket_fd;
  socklen_t addrlen;
  struct sockaddr_in address;
  pid_t pid;
  // int status;
 
  printf("\n-----------Starting Sever-----------\n");
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0) {
    printf("Socket created: %d\n", socket_fd);
    printf("Binding socket ...\n");
  }
  else {
    fprintf(stderr, "Unable to create socket: %s\n",
            strerror(errno));
    exit (1);
  }
  
  /* SET socket address/host machine/port number */ 
  // memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(i_address);
  address.sin_port = htons(p_port);

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
    if ((newsocket_fd=accept(socket_fd, (struct sockaddr *) &address,
                          &addrlen)) < 0) {    
      perror("server: accept");    
      exit(1);
    }
    /*
     * Client Connection Response
     */
    if ((pid = fork()) < 0)
      printf("Failed to fork process to response client request.\n");

    if (pid == 0) {             /* Child Process */
      close(socket_fd);
      if (ClientResponse(newsocket_fd) > 0)
        printf("ClientResponse Error. PID: %d\n", pid);
      exit(0);    /* exit child process */
    }
    else
      close(newsocket_fd);
    // sleep(2);
    // wait(&status);
 }
}