
/*
 * atr.c --  Various funcs used to help read/write a attrib
 */

#include "header.h"


#define FETCH_LBUFS 4
#define TRUE -1
#define FALSE 0

char *vfetch_attribute( object,attribute )
dbref object;
char *attribute;
{
static char null_char = '\0';
unsigned int i;
ATTR *attr;
static char *lbufs[FETCH_LBUFS];
static int first_time = TRUE;
static int on_buf;
dbref owner;
int flags;

  /* if this is the first call, prepare the lbufs */
  if( first_time ) {
    for( i = 0; i < FETCH_LBUFS; lbufs[i++] = alloc_lbuf(
	 "smisc.c:fetch_attribute()" ));
    on_buf = -1;
    first_time = FALSE;
  }

  attr = atr_str( attribute );

  if( attr == NULL ) {
    /* can't find the requested attribute.  note it in the log */
    return &null_char;
  }

  /* the attribute exists.  read it */
  /* attrib_val = atr_get_raw( object, attr->number ); */
  if( ++on_buf >= FETCH_LBUFS )
    on_buf = 0;

  return ( atr_pget_str( lbufs[on_buf], object, attr->number, &owner, &flags ));
}

/*
 * put_attribute
 *
 *  writes text to attribute on object, only if the attribute is defined.
 *
 * JMS 26 Aug 93
 */
void vput_attribute( dbref object, char *attribute, char *text )
{
ATTR *attr;

  attr = atr_str( attribute );

  if( attr == NULL ) {
    return;
  }

  atr_add_raw( object, attr->number, text );
  return;
}




char *vget_a( dbref object, dbref att)
{
static char null_char = '\0';
unsigned int i;
ATTR *attr;
static char *lbufs[FETCH_LBUFS];
static int first_time = TRUE;
static int on_buf;
dbref owner;
int flags;

  /* if this is the first call, prepare the lbufs */
  if( first_time ) {
    for( i = 0; i < FETCH_LBUFS; lbufs[i++] = alloc_lbuf(
	 "smisc.c:fetch_attribute()" ));
    on_buf = -1;
    first_time = FALSE;
  }

  /*attr = atr_str( attribute );*/
/*
  if( attr == NULL ) {
    return &null_char;
  }
*/

  /* the attribute exists.  read it */
  /* attrib_val = atr_get_raw( object, attr->number ); */
  if( ++on_buf >= FETCH_LBUFS )
    on_buf = 0;

  return ( atr_pget_str( lbufs[on_buf], object, att, &owner, &flags ));
}



dbref att_dbref( dbref object, char *attribute )
{
ATTR *attrib;
dbref result;

  attrib = atr_str( attribute );
  if( attrib == NULL )
    return NOTHING;

  init_match( GOD, vfetch_attribute( object, attribute ), NOTYPE );
  match_absolute();
  result = match_result();

  if( result == AMBIGUOUS ) result = NOTHING;

  return result;
}


static char *rcsatrc="$Id: atr.c,v 1.1 2001/01/15 03:23:15 wenk Exp $";
