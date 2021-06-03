/*
 * lists.c --  Various funcitons used to read/write an attrib as a list
 */
/*
 */


#include "header.h"
#include <string.h>

int ValDBMax;
DBItem ValDB[2000];
/*DBItem ValDB[20];*/

int VMsmatch(str, mat)
char *str, *mat;
{

	int wcount;
	char *r, *s, sep;

	sep=' ';
	/*
	 * Check each word individually, returning the word number of the * * 
	 * 
	 * *  * * first one that matches.  If none match, return 0. 
	 */

	wcount = 1;
	s = trim_space_sep(str, sep);
	do {
		r = split_token(&s, sep);
		if (quick_wild(mat, r)) {
			return wcount;
		}
		wcount++;
	} while (s);
	return 0;
}

/*
int valdb_find_index(dbit,skillnum,attrb)
int dbit;
int skillnum,attrb;
{
int i;
if(attrb==SKILL_A) {
	for(i=0;i<ValDB[dbit].sidmax;i++) 
			if(ValDB[dbit].sid[i]==skillnum) return(i);
	return(-1);
}
if(attrb==COMMOD_A) {
	for(i=0;i<ValDB[dbit].cidmax;i++) 
			if(ValDB[dbit].cid[i]==skillnum) return(i);
	return(-1);
}
}

int valdb_findadd_index(dbit,skillnum,attrb)
int dbit;
int skillnum,attrb;
{
int id;

id=valdb_find_index(dbit,skillnum,attrb);
if(id==-1 && attrb==SKILL_A && ValDB[dbit].sidmax < 50) {
	id=ValDB[dbit].sidmax;
	ValDB[dbit].sidmax++;
}
else if(id==-1 && attrb==COMMOD_A && ValDB[dbit].cidmax < 50) {
	id=ValDB[dbit].cidmax;
	ValDB[dbit].cidmax++;
}

return(id);
}

*/
float read_flist(player,skillnum,attrb) 
dbref player;
int skillnum;
{
int k=1,dbit;
char *strt,*tt,skv[1000];
if(attrb==SKILL_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	return(ValDB[dbit].skills[skillnum]);
	}
if(attrb==COMMOD_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	return(ValDB[dbit].commods[skillnum]);
	}
	

tt=vget_a(player,attrb);
if(tt==NULL) {return -1;}
strt=tt;
while(sscanf(tt,"%s",skv)>0) {
 tt+=strlen(skv)+1;
 if(k==skillnum) {
   /*free_lbuf(strt);*/
   return(atof(skv));}
k++;}

/*free_lbuf(strt);*/
return -1;
}



int change_ilist(player,skillnum,newval,attrb)
dbref player;
int skillnum;
int newval;
int attrb;
{
int num,k,dbit;
char *strt,skv[1000],new[9000],*tt,*tmp,*end;
if(attrb==SKILL_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	ValDB[dbit].skills[skillnum]=newval;
	return(1);
	}
if(attrb==COMMOD_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	ValDB[dbit].commods[skillnum]=newval;
	return(1);
	}

tt=vget_a(player,attrb);
tmp=tt;
strt=tt;
for(k=1;k<skillnum;k++) {
 num=sscanf(tmp,"%s",skv);
 if(num<1) {
  /*free_lbuf(strt);*/
  return -1;}
 tmp+=strlen(skv)+1;
 }
if(skillnum!=1) {
tmp--;
*tmp='\0';
end=tmp+1;
if(sscanf(end,"%s",skv)<1) {
  /*free_lbuf(strt);*/
  return -1;}
end=end+strlen(skv)+1;
sprintf(new,"%s %d %s",tt,newval,end);}
else { 
end=tmp;
if(sscanf(end,"%s",skv)<1) {
  /*free_lbuf(strt);*/
  return -1;}
end=end+strlen(skv)+1;
sprintf(new,"%d %s",newval,end);
 }

atr_add_raw(player,attrb,new);
/*free_lbuf(strt);*/
return 1;
}


int read_ilist(player,skillnum,attrb) 
dbref player;
int skillnum;
int attrb;
{
int k=1,dbit;
char *strt,*tt,skv[1000];
if(attrb==SKILL_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	return(ValDB[dbit].skills[skillnum]);
	}
if(attrb==COMMOD_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	return(ValDB[dbit].commods[skillnum]);
	}
tt=vget_a(player,attrb);
if(tt==NULL) {return -1;}
strt=tt;
while(sscanf(tt,"%s",skv)>0) {
 tt+=strlen(skv)+1;
 if(k==skillnum) {

/*   free_lbuf(strt);*/
   return(atoi(skv));}
k++;}

/*free_lbuf(strt);
*/
return -1;
}


int change_flist(player,skillnum,newval,attrb)
dbref player;
int skillnum;
float newval;
int attrb;
{
int num,k,dbit;
char *strt,skv[1000],new[9000],*tt,*tmp,*end;
if(attrb==SKILL_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	ValDB[dbit].skills[skillnum]=newval;
	return(1);
	}
if(attrb==COMMOD_A) {
	tt=vget_a(player,attrb);
	dbit=atoi(tt);
	ValDB[dbit].commods[skillnum]=newval;
	return(1);
	}

tt=vget_a(player,attrb);
tmp=tt;
strt=tt;
for(k=1;k<skillnum;k++) {
 num=sscanf(tmp,"%s",skv);
 if(num<1) {
  /*free_lbuf(strt);*/
  return -1;}
 tmp+=strlen(skv)+1;
 }
if(skillnum!=1) {
tmp--;
*tmp='\0';
end=tmp+1;
if(sscanf(end,"%s",skv)<1) {
  /*free_lbuf(strt);*/
  return -1;}
end=end+strlen(skv)+1;
sprintf(new,"%s %f %s",tt,newval,end);}
else { 
end=tmp;
if(sscanf(end,"%s",skv)<1) {
  /*free_lbuf(strt);*/
  return -1;}
end=end+strlen(skv)+1;
sprintf(new,"%f %s",newval,end);
 }

atr_add_raw(player,attrb,new);
/*free_lbuf(strt);*/
return 1;
}


int verify_slist(player,skillnum,attrb,wrd) 
dbref player;
int skillnum;
int attrb;
char *wrd;
{
int k=1;
char *strt,*tt,skv[1000];
tt=vget_a(player,attrb);
if(tt==NULL) {return -1;}
strt=tt;
while(sscanf(tt,"%s",skv)>0) {
 tt+=strlen(skv)+1;
 if(k==skillnum) {

   if(strcmp(wrd,skv)==0) return(1);
   return(0);}
k++;}

return -1;
}


int change_slist(player,skillnum,newval,attrb)
dbref player;
int skillnum;
char *newval;
int attrb;
{
int num,k;
char *strt,skv[1000],new[9000],*tt,*tmp,*end;

tt=vget_a(player,attrb);
tmp=tt;
strt=tt;
for(k=1;k<skillnum;k++) {
 num=sscanf(tmp,"%s",skv);
 if(num<1) {
  return -1;}
 tmp+=strlen(skv)+1;
 }
if(skillnum!=1) {
tmp--;
*tmp='\0';
end=tmp+1;
if(sscanf(end,"%s",skv)<1) {
  return -1;}
end=end+strlen(skv)+1;
sprintf(new,"%s %s %s",tt,newval,end);}
else { 
end=tmp;
if(sscanf(end,"%s",skv)<1) {
  /*free_lbuf(strt);*/
  return -1;}
end=end+strlen(skv)+1;
sprintf(new,"%s %s",newval,end);
 }

atr_add_raw(player,attrb,new);
return 1;
}


char *read_slist(player,skillnum,attrb) 
dbref player;
int skillnum;
int attrb;
{
int k=1;
char *strt,*tt,skv[1000];
tt=vget_a(player,attrb);
if(tt==NULL) {return -1;}
strt=tt;
while(sscanf(tt,"%s",skv)>0) {
 tt+=strlen(skv)+1;
 if(k==skillnum) {
   return(skv);}
k++;}

return NULL;
}

int do_VCheck(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int k,object,vdb;

for(k=1;k<ValDBMax;k++) {
 	object=ValDB[k].object;
	if(!Good_obj(object)) {
		ValDB[k].used=0;	
		}
	else {
		vdb=atoi(vget_a(object,SKILL_A));
		if(vdb!=k) {
			ValDB[k].used=0;	
			notify(player,tprintf("Marking VDB %d which originally pointed to %d as unused",k,object));
		}
	}

    }
	

}

void read_ValDB()
{
FILE *fptr;
int num;
fptr=fopen("./ValDB","r");
if(fptr==NULL) {
	ValDBMax=0;
	return;
	}
fread(&num,sizeof(int),1,fptr);
fread(ValDB,sizeof(DBItem),num,fptr);
ValDBMax=num;
fclose(fptr);
}

void write_ValDB()
{
FILE *fptr;
int num;
fptr=fopen("./ValDB","w");
fwrite(&ValDBMax,sizeof(int),1,fptr);
fwrite(ValDB,sizeof(DBItem),ValDBMax,fptr);
fclose(fptr);
}
int PDB_ValDB(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i,rdb,pdb;
char *fargs[3];

if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}

i=xcode_parse(buffer,fargs,2);
if(i!=2) return;
rdb=atoi(fargs[0]);
pdb=atoi(fargs[1]);
if(rdb < 0 || rdb > ValDBMax) {
		notify(player,"Invalid Val DB Entry");
		return;
	}
	ValDB[rdb].object=pdb;
	notify(player,"ValDB Entry updated");
}


int sset_ValDB(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i;
char *fargs[3];

if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}

i=xcode_parse(buffer,fargs,3);
if(i!=3) return;
change_ilist(atoi(fargs[0]),atoi(fargs[1]),atoi(fargs[2]),SKILL_A);

}

int cset_ValDB(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i;
char *fargs[3];
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}
i=xcode_parse(buffer,fargs,3);
if(i!=3) return;
change_ilist(atoi(fargs[0]),atoi(fargs[1]),atoi(fargs[2]),COMMOD_A);

}

int read_ValDB_interface(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}
read_ValDB();
}

int read_ValDBFind_interface(player,data,buffer)
{

char *fargs[3];
int i,k,object,rdb;
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}
i=xcode_parse(buffer,fargs,1);
if(i!=1) return;
rdb=-1;
object=atoi(fargs[0]);
for(k=1;k<ValDBMax;k++) {
 	if(ValDB[k].object==object) {
		rdb=k;
	}
	}
notify(player,tprintf("ValDB found to be = %d",rdb));

}

int read_ValDBMax_interface(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}
notify(player,tprintf("The maximum # of Val DBs is %d",ValDBMax));

}


int read_ValDBNum_interface(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[2],bf[50];
int num,i,ndb;
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}
i=xcode_parse(buffer,fargs,1);
if(i!=1) {
	notify(player,"Usage: CHECK <VALDB#>");
	return;
	}
num=atoi(fargs[0]);
if(num<0 || num > ValDBMax) {
	notify(player,"Invalid ValDB number");
	return;
	}
notify(player,"---------------------------------------");
notify(player,tprintf("ValDB %d",num));
notify(player,"---------------------------------------");
notify(player,tprintf(" DB#: %d		Used: %d",ValDB[num].object,ValDB[num].used));
ndb=ValDB[num].object;
if(isExit(ndb)) sprintf(bf,"Exit");
if(isPlayer(ndb)) sprintf(bf,"Player");
if(isThing(ndb)) sprintf(bf,"Thing");
if(isRoom(ndb)) sprintf(bf,"Room");
notify(player,tprintf("Name: %s		%s",Name(ndb),bf));
notify(player,"---------------------------------------");
}


int write_ValDB_interface(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}
write_ValDB();
}

int new_ValDB_interface(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i;
char *fargs[2];
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that.");
	return;
	}
i=xcode_parse(buffer,fargs,1);
if(i!=1) return;
new_ValDB(atoi(fargs[0]));
}


int new_ValDB(dbref object)
{
int k;
char buff[50];
for(k=1;k<ValDBMax;k++) {
 	if(ValDB[k].object==object) {
		clear_ValDB(k);
		ValDB[k].used=1;
		ValDB[k].object=object;
		sprintf(buff,"%d",k);
		atr_add_raw(object,SKILL_A,buff);
		atr_add_raw(object,COMMOD_A,buff);
		return(k);
	}
  }
for(k=0;k<ValDBMax;k++) {
	if(ValDB[k].used==0) {
		clear_ValDB(k);
		ValDB[k].used=1;
		ValDB[k].object=object;
		sprintf(buff,"%d",k);
		atr_add_raw(object,SKILL_A,buff);
		atr_add_raw(object,COMMOD_A,buff);
		return(k);
		}
	}
k=ValDBMax;
ValDBMax++;
clear_ValDB(k);
ValDB[k].used=1;
ValDB[k].object=object;
sprintf(buff,"%d",k);
atr_add_raw(object,SKILL_A,buff);
atr_add_raw(object,COMMOD_A,buff);

write_ValDB();

return(k);

}

void clear_ValDB(int index)
{
int k;
	bzero(ValDB[index].skills,sizeof(ValDB[index].skills));
	bzero(ValDB[index].commods,sizeof(ValDB[index].commods));
}


static char *rcslistsc="$Id: lists.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
