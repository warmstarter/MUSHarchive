/* info_slave.c */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#ifdef I_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_MEMORY
#include <memory.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef WIN32
#define FD_SETSIZE 256
#include <winsock.h>
#include <io.h>
#undef OPAQUE			/* Clashes with flags.h */
#define EINTR WSAEINTR
#define EMFILE WSAEMFILE
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32
#include <time.h>
#else				/* WIN32 */
#ifdef I_SYS_FILE
#include <sys/file.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#include <sys/ioctl.h>
#ifdef I_ERRNO
#include <errno.h>
#else
#ifdef I_SYS_ERRNO
#include <sys/errno.h>
#endif
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef I_SYS_PARAM
#include <sys/param.h>
#endif
#endif				/* WIN32 */
#ifdef I_SYS_WAIT
#include <sys/wait.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_SELECT
#include <sys/select.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef HAS_GETRLIMIT
#include <sys/resource.h>
#endif
#ifdef I_LIMITS
#include <limits.h>
#endif
#ifdef I_ARPA_INET
#include <arpa/inet.h>
#endif

#include "conf.h"
#include "ident.h"
#include "confmagic.h"

/* Win32 uses closesocket() to close a socket, and so will we */
#ifndef WIN32
#define closesocket(s)  close(s)
#endif

int
main(argc, argv)
    int argc;
    char *argv[];
{
  int mush;
  int port;
  int fd;
  struct sockaddr_in local, remote;
  static char buf[10000];	/* overkill */
  int len, size;
  IDENT *ident_result;
  struct hostent *he;
  int use_ident;

  if (argc < 2) {
    fprintf(stderr, "info_slave needs port number!\n");
    exit(1);
  }
  sscanf(argv[1], "%d", &port);
  use_ident = 1;
  if (argc >= 3) {
    /* The second argument is 'noident' if we don't want ident used.
     * Anything else, or no argument present, and we default to ident
     */
    if (!strcmp(argv[2], "noident"))
      use_ident = 0;
  }
  mush = socket(AF_INET, SOCK_STREAM, 0);
  if (mush < 0) {
    perror("creating info slave connect stream socket");
    exit(3);
  }
  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = inet_addr("127.0.0.1");
  remote.sin_port = htons(port);
  if (connect(mush, (struct sockaddr *) &remote, sizeof(remote))) {
    perror("connecting stream socket");
    closesocket(mush);
    exit(4);
  }
  /* yes, we are _blocking_ */

  for (;;) {
    /* grab a request */
#ifdef WIN32
    len = recv(mush, (char *) &remote.sin_addr, sizeof(remote.sin_addr), 0);
#else
    len = read(mush, (char *) &remote.sin_addr, sizeof(remote.sin_addr));
#endif
    if ((Size_t) len < sizeof(remote.sin_addr)) {
      perror("info_slave reading remote.sin_addr");
      exit(1);
    }
#ifdef WIN32
    len = recv(mush, (char *) &remote.sin_port, sizeof(remote.sin_port), 0);
#else
    len = read(mush, (char *) &remote.sin_port, sizeof(remote.sin_port));
#endif
    if ((Size_t) len < sizeof(remote.sin_port)) {
      perror("info_slave reading remote.sin_port");
      exit(1);
    }
#ifdef WIN32
    len = recv(mush, (char *) &local.sin_addr, sizeof(local.sin_addr), 0);
#else
    len = read(mush, (char *) &local.sin_addr, sizeof(local.sin_addr));
#endif
    if ((Size_t) len < sizeof(local.sin_addr)) {
      perror("info_slave reading local.sin_addr");
      exit(1);
    }
#ifdef WIN32
    len = recv(mush, (char *) &local.sin_port, sizeof(local.sin_port), 0);
#else
    len = read(mush, (char *) &local.sin_port, sizeof(local.sin_port));
#endif
    if ((Size_t) len < sizeof(local.sin_port)) {
      perror("info_slave reading local.sin_port");
      exit(1);
    }
#ifdef WIN32
    len = recv(mush, (char *) &fd, sizeof(int), 0);
#else
    len = read(mush, (char *) &fd, sizeof(int));
#endif
    if ((Size_t) len < sizeof(int)) {
      perror("info_slave reading fd");
      exit(1);
    }
    if (!fd)
      /* MUSH aborted query part way through */
      continue;

    memcpy(buf, &fd, sizeof(fd));
    size = sizeof(fd) + sizeof(int);
    strncpy(buf + size, inet_ntoa(remote.sin_addr), sizeof(buf) - size);
    buf[sizeof(buf) - 1] = '\0';
    while (isprint(buf[size]))
      size++;
    buf[size++] = ':';
    buf[size] = '\0';
    if (use_ident) {
      ident_result = ident_query(&local.sin_addr, &remote.sin_addr,
			   ntohs(local.sin_port), ntohs(remote.sin_port),
				 5);
      if (ident_result && ident_result->identifier) {
	strncpy(buf + size, ident_result->identifier, sizeof(buf) - size);
	buf[sizeof(buf) - 1] = '\0';
	size = sizeof(fd) + sizeof(int);
	while (isprint(buf[size]))
	  size++;
	buf[size++] = '@';
      }
      ident_free(ident_result);
    }
    buf[size] = '\0';
    he = gethostbyaddr((char *) &remote.sin_addr.s_addr,
		       sizeof(remote.sin_addr.s_addr), AF_INET);
    if (!he)
      strncpy(buf + size, inet_ntoa(remote.sin_addr), sizeof(buf) - size);
    else
      strncpy(buf + size, he->h_name, sizeof(buf) - size);
    buf[sizeof(buf) - 1] = '\0';
    size = strlen(buf + sizeof(fd) + sizeof(int));
    memcpy(buf + sizeof(fd), &size, sizeof(int));
    size += sizeof(fd) + sizeof(int);

#ifdef WIN32
    len = send(mush, buf, size, 0);
#else
    len = write(mush, buf, size);
#endif
    if (len < size) {
      perror("info_slave write packet");
      exit(1);
    }
  }
}
