#ifdef LINUX
/* DOS-style date and filesystem functions for use in Linux NLmake */
/* (c) 2001 Carl Austin Bennett, Kingston_ON - no rights reserved  */
/* Note that this is not part of the original DSO Enterprises code */
/* and was created solely for use in the Linux port of the program */
/* Most of these remap DOS-style calls to their Linux equivalents  */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include "doslinux.h"

/* open a file, return file handle as *handle and errors as return code */
unsigned
_dos_open (const char *fname, unsigned mode, int *handle)
{
  int file;
  file = open (fname, mode);
  if (file < 0)
    return file;
  *handle = file;
  return 0;
}

/* remap _dos_close to close() */
unsigned
_dos_close (int file)
{
  close (file);
}

/* split a path into dir/file.ext - DOS-style drive letter is not used */
void
_splitpath (const char *path, char *drive, char *dir, char *file, char *ext)
{
  unsigned char pos = strlen (path),	/* posn in string, scan right->left */
    state = 'e';		/* presently copy: e-ext, f-file, d-dir */
  drive[0] = dir[1] = file[0] = ext[0] = 0;
  dir[0] = '/';
  while (pos)
    {
      pos--;
      switch (state)
	{
	case 'e':		/* copy .extension */
	  if (path[pos] == '/')
	    {
	      strcpy (file, ext);
	      ext[0] = 0;
	      state = 'd';
	    }
	  else
	    {
	      memmove (ext + 1, ext, strlen (ext) + 1);
	      ext[0] = path[pos];
	    }
	  if (path[pos] == '.')
	    state = 'f';
	  break;
	case 'f':		/* copy /filename */
	  if (path[pos] == '/')
	    state = 'd';
	  else
	    {
	      memmove (file + 1, file, strlen (file) + 1);
	      file[0] = path[pos];
	    }
	  break;
	case 'd':		/* copy base directory */
	  memmove (dir + 1, dir, strlen (dir) + 1);
	  dir[0] = path[pos];
	  break;
	}
    }
}

/* recombine a path from constituent parts (inverse of splitpath operation) */
void
_makepath (const char *path, char *drive, char *dir, char *file, char *ext)
{
  if (dir[strlen (dir) - 1] != '/')
    strcat (dir, "/");
  if (ext[0] == '.')
    sprintf ((char *) path, "%s%s%s", dir, file, ext);
  else if (ext[0])
    sprintf ((char *) path, "%s%s.%s", dir, file, ext);
  else
    sprintf ((char *) path, "%s%s", dir, file);
}

/* convert findfirst/findnext to Linux readdir() calls */
unsigned
_dos_findnext (struct find_t *f)	//translate dos-style findfirst/next()
{				//calls to linux direntry.h functions
  char fullpath[NAME_MAX];
  struct stat fs;
  do
    {
      f->size = 0;
      f->name = NULL;
      f->dd = readdir (f->dir);
      if (!f->dd)
	{
	  closedir (f->dir);
	  return (unsigned) -1;
	}
    }
  while (!strstr (f->dd->d_name, f->fname));
  f->name = f->dd->d_name;
  strcpy (fullpath, f->path);
  strcat (fullpath, "/");
  strcat (fullpath, f->name);
  stat (fullpath, &fs);
  f->size = fs.st_size;
  f->wr_date = (unsigned) (fs.st_mtime / 86400);	/* # of days since 1/1/70 */
  return 0;
}

unsigned
_dos_findfirst (char *name, unsigned start, struct find_t *f)
{
  unsigned char p = strlen (name);
  strcpy (f->path, name);
  while ((!!p) && (name[p] != '/'))
    p--;
  if (!p)
    f->path[p++] = '.';
  f->path[p] = 0;
  while ((name[p] == '/') || (name[p] == '*'))
    p++;
  strcpy (f->fname, name + p);
  p = strlen (f->fname);
  while ((!!p) && f->fname[p - 1] == '*')
    p--;
  f->fname[p] = 0;
  f->dir = opendir (f->path);
  if (!f->dir)
    return (unsigned) -1;
  f->size = 0;
  f->name = NULL;
  return _dos_findnext (f);
}

/* split unix file timestamp into time (2s increments), and dosdate  */
/* note that time is non-standard and used only to find newest files */
unsigned
_dos_getftime (int file, unsigned short *d, unsigned short *t)
{
  time_t utime;
  struct tm *tm;
  unsigned short day, month, year;

  time (&utime);
  tm = localtime (&utime);

  day = (unsigned char) tm->tm_mday;	/* 1-31 */
  month = (unsigned char) tm->tm_mon + 1;	/* 1-12 */
  year = (unsigned short) tm->tm_year - 80;	/* 1980-2099 */

  *d = (unsigned short) (day | (month << 5) | (year << 9));
  *t = (unsigned short) ((utime % 86400) / 2);
}

/* convert UNIX-style time() to DOS date and time format */
void
_dos_getdate (struct dosdate_t *t)
{
  time_t utime;
  struct tm *tm;
  time (&utime);
  tm = localtime (&utime);
  t->day = (unsigned char) tm->tm_mday;	/* 1-31 */
  t->month = (unsigned char) tm->tm_mon + 1;	/* 1-12 */
  t->year = (unsigned short) tm->tm_year + 1900;	/* 1980-2099 */
  t->dayofweek = (unsigned char) tm->tm_wday;	/* 0-6, 0=Sunday */
}

void
_dos_gettime (struct dostime_t *t)
{
  time_t utime;
  struct tm *tm;
  time (&utime);
  tm = localtime (&utime);
  t->hour = (unsigned char) tm->tm_hour;	/* 0-23 */
  t->minute = (unsigned char) tm->tm_min;	/* 0-59 */
  t->second = (unsigned char) tm->tm_sec;	/* 0-59 */
  t->hsecond = 0;		/* 0-99 */
}

/* a simple routine to convert integers (0-99999) to string values */
char *
itoa (int value, char *string, int radix)
{
  const char digit[] = "0123456789abcdef";
  char *p = string;
  *p = 0;
  if (radix != 10)
    return string;		/* radix other than 10 not implemented */
  if (value >= 10000)		/* split integer into 5 ASCII digits */
    *(p++) = '0' + ((value / 10000) % 10);
  if (value >= 1000)
    *(p++) = '0' + ((value / 1000) % 10);
  if (value >= 100)
    *(p++) = '0' + ((value / 100) % 10);
  if (value >= 10)
    *(p++) = '0' + ((value / 10) % 10);
  *(p++) = '0' + ((value) % 10);
  *p = 0;
  return string;
}

/* convert a string to UPPERCASE */
char *
strupr (char *string)
{
  char *p = string;
  while (*p)
    {
      (*p) = toupper (*p);
      p++;
    }
  return string;
}

#endif
