#include "copyrite.h"

#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "mymalloc.h"
#include "myregexp.h"
#include "confmagic.h"

#define ALPHANUM_LIST  0
#define NUMERIC_LIST   1
#define DBREF_LIST     2
#ifdef FLOATING_POINTS
#define FLOAT_LIST     3
#else
#define FLOAT_LIST     1
#endif

#define MAX_SORTSIZE (BUFFER_LEN / 2)

static char *next_token _((char *str, char sep));
static int list2arr _((char *r[], int max, char *list, char sep));
static void arr2list _((char *r[], int max, char *list, char **lp, char sep));
static void swap _((char **p, char **q));
static int autodetect_list _((char **ptrs, int nptrs));
static int get_list_type _((char **args, int nargs, int type_pos, char **ptrs, int nptrs));
static int a_comp _((const void *s1, const void *s2));
static int i_comp _((const void *s1, const void *s2));
static int f_comp _((const void *s1, const void *s2));
static int u_comp _((const void *s1, const void *s2));
void do_gensort _((char **s, int n, int sort_type));
static void sane_qsort _((void **array, int left, int right, int (*compare) (const void *, const void *)));
static void do_itemfuns _((char *buff, char **bp, char *str, char *num, char *word, char *sep, int flag));

#ifdef CAN_NEWSTYLE
static char *
next_token(char *str, char sep)
#else
static char *
next_token(str, sep)
    char *str;
    char sep;
#endif
{
  /* move pointer to start of the next token */

  while (*str && (*str != sep))
    str++;
  if (!*str)
    return NULL;
  str++;
  if (sep == ' ') {
    while (*str == sep)
      str++;
  }
  return str;
}

#ifdef CAN_NEWSTYLE
static int
list2arr(char *r[], int max, char *list, char sep)
#else
static int
list2arr(r, max, list, sep)
    char *r[];
    int max;
    char *list;
    char sep;
#endif
{
  /* chops up a list of words into an array of words. The list is
   * destructively modified.
   */
  char *p;
  int i;

  list = trim_space_sep(list, sep);
  p = split_token(&list, sep);
  for (i = 0; p && (i < max); i++, p = split_token(&list, sep))
    r[i] = p;
  return i;
}

#ifdef CAN_NEWSTYLE
static void
arr2list(char *r[], int max, char *list, char **lp, char sep)
#else
static void
arr2list(r, max, list, lp, sep)
    char *r[];
    int max;
    char *list;
    char **lp;
    char sep;
#endif
{
  int i;

  if (!max)
    return;

  safe_str(r[0], list, lp);
  for (i = 1; i < max; i++) {
    safe_chr(sep, list, lp);
    safe_str(r[i], list, lp);
  }
  **lp = '\0';
}

static void
swap(p, q)
    char **p;
    char **q;
{
  /* swaps two pointers to strings */

  char *temp;
  temp = *p;
  *p = *q;
  *q = temp;
}

/* ARGSUSED */
FUNCTION(fun_munge)
{
  /* This is a function which takes three arguments. The first is
   * an obj-attr pair referencing a u-function to be called. The
   * other two arguments are lists. The first list is passed to the
   * u-function.  The second list is then rearranged to match the
   * order of the first list as returned from the u-function.
   * This rearranged list is returned by MUNGE.
   * A fourth argument (separator) is optional.
   */

  char list1[BUFFER_LEN], *lp, rlist[BUFFER_LEN], *rp;
  char **ptrs1, **ptrs2, **results;
  int i, j, nptrs1, nptrs2, nresults;
  dbref thing;
  ATTR *attrib;
  char sep;
  int first;

  if (!delim_check(buff, bp, nargs, args, 4, &sep))
    return;

  /* find our object and attribute */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib ||
      !Can_Read_Attr(executor, thing, attrib))
    return;
  if (!CanEval(executor, thing))
    return;

  /* Copy the first list, since we need to pass it to two destructive
   * routines.
   */

  strcpy(list1, args[1]);

  /* Break up the two lists into their respective elements. */

  ptrs1 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  ptrs2 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  if (!ptrs1 || !ptrs2)
    panic("Unable to allocated memory in fun_munge");
  nptrs1 = list2arr(ptrs1, MAX_SORTSIZE, args[1], sep);
  nptrs2 = list2arr(ptrs2, MAX_SORTSIZE, args[2], sep);

  if (nptrs1 != nptrs2) {
    safe_str("#-1 LISTS MUST BE OF EQUAL SIZE", buff, bp);
    mush_free((Malloc_t) ptrs1, "ptrarray");
    mush_free((Malloc_t) ptrs2, "ptrarray");
    return;
  }
  /* Call the user function */

  lp = list1;
  rp = rlist;
  do_userfn(rlist, &rp, thing, attrib, 1, &lp,
	    executor, caller, enactor, pe_info);
  *rp = '\0';

  /* Now that we have our result, put it back into array form. Search
   * through list1 until we find the element position, then copy the
   * corresponding element from list2.  Mark used elements with
   * NULL to handle duplicates
   */
  results = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  if (!results)
    panic("Unable to allocate memory in fun_munge");
  nresults = list2arr(results, MAX_SORTSIZE, rlist, sep);

  first = 1;
  for (i = 0; i < nresults; i++) {
    for (j = 0; j < nptrs1; j++) {
      if (ptrs2[j] && !strcmp(results[i], ptrs1[j])) {
	if (first)
	  first = 0;
	else
	  safe_chr(sep, buff, bp);
	safe_str(ptrs2[j], buff, bp);
	ptrs2[j] = NULL;
	break;
      }
    }
  }
  mush_free((Malloc_t) ptrs1, "ptrarray");
  mush_free((Malloc_t) ptrs2, "ptrarray");
  mush_free((Malloc_t) results, "ptrarray");
}

/* ARGSUSED */
FUNCTION(fun_elements)
{
  /* Given a list and a list of numbers, return the corresponding
   * elements of the list. elements(ack bar eep foof yay,2 4) = bar foof
   * A separator for the first list is allowed.
   * This code modified slightly from the Tiny 2.2.1 distribution
   */
  int nwords, cur;
  char **ptrs;
  char *wordlist;
  char *s, *r, sep;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  ptrs = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  wordlist = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!ptrs || !wordlist)
    panic("Unable to allocate memory in fun_elements");

  /* Turn the first list into an array. */
  strcpy(wordlist, args[0]);
  nwords = list2arr(ptrs, MAX_SORTSIZE, wordlist, sep);

  s = trim_space_sep(args[1], ' ');

  /* Go through the second list, grabbing the numbers and finding the
   * corresponding elements.
   */
  r = split_token(&s, ' ');
  cur = atoi(r) - 1;
  if ((cur >= 0) && (cur < nwords) && ptrs[cur]) {
    safe_str(ptrs[cur], buff, bp);
  }
  while (s) {
    r = split_token(&s, ' ');
    cur = atoi(r) - 1;
    if ((cur >= 0) && (cur < nwords) && ptrs[cur]) {
      safe_chr(sep, buff, bp);
      safe_str(ptrs[cur], buff, bp);
    }
  }
  mush_free((Malloc_t) ptrs, "ptrarray");
  mush_free((Malloc_t) wordlist, "string");
}

/* ARGSUSED */
FUNCTION(fun_matchall)
{
  /* Check each word individually, returning the word number of all
   * that match. If none match, return an empty string.
   */

  int wcount;
  char *r, *s, *b, sep;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  wcount = 1;
  s = trim_space_sep(args[0], sep);
  b = *bp;
  do {
    r = split_token(&s, sep);
    if (quick_wild(args[1], r)) {
      if (*bp != b)
	safe_chr(' ', buff, bp);
      safe_str(unparse_integer(wcount), buff, bp);
    }
    wcount++;
  } while (s);
}

/* ARGSUSED */
FUNCTION(fun_graball)
{
  /* Check each word individually, returning all that match.
   * If none match, return an empty string.  This is to grab()
   * what matchall() is to match().
   */

  char *r, *s, *b, sep;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  s = trim_space_sep(args[0], sep);
  b = *bp;
  do {
    r = split_token(&s, sep);
    if (quick_wild(args[1], r)) {
      if (*bp != b)
	safe_chr(sep, buff, bp);
      safe_str(r, buff, bp);
    }
  } while (s);
}



/* ARGSUSED */
FUNCTION(fun_fold)
{
  /* iteratively evaluates an attribute with a list of arguments and
   * optional base case. With no base case, the first list element is
   * passed as %0, and the second as %1. The attribute is then evaluated
   * with these args. The result is then used as %0, and the next arg as
   * %1. Repeat until no elements are left in the list. The base case 
   * can provide a starting point.
   */

  dbref thing;
  ATTR *attrib;
  char const *abuf, *ap;
  char *result, *rp, *rsave;
  char *cp;
  char *tptr[2];
  char sep;

  if (!delim_check(buff, bp, nargs, args, 4, &sep))
    return;

  /* find our object and attribute */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib) {
    safe_str("#-1 NOT FOUND", buff, bp);
    return;
  }
  if (!Can_Read_Attr(executor, thing, attrib)) {
    safe_str("#-1 NO PERMISSION TO GET ATTRIBUTE", buff, bp);
    return;
  }
  if (!CanEval(executor, thing)) {
    safe_str("#-1 NO PERMISSION TO RUN ATTRIBUTE", buff, bp);
    return;
  }
  /* Now we can go to work */
  result = (char *) mush_malloc(BUFFER_LEN, "string");
  rsave = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!result || !rsave)
    panic("Unable to allocate memory in fun_fold");

  abuf = safe_uncompress(attrib->value);

  /* save our stack */
  tptr[0] = wenv[0];
  tptr[1] = wenv[1];

  cp = args[1];

  /* If we have three or more arguments, the third one is the base case */
  if (nargs >= 3) {
    wenv[0] = args[2];
    wenv[1] = split_token(&cp, sep);
  } else {
    wenv[0] = split_token(&cp, sep);
    wenv[1] = split_token(&cp, sep);
  }
  rp = result;
  ap = abuf;
  process_expression(result, &rp, &ap, thing, executor, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *rp = '\0';
  strcpy(rsave, result);

  /* handle the rest of the cases */
  while (cp && *cp) {
    wenv[0] = rsave;
    wenv[1] = split_token(&cp, sep);
    rp = result;
    ap = abuf;
    process_expression(result, &rp, &ap, thing, executor, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    *rp = '\0';
    strcpy(rsave, result);
  }
  safe_str(rsave, buff, bp);

  /* restore the stack */
  wenv[0] = tptr[0];
  wenv[1] = tptr[1];

  free((Malloc_t) abuf);
  mush_free((Malloc_t) result, "string");
  mush_free((Malloc_t) rsave, "string");
}

/* ARGSUSED */
FUNCTION(fun_filter)
{
  /* take a user-def function and a list, and return only those elements
   * of the list for which the function evaluates to 1.
   */

  dbref thing;
  ATTR *attrib;
  char const *abuf, *ap;
  char result[BUFFER_LEN], *rp;
  char *cp;
  char *tptr;
  char sep;
  int first;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  /* find our object and attribute */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib) {
    safe_str("#-1 NOT FOUND", buff, bp);
    return;
  }
  if (!Can_Read_Attr(executor, thing, attrib)) {
    safe_str("#-1 NO PERMISSION TO GET ATTRIBUTE", buff, bp);
    return;
  }
  if (!CanEval(executor, thing)) {
    safe_str("#-1 NO PERMISSION TO RUN ATTRIBUTE", buff, bp);
    return;
  }
  abuf = safe_uncompress(attrib->value);

  tptr = wenv[0];

  cp = trim_space_sep(args[1], sep);
  first = 1;
  while (cp && *cp) {
    wenv[0] = split_token(&cp, sep);
    ap = abuf;
    rp = result;
    process_expression(result, &rp, &ap, thing, executor, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    if (*result == '1') {
      if (first)
	first = 0;
      else
	safe_chr(sep, buff, bp);
      safe_str(wenv[0], buff, bp);
    }
  }

  wenv[0] = tptr;

  free((Malloc_t) abuf);
}

/* ARGSUSED */
FUNCTION(fun_shuffle)
{
  /* given a list of words, randomize the order of words. 
   * We do this by taking each element, and swapping it with another
   * element with a greater array index (thus, words[0] can be swapped
   * with anything up to words[n], words[5] with anything between
   * itself and words[n], etc.
   * This is relatively fast - linear time - and reasonably random.
   * Will take an optional delimiter argument.
   */

  char *words[BUFFER_LEN / 2];
  int n, i, j;
  char sep;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;

  /* split the list up, or return if the list is empty */
  if (!*args[0])
    return;
  n = list2arr(words, BUFFER_LEN / 2, args[0], sep);

  /* shuffle it */
  for (i = 0; i < n; i++) {
    j = getrandom(n - i) + i;
    swap(&words[i], &words[j]);
  }

  arr2list(words, n, buff, bp, sep);
}

static int
autodetect_list(ptrs, nptrs)
    char *ptrs[];
    int nptrs;
{
  int sort_type, i;
  char *p;

  sort_type = NUMERIC_LIST;

  for (i = 0; i < nptrs; i++) {
    switch (sort_type) {
    case NUMERIC_LIST:
      if (!is_strict_number(ptrs[i])) {
	/* If we get something non-numeric, switch to an
	 * alphanumeric guess, unless this is the first
	 * element and we have a dbref.
	 */
	if (i == 0) {
	  p = ptrs[i];
	  if (*p++ != NUMBER_TOKEN)
	    return ALPHANUM_LIST;
	  else if (is_strict_number(p))
	    sort_type = DBREF_LIST;
	  else
	    return ALPHANUM_LIST;
	} else {
	  return ALPHANUM_LIST;
	}
      } else if (strchr(ptrs[i], '.'))
	sort_type = FLOAT_LIST;
      break;
#ifdef FLOATING_POINTS
    case FLOAT_LIST:
      if (!is_strict_number(ptrs[i]))
	return ALPHANUM_LIST;
      break;
#endif
    case DBREF_LIST:
      /* If what we get following the '#' sign isn't a number,
       * we sort on alphanumeric.
       */
      p = ptrs[i];
      if (*p++ != NUMBER_TOKEN)
	return ALPHANUM_LIST;
      if (!is_strict_number(p))
	return ALPHANUM_LIST;
      break;
    default:
      return ALPHANUM_LIST;
    }
  }
  return sort_type;
}

static int
get_list_type(args, nargs, type_pos, ptrs, nptrs)
    char *args[];
    int nargs;
    int type_pos;
    char *ptrs[];
    int nptrs;
{
  if (nargs >= type_pos) {
    switch (tolower(*args[type_pos - 1])) {
    case 'a':
      return ALPHANUM_LIST;
    case 'd':
      return DBREF_LIST;
    case 'n':
      return NUMERIC_LIST;
    case 'f':
      return FLOAT_LIST;
    case '\0':
      return autodetect_list(ptrs, nptrs);
    default:
      return ALPHANUM_LIST;
    }
  }
  return autodetect_list(ptrs, nptrs);
}

static int
a_comp(s1, s2)
    const void *s1, *s2;
{
  return strcmp(*(char **) s1, *(char **) s2);
}

typedef struct i_record i_rec;
struct i_record {
  char *str;
  int num;
};

static int
i_comp(s1, s2)
    const void *s1, *s2;
{
  if (((i_rec *) s1)->num > ((i_rec *) s2)->num)
    return 1;
  if (((i_rec *) s1)->num < ((i_rec *) s2)->num)
    return -1;
  return 0;
}

#ifdef FLOATING_POINTS
typedef struct f_record f_rec;
struct f_record {
  char *str;
  NVAL num;
};

static int
f_comp(s1, s2)
    const void *s1, *s2;
{
  if (((f_rec *) s1)->num > ((f_rec *) s2)->num)
    return 1;
  if (((f_rec *) s1)->num < ((f_rec *) s2)->num)
    return -1;
  return 0;
}
#endif				/* FLOATING_POINTS */

static dbref ucomp_executor, ucomp_caller, ucomp_enactor;
static char ucomp_buff[BUFFER_LEN];
static PE_Info *ucomp_pe_info;

static int
u_comp(s1, s2)
    const void *s1, *s2;
{
  char result[BUFFER_LEN], *rp;
  char const *tbuf;
  int n;

  /* Our two arguments are passed as %0 and %1 to the sortby u-function. */

  /* Note that this function is for use in conjunction with our own
   * sane_qsort routine, NOT with the standard library qsort!
   */
  wenv[0] = (char *) s1;
  wenv[1] = (char *) s2;

  /* Run the u-function, which should return a number. */

  tbuf = ucomp_buff;
  rp = result;
  process_expression(result, &rp, &tbuf,
		     ucomp_executor, ucomp_caller, ucomp_enactor,
		     PE_DEFAULT, PT_DEFAULT, ucomp_pe_info);
  n = parse_integer(result);

  return n;
}

void
do_gensort(s, n, sort_type)
    char *s[];
    int n;
    int sort_type;
{
  int i;
#ifdef FLOATING_POINTS
  f_rec *fp;
#endif
  i_rec *ip;

  switch (sort_type) {
  case ALPHANUM_LIST:
    qsort((void *) s, n, sizeof(char *), a_comp);
    break;
  case NUMERIC_LIST:
    ip = (i_rec *) mush_malloc(n * sizeof(i_rec), "do_gensort.int_list");
    for (i = 0; i < n; i++) {
      ip[i].str = s[i];
      ip[i].num = parse_integer(s[i]);
    }
    qsort((void *) ip, n, sizeof(i_rec), i_comp);
    for (i = 0; i < n; i++)
      s[i] = ip[i].str;
    mush_free((Malloc_t) ip, "do_gensort.int_list");
    break;
  case DBREF_LIST:
    ip = (i_rec *) mush_malloc(n * sizeof(i_rec), "do_gensort.dbref_list");
    for (i = 0; i < n; i++) {
      ip[i].str = s[i];
      ip[i].num = parse_dbref(s[i]);
    }
    qsort((void *) ip, n, sizeof(i_rec), i_comp);
    for (i = 0; i < n; i++)
      s[i] = ip[i].str;
    mush_free((Malloc_t) ip, "do_gensort.dbref_list");
    break;
#ifdef FLOATING_POINTS
  case FLOAT_LIST:
    fp = (f_rec *) mush_malloc(n * sizeof(f_rec), "do_gensort.num_list");
    for (i = 0; i < n; i++) {
      fp[i].str = s[i];
      fp[i].num = parse_number(s[i]);
    }
    qsort((void *) fp, n, sizeof(f_rec), f_comp);
    for (i = 0; i < n; i++)
      s[i] = fp[i].str;
    mush_free((Malloc_t) fp, "do_gensort.num_list");
    break;
#endif				/* FLOATING_POINTS */
  }
}

/* ARGSUSED */
FUNCTION(fun_sort)
{
  char *ptrs[MAX_SORTSIZE];
  int nptrs, sort_type;
  char sep;

  if (!nargs || !*args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  nptrs = list2arr(ptrs, MAX_SORTSIZE, args[0], sep);
  sort_type = get_list_type(args, nargs, 2, ptrs, nptrs);
  do_gensort(ptrs, nptrs, sort_type);
  arr2list(ptrs, nptrs, buff, bp, sep);
}

static void
sane_qsort(array, left, right, compare)
    void *array[];
    int left, right;
    int (*compare) _((const void *, const void *));
{
  /* Andrew Molitor's qsort, which doesn't require transitivity between
   * comparisons (essential for preventing crashes due to boneheads
   * who write comparison functions where a > b doesn't mean b < a).
   */
  /* Actually, this sort doesn't require commutivity.
   * Sorting doesn't make sense without transitivity...
   */

  int i, last;
  void *tmp;

loop:
  if (left >= right)
    return;

  /* Pick something at random at swap it into the leftmost slot   */
  /* This is the pivot, we'll put it back in the right spot later */

  i = getrandom(1 + (right - left));
  tmp = array[left + i];
  array[left + i] = array[left];
  array[left] = tmp;

  last = left;
  for (i = left + 1; i <= right; i++) {

    /* Walk the array, looking for stuff that's less than our */
    /* pivot. If it is, swap it with the next thing along     */

    if ((*compare) (array[i], array[left]) < 0) {
      last++;
      if (last == i)
	continue;

      tmp = array[last];
      array[last] = array[i];
      array[i] = tmp;
    }
  }

  /* Now we put the pivot back, it's now in the right spot, we never */
  /* need to look at it again, trust me.                             */

  tmp = array[last];
  array[last] = array[left];
  array[left] = tmp;

  /* At this point everything underneath the 'last' index is < the */
  /* entry at 'last' and everything above it is not < it.          */

  if ((last - left) < (right - last)) {
    sane_qsort(array, left, last - 1, compare);
    left = last + 1;
    goto loop;
  } else {
    sane_qsort(array, last + 1, right, compare);
    right = last - 1;
    goto loop;
  }
}


/* ARGSUSED */
FUNCTION(fun_sortby)
{
  char *ptrs[MAX_SORTSIZE], *tptr[10];
  char *up, sep;
  int nptrs, i;
  dbref thing;
  ATTR *attrib;

  if (!nargs || !*args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  /* Find object and attribute to get sortby function from. */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib ||
      !Can_Read_Attr(executor, thing, attrib))
    return;
  if (!CanEval(executor, thing))
    return;
  up = ucomp_buff;
  safe_str(uncompress(attrib->value), ucomp_buff, &up);
  *up = '\0';

  ucomp_executor = thing;
  ucomp_caller = executor;
  ucomp_enactor = enactor;
  ucomp_pe_info = pe_info;

  /* Save the stack. */
  for (i = 0; i < 10; i++)
    tptr[i] = wenv[i];

  /* Split up the list, sort it, reconstruct it. */
  nptrs = list2arr(ptrs, MAX_SORTSIZE, args[1], sep);
  if (nptrs > 1)		/* pointless to sort less than 2 elements */
    sane_qsort((void *) ptrs, 0, nptrs - 1, u_comp);

  arr2list(ptrs, nptrs, buff, bp, sep);

  /* Restore the stack */
  for (i = 0; i < 10; i++)
    wenv[i] = tptr[i];
}

/* ARGSUSED */
FUNCTION(fun_setunion)
{
  char sep;
  char **a1, **a2;
  char *tempbuff;
  int n1, i, a;

  /* if no lists, then no work */
  if (!*args[0] && !*args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  tempbuff = (char *) mush_malloc(BUFFER_LEN * 2, "string");
  a1 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  a2 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  if (!tempbuff || !a1 || !a2)
    panic("Unable to allocate memory in fun_setunion");
  /* Concat both lists, make array, sort */
  sprintf(tempbuff, "%s%c%s", args[0], sep, args[1]);
  n1 = list2arr(a1, MAX_SORTSIZE, tempbuff, sep);
  do_gensort(a1, n1, ALPHANUM_LIST);

  /* Strip the duplicates and make a2 contain the list */
  a = 0;
  for (i = 0; i < n1; i++) {
    if (((a == 0) || (strcmp(a1[i], a2[a - 1]) != 0)) && (*a1[i])) {
      a2[a] = a1[i];
      a++;
    }
  }

  /* Return our sorted result */
  arr2list(a2, a, buff, bp, sep);
  mush_free((Malloc_t) tempbuff, "string");
  mush_free((Malloc_t) a1, "ptrarray");
  mush_free((Malloc_t) a2, "ptrarray");
}

/* ARGSUSED */
FUNCTION(fun_setinter)
{
  char sep;
  char **a1, **a2;
  int n1, n2, x1, x2, val;

  /* if no lists, then no work */
  if (!*args[0] && !*args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  a1 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  a2 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  if (!a1 || !a2)
    panic("Unable to allocate memory in fun_setunion");

  /* make arrays out of the lists */
  n1 = list2arr(a1, MAX_SORTSIZE, args[0], sep);
  n2 = list2arr(a2, MAX_SORTSIZE, args[1], sep);

  /* sort each array */
  do_gensort(a1, n1, ALPHANUM_LIST);
  do_gensort(a2, n2, ALPHANUM_LIST);

  /* get the first value for the intersection, removing duplicates */
  x1 = x2 = 0;
  while ((val = strcmp(a1[x1], a2[x2]))) {
    if (val < 0) {
      x1++;
      if (x1 >= n1) {
	mush_free((Malloc_t) a1, "ptrarray");
	mush_free((Malloc_t) a2, "ptrarray");
	return;
      }
    } else {
      x2++;
      if (x2 >= n2) {
	mush_free((Malloc_t) a1, "ptrarray");
	mush_free((Malloc_t) a2, "ptrarray");
	return;
      }
    }
  }
  safe_str(a1[x1], buff, bp);
  while (!strcmp(a1[x1], a2[x2])) {
    x1++;
    if (x1 >= n1) {
      mush_free((Malloc_t) a1, "ptrarray");
      mush_free((Malloc_t) a2, "ptrarray");
      return;
    }
  }

  /* get values for the intersection, until at least one list is empty */
  while ((x1 < n1) && (x2 < n2)) {
    while ((val = strcmp(a1[x1], a2[x2]))) {
      if (val < 0) {
	x1++;
	if (x1 >= n1) {
	  mush_free((Malloc_t) a1, "ptrarray");
	  mush_free((Malloc_t) a2, "ptrarray");
	  return;
	}
      } else {
	x2++;
	if (x2 >= n2) {
	  mush_free((Malloc_t) a1, "ptrarray");
	  mush_free((Malloc_t) a2, "ptrarray");
	  return;
	}
      }
    }
    safe_chr(sep, buff, bp);
    safe_str(a1[x1], buff, bp);
    while (!strcmp(a1[x1], a2[x2])) {
      x1++;
      if (x1 >= n1) {
	mush_free((Malloc_t) a1, "ptrarray");
	mush_free((Malloc_t) a2, "ptrarray");
	return;
      }
    }
  }
  mush_free((Malloc_t) a1, "ptrarray");
  mush_free((Malloc_t) a2, "ptrarray");
}

/* ARGSUSED */
FUNCTION(fun_setdiff)
{
  char sep;
  char **a1, **a2;
  int n1, n2, x1, x2, val;

  /* if no lists, then no work */
  if (!*args[0] && !*args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  a1 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  a2 = (char **) mush_malloc(MAX_SORTSIZE * sizeof(char *), "ptrarray");
  if (!a1 || !a2)
    panic("Unable to allocate memory in fun_setunion");

  /* make arrays out of the lists */
  n1 = list2arr(a1, MAX_SORTSIZE, args[0], sep);
  n2 = list2arr(a2, MAX_SORTSIZE, args[1], sep);

  /* sort each array */
  do_gensort(a1, n1, ALPHANUM_LIST);
  do_gensort(a2, n2, ALPHANUM_LIST);

  /* get the first value for the difference, removing duplicates */
  x1 = x2 = 0;
  while ((val = strcmp(a1[x1], a2[x2])) >= 0) {
    if (val > 0) {
      x2++;
      if (x2 >= n2)
	break;
    }
    if (!val) {
      x1++;
      if (x1 >= n1) {
	mush_free((Malloc_t) a1, "ptrarray");
	mush_free((Malloc_t) a2, "ptrarray");
	return;
      }
    }
  }
  safe_str(a1[x1], buff, bp);
  do {
    x1++;
    if (x1 >= n1) {
      mush_free((Malloc_t) a1, "ptrarray");
      mush_free((Malloc_t) a2, "ptrarray");
      return;
    }
  } while (!strcmp(a1[x1], a1[x1 - 1]));

  /* get values for the difference, until at least one list is empty */
  while (x2 < n2) {
    if ((val = strcmp(a1[x1], a2[x2])) < 0) {
      safe_chr(sep, buff, bp);
      safe_str(a1[x1], buff, bp);
    }
    if (val <= 0) {
      do {
	x1++;
	if (x1 >= n1) {
	  mush_free((Malloc_t) a1, "ptrarray");
	  mush_free((Malloc_t) a2, "ptrarray");
	  return;
	}
      } while (!strcmp(a1[x1], a1[x1 - 1]));
    }
    if (val >= 0)
      x2++;
  }

  /* empty out remaining values, still removing duplicates */
  while (x1 < n1) {
    safe_chr(sep, buff, bp);
    safe_str(a1[x1], buff, bp);
    do {
      x1++;
    } while ((x1 < n1) && !strcmp(a1[x1], a1[x1 - 1]));
  }
  mush_free((Malloc_t) a1, "ptrarray");
  mush_free((Malloc_t) a2, "ptrarray");
}

/* ARGSUSED */
FUNCTION(fun_lnum)
{
  NVAL j;
  NVAL start;
  NVAL end;
  char const *osep = " ";

  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  end = parse_number(args[0]);
  if (nargs > 1) {
    if (!is_number(args[1])) {
      safe_str(e_num, buff, bp);
      return;
    }
    start = end;
    end = parse_number(args[1]);
  } else {
    if (end == 0)
      return;			/* Special case - lnum(0) -> blank string */
    end--;
    if (end < 0) {
      safe_str("#-1 NUMBER OUT OF RANGE", buff, bp);
      return;
    }
    start = 0;
  }
  if (nargs > 2) {
    osep = args[2];
  }
  if (start <= end) {
    for (j = start; j <= end; j++) {
      if (j > start)
	safe_str(osep, buff, bp);
      if (safe_str(unparse_number(j), buff, bp))
	break;
    }
  } else {
    for (j = start; j >= end; j--) {
      if (j < start)
	safe_str(osep, buff, bp);
      if (safe_str(unparse_number(j), buff, bp))
	break;
    }
  }
}

/* ARGSUSED */
FUNCTION(fun_first)
{
  /* read first word from a string */

  char *p;
  char sep;

  if (!*args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;

  p = trim_space_sep(args[0], sep);
  safe_str(split_token(&p, sep), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_rest)
{
  char *p;
  char sep;

  if (!*args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;

  p = trim_space_sep(args[0], sep);
  (void) split_token(&p, sep);
  safe_str(p, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_last)
{
  /* read last word from a string */

  char *p, *r;
  char sep;

  if (!*args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;

  p = trim_space_sep(args[0], sep);
  if (!(r = strrchr(p, sep)))
    r = p;
  else
    r++;
  safe_str(r, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_grab)
{
  /* compares two strings with possible wildcards, returns the
   * word matched. Based on the 2.2 version of this function.
   */

  char *r, *s, sep;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  /* Walk the wordstring, until we find the word we want. */
  s = trim_space_sep(args[0], sep);
  do {
    r = split_token(&s, sep);
    if (quick_wild(args[1], r)) {
      safe_str(r, buff, bp);
      return;
    }
  } while (s);
}

/* ARGSUSED */
FUNCTION(fun_match)
{
  /* compares two strings with possible wildcards, returns the
   * word position of the match. Based on the 2.0 version of this
   * function.
   */

  char *s, *r;
  char sep;
  int wcount = 1;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  /* Walk the wordstring, until we find the word we want. */
  s = trim_space_sep(args[0], sep);
  do {
    r = split_token(&s, sep);
    if (quick_wild(args[1], r)) {
      safe_str(unparse_integer(wcount), buff, bp);
      return;
    }
    wcount++;
  } while (s);
  safe_chr('0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_wordpos)
{
  int charpos, i;
  char *cp, *tp, *xp;
  char sep;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }
  charpos = parse_integer(args[1]);
  cp = args[0];
  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  if ((charpos <= 0) || ((Size_t) charpos > strlen(cp))) {
    safe_str("#-1", buff, bp);
    return;
  }
  tp = cp + charpos - 1;
  cp = trim_space_sep(cp, sep);
  xp = split_token(&cp, sep);
  for (i = 1; xp; i++) {
    if (tp < (xp + strlen(xp)))
      break;
    xp = split_token(&cp, sep);
  }
  safe_str(unparse_integer(i), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_extract)
{
  char sep;
  int start, len;
  char *s, *r;

  if (!is_integer(args[1]) || !is_integer(args[2])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  s = args[0];
  start = parse_integer(args[1]);
  len = parse_integer(args[2]);
  if (!delim_check(buff, bp, nargs, args, 4, &sep))
    return;

  if ((start < 1) || (len < 1))
    return;

  /* Go to the start of the token we're interested in. */
  start--;
  s = trim_space_sep(s, sep);
  while (start && s) {
    s = next_token(s, sep);
    start--;
  }

  if (!s || !*s)		/* ran off the end of the string */
    return;

  /* Find the end of the string that we want. */
  r = s;
  len--;
  while (len && s) {
    s = next_token(s, sep);
    len--;
  }

  /* Chop off the end, and copy. No length checking needed. */
  if (s && *s)
    (void) split_token(&s, sep);
  safe_str(r, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_cat)
{
  int i;

  safe_str(args[0], buff, bp);
  for (i = 1; i < nargs; i++) {
    safe_chr(' ', buff, bp);
    safe_str(args[i], buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_remove)
{
  char *s, *sp;
  char sep;

  /* zap word from string */

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  if (strchr(args[1], sep)) {
    safe_str("#-1 CAN ONLY DELETE ONE ELEMENT", buff, bp);
    return;
  }
  s = args[0];

  sp = split_token(&s, sep);
  if (!strcmp(sp, args[1])) {
    sp = split_token(&s, sep);
    safe_str(sp, buff, bp);
  } else {
    safe_str(sp, buff, bp);
    while (s && strcmp(sp = split_token(&s, sep), args[1])) {
      safe_chr(sep, buff, bp);
      safe_str(sp, buff, bp);
    }
  }
  while (s) {
    sp = split_token(&s, sep);
    safe_chr(sep, buff, bp);
    safe_str(sp, buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_items)
{
  /* the equivalent of WORDS for an arbitrary separator */
  /* This differs from WORDS in its treatment of the space
   * separator.
   */

  char *s = args[0];
  char c = *args[1];
  int count = 1;

  if (c == '\0')
    c = ' ';

  while ((s = strchr(s, c))) {
    count++;
    s++;
  }

  safe_str(unparse_integer(count), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_element)
{
  /* the equivalent of MEMBER for an arbitrary separator */
  /* This differs from MEMBER in its use of quick_wild()
   * instead of strcmp().
   */

  char *s, *t;
  char c;
  int el;

  c = *args[2];

  if (c == '\0')
    c = ' ';
  if (strchr(args[1], c)) {
    safe_str("#-1 CAN ONLY TEST ONE ELEMENT", buff, bp);
    return;
  }
  s = args[0];
  el = 1;

  do {
    t = s;
    s = seek_char(t, c);
    if (*s)
      *s++ = '\0';
    if (quick_wild(args[1], t)) {
      safe_str(unparse_integer(el), buff, bp);
      return;
    }
    el++;
  } while (*s);

  safe_chr('0', buff, bp);	/* no match */
}

/* ARGSUSED */
FUNCTION(fun_index)
{
  /* more or less the equivalent of EXTRACT for an arbitrary separator */
  /* This differs from EXTRACT in its handling of space separators. */

  int start, end;
  char c;
  char *s, *p;

  if (!is_integer(args[2]) || !is_integer(args[3])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  s = args[0];
  c = *args[1];
  if (!c)
    c = ' ';

  start = parse_integer(args[2]);
  end = parse_integer(args[3]);

  if ((start < 1) || (end < 1) || (*s == '\0'))
    return;

  /* move s to the start of the item we want */
  while (--start) {
    if (!(s = strchr(s, c)))
      return;
    s++;
  }

  /* skip just spaces, not tabs or newlines, since people may MUSHcode things
   * like "%r%tPolgara %r%tDurnik %r%tJavelin"
   */
  while (*s == ' ')
    s++;
  if (!*s)
    return;

  /* now figure out where to end the string */
  p = s + 1;
  /* we may already be pointing to a separator */
  if (*s == c)
    end--;
  while (end--)
    if (!(p = strchr(p, c)))
      break;
    else
      p++;

  if (p)
    p--;
  else
    p = s + strlen(s);

  /* trim trailing spaces (just true spaces) */
  while ((p > s) && (p[-1] == ' '))
    p--;
  *p = '\0';

  safe_str(s, buff, bp);
}

static void
do_itemfuns(buff, bp, str, num, word, sep, flag)
    char *buff;			/* the return buffer */
    char **bp;			/* the active point in the return buffer */
    char *str;			/* the original string */
    char *num;			/* the element number */
    char *word;			/* word to insert or replace */
    char *sep;			/* the separator */
    int flag;			/* op -- 0 delete, 1 replace, 2 insert */
{
  char c;
  int el, count;
  char *sptr, *eptr;

  if (!is_integer(num)) {
    safe_str(e_int, buff, bp);
    return;
  }
  el = parse_integer(num);

  /* figure out the separator character */
  if (sep && *sep)
    c = *sep;
  else
    c = ' ';

  /* we can't remove anything before the first position */
  if (el < 1) {
    safe_str(str, buff, bp);
    return;
  }
  sptr = str;
  eptr = strchr(sptr, c);
  count = 1;

  /* go to the correct item in the string */
  /* Loop invariant: if sptr and eptr are not NULL, eptr points to
   * the count'th instance of c in str, and sptr is the beginning of
   * the count'th item. */
  while (eptr && (count < el)) {
    sptr = eptr + 1;
    eptr = strchr(sptr, c);
    count++;
  }

  if (!eptr && (count < el)) {
    /* we've run off the end of the string without finding anything */
    safe_str(str, buff, bp);
    return;
  }
  /* now find the end of that element */
  if (sptr != str)
    sptr[-1] = '\0';

  switch (flag) {
  case 0:
    /* deletion */
    if (!eptr) {		/* last element in the string */
      if (el != 1)
	safe_str(str, buff, bp);
    } else if (sptr == str) {	/* first element in the string */
      eptr++;			/* chop leading separator */
      safe_str(eptr, buff, bp);
    } else {
      safe_str(str, buff, bp);
      safe_str(eptr, buff, bp);
    }
    break;
  case 1:
    /* replacing */
    if (!eptr) {		/* last element in string */
      if (el != 1) {
	safe_str(str, buff, bp);
	safe_chr(c, buff, bp);
      }
      safe_str(word, buff, bp);
    } else if (sptr == str) {	/* first element in string */
      safe_str(word, buff, bp);
      safe_str(eptr, buff, bp);
    } else {
      safe_str(str, buff, bp);
      safe_chr(c, buff, bp);
      safe_str(word, buff, bp);
      safe_str(eptr, buff, bp);
    }
    break;
  case 2:
    /* insertion */
    if (sptr == str) {		/* first element in string */
      safe_str(word, buff, bp);
      safe_chr(c, buff, bp);
      safe_str(str, buff, bp);
    } else {
      safe_str(str, buff, bp);
      safe_chr(c, buff, bp);
      safe_str(word, buff, bp);
      safe_chr(c, buff, bp);
      safe_str(sptr, buff, bp);
    }
    break;
  }
}


/* ARGSUSED */
FUNCTION(fun_ldelete)
{
  /* delete a word at position X of a list */

  do_itemfuns(buff, bp, args[0], args[1], NULL, args[2], 0);
}

/* ARGSUSED */
FUNCTION(fun_replace)
{
  /* replace a word at position X of a list */

  do_itemfuns(buff, bp, args[0], args[1], args[2], args[3], 1);
}

/* ARGSUSED */
FUNCTION(fun_insert)
{
  /* insert a word at position X of a list */

  do_itemfuns(buff, bp, args[0], args[1], args[2], args[3], 2);
}

/* ARGSUSED */
FUNCTION(fun_member)
{
  char *s, *t;
  char sep;
  int el;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  if (strchr(args[1], sep)) {
    safe_str("#-1 CAN ONLY TEST ONE ELEMENT", buff, bp);
    return;
  }
  s = trim_space_sep(args[0], sep);
  el = 1;

  do {
    t = split_token(&s, sep);
    if (!strcmp(args[1], t)) {
      safe_str(unparse_integer(el), buff, bp);
      return;
    }
    el++;
  } while (s);

  safe_chr('0', buff, bp);	/* not found */
}

/* ARGSUSED */
FUNCTION(fun_before)
{
  char *p;

  if (!*args[1])
    p = strchr(args[0], ' ');
  else
    p = strstr(args[0], args[1]);
  if (p)
    *p = '\0';
  safe_str(args[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_after)
{
  char *p;

  if (!*args[1]) {
    args[1][0] = ' ';
    args[1][1] = '\0';
  }
  p = strstr(args[0], args[1]);
  if (p)
    safe_str(p + strlen(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_revwords)
{
  char *words[BUFFER_LEN / 2];
  char *p;
  int count;
  char sep;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;

  count = 0;
  p = args[0];
  while ((words[count] = split_token(&p, sep)))
    count++;

  safe_str(words[--count], buff, bp);
  while (count) {
    safe_chr(sep, buff, bp);
    safe_str(words[--count], buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_words)
{
  char sep;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;
  safe_str(unparse_integer(do_wordcount(trim_space_sep(args[0], sep), sep)),
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_splice)
{
  /* like MERGE(), but does it for a word */

  char *s0, *s1, *s2;
  char *p0, *p1;
  char sep;

  if (!delim_check(buff, bp, nargs, args, 4, &sep))
    return;

  s0 = trim_space_sep(args[0], sep);
  s1 = trim_space_sep(args[1], sep);
  s2 = trim_space_sep(args[2], sep);

  /* length checks */
  if (!*args[2]) {
    safe_str("#-1 NEED A WORD", buff, bp);
    return;
  }
  if (do_wordcount(s2, sep) != 1) {
    safe_str("#-1 TOO MANY WORDS", buff, bp);
    return;
  }
  if (do_wordcount(s0, sep) != do_wordcount(s1, sep)) {
    safe_str("#-1 NUMBER OF WORDS MUST BE EQUAL", buff, bp);
    return;
  }
  /* loop through the two lists */
  p0 = split_token(&s0, sep);
  p1 = split_token(&s1, sep);
  safe_str(strcmp(p0, s2) ? p0 : p1, buff, bp);
  while (s0) {
    p0 = split_token(&s0, sep);
    p1 = split_token(&s1, sep);
    safe_chr(sep, buff, bp);
    safe_str(strcmp(p0, s2) ? p0 : p1, buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_iter)
{
  /* Based on the TinyMUSH 2.0 code for this function. Please note that
   * arguments to this function are passed _unparsed_.
   */

  char sep;
  char *outsep, *list;
  char *tbuf1, *tbuf2, *lp;
  char const *sp;
  int place;

  if (nargs >= 3) {
    /* We have a delimiter. We've got to parse the third arg in place */
    char insep[BUFFER_LEN];
    char *isep = insep;
    const char *arg3 = args[2];
    process_expression(insep, &isep, &arg3, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    *isep = '\0';
    strcpy(args[2], insep);
  }
  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  outsep = (char *) mush_malloc(BUFFER_LEN, "string");
  list = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!outsep || !list)
    panic("Unable to allocated memory in fun_iter");
  if (nargs < 4)
    strcpy(outsep, " ");
  else {
    const char *arg4 = args[3];
    char *osep = outsep;
    process_expression(outsep, &osep, &arg4, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    *osep = '\0';
  }
  lp = list;
  sp = args[0];
  process_expression(list, &lp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *lp = '\0';
  lp = trim_space_sep(list, sep);
  if (!*lp) {
    mush_free((Malloc_t) outsep, "string");
    mush_free((Malloc_t) list, "string");
    return;
  }
  place = 0;
  while (lp) {
    if (place)
      safe_str(outsep, buff, bp);
    place++;
    tbuf1 = split_token(&lp, sep);
    tbuf2 = replace_string("##", tbuf1, args[1]);
    tbuf1 = replace_string("#@", unparse_integer(place), tbuf2);
    mush_free((Malloc_t) tbuf2, "replace_string.buff");
    sp = tbuf1;
    process_expression(buff, bp, &sp, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    mush_free((Malloc_t) tbuf1, "replace_string.buff");
  }
  mush_free((Malloc_t) outsep, "string");
  mush_free((Malloc_t) list, "string");
}

/* ARGSUSED */
FUNCTION(fun_map)
{
  /* Like iter(), but calls an attribute with list elements as %0 instead.
   * If the attribute is not found, null is returned, NOT an error.
   * This function takes delimiters.
   */

  dbref thing;
  ATTR *attrib;
  char const *asave, *ap;
  char *lp;
  char *tptr;
  char sep;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  lp = trim_space_sep(args[1], sep);
  if (!*lp)
    return;

  /* find our object and attribute */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib || !Can_Read_Attr(executor, thing, attrib))
    return;
  if (!CanEval(executor, thing))
    return;

  asave = safe_uncompress(attrib->value);

  /* save our stack */
  tptr = wenv[0];

  wenv[0] = split_token(&lp, sep);
  ap = asave;
  process_expression(buff, bp, &ap, thing, executor, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  while (lp) {
    safe_chr(sep, buff, bp);
    wenv[0] = split_token(&lp, sep);
    ap = asave;
    process_expression(buff, bp, &ap, thing, executor, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
  }

  free((Malloc_t) asave);
  wenv[0] = tptr;
}


/* ARGSUSED */
FUNCTION(fun_mix)
{
  /* Like map(), but goes through two lists, passing them as %0 and %1.
   * If the attribute is not found, null is returned, NOT an error.
   * This function takes delimiters.
   */

  dbref thing;
  ATTR *attrib;
  char const *asave, *ap;
  char *l1p, *l2p;
  char *tptr[2];
  char sep;

  if (!delim_check(buff, bp, nargs, args, 4, &sep))
    return;

  l1p = trim_space_sep(args[1], sep);
  l2p = trim_space_sep(args[2], sep);

  if (do_wordcount(l1p, sep) != do_wordcount(l2p, sep)) {
    safe_str("#-1 LISTS MUST BE OF EQUAL SIZE", buff, bp);
    return;
  }
  if (!*l1p)
    return;

  /* find our object and attribute */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib || !Can_Read_Attr(executor, thing, attrib))
    return;
  if (!CanEval(executor, thing))
    return;

  asave = safe_uncompress(attrib->value);

  /* save our stack */
  tptr[0] = wenv[0];
  tptr[1] = wenv[1];

  wenv[0] = split_token(&l1p, sep);
  wenv[1] = split_token(&l2p, sep);
  ap = asave;
  process_expression(buff, bp, &ap, thing, executor, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  while (l1p) {
    safe_chr(sep, buff, bp);
    wenv[0] = split_token(&l1p, sep);
    wenv[1] = split_token(&l2p, sep);
    ap = asave;
    process_expression(buff, bp, &ap, thing, executor, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
  }

  free((Malloc_t) asave);
  wenv[0] = tptr[0];
  wenv[1] = tptr[1];
}

/* ARGSUSED */
FUNCTION(fun_table)
{
  /* TABLE(list, field_width, line_length, delimiter, output sep)
   * Given a list, produce a table (a column'd list)
   * Optional parameters: field width, line length, delimiter, output sep
   * Number of columns = line length / (field width+1)
   */
  int line_length = 78;
  int field_width = 10;
  int col = 0;
  int spaces;
  char sep, osep, *cp, *t;
  char tbuf1[BUFFER_LEN];

  if (!delim_check(buff, bp, nargs, args, 5, &osep))
    return;
  if ((nargs == 5) && !*args[4])
    osep = 0;

  if (!delim_check(buff, bp, nargs, args, 4, &sep))
    return;

  if (nargs > 2) {
    if (!is_integer(args[2])) {
      safe_str(e_ints, buff, bp);
      return;
    }
    line_length = parse_integer(args[2]);
    if (line_length < 2)
      line_length = 2;
  }
  if (nargs > 1) {
    if (!is_integer(args[1])) {
      safe_str(e_ints, buff, bp);
      return;
    }
    field_width = parse_integer(args[1]);
    if (field_width < 1)
      field_width = 1;
    if (field_width >= BUFFER_LEN)
      field_width = BUFFER_LEN - 1;
  }
  if (field_width >= line_length)
    field_width = line_length - 1;

  /* Split out each token, truncate/pad it to field_width, and pack
   * it onto the line. When the line would go over line_length,
   * send a return
   */

  cp = trim_space_sep(args[0], sep);
  if (!*cp)
    return;

  col = field_width + !!osep;
  t = split_token(&cp, sep);
  strcpy(tbuf1, t);
  tbuf1[field_width] = '\0';
  safe_str(tbuf1, buff, bp);
  for (spaces = field_width - ansi_strlen(t); spaces > 0; spaces--)
    safe_chr(' ', buff, bp);

  while (cp) {
    col += field_width + !!osep;
    if (col > line_length) {
      safe_str("\r\n", buff, bp);
      col = field_width + !!osep;
    } else {
      if (osep)
	safe_chr(osep, buff, bp);
    }
    t = split_token(&cp, sep);
    strcpy(tbuf1, t);
    tbuf1[field_width] = '\0';
    safe_str(tbuf1, buff, bp);
    for (spaces = field_width - ansi_strlen(t); spaces > 0; spaces--)
      safe_chr(' ', buff, bp);
  }
}


FUNCTION(fun_regmatch)
{
/* ---------------------------------------------------------------------------
 * fun_regmatch: Return 0 or 1 depending on whether or not a regular
 * expression matches a string. If a third argument is specified, dump
 * the results of a regexp pattern match into a set of arbitrary r()-registers.
 *
 * regmatch(string, pattern, list of registers)
 * If the number of matches exceeds the registers, those bits are tossed
 * out.
 * If -1 is specified as a register number, the matching bit is tossed.
 * Therefore, if the list is "-1 0 3 5", the regexp $0 is tossed, and
 * the regexp $1, $2, and $3 become r(0), r(3), and r(5), respectively.
 *
 * Based on fun_regmatch from TinyMUSH 2.2.4
 */
  int i, nqregs, curq, len;
  char *qregs[NSUBEXP];
  regexp *re;
  int matched;

  if ((re = regcomp(args[1])) == NULL) {
    /* Matching error. */
    safe_str("#-1 REGEXP ERROR: ", buff, bp);
    safe_str((const char *) regexp_errbuf, buff, bp);
    return;
  }
  matched = (int) regexec(re, args[0]);
  safe_str(unparse_integer(matched), buff, bp);

  /* If we don't have a third argument, we're done. */
  if (nargs < 3) {
    mush_free(re, "regexp");
    return;
  }
  /* We need to parse the list of registers.  Anything that we don't parse
   * is assumed to be -1.  If we didn't match, or the match went wonky,
   * then set the register to empty.  Otherwise, fill the register with
   * the subexpression.
   */
  nqregs = list2arr(qregs, NSUBEXP, args[2], ' ');
  for (i = 0; i < nqregs; i++) {
    if (qregs[i] && *qregs[i] && is_integer(qregs[i]))
      curq = parse_integer(qregs[i]);
    else
      curq = -1;
    if (curq < 0 || curq >= 10)
      continue;
    if (!matched || !re->startp[i] || !re->endp[i])
      renv[curq][0] = '\0';
    else {
      len = re->endp[i] - re->startp[i];
      if (len > BUFFER_LEN - 1)
	len = BUFFER_LEN - 1;
      if (len < 0)
	len = 0;
      strncpy(renv[curq], re->startp[i], len);
      renv[curq][len] = '\0';	/* must null-terminate */
    }
  }
  mush_free(re, "regexp");
}

FUNCTION(fun_regmatchi)
{
/* ---------------------------------------------------------------------------
 * fun_regmatchi: Return 0 or 1 depending on whether or not a regular
 * expression matches a string case-insensitively. 
 * If a third argument is specified, dump
 * the results of a regexp pattern match into a set of arbitrary r()-registers.
 *
 * regmatch(string, pattern, list of registers)
 * If the number of matches exceeds the registers, those bits are tossed
 * out.
 * If -1 is specified as a register number, the matching bit is tossed.
 * Therefore, if the list is "-1 0 3 5", the regexp $0 is tossed, and
 * the regexp $1, $2, and $3 become r(0), r(3), and r(5), respectively.
 *
 * Based on fun_regmatch from TinyMUSH 2.2.4
 */
  int i, nqregs, curq, len;
  char *qregs[NSUBEXP];
  regexp *re;
  int matched;

  if ((re = regcomp(strupper(args[1]))) == NULL) {
    /* Matching error. */
    safe_str("#-1 REGEXP ERROR: ", buff, bp);
    safe_str((const char *) regexp_errbuf, buff, bp);
    return;
  }
  matched = (int) regexec(re, strupper(args[0]));
  safe_str(unparse_integer(matched), buff, bp);

  /* If we don't have a third argument, we're done. */
  if (nargs < 3) {
    mush_free(re, "regexp");
    return;
  }
  /* We need to parse the list of registers.  Anything that we don't parse
   * is assumed to be -1.  If we didn't match, or the match went wonky,
   * then set the register to empty.  Otherwise, fill the register with
   * the subexpression.
   */
  nqregs = list2arr(qregs, NSUBEXP, args[2], ' ');
  for (i = 0; i < nqregs; i++) {
    if (qregs[i] && *qregs[i] && is_integer(qregs[i]))
      curq = parse_integer(qregs[i]);
    else
      curq = -1;
    if (curq < 0 || curq >= 10)
      continue;
    if (!matched || !re->startp[i] || !re->endp[i])
      renv[curq][0] = '\0';
    else {
      len = re->endp[i] - re->startp[i];
      if (len > BUFFER_LEN - 1)
	len = BUFFER_LEN - 1;
      if (len < 0)
	len = 0;
      strncpy(renv[curq], re->startp[i], len);
      renv[curq][len] = '\0';	/* must null-terminate */
    }
  }
  mush_free(re, "regexp");
}
