/*
   Copyright (C) 1991, Marcus J. Ranum. All rights reserved.
 */


/*
   code to interface client MUDs with rwho server
   this is a standalone library.
 */

#include "config.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<fcntl.h>
#ifdef WIN32
#define NO_HUGE_RESOLVER_CODE
#define FD_SETSIZE 256
#include <winsock.h>
#include <io.h>
#undef OPAQUE			/* Clashes with flags.h */
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32
#else				/* WIN32 */
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#ifdef I_SYS_SOCKET
#include <sys/socket.h>
#endif
#ifdef I_SYS_FILE
#include <sys/file.h>
#endif
#ifdef I_NETINET_IN
#include <netinet/in.h>
#endif
#ifdef I_ARPA_INET
#include <arpa/inet.h>
#endif
#include	<netdb.h>
#endif				/* WIN32 */
#include	<sys/types.h>
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#include "conf.h"
#include "mymalloc.h"
#include "confmagic.h"

#ifndef INADDR_NONE
/* This is a bad value because 255.255.255.255 resolves to -1,
 * but this is what the manpage says it's normally defiend as.
 * A better choice would be to use inet_aton(), but I don't think
 * rwho is important enough to change.
 */
#define INADDR_NONE -1
#endif

#define	DGRAMPORT		6888
#ifndef	NO_HUGE_RESOLVER_CODE
extern struct hostent *gethostbyname();
#endif

static int dgramfd = -1;
static char *password;
static char *localnam;
static char *lcomment;
static struct sockaddr_in addr;
time_t senttime;

int rwhocli_setup _((const char *server, const char *serverpw, const char *myname, const char *comment));
int rwhocli_shutdown _((void));
int rwhocli_userlogin _((const char *uid, const char *name, time_t tim));
int rwhocli_userlogout _((const char *uid));
int rwhocli_pingalive _((void));


/* enable RWHO and send the server a "we are up" message */
int
rwhocli_setup(server, serverpw, myname, comment)
    const char *server;
    const char *serverpw;
    const char *myname;
    const char *comment;
{
#ifndef	NO_HUGE_RESOLVER_CODE
  struct hostent *hp;
#endif
  char pbuf[512];
  const char *p;

  if (dgramfd != -1)
    return (1);

  password = (char *) malloc(strlen(serverpw) + 1);
  localnam = (char *) malloc(strlen(myname) + 1);
  lcomment = (char *) malloc(strlen(comment) + 1);
  if (!password || !localnam || !lcomment)
    return (1);
  strcpy(password, serverpw);
  strcpy(localnam, myname);
  strcpy(lcomment, comment);

  p = server;
  while (*p != '\0' && (*p == '.' || isdigit(*p)))
    p++;

  if (*p != '\0') {
#ifndef	NO_HUGE_RESOLVER_CODE
    if ((hp = gethostbyname(server)) == (struct hostent *) 0)
      return (1);
    (void) bcopy(hp->h_addr, (char *) &addr.sin_addr, hp->h_length);
#else
    return (1);
#endif
  } else {
    unsigned long f;

    if ((f = inet_addr(server)) == INADDR_NONE)
      return (1);
    (void) bcopy((char *) &f, (char *) &addr.sin_addr, sizeof(f));
  }

  addr.sin_port = htons(DGRAMPORT);
  addr.sin_family = AF_INET;

  if ((dgramfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    return (1);

  time(&senttime);

  sprintf(pbuf, "U\t%.20s\t%.20s\t%.20s\t%.10d\t0\t%.25s",
	  localnam, password, localnam, (int) senttime, comment);
  sendto(dgramfd, pbuf, strlen(pbuf), 0, (struct sockaddr *) &addr, sizeof(addr));
  return (0);
}





/* disable RWHO */
int
rwhocli_shutdown()
{
  char pbuf[512];

  if (dgramfd != -1) {
    sprintf(pbuf, "D\t%.20s\t%.20s\t%.20s", localnam, password, localnam);
    sendto(dgramfd, pbuf, strlen(pbuf), 0, (struct sockaddr *) &addr, sizeof(addr));
    close(dgramfd);
    dgramfd = -1;
    free(password);
    free(localnam);
  }
  return (0);
}





/* send an update ping that we're alive */
int
rwhocli_pingalive()
{
  char pbuf[512];

  if (dgramfd != -1) {
    sprintf(pbuf, "M\t%.20s\t%.20s\t%.20s\t%.10d\t0\t%.25s",
	    localnam, password, localnam, (int) senttime, lcomment);
    sendto(dgramfd, pbuf, strlen(pbuf), 0, (struct sockaddr *) &addr, sizeof(addr));
  }
  return (0);
}





/* send a "so-and-so-logged in" message */
int
rwhocli_userlogin(uid, name, tim)
    const char *uid;
    const char *name;
    time_t tim;
{
  char pbuf[512];

  if (dgramfd != -1) {
    sprintf(pbuf, "A\t%.20s\t%.20s\t%.20s\t%.20s\t%.10d\t0\t%.20s",
	    localnam, password, localnam, uid, (int) tim, name);
    sendto(dgramfd, pbuf, strlen(pbuf), 0, (struct sockaddr *) &addr, sizeof(addr));
  }
  return (0);
}





/* send a "so-and-so-logged out" message */
int
rwhocli_userlogout(uid)
    const char *uid;
{
  char pbuf[512];

  if (dgramfd != -1) {
    sprintf(pbuf, "Z\t%.20s\t%.20s\t%.20s\t%.20s",
	    localnam, password, localnam, uid);
    sendto(dgramfd, pbuf, strlen(pbuf), 0, (struct sockaddr *) &addr, sizeof(addr));
  }
  return (0);
}
