/* $NetBSD: net.c,v 1.04 2013/11/22 19:18:13 Weiyu Exp $ */
/* $NetBSD: net.c,v 1.02 2013/11/19 19:19:10 Lin Exp $ */
 
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

int
clientresponse(int clientsocket_fd)
{
  int bufsize = 1024;
  char *buffer;
  ReqInfo req_info;
  socklen_t client_addrlen;
  struct sockaddr_in client_address;

  if ((buffer = (char*)malloc(bufsize*sizeof(char))) == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n",
            strerror(errno));
    return 1;
  }

  client_addrlen = sizeof(client_address);
  if (getpeername(clientsocket_fd, (struct sockaddr *) &client_address, &client_addrlen) == -1) {
    fprintf(stderr, "Get client socket name failed: %s\n",
      strerror(errno));
    return 1;
  }
  
  memset(buffer, 0, strlen(buffer));
  initreq(& req_info);
  recv(clientsocket_fd, buffer, bufsize, 0);
  parsereq(buffer, & req_info);

  printf("-----------------------");
  printf(" INFO ");
  printf("-----------------------\n");
  printf("Clent: %s:%d\n", inet_ntoa(client_address.sin_addr), 
         ntohs(client_address.sin_port));
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