#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
#include "doslinux.h" // _splitpath
#endif

#include "compress.h"
#include "logdef.h"
#include "records.h"


#ifdef __linux__
#define strnicmp strncasecmp
#endif


COMPRESSTYPE CompressType[10];
short DefCompressor;
extern char *compressor_name[];
extern char *compressor_verbs[];
extern THRESHOLD threshold;

// extern
extern short strlenII (char *ptr);
extern void fatal_error(char *reason);
extern void logwrite (short message, short indicator);
extern void logtext (char *string, short indicator, short dateon);
extern short is_file_there (char *filename);
extern void deletefile (char *filename);
extern char *extchr (char *, char);

// prototypes
int call_spawn (char *command, char *filename, char *compress);


short
compress (char *filename)
{
  char compress[255];
  char logline[255];
  char *ptr;

  // Calculate archive name
  strcpy (compress, filename);
  ptr = extchr (compress, '.');
  ptr++;
  if (*ptr == 'D')
    {
      *ptr = CompressType[DefCompressor].ext;
      *++ptr = 'D';
    }
  else
    {
      *ptr = CompressType[DefCompressor].ext;
    }

  sprintf (logline, "Creating Archive %s", compress);
  logtext (logline, 1, YES);

  sprintf (logline, "Containing %s", filename);
  logtext (logline, 1, YES);

  if (is_file_there (compress) == 0)
    deletefile (compress);

  call_spawn (CompressType[DefCompressor].add, filename, compress);
  // What to do with return code?

  strcpy (filename, compress);

  return (is_file_there (compress));
}


short
decompress (char *filename)
{
  char logline[255];
  char drive[5];
  char path[255];
  char fname[9];
  char exten[4];
  short value;

  char compress[255];
  char *ptr;
  short cnt;

  _splitpath (filename, drive, path, fname, exten);

  ptr = extchr (filename, '.');
  ptr++;

  for (cnt = 0; cnt <= 9; cnt++)
    {
      if (tolower (*ptr) == tolower (CompressType[cnt].ext))
        break;
    }
  if (cnt == 10)
    return 1;                   // 5/1/01 - if not an archive, exit

  strcpy (compress, drive);
  strcat (compress, path);
  if (CompressType[cnt].compressor == 1)
    strcat (compress, "*.*");

  sprintf (logline, "Extracting Archive %s", filename);
  logtext (logline, 1, YES);
  value = call_spawn (CompressType[cnt].extract, compress, filename);

  return value;
}

short
init_compressors (void)
{
  FILE *comctl;
  char str[256];
  short cntr = 0, compcnt = 0;
  char *ptr, *ptr1;

  comctl = fopen ("compress.ctl", "rt");

  if (comctl == NULL)
    return (1);

  while (fgets (str, 254, comctl) != NULL)
    {
      // kill leading spaces
      while (str[0] == ' ')
        memmove (str, str + 1, 253);

      // remove comments
      ptr = strchr (str, ';');
      if (ptr != NULL)
        *ptr = 0;

      if (str[0] != 0)
        {
          while (compressor_verbs[cntr] != NULL)
            {
              if (strnicmp
                  (str, compressor_verbs[cntr],
                   strlen (compressor_verbs[cntr])) == 0)
                {
                  //printf("found verb %s\n", compressor_verbs[cntr]);
                  break;
                }
              else
                cntr++;
            }
          if (compressor_verbs[cntr] != NULL)
            {
              switch (cntr)
                {
                case 0: // add
                  ptr = str;
                  ptr += strlen (compressor_verbs[cntr]);       // advance ptr to end of verb
                  while (*ptr == ' ')
                    ptr++;
                  if (strlen (ptr) <= 99)
                    strncpy (CompressType[compcnt].add, ptr,
                             (strlen (ptr) - 1));
                  else
                    strncpy (CompressType[compcnt].add, ptr, 99);
                  break;
                case 1: // archiver
                  ptr = str;
                  ptr += strlen (compressor_verbs[cntr]);       // advance ptr to end of verb
                  while (*ptr == ' ')
                    ptr++;
                  cntr = 0;
                  while (compressor_name[cntr] != NULL)
                    {
                      if (strnicmp
                          (ptr, compressor_name[cntr],
                           strlen (compressor_name[cntr])) == 0)
                        {
                          compcnt = cntr;
                          CompressType[compcnt].compressor = 0;
                          break;
                        }
                      else
                        cntr++;
                    }
                  //printf("unknown compressor\n");
                  break;
                case 2: // extension
                  ptr = str;
                  ptr += strlen (compressor_verbs[cntr]);       // advance ptr to end of verb
                  while (*ptr == ' ')
                    ptr++;
                  if (*(ptr + 1) == '?' && *(ptr + 2) == '?')
                    {
                      CompressType[compcnt].ext = ptr[0];
                    }
                  else
                    {
                      // major error
                      //printf("unknown compressor extension\n");
                    }
                  break;
                case 3: // extract
                  ptr = str;
                  ptr += strlen (compressor_verbs[cntr]);       // advance ptr to end of verb
                  while (*ptr == ' ')
                    ptr++;
                  if (strlen (ptr) <= 99)
                    strncpy (CompressType[compcnt].extract, ptr,
                             (strlen (ptr) - 1));
                  else
                    strncpy (CompressType[compcnt].extract, ptr, 99);
                  break;
                case 4: // ident
                  ptr = str;
                  ptr += strlen (compressor_verbs[cntr]);       // advance ptr to end of verb
                  while (*ptr == ' ')
                    ptr++;
                  ptr1 = strchr (str, ',');
                  if (ptr1 != NULL)
                    {
                      ptr1++;
                      if (strlen (ptr1) <= 99)
                        strncpy (CompressType[compcnt].ident, ptr1,
                                 (strlenII (ptr1)));
                      else
                        strncpy (CompressType[compcnt].ident, ptr1, 99);
                      ptr1--;
                      *ptr1 = 0;
                      CompressType[compcnt].offset = atol (ptr);
                    }
                  else
                    {
                      // major error
                      //printf("unknown compressor ident\n");
                    }
                  break;
                case 5: // end
                  if (CompressType[compcnt].add[0] == 0)
                    {
                      // major error
                      printf ("Compressor Config Error\n");
                    }
                  else if (CompressType[compcnt].extract[0] == 0)
                    {
                      // major error
                      printf ("Compressor Config Error\n");
                    }
                  else if (CompressType[compcnt].ext == 0)
                    {
                      // major error
                      printf ("Compressor Config Error\n");
                    }
                  else if (CompressType[compcnt].ident[0] == 0)
                    {
                      // major error
                      printf ("Compressor Config Error\n");
                    }
                  //    printf("got %s \n%s \n%c \n%s \n%ul", CompressType[compcnt].add,
                  //                             CompressType[compcnt].extract,
                  //                                                                        CompressType[compcnt].ext,
                  //                                                                        CompressType[compcnt].ident,
                  //                                                                        CompressType[compcnt].offset);
                  //compcnt++;
                  break;
                case 6: // Needs *.* for extraction path
                  CompressType[compcnt].compressor = 1;
                  break;
                default:
                  break;
                }
              //printf("here %s\n",ptr);
              cntr = 0;

            }
        }
    }

  fclose (comctl);
  return (0);
}

int
call_spawn (char *command, char *filename, char *compress)
{
  long val_exit;
  char logline[255], cmdline[255];
  char *p1, *p2;

  // Expand macros & create command line
  p1 = command;
  while ((p2 = strchr (p1, '%')) != NULL)
    {
      *p2++ = 0;
      strcat (cmdline, p1);

      switch (*p2)
        {
        case 'a':
          strcat (cmdline, compress);
          p2++;
          break;
        case 'f':
          strcat (cmdline, filename);
          p2++;
          break;
        default:
          // not a macro - let the '%' character flow through
          strcat (cmdline, "%");
          break;
        }
      p1 = p2;
    }
  strcat (cmdline, p1);

  sprintf (logline, "spawning compressor %s", cmdline);
  logtext (logline, 4, YES);

  fflush (stdout);
  val_exit = system (cmdline);

  if (val_exit < 0)
    {
      fatal_error("Couldn't start archiver");
    }

  sprintf (logline, "archiver return value %ld", val_exit);
  logtext (logline, 5, YES);

  return val_exit;
}

short
fcn_threshold (char *filename)
{
  long rc;
  struct stat buf;

  rc = stat (filename, &buf);
  if (rc)
    {
      return (2);
    }

  if (threshold.diff_size == 0)
    threshold.diff_size = (long) (threshold.arc_size * (5 / 3));

  if (threshold.diff_size <= -1 && threshold.arc_size <= -1)
    return (0);
  if (threshold.diff_size <= -1 && threshold.arc_size == 0)
    return (1);
  if (threshold.diff_size == 0 && threshold.arc_size <= -1)
    return (2);
  if (threshold.diff_size == 0 && threshold.arc_size == 0)
    return (3);


  if (threshold.diff_size <= -1)
    {
      if (buf.st_size >= threshold.arc_size)
        return (1);
      else
        return (0);
    }
  else
    {
      if ((buf.st_size / 50) >= threshold.diff_size)
        return (3);
      else
        if (buf.st_size >= threshold.arc_size
            && buf.st_size <= threshold.diff_size)
        return (1);
      else
        return (2);
    }
}
