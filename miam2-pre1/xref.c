/* xref.c
   a tool for MU replication
   09/12/2001  basics. an evil hack.                        Azundris
 */


#include <stdio.h>
#include <stdlib.h>

#include "mu.h"


#undef DEBUG


struct _obj {
  char        *name;
  int          type;
  int          cost;
  int          home,
               owner,
               loc;
  int          oldref,
               newref;
  char        *obj;
  struct _obj *next; } *anchor=NULL;



struct _obj *new_obj(void) {
  struct _obj *o;
  if(o=malloc(sizeof(struct _obj))) {
    o->name  =NULL;
    o->type  =TYPE_NONE;
    o->cost  =10;
    o->home  =
    o->owner =
    o->loc   =0;
    o->oldref=
    o->newref=0;
    o->obj   =NULL;
    o->next  =NULL; }
  return o; }

void dst_obj(struct _obj *o) {
  if(o->name)
    free(o->name);
  if(o->obj)
    free(o->obj);
#ifdef DEBUG
  o->name=NULL;
  o->obj =NULL;
  o->next=NULL;
  o->type=TYPE_NONE;
#endif
  free(o); }

struct _obj *dst_objs(struct _obj *a) {
  struct _obj *b;
  while(a) {
    b=a->next;
    dst_obj(a);
    a=b; }
  return NULL; }

struct _obj *set_obj_name(struct _obj *o,char *n) {
  char *s;
  if(s=strdup(n)) {
    char *p;
    if(p=strchr(s,'=')) {        /* cost for @create */
      *(p++)='\0';
      o->cost=atoi(p); }}
  o->name=s;
  return o; }

struct _obj *set_obj_body(struct _obj *o,char *n) {
  if(n&&o) {
    char *s;
    if(o->obj) {
      size_t l=strlen(o->obj);
      if(s=realloc(o->obj,l+strlen(n)+3L)) {
        if((n[0]=='&')||(n[0]=='@'))
          strcat(&o->obj[l],"\n");
        else
          strcat(&o->obj[l],"%r");
        strcat(&o->obj[l],n); }
      else {
        free(o->obj);
        o->obj=NULL;
        return NULL; }}
    else if(s=strdup(n)) {
      o->obj=s; }
    else
      return NULL; }
  return o; }

struct _obj *set_obj_type(  struct _obj *o,int v) { o->type  =v; return o; }
struct _obj *set_obj_home(  struct _obj *o,int v) { o->home  =v; return o; }
struct _obj *set_obj_loc(   struct _obj *o,int v) { o->loc   =v; return o; }
struct _obj *set_obj_owner( struct _obj *o,int v) { o->owner =v; return o; }
struct _obj *set_obj_oldref(struct _obj *o,int v) { o->oldref=v; return o; }
struct _obj *set_obj_newref(struct _obj *o,int v) { o->newref=v; return o; }

struct _obj *new_o(long objs) {
  struct _obj *o;
  if(!(o=new_obj())) {
    fprintf(stderr,"out of memory (%ld objects)...\n",objs);
    exit(-1); }
  return o; }



int do_x2c(char *p) {
  char *l=p;
  long cnt=0;
  if(!p)
    return cnt;
  while(p=strstr(p,"%x")) {
    if((p==l)||(p[-1]!='%')) {
      cnt++;
      *(++p)='c'; }
    p++; }
  return cnt; }



int xform_ref(int from) {
  struct _obj *o=anchor;
  int to;
  while(o&&o->oldref!=from)
    o=o->next;
  if(o) {
    if(o->newref)
      return o->newref;
    else
      fprintf(stderr,"object (#%d) has no value\n",from); }
  else
    fprintf(stderr,"object (#%d) not found\n",from);
  return 0; }



int main(int argc,char **argv,char **env) {
  FILE *fh=stdin,
       *fx=NULL;
  char *l;
  int   ret=-1;
  int   x2c=TRUE;

  if(argc>1) {
    if(!strcmp(argv[1],"--help")||!strcmp(argv[1],"-h")) {
      printf("%s <file> -- processes MU*-@decompile lists, with automagic xrefs\n\t--Azundris 2001/09/12\n",
             argv[0]);
      return 0; }
    if(argc>2) {
      if(fh=fopen(argv[1],"r"))
        fx=fopen(argv[2],"r"); }
    else
      fx=fopen(argv[1],"r"); }

  if(fh&&fx) {
    struct _obj *o,*o2=NULL;
    int          state=STATE_FIRST;
    long         objs=0;
    char        *p;
    int          dbref;

    if(o=anchor=new_o(objs))
      objs++;

    while(l=getline(fh)) {
      switch(state) {
      case STATE_DBREF:
        if((dbref=atoi(&l[1]))>0)
          set_obj_oldref(o,dbref);
        state++;
        break;

      case STATE_HOME:
        if((dbref=atoi(&l[1]))>0)
          set_obj_home(o,dbref);
        state++;
        break;

      case STATE_LOC:
        if((dbref=atoi(&l[1]))>0)
          set_obj_loc(o,dbref);
        state++;
        break;

      case STATE_OWNER:
        if((dbref=atoi(&l[1]))>0)
          set_obj_owner(o,dbref);
        state++;
        break;

      case STATE_OBJ:
        if(streq(l,STR_NEXT)) {                      /* end of object */
          o2=o;
#ifdef DEBUG
          printf("filed %s.\n",o->name);
#endif
          if(o=new_o(objs)) {
            objs++;
            o2->next=o; }
          state=STATE_FIRST; }

        else if(streq(l,STR_CREATE)) {               /* @create */
          set_obj_name(o,next_word(l));
          set_obj_type(o,TYPE_OBJECT); }

        else if(streq(l,STR_DIG)) {                  /* @dig */
          set_obj_name(o,next_word(l));
          set_obj_type(o,TYPE_ROOM); }

        else if(streq(l,STR_OPEN)) {                 /* @open */
          set_obj_name(o,next_word(l));
          set_obj_type(o,TYPE_EXIT); }

        else if(streq(l,STR_PLAYER)) {               /* @pcreate */
          char *p2;
          p=next_word(l);
          if(p2=strchr(p,'='))
            *p2='\0';
          set_obj_name(o,p);
          set_obj_type(o,TYPE_PLAYER); }

        else {                                       /* body */
          if(!o->type) {
            fprintf(stderr,"%s: invalid object %s -- no type!  dump: \"%s\"\n",argv[0],o->name,l);
            o->name=strdup("God");
            o->type=TYPE_PLAYER; }
          set_obj_body(o,l); }

        break;

      default:
        fprintf(stderr,"%s: error -- we should never get here! state=%d, line=%s\n",
                argv[0],state,l);
        break; }

      free(l); }

    ret=0;

    if(o2) {
      o2->next=NULL;
      dst_obj(o); }

    { int from,to;
      while(l=getline(fx)) {
        from=(int)strtol(l,&p,10);
        to=atoi(&p[2]);
        o=anchor;
        while(o&&o->oldref!=from)
          o=o->next;
        if(o)
          o->newref=to;
        else
          fprintf(stderr,"object #%d (now #%d) not found\n",from,to);
        free(l); }}

    o=anchor;
    while(o) {
      o->newref=xform_ref(o->oldref);
      o->home  =xform_ref(o->home);
      o->loc   =xform_ref(o->loc);
      o->owner =xform_ref(o->owner);

      { char   *p=o->obj;
        char   *e;
#define MAXDBREFLEN 12
        char    buf[MAXDBREFLEN];       /* semi-evil */
        size_t  lo,ln;
        int     oldref,newref;

        if(x2c) {                       /* %x -> %c */
          do_x2c(o->obj);
          do_x2c(o->name); }

        while(p=find_dbref(p)) {        /* fix dbrefs */
          if(oldref=(int)strtol(++p,&e,10)) {
#ifdef DEBUG
            fprintf(stderr,"%d->???\n",oldref);
#endif
            newref=xform_ref(oldref); }
          if(oldref&&newref) {
#ifdef DEBUG
            printf("%d->%d\n",oldref,newref);
#endif
            snprintf(buf,MAXDBREFLEN,"%d",newref);
            ln=strlen(buf);
            lo=e-p;
            if(ln<lo) {
              lo-=ln;
#ifdef DEBUG
              printf("shrink %ld\n",lo);
#endif
              e=&p[lo];
              memmove(p,e,strlen(e)+1L); }
            else if(ln>lo) {
              ln-=lo;
#ifdef DEBUG
              printf("grow %ld\n",ln);
#endif
              if(e=realloc(o->obj,strlen(o->obj)+1L+ln)) {
                p+=(e-o->obj);
                o->obj=e;
                memmove(&p[ln],p,strlen(p)+1L); }}
            memcpy(p,buf,strlen(buf));
#ifdef DEBUG
            puts("done");
            puts(o->obj);
#endif
          }                             /* if(oldref&&newref) */
          else
            fprintf(stderr,"#%d -> #%d requires %d -> %d\n",
                           o->oldref,o->newref,oldref,newref); }}

      printf("@pemit me=object #%d->#%d (%s) TYPE: %d\n",
             o->oldref,o->newref,
             o->name,
             o->type);
      fflush(stdout);
      puts(o->obj);

      if(o->home)
        printf("@link #%d=%d\n",o->newref,o->home);
      if(o->loc)
        printf("@tel #%d=%d\n",o->newref,o->loc);
      if(o->owner)
        printf("@chown #%d=%d\n",o->newref,o->owner);

      puts("@pemit me=%r\n");
      o=o->next; }

    fclose(fx);

    if(fh!=stdin)                       /* defensive */
      fclose(fh); }

  return ret; }

/* ends */

/*
*/
