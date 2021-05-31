/* mushdb.h */

#include "copyrite.h"

#ifndef __DB_H
#define __DB_H
#include <stdio.h>

#include "attrib.h"
#include "flags.h"
#include "lock.h"
#include "dbdefs.h"

/* Power macros */

#define Builder(x)       ( \
			  ((Powers(Owner(x)) & CAN_BUILD) || \
			   (Powers(x) & CAN_BUILD) || \
			   Hasprivs(x) || Hasprivs(Owner(x))))
#define Guest(x)	 (Powers(x) & IS_GUEST)
#define Tel_Anywhere(x)  (Hasprivs(x) || \
					 (Powers(x) & TEL_ANYWHERE))
#define Tel_Anything(x)  (Hasprivs(x) || \
					 (Powers(x) & TEL_OTHER))
#define See_All(x)       (Hasprivs(x) || (Powers(x) & SEE_ALL))
#define Priv_Who(x)      (Hasprivs(x) || (Powers(x) & SEE_ALL))
#define Can_Hide(x)      (Hasprivs(x) || (Powers(x) & CAN_HIDE))
#define Can_Login(x)     (Hasprivs(x) || \
					 (Powers(x) & LOGIN_ANYTIME))
#define Can_Idle(x)      (Hasprivs(x) || \
					 (Powers(x) & UNLIMITED_IDLE))
#define Long_Fingers(x)  (Hasprivs(x) || \
					 (Powers(x) & LONG_FINGERS))
#define Open_Anywhere(x) (Hasprivs(x) || \
					 (Powers(x) & OPEN_ANYWHERE))
#define Can_Boot(x)      (Hasprivs(x) || (Powers(x) & CAN_BOOT))
#define Do_Quotas(x)     (Wizard(x) || \
					 (Powers(x) & CHANGE_QUOTAS))
#define Change_Poll(x)   (Wizard(x) || (Powers(x) & SET_POLL))
#define HugeQueue(x)     (Wizard(x) || (Powers(x) & HUGE_QUEUE))
#define LookQueue(x)     (Hasprivs(x) || (Powers(x) & PS_ALL))
#define HaltAny(x)       (Wizard(x) || \
					 (Powers(x) & HALT_ANYTHING))
#define NoPay(x)         (Hasprivs(x) || Hasprivs(Owner(x)) || \
				(Powers(x) & NO_PAY) || \
				(Powers(Owner(x)) & NO_PAY))
#define NoQuota(x)       (Hasprivs(x) || Hasprivs(Owner(x)) || \
				(Powers(x) & NO_QUOTA) || \
				(Powers(Owner(x)) & NO_QUOTA))
#define NoKill(x)        (Hasprivs(x) || Hasprivs(Owner(x)) || \
				(Powers(x) & UNKILLABLE) || \
				(Powers(Owner(x)) & UNKILLABLE))
#define Search_All(x)    (Hasprivs(x) || \
					 (Powers(x) & SEARCH_ALL))
#define Global_Funcs(x)  (Hasprivs(x) || \
					 (Powers(x) & GLOBAL_FUNCS))
#define Create_Player(x) (Wizard(x) || \
					 (Powers(x) & CREATE_PLAYER))
#define Can_Announce(x)  (Wizard(x) || (Powers(x) & CAN_WALL))
#define Can_Cemit(x)	 (Hasprivs(x) || (Powers(x) & CEMIT))
#define Pemit_All(x)	 (Wizard(x) || (Powers(x) & PEMIT_ALL))

/* Permission macros */
#define Can_See_Flag(p,t,f)   (!(f->perms & (F_DARK | F_MDARK | F_ODARK)) || \
			     ((Owner(p) == Owner(t)) && \
				!(f->perms & (F_DARK | F_MDARK))) || \
   			     (See_All(p) && !(f->perms & F_DARK)) || \
			     God(p))

/* Can p locate x? */
#define Can_Locate(p,x) \
    (controls(p,x) || nearby(p,x) || See_All(p) \
  || (PLAYER_LOCATE && (IsPlayer(x) && !Unfind(x) \
                     && !Unfind(Location(x)))))


#define Can_Examine(p,x)    (controls(p,x) || See_All(p) || Visual(x))
#define can_link(p,x)  (controls(p,x) || \
			(IsExit(x) && (Location(x) == NOTHING)))

/* Can p link an exit to x? */
#define can_link_to(p,x) \
     (GoodObject(x) \
   && (controls(p,x) || (LinkOk(x) && eval_lock(p,x,Link_Lock))) \
   && (!NO_LINK_TO_OBJECT || IsRoom(x)))

/* can p access attribute a on object x? */
#define Can_Read_Attr(p,x,a)   \
   (!((a)->flags & AF_INTERNAL) && \
    (See_All(p) || \
     (!((a)->flags & AF_MDARK) && \
      (controls(p,x) || ((a)->flags & AF_VISUAL) || Visual(x) || \
       (Owner((a)->creator) == Owner(p))))))

/* can p write attribute a on object x, assuming p may modify x? 
 * Must be (1) God, or (2) a non-internal flag and (2a) a Wizard 
 * or (2b) a non-wizard attrib and (2b1) you own the attrib or 
 * (2b2) it's not atrlocked.
 */
#define Can_Write_Attr(p,x,a)  \
   (God(p) || \
    (!((a)->flags & AF_INTERNAL) && \
     (Wizard(p) || \
      (!((a)->flags & AF_WIZARD) && \
       (((a)->creator == Owner(p)) || !((a)->flags & AF_LOCKED)) \
   ))))


/* can p read/evaluete lock l on object x? */
#define Can_Read_Lock(p,x,l)   \
    (See_All(p) || controls(p,x) || Visual(x))

/* DB flag macros - these should be defined whether or not the
 * corresponding system option is defined 
 * They are successive binary numbers
 */
#define DBF_NO_CHAT_SYSTEM      0x01
#define DBF_WARNINGS		0x02
#define DBF_CREATION_TIMES	0x04
#define DBF_NO_POWERS		0x08
#define DBF_NEW_LOCKS		0x10
#define DBF_NEW_STRINGS		0x20
#define DBF_TYPE_GARBAGE        0x40
#define DBF_SPLIT_IMMORTAL	0x80
#define DBF_NO_TEMPLE		0x100
#define DBF_LESS_GARBAGE	0x200
#define DBF_AF_VISUAL		0x400
#define DBF_VALUE_IS_COST	0x800

#endif				/* __DB_H */
