/* 2creates.c
   a tool for MU replication
   09/12/2001 Azundris
 */

#include <stdio.h>
#include <string.h>
#include "mu.h"

#define DO_MAGIC(x) printf("@pemit me=#%d (%s)%%t" MAGIC "\n",(x)->ref,(x)->name)

struct _obj {
  char        *name;
  int          type,
               ref,
               from;
  struct _obj *next; } *anchor=NULL;



void dst_obj(struct _obj *o) {
  if(o) {
    if(o->name) { free(o->name); }
#ifdef DEBUG
    o->type=TYPE_NONE; o->ref=0; o->next=NULL;
#endif
    free(o); }}

struct _obj *dst_objs(struct _obj *a) {
  struct _obj *b;
  while(a) {
    b=a->next;
    dst_obj(a);
    a=b; }
  return NULL; }

struct _obj *new_obj(void) {
  struct _obj *o;
  if(o=malloc(sizeof(struct _obj))) {
    o->name  =NULL;
    o->type  =TYPE_NONE;
    o->ref   =0;
    o->from  =0;
    o->next  =NULL; }
  return o; }

struct _obj *set_obj_type(  struct _obj *o,int v) { o->type  =v; return o; }
struct _obj *set_obj_ref(   struct _obj *o,int v) { o->ref   =v; return o; }
struct _obj *set_obj_from(  struct _obj *o,int v) { o->from  =v; return o; }
struct _obj *set_obj_name(  struct _obj *o,char *n) {
  if(o) {
    char *prv=o->name;
    if(n) {
      if(!(o->name=strdup(n))) {
        o->name=prv;
        return NULL; }}
    else
      o->name=NULL;
    if(prv)
      free(prv); }
  return o; }

struct _obj *new_o(long objs,int r,int t,char *l) {
  struct _obj *o;
  if(!(o=new_obj())) {
    fprintf(stderr,"out of memory (%ld objects)...\n",objs);
    exit(-1); }
  set_obj_ref( o,r);
  set_obj_type(o,t);
  return set_obj_name(o,l); }





int main(int argc,char **argv,char **env) {
  FILE *fh=stdin;
  char *l;
  int   ret=-1;

  /* quasi-positional parameters. bad, bad. */
  if(argc>1) {
    if(!strcmp(argv[1],"--help")||!strcmp(argv[1],"-h")) {
      printf("%s [<file>] -- converts MU* object lists to @create lists\n\t--Azundris 2001/09/13\n",
             argv[0]);
      return 0; }

    fh=fopen(argv[1],"r"); }

  if(fh) {
    struct _obj *po=NULL,*o;
    int          dbref;
    long         objs=0;

    while(l=getline(fh)) {
      char *p=l-1L;
      do {
        p=find_dbref(++p);
      } while(p&&((p>&l[1])&&(p[-1]!='(')));

      if(p) {
        int          from=0,to=0;

        o=NULL;
        dbref=atoi(++p);
        p[-2]='\0';

        while(isdigit(*(++p)))
          ;

        switch(*p) {
        case 'E':
          if(p=find_dbref(p)) {
            from=atoi(++p);
            if(p=find_dbref(p)) {
              to=atoi(++p); }}
          if(o=new_o(objs,dbref,TYPE_EXIT,l)) {
            set_obj_from(o,from); }
          break;
        case 'R':
          o=new_o(objs,dbref,TYPE_ROOM,l);
          break;
        case 'P':
          o=new_o(objs,dbref,TYPE_PLAYER,l);
          break;
        default:
          o=new_o(objs,dbref,TYPE_OBJECT,l);
          break; }

        if(o) {
          objs++;
          if(!anchor)
            anchor=o;
          else
            po->next=o;
          po=o; }}
      free(l); }
    ret=0;

    o=anchor;
    while(o) {
      switch(o->type) {
      case TYPE_NONE:
      case TYPE_EXIT:
      case TYPE_EXIT_F:
        break;

      case TYPE_ROOM:
        printf("@dig/teleport %s\n",o->name);
        DO_MAGIC(o);
        po=anchor;
        while(po) {
          if((po->type==TYPE_EXIT)&&(po->from==o->ref)) {
            printf("@open %s\n"
                   "@pemit me=%s created as exit [num(%s)]\n",
                   po->name,po->name,po->name);
            DO_MAGIC(po);
            po->type=TYPE_EXIT_F; }
          po=po->next; }
        break;

      case TYPE_PLAYER:
        printf("@pcreate %s=%d\n",o->name,o->ref);
        DO_MAGIC(o);
        break;

      case TYPE_OBJECT:
        printf("@create %s\n",o->name);
        DO_MAGIC(o);
        break;

      default:
        fprintf(stderr,"unknown object type %d (\"%s\")\n",o->type,o->name); }

      o=o->next; }

    o=anchor;
    while(o) {
      if(o->type==TYPE_EXIT)
        fprintf(stderr,"exit #%d (\"%s\") not created; no room #%d to open from!\n",o->ref,o->name,o->from);
      o=o->next; }

    if(fh!=stdin)                       /* defensive */
      fclose(fh); }

  return ret; }

/* ends */
