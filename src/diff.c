#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef LINUX
#include "doslinux.h"
#define strnicmp strncasecmp
#define PathChar '/'
#else
#include <dos.h>
#define PathChar '\\'
#endif
#include "logdef.h"
#include "records.h"

// globals
extern char OutFile[];
extern char OutDiff[];
extern char Master[];
extern char Update[];
extern char Uploads[];
extern char MailFiles[];
extern char BadFiles[];
extern LINEPRMS SegmentType[];
extern short MAKEZONE;
extern char MAKETYPE;
extern short MAKENET;
extern short MAKENODE;
extern short MAKENUMBER;
FILE *olist, *nlist, *diff;
long oldeof;
long neweof;
long difeof;
long add = 0, del = 0, copy = 0;
char oldstr[MAXSTR];
char newstr[MAXSTR];
char diffstr[MAXSTR];

// prototype
long find_match (void);

// extern
extern void logwrite (short message, short indicator);
extern void logtext (char *string, short indicator, short dateon);
extern short test_crc (char *filename, char *CRCLine);
extern short FindMostCurr (char *FileName);
extern void copyfile (char *filename, char *destination);
extern void movefile (char *filename, char *destination);
extern void deletefile (char *filename);
extern void netmail_text (char *message);
extern void send_netmail (char *subject, short SFI, char Type);
extern MSG mnotify[];
extern short Findiff (char *FileName);
extern unsigned short getcrc (char *filename, long offset);
extern void add_crc (unsigned short crctt, char *filename);
extern long calc_eof (char *filename);
extern char *extchr (char *string, char dot);


short
create_diff (char *ODFile)
{
  struct dosdate_t date;
  fpos_t position;

  char logline[255];
  char CRCLine[100];
  char drive[5];
  char path[255];
  char fname[9];
  char exten[5];
//      char newext[5];
  char fullname[255];
  short ext, i, rc;
  long matchline;

  _splitpath (OutFile, drive, path, fname, exten);

  // here
  ext = atoi (exten + 1);
  //ext -= 7; - moved 4/7/2001 as day 8 (prev wk==day 1) would become 367
  if (ext <= 7)
    {
      _dos_getdate (&date);
      date.year--;
      if ((date.year % 4) == 0)
        ext += 366;
      else
        ext += 365;
    }
  ext -= 7;

  memset (exten, 0, sizeof (exten));
  sprintf (exten, ".%03d", ext);
  //    strncpy(exten+1,newext,3);

  _makepath (fullname, drive, path, fname, exten);

  //printf("filename :%s\n",fullname);
  if ((rc = test_crc (fullname, CRCLine)) != 0)
    {
      if (rc == 1)
        {
          logtext ("Can not produce diff file ", 1, YES);
          sprintf (logline, "CRC error file %s", fullname);
          logtext (logline, 1, YES);
          logtext (CRCLine, 1, YES);
        }
      if (rc == 2)
        {
          logtext ("Can not produce diff file ", 1, YES);
          sprintf (logline, "file %s not found", fullname);
          logtext (logline, 1, YES);
        }
      return (1);
    }

  if ((rc = test_crc (OutFile, CRCLine)) != 0)
    {
      if (rc == 1)
        {
          logtext ("Can not produce diff file ", 1, YES);
          sprintf (logline, "CRC error file %s", OutFile);
          logtext (logline, 1, YES);
          logtext (CRCLine, 1, YES);
        }
      if (rc == 2)
        {
          logtext ("Can not produce diff file ", 1, YES);
          sprintf (logline, "file %s not found", OutFile);
          logtext (logline, 1, YES);
        }
      return (1);
    }

  oldeof = calc_eof (fullname) - 1;
  olist = fopen (fullname, "rb");

  if (olist == NULL)
    {
      logtext ("Can not produce diff file - Days > 7", 1, YES);
      // can not produce diff
      return (1);
    }

  //fseek (olist, 0L, SEEK_END);
  //oldeof = ftell (olist);
  //fseek (olist, 0L, SEEK_SET);
  neweof = calc_eof (OutFile) - 1;
  nlist = fopen (OutFile, "rb");
  //fseek (nlist, 0L, SEEK_END);
  //neweof = ftell (nlist);
  //fseek (nlist, 0L, SEEK_SET);

  diff = fopen (ODFile, "wb");
  fgets (oldstr, MAXSTR, olist);
  fprintf (diff, "%s", oldstr);
  del++;                        // set delete for copied line


  // total old comments to be deleted
  while (1)
    {
      if (oldeof == ftell (olist))
        break;                  // hard error
      memset (oldstr, 0, sizeof (oldstr));
      fgetpos (olist, &position);
      fgets (oldstr, MAXSTR, olist);
      if (strnicmp (oldstr, ";A", 2) != 0)
        break;
      if (oldstr[0] == 0)
        break;
      del++;
    }
  //fseek (olist,(long)((-1) * (strlen(oldstr)+1)),SEEK_CUR);
  fsetpos (olist, &position);

  // total new comments to be added
  while (1)
    {
      if (neweof == ftell (nlist))
        break;                  // hard error
      memset (newstr, 0, sizeof (newstr));
      fgets (newstr, MAXSTR, nlist);
      if (strnicmp (newstr, ";A", 2) != 0)
        break;
      if (newstr[0] == 0)
        break;
      add++;
    }

  // reset to file begining
  fseek (nlist, 0L, SEEK_SET);

  // indicate directives
  fprintf (diff, "D%ld\r\n", del);
  fprintf (diff, "A%ld\r\n", add);
  del = 0;

  // copy over added lines
  while (add >= 1)
    {
      if (neweof == ftell (nlist))
        break;                  // hard error
      memset (newstr, 0, sizeof (newstr));
      fgets (newstr, MAXSTR, nlist);
      if (newstr[0] == 0)
        break;
      fprintf (diff, "%s", newstr);
      add--;
    }

  // set copy directive
  while (1)
    {
      fgetpos (olist, &position);
      matchline = find_match ();

      if (matchline == 0)
        {
          if (del > 0)
            {
              if (neweof != ftell (nlist))
                fprintf (diff, "D%ld\r\n", del);
              del = 0;
            }
          if (newstr[0] == 0 && oldstr[0] == 0)
            break;
          else
            copy++;
        }
      else if (matchline < 0)
        {
          if (copy > 0)
            {
              fprintf (diff, "C%ld\r\n", copy);
              copy = 0;
            }
          if (oldeof == ftell (olist) && del > 0)
            fprintf (diff, "D%ld\r\n", del);
          del++;
        }
      else if (matchline > 0)
        {
          if (copy > 0)
            {
              fprintf (diff, "C%ld\r\n", copy);
              copy = 0;
            }
          if (del > 0)
            {
              if (neweof != ftell (nlist))
                fprintf (diff, "D%ld\r\n", del);
              del = 0;
            }
          add = matchline;
          if (oldeof == ftell (olist) && add >= 2)
            fprintf (diff, "A%ld\r\n", add - 1);
          else
            fprintf (diff, "A%ld\r\n", add);
          for (i = 1; i <= add; i++)
            {
              fgets (newstr, MAXSTR, nlist);
              fprintf (diff, "%s", newstr);
            }
          // fseek (olist,(long)((-1) * (strlen(oldstr)+1)),SEEK_CUR);
          fsetpos (olist, &position);
          add = 0;
        }

      if (newstr[0] == 0 && oldstr[0] == 0)
        break;
    }

  if (copy > 1)
    {
      fprintf (diff, "C%ld\r\n", copy);
      copy = 0;
    }

  fclose (olist);
  fclose (nlist);
  fclose (diff);
  return (0);
}

long
find_match (void)
{
  long linecnt = 0;
  fpos_t position2;
  memset (newstr, 0, sizeof (newstr));
  memset (oldstr, 0, sizeof (oldstr));

  if (oldeof != ftell (olist))
    {
      //printf("gets old\n");
      fgets (oldstr, MAXSTR, olist);
      //printf("oldstr - %s\n",oldstr);
    }

  fgetpos (nlist, &position2);

  while (1)
    {
      if (neweof == ftell (nlist))
        {
          if (oldstr[0] != 0)
            linecnt = -1;
          break;
        }
      if (linecnt == 2500)
        {
          linecnt = -1;
          break;
        }
      fgets (newstr, MAXSTR, nlist);
      //resetcntr += (long) ((-1) * (strlen(newstr) + 1));
      //printf("Seek 3%d\n",resetcntr);
      //printf("oldstr - %s\n",oldstr);
      //printf("newstr - %s\n",newstr);
      //printf("cntr - %d\n",linecnt);
      if (strcmp (oldstr, newstr) == 0)
        break;
      else
        linecnt++;
    }

  if (linecnt != 0)
    fsetpos (nlist, &position2);

  return (linecnt);

}


short
apply_diff (char *filename, short SFI)
{

  char diffname[255];
  char localname[255];
  char ofname[255];
  char logline[255];
  char CRCLine[100];
  char newext[5];
  short ext, i;

  if (strchr (filename, '.') != NULL)
    {
      logtext ("Can not apply Diff with explicit file name", 5, YES);
      return (1);
    }

  sprintf (diffname, "%s%s", MailFiles, filename);
  if (Findiff (diffname) == 1)
    return (1);
  difeof = calc_eof (diffname);
  diff = fopen (diffname, "rt");

  if (diff == NULL)
    {
      sprintf (diffname, "%s%s", Uploads, filename);
      if (Findiff (diffname) == 1)
        return (1);
      difeof = calc_eof (diffname);
      diff = fopen (diffname, "rt");
    }

  if (diff == NULL)
    {
      sprintf (logline, "No Diff to process for %s", diffname);
      logtext (logline, 5, YES);
      return (1);
    }

  memset (diffstr, 0, sizeof (diffstr));
  fgets (diffstr, MAXSTR, diff);

  ext = atoi ((diffstr + (strlen (diffstr) - 12)));

  sprintf(newext, "%d", ext);

  // the file the diff applies to
  sprintf (localname, "%s%s.%s", Master, filename, newext);

  difeof = calc_eof (localname);
  olist = fopen (localname, "rt");

  if (olist == NULL)
    {
      sprintf (logline, "Can not find segment <%s> to apply diff to",
               localname);
      logtext (logline, 5, YES);
      fclose (diff);
      fclose (olist);
      fclose (nlist);
      movefile (diffname, BadFiles);    // kill processed diff
      return (1);
    }

  for (i = 1; i <= 3; i++)
    {
      memset (diffstr, 0, sizeof (diffstr));
      fgets (diffstr, MAXSTR, diff);
      //printf("<%d> %s\n",i,diffstr);
    }
  ext = atoi ((diffstr + (strlen (diffstr) - 12)));

  sprintf(newext, "%d", ext);

  if (ext == 0)
    {
      sprintf (logline, "Can not figure new segment name from diff file");
      logtext (logline, 5, YES);
      fclose (diff);
      fclose (olist);
      fclose (nlist);
      movefile (diffname, BadFiles);    // kill processed diff
      return (1);
    }

  // the file the diff applies to
  sprintf (ofname, "%s%s.%s", Update, filename, newext);

  nlist = fopen (ofname, "wt");
  // if diff not found or old list to apply it to exit
  // if can't open new file exit

  if (diff == NULL || olist == NULL || nlist == NULL)
    {
      fclose (diff);
      fclose (olist);
      fclose (nlist);
      deletefile (ofname);      // Kill bad new file
      logtext ("Apply Diff Failed! ", 1, YES);
      return (1);
    }

  // do some checking on diff file
  //fseek (diff, 0L, SEEK_END);
  //difeof = ftell (diff);
  //fseek (diff, 0L, SEEK_SET);

  //fseek (olist, 0L, SEEK_END);
  //oldeof = ftell (olist);
  //fseek (olist, 0L, SEEK_SET);

  while (1)
    {
      if (difeof == ftell (diff))
        break;
      memset (diffstr, 0, sizeof (diffstr));
      fgets (diffstr, MAXSTR, diff);
      if (diffstr[0] == 0)
        break;
      switch (diffstr[0])
        {
        case 'A':
          add = atoi (diffstr + 1);
          for (i = 1; i <= add; i++)
            {
              memset (diffstr, 0, sizeof (diffstr));
              if (difeof != ftell (diff))
                {
                  fgets (diffstr, MAXSTR, diff);
                  fprintf (nlist, "%s", diffstr);
                }
              else
                {
                  fclose (diff);
                  fclose (olist);
                  fclose (nlist);
                  deletefile (ofname);  // Kill bad new file
                  movefile (diffname, BadFiles);        // kill processed diff
                  logtext ("Apply Diff Failed! ", 1, YES);
                  logtext ("Moved Diff to BadFiles ", 1, YES);
                  return (1);
                }
            }
          break;
        case 'D':
          if (diffstr[1] != 'o' || diffstr[1] != 'O')
            del = atoi (diffstr + 1);
          for (i = 1; i <= del; i++)
            if (oldeof != ftell (olist))
              fgets (oldstr, MAXSTR, olist);
            else
              {
                fclose (diff);
                fclose (olist);
                fclose (nlist);
                deletefile (ofname);    // Kill bad new file
                movefile (diffname, BadFiles);  // kill processed diff
                logtext ("Apply Diff Failed! ", 1, YES);
                logtext ("Moved Diff to BadFiles ", 1, YES);
                return (1);
              }
          break;
        case 'C':
          copy = atoi (diffstr + 1);
          for (i = 1; i <= copy; i++)
            {
              memset (oldstr, 0, sizeof (oldstr));
              if (oldeof != ftell (olist))
                {
                  fgets (oldstr, MAXSTR, olist);
                  fprintf (nlist, "%s", oldstr);
                }
              else
                {
                  fclose (diff);
                  fclose (olist);
                  fclose (nlist);
                  deletefile (ofname);  // Kill bad new file
                  movefile (diffname, BadFiles);        // kill processed diff
                  logtext ("Apply Diff Failed! ", 1, YES);
                  logtext ("Moved Diff to BadFiles ", 1, YES);
                  return (1);
                }
            }
          break;
        default:
          break;
        }

    }

  fclose (diff);
  fclose (olist);
  fclose (nlist);
  if (test_crc (ofname, CRCLine) == 1)
    {
      deletefile (ofname);      // kill bad make
      movefile (diffname, BadFiles);    // kill processed diff
      logtext ("Apply Diff Failed! CRC invalid", 1, YES);
      logtext ("Moved Diff to BadFiles ", 1, YES);
      if (mnotify[1].active == 'Y')
        {
          netmail_text ("\rFile not processed! \r\r---\r");
          send_netmail ("Apply Diff Failed! CRC invalid!", SFI, 1);
        }
      return (1);
    }
  else
    {
      deletefile (diffname);    // kill processed diff
      logtext ("Applied Diff sucessful!", 1, YES);
      netmail_text ("\rApplied Diff sucessful! \r\r---\r");
      send_netmail ("Info", SFI, 1);
    }
  return (0);
}

short
merge_list (char *filename)
{

  char str[512];
  char cpstr[50];
  FILE *Nodelst, *temp, *outseg;
  char *dot;
  char lmaketype;


  if (MAKETYPE == 6)
    return (1);
  //FindMostCurr(filename);

  if (MAKETYPE == 3)
    lmaketype = 2;
  else
    lmaketype = MAKETYPE;
// printf("Nodelist       %s\n",filename);

  Nodelst = fopen (filename, "rt");

  dot = extchr (filename, '.');

  if (dot == NULL)
    {
      fclose (Nodelst);
      return (1);
    }
  dot++;
  strncpy (dot, "999", 3);
  deletefile (filename);
  temp = fopen (filename, "wt");
  //printf("Segment       %s\n",OutFile);
// printf("Temp Nodelist %s\n",filename);

  outseg = fopen (OutFile, "rt");

  if (outseg == NULL)
    {
      fclose (Nodelst);
      fclose (temp);
      deletefile (filename);
      return (1);
    }
  // printf("Temp Nodelist %s\n",filename);


  // build comp string to find Zone Region or Host

  if (lmaketype >= 2)
    sprintf (cpstr, "%s,%d", SegmentType[lmaketype].String, MAKENUMBER);
  else
    sprintf (cpstr, "Host,%d", MAKENET);


  // copy nodelist from start to our Zone Region or Host
  while (1)
    {
      memset (str, 0, sizeof (str));
      fgets (str, MAXSTR, Nodelst);
      if (strnicmp (str, cpstr, strlen (cpstr)) != 0)
        fprintf (temp, "%s", str);
      else
        break;
    }

  // If we're a hub or node scan for us in segment
  if (lmaketype <= 1)
    {
      if (lmaketype == 1)
        sprintf (cpstr, "%s,%d", SegmentType[lmaketype].String, MAKENUMBER);
      else
        sprintf (cpstr, ",%d", MAKENODE);
      while (1)
        {
          memset (str, 0, sizeof (str));
          fgets (str, MAXSTR, Nodelst);
          if (str[0] == 0)
            break;
          if (strnicmp (str, "ZONE", 4) == 0)
            break;
          if (strnicmp (str, "REGION", 6) == 0)
            break;
          if (strnicmp (str, "HOST", 4) == 0)
            break;
          if (strnicmp (str, cpstr, strlen (cpstr)) != 0)
            fprintf (temp, "%s", str);
          else
            break;
        }

    }

  // Now copy the new segment from our submitle file in to the new list

  while (1)
    {
      memset (str, 0, sizeof (str));
      if (fgets (str, MAXSTR, outseg) == NULL)
        break;
      if (str[0] == ';' && strlen (str) >= 3);
      else
        fprintf (temp, "%s", str);
    }


  // only if we are not a HUB or a NODE
  if (lmaketype >= 2)
    {
      sprintf (cpstr, "%s,", SegmentType[lmaketype].String);
      fprintf (temp, ";\n");

      // skip nodelist to next Zone Region or Host
      while (1)
        {
          memset (str, 0, sizeof (str));
          if (fgets (str, MAXSTR, Nodelst) == NULL)
            break;
          if (strnicmp (str, cpstr, strlen (cpstr)) == 0)
            break;
        }
      // because we read one in to test it must be output
      fprintf (temp, "%s", str);

    }

  // If we're a hub or node skip ours
  if (lmaketype <= 1)
    {
      if (lmaketype == 1)
        sprintf (cpstr, "%s,", SegmentType[lmaketype].String);
      else
        sprintf (cpstr, ",%d", MAKENODE);
      while (1)
        {
          memset (str, 0, sizeof (str));
          fgets (str, MAXSTR, Nodelst);
          if (strnicmp (str, "ZONE", 4) == 0)
            break;
          if (strnicmp (str, "REGION", 6) == 0)
            break;
          if (strnicmp (str, "HOST", 4) == 0)
            break;
          if (strnicmp (str, cpstr, strlen (cpstr)) == 0)
            break;
        }

      // because we read one in to test it must be output
      fprintf (temp, "%s", str);
    }

  // copy in to the end of the nodelist
  while (1)
    {
      memset (str, 0, sizeof (str));
      if (fgets (str, MAXSTR, Nodelst) == NULL)
        break;
      fprintf (temp, "%s", str);
    }

  fclose (Nodelst);
  fclose (outseg);
  fclose (temp);

  temp = fopen (filename, "rb");
  fgets (str, MAXSTR, temp);
  fclose (temp);

  add_crc (getcrc (filename, strlen (str)), filename);

  return (0);
}
