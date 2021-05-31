/*
   ** ident.c   High-level calls to the ident lib
   **
   ** Author: Pär Emanuelsson <pell@lysator.liu.se>
   ** Hacked by: Peter Eriksson <pen@lysator.liu.se>
 */

#include "config.h"
#ifdef NeXT3
#include <libc.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef I_TIME
#include <time.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#ifdef I_SYS_WAIT
#include <sys/wait.h>
#endif
#ifdef I_ERRNO
#include <errno.h>
#else
#ifdef I_SYS_ERRNO
#include <sys/errno.h>
#endif
#endif
#ifdef WIN32
#define FD_SETSIZE 256
#include <winsock.h>
#include <io.h>
#undef OPAQUE			/* Clashes with flags.h */
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32
#else				/* WIN32 */
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
#endif				/* WIN32 */

#ifdef I_UNISTD
#include <unistd.h>
#endif

#include "conf.h"
#include "ident.h"
#include "mymalloc.h"
#include "externs.h"
#include "confmagic.h"


/* Do a complete ident query and return result */

IDENT *ident_lookup
#ifdef CAN_NEWSTYLE
 (int fd,
  int timeout)
#else
 (fd, timeout)
    int fd;
    int timeout;
#endif
{
  struct sockaddr_in localaddr, remoteaddr;
#ifdef macintosh
  unsigned int len;
#else
  int len;
#endif

  len = sizeof(remoteaddr);
  if (getpeername(fd, (struct sockaddr *) &remoteaddr, &len) < 0)
    return 0;

  len = sizeof(localaddr);
  if (getsockname(fd, (struct sockaddr *) &localaddr, &len) < 0)
    return 0;

  return ident_query(&localaddr.sin_addr, &remoteaddr.sin_addr,
		   ntohs(localaddr.sin_port), ntohs(remoteaddr.sin_port),
		     timeout);
}


IDENT *ident_query
#ifdef CAN_NEWSTYLE
 (__STRUCT_IN_ADDR_P laddr,
  __STRUCT_IN_ADDR_P raddr,
  int lport,
  int rport,
  int timeout)
#else
 (laddr, raddr, lport, rport, timeout)
    __STRUCT_IN_ADDR_P laddr;
    __STRUCT_IN_ADDR_P raddr;
    int lport;
    int rport;
    int timeout;
#endif
{
  int res;
  ident_t *id;
  struct timeval timout;
  IDENT *ident = 0;


  timout.tv_sec = timeout;
  timout.tv_usec = 0;

  if (timeout)
    id = id_open(laddr, raddr, &timout);
  else
    id = id_open(laddr, raddr, (struct timeval *) 0);

  if (!id) {
#ifndef WIN32
    errno = EINVAL;
#endif
    return 0;
  }
  if (timeout)
    res = id_query(id, rport, lport, &timout);
  else
    res = id_query(id, rport, lport, (struct timeval *) 0);

  if (res < 0) {
    id_close(id);
    return 0;
  }
  ident = (IDENT *) malloc(sizeof(IDENT));
  if (!ident) {
    id_close(id);
    return 0;
  }
  if (timeout)
    res = id_parse(id, &timout,
		   &ident->lport,
		   &ident->fport,
		   &ident->identifier,
		   &ident->opsys,
		   &ident->charset);
  else
    res = id_parse(id, (struct timeval *) 0,
		   &ident->lport,
		   &ident->fport,
		   &ident->identifier,
		   &ident->opsys,
		   &ident->charset);

  if (res != 1) {
    free(ident);
    id_close(id);
    return 0;
  }
  id_close(id);
  return ident;			/* At last! */
}

char *ident_id
#ifdef CAN_NEWSTYLE
 (int fd,
  int timeout)
#else
 (fd, timeout)
    int fd;
    int timeout;
#endif
{
  IDENT *ident;
  char *id = 0;

  ident = ident_lookup(fd, timeout);
  if (ident && ident->identifier && *ident->identifier)
    id = strdup(ident->identifier);
  ident_free(ident);
  return id;
}

void ident_free
#ifdef CAN_NEWSTYLE
 (IDENT * id)
#else
 (id)
    IDENT *id;
#endif
{
  if (!id)
    return;
  if (id->identifier)
    free(id->identifier);
  if (id->opsys)
    free(id->opsys);
  if (id->charset)
    free(id->charset);
  free(id);
}

/*
   ** id_open.c                 Establish/initiate a connection to an IDENT server
   **
   ** Author: Peter Eriksson <pen@lysator.liu.se>
   ** Fixes: Pär Emanuelsson <pell@lysator.liu.se>
 */



static char *xmemset
#ifdef CAN_NEWSTYLE
 (char *buf, char val, int len)
#else
 (buf, val, len)
    char *buf;
    char val;
    int len;
#endif
{
  char *cp;

  cp = buf;
  while (len-- > 0)
    *cp++ = val;

  return buf;
}


ident_t *id_open
#ifdef CAN_NEWSTYLE
 (__STRUCT_IN_ADDR_P laddr, __STRUCT_IN_ADDR_P faddr, __STRUCT_TIMEVAL_P timeout)
#else
 (laddr, faddr, timeout)
    __STRUCT_IN_ADDR_P laddr;
    __STRUCT_IN_ADDR_P faddr;
    __STRUCT_TIMEVAL_P timeout;
#endif
{
  ident_t *id;
  int res;
#ifndef WIN32
  int tmperrno;
#endif
  struct sockaddr_in sin_laddr, sin_faddr;
  fd_set rs, ws, es;
#ifndef OLD_SETSOCKOPT
  int on = 1;
  struct linger linger;
#endif

  if ((id = (ident_t *) malloc(sizeof(*id))) == 0)
    return 0;

  if ((id->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    free(id);
    return 0;
  }
  if (timeout) {
#ifndef WIN32
    if ((res = fcntl(id->fd, F_GETFL, 0)) < 0)
      goto ERROR_BRANCH;

    if (fcntl(id->fd, F_SETFL, res | O_NDELAY) < 0)
      goto ERROR_BRANCH;
#endif
  }
  /* We silently ignore errors if we can't change LINGER */
#ifdef OLD_SETSOCKOPT
  /* Old style setsockopt() */
  (void) setsockopt(id->fd, SOL_SOCKET, SO_DONTLINGER);
  (void) setsockopt(id->fd, SOL_SOCKET, SO_REUSEADDR);
#else
  /* New style setsockopt() */
  linger.l_onoff = 0;
  linger.l_linger = 0;

  (void) setsockopt(id->fd, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger));
  (void) setsockopt(id->fd, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on));
#endif

  id->buf[0] = '\0';

  xmemset((char *) &sin_laddr, 0, sizeof(sin_laddr));
  sin_laddr.sin_family = AF_INET;
  sin_laddr.sin_addr = *(struct in_addr *) laddr;
  sin_laddr.sin_port = 0;

  if (bind(id->fd, (struct sockaddr *) &sin_laddr, sizeof(sin_laddr)) < 0) {
#ifdef DEBUG
    perror("libident: bind");
#endif
    goto ERROR_BRANCH;
  }
  xmemset((char *) &sin_faddr, 0, sizeof(sin_faddr));
  sin_faddr.sin_family = AF_INET;
  sin_faddr.sin_addr = *(struct in_addr *) faddr;
  sin_faddr.sin_port = htons(IDPORT);

#ifndef WIN32
  errno = 0;
#endif
  res = connect(id->fd, (struct sockaddr *) &sin_faddr, sizeof(sin_faddr));
  if (res < 0
#ifndef WIN32
      && errno != EINPROGRESS
#endif
    ) {
#ifdef DEBUG
    perror("libident: connect");
#endif
    goto ERROR_BRANCH;
  }
  if (timeout) {
    FD_ZERO(&rs);
    FD_ZERO(&ws);
    FD_ZERO(&es);
    FD_SET(id->fd, &rs);
    FD_SET(id->fd, &ws);
    FD_SET(id->fd, &es);

    if ((res = select(FD_SETSIZE, &rs, &ws, &es, timeout)) < 0) {
#ifdef DEBUG
      perror("libident: select");
#endif
      goto ERROR_BRANCH;
    }
    if (res == 0) {
#ifndef WIN32
      errno = ETIMEDOUT;
#endif
      goto ERROR_BRANCH;
    }
    if (FD_ISSET(id->fd, &es))
      goto ERROR_BRANCH;

    if (!FD_ISSET(id->fd, &rs) && !FD_ISSET(id->fd, &ws))
      goto ERROR_BRANCH;
  }
  return id;

ERROR_BRANCH:
#ifndef WIN32
  tmperrno = errno;		/* Save, so close() won't erase it */
#endif
  close(id->fd);
  free(id);
#ifndef WIN32
  errno = tmperrno;
#endif
  return 0;
}


/*
   ** id_close.c                            Close a connection to an IDENT server
   **
   ** Author: Peter Eriksson <pen@lysator.liu.se>
 */

int id_close
#ifdef CAN_NEWSTYLE
 (ident_t * id)
#else
 (id)
    ident_t *id;
#endif
{
  int res;

  res = close(id->fd);
  free(id);

  return res;
}


/*
   ** id_query.c                             Transmit a query to an IDENT server
   **
   ** Author: Peter Eriksson <pen@lysator.liu.se>
   ** Slight modifications by Alan Schwartz
 */



int id_query _((ident_t * id, int lport, int fport, __STRUCT_TIMEVAL_P timeout));
int id_query
#ifdef CAN_NEWSTYLE
 (ident_t * id, int lport, int fport, __STRUCT_TIMEVAL_P timeout)
#else
 (id, lport, fport, timeout)
    ident_t *id;
    int lport;
    int fport;
    __STRUCT_TIMEVAL_P timeout;
#endif
{
#ifndef WIN32
  Signal_t(*old_sig) ();
#endif
  int res;
  char buf[80];
  fd_set ws;

  sprintf(buf, "%d , %d\r\n", lport, fport);

  if (timeout) {
    FD_ZERO(&ws);
    FD_SET(id->fd, &ws);

    if ((res = select(FD_SETSIZE,
		      (fd_set *) 0, &ws,
		      (fd_set *) 0,
		      timeout)) < 0)
      return -1;

    if (res == 0) {
#ifndef WIN32
      errno = ETIMEDOUT;
#endif
      return -1;
    }
  }
#ifndef WIN32
  old_sig = signal(SIGPIPE, SIG_IGN);
#endif

  res = write(id->fd, buf, strlen(buf));

#ifndef WIN32
  signal(SIGPIPE, old_sig);
#endif

  return res;
}


/*
   ** id_parse.c                    Receive and parse a reply from an IDENT server
   **
   ** Author: Peter Eriksson <pen@lysator.liu.se>
   ** Fiddling: Pär Emanuelsson <pell@lysator.liu.se>
 */

static char *xstrtok
#ifdef CAN_NEWSTYLE
 (char *cp, char *cs, char *dc)
#else
 (cp, cs, dc)
    char *cp;
    char *cs;
    char *dc;
#endif
{
  static char *bp = 0;

  if (cp)
    bp = cp;

  /*
     ** No delimitor cs - return whole buffer and point at end
   */
  if (!cs) {
    while (*bp)
      bp++;
    return cs;
  }
  /*
     ** Skip leading spaces
   */
  while (isspace(*bp))
    bp++;

  /*
     ** No token found?
   */
  if (!*bp)
    return 0;

  cp = bp;
  while (*bp && !strchr(cs, *bp))
    bp++;

  /*
     ** Remove trailing spaces
   */
  *dc = *bp;
  for (dc = bp - 1; dc > cp && isspace(*dc); dc--) ;
  *++dc = '\0';

  bp++;

  return cp;
}


int id_parse
#ifdef CAN_NEWSTYLE
 (ident_t * id,
  __STRUCT_TIMEVAL_P timeout,
  int *lport,
  int *fport,
  char **identifier,
  char **opsys,
  char **charset)
#else
 (id, timeout, lport, fport, identifier, opsys, charset)
    ident_t *id;
    __STRUCT_TIMEVAL_P timeout;
    int *lport;
    int *fport;
    char **identifier;
    char **opsys;
    char **charset;
#endif
{
  char c, *cp, *tmp_charset;
  fd_set rs;
  int res = 0, lp, fp;
  Size_t pos;

#ifndef WIN32
  errno = 0;
#endif

  tmp_charset = 0;

  if (!id)
    return -1;
  if (lport)
    *lport = 0;
  if (fport)
    *fport = 0;
  if (identifier)
    *identifier = 0;
  if (opsys)
    *opsys = 0;
  if (charset)
    *charset = 0;

  pos = strlen(id->buf);

  if (timeout) {
    FD_ZERO(&rs);
    FD_SET(id->fd, &rs);

    if ((res = select(FD_SETSIZE,
		      &rs,
		      (fd_set *) 0,
		      (fd_set *) 0,
		      timeout)) < 0)
      return -1;

    if (res == 0) {
#ifndef WIN32
      errno = ETIMEDOUT;
#endif
      return -1;
    }
  }
  while (pos < sizeof(id->buf) &&
	 (res = read(id->fd, id->buf + pos, 1)) == 1 &&
	 id->buf[pos] != '\n')
    pos++;

  if (res < 0)
    return -1;

  if (res == 0) {
#ifndef WIN32
    errno = ENOTCONN;
#endif
    return -1;
  }
  if (id->buf[pos] != '\n')
    return 0;

  id->buf[pos++] = '\0';

  /*
     ** Get first field (<lport> , <fport>)
   */
  cp = xstrtok(id->buf, (char *) ":", &c);
  if (!cp)
    return -2;

  if (sscanf(cp, " %d , %d", &lp, &fp) != 2) {
    if (identifier)
      *identifier = strdup(cp);
    return -2;
  }
  if (lport)
    *lport = lp;
  if (fport)
    *fport = fp;

  /*
     ** Get second field (USERID or ERROR)
   */
  cp = xstrtok((char *) 0, (char *) ":", &c);
  if (!cp)
    return -2;

  if (strcmp(cp, "ERROR") == 0) {
    cp = xstrtok((char *) 0, (char *) "\n\r", &c);
    if (!cp)
      return -2;

    if (identifier)
      *identifier = strdup(cp);

    return 2;
  } else if (strcmp(cp, "USERID") == 0) {
    /*
       ** Get first subfield of third field <opsys>
     */
    cp = xstrtok((char *) 0, (char *) ",:", &c);
    if (!cp)
      return -2;

    if (opsys)
      *opsys = strdup(cp);

    /*
       ** We have a second subfield (<charset>)
     */
    if (c == ',') {
      cp = xstrtok((char *) 0, (char *) ":", &c);
      if (!cp)
	return -2;

      tmp_charset = cp;
      if (charset)
	*charset = strdup(cp);

      /*
         ** We have even more subfields - ignore them
       */
      if (c == ',')
	xstrtok((char *) 0, (char *) ":", &c);
    }
    if (tmp_charset && strcmp(tmp_charset, "OCTET") == 0)
      cp = xstrtok((char *) 0, (char *) 0, &c);
    else
      cp = xstrtok((char *) 0, (char *) "\n\r", &c);

    if (identifier)
      *identifier = strdup(cp);
    return 1;
  } else {
    if (identifier)
      *identifier = strdup(cp);
    return -3;
  }
}
