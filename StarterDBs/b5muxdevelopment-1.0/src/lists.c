/*
 * lists.c --  Various funcitons used to read/write an attrib as a list
 */
/*
 */


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
int change_flist(player,skillnum,newval,attrb)
dbref player;
int skillnum;
float newval;
int attrb;
{
int num,k;
char *strt,skv[1000],new[9000],*tt,*tmp,*end;

tt=vget_a(player,attrb);
strt=tt;
tmp=tt;
for(k=1;k<skillnum;k++) {
 num=sscanf(tmp,"%s",skv);
 if(num<1) {
  free_lbuf(strt);
  return -1;}
 tmp+=strlen(skv)+1;
 }
*tmp='\0';
end=tmp+1;
if(sscanf(end,"%s",skv)<1) {
  free_lbuf(strt);
  return -1;}
end=end+strlen(skv)+1;
sprintf(new,"%s%f %s",tt,newval,end);
tt=tmp=new;
atr_add_raw(player,attrb,tt);
free_lbuf(strt);
return 1;
}
*/

float read_flist(player,skillnum,attrb) 
dbref player;
int skillnum;
{
int k=1;
char *strt,*tt,skv[1000];
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
int num,k;
char *strt,skv[1000],new[9000],*tt,*tmp,*end;

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
int k=1;
char *strt,*tt,skv[1000];
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
int num,k;
char *strt,skv[1000],new[9000],*tt,*tmp,*end;

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



