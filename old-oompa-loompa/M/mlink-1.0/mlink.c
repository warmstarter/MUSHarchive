/* $Id: mlink.c,v 1.15 1996/07/19 20:16:31 tf2005 Exp $ */ 
#undef __vax     /* This line may not be needed on some machines */

#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

char *strtok();

typedef struct {
    char *hostname;
    char *hostip;
    int port;
    char *charname;
    char *password;
/* here we define the type of mush; this determines the type of output
 * in some cases. The number is set in the link.worlds file, and should
 * be one digit after the port as follows:
 *	0 - PennMUSHes
 *	1 - TinyMUSHes 
 *	2 - DarkZone
 *	3 - TinyMUX
 *	4 - MUSE
 */
    int mushtype;
/* We're going to add support for mailtype so we don't have to define a
 * new mushtype for each kind of mail system
 *	0 - Penn extended mailer with subject (@mail person=subj/msg)
 *	1 - Penn basic @mail (@mail person=msg)
 *	2 - Tiny basic +mail (+mail person=msg)
 *	3 - Tiny BrandyMailer (+mail person=subj;-msg;+send)
 *	4 - Tiny SeawolfMailer (+smail person=msg)
 *	5 - MUX @mailer (@mail person=subj;-msg;@mail/send)
 */
    int mailtype;
/* Pemittype will allow us to tailor output nicely. 
 * 	0 - PennMUSH standard: @pemit + lit()
 *	1 - TinyMUSH standard: @pemit, no lit()
 *	2 - MUX/MUSE/DZ standard: @npemit
 */
    int pemittype;
    int socket;
    int status;
    time_t upsince;
    time_t idle;

} host_type;

#define VERSION "1.0"		/* the version number of MushLink */
#define RLINKNAME "MushLink"	/* The name of the robot character */
#define MAXHOSTS 90		/* Maximum number of worlds */

/* Define the MU* server commands the robot needs to use 
 *
 * Please be sure to put in the associated softcode commands
 * included in mlink.txt before running this robot on
 * that MUSH. 
 *
 */

#define PEMITCMD "@pemit"
#define DZPEMIT "@npemit"

#define WFILE "mlink.config"	/* Configuration file for rlink */

/* Modify the first 5 fields of each line with appropriate host information */
/* but leave the last four fields alone.  Add additional lines as needed.   */

host_type hosts[MAXHOSTS] = {
    NULL, NULL, 0, NULL, NULL, 0, 0, 0, 0, 0, 0
  };


#define DISCONNECT "QUIT\n"

/* Replace 'God' here with the owner's name (and make sure all rlink
 * characters are pagelocked to the appropriate characters)
 */
 
#define DISCMSG "God pages: Shutdown operations"
#define SUSPMSG "God pages: Suspend operations"
#define RESUMEMSG "God pages: Resume operations"
#define RECONNMSG "God pages: Reconnect"

#define STAT_DEAD 0
#define STAT_PAUSE 1
#define STAT_OK 2
#define STAT_ON 3

#define SENDSYNCMSG "prtyxnonsense\n"
#define ACKSYNCMSG "Huh?  (Type \"help\" for help.)"

#define RWHOKEY  "RWHOrequest:"
#define RWHOKEYLEN  12

#define RFINGERKEY "RFINGERrequest:"
#define RFINGERKEYLEN 15

#define RINFOKEY "RINFOrequest:"
#define RINFOKEYLEN 13 

#define RPAGEKEY "RPAGErequest:"
#define RPAGEKEYLEN 13

#define RWORLDSKEY "RWORLDSrequest:"
#define RWORLDSKEYLEN 15

#define RMAILKEY "RMAILrequest:"
#define RMAILKEYLEN 13

#define INPREFIX "ghoti"
#define INPREFIXLEN 5

#define PREFIXTEXT	">>..RemoteLink PrefixText..<<"
#define SUFFIXTEXT	">>..RemoteLink SuffixText..<<"

#define SOFTPREFIX	"START"
#define SOFTSUFFIX	"END"

#define MAXSTRLEN 4096

#define RECON_TIME 300	/* Number of seconds to wait before reconnecting */

#define IBUF_LINES 50

/* Add elements at the tail, remove them from the head */
int ibufhead = 0, ibuftail = 0;
int ibufwrld[IBUF_LINES];
char ibuftxt[IBUF_LINES][MAXSTRLEN];

int num_worlds = 0;
int maxd = 0;
int debug_flg;

void do_alarm();

#define LAG_TIMEOUT 10
#define CONNECT_TIMEOUT 30

time_t timer;

/* The main program sets up the socket connection to the MU*s and
 * then calls link_worlds.
 */

main(argc,argv)
int argc;
char **argv;
{
    int i;
    int n = 0;

    if (argc > 1 && argv[1][0]=='-' && argv[1][1]=='d') {
	fprintf(stderr, "Dbg: Debugging on.\n");
	debug_flg++;
    }

    if (signal(SIGALRM, do_alarm) < 0) {
    	fprintf(stderr, "Error in signal()\n");
    	exit(0);
    }

    if ((n = load_worlds()) == 0) {
    	exit(0);
    } else
    	fprintf(stderr, "Init: Read in %d worlds\n", n);

   if(debug_flg)
    for (i=0; hosts[i].hostname != NULL; i++) {
      fprintf(stderr, "Init: Read world %s as %d\n",
		hosts[i].hostname, i);
    }

    for (i=0; hosts[i].hostname != NULL; i++) {
	if (!open_socket(i)) {
	    fprintf(stderr,"Sock: Can't connect to %s\n",hosts[i].hostname);
	    hosts[i].upsince = time(0);
	    hosts[i].status = STAT_DEAD;
	}
	else
	    hosts[i].status = STAT_OK;
    }

    for (i=0; hosts[i].hostname != NULL; i++) {
	if (hosts[i].status == STAT_OK)
	  log_in(i);
    }
    do_alarm();
    link_worlds();

    /* All worlds closed*/

    fprintf(stderr, "Exit: exiting, all worlds closed\n");
    exit(0);
}

/* The open_socket routine sets up a socket connection over the Internet
   to the MUD, using the appropriate Unix system incantations. */

int open_socket(host)
int host;
{
  struct sockaddr_in sin;
  int n;

  /* Set up socket and connect to designated host and port*/

  bzero((char *) &sin, sizeof(sin));        
  sin.sin_port = htons(hosts[host].port);
  sin.sin_addr.s_addr = inet_addr(hosts[host].hostip);
  sin.sin_family = AF_INET;
  hosts[host].socket = socket(AF_INET, SOCK_STREAM, 0);

fprintf(stderr,"Socket: %d",hosts[host].socket);

  if (hosts[host].socket < 0)
    return 0;

fprintf(stderr,"Sock: %d\n",hosts[host].socket);
n = connect(hosts[host].socket, (struct sockaddr *) &sin, sizeof(sin));

  if (n < 0) {
    fprintf(stderr,"Sock Connect: %d\n",n);
    close (hosts[host].socket);
    return 0;
  }

fprintf(stderr,"Sock Connect: %d\n",n);

  if (fcntl(hosts[host].socket, F_SETFL, O_NDELAY) == -1) {
    close (hosts[host].socket);
    return 0;
  }
  
  fprintf(stderr,"Sock: [%s] [%s %d] socket %d\n",
	  hosts[host].hostname, hosts[host].hostip,
	  hosts[host].port ,hosts[host].socket);
  hosts[host].status = STAT_OK;
  num_worlds++;
  if (hosts[host].socket > (maxd - 1)) {
  	maxd = hosts[host].socket + 1;
  }
  return 1;
}

/* Connects to a given world */

log_in(i)
int i;
{
    char buf[MAXSTRLEN];

    if (hosts[i].status == STAT_OK) {
	fcntl (hosts[i].socket, F_SETFL, FNDELAY);
	sleep(15);
	fprintf(stderr,"Conn: [%s] %s\n",
		hosts[i].hostname, hosts[i].charname);
	sprintf(buf,"connect %s %s\n", hosts[i].charname, hosts[i].password);
	send_host(i, buf);
 	wait(1);
        send_host(i, "\n\r");
	wait(1);
	send_host(i, "\n\r");
	if (sync_bot(i,buf)) {
	    sprintf(buf,"OUTPUTPREFIX %s\n", PREFIXTEXT);      
	    send_host(i,buf);
	    sprintf(buf,"OUTPUTSUFFIX %s\n", SUFFIXTEXT);      
	    send_host(i,buf);
	    hosts[i].upsince = time(0);
	    hosts[i].status = STAT_OK;
	    return(1);
	}
	else {
	    fprintf(stderr,"Conn: [%s] Error: problem synching robot.\n",
		    hosts[i].hostname);
	    close(hosts[i].socket);	    
	    hosts[i].status = STAT_DEAD;
	     hosts[i].upsince = time(0);
	    num_worlds--;
	    return(0);
	}
    }
}

/* The link_worlds routine is the main loop of the program. */

link_worlds()
{
    int wi;
    struct timeval tval;
    char inbuf[MAXSTRLEN];
    fd_set rfd, wfd;
    
    while (num_worlds > 0) {

	if (ibufhead != ibuftail) {
	    if (hosts[ibufwrld[ibufhead]].status != STAT_DEAD)
	      do_one_line(ibuftxt[ibufhead],ibufwrld[ibufhead]);
	    ibufhead = (ibufhead + 1) % IBUF_LINES;
	}
	else {
	    FD_ZERO(&rfd);
	    FD_ZERO(&wfd);

	    for (wi = 0; hosts[wi].hostname != NULL; wi++) {
		if (hosts[wi].status != STAT_DEAD) {
		    FD_SET(hosts[wi].socket, &rfd);
		    FD_SET(hosts[wi].socket, &wfd);
		}
	    }
	    tval.tv_sec=10;
	    tval.tv_usec=0;
	    
	    select(maxd, &rfd, (fd_set *)NULL, (fd_set *)NULL, &tval);

	    for (wi = 0; hosts[wi].hostname != NULL; wi++) {
		if (hosts[wi].status != STAT_DEAD) {
		    if ((getline(inbuf, wi) != 0) && (strlen(inbuf) > 0))
		      do_one_line(inbuf,wi);
		}
	    }

	}
    }
}


do_one_line(inbuf,wi)
char *inbuf;
int wi;
{
    int wo;

    char tmp1[MAXSTRLEN];
    char tmp2[MAXSTRLEN];

    if (inbuf[strlen(inbuf)-1] == '\n' || inbuf[strlen(inbuf)-1] == '\r')
	inbuf[strlen(inbuf)-1] = 0;

    if (debug_flg)
      fprintf(stderr,"In [%s]: %s\n",
	      hosts[wi].hostname, inbuf);

    if (strcmp(inbuf,DISCMSG)==0) {
	fprintf(stderr,"Conn: [%s] closing on command\n", hosts[wi].hostname);
        send_host(wi,"page God=Quitting\n");
	send_host(wi,DISCONNECT);
	close(hosts[wi].socket);
	hosts[wi].status = STAT_DEAD;
	num_worlds--;
	hosts[wi].upsince = time(0);
    }

    else if (strcmp(inbuf,SUSPMSG)==0) {
	fprintf(stderr,"Conn: [%s] suspending\n", hosts[wi].hostname);
        send_host(wi,"page God=Suspending\n");
	hosts[wi].status = STAT_PAUSE;

    }

    else if (strcmp(inbuf,RESUMEMSG)==0) {
	fprintf(stderr,"Conn: [%s] resuming\n", hosts[wi].hostname);
        send_host(wi,"page God=Resuming\n");
	hosts[wi].status = STAT_OK;
    }

    else if (strncmp(inbuf,RECONNMSG,strlen(RECONNMSG)) == 0) {
	for (wo=0; hosts[wo].hostname != NULL; wo++) {
	    if (substring(&(inbuf[strlen(RECONNMSG)+1]),
			  hosts[wo].hostname)) {
		if (hosts[wo].status != STAT_DEAD) {
            sprintf(tmp1,"page God=Already connected to %s\n",
			    hosts[wo].hostname);
		    send_host(wi,tmp1);
		}
		else if (open_socket(wo) && log_in(wo)) {
		    fprintf(stderr,
			    "Conn: [%s] Reconnecting on command from %s\n",
			    hosts[wo].hostname, hosts[wi].hostname);
            sprintf(tmp1,"page God=Ok, now connected to %s\n",
			    hosts[wo].hostname);
		    send_host(wi,tmp1);
		}
		else {
            sprintf(tmp1,"page God=Can't connect to %s\n",
			    hosts[wo].hostname);
		    send_host(wi,tmp1);
		}
		break;
	    }
	}
/*	if (wo >= MAXHOSTS) {
        sprintf(tmp1,"page God=I don't know world %s\n",
		    &(inbuf[strlen(RECONNMSG)+1]));
	    send_host(wi, tmp1);
	}   Is this really necesssary now? */
    }

    else if (hosts[wi].status == STAT_OK) {
	if (strncmp(inbuf, RWHOKEY,RWHOKEYLEN)==0)
	  do_rwho(wi, &(inbuf[RWHOKEYLEN]));
	else if (strncmp(inbuf, RFINGERKEY, RFINGERKEYLEN)==0)
	  do_rfinger(wi, &(inbuf[RFINGERKEYLEN]));
	else if (strncmp(inbuf, RINFOKEY,RINFOKEYLEN)==0)
	  do_rinfo(wi, &(inbuf[RINFOKEYLEN]));
	else if (strncmp(inbuf, RPAGEKEY,RPAGEKEYLEN)==0)
	  do_rpage(wi, &(inbuf[RPAGEKEYLEN]));
	else if (strncmp(inbuf, RWORLDSKEY,RWORLDSKEYLEN)==0)
	  do_rworlds(wi, &(inbuf[RWORLDSKEYLEN]));
	else if (strncmp(inbuf, RMAILKEY,RMAILKEYLEN)==0)
	  do_rmail(wi, &(inbuf[RMAILKEYLEN]));
    }
}


do_rwho(wi, inbuf)
int wi;
char *inbuf;
{
    char *name;
    char *world;
    char outbuf[MAXSTRLEN];
    char tmpbuf[MAXSTRLEN];

    int i;

    hosts[wi].idle = time(0);

    name = strtok(inbuf,":");

    if (name == 0) {
	fprintf(stderr,"Rwho: bad RWHOrequest '%s'\n", inbuf);
	return(0);
    }

    world = strtok(0,":");

    if (world == 0) {
	fprintf(stderr,"Rwho: bad RWHOrequest '%s'\n", inbuf);
	return(0);
    }

    for (i = 0; hosts[i].hostname != NULL; i++) {
	if (substring(world,hosts[i].hostname) &&
	    hosts[i].status == STAT_OK) {
	    fprintf(stderr,"Rwho: [%s@%s] asked '%s'; found [%s]\n",
		    name, hosts[wi].hostname, world, hosts[i].hostname);
	    sprintf(outbuf,"WHO\n");
	    send_host(i,outbuf);
	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,PREFIXTEXT,strlen(PREFIXTEXT)) != 0) {
		if (lagging(i,name,wi,tmpbuf))
		  return(0); 
		getline(tmpbuf, i);
		buffer_line(i,tmpbuf);
	    }

	    set_timer();
	    while (strncmp(tmpbuf,SUFFIXTEXT,strlen(SUFFIXTEXT)) != 0) {
		if (lagging(i,name,wi,tmpbuf))
		  return(0);
		  getline(tmpbuf, i);
		if (strncmp(tmpbuf,SUFFIXTEXT,strlen(SUFFIXTEXT)) &&
		    !buffer_line(i,tmpbuf)) {
		    if (strlen(tmpbuf) > 1) {
			bzero(outbuf,MAXSTRLEN);

/* Send the right type of output depending on the MUSH */
	switch (hosts[wi].pemittype) {
	    case 0:
            sprintf(outbuf,"%s *%s=<%s> [mid({[lit(%s)]},0,70)]\n", PEMITCMD,
				name, hosts[i].hostname, tmpbuf);
	    break;
	    case 2:
	    sprintf(outbuf,"%s *%s=<%s> %s\n",
		DZPEMIT, name, hosts[i].hostname, tmpbuf);
	    break;
	    case 1:
	    default:
	    sprintf(outbuf,"%s *%s=<%s> [mid({%s},0,70)]\n",
		PEMITCMD, name, hosts[i].hostname, tmpbuf);
	    break;
	}

			send_host(wi,outbuf);
		    }
		}
	    }
        sprintf(outbuf,"%s *%s=Done.\n",PEMITCMD, name);
	    send_host(wi,outbuf);
	    return(1);
	}
    }
    fprintf(stderr,"Rwho: [%s@%s] World '%s' not found\n",
	    name, hosts[wi].hostname, world);
    bzero(outbuf,MAXSTRLEN);
    sprintf(outbuf,"%s *%s=Sorry, there is no link to world %s.\n",
	    PEMITCMD, name, world);
    send_host(wi,outbuf);
}


do_rfinger(wi, inbuf)
int wi;
char *inbuf;
{
    char *name;
    char *player;
    char *world;
    char outbuf[MAXSTRLEN];
    char tmpbuf[MAXSTRLEN];

    int i;

    hosts[wi].idle = time(0);

    name = strtok(inbuf,":");

    if (name == 0) {
	fprintf(stderr,"Rfinger: bad RFINGERrequest '%s'\n", inbuf);
	return(0);
    }
    
    player = strtok(0,":");
    
    if (player == 0) {
    	fprintf(stderr,"Rfinger: bad RFINGERrequest '%s'\n", inbuf);
    	return(0);
    }
    
    world = strtok(0,":");

    if (world == 0) {
	fprintf(stderr,"Rfinger: bad RFINGERrequest '%s'\n", inbuf);
	return(0);
    }

    for (i = 0; hosts[i].hostname != NULL; i++) {
	if (substring(world,hosts[i].hostname) &&
	    hosts[i].status == STAT_OK) {
	    fprintf(stderr,"Rfinger: [%s@%s] asked '%s'; found [%s]\n",
		    name, hosts[wi].hostname, world, hosts[i].hostname);

	    bzero(outbuf,MAXSTRLEN);
            sprintf(outbuf,"+mlfinger %s\n", player);

	    send_host(i,outbuf);
	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,SOFTPREFIX,strlen(SOFTPREFIX)) != 0) {
		if (lagging(i,name,wi,tmpbuf)) {
		  fprintf(stderr,"Rfinger: lagged out at 1\n");
		  return(0); 
		  }
		getline(tmpbuf, i);
		buffer_line(i,tmpbuf);
	    }

	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,SOFTSUFFIX,strlen(SOFTSUFFIX)) != 0) {
		if (lagging(i,name,wi,tmpbuf)) {
		  fprintf(stderr,"Rfinger: lagged out at 2\n");
		  return(0);
		  }
		getline(tmpbuf, i);
		if (strncmp(tmpbuf,SOFTSUFFIX,strlen(SOFTSUFFIX)) &&
		    !buffer_line(i,tmpbuf)) {
		    if (strlen(tmpbuf) > 1 && strncmp(tmpbuf,"Notified.",strlen("Notified.")) &&
		    		strncmp(tmpbuf,PREFIXTEXT,strlen(PREFIXTEXT)) &&
		    		strncmp(tmpbuf,SUFFIXTEXT,strlen(SUFFIXTEXT))) {
			bzero(outbuf,MAXSTRLEN);

/* send right type of output based on type of MUSH */

	switch (hosts[wi].pemittype) {
		case 0:
            sprintf(outbuf,"%s *%s=[lit(%s)]\n", PEMITCMD, name, tmpbuf);
		break;
		case 2:
	    sprintf(outbuf,"%s *%s=%s\n", DZPEMIT, name, tmpbuf);
		break;
		case 1:
		default:
	    sprintf(outbuf,"%s *%s=%s\n", PEMITCMD, name, tmpbuf);
		break;
	}

			send_host(wi,outbuf);
		    } 
		}
	    }
        sprintf(outbuf,"%s *%s=Done.\n",PEMITCMD, name);
	    send_host(wi,outbuf);
	    return(1);
	}
    }
    fprintf(stderr,"Rfinger: [%s@%s] World '%s' not found\n",
	    name, hosts[wi].hostname, world);
    bzero(outbuf,MAXSTRLEN);
    sprintf(outbuf,"%s *%s=Sorry, there is no link to world %s.\n",
	    PEMITCMD, name, world);
    send_host(wi,outbuf);
}


do_rinfo(wi, inbuf)
int wi;
char *inbuf;
{
    char *name;
    char *world;
    char outbuf[MAXSTRLEN];
    char tmpbuf[MAXSTRLEN];

    int i;

    hosts[wi].idle = time(0);

    name = strtok(inbuf,":");

    if (name == 0) {
	fprintf(stderr,"Rinfo: bad RINFOrequest '%s'\n", inbuf);
	return(0);
    }
    
    world = strtok(0,":");

    if (world == 0) {
	fprintf(stderr,"Rinfo: bad RINFOrequest '%s'\n", inbuf);
	return(0);
    }

    for (i = 0; hosts[i].hostname != NULL; i++) {
	if (substring(world,hosts[i].hostname) &&
	    hosts[i].status == STAT_OK) {
	    fprintf(stderr,"Rinfo: [%s@%s] asked '%s'; found [%s]\n",
		    name, hosts[wi].hostname, world, hosts[i].hostname);

	    bzero(outbuf,MAXSTRLEN);
            sprintf(outbuf,"+mlinfo\n");

	    send_host(i,outbuf);
	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,SOFTPREFIX,strlen(SOFTPREFIX)) != 0) {
		if (lagging(i,name,wi,tmpbuf)) {
		  fprintf(stderr,"Rinfo: lagged out at 1\n");
		  return(0); 
		  }
		getline(tmpbuf, i);
		buffer_line(i,tmpbuf);
	    }

	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,SOFTSUFFIX,strlen(SOFTSUFFIX)) != 0) {
		if (lagging(i,name,wi,tmpbuf)) {
		  fprintf(stderr,"Rinfo: lagged out at 2\n");
		  return(0);
		  }
		getline(tmpbuf, i);
		if (strncmp(tmpbuf,SOFTSUFFIX,strlen(SOFTSUFFIX)) &&
		    !buffer_line(i,tmpbuf)) {
		    if (strlen(tmpbuf) > 1 && strncmp(tmpbuf,"Notified.",strlen("Notified.")) &&
		    		strncmp(tmpbuf,PREFIXTEXT,strlen(PREFIXTEXT)) &&
		    		strncmp(tmpbuf,SUFFIXTEXT,strlen(SUFFIXTEXT))) {
			bzero(outbuf,MAXSTRLEN);

/* send right type of output based on type of MUSH */

	switch (hosts[wi].pemittype) {
		case 0:
            sprintf(outbuf,"%s *%s=[lit(%s)]\n", PEMITCMD, name, tmpbuf);
		break;
		case 2:
	    sprintf(outbuf,"%s *%s=%s\n", DZPEMIT, name, tmpbuf);
		break;
		case 1:
		default:
	    sprintf(outbuf,"%s *%s=%s\n", PEMITCMD, name, tmpbuf);
		break;
	}

			send_host(wi,outbuf);
		    } 
		}
	    }
        sprintf(outbuf,"%s *%s=Done.\n",PEMITCMD, name);
	    send_host(wi,outbuf);
	    return(1);
	}
    }
    fprintf(stderr,"Rinfo: [%s@%s] World '%s' not found\n",
	    name, hosts[wi].hostname, world);
    bzero(outbuf,MAXSTRLEN);
    sprintf(outbuf,"%s *%s=Sorry, there is no link to world %s.\n",
	    PEMITCMD, name, world);
    send_host(wi,outbuf);
}

do_rpage(wi, inbuf)
int wi;
char *inbuf;
{
    char *from;
    char *to;
    char *world;
    char *msg;

    char outbuf[MAXSTRLEN];
    char tmpbuf[MAXSTRLEN];

    int i;

    hosts[wi].idle = time(0);

    from = strtok(inbuf,":");

    if (from == 0) {
	fprintf(stderr,"Page: bad RPAGErequest '%s'\n", inbuf);
	return(0);
    }

    to = strtok(0,":");

    if (to == 0) {
	fprintf(stderr,"Page: bad RPAGErequest '%s'\n", inbuf);
	return(0);
    }

    world = strtok(0,":");

    if (world == 0) {
	fprintf(stderr,"Page: bad RPAGErequest '%s'\n", inbuf);
	return(0);
    }
    if (world[strlen(world)-1] == ' ')
      world[strlen(world)-1] = '\0';

    msg = strtok(0,"\0");

    if(!msg || !*msg) {
	sprintf(outbuf, "%s *%s=No message to page (%s@%s)\n",
		PEMITCMD, from, to, world);
	send_host(wi,outbuf);
	return;
    }

    if (debug_flg)
      fprintf(stderr,"Page: [%s@%s] %s@%s = %s\n",
	      from, hosts[wi].hostname, to, world, msg);
    else
      fprintf(stderr,"Page: [%s@%s] %s@%s\n",
	      from, hosts[wi].hostname, to, world);
      
    for (i = 0; hosts[i].hostname != NULL; i++) {
	if (substring(world,hosts[i].hostname) &&
	    hosts[i].status == STAT_OK) {

	    sprintf(outbuf,"@emit %s [flags(*%s)]\n",INPREFIX, to);
	    send_host(i,outbuf);

	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,INPREFIX,strlen(INPREFIX)) != 0) {
		if (lagging(i,from,wi,tmpbuf))
		  return(0);
		getline(tmpbuf, i);
		buffer_line(i,tmpbuf);
	    }

	    if (tmpbuf[INPREFIXLEN+1] != 'P') {
        sprintf(outbuf,"%s *%s=Sorry, %s does not exist on %s.\n", PEMITCMD,
			from,to,world);
		send_host(wi,outbuf);
		return(0);
	    }

	    sprintf(outbuf,"@emit %s [u(onfor, %s)]\n",INPREFIX, to);
	    send_host(i,outbuf);

	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,INPREFIX,strlen(INPREFIX)) != 0) {
		if (lagging(i,from,wi,tmpbuf))
		  return(0);
		getline(tmpbuf, i);
		buffer_line(i,tmpbuf);
	    }
	    if (tmpbuf[INPREFIXLEN+1] == '#') {
		send_host(wi,outbuf);
	    }

        switch(msg[0]) {
        	case ':':
        		sprintf(outbuf,"%s *%s=From afar, %s@%s %s\n", PEMITCMD,
				to,from,hosts[wi].hostname,msg + 1);
		    		send_host(i,outbuf);

		        sprintf(outbuf,"%s *%s=Long distance to %s@%s: \"%s %s\".\n", PEMITCMD,
				from,to,hosts[i].hostname,from,msg + 1);
	    			send_host(wi,outbuf);
			break;
		case ';':
        		sprintf(outbuf,"%s *%s=From afar, %s@%s%s\n", PEMITCMD,
				to,from,hosts[wi].hostname,msg + 1);
		    		send_host(i,outbuf);

		        sprintf(outbuf,"%s *%s=Long distance to %s@%s: \"%s%s\".\n", PEMITCMD,
				from,to,hosts[i].hostname,from,msg + 1);
	    			send_host(wi,outbuf);
			break;
		default:
        		sprintf(outbuf,"%s *%s=%s@%s pages: %s\n", PEMITCMD,
				to,from,hosts[wi].hostname,msg);
		    		send_host(i,outbuf);

		        sprintf(outbuf,"%s *%s=You paged %s@%s with \"%s\".\n", PEMITCMD,
				from,to,hosts[i].hostname,msg);
	    			send_host(wi,outbuf);
	}
	return(1);
	}
    }
    bzero(outbuf,MAXSTRLEN);
    sprintf(outbuf,"%s *%s=Sorry, there is no link to world %s.\n",
	    PEMITCMD, from, world);
    send_host(wi,outbuf);
}


do_rmail(wi, inbuf)
int wi;
char *inbuf;
{
    char *from;
    char *to;
    char *world;
    char *msg;

    char outbuf[MAXSTRLEN];
    char tmpbuf[MAXSTRLEN];

    int i;

    hosts[wi].idle = time(0);

    from = strtok(inbuf,":");

    if (from == 0) {
	fprintf(stderr,"Mail: bad RMAILrequest '%s'\n", inbuf);
	return(0);
    }

    to = strtok(0,":");

    if (to == 0) {
	fprintf(stderr,"Mail: bad RMAILrequest '%s'\n", inbuf);
	return(0);
    }

    world = strtok(0,":");

    if (world == 0) {
	fprintf(stderr,"Mail: bad RMAILrequest '%s'\n", inbuf);
	return(0);
    }
    if (world[strlen(world)-1] == ' ')
      world[strlen(world)-1] = '\0';

    msg = strtok(0,"\0");

    if (debug_flg)
      fprintf(stderr,"Mail: [%s@%s] %s@%s = %s\n",
	      from, hosts[wi].hostname, to, world, msg);
    else
      fprintf(stderr,"Mail: [%s@%s] %s@%s\n",
	      from, hosts[wi].hostname, to, world);
      
    for (i = 0; hosts[i].hostname != NULL; i++) {
	if (substring(world,hosts[i].hostname) &&
	    hosts[i].status == STAT_OK) {

	    sprintf(outbuf,"@emit %s [flags(*%s)]\n",INPREFIX, to);
	    send_host(i,outbuf);

	    set_timer();
	    bzero(tmpbuf,MAXSTRLEN);
	    while (strncmp(tmpbuf,INPREFIX,strlen(INPREFIX)) != 0) {
		if (lagging(i,from,wi,tmpbuf))
		  return(0);
		getline(tmpbuf, i);
		buffer_line(i,tmpbuf);
	    }

	    if (tmpbuf[INPREFIXLEN+1] != 'P') {
        sprintf(outbuf,"%s *%s=Sorry, %s does not exist on %s.\n", PEMITCMD,
			from,to,world);
		send_host(wi,outbuf);
		return(0);
	    }

	   bzero(outbuf,MAXSTRLEN);

/* Send right type of output based on type of MUSH */

	switch (hosts[i].mailtype) {
		case 0:
        sprintf(outbuf,"@mail %s=MushLink Mail from %s@%s/", to, from, hosts[wi].hostname);
	send_host(i, outbuf);
	bzero(outbuf,MAXSTRLEN);
	sprintf(outbuf,"%s", msg);
	send_host(i, outbuf);
	bzero(outbuf, MAXSTRLEN);
	sprintf(outbuf,"%r** reply using \"mlmail\". See +help mushlink for help. **\n");
		break;
		case 2:
        sprintf(outbuf,"+mail %s=MushLink Mail from %s@%s %R", to, from, hosts[wi].hostname);
	send_host(i, outbuf);
	bzero(outbuf,MAXSTRLEN);
	sprintf(outbuf,"%s", msg);
	send_host(i, outbuf);
	bzero(outbuf, MAXSTRLEN);
	sprintf(outbuf,"%r** reply using \"mlmail\". See +help mushlink for help. **\n");
		break;
		case 3:
	sprintf(outbuf,"+mail %s=MushLink Mail from %s@%s\n",
	    to, from, hosts[wi].hostname);
		send_host(i, outbuf);
		bzero(outbuf,MAXSTRLEN);
		sprintf(outbuf,"-%s\n",msg);
		send_host(i, outbuf);
		bzero(outbuf,MAXSTRLEN);
		sprintf(outbuf,"+send\n");
		break;
		case 4:
        sprintf(outbuf,"+smail %s=MushLink Mail from %s@%s %R", to, from, hosts[wi].hostname);
	send_host(i, outbuf);
	bzero(outbuf,MAXSTRLEN);
	sprintf(outbuf,"%s", msg);
	send_host(i, outbuf);
	bzero(outbuf, MAXSTRLEN);
	sprintf(outbuf,"%r** reply using \"mlmail\". See +help mushlink for help. **\n");
		break;
		case 5:
	sprintf(outbuf,"@mail %s=MushLink Mail from %s@%s\n",
	    to, from, hosts[wi].hostname);
		send_host(i, outbuf);
		bzero(outbuf,MAXSTRLEN);
		sprintf(outbuf,"-%s\n",msg);
		send_host(i, outbuf);
		bzero(outbuf,MAXSTRLEN);
		sprintf(outbuf,"@mail/send\n");
		break;
		case 1:
		default:
        sprintf(outbuf,"@mail %s=MushLink Mail from %s@%s %R", to, from, hosts[wi].hostname);
	send_host(i, outbuf);
	bzero(outbuf,MAXSTRLEN);
	sprintf(outbuf,"%s", msg);
	send_host(i, outbuf);
	bzero(outbuf, MAXSTRLEN);
	sprintf(outbuf,"%r** reply using \"mlmail\". See +help mushlink for help. **\n");
		break;
	}

	    send_host(i,outbuf);

	bzero(outbuf,MAXSTRLEN);
        sprintf(outbuf,"%s *%s=Your mail message, \"%s\", has been sent to  %s@%s.\n",
		    PEMITCMD, from,msg,to,world);
	    send_host(wi,outbuf);

	    return(1);
	}
    }
    bzero(outbuf,MAXSTRLEN);
    sprintf(outbuf,"%s *%s=Sorry, there is no link to world %s.\n",
	    PEMITCMD, from, world);
    send_host(wi,outbuf);
}


do_rworlds(wi, inbuf)
int wi;
char *inbuf;
{
    char *name;
    char outbuf[MAXSTRLEN];

    int i;
    char mtype;

    hosts[wi].idle = time(0);

    name = inbuf;

    bzero(outbuf,MAXSTRLEN);
    sprintf(outbuf, "%s *%s=[center(%s Version %s,79)]%R[repeat(-,79)]\n",PEMITCMD, name, RLINKNAME, VERSION);
    send_host(wi,outbuf);
    sprintf(outbuf, "%s *%s=[ljust(World,13)] [ljust(IP Address,24)] Status\n", PEMITCMD, name);
    send_host(wi,outbuf);
    sprintf(outbuf, "%s *%s=[repeat(-,79)]\n",
	    PEMITCMD, name);
    send_host(wi,outbuf);

    for (i=0; hosts[i].hostname != NULL; i++) {
	bzero(outbuf,MAXSTRLEN);
	switch (hosts[i].mushtype) {
		case 0: mtype='P'; break;
		case 1: mtype='T'; break;
		case 2: mtype='D'; break;
		case 3: mtype='X'; break;
		case 4: mtype='S'; break;
		default: mtype='?'; break;
	}

	if (hosts[i].status == STAT_OK) {
        sprintf(outbuf,"%s *%s=[ljust(%s,9)] <%c> [ljust(%s %d,24)] [ljust(UP,5)] for [u(csecs, %d)]\n",
		    PEMITCMD, name, hosts[i].hostname, mtype, hosts[i].hostip, hosts[i].port, 
		    time(0)-hosts[i].upsince);
	    send_host(wi,outbuf);
	}
	else if (hosts[i].status == STAT_PAUSE) {
        sprintf(outbuf, "%s *%s=[ljust(%s,9)] <%c> [ljust(%s %d,24)] [ljust(UP,5)] % % for [u(csecs, %d)] but PAUSED\n",
	    PEMITCMD, name,hosts[i].hostname, mtype, hosts[i].hostip, hosts[i].port, time(0) - hosts[i].upsince);
	    send_host(wi,outbuf);
	}		
	else if (hosts[i].status == STAT_DEAD) {
	    sprintf(outbuf,
            "%s *%s=[ljust(%s,9)] <%c> [ljust(%s %d,24)] [ljust(DOWN,5)] for [u(csecs, %d)]\n",
	    PEMITCMD, name,hosts[i].hostname, mtype, hosts[i].hostip, hosts[i].port, time(0) - hosts[i].upsince);
	    send_host(wi,outbuf);
	}
    }

    sprintf(outbuf,"%s *%s=[repeat(-,79)]\n",PEMITCMD, name);
    send_host(wi,outbuf);
    bzero(outbuf,MAXSTRLEN);
    sprintf(outbuf,"%s *%s=%B%B%BP - PennMUSH%B%B%BT - TinyMUSH%B%B%BD - DarkZone%B%B%BX - TinyMUX%B%B%BS - MUSE\n",
	PEMITCMD, name);
    send_host(wi,outbuf);
}



/* sync_bot sends a garbage message and waits for the HUH? to come back.
    This is useful for synchronizing dialogue.                           */

sync_bot(host, buf)
char *buf;
{
    int count;

    count = 0;

    send_host(host, SENDSYNCMSG);      

    bzero (buf, MAXSTRLEN);
    do {
	sleep(1);
	read (hosts[host].socket, buf, MAXSTRLEN);
	if (count++ > 30)
	  return(0);
     }
   while(strstr(buf,ACKSYNCMSG)==0);

   return(1);
}



/* Getline takes a line from specified file descriptor and   */
/* stuffs it into inbuf.                                     */

int getline(inbuf, world)
char *inbuf;
int world;
{
    char c, buf[MAXSTRLEN];
    int noise, i;

    bzero (inbuf, MAXSTRLEN);
    noise = read(hosts[world].socket,inbuf,1);
    if (noise == 0) {
	close(hosts[world].socket);
	hosts[world].status = STAT_DEAD;
	fprintf(stderr,"Conn: [%s] involuntarily closed.\n",
		hosts[world].hostname);
	num_worlds--;
	hosts[world].upsince = time(0);
	inbuf[0] = 0;
	return(0);
    }
    else if (noise == -1 || inbuf[0] == '\n') {
	inbuf[0] = 0;
	return(0);
    }
    else {
	i = 1;
	do {
	    noise = read(hosts[world].socket,&(inbuf[i++]), 1);
	    if (inbuf[i] == '\r') i--;
	} while ((inbuf[i-1] != '\n') && (i != MAXSTRLEN-1));

	inbuf[i-1] = 0;

	return(1);
    }
}


send_host (host,message)
int host;
char *message;
{
    if (debug_flg)
      fprintf(stderr,"Send [%s]: %s\n", hosts[host].hostname,message);

    write (hosts[host].socket, message, strlen(message));	
}

substring(s1, s2)
char *s1, *s2;
{
    int i;
    
    if (strlen(s1) > strlen(s2))
      return(0);

    for (i = 0; i < strlen(s1); i++) {
      if (s1[i] == '\r' || s1[i] == '\n') break;
      if (tolower(s1[i]) != tolower(s2[i]))
	return(0);
    }

    return(1);
}


reconnect_world(w)
int w;
{
    if (open_socket(w)) {
        if (log_in(w)) {
	fprintf(stderr,"Conn: [%s] Reconnecting on alarm\n",
		hosts[w].hostname);
	return(1);
        }
    }
    else {
	fprintf(stderr,"Conn: [%s] Can't reconnect on alarm\n",
		hosts[w].hostname);
	return(0);
    }
}

void do_alarm()
{
int w;
struct itimerval itime;
struct timeval interval;

    fprintf(stderr,"Alrm: timeout (%d seconds with a world down)\n",
	    RECON_TIME);

    for (w = 0; hosts[w].hostname != NULL ; w++)
      if (hosts[w].status == STAT_DEAD)
	reconnect_world(w);
	
    signal(SIGALRM, do_alarm);
    interval.tv_sec = RECON_TIME;
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_REAL, &itime, 0);
}

buffer_line(w,s)
     int w;
     char *s;
{
	

    if (
	strncmp(s, RWHOKEY,RWHOKEYLEN)==0 ||
	strncmp(s, RFINGERKEY, RFINGERKEYLEN)==0 ||
	strncmp(s, RINFOKEY,RINFOKEYLEN)==0 ||
	strncmp(s, RPAGEKEY,RPAGEKEYLEN)==0 ||
	strncmp(s, RWORLDSKEY,RWORLDSKEYLEN)==0) {
	
	if (ibufhead == (ibuftail + 1) % IBUF_LINES)
	  return(-1);

	else {
	    ibufwrld[ibuftail] = w;
	    strcpy(ibuftxt[ibuftail],s);
	    ibuftail = (ibuftail+1) % IBUF_LINES;

	    if (debug_flg)
	      fprintf(stderr,"Buff: [%s] %s\n",
		      hosts[w].hostname, s);
	    else
	      fprintf(stderr,"Buff: [%s]\n", hosts[w].hostname);
	}
    }
    else
      return(0);
}

set_timer()
{
    timer = time(0);
}

lagging(world,name,orig,buff)
int world,orig;
char *name,*buff;
{
    char buf[MAXSTRLEN];

    if (!strncmp(buff, "Huh", 3)) 
    	return(1);
    else if (time(0) - timer > LAG_TIMEOUT) {
	sprintf(buf,
        "%s *%s=Sorry, world '%s' is lagging; command halted.\n",
		PEMITCMD, name,hosts[world].hostname);
	send_host(orig,buf);
	fprintf(stderr,"Lag: On [%s], reported to %s@%s\n",
		hosts[world].hostname,name,hosts[orig].hostname);
	return(1);
    }
    else
      return(0);
}

int timeout()
{
  char buf[MAXSTRLEN];

  if (time(0) - timer > CONNECT_TIMEOUT) {
	fprintf(stderr,"Connect Timeout\n");
	return(1);
  } else {
	return(0);
}

int load_worlds() {
   FILE *fp;
   int i = 0, j = 0;
   int p1, p2, p3, p4;
   int nworlds;
   char buffer[200];

   nworlds = 0;
   if (!(fp = fopen(WFILE,"r")))
   { 
     fprintf(stderr, "Init: Cannot read %s.\n", WFILE);
     return 0;
   } else {
     fscanf(fp, "%d\n", &nworlds);
     if(nworlds < 1) {
	fprintf(stderr, "Init: No worlds to read in\n");
	fclose (fp);
	return 0;
     } else if (nworlds > MAXHOSTS) {
	fprintf(stderr, "Init: Number of Worlds greater than define");
  	fclose (fp);
    	return 0;
     }
     for(i = 0; i < nworlds; i++) {
       /*  Init the port Ip's */
       p1 = p2 = p3 = p4 = 0;
       hosts[i].upsince = time(0);
       hosts[i].port = hosts[i].mushtype = hosts[i].mailtype = hosts[i].pemittype
		= hosts[i].socket = hosts[i].status = 0;

       /* Read in the name of the world */
       fscanf(fp,"%[^\n]\n",&buffer);
       hosts[i].hostname = (char *)malloc(sizeof(char)*strlen(buffer)+1);
       strcpy(hosts[i].hostname, buffer);

       /* Read in the Password for the account */
       fscanf(fp,"%[^\n]\n",&buffer);
       hosts[i].password = (char *)malloc(sizeof(char)*strlen(buffer)+1);
       strcpy(hosts[i].password, buffer);

       /* Read in the various #'s */
       fscanf(fp,"%d.%d.%d.%d %d %d %d %d\n",&p1, &p2, &p3, &p4,
	 &(hosts[i].port), &(hosts[i].mushtype), &(hosts[i].mailtype), &(hosts[i].pemittype));
       hosts[i].charname = (char *)malloc(sizeof(char)*strlen(RLINKNAME)+1);
       strcpy(hosts[i].charname, RLINKNAME);
       sprintf(buffer, "%d.%d.%d.%d", p1, p2, p3, p4);
       hosts[i].hostip = (char *)malloc(sizeof(char)*strlen(buffer)+1);
       strcpy(hosts[i].hostip,buffer);

     }
     
     hosts[i].hostname = hosts[i].hostip = hosts[i].password =
	hosts[i].charname = NULL;
     hosts[i].port = hosts[i].mushtype = hosts[i].mailtype = hosts[i].pemittype =
	hosts[i].upsince = hosts[i].status = hosts[i].idle = 0;
   }

   fclose (fp);   
   return nworlds;
}

