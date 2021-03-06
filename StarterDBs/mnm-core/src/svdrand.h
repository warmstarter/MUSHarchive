/*! \file svdrand.h
 * \brief Random Numbers.
 *
 * $Id: svdrand.h 5476 2012-02-29 22:45:33Z brazilofmux $
 *
 * Random Numbers based on algorithms presented in "Numerical Recipes in C",
 * Cambridge Press, 1992.
 */

#ifndef SVDRAND_H
#define SVDRAND_H

void SeedRandomNumberGenerator(void);
double RandomFloat(double flLow, double flHigh);
INT32 RandomINT32(INT32 lLow, INT32 lHigh);

#endif // SVDRAND_H
