
#include <stdio.h>

main(argc, argv)
    int argc;
    char *argv[];
{
  char buff[BUFFER_LEN];
  int child;

  FILE *fi = fopen(argv[1], "r+");
  if (!(child = fork())) {
    int a;
    while (-1 != (a = read(fileno(fi), buff, 1023))) {
      if (!a) {
	sleep(1);
	continue;
      }
      write(fileno(stdout), buff, a);
    }
    exit(1);
  }
  while (fgets(buff, 1023, stdin)) {
    fprintf(fi, "%s", buff);
    fflush(fi);
  }
  kill(child, 9);
}
