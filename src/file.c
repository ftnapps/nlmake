#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#if defined(__linux__) || defined(__EMX__)
#include "doslinux.h"
#elif defined(__DOS__) || defined(__NT__) || defined(__OS2__)
#include <dos.h>
#include <direct.h>
#endif

#include "records.h"
#include "logdef.h"

#ifdef __linux__
#define PathChar '/'
#else
#define PathChar '\\'
#endif


extern char OutPath[];
extern char Master[];
extern char Update[];
extern short sfilecnt;
extern SEGFILE segfile[];
extern LINEPRMS Months[];
extern LINEPRMS SegmentType[];
extern char Publish_day[];
extern char *extchr (char *string, char dot);

// externs
extern void logtext (char *string, short indicator, short dateon);
extern void logwrite (short message, short indicator);
extern void closelog (void);
extern void get_proc_date (char *buf, int maxlen);
extern short FindMostCurr (char *FileName);
extern int getJDate (void);

// prototype
long calc_eof (char *filename);
short is_file_there (char *filename);
short file_age (char *filename);
short comp_compile_date (void);
void get_file_date (char *filename, char *date_stamp);

void
create_directory (char *dirname)
{
  int result;
  char msg[256];
  int len;

  // remove trailing PathChar
  len = strlen(dirname);
  if (dirname[len] == PathChar)
  {
    dirname[len] = 0;
  }

#if defined(__linux__) || defined(__EMX__)
  result = mkdir(dirname, 0750);
#elif defined(__DOS__) || defined(__NT__) || defined(__OS2__)
  result = mkdir(dirname);
#endif

  // add trailing PathChar
  dirname[len] = PathChar;
  dirname[++len] = 0;

  if (result == 0){
    snprintf (msg, sizeof (msg), "Created directory %s", dirname);
    logtext (msg, 1, YES);
    return;
  }

  if (errno == EEXIST) {
    // It is not an error if the directory already exists
    return;
  }

  snprintf (msg, sizeof (msg), "Critical error - can not create directory %s (%s)", dirname, strerror(errno));
  logtext (msg, 0, YES);
  logwrite (CFE_ABORT, 0);
  logwrite (SYS_STOP, 0);
  closelog ();              // close logfile
  exit (255);
}

void
deletefile (char *filename)
{
  char msg[256];

  if (remove (filename))
    {
      snprintf (msg, sizeof (msg), "Cannot delete %s", filename);
    }
  else
    {
      snprintf (msg, sizeof (msg), "Deleted %s", filename);
    }
  logtext (msg, 5, YES);
}

int
copyfile (char *filename, char *destination)
{
  FILE *in, *out;
  char buf[1024];
  char msg[256];
  size_t len;
  int result;

  /* open the input file */
  if ((in = fopen (filename, "rb")) == NULL)
    {
      return -1;
    }
  if ((out = fopen (destination, "wb")) == NULL)
    {
      fclose (in);
      return -1;
    }

  /* copy the file contents */
  while ((len = fread (buf, 1, 100, in)) != 0)
    {
      if (fwrite (buf, 1, len, out) != len)
        {
          break;
        }
    }

  /* now set up the return */
  result = (ferror (in) || ferror (out)) ? -1 : 0;

  /* close the files */
  fclose (in);
  fclose (out);

  if (result)
    {
      snprintf (msg, sizeof (msg), "Couldn't copy %s to %s", filename, destination);
    }
  else
    {
      snprintf (msg, sizeof (msg), "Copied %s to %s", filename, destination);
    }
  logtext (msg, 5, YES);

  return result;
}

void
movefile (char *filename, char *destdir)
{
  char msg[256];
  char newname[256];            // FIXME: how big buffer?
  char *basename;

  if (destdir[0] == 0)
    {
      // Empty destination - kill file
      remove (filename);
      return;
    }

  // Extract filename from path
  basename = strrchr (filename, PathChar);
  if (basename == NULL)
    {
      basename = filename;
    }

  snprintf (newname, sizeof (newname), "%s%s", destdir, basename);

  if (rename (filename, newname))
    {
      // Renaming didn't work - we need to copy the file instead
      if (!copyfile (filename, newname))
        {
          // Ok, delete the original
          deletefile (filename);
        }
    }
  else
    {
      snprintf (msg, sizeof (msg), "Moved %s to %s", filename, newname);
      logtext (msg, 5, YES);
    }

}


/* Remove old segment files (matching the filename) */
void
clean_dir (char *filename)
{
  long rc;
  char cleaname[255];
  struct find_t fileinfo;
  int jfriday, lastfriday;
  char *dot;
  char ascjfriday[5];

  dot = extchr (filename, '.');
  if (dot == NULL)
    return;

  sprintf (cleaname, "%s", filename);
  dot = extchr (cleaname, '.');
  if (dot != NULL)
    strcpy (dot, ".*");
  else
    strcpy (cleaname, ".*");    // Should probably be strcat?

  jfriday = getJDate ();
  lastfriday = jfriday - 7;
  if (lastfriday <= 0)
    {
      time_t utime;
      struct tm *tm;

      time (&utime);
      tm = localtime (&utime);

      // FIXME: bad leap year detection?
      if ((tm->tm_year % 4) == 0)
        lastfriday += 366;
      else
        lastfriday += 365;
    }

  sprintf (ascjfriday, "%03d", jfriday);

  rc = _dos_findfirst (cleaname, _A_NORMAL, &fileinfo);
  //printf("\nfind first %s\n",cleaname);

  if (rc == 0)
    {
      while (rc == 0)
        {
          dot = extchr (fileinfo.name, '.');
          if (strncmp (filename, Master, strlen (Master)) == 0)
            sprintf (cleaname, "%s%s", Master, fileinfo.name);
          else
            sprintf (cleaname, "%s%s", OutPath, fileinfo.name);
//     printf("\nfind  %s\n",cleaname);
          if (dot != NULL)
            {
              dot++;            // deal with outfiles
              if (atoi (dot) != lastfriday && atoi (dot) != jfriday)
                {
                  //printf("\nfind  %c\n",*dot);
                  if (isalpha (*dot))   // deal with archives
                    {
                      dot++;
                      //printf("\nfind  %c\n",*dot);
                      //printf("\nfind  %s\n",ascjfriday);
                      if (strncmp (dot, ascjfriday + 1, 2) != 0)
                        deletefile (cleaname);
                    }
                  else
                    deletefile (cleaname);
                }
            }
          // delete
          rc = _dos_findnext (&fileinfo);
        }
    }
}

/* Compare two files; return 0 if equal and 1 if they differ */
short
filecomp (char *filename1, char *filename2)
{
  FILE *file1, *file2;
  char str1[255];
  char str2[255];
  int first = 1;
  int result = 0;

  file1 = fopen (filename1, "rb");
  file2 = fopen (filename2, "rb");

  if (file1 == NULL || file2 == NULL)
    {
      if (file1)
        fclose (file1);
      if (file2)
        fclose (file2);
      return (1);
    }

  while (1)
    {
      char *r1, *r2;

      r1 = fgets (str1, sizeof (str1), file1);
      r2 = fgets (str2, sizeof (str2), file2);

      if (first)
        {
          // ignore first line of file ;)
          first = 0;
        }
      else
        {
          if ((r1 == NULL) && (r2 == NULL))
            {
              // End of both files, without any differences
              break;
            }
          if ((r1 == NULL) || (r2 == NULL))
            {
              // We've reached the end of one of the files (but not both)
              result = 1;
              break;
            }
          if (strcmp (str1, str2) != 0)
            {
              // The lines differ
              result = 1;
              break;
            }
        }
    }

  fclose (file1);
  fclose (file2);

  return (result);
}

short
is_file_there (char *filename)
{
  FILE *temp;

  temp = fopen (filename, "rt");
  if (temp == NULL)
    return (1);

  fclose (temp);
  return (0);

}

long
calc_eof (char *filename)
{
  struct stat st;

  // Get file info
  if (stat (filename, &st))
    {
      // FIXME: check errno
      logtext ("Unable to stat file", 2, YES);
      return 0;
    }

  return st.st_size;
}

void
add_eof (char *filename)
{
  FILE *temp;
  char ch = 0x1A;

  temp = fopen (filename, "ab");
  if (temp == NULL)
    return;

  fputc (ch, temp);

  fclose (temp);
}

void
touch_file_date (char *filename)
{
  FILE *f;

  if ((f = fopen (filename, "a")) == NULL)
    {
      logtext ("Unable to set file time stamp", 2, YES);
    }
  else
    {
      logtext ("Reset file time stamp", 5, YES);
      fclose (f);
    }

}

void
list_file_dates (char *filename)
{
  short bcnt;
  char namebuf[255];
  char date_stamp[100];
  FILE *segmentfile;
  FILE *stats2;
  char date[50];

  stats2 = fopen (filename, "wt");
  if (stats2 == NULL)
    {
      logtext ("Unable to open file lastin.txt", 0, YES);
      return;
    }

  get_proc_date (date, sizeof(date));
  fprintf (stats2, "Segment file Dates for the week of %s \n\n", date);
  fprintf (stats2, "Last Processed Files:\n\n");

  for (bcnt = 1; bcnt <= sfilecnt; bcnt++)
    {
      sprintf (namebuf, "%s%s", Master, segfile[bcnt].FileName);
      if (strchr (segfile[bcnt].FileName, '.') == NULL)
        FindMostCurr (namebuf);
      segmentfile = fopen (namebuf, "rt");
      if (segmentfile != NULL)
        {
          fclose (segmentfile);
          get_file_date (namebuf, date_stamp);
          if (segfile[bcnt].SegmentType >= 2)
            fprintf (stats2, "%-8.8s %-5d segment was last modified %s\n",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Net, date_stamp);
          else
            fprintf (stats2, "%-8.8s %-5d segment was last modified %s",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Net, date_stamp);
        }
      else
        {
          if (segfile[bcnt].SegmentType >= 2)
            fprintf (stats2,
                     "%-8.8s %-5d segment is not found in master directory\n",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Net);
          else
            fprintf (stats2,
                     "%-8.8s %-5d segment is not found in master directory\n",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Node);
        }

    }

  fprintf (stats2, "\nUpdates to be processed:\n\n");

  for (bcnt = 1; bcnt <= sfilecnt; bcnt++)
    {
      sprintf (namebuf, "%s%s", Update, segfile[bcnt].FileName);
      if (strchr (segfile[bcnt].FileName, '.') == NULL)
        FindMostCurr (namebuf);
      segmentfile = fopen (namebuf, "rt");
      if (segmentfile != NULL)
        {
          fclose (segmentfile);
          get_file_date (namebuf, date_stamp);
          if (segfile[bcnt].SegmentType >= 2)
            fprintf (stats2, "%-8.8s %-5d segment was last modified %s\n",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Net, date_stamp);
          else
            fprintf (stats2, "%-8.8s %-5d segment was last modified %s",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Net, date_stamp);
        }

    }

  fclose (stats2);
  logtext ("Created lastin.txt", 2, YES);
}

/* Get last modification time for a file */
void
get_file_date (char *filename, char *date_stamp)
{
  struct stat st;

  // Get file info
  if (stat (filename, &st))
    {
      // FIXME: check errno
      logtext ("Unable to get file time stamp", 2, YES);
    }
  else
    {
      struct tm *t;

      t = localtime (&st.st_mtime);
      strftime (date_stamp, 20, "%Y-%m-%d %H:%M:%S", t);
      logtext ("Retrieved file time stamp", 6, YES);
    }
}

/* Returns the age of a file, counted in days */
short
file_age (char *filename)
{
  struct stat st;

  // Get file info
  if (stat (filename, &st))
    {
      // FIXME: check errno
      logtext ("Unable to get file time stamp", 2, YES);
      return 0;
    }
  else
    {
      time_t now;

      // Get current time
      now = time (NULL);

      logtext ("Retrieved file time stamp", 6, YES);

      // return current time minus file time, in days instead of seconds
      return (now - st.st_mtime) / (3600 * 24);
    }
}

/* Return 1 if flags.ctl is newer than quick.lst, or if quick.lst doesn't exist */
short
comp_compile_date (void)
{
  struct stat st1, st2;

  // Get file info
  if (stat ("flags.ctl", &st1))
    {
      logtext ("Unable to read flags.ctl date", 2, YES);
      return 0;
    }

  if (stat ("quick.lst", &st2))
    {
      logtext ("Unable to read quick.lst date", 2, YES);
      return 1;
    }

  logtext ("Checking for update of FLAGS lists.", 5, YES);

  // Check if quick.lst has later time stamp
  if (st1.st_mtime <= st2.st_mtime)
    {
      logtext ("Compile of flags.ctl is not required.", 5, YES);
      return 0;
    }
  else
    {
      logtext ("Compile of flags.ctl is required.", 5, YES);
      return 1;
    }
}
