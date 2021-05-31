/* Win32 services routines */

/* Author: Nick Gammon */

#ifdef WIN32

#include "copyrite.h"
#include "config.h"

#include <windows.h>		/* for find first */
#undef OPAQUE			/* clashes with MUSH definition */

#ifdef I_STDLIB
#include <stdlib.h>
#endif
#include <process.h>
#include <direct.h>

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "match.h"
#include "externs.h"
#include "mymalloc.h"
#include "confmagic.h"

/* This is bad, but only for WIN32, which is bad anyway... */
#define EMBEDDED_MKINDX
#include "mkindx.c"

int makeindex(const char *inputfile, const char *outputfile);

static char buff[1024];

BOOL
ConcatenateFiles(const char *path, const char *outputfile)
{
  HANDLE filscan;
  WIN32_FIND_DATA fildata;
  BOOL filflag;
  DWORD status;
  FILE *fo = NULL;
  FILE *f = NULL;
  size_t bytes_in, bytes_out;
  long total_bytes = 0;
  int total_files = 0;
  char directory[MAX_PATH];
  char fullname[MAX_PATH];

  char *p;

  /* If outputfile is an empty string, forget it. */
  if (!outputfile || !*outputfile)
    return FALSE;

/* extract the directory from the path name */

  strcpy(directory, path);
  p = strrchr(directory, '\\');
  if (p)
    p[1] = 0;
  else {
    p = strrchr(directory, '/');
    if (p)
      p[1] = 0;
  }

/* Open output file */

  fo = fopen(outputfile, "wb");

  if (!fo) {
    fprintf(stderr, "Unable to open file: %s\n", outputfile);
    return FALSE;
  }
  fprintf(stderr, "Creating file: %s\n", outputfile);

/* Find first file matching the wildcard */

  filscan = FindFirstFile(path, &fildata);
  if (filscan == INVALID_HANDLE_VALUE) {
    status = GetLastError();

    fclose(fo);

    fprintf(stderr, "**** No files matching: \"%s\" found.\n", path);

    if (status == ERROR_NO_MORE_FILES)
      return TRUE;
    else
      return FALSE;
  }
/*
   Now enter the concatenation loop.
 */

  do {
    if (!(fildata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

      fprintf(stderr, "    Copying file: %s, %ld byte%s\n",
	      fildata.cFileName,
	      fildata.nFileSizeLow,
	      fildata.nFileSizeLow == 1 ? "" : "s"
	);

      strcpy(fullname, directory);
      strcat(fullname, fildata.cFileName);

/* Open the input file */

      f = fopen(fullname, "rb");

      if (!f)
	fprintf(stderr, "    ** Unable to open file: %s\n", fullname);
      else {

	total_files++;

	/* do the copy loop */

	while (!feof(f)) {
	  bytes_in = fread(buff, 1, sizeof(buff), f);
	  if (bytes_in <= 0)
	    break;

	  bytes_out = fwrite(buff, 1, bytes_in, fo);
	  total_bytes += bytes_out;
	  if (bytes_in != bytes_out) {
	    fprintf(stderr, "Unable to write to file: %s\n", outputfile);
	    fclose(f);
	    break;
	  }
	}			/* end of copy loop */


	fclose(f);
      }				/* end of being able to open file */

    }				/* end of not being a directory */
    /* get next file matching the wildcard */
    filflag = FindNextFile(filscan, &fildata);
  } while (filflag);

  status = GetLastError();

  FindClose(filscan);

  fclose(fo);

  fprintf(stderr, "Copied %i file%s, %ld byte%s\n",
	  total_files,
	  total_files == 1 ? "" : "s",
	  total_bytes,
	  total_bytes == 1 ? "" : "s");

  if (status == ERROR_NO_MORE_FILES)
    return TRUE;
  else
    return FALSE;

}

int
CheckDatabase(const char *path, FILETIME * modified, long *filesize)
{
  HANDLE filscan;
  WIN32_FIND_DATA fildata;
  SYSTEMTIME st;
  static char *months[] =
  {">!<",
   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  FILE *f;
  size_t bytes;

  filscan = FindFirstFile(path, &fildata);
  if (filscan == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "File \"%s\" not found.\n", path);
    return FALSE;
  }
  *modified = fildata.ftLastWriteTime;
  *filesize = fildata.nFileSizeLow;

  FindClose(filscan);

  FileTimeToSystemTime(&fildata.ftLastWriteTime, &st);

  if (st.wMonth < 1 || st.wMonth > 12)
    st.wMonth = 0;

  fprintf(stderr, "File \"%s\" found, size %ld byte%s, "
	  "\n         modified on %02d %s %04d %02d:%02d:%02d\n",
	  path,
	  fildata.nFileSizeLow,
	  fildata.nFileSizeLow == 1 ? "" : "s",
	  st.wDay, months[st.wMonth], st.wYear,
	  st.wHour, st.wMinute, st.wSecond);

  if (fildata.nFileSizeHigh == 0 && fildata.nFileSizeLow < 80) {
    fprintf(stderr, "File is too small to be a MUSH database.\n");
    return FALSE;
  }
/* check file for validity */

  f = fopen(path, "rb");

  if (!f) {
    fprintf(stderr, "Unable to open file %s\n", path);
    return FALSE;
  }
  if (fseek(f, -80, SEEK_END) != 0) {
    fprintf(stderr, "Unable to check file %s\n", path);
    fclose(f);
    return FALSE;
  }
  bytes = fread(buff, 1, 80, f);

  fclose(f);

  if (bytes != 80) {
    fprintf(stderr, "Unable to read last part of file %s\n", path);
    return FALSE;
  }
  if (strstr(buff, "***END OF DUMP***") == 0) {
    fprintf(stderr, "Database not terminated correctly, file %s\n", path);
    return FALSE;
  }
  return TRUE;

}				/* end of  CheckDatabase */

void
Win32MUSH_setup(void)
{

  int indb_OK, outdb_OK, panicdb_OK;

  FILETIME indb_time, outdb_time, panicdb_time;

  long indb_size, outdb_size, panicdb_size;

  ConcatenateFiles("txt\\hlp\\*.hlp", HELPTEXT);
  ConcatenateFiles("txt\\nws\\*.nws", NEWS_FILE);
  ConcatenateFiles("txt\\evt\\*.evt", EVENT_FILE);
  ConcatenateFiles("txt\\rul\\*.rul", RULES_FILE);
  ConcatenateFiles("txt\\idx\\*.idx", INDEX_FILE);
  makeindex(HELPTEXT, HELPINDX);
  makeindex(NEWS_FILE, NEWSINDX);
  makeindex(EVENT_FILE, EVENTINDX);
  makeindex(RULES_FILE, RULESINDX);
  makeindex(INDEX_FILE, INDEXINDX);

  indb_OK = CheckDatabase(options.input_db, &indb_time, &indb_size);
  outdb_OK = CheckDatabase(options.output_db, &outdb_time, &outdb_size);
  panicdb_OK = CheckDatabase(options.crash_db, &panicdb_time, &panicdb_size);

  if (indb_OK) {		/* Look at outdb */
    if (outdb_OK) {		/* Look at panicdb */
      if (panicdb_OK) {		/* outdb or panicdb or indb */
	if (CompareFileTime(&panicdb_time, &outdb_time) > 0) {	/* panicdb or indb */

	  if (CompareFileTime(&panicdb_time, &indb_time) > 0) {		/* panicdb */

	    ConcatenateFiles(options.crash_db, options.input_db);
	  } else {		/* indb */
	  }
	} else {		/* outdb or indb */
	  if (CompareFileTime(&outdb_time, &indb_time) > 0) {	/* outdb */

	    ConcatenateFiles(options.output_db, options.input_db);
	  } else {		/* indb */
	  }
	}
      } else {			/* outdb or indb */
	if (CompareFileTime(&outdb_time, &indb_time) > 0) {	/* outdb */

	  ConcatenateFiles(options.output_db, options.input_db);
	} else {		/* indb */
	}
      }
    } else {			/* outdb not OK */
      if (panicdb_OK) {		/* panicdb or indb */
	if (CompareFileTime(&panicdb_time, &indb_time) > 0) {	/* panicdb */

	  ConcatenateFiles(options.crash_db, options.input_db);
	} else {		/* indb */
	}
      } else {			/* indb */
      }
    }
  } else {			/* indb not OK */
    if (outdb_OK) {		/* look at panicdb */
      if (panicdb_OK) {		/* out or panic */
	if (CompareFileTime(&panicdb_time, &outdb_time) > 0) {	/* panicdb */

	  ConcatenateFiles(options.crash_db, options.input_db);
	} else {		/* outdb */

	  ConcatenateFiles(options.output_db, options.input_db);
	}
      } else {			/* outdb */
	ConcatenateFiles(options.output_db, options.input_db);
      }
    } else {			/* outdb not OK */
      if (panicdb_OK) {		/* panicdb */
	ConcatenateFiles(options.crash_db, options.input_db);
      } else {			/* NOTHING */
	exit(-1);
      }
    }
  }
/* Final failsafe - input database SHOULD still be OK. */
  fprintf(stderr, "Verifying selected database.\n");
  if (!CheckDatabase(options.input_db, &indb_time, &indb_size)) {
    fprintf(stderr, "File corrupted during selection process.\n");
    exit(-1);
  } else {
    fprintf(stderr, "Input database verified. Proceeding to analysis.\n");
  }
}
#endif				/* WIN32 */
