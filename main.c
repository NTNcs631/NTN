/* $NetBSD: main.c,v 1.01 2013/11/15 13:40:40 Weiyu Exp $ */
 
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "net.h"

/*
 * Flags
 */

int flag_d = 0; 
int flag_h = 0; 

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

void
dircheck(char *dir)
{
  struct stat dir_stat;

  if (stat(dir, &dir_stat) == -1) {
    if(errno == ENOENT) {
      fprintf(stderr, "No such directory: %s\n", dir);
      exit(EXIT_FAILURE);
    }
    else {
      perror("Stat Error");
      exit(EXIT_FAILURE);
    }
  } 

  if(S_ISDIR(dir_stat.st_mode))
    exit(EXIT_SUCCESS);
  else {
    fprintf(stderr, "Not a directory: %s\n", dir);
    exit(EXIT_FAILURE);
  }

  // closedir(dir);
}

/*
 * Main
 */
int 
main(int argc, char *argv[])
{
  int ch;
  char *c_dir=NULL, *l_file=NULL, *sws_dir=NULL,
       *i_address = "0.0.0.0";
  int p_port = 8080;

  // printf("argc: %d  argv: %s\n", argc, *argv);

  while ((ch = getopt(argc, argv, "-dhcilp")) != -1) {
    switch (ch) {
    case 'd':
      flag_d = 1;
      break;

    case 'h':
      flag_h = 1;
      break;

    case 'c':
      c_dir = argv[optind];
      optind++;
      break;

    case 'i':
      i_address = argv[optind];
      optind++;
      break;

    case 'l':
      l_file = argv[optind];
      optind++;
      break;

    case 'p':
      p_port = atoi(argv[optind]);
      optind++;
      break;

    default:
    case '?':
      usage();
    }
  }
  argc -= optind;
  argv += optind;

  sws_dir = *argv;

  // printf("flag_d: %d, flag_h: %d\n", flag_d, flag_h);
  // printf("argc: %d  argv: %s\n", argc, *argv);
  // printf("c_dir: %s; i_address: %s; l_file: %s; p_port: %d; sws_dir: %s\n",
  //        c_dir, i_address, l_file, p_port, sws_dir);

  /* Options Validation Check */
  // if (c_dir != NULL)
  //   dircheck(c_dir);
  if (sws_dir != NULL)
    dircheck(sws_dir);

  startsws(i_address, p_port);

  return 0;
}