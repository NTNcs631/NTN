/* $NetBSD: net.h,v 1.07 2013/12/13 21:11:02 Weiyu Exp $ */
/* $NetBSD: net.h,v 1.05 2013/11/26 23:03:33 Lin Exp $ */
/* $NetBSD: net.h,v 1.06 2013/12/06 20:00:02 Qihuang Exp $ */
 
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
 #define INIT 0
/* Type */
#define SIMPLE 1
#define FULL 2
/* Method */
#define GET 1
#define HEAD 2
#define POST 3

#ifdef AF_INET6
  #define IPv6 1
#else 
  #define IPv6 0
#endif

#define OK 0
#define CREATED 1
#define ACCEPTED 2
#define NO_CONTENT 3
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
#define SIMPLE_RESPONSE 17

typedef struct {
  char *resource;
  char *text;
  int method;
  int status;
  int type;
}ReqInfo;

/* Define in net.c */
void startsws(char *, int, char *, char *, int, int);
int clientwrite(int, ReqInfo *, char *, char*);

/* Define in http.c */
void freereq(ReqInfo *);
void initreq(ReqInfo *);
void parsereq(unsigned char*, ReqInfo *);
