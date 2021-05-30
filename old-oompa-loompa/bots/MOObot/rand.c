/*****************************************************************
 * rand: A very, very random generator, period approx 6.8064e16.
 *
 * Uses algorithm M, "Art of Computer Programming", Vol 2. 1969, D.E.Knuth.
 *
 * Two generators are used to derive the high and low parts of sequence X,
 * and another for sequence Y. These were derived by Michael Mauldin.
 *
 * Usage:  initialize by calling srand(seed), then rand() returns a random 
 *         number from 0..2147483647. srand(0) uses the current time as
 *         the seed.
 *
 * Author: Michael Mauldin, June 14, 1983.
 *
 * EDITLOG
 *	LastEditDate = Fri Aug 29 22:01:13 1986 - Michael Mauldin
 *	LastFileName = /usre3/mlm/src/misc/rand.c
 *
 * HISTORY
 * 06-Nov-91  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Add a non-repeating nrrint function.
 *
 * 29-Aug-86  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 *****************************************************************/
 
# include <stdio.h>

/* Rand 1, period length 444674 */
# define MUL1 1156
# define OFF1 312342
# define MOD1 1334025
# define RAND1 (seed1=((seed1*MUL1+OFF1)%MOD1))
# define Y      RAND1

/* Rand 2, period length 690709 */
# define MUL2 1366
# define OFF2 827291
# define MOD2 1519572
# define RAND2 (seed2=((seed2*MUL2+OFF2)%MOD2))

/* Rand 3, period length 221605 */
# define MUL3 1156
# define OFF3 198273
# define MOD3 1329657
# define RAND3 (seed3=((seed3*MUL3+OFF3)%MOD3))

/*
 * RAND2 generates 19 random bits, RAND3 generates 17. The X sequence
 * is made up off both, and thus has 31 random bits.
 */

# define X    ((RAND2<<13 ^ RAND3>>3) & 017777777777)

# define AUXLEN 97
# define INIT1	872978
# define INIT2	518652
# define INIT3	226543

/* Defaults for seeds & auxtab (in case the user forgets to call srand() */
static int seed1=INIT1, seed2=INIT2, seed3=INIT3;
static int auxtab[AUXLEN] = {
 1059301927,  556254465,  206610184, 1149150814, 1073452261, 2079648074,
  648659266,  569081795,  141877783, 2144266766,  871957966,  956597516,
 2115853434,   67999908,  907505314, 1721895810, 1855199589, 1289876501,
  259437382, 2001538405,  221994133,  225581774, 1399339965,  174108667,
 1158240020,  161378470, 1800726297, 2022066144,  855341866, 2008415576,
  317514688, 2006197847, 1090972817, 1480636060, 1418672845, 1821869088,
 1325060189,  618322526, 1207758378, 1644444334, 1296371882,  529059318,
  602524496, 1594248688, 1491811114,  740721307, 2010962763,  375426230,
 1939855564,  671726401, 2066805129,  215499663,  548011539, 1835055325,
 1093228058,  918063272, 1073295930,  737870789, 1255953262,  490721328,
  820402065,  200024163, 1161968569, 1572448147,  518036323, 1977784945,
 1865133559,  543933550, 1306031609,  552730741,  162160880,   27085579,
 1962062704, 1999835684, 1422323117,  101227069,  225950047, 1374733307,
 1857195651,  689152148, 1137304993, 1386534000, 1321823018, 1699716535,
  330998055, 1078712123,  802914134, 1014987021, 2138340736,  926931475,
 1963958264, 1823281719, 1519441747, 1677407648, 1218083658, 1274288969,
 2121116764
};

/****************************************************************
 * srand: Initialize the seed (0 => use the current time+pid)
 ****************************************************************/

srand (seed)
int seed;
{ register int i;

  if (seed == 0) seed = (getuid() + getpid() + time(0));  

  /* Set the three random number seeds */
  seed1 = (INIT1+seed) % MOD1;
  seed2 = (INIT2+seed) % MOD2;
  seed3 = (INIT3+seed) % MOD3;
  
  for (i=AUXLEN; i--; )
    auxtab[i] = X;
}

/****************************************************************
 * rand: Random integer from 0..2147483647
 ****************************************************************/

int rand ()
{ register int j, result;

  j = AUXLEN * Y / MOD1;	/* j random from 0..AUXLEN-1 */
  result = auxtab[j];
  auxtab[j] = X;
  return (result);
}

/****************************************************************
 * randint: Return a random integer in a given Range
 ****************************************************************/

int randint (range)
int range;
{ double rrand;
  int result;

  rrand = rand();
  result = range * (rrand / 2147483648.);
  return (result);
}

double rand01 ()
{ return ((double) rand () / 2147483648.);
}


int **rint_used = NULL;
int max_ru = 0;

int nrrint (seq, range)
int seq, range;
{ register int j, num, size, cnt=0;

  if (seq >= max_ru) return (randint (range));

  /* If we used this sequence before */
  if (rint_used[seq])
  { size = rint_used[seq][0];

    /* Shift all used numbers */
    for (j=size; --j > 1; )
    { rint_used[seq][j] = rint_used[seq][j-1]; }
      
    /* Search for an unused number */
    while (1)
    { num = rint_used[seq][1] = randint (range);

      for (j=2; j<size; j++)
      { if (num == rint_used[seq][j]) break; }
            
      /* Return the new number if not a duplicate */
      if (j >= size || ++cnt > 50) return (num);
    }
  }
  
  /* Allocate memory to remember numbers used in this seq */
  else
  { size = (range > 8) ? 8 : range;

    rint_used[seq] = (int *) malloc (size * sizeof (int *));
    rint_used[seq][0] = size;
    for (j=1; j<size; j++) rint_used[seq][j] = -1;
    num = rint_used[seq][1] = randint (range);
    return (num);
  }
}

max_nrrint (seq)
{ register int i;

  max_ru = seq;

  rint_used = (int **) malloc (max_ru * sizeof (int *));
  
  for (i=0; i<max_ru; i++) rint_used[i] = NULL;
}
