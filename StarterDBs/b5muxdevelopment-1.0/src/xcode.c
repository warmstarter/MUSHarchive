int IsIcReg();
int QuietIsIcReg();

int IsIcReg(player)
dbref player;
{


if(!(Flags3(player) & REGISTERED)) {
	notify(player,"Only registered players may do that");
	return 0;
	}
if(!(Flags3(player) & IC)) {
	notify(player,"Only IC players may do that");
	return 0;
	}
	return 1;
}


int QuietIsIcReg(player)
dbref player;
{
if(!(Flags3(player) & REGISTERED) || !(Flags3(player) & IC)) 
	return 0;

	return 1;
}


int xcode_parse();

int xcode_parse(buffer,args,maxargs)
char *buffer; 
char *args[]; 
int maxargs; 
{
  int count=0;
  char *parsed=buffer;
  int num_args=0;
  
  while((count < maxargs) && parsed) {
    if(!count) {
      /* first time through */
      parsed=strtok(buffer, " \t");
    } else {
      parsed=strtok(NULL, " \t");
    }
    args[count]=parsed;    /* Set the args pointer */
    if(parsed) num_args++; /* Actual count of arguments */
    count++;               /* Loop to make sure we don't overrun our */
                           /* buffer */
  }
  return (num_args);
}




