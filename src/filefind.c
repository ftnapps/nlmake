#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef LINUX
#include "doslinux.h"
#define PathChar '/'
#else
#include <dos.h>
#define PathChar '\\'
#endif
// #include "records.h"
// #include "logdef.h"
#include "compress.h"

extern COMPRESSTYPE CompressType[];

//find last dot in "/path.ext/dir/filename.ext" string - 4/10/2001
char *
extchr (char *string, char dot)
{
  char *p = strchr (string, '.');       // find first '.'
  if (!p)
    return NULL;
  while (strchr (p + 1, '.') != NULL)   // if there are more, find next one
    p = strchr (p + 1, '.');
  if (strchr (p, PathChar))     //if further path info at end, not an .ext
    return NULL;
  return p;                     //else return pointer to last dot in name
}

short
FindMostCurr (char *FileName)
{
  long rc;
  char *dot;
  short Jdate = 0;
  unsigned filedate = 0;
  struct find_t fileinfo;
  char lookfor[255];

  dot = extchr (FileName, '.'); //4/10/2001 - need last dot (.ext) in name
  if (dot != NULL)
    *dot = 0;

  strcpy (lookfor, FileName);
  strcat (lookfor, ".*");

  rc = _dos_findfirst (lookfor, _A_NORMAL, &fileinfo);
  //filedate = fileinfo.wr_date;

  while (rc == 0)
    {
//      printf("%s %u %u %u\n",fileinfo.name,fileinfo.wr_time, fileinfo.wr_date, fileinfo.size);
      dot = extchr (fileinfo.name, '.');
      if (dot != NULL)
        {
          if ((Jdate <= atoi (dot + 1)) && (atoi (dot + 1) <= 366)
              && (filedate <= fileinfo.wr_date))
            {
              Jdate = atoi (dot + 1);
              filedate = fileinfo.wr_date;
            }
        }
      rc = _dos_findnext (&fileinfo);
    }
  memset (lookfor, 0, sizeof (lookfor));
  sprintf (lookfor, "%03d", Jdate);
  strcat (FileName, ".");
  strcat (FileName, lookfor);
  if (Jdate >= 0)
    return (0);
  else
    return (1);

}

short
Findiff (char *FileName)
{
  long rc;
  char *dot;
  struct find_t fileinfo;
  char lookfor[255];

  dot = extchr (FileName, '.');
  if (dot != NULL)
    *dot = 0;

  strcpy (lookfor, FileName);
  strcat (lookfor, ".D*");

  rc = _dos_findfirst (lookfor, _A_NORMAL, &fileinfo);
  //filedate = fileinfo.wr_date;
  if (rc == 0)
    {
      dot = extchr (fileinfo.name, '.');
      if (dot != NULL)
        {
          strcat (FileName, dot);
          return (0);
        }
    }
  return (1);

}


short
FindArch (char *FileName)
{
  long rc;
  char *dot;
  char ext[4];
  struct find_t fileinfo;
  short i;
  char lookfor[255];

  for (i = 0; i <= 9; i++)
    {
      dot = extchr (FileName, '.');
      if (dot != NULL)
        *dot = 0;
      strcpy (lookfor, FileName);
      ext[0] = '.';
      ext[1] = CompressType[i].ext;
      ext[2] = '*';
      ext[3] = 0;
      strcat (lookfor, ext);
//      printf("lookfor %s\n",lookfor);

      rc = _dos_findfirst (lookfor, _A_NORMAL, &fileinfo);
      if (rc == 0)
        {
          dot = extchr (fileinfo.name, '.');
          if (dot != NULL)
            {
              strcat (FileName, dot);
              return (0);
            }
        }
    }
  return (1);
}
