#if defined(LINUX) || defined(OS2)

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

#ifndef OS2
/* split a path into dir/file.ext - DOS-style drive letter is not used */
void
_splitpath (const char *path, char *drive, char *dir, char *file, char *ext)
{
  unsigned char pos = strlen (path),    /* posn in string, scan right->left */
    state = 'e';                /* presently copy: e-ext, f-file, d-dir */
  drive[0] = dir[1] = file[0] = ext[0] = 0;
  dir[0] = '/';
  while (pos)
    {
      pos--;
      switch (state)
        {
        case 'e':               /* copy .extension */
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
        case 'f':               /* copy /filename */
          if (path[pos] == '/')
            state = 'd';
          else
            {
              memmove (file + 1, file, strlen (file) + 1);
              file[0] = path[pos];
            }
          break;
        case 'd':               /* copy base directory */
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
#endif


/* convert findfirst/findnext to Linux readdir() calls */
unsigned
_dos_findnext (struct find_t *f)        //translate dos-style findfirst/next()
{                               //calls to linux direntry.h functions
  char fullpath[FILENAME_MAX];
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
  f->wr_date = (unsigned) (fs.st_mtime / 86400);        /* # of days since 1/1/70 */
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
