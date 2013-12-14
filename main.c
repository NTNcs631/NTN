/* $NetBSD: main.c,v 1.03 2013/11/25 22:42:00 Weiyu Exp $ */
/* $NetBSD: main.c,v 1.04 2013/11/26 23:01:11 Lin Exp $ */
 
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>

#include "net.h"

/*
 * Flags
 */

int flag_d = 0; 
int host_ipv = INIT;


/*
 * Functions
 */
void
usage()
{
  fprintf(stderr, "Usage: sws[-dh][-c dir][-i address][-l file][-p port]dir\n");
  exit(EXIT_FAILURE);
  /* NOTREACHED */
}

int
dircheck(char *dir)
{
  struct stat dir_stat;

  if (stat(dir, &dir_stat) == -1) {
    if(errno == ENOENT) {
      fprintf(stderr, "No such directory: %s\n", dir);
      return 1;
    }
    else {
      perror("Stat Error");
      return 1;
    }
  } 

  if(S_ISDIR(dir_stat.st_mode))
    return 0;
  else {
    fprintf(stderr, "Not a directory: %s\n", dir);
    return 1;
  }
}

int
ipcheck(char *i_address)
{
  unsigned char buf[sizeof(struct in6_addr)];

  if (inet_pton(AF_INET6, i_address, buf) == 1) {
    /* IPv6 */
    host_ipv = IPADDR_V6;
    return 0;
  }
  else if (inet_pton(AF_INET, i_address, buf) == 1) {
    /* IPv4 */
    host_ipv = IPADDR_V4;
    return 0;
  }
  else {
    fprintf(stderr, "IP address not valid: %s\n", i_address);
    return 1;
  }
}

int
filecheck(char *file)
{
  struct stat file_stat;

  if (stat(file, &file_stat) == -1) {
    if(errno == ENOENT) {
      fprintf(stderr, "No such file: %s\n", file);
      return 1;
    }
    else {
      perror("Stat Error");
      return 1;
    }
  } 

  if(S_ISREG(file_stat.st_mode))
    return 0;
  else {
    fprintf(stderr, "Not a regular file: %s\n", file);
    return 1;
  }
}

int
portcheck(int port)
{
  if (port <= 0) {
    fprintf(stderr, "Port not valid\n");
    return 1;
  }
  else
    return 0;
}


/*
 * Main
 */
int 
main(int argc, char *argv[])
{
  int ch;
  int p_port = 8080;
  char *c_dir = NULL;
  char *l_file = NULL;
  char *sws_dir = NULL;
  char *i_address = "::1";
  while ((ch = getopt(argc, argv, "dhc:i:l:p:")) != -1) {
    switch (ch) {
    case 'd':
      flag_d = 1;
      break;

    case 'c':
      c_dir = optarg;
      if(dircheck(c_dir))
        exit(EXIT_FAILURE);
      break;

    case 'i':
      i_address = optarg;
      if (ipcheck(i_address))
        exit(EXIT_FAILURE);
      break;

    case 'l':
      l_file = optarg;
      if (filecheck(l_file))
        exit(EXIT_FAILURE);
      break;

    case 'p':
      p_port = atoi(optarg);
    if (portcheck(p_port))
        exit(EXIT_FAILURE);
      break;

    default:
    case 'h':
    case '?':
      usage();
    }
  }
  argc -= optind;
  argv += optind;

  if (argc==1)
    sws_dir = *argv;
  else
    usage();

  if (!IPv6) {
    printf("Host environment does not support IPv6.");
    exit(EXIT_FAILURE);
  }

  if (dircheck(sws_dir))
    exit(EXIT_FAILURE);

  startsws(i_address, p_port, sws_dir, c_dir, host_ipv, flag_d);

  return 0;
}