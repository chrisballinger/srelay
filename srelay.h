/*
  srelay.h:
         common definitions.

Copyright (C) 2001 Tomo.M (author).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the author nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef SOLARIS
#define BSD_COMP
# ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h>
# endif
# ifdef HAVE_RESOLV_H
#  include <resolv.h>
# endif
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#endif

#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#define version  "srelay 0.3 2002/06/14 (Tomo.M)"

#ifndef SYSCONFDIR
# define SYSCONFDIR "/usr/local/etc"
#endif
#define CONFIG    SYSCONFDIR "/srelay.conf"
#define PWDFILE   SYSCONFDIR "/srelay.passwd"
#define PIDFILE   "/var/run/srelay.pid"
#define WORKDIR0  "/var/run"
#define WORKDIR1  "/var/tmp"

#define S4DEFUSR  "user"

#define BUFSIZE    8192

#define PROCUID  65534
#define PROCGID  65534

#ifdef SOLARIS
# undef PROCUID
# undef PROCGID
# define PROCUID  60001
# define PROCGID  60001
#endif

#ifdef FD_SETSIZE
#define MAX_FD FD_SETSIZE
#else
#define MAX_FD 1024
#endif

/* Fixed maximum proxy route entry */
#define MAX_ROUTE  256

/* Fixed maximum listen sockets */
#define MAX_SOCKS  256

/* default socks port */
#define SOCKS_PORT    1080

/* idle timeout minutes 0 = never timeout */
#define IDLE_TIMEOUT  0

/* default maximum number of child process */
#define MAX_CHILD     100

/* Solaris did not define this */
#ifndef IPPORT_RESERVEDSTART
# define IPPORT_RESERVEDSTART 600
#endif

#ifdef USE_THREAD
# include <pthread.h>
typedef struct {
  pthread_t     tid;
  unsigned long count;
} thread_tab;

extern thread_tab *t_t;
extern pthread_t main_thread;  /* holding the main thread ID */
extern pthread_mutex_t mutex_select;
extern pthread_mutex_t mutex_gh0;
extern int threading;
#endif

#ifdef USE_THREAD
# define MUTEX_LOCK(mutex) \
    if (threading) { \
      pthread_mutex_lock(&mutex); \
    }
# define MUTEX_UNLOCK(mutex) \
    if (threading) { \
      pthread_mutex_unlock(&mutex); \
    }
#else
# define MUTEX_LOCK(mutex)
# define MUTEX_UNLOCK(mutex)
#endif

#ifdef USE_THREAD
# if (MAX_FD > 22)
#   define THREAD_LIMIT   (MAX_FD - 20)/2
# else
#   define THREAD_LIMIT    1     /* wooo !!! */
# endif
# define MAX_THREAD (THREAD_LIMIT > 64 ? 64 : THREAD_LIMIT)    
#endif

enum { norm=0, warn, crit };

/*  address types */
#define S5ATIPV4    1
#define S5ATFQDN    3
#define S5ATIPV6    4

#define S4ATIPV4    1
#define S4ATFQDN    3

/* authentication  methods */
#define S5ANOAUTH     0
#define S5AGSSAPI     1
#define S5AUSRPAS     2
#define S5ACHAP       3
#define S5ANOTACC     0xff

/*
  struct rtbl:
        atype:  address type
        dest:   destination host or network.
	mask:   destination address mask;
	        dest.s_addr = 0 means 'any' IP address. so that
	        mask is ignored (zeroed forcibly).
	len:    domain name string length (atype = S5ATFQDN);
	domain: destination by domainname (atype = S5ATFQDN)
	port_l: lower port of destination port range.
	port_h: higher port of destination port range.
	proxy:  next hop proxy server.
	port:   next hop proxy server port.
	        proxy.s_addr = 0 means direct connect.
	        if no 'proxy' specified, try direct connect to dest.

        note -  port numbers are stored in 'host-byte-order'.
*/

struct rtbl {
  int    atype;
  struct in_addr dest;
  struct in_addr mask;
  int    len;
  char   *domain;
  u_short port_l;
  u_short port_h;
  struct in_addr proxy;
  u_short port;
};

#ifndef SIGFUNC_DEFINED
typedef void            (*sigfunc_t)();
#endif

#ifndef MAX
# define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef __P
#if defined(__STDC__) || defined(__cplusplus)
#define __P(protos)     protos          /* full-blown ANSI C */
#else   /* !(__STDC__ || __cplusplus) */
#define __P(protos)     ()              /* traditional C preprocessor */
#endif
#endif

/*
 *   Externals.
 */

/* from main.c */
extern char *config;
extern char *ident;
extern char *pidfile;
extern char *pwdfile;
extern int max_child;
extern int cur_child;
extern char method_tab[];
extern int method_num;
extern int bind_restrict;

/* from init.c */
extern char **str_serv_sock;
extern int *serv_sock;
extern int serv_sock_ind;
extern int maxsock;
extern fd_set allsock;

/* from readconf.c */
extern struct rtbl *proxy_tbl;
extern int proxy_tbl_ind;

/* from relay.c */
extern int resolv_client;
extern u_long   idle_timeout; 

/* from util.c */
extern int forcesyslog;

/* from socks.c */

/* from auth-pwd.c */
extern char *pwdfile;

/*
 *   external functions
 */

/* init.c */
extern int serv_init __P((char *));

/* main.c */

/* readconf.c */
extern int readconf __P((FILE *));
extern int readpasswd __P((FILE *, int, char *, int, char *, int));

/* relay.c */
extern int serv_loop __P((void *));

/* socks.c */
int wait_for_read __P((int, long));
ssize_t timerd_read __P((int, char *, size_t, int));
ssize_t timerd_write __P((int, char *, size_t, int));
extern int proto_socks __P((int));

/* get-bind.c */
int get_bind_addr __P((struct in_addr *, struct in_addr * ));

/* util.c */
extern void msg_out __P((int, const char *, ...));
extern void set_blocking __P((int));
extern int settimer __P((int));
extern void timeout __P((int));
extern void reapchild __P((int));
extern void cleanup __P((int));
extern void reload __P((int));
extern sigfunc_t setsignal __P((int, sigfunc_t));
extern int blocksignal __P((int));
extern int releasesignal __P((int));
#ifndef HAVE_INET_PTON
extern int inet_pton __P((int, char *, void *));
#endif

/* auth-pwd.c */
int auth_pwd_server __P((int));
int auth_pwd_client __P((int, int));
