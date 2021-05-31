/* mkindx.c */

#include "config.h"
#include "copyrite.h"

#include  <stdio.h>
#include  <stdlib.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "conf.h"
#include "help.h"
#include "globals.h"
#include "mymalloc.h"
#include "confmagic.h"
#ifdef EMBEDDED_MKINDX
#include "externs.h"
#endif

#define TRUE	1
#define FALSE	0

typedef struct TLIST {
  char topic[TOPIC_NAME_LEN + 1];
  struct TLIST *next;
} tlist;
tlist *top = NULL;

help_indx *topics = NULL;
unsigned num_topics = 0;
unsigned top_topics = 0;

void write_topic _((FILE * out, long int p, int l));
void flush_topics _((FILE * out));
static void add_topic _((char *name));
#ifndef EMBEDDED_MKINDX
int main _((int argc, char **argv));
#endif

#ifndef EMBEDDED_MKINDX
/* EMBEDDED_MKINDX includes this in the main prog, and thus gets strcasecmp
 * from strutil */
#ifndef HAS_STRCASECMP
int strcasecmp _((const char *s1, const char *s2));

int
strcasecmp(s1, s2)
    const char *s1;
    const char *s2;
{
  while (*s1 && *s2 && DOWNCASE(*s1) == DOWNCASE(*s2))
    s1++, s2++;

  return (DOWNCASE(*s1) - DOWNCASE(*s2));
}

#endif
#endif

void
write_topic(out, p, l)
    FILE *out;
    long int p;
    int l;
{
  tlist *cur, *nextptr;
  help_indx *temp;
  for (cur = top; cur; cur = nextptr) {
    nextptr = cur->next;
    if (num_topics >= top_topics) {
      top_topics += top_topics / 2 + 20;
      if (topics)
	topics = (help_indx *) realloc(topics, top_topics * sizeof(help_indx));
      else
	topics = (help_indx *) malloc(top_topics * sizeof(help_indx));
      if (!topics) {
	fprintf(stderr, "out of memory\n");
	exit(-1);
      }
    }
    temp = &topics[num_topics++];
    temp->pos = p;
    temp->len = l;
    strcpy(temp->topic, cur->topic);
    free(cur);
  }
  top = NULL;
}

static void
add_topic(name)
    char *name;
{
  tlist *cur;
  cur = (tlist *) malloc(sizeof(tlist));
  strcpy(cur->topic, name);
  cur->next = top;
  top = cur;
}

static int topic_cmp _((const void *s1, const void *s2));
static int
topic_cmp(s1, s2)
     const void *s1, *s2;
{
  const help_indx *a = s1;
  const help_indx *b = s2;

  return strcasecmp(a->topic, b->topic);

}

void
flush_topics(out)
    FILE *out;
{
  qsort(topics, num_topics, sizeof(help_indx), topic_cmp);
  if ((Size_t) fwrite(topics, sizeof(help_indx), num_topics, out) < num_topics) {
    fprintf(stderr, "error writing\n");
    exit(-1);
  }
}


char line[LINE_SIZE + 1];
#ifdef EMBEDDED_MKINDX
int
makeindex(char const *inputfile, char const *outputfile)
#else
int
main(argc, argv)
    int argc;
    char **argv;
#endif
{
  long bigpos, pos = 0;
  int in_topic;
  int i, n, lineno, ntopics;
  char *s, *topic;
  char the_topic[TOPIC_NAME_LEN + 1];
  FILE *rfp, *wfp;
#ifndef EMBEDDED_MKINDX
  char c;
#endif

#ifdef EMBEDDED_MKINDX
  /* Quietly ignore null values for the file */
  if (!inputfile || !*inputfile || !outputfile || !*outputfile)
    return 0;
  if ((rfp = fopen(inputfile, "rb")) == NULL) {
    fprintf(stderr, "can't open %s for reading\n", inputfile);
    exit(-1);
  }
  if ((wfp = fopen(outputfile, "wb")) == NULL) {
    fprintf(stderr, "can't open %s for writing\n", outputfile);
    exit(-1);
  }
  fprintf(stderr, "Indexing file %s\n", inputfile);
#else
  if (argc < 2 || argc > 3) {
    printf("Usage:\tmkindx <file_to_be_indexed> <output_index_filename>\n");
    exit(-1);
  }
  if ((rfp = fopen(argv[1], "rb")) == NULL) {
    fprintf(stderr, "can't open %s for reading\n", argv[1]);
    exit(-1);
  }
  if ((wfp = fopen(argv[2], "wb")) == NULL) {
    fprintf(stderr, "can't open %s for writing\n", argv[2]);
    exit(-1);
  }
#endif
  topics = NULL;
  num_topics = 0;
  top_topics = 0;
  bigpos = 0L;
  lineno = 0;
  ntopics = 0;

#ifndef EMBEDDED_MKINDX
  /* try to prevent accidental clobbering if user reverses file order */
  c = getc(rfp);
  if (c != '&') {
    printf("%s is probably not a text file.\n", argv[1]);
    printf("Usage:\tmkindx <file_to_be_indexed> <output_index_filename>\n");
    fclose(rfp);
    fclose(wfp);
    exit(-1);
  }
  ungetc(c, rfp);
#endif
  in_topic = FALSE;

  while (fgets(line, LINE_SIZE, rfp) != NULL) {
    ++lineno;

    n = strlen(line);
    if (line[n - 1] != '\n') {
      fprintf(stderr, "line %d: line too long\n", lineno);
    }
    if (line[0] == '&') {
      ++ntopics;
      if (!in_topic) {
	/* Finish up last entry */
	if (ntopics > 1) {
	  write_topic(wfp, pos, bigpos - pos);
	}
	in_topic = TRUE;
      }
      /* parse out the topic */
      /* Get the beginning of the topic string */
      for (topic = &line[1];
	   (*topic == ' ' || *topic == '\t') && *topic != '\0'; topic++) ;

      /* Get the topic */
      strcpy(the_topic, "");
      for (i = -1, s = topic; *s != '\n' && *s != '\0'; s++) {
	if (i >= TOPIC_NAME_LEN - 1)
	  break;
	if (*s != ' ' || the_topic[i] != ' ')
	  the_topic[++i] = *s;
      }
      the_topic[++i] = '\0';
      add_topic(the_topic);
    } else {
      if (in_topic) {
	pos = bigpos;
      }
      in_topic = FALSE;
    }
#ifdef EMBEDDED_MKINDX
    bigpos = ftell(rfp);
#else
    bigpos += (long) n;
#endif
  }

  /* Handle last topic */
  write_topic(wfp, pos, bigpos - pos);
  flush_topics(wfp);

  fclose(rfp);
  fclose(wfp);

#ifdef EMBEDDED_MKINDX
  fprintf(stderr, "    %d topics indexed\n", ntopics);
  return 0;
#else
  printf("%d topics indexed\n", ntopics);
  exit(0);
#endif
}
