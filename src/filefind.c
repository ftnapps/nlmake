#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__) || defined(__EMX__)
#include "doslinux.h"
#elif defined(__DOS__) || defined(__NT__) || defined(__OS2__)
#include <dos.h>
#endif

#include "compress.h"


#ifdef __linux__
#define PathChar '/'
#else
#define PathChar '\\'
#endif

extern COMPRESSTYPE CompressType[];

//find last dot in "/path.ext/dir/filename.ext" string - 4/10/2001
char *
extchr (char *string, char dot)
{
  char *p = strrchr (string, '.');       // find last '.'
  if (p == NULL)
    return NULL;
  if (strchr (p, PathChar))     //if further path info at end, not an .ext
    return NULL;
  return p;                     //else return pointer to last dot in name
}

// Find the most current version of a segment file
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

  sprintf (lookfor, "%s.*", FileName);

  rc = _dos_findfirst (lookfor, _A_NORMAL, &fileinfo);
  while (rc == 0)
    {
//      printf("%s %u %u %u\n",fileinfo.name,fileinfo.wr_time, fileinfo.wr_date, fileinfo.size);
      dot = extchr (fileinfo.name, '.');
      if (dot != NULL)
        {
          int fileDay = atoi (dot + 1);
          if ((Jdate <= fileDay) && (fileDay <= 366) && (filedate <= fileinfo.wr_date))
            {
              Jdate = fileDay;
              filedate = fileinfo.wr_date;
            }
        }
      rc = _dos_findnext (&fileinfo);
    }

  sprintf (lookfor, "%03d", Jdate);
  strcat (FileName, ".");
  strcat (FileName, lookfor);

  if (Jdate >= 0)
    return (0);
  else
    return (1);
}

// Find a diff file
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

  sprintf (lookfor, "%s.D*", FileName);

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

  return (1);
}

// Find a compressed segment
short
FindArch (char *FileName)
{
  long rc;
  char *dot;
  struct find_t fileinfo;
  short i;
  char lookfor[255];

  for (i = 0; i <= 9; i++)
    {
      dot = extchr (FileName, '.');
      if (dot != NULL)
        *dot = 0;

      sprintf (lookfor, "%s.%c*", FileName, CompressType[i].ext);
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
