#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#ifndef LINUX
#include <process.h>
#include <dos.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#else
#include "doslinux.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <unistd.h>
#define strnicmp strncasecmp
#endif
#include <fcntl.h>
#include <malloc.h>
#include "compress.h"
#include "logdef.h"
#include "records.h"

COMPRESSTYPE CompressType[10];
short DefCompressor;
extern char *compressor_name[];
extern char *compressor_verbs[];
extern THRESHOLD threshold;
extern long calc_eof (char *filename);

// extern
extern short strlenII (char *ptr);
extern void logwrite (short message, short indicator);
extern void logtext (char *string, short indicator, short dateon);
extern short is_file_there (char *filename);
extern void deletefile (char *filename);
extern char *extchr (char *, char);

// prototypes
short call_spawn (void);
void comsplit_parms (char *file, char *params);

char *aparms[18];
char argv[100];

short
compress (char *filename)
{
  char compress[255];
  char logline[255];
  char *ptr;
  short value = 0;

  strcpy (compress, filename);

  ptr = extchr (compress, '.');	/* 4/4/01 - handle more than one . in pathname */
  ptr++;
  if (*ptr != 'D')
    {
      *ptr = CompressType[DefCompressor].ext;
    }
  else
    {
      *ptr = CompressType[DefCompressor].ext;
      ptr++;
      *ptr = 'D';
    }
  memset (aparms, 0, sizeof (aparms));

  strcpy (argv, CompressType[DefCompressor].add);
  comsplit_parms (filename, compress);

  sprintf (logline, "Creating Archive %s", compress);
  logtext (logline, 1, YES);
  //    printf("%s\n",logline);

  sprintf (logline, "Containing %s", filename);
  logtext (logline, 1, YES);

  if (is_file_there (compress) == 0)
    deletefile (compress);

  //     printf("%s\n",logline);
  value = call_spawn ();

  //memset(filename,0,254);
  filename[0] = 0;
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
  short value = 0;

  char compress[255];
  char *ptr;
  short cnt;

  _splitpath (filename, drive, path, fname, exten);
  strcpy (compress, drive);
  strcat (compress, path);
//      strcat(compress,"*.*");

//      strcpy(compress,filename);
  ptr = extchr (filename, '.');
  ptr++;

  memset (aparms, 0, sizeof (aparms));

  for (cnt = 0; cnt <= 9; cnt++)
    {
      if (tolower (*ptr) == tolower (CompressType[cnt].ext))
	break;
    }
  if (cnt == 10)
    return 1;			// 5/1/01 - if not an archive, exit

  if (CompressType[cnt].compressor == 1)
    strcat (compress, "*.*");

  strcpy (argv, CompressType[cnt].extract);
  comsplit_parms (compress, filename);


  sprintf (logline, "Extracting Archive %s", filename);
  // printf("%s\n",logline);
  logtext (logline, 1, YES);
  value = call_spawn ();

  return value;
}

void
comsplit_parms (char *filename, char *compress)
{
  short pcnt;
  char *delim = { " " };

  aparms[0] = strtok (argv, delim);

  aparms[1] = aparms[0];

  for (pcnt = 2; pcnt <= 18; pcnt++)
    {
      aparms[pcnt] = strtok (NULL, delim);
      if (aparms[pcnt] == NULL)
	break;
      if (strnicmp (aparms[pcnt], "%a", 2) == 0)
	aparms[pcnt] = compress;	// %a
      if (strnicmp (aparms[pcnt], "%f", 2) == 0)
	aparms[pcnt] = filename;	// %f
      if (strnicmp (aparms[pcnt], "\"%f\"", 4) == 0)	// 4/10/2001 "%f" for linux use
	{
	  static char qfilename[80];	//a filename enclosed in quotes
	  qfilename[0] = 34;
	  qfilename[1] = 0;	//(to protect it from Linux shell)
	  strcat (qfilename, filename);
	  strcat (qfilename, "\"");
	  aparms[pcnt] = qfilename;
	}
    }

}

short
init_compressors (void)
{
  FILE *comctl;
  char str[256];
  short linecnt = 0, cntr = 0, compcnt = 0;
  char *ptr, *ptr1;
  long eof;

  eof = calc_eof ("compress.ctl");
  comctl = fopen ("compress.ctl", "rt");

  if (comctl == NULL)
    return (1);

  if (comctl != NULL)
    {
      //fseek (comctl, 0L, SEEK_END);
      //    eof = ftell (comctl);
      //fseek (comctl, 0L, SEEK_SET);
      while (1)
	{
	  //memset (str, 0, 254);
	  str[0] = 0;
	  fgets (str, 254, comctl);
	  linecnt++;
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
		    case 0:	// add
		      ptr = str;
		      ptr += strlen (compressor_verbs[cntr]);	// advance ptr to end of verb
		      while (*ptr == ' ')
			ptr++;
		      if (strlen (ptr) <= 99)
			strncpy (CompressType[compcnt].add, ptr,
				 (strlen (ptr) - 1));
		      else
			strncpy (CompressType[compcnt].add, ptr, 99);
		      break;
		    case 1:	// archiver
		      ptr = str;
		      ptr += strlen (compressor_verbs[cntr]);	// advance ptr to end of verb
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
		    case 2:	// extension
		      ptr = str;
		      ptr += strlen (compressor_verbs[cntr]);	// advance ptr to end of verb
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
		    case 3:	// extract
		      ptr = str;
		      ptr += strlen (compressor_verbs[cntr]);	// advance ptr to end of verb
		      while (*ptr == ' ')
			ptr++;
		      if (strlen (ptr) <= 99)
			strncpy (CompressType[compcnt].extract, ptr,
				 (strlen (ptr) - 1));
		      else
			strncpy (CompressType[compcnt].extract, ptr, 99);
		      break;
		    case 4:	// ident
		      ptr = str;
		      ptr += strlen (compressor_verbs[cntr]);	// advance ptr to end of verb
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
		    case 5:	// end
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
		    case 6:	// Needs *.* for extraction path
		      CompressType[compcnt].compressor = 1;
		      break;
		    default:
		      break;
		    }
		  //printf("here %s\n",ptr);
		  cntr = 0;

		}
	    }
	  if (eof == ftell (comctl))
	    break;

	}
    }
  fclose (comctl);
  return (0);
}

short
call_spawn (void)
{
  short i = 1;
  long val_exit = 0;
  char logline[255], cmdline[255];

  fflush (stdout);
  //printf("Parms ");

  cmdline[0] = 0;
  while (aparms[i] != NULL)
    {
      //printf("%s ",aparms[i]);
      if (i > 1)
	strcat (cmdline, " ");
      strcat (cmdline, aparms[i]);
      i++;
    }
  //printf("(%d)\n",i);
  //  printf("cmdline:%s\n",cmdline);

  //     pathp = getenv("PATH");
//      printf("Path = %s\n",pathp);

#ifdef LINUX
  val_exit = system (cmdline);
#else
//      _heapshrink();
  switch (i)
    {
    case 2:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3], NULL);
      break;
    case 3:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], NULL);
      break;
    case 4:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], NULL);
      break;
    case 5:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], NULL);
      break;
    case 6:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], NULL);
      break;
    case 7:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8], NULL);
      break;
    case 8:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], NULL);
      break;
    case 9:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], NULL);
      break;
    case 10:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], aparms[11], NULL);
      break;
    case 11:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], aparms[11], aparms[12], NULL);
      break;
    case 12:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], aparms[11], aparms[12], aparms[13],
		 NULL);
      break;
    case 13:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], aparms[11], aparms[12], aparms[13],
		 aparms[14], NULL);
      break;
    case 14:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], aparms[11], aparms[12], aparms[13],
		 aparms[14], aparms[15], NULL);
      break;
    case 15:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], aparms[11], aparms[12], aparms[13],
		 aparms[14], aparms[15], aparms[16], NULL);
      break;
    default:
      val_exit =
	spawnlp (P_WAIT, aparms[0], aparms[1], aparms[2], aparms[3],
		 aparms[4], aparms[5], aparms[6], aparms[7], aparms[8],
		 aparms[9], aparms[10], aparms[11], aparms[12], aparms[13],
		 aparms[14], aparms[15], aparms[16], aparms[17], NULL);
      break;
    }
#endif

  sprintf (logline, "spawned compressor %s", aparms[0]);
  logtext (logline, 4, YES);

  i = 2;
  sprintf (logline, "PRMS : ", aparms[i]);

  while (aparms[i] != NULL)
    {
      strcat (logline, aparms[i]);
      strcat (logline, " ");
      i++;
    }
  logtext (logline, 5, YES);

  sprintf (logline, "archiver return value %d", val_exit);
  logtext (logline, 5, YES);

//       printf("error no %d\n",errno);
//       printf(logline);
  if (val_exit < 0)
    switch (errno)
      {
      case EZERO:		// No Error
	break;
      case ENOENT:		// No Such File
	sprintf (logline, "Archiver not found - Check PATH.\n");
	logtext (logline, 0, YES);
	break;
      case E2BIG:		// Too many arguments
	sprintf (logline,
		 "Archiver command line too long. - Check compress.ctl!\n");
	logtext (logline, 0, YES);
	break;
      case ENOMEM:		// Not Enough Memory
	sprintf (logline, "Not enough memory to activate archiver.\n");
	logtext (logline, 0, YES);
	break;
      default:
	sprintf (logline, "Unknown Archiver error.\n");
	logtext (logline, 0, YES);
	break;
      }

  return val_exit;		//5-01-01 return 0 if success, error code if not
}

//#endif

short
fcn_threshold (char *filename)
{
  long handle, rc;
  struct stat buf;

  handle = open (filename, O_RDONLY);

  if (handle != -1)
    {
      rc = fstat (handle, &buf);
      if (rc == -1)
	{
	  close (handle);
	  return (2);
	}
    }
  else
    {
      close (handle);
      return (2);
    }

  close (handle);

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

//      return(0);
}
