/*! \file _build.cpp
 * \brief Build-identifying declarations.
 *
 * $Id: _build.cpp 654 2007-01-05 04:18:02Z brazilofmux $
 *
 * Ignore the contents of this file, use macros in _build.h instead.
 * These declarations are usually overridden by VER_FLG in the Makefile.
 */

#include "_build.h"
char szBuildDate[] = __DATE__ " " __TIME__;
char szBuildNum[] = "1";
