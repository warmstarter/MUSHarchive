#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef int dbref;

#include "mech.h"

void main() 
{
int fp;
dbref num;
struct mech_data data;
int done=0;
int datasize=sizeof(struct mech_data);

  fp=open("MECH", O_RDONLY, 0644);
  if(fp!=-1) {
    while(!done) {
      if(sizeof(dbref)==read(fp, (char *)&num, sizeof(dbref))) {
	if(datasize==read(fp, (char *)&data, datasize)) {
	  printf("KEY: %d\n", num);
	  printf("Facing: %d\n----------\n", data.facing);
	} else {
	  done=1;
	}
      } else {
	done=1;
      }
    }
    close(fp);
  } else {
    printf("Unable to open file!");
  }
}
  
