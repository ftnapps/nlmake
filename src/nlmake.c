#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <malloc.h> // needed for mem test
#ifdef LINUX
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "doslinux.h"
#define strnicmp strncasecmp
#define PathChar '/'
#else
#include <sys\types.h>
#include <direct.h>
#include <dos.h>
#define PathChar '\\'
#endif
#include "records.h"
#include "logdef.h"
#include "textstr.inc"          // Processing commands in string form
#ifdef DOS
#define MAXSUBFILES 35          //Files in CTL
#else
#define MAXSUBFILES 100         // Files in CTL
#endif
char ProgName[] = { "NLMake" };
int MajVer = 1;
int MinVer = 13;
int Rev = 1;
int SubRev = 2;
#ifdef OS2
char OStype[] = { "OS/2" };
#endif
#ifdef DOS
char OStype[] = { "DOS" };
#endif
#ifdef WIN
char OStype[] = { "Win32" };
#endif
#ifdef LINUX
char OSType[] = { "Linux" };
#endif

// Externs
extern short DefCompressor;

//void process_segment(void);
extern void logwrite (short message, short indicator);
extern void logtext (char *string, short indicator, short dateon);
extern short FindMostCurr (char *FileName);
extern short process_segment (void);
extern short getJDate (short type);
extern short test_segment (void);
extern short process_new (void);
extern short copynew (void);
extern void copyfile (char *filename, char *destination);
extern void deletefile (char *filename);
extern short init_compressors (void);
extern short merge_list (char *filename);
extern void cre_flags_db (char *filename);
// file crap
extern short openlog (void);
extern void closelog (void);
extern long calc_eof (char *filename);
extern void list_file_dates (char *filename);
extern void fix_compile_date (void);
extern short comp_compile_date (void);


// prototype
short strlenII (char *ptr);
short breakaddress (void);
short is_proc_day (short file_day);
void initglobals (void);
void testctrlinfo (void);
void printctrlinfo (void);
void breakbaud (void);
int proctrlfile (void);
void parnotify (char *str);
void makedirerror (void);

// Global Switches
char TEST;
char TESTALL;
char PROCESS;
char DISP;
char FORCE;
char CLEANUP;
char CLEANUPOUT;
char MAKETYPE;
char STATS;
char FLAGCHK;
char FLAGCHKAUTO;
char WARNINGS;
char PVTlvl;
char LASTDATES;
char NOTIFY;
char ISLATE;
char IGNORCASE;
short MAKENUMBER;
short MAKEZONE = 1;
short MAKENET = 0;
short MAKENODE = 0;
short MAXAGE = 0;
extern char *errnostr[];


//Global Paths
char ControlFile[50];
char MergePath[250];
char Master[250];
char OutPath[250];
char Messages[250];
char Uploads[250];
char MailFiles[250];
char Update[250];
char BadFiles[250];

// Global File Names
char OutFile[250];
char OutDiff[250];
char SourceFile[250];
char Process_day[10];
char Publish_day[10];


// Global File Names with paths
char CopyRight[250];
char Prolog[250];
char Epilog[250];
char Comments[250];

//Global idents
char NetWorkName[20];
char NetAddress[20];
char SubAddress[20];
char SubNameNotify[73];
MSG mnotify[4];
short MinPhone, MaxPhone;
char BaudRates[255];
#define MAXBAUD 25
long bauds[MAXBAUD];
THRESHOLD threshold;
char ARC;
char expSourceFile[50];
char DispPVT;
SEGFILE segfile[MAXSUBFILES];
short sfilecnt = 1;
short linecnt = 0;
short loglvl = 2;
short errorlvl = 0;
short Dlineoff = 0;
short Flineoff = 0;

int
main (short ParmsCtr, char *Parms[])
{

  char *Pathptr;
  FILE *temp;
  char textline[512];
  short cntr = 0, cntr2 = 0;

  printf ("%s v%i.%i.%i%i (%s) \n", ProgName, MajVer, MinVer, Rev, SubRev,
          OSType);
  printf ("Copyright (c) 2002, RuneSoft Creations\n");
  printf ("Original Copyright: (c) 1999, DSO Enterprises\n");
  printf ("Original Linux Port: (c) 2001, Carl Austin Bennett 1:249/116\n");
  printf
    ("This version retains orginal copyrights with modifications\nreleased under the GPL\n");

  memset (textline, 0, sizeof (textline));

  errorlvl = openlog ();        // open logfile
  if (errorlvl > 0)
    {
      printf ("ERRORLEVEL %d - Critial stop - I/O problem\n", errorlvl);
      exit (errorlvl);
    }


  logwrite (SYS_START, 0);

  deletefile ("data.tmp");
  deletefile ("outfile.tmp");
  deletefile ("msgtext.tmp");

  initglobals ();

  logwrite (SYS_COMMAND, 0);
  strcat (textline, Parms[0]);  // Log Comand Line
  strcat (textline, " ");       // Space

  for (cntr2 = 1; cntr2 <= (ParmsCtr - 1); cntr2++)
    {
      strcat (textline, Parms[cntr2]);  // Log Comand Line
      strcat (textline, " ");   // Space
      //printf("got %s\n",Parms[cntr2]);

      while (LinePrms[cntr].String != NULL)
        {
          if (strnicmp
              (Parms[cntr2], LinePrms[cntr].Single,
               strlen (LinePrms[cntr].Single)) == 0)
            break;
          if (LinePrms[cntr].String == NULL)
            {
              cntr = 0;         // HardCode
              break;
            }
          cntr++;
        }
      switch (LinePrms[cntr].Type)
        {
        case 0:
          cntr = 0;
          break;
        case 1:
          PROCESS = 'Y';
          cntr = 0;
          break;
        case 2:
          TEST = 'Y';
          cntr = 0;
          break;
        case 3:
          Pathptr = strchr (Parms[cntr2], '=');
          if (Pathptr != NULL)
            {
              Pathptr++;
              strcpy (MergePath, strupr (Pathptr));
            }
          cntr = 0;
          break;
        case 4:
          Pathptr = strchr (Parms[cntr2], '=');
          if (Pathptr != NULL)
            {
              Pathptr++;
              strcpy (NetWorkName, Pathptr);
            }
          cntr = 0;
          break;
        case 5:
          DISP = 'Y';
          cntr = 0;
          break;
        case 6:
          FORCE = 'Y';
          cntr = 0;
          break;
        case 7:
          //printf("Nodelist production/testing utility\n");
          printf ("Syntax:\n\n");
          printf ("-Standard Functions:\n");
#ifdef LINUX
          printf
            ("nlmake [nlmake.ctl] -P -F -T -M=[Nodelist] -N=[NetWork]\n\n");
          printf
            ("-Process - Processes all segment files and submits changes, if needed.\n");
          printf
            ("-Force   - During a production run, Forces submission to submit address\n");
          printf
            ("-Test    - Tests & accepts new inbound segments. (default)\n");
          printf
            ("-Merge=  - Creates a merged nodelist. [Nodelist + Segment]\n");
          printf ("-Name=   - Network Name.\n\n");
          printf ("Statistic Files and Netmail:\n");
          printf ("nlmake [nlmake.ctl] -ALL -ERR -STA -LAS\n\n");
          printf ("-ALL     - Tests All & accepts new inbound segments.\n");
          printf ("-ERRors  - NetMail Notify all (current errors).\n");
          printf ("-STAts   - Saves stats to [nlmake].ERR.\n");
          printf
            ("-LASt    - Last segment receive dates output to lastin.txt\n");
          printf
            ("-LATe    - NetMail Notify all who's segments are older than MAXAGE.\n\n");
          printf ("Debugging and Flags:\n");
          printf ("nlmake [nlmake.ctl] -D -COM\n\n");
          printf
            ("-Display - Displays what is read from the *.ctl file. (debug)\n");
          printf ("-COMpile - Refresh Database with flags.ctl.\n");
#else
          printf
            ("nlmake [nlmake.ctl] /P /F /T /M=[Nodelist] /N=[NetWork]\n");
          printf
            ("/Process - Processes all segment files and submits changes, if needed.\n");
          printf
            ("/Force   - During a production run, Forces submission to submit address\n");
          printf
            ("/Test    - Tests & accepts new inbound segments. (default)\n");
          printf
            ("/Merge=  - Creates a merged nodelist. [Nodelist + Segment]\n");
          printf ("/Name=   - Network Name.\n\n");
          printf ("-Statistic Files and Netmail:\n");
          printf ("nlmake [nlmake.ctl] /ALL /ERR /STA /LAS\n\n");
          printf ("/ALL     - Tests All & accepts new inbound segments.\n");
          printf ("/ERRors  - NetMail Notify all (current errors).\n");
          printf ("/STAts   - Saves stats to [nlmake].ERR.\n");
          printf
            ("/LASt    - Last segment receive dates output to lastin.txt\n");
          printf
            ("/LATe    - NetMail Notify all who's segments are older than MAXAGE.\n\n");
          printf ("-Debugging and Flags:\n");
          printf ("nlmake [nlmake.ctl] /D /COM\n\n");
          printf
            ("/Display - Displays what is read from the *.ctl file. (debug)\n");
          printf ("/COMpile - Refresh Database with flags.ctl.\n");
#endif
          logtext ("User requested command line help", 4, YES);
          logwrite (SYS_STOP, 0);
          closelog ();          // close logfile
          exit (0);
          cntr = 0;
          break;
        case 8:
          deletefile ("quick.lst");
          cntr = 0;
          break;
        case 9:
          TESTALL = 'Y';
          cntr = 0;
          break;
        case 10:
          STATS = 'Y';
          TESTALL = 'Y';
          cntr = 0;
          break;
        case 11:
          LASTDATES = 'Y';
          cntr = 0;
          break;
        case 12:
          NOTIFY = 'Y';
          TESTALL = 'Y';
          cntr = 0;
          break;
        case 13:
          ISLATE = 'Y';
          TEST = 'Y';
          cntr = 0;
          break;
        case 99:
          if (ControlFile[0] == 0)
            {
              Pathptr = strchr (Parms[cntr2], '.');
              if (Pathptr != NULL)
                strcpy (ControlFile, Parms[cntr2]);
              else
                {
                  strcpy (ControlFile, Parms[cntr2]);
                  strcat (ControlFile, ".ctl");
                }
              temp = fopen (ControlFile, "rt");
              if (temp == NULL)
                ControlFile[0] = 0;
              if (temp)
                fclose (temp);
            }
          else
            printf ("Ignoring bad command in commandline <%s>\n",
                    Parms[cntr2]);
          cntr = 0;
          break;
        default:
          cntr = 0;
          break;
        }
    }

  logtext (textline, 5, YES);

  if (ControlFile[0] == 0)
    {
      sprintf (textline, "Config file not found: assuming nlmake.ctl");
      logtext (textline, 0, YES);
      strcpy (ControlFile, "nlmake.ctl");
    }

  temp = fopen (ControlFile, "rt");

  if (temp == NULL)
    {
      sprintf (textline, "nlmake.ctl config file not found.");
      logtext (textline, 0, YES);
      logwrite (SYS_COMMAND_ABORT, 0);
      logwrite (SYS_STOP, 0);
      closelog ();              // close logfile
      exit (255);
    }
  fclose (temp);


  logwrite (SYS_CF_PROC, 0);
  logtext (ControlFile, 5, NO);

  // process control info
  errorlvl = proctrlfile ();
  if (errorlvl > 0)
    {
      printf ("ERRORLEVEL %d - Critial stop - I/O problem.\n", errorlvl);
      exit (errorlvl);
    }
  sfilecnt--;

  // read compressor file
  if (init_compressors () == 1)
    {
      sprintf (textline, "compress.ctl archiver config file not found.");
      logtext (textline, 0, YES);
      printf ("%s\n", textline);
      logwrite (CFE_ABORT, 0);
      logwrite (SYS_STOP, 0);
      closelog ();              // close logfile
      exit (255);
    }
  else
    {
      sprintf (textline, "compress.ctl successfully loaded.");
      logtext (textline, 5, YES);
    }

  // test control info
  testctrlinfo ();

  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
    {
      temp = fopen ("quick.lst", "rt");
      if (temp == NULL)
        {
          //fclose(temp); - don't do this, temp==NULL!!!
          sprintf (textline,
                   "Compiling flag lookup file from flags.ctl ... ");
          logtext (textline, 0, YES);
          cre_flags_db ("flags.ctl");
          sprintf (textline, "Compile completed.");
          logtext (textline, 0, YES);
          fix_compile_date ();
        }
      else
        {
          fclose (temp);
          if (comp_compile_date () == 1)
            {
              sprintf (textline,
                       "Compiling flag lookup file from flags.ctl ... ");
              logtext (textline, 0, YES);
              cre_flags_db ("flags.ctl");
              sprintf (textline, "Compile completed.");
              logtext (textline, 0, YES);
              fix_compile_date ();
            }
        }
    }

  // disp control info
  if (DISP == 'Y')
    printctrlinfo ();

  if (PROCESS == 'Y')
    {
      errorlvl = process_new ();
      //test if file is here first
      if (CLEANUPOUT == 'N')
        copyfile (OutFile, "outfile.tmp");
      else
        {
          memset (textline, 0, sizeof (textline));
          memcpy (textline, OutFile, strlen (OutFile) - 4);
          FindMostCurr (textline);
          copyfile (textline, "outfile.tmp");
        }
      copynew ();
      errorlvl = process_segment ();
    }

  if (TEST == 'Y')
    {
      errorlvl = process_new ();
//        errorlvl = test_segment();
    }
  if (TESTALL == 'Y')
    errorlvl = test_segment ();

  if (LASTDATES == 'Y')
    list_file_dates ("lastin.txt");


  if (MergePath[0] != 0)
    merge_list (MergePath);

  // save origional data if in and out file have same name
  if (errorlvl >= 2)
    {
      copyfile ("outfile.tmp", OutFile);
    }

  deletefile ("data.tmp");
  deletefile ("outfile.tmp");

  logwrite (SYS_STOP, 0);
  closelog ();                  // close logfile

  if (errorlvl <= 5)
    {
      if (PROCESS == 'Y')
        {
          printf ("ERRORLEVEL %d Process mode - %s\n", errorlvl,
                  errorlevelstr[errorlvl]);
          exit (errorlvl);
        }
      else if (TEST == 'Y')
        {
          errorlvl += 3;
          printf ("ERRORLEVEL %d Test mode - %s\n", errorlvl,
                  errorlevelstr[errorlvl]);
          exit (errorlvl);
        }
    }
  else
    {
      printf ("ERRORLEVEL %d - Critial stop - I/O problem\n", errorlvl);
      exit (errorlvl);
    }


  // Exit Cleanup
}

int
proctrlfile (void)
{

  FILE *ctrlfile;
  FILE *outfilep;
  long eof;
  char str[255];
  char altnot[75];
  char LogLine[255];
  short cntr = 0, cnt = 0;
  char *ptr, *ptr2;
  short mark_file = 0;
  short mark_data = 0;

  memset (LogLine, 0, sizeof (LogLine));

  if (ControlFile[0] == 0)
    return errorlvl;

  outfilep = fopen ("data.tmp", "wt+");
  if (outfilep == NULL)
    {
      if (errno <= 39)
        sprintf (LogLine, "Critial Error - Can not open temporary file <%s>",
                 errnostr[errno]);
      else
        sprintf (LogLine, "Critial Error - Can not open temporary <???>");
      logtext (LogLine, 0, YES);
      errorlvl = 254;
      return errorlvl;
    }


  fprintf (outfilep, ";\n");

  eof = calc_eof (ControlFile);
  ctrlfile = fopen (ControlFile, "rt+");

  if (outfilep == NULL)
    {
      if (errno <= 39)
        sprintf (LogLine, "Critial Error - Can not open file %s <%s>",
                 ControlFile, errnostr[errno]);
      else
        sprintf (LogLine, "Critial Error - Can not open file %s <???>",
                 ControlFile);
      logtext (LogLine, 0, YES);
      //fclose(outfilep); - outfilep is NULL
      errorlvl = 254;
      return errorlvl;
    }
  //fseek (ctrlfile, 0L, SEEK_END);
  //eof = ftell (ctrlfile);
  //fseek (ctrlfile, 0L, SEEK_SET);

  while (1)
    {
      memset (str, 0, sizeof (str));

      str[0] = ';';             // to get us to the first fgets (cheat)

      fgets (str, MAXSTR, ctrlfile);
      linecnt++;

      while (str[0] == ' ')
        memmove (str, str + 1, 253);

      ptr = strchr (str, ';');
      if (ptr != NULL)
        *ptr = 0;

//                      *(ptr+strlen(str)) = 0;
      if (str[0] != 0 && strlen (str) >= 4)
        {
          while (CfilePrms[cntr].String != NULL)
            {
              if (strnicmp
                  (str, CfilePrms[cntr].Single,
                   strlen (CfilePrms[cntr].Single)) == 0)
                {
                  break;
                }
              else
                cntr++;
            }
          if (CfilePrms[cntr].Type == 99)
            {
              if (mark_data == 1)
                cntr = 27;
              else if (mark_file == 1)
                cntr = 28;
            }
          else
            {
              mark_data = 0;
              mark_file = 0;
            }
          switch (CfilePrms[cntr].Type)
            {
            case 0:
              break;
            case 1:             // Make
              ptr = strchr (str, ' ');
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              cnt = 0;
              while (SegmentType[cnt].Single != NULL)
                {
                  if (strnicmp
                      (ptr, SegmentType[cnt].Single,
                       strlen (SegmentType[cnt].Single)) == 0)
                    {
                      MAKETYPE = SegmentType[cnt].Type;
                      break;
                    }
                  cnt++;
                }
              if (SegmentType[cnt].Single == NULL)
                {
                  logwrite (CFE_MAKE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr2 = strchr (ptr, ' ');
              if (ptr2 != NULL)
                {
                  ptr2++;
                  while (ptr2[0] == ' ')
                    memmove (ptr2, ptr2 + 1, (strlen (ptr2) - 1));
                  MAKENUMBER = atoi (ptr2);
                  ptr2 = strchr (ptr2, ' ');
                }
              if (ptr2 != NULL)
                {
                  ptr2++;
                  while (ptr2[0] == ' ')
                    memmove (ptr2, ptr2 + 1, (strlen (ptr) - 1));
                  strncpy (expSourceFile, ptr2, strlenII (ptr2));
                }
              logwrite (CF_MAKE, 0);
              sprintf (LogLine, "%s [%d] [%s]", SegmentType[MAKETYPE].String,
                       MAKENUMBER, expSourceFile);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 2:             // Name
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_NAME, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              if (strlenII (ptr) <= 20)
                strncpy (NetWorkName, ptr, strlenII (ptr));
              else
                strncpy (NetWorkName, ptr, 20);
              logwrite (CF_NAME, 0);
              logtext (NetWorkName, 5, NO);
              cntr = 0;
              break;
            case 3:             // Publish
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_PUBLISH, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              cnt = 0;
              while (WeekDays[cnt].Single != NULL)
                {
                  if (strnicmp
                      (ptr, WeekDays[cnt].Single,
                       strlen (WeekDays[cnt].Single)) == 0)
                    {
                      strncpy (Publish_day, WeekDays[cnt].String,
                               strlen (WeekDays[cnt].String));
                      break;
                    }
                  cnt++;
                }
              if (WeekDays[cnt].Single == NULL)
                {
                  logwrite (CFE_PUBLISH, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_PUBLISH, 0);
              sprintf (LogLine, "%s", Publish_day);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 4:             // Process
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_PROCESS, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              cnt = 0;
              while (WeekDays[cnt].Single != NULL)
                {
                  if (strnicmp
                      (ptr, WeekDays[cnt].Single,
                       strlen (WeekDays[cnt].Single)) == 0)
                    {
                      strncpy (Process_day, WeekDays[cnt].String,
                               strlen (WeekDays[cnt].String));
                      break;
                    }
                  cnt++;
                }
              if (WeekDays[cnt].Single == NULL)
                {
                  logwrite (CFE_PROCESS, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                {
                  logwrite (CF_PROCESS, 0);
                  sprintf (LogLine, "%s", Process_day);
                  logtext (LogLine, 5, NO);
                  if (is_proc_day (cnt) == 0)
                    {
                      if (TEST == 'N')
                        PROCESS = 'Y';
                      else
                        PROCESS = 'N';
                    }
                  else if (PROCESS == 'Y')
                    TEST = 'N';
                  else
                    TEST = 'Y';
                }
              cntr = 0;
              break;
            case 5:             // Merge
              if (MergePath[0] == 0)
                {
                  ptr = strchr (str, ' ');
                  if (ptr == NULL)
                    {
                      logwrite (CFE_MERGE, linecnt);
                      logwrite (CFE_ABORT, 0);
                      logwrite (SYS_STOP, 0);
                      closelog ();      // close logfile
                      exit (255);       // abort fatal cfg file error
                    }
                  ptr++;

                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  strncpy (MergePath, ptr, strlenII (ptr));
                }
              if (MergePath[0] == 0)
                logwrite (CFE_MERGE, linecnt);
              else
                logwrite (CF_MERGE, 0);
              sprintf (LogLine, "%s", MergePath);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 6:             // Private
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_PRIVATE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              cnt = 0;
              while (PVTDisposition[cnt].Single != NULL)
                {
                  if (strnicmp
                      (ptr, PVTDisposition[cnt].Single,
                       strlen (PVTDisposition[cnt].Single)) == 0)
                    {
                      DispPVT = PVTDisposition[cnt].Type;
                      break;
                    }
                  else
                    cnt++;
                }
              if (PVTDisposition[cnt].Single == NULL)
                {
                  logwrite (CFE_PRIVATE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_PRIVATE, 0);
              sprintf (LogLine, "%s", PVTDisposition[cnt].String);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 7:             // MinPhone
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_MINPHONE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              MinPhone = atoi (ptr);
              if (MinPhone >= 30 || ptr[0] == 0)
                {
                  logwrite (CFE_MINPHONE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_MINPHONE, 0);
              sprintf (LogLine, "%d", MinPhone);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 8:             // BaudRate
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_BAUDRATE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              BaudRates[0] = ',';
              strncat (BaudRates, ptr, strlenII (ptr));
              breakbaud ();
              // here
              cntr = 0;
              break;
            case 9:             // Master
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_MASTER, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              strncpy (Master, ptr, strlenII (ptr));
              if (Master[0] == 0)
                logwrite (CFE_MASTER, linecnt);
              else
                logwrite (CF_MASTER, 0);
              sprintf (LogLine, "%s", Master);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 10:            // Uploads
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_UPLOADS, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              strncpy (Uploads, ptr, strlenII (ptr));
              if (Uploads[0] == 0)
                logwrite (CFE_UPLOADS, linecnt);
              else
                logwrite (CF_UPLOADS, 0);
              sprintf (LogLine, "%s", Uploads);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 11:            // MailFiles
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_MAILFILES, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              strncpy (MailFiles, ptr, strlenII (ptr));
              if (MailFiles[0] == 0)
                {
                  logwrite (CFE_MAILFILES, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_MAILFILES, 0);
              sprintf (LogLine, "%s", MailFiles);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 12:            // Update
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_UPDATE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              strncpy (Update, ptr, strlenII (ptr));
              if (Update[0] == 0)
                {
                  logwrite (CFE_UPDATE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_UPDATE, 0);
              sprintf (LogLine, "%s", Update);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 13:            // BadFiles
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_BADFILES, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              strncpy (BadFiles, ptr, strlenII (ptr));
              if (BadFiles[0] == 0)
                {
                  logwrite (CFE_BADFILES, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_BADFILES, 0);
              sprintf (LogLine, "%s", BadFiles);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 14:            // OutFile
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_OUTFILE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              if (strlenII (ptr) <= MAXFILELEN)
                strncpy (OutFile, ptr, strlenII (ptr));
              else
                strncpy (OutFile, ptr, MAXFILELEN);
              if (OutFile[0] == 0)
                {
                  logwrite (CFE_OUTFILE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_OUTFILE, 0);
              sprintf (LogLine, "%s", OutFile);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 15:            // OutPath
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_OUTPATH, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              strncpy (OutPath, ptr, strlenII (ptr));
              if (OutPath[0] == 0)
                {
                  logwrite (CFE_OUTPATH, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_OUTPATH, 0);
              sprintf (LogLine, "%s", OutPath);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 16:            // Threshold
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_THRESHOLD, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              ptr = strchr (ptr, ' ');
              if (ptr != NULL)
                *ptr = 0;
              ptr = strchr (str, ' ');
              ptr++;
              threshold.arc_size = atoi (ptr);
              ptr = strchr (ptr, 0);
              ptr++;
              threshold.diff_size = atoi (ptr);
              //here
              cntr = 0;
              break;
            case 17:            // Arc
              ptr = strchr (str, ' ');
              if (ptr != NULL)
                {
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  ARC = atoi (ptr);
                }
              else
                {
                  logwrite (CFE_ARC, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              // here
              cntr = 0;
              break;
            case 18:            // OutDiff
              ptr = strchr (str, ' ');
              if (ptr != NULL)
                {
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  if (strlenII (ptr) <= MAXFILELEN)
                    strncpy (OutDiff, ptr, strlenII (ptr));
                  else
                    strncpy (OutDiff, ptr, MAXFILELEN);
                  logwrite (CF_OUTDIFF, 0);
                  sprintf (LogLine, "%s", OutDiff);
                  logtext (LogLine, 5, NO);
                }
              else
                {
                  logwrite (CFE_OUTDIFF, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              cntr = 0;
              break;
            case 19:            // Cleanup
              CLEANUP = 'Y';
              //here
              cntr = 0;
              break;
            case 20:            // NetAddess
              if (NetAddress[0] == 0)
                {
                  ptr = strchr (str, ' ');
                  if (ptr == NULL)
                    {
                      logwrite (CFE_NETADDRESS, linecnt);
                      logwrite (CFE_ABORT, 0);
                      logwrite (SYS_STOP, 0);
                      closelog ();      // close logfile
                      exit (255);       // abort fatal cfg file error
                    }
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  strncpy (NetAddress, ptr, strlenII (ptr));
                }
              if (NetAddress[0] == 0)
                {
                  logwrite (CFE_NETADDRESS, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              if (breakaddress () == 1)
                {
                  logwrite (CFE_NETADDRESS_INV, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              logwrite (CF_NETADDRESS, 0);
              sprintf (LogLine, "%s", NetAddress);
              logtext (LogLine, 5, NO);
              // here
              strcpy (altnot, ptr);
              ptr = strchr (altnot, '<');
              ptr2 = strchr (altnot, '>');
              if (ptr == NULL || ptr2 == NULL)
                segfile[0].NameNotify[0] = 0;
              else
                {
                  if ((ptr2 - ptr) <= 72)
                    strncpy (segfile[0].NameNotify, ptr + 1,
                             (ptr2 - ptr - 1));
                  else
                    strncpy (segfile[0].NameNotify, ptr + 1, 72);
                }
              cntr = 0;
              break;
            case 21:            // Messages
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_MESSAGES, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              strncpy (Messages, ptr, strlenII (ptr));
              if (Messages[0] == 0)
                {
                  logwrite (CFE_MESSAGES, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_MESSAGES, 0);
              sprintf (LogLine, "%s", Messages);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 22:            // Submit
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_SUBMIT, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              if (SubAddress[0] == 0)
                {
                  strncpy (SubAddress, ptr, strlenII (ptr));
                  if (SubAddress[0] == 0)
                    {
                      logwrite (CFE_SUBMIT, linecnt);
                      logwrite (CFE_ABORT, 0);
                      logwrite (SYS_STOP, 0);
                      closelog ();      // close logfile
                      exit (255);       // abort fatal cfg file error
                    }
                  else
                    logwrite (CF_SUBMIT, 0);
                  sprintf (LogLine, "%s ", SubAddress);
                  strcpy (altnot, ptr);
                  cnt = 0;
                  while (MFlagstype[cnt].String != NULL)
                    {
                      if (strstr (strupr (ptr), MFlagstype[cnt].String) !=
                          NULL)
                        {
                          switch (MFlagstype[cnt].Type)
                            {
                            case 0:
                              break;
                            case 1:
                              mnotify[3].Crash = 'Y';
                              strcat (LogLine, "Crash,");
                              break;
                            case 2:
                              mnotify[3].Hold = 'Y';
                              strcat (LogLine, "Hold,");
                              break;
                            case 3:
                              mnotify[3].Intl = 'Y';
                              strcat (LogLine, "Intl,");
                              break;
                            case 4:
                              mnotify[3].Normal = 'Y';
                              strcat (LogLine, "Normal,");
                              break;
                            default:
                              break;
                            }
                        }
                      cnt++;
                    }
                  logtext (LogLine, 5, NO);
                }
              // here
              ptr = strchr (altnot, '<');
              ptr2 = strchr (altnot, '>');
              if (ptr == NULL || ptr2 == NULL)
                SubNameNotify[0] = 0;
              else
                {
                  if ((ptr2 - ptr) <= 72)
                    strncpy (SubNameNotify, ptr + 1, (ptr2 - ptr - 1));
                  else
                    strncpy (SubNameNotify, ptr + 1, 72);
                }
              cntr = 0;
              break;
            case 23:            // Notify
              parnotify (str);
              cntr = 0;
              break;
            case 24:            // Copyright
              ptr = strchr (str, ' ');
              if (ptr != NULL)
                {
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  memset (CopyRight, 0, sizeof (CopyRight));
                  strncpy (CopyRight, ptr, strlenII (ptr));
                }
              if (CopyRight[0] == 0)
                {
                  logwrite (CFE_COPYRIGHT, linecnt);
                  sprintf (LogLine,
                           "Assuming default copyright: cpyright.txt");
                  logtext (LogLine, 5, YES);
                }
              else
                {
                  logwrite (CF_COPYRIGHT, 0);
                  sprintf (LogLine, "%s", CopyRight);
                  logtext (LogLine, 5, NO);
                }
              cntr = 0;
              break;
            case 25:            // Prolog
              ptr = strchr (str, ' ');
              if (ptr != NULL)
                {
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  memset (Prolog, 0, sizeof (Prolog));
                  strncpy (Prolog, ptr, strlenII (ptr));
                }
              if (Prolog[0] == 0)
                {
                  logwrite (CFE_PROLOG, linecnt);
                  sprintf (LogLine, "Assuming default prolog: prolog.txt");
                  logtext (LogLine, 5, YES);
                }
              else
                {
                  logwrite (CF_PROLOG, 0);
                  sprintf (LogLine, "%s", Prolog);
                  logtext (LogLine, 5, NO);
                }
              cntr = 0;
              break;
            case 26:            // Epilog
              ptr = strchr (str, ' ');
              if (ptr != NULL)
                {
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  memset (Epilog, 0, sizeof (Epilog));
                  strncpy (Epilog, ptr, strlenII (ptr));
                }
              if (Epilog[0] == 0)
                {
                  logwrite (CFE_EPILOG, linecnt);
                  sprintf (LogLine, "Assuming default epilog: epilog.txt");
                  logtext (LogLine, 5, YES);
                }
              else
                logwrite (CF_EPILOG, 0);
              sprintf (LogLine, "%s", Epilog);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 27:            // Comments
              ptr = strchr (str, ' ');
              if (ptr != NULL)
                {
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  memset (Comments, 0, sizeof (Comments));
                  strncpy (Comments, ptr, strlenII (ptr));
                }
              if (Comments[0] == 0)
                {
                  logwrite (CFE_COMMENTS, linecnt);
                  sprintf (LogLine, "No File Name");
                  logtext (LogLine, 5, YES);
                }
              else
                logwrite (CF_COMMENTS, 0);
              sprintf (LogLine, "%s", Comments);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 28:            // Data
              if (expSourceFile[0] != 0)
                {
                  logtext ("Master input file and DATA stream both specified",
                           0, YES);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);
                }
              if (mark_data != 0)
                {
                  if (str[0] == 0)
                    {
                      fprintf (outfilep, ";\n");
                      cntr = 0;
                      break;
                    }
                  fprintf (outfilep, "%s", str);
                }
              else
                {
                  mark_data = 1;
                  Dlineoff = linecnt;
                }
              cntr = 0;
              break;
            case 29:            // Files
              if (sfilecnt > (MAXSUBFILES - 2))
                {
                  logwrite (CFE_MAXFILES, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              if (mark_file != 0)
                {
                  if (str[0] == 0)
                    {
                      cntr = 0;
                      break;
                    }
                  cnt = 0;
                  while (SegmentType[cnt].Single != NULL)
                    {
                      if (strnicmp
                          (str, SegmentType[cnt].Single,
                           strlen (SegmentType[cnt].Single)) == 0)
                        {
                          segfile[sfilecnt].SegmentType =
                            SegmentType[cnt].Type;
                          break;
                        }
                      else
                        cnt++;
                    }
                  if (SegmentType[cnt].Single == NULL)
                    {
                      logwrite (CFE_FILES_STYPE, linecnt);
                      logwrite (CFE_ABORT, 0);
                      logwrite (SYS_STOP, 0);
                      closelog ();      // close logfile
                      exit (255);       // abort fatal cfg file error
                      // Error
                      cntr = 0;
                      break;
                    }
                  ptr = strchr (str, ' ');
                  if (ptr == NULL)
                    {
                      logwrite (CFE_FILES, linecnt);
                      logwrite (CFE_ABORT, 0);
                      logwrite (SYS_STOP, 0);
                      closelog ();      // close logfile
                      exit (255);       // abort fatal cfg file error
                    }
                  // pointer crap
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  // need to extp zone and net
                  if (MAKETYPE <= segfile[sfilecnt].SegmentType)
                    {
                      logwrite (CFE_FILES_STYPE, linecnt);
                      sprintf (LogLine,
                               "A %s segment can not contain a %s segment",
                               SegmentType[MAKETYPE].String,
                               SegmentType[segfile[sfilecnt].SegmentType].
                               String);
                      logtext (LogLine, 0, YES);
                      logwrite (CFE_ABORT, 0);
                      logwrite (SYS_STOP, 0);
                      closelog ();      // close logfile
                      exit (255);       // abort fatal cfg file error
                    }
                  switch (segfile[sfilecnt].SegmentType)
                    {
                    case 0:     // Node
                      segfile[sfilecnt].Zone = MAKEZONE;
                      segfile[sfilecnt].Net = MAKENET;
                      segfile[sfilecnt].Node = atoi (ptr);
                      break;
                    case 1:     // Hub
                      segfile[sfilecnt].Zone = MAKEZONE;
                      segfile[sfilecnt].Net = MAKENET;
                      segfile[sfilecnt].Node = atoi (ptr);
                      break;
                    case 2:     // Host
                      segfile[sfilecnt].Zone = MAKEZONE;
                      segfile[sfilecnt].Net = atoi (ptr);
                      segfile[sfilecnt].Node = 0;
                      break;
                    case 3:     // Net
                      segfile[sfilecnt].Zone = MAKEZONE;
                      segfile[sfilecnt].Net = atoi (ptr);
                      segfile[sfilecnt].Node = 0;
                      break;
                    case 4:     // Region
                      segfile[sfilecnt].Zone = MAKEZONE;
                      segfile[sfilecnt].Net = atoi (ptr);
                      segfile[sfilecnt].Node = 0;
                      break;
                    case 5:     // Zone
                      segfile[sfilecnt].Zone = atoi (ptr);
                      segfile[sfilecnt].Net = atoi (ptr);
                      segfile[sfilecnt].Node = 0;
                      break;
                    case 6:     // Composite
                      logwrite (CFE_FILES_STYPE, linecnt);
                      logwrite (CFE_ABORT, 0);
                      logwrite (SYS_STOP, 0);
                      closelog ();      // close logfile
                      exit (255);       // abort fatal cfg file error
                      break;
                    default:
                      cntr = 0;
                      break;
                    }
                  ptr = strchr (ptr, ' ');
                  ptr++;
                  while (ptr[0] == ' ')
                    memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                  strncpy (segfile[sfilecnt].FileName, ptr, strlenII (ptr));
                  if (strlen (segfile[sfilecnt].FileName) >= 14)
                    logwrite (CFE_FILES_NAME, linecnt);
                  else
                    {
                      logwrite (CF_FILES, 0);
                      sprintf (LogLine, "%s", segfile[sfilecnt].FileName);
                      logtext (LogLine, 5, NO);
                    }
                  ptr = strchr (ptr, ' ');
                  if (ptr != NULL)
                    {
                      while (ptr[0] == ' ')
                        memmove (ptr, ptr + 1, (strlen (ptr) - 1));
                      memset (altnot, 0, sizeof (altnot));
                      strcpy (altnot, ptr);
                      // printf("anot1  %s\n",altnot);
                      ptr2 = strtok (altnot, " \r\n");  // 1
                      if (strchr (ptr2, '/') != NULL && ptr2 != NULL)
                        strcpy (segfile[sfilecnt].AltNotify, ptr2);
                      else
                        segfile[sfilecnt].AltNotify[0] = 0;
                      strcpy (altnot, ptr);
                      ptr = strchr (altnot, '<');
                      ptr2 = strchr (altnot, '>');
                      if (ptr == NULL || ptr2 == NULL)
                        segfile[sfilecnt].NameNotify[0] = 0;
                      else
                        {
                          if ((ptr2 - ptr) <= 72)
                            strncpy (segfile[sfilecnt].NameNotify, ptr + 1,
                                     (ptr2 - ptr - 1));
                          else
                            strncpy (segfile[sfilecnt].NameNotify, ptr + 1,
                                     72);
                        }
                      //printf("%d - NN: %s AN: %s\n",sfilecnt,segfile[sfilecnt].NameNotify,segfile[sfilecnt].AltNotify);
                    }
                  sfilecnt++;
                }
              else
                {
                  mark_file = 1;
                  Flineoff = linecnt - 1;
                }
              cntr = 0;
              break;
            case 30:            // LogLevel
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_LOGLEVEL, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              loglvl = atoi (ptr);
              if (loglvl <= 0)
                loglvl = 0;
              if (loglvl >= 5)
                loglvl = 5;
              // here
              cntr = 0;
              break;
            case 31:            // Compress
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_COMPRESS, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              cnt = 0;
              while (compressor_name[cnt] != 0)
                {
                  if (strnicmp
                      (ptr, compressor_name[cnt],
                       strlen (compressor_name[cnt])) == 0)
                    {
                      DefCompressor = cnt;
                      sprintf (LogLine, "Outbound compressor set to %s",
                               compressor_name[cnt]);
                      logtext (LogLine, 2, YES);
                      break;
                    }
                  else
                    cnt++;
                }
              if (compressor_name[cnt] == 0)
                {
                  DefCompressor = 0;
                  sprintf (LogLine, "Outbound compressor default %s",
                           compressor_name[0]);
                  logtext (LogLine, 2, YES);
                }
              cntr = 0;
              break;
            case 32:            // Flags
              FLAGCHK = 'Y';
              ptr = strstr (strupr (str), "NOCASE");
              if (ptr != NULL)
                IGNORCASE = 'Y';
              cntr = 0;
              break;
            case 33:            // FlagsAuto
              FLAGCHKAUTO = 'Y';
              ptr = strstr (strupr (str), "NOCASE");
              if (ptr != NULL)
                IGNORCASE = 'Y';
              cntr = 0;
              break;
            case 34:            // Warnings
              WARNINGS = 'Y';
              cntr = 0;
              break;
            case 35:            // PVTlevel
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_PVTLEVEL, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              cnt = 0;
              while (SegmentType[cnt].Single != NULL)
                {
                  if (strnicmp
                      (ptr, SegmentType[cnt].Single,
                       strlen (SegmentType[cnt].Single)) == 0)
                    {
                      PVTlvl = SegmentType[cnt].Type;
                      break;
                    }
                  else
                    cnt++;
                }
              if (SegmentType[cnt].Single == NULL)
                {
                  logwrite (CFE_PVTLEVEL, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              else
                logwrite (CF_PVTLEVEL, 0);
              sprintf (LogLine, "%s", SegmentType[cnt].String);
              logtext (LogLine, 5, NO);
              cntr = 0;
              break;
            case 36:            // LogLevel
              ptr = strchr (str, ' ');
              if (ptr == NULL)
                {
                  logwrite (CFE_MAXAGE, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              ptr++;
              while (ptr[0] == ' ')
                memmove (ptr, ptr + 1, (strlen (ptr) - 1));
              MAXAGE = atoi (ptr);
              if (MAXAGE <= 0)
                MAXAGE = 0;
              if (MAXAGE >= 365)
                MAXAGE = 365;
              // here
              cntr = 0;
              break;
            case 99:
            default:
              //printf("Ignoring bad command in connand file\n");
              if (str[0] != 0)
                {
                  logwrite (CFE_ERR_BAD, linecnt);
                  logwrite (CFE_ABORT, 0);
                  logwrite (SYS_STOP, 0);
                  closelog ();  // close logfile
                  exit (255);   // abort fatal cfg file error
                }
              cntr = 0;
              break;
            }

        }
      if (eof == ftell (ctrlfile))
        break;
    }

  fclose (outfilep);
  fclose (ctrlfile);

  return 0;
}

short
strlenII (char *ptr)
{
  short i = 0;

  while (ptr[i] >= 33 && ptr[i] != ';' && i <= MAXSTR)
    i++;
  return (i);
}

short
breakaddress (void)
{
  char *pnet, *pnode, *pzone;

  if (strchr (NetAddress, ':') != NULL)
    {
      pzone = NetAddress;
      if (strchr (NetAddress, '/') != NULL)
        {
          pnet = strchr (NetAddress, ':');
          pnode = strchr (NetAddress, '/');
        }
    }
  else
    {
      if (strchr (NetAddress, '/') != NULL)
        {
          pnode = strchr (NetAddress, '/');
        }
      else
        return (1);
    }

  if (pzone == NULL)
    MAKEZONE = 1;
  else
    MAKEZONE = atoi (pzone);

  if (pnet != NULL)
    MAKENET = atoi (pnet + 1);

  if (pnode != NULL)
    MAKENODE = atoi (pnode + 1);


  //printf("submit address %d:%d/%d",MAKEZONE,MAKENET,MAKENODE);

  return (0);

}

void
breakbaud (void)
{
  char *cpt;
  short cntr = 0;
  char *delims = { " ,\r\n" };

  cpt = strtok (BaudRates, delims);     // 1
  bauds[cntr] = atol (cpt);

  for (cntr = 1; cntr < MAXBAUD; cntr++)
    {
      cpt = strtok (NULL, delims);
      if (cpt != NULL)
        {
          bauds[cntr] = atol (cpt);
        }
      else
        bauds[cntr] = 0;
    }


}

void
parnotify (char *str)
{
  short cnt1 = 0, cnt2 = 0;

  while (NotifyType[cnt1].Single != NULL)
    {
      if (strstr (strupr (str), NotifyType[cnt1].Single) != NULL)
        {
          mnotify[cnt1].active = 'Y';
          while (MFlagstype[cnt2].String != NULL)
            {
              if (strstr (strupr (str), MFlagstype[cnt2].String) != NULL)
                {
                  switch (MFlagstype[cnt2].Type)
                    {
                    case 0:
                      break;
                    case 1:
                      mnotify[cnt1].Crash = 'Y';
                      break;
                    case 2:
                      mnotify[cnt1].Hold = 'Y';
                      break;
                    case 3:
                      mnotify[cnt1].Intl = 'Y';
                      break;
                    case 4:
                      mnotify[cnt1].Normal = 'Y';
                      break;
                    default:
                      mnotify[cnt1].Normal = 'Y';
                      break;
                    }

                }
              cnt2++;
            }
        }
      cnt1++;
      cnt2 = 0;
    }


}

void
testctrlinfo (void)
{
  FILE *temp;
  char buffer[255];
  short jfriday;
  char *CWD;

  CWD = getcwd (NULL, 0);

  // make directive test

  if (MAKETYPE != 6)
    if (MAKENUMBER != 0)
      {
        segfile[0].Net = MAKENUMBER;
      }
    else
      {
        logtext ("Make Directive does not indicate a number ", 0, YES);
        logwrite (CFE_ABORT, 0);
        logwrite (SYS_STOP, 0);
        closelog ();            // close logfile
        exit (255);
      }

  // set and test name
  if (NetWorkName[0] == 0 && MAKETYPE != 5)
    {
      strcpy (NetWorkName, SegmentType[MAKETYPE].String);
    }

  // set publish day
  if (Publish_day[0] == 0)
    {
      logtext ("No Publish statement - Publish default = Friday", 4, YES);
      strcpy (Publish_day, "Friday");
    }

  // set process directive

  if (Process_day[0] == 0)
    {
      logtext ("No Process statement - Processing day not set", 4, YES);
    }


  // test merge path
  if (MergePath[0] == 0)
    logtext ("No Merge statement - Merging not done", 4, YES);
  else
    {
      FindMostCurr (MergePath);
      temp = fopen (MergePath, "rt");
      if (temp == NULL)
        {
          logtext ("Merge File not found - Merging not done", 2, YES);
          MergePath[0] = 0;
        }
      else
        fclose (temp);
    }
  // test baudrates set defaults
  if (BaudRates[0] == 0)
    {
      logtext ("No BaudRates statement - Def 300,1200,2400,4800,9600", 4,
               YES);
      strcpy (BaudRates, ",300,1200,2400,4800,9600");
      breakbaud ();
    }

  // test master
  if (Master[0] == 0)
    {
      logtext ("No Master statement Assuming Current Directory", 3, YES);
      sprintf (Master, "%s\\", CWD);
    }
  else
    {
      if (Master[strlen (Master) - 1] != PathChar)
        {
#ifdef LINUX
          if (mkdir (Master, 0750) == 0)
#else
          if (mkdir (Master) == 0)
#endif
            logtext ("Master Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Master[strlen (Master)] = PathChar;
        }
      else
        {
          Master[strlen (Master) - 1] = 0;
#ifdef LINUX
          if (mkdir (Master, 0750) == 0)
#else
          if (mkdir (Master) == 0)
#endif
            logtext ("Master Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Master[strlen (Master)] = PathChar;
        }

    }
  // test uploads
  if (Uploads[0] == 0)
    logtext ("No Uploads statement NotScanned", 3, YES);
  else
    {
      if (Uploads[strlen (Uploads) - 1] != PathChar)
        {
#ifdef LINUX
          if (mkdir (Uploads, 0750) == 0)
#else
          if (mkdir (Uploads) == 0)
#endif
            logtext ("Uploads Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Uploads[strlen (Uploads)] = PathChar;
        }
      else
        {
          Uploads[strlen (Uploads) - 1] = 0;
#ifdef LINUX
          if (mkdir (Uploads, 0750) == 0)
#else
          if (mkdir (Uploads) == 0)
#endif
            logtext ("Uploads Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Uploads[strlen (Uploads)] = PathChar;
        }

    }
  // test mailfiles
  // need to figure out if this is needed
  if (MailFiles[0] == 0)
    {
      logtext ("No MailFiles statement Can not receive segments", 1, YES);
    }
  else
    {
      if (MailFiles[strlen (MailFiles) - 1] != PathChar)
        {
#ifdef LINUX
          if (mkdir (MailFiles, 0750) == 0)
#else
          if (mkdir (MailFiles) == 0)
#endif
            logtext ("MailFiles Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          MailFiles[strlen (MailFiles)] = PathChar;
        }
      else
        {
          MailFiles[strlen (MailFiles) - 1] = 0;
#ifdef LINUX
          if (mkdir (MailFiles, 0750) == 0)
#else
          if (mkdir (MailFiles) == 0)
#endif
            logtext ("MailFiles Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          MailFiles[strlen (MailFiles)] = PathChar;
        }
    }

  // update
  // need to figure out if this is needed
  if (Update[0] == 0)
    {
      logtext ("No Update statement Assuming Current Directory", 2, YES);
      sprintf (Update, "%s\\", CWD);
    }
  else
    {
      if (Update[strlen (Update) - 1] != PathChar)
        {
#ifdef LINUX
          if (mkdir (Update, 0750) == 0)
#else
          if (mkdir (Update) == 0)
#endif
            logtext ("Update Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Update[strlen (Update)] = PathChar;
        }
      else
        {
          Update[strlen (Update) - 1] = 0;
#ifdef LINUX
          if (mkdir (Update, 0750) == 0)
#else
          if (mkdir (Update) == 0)
#endif
            logtext ("Update Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Update[strlen (Update)] = PathChar;
        }
    }

  // test badfiles
  // need to figure out if this is needed
  if (BadFiles[0] == 0)
    logtext ("No BadFiles statement - Files will be deleted", 2, YES);
  else
    {
      if (BadFiles[strlen (BadFiles) - 1] != PathChar)
        {
#ifdef LINUX
          if (mkdir (BadFiles, 0750) == 0)
#else
          if (mkdir (BadFiles) == 0)
#endif
            logtext ("BadFiles Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          BadFiles[strlen (BadFiles)] = PathChar;
        }
      else
        {
          BadFiles[strlen (BadFiles) - 1] = 0;
#ifdef LINUX
          if (mkdir (BadFiles, 0750) == 0)
#else
          if (mkdir (BadFiles) == 0)
#endif
            logtext ("BadFiles Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          BadFiles[strlen (BadFiles)] = PathChar;
        }
    }
  // test outpath
  if (OutPath[0] == 0)
    {
      logtext ("No OutPath statement Assuming Master Directory", 4, YES);
      strcpy (OutPath, Master);
    }
  else
    {
      if (OutPath[strlen (OutPath) - 1] != PathChar)
        {
#ifdef LINUX
          if (mkdir (OutPath, 0750) == 0)
#else
          if (mkdir (OutPath) == 0)
#endif
            logtext ("OutPath Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          OutPath[strlen (OutPath)] = PathChar;
        }
      else
        {
          OutPath[strlen (OutPath) - 1] = 0;
#ifdef LINUX
          if (mkdir (OutPath, 0750) == 0)
#else
          if (mkdir (OutPath) == 0)
#endif
            logtext ("OutPath Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          OutPath[strlen (OutPath)] = PathChar;
        }
    }

  // test outfile

  if (OutFile[0] == 0)
    {
      logtext ("No OutFile statement critical error", 0, YES);
      logwrite (CFE_ABORT, 0);
      logwrite (SYS_STOP, 0);
      closelog ();              // close logfile
      exit (255);
    }
  else
    {
      memset (buffer, 0, sizeof (buffer));
      strcpy (buffer, OutFile);
      memset (OutFile, 0, sizeof (OutFile));
      CLEANUPOUT = 'N';
      if (strchr (buffer, '.') == NULL)
        {
          jfriday = getJDate (1);
          sprintf (OutFile, "%s%s.%03d", OutPath, buffer, jfriday);
          if (CLEANUP == 'Y')
            CLEANUPOUT = 'Y';
        }
      else
        {
          sprintf (OutFile, "%s%s", OutPath, buffer);
          if (threshold.diff_size > -1)
            {
              logwrite (CFE_THRESHOLD, 0);
              logtext
                ("Can not create Diff - Outfile has an Explicit extension", 0,
                 YES);
              logwrite (CFE_ABORT, 0);
              logwrite (SYS_STOP, 0);
              closelog ();      // close logfile
              exit (255);
            }
        }
      // set threshold crap
    }
  // test outdiff
  if (OutDiff[0] == 0)
    logtext ("No OutDiff statement - Diff not produced", 4, YES);
  else
    {
      memset (buffer, 0, sizeof (buffer));
      strcpy (buffer, OutDiff);
      memset (OutDiff, 0, sizeof (OutDiff));
      if (strchr (OutDiff, '.') == NULL)
        {
          jfriday = getJDate (1);
          sprintf (OutDiff, "%s%s.%03d", OutPath, buffer, jfriday);
        }
      else
        {
          logtext ("OutDiff statement critical error", 0, YES);
          logtext ("OutDiff can not have an extension", 0, YES);
          logwrite (CFE_ABORT, 0);
          logwrite (SYS_STOP, 0);
          closelog ();          // close logfile
          exit (255);
        }
    }


  // test messages
  if (Messages[0] == 0)
    {
      logtext ("No Messages will be sent including Attached Segments", 1,
               YES);
    }
  else
    {
      if (Messages[strlen (Messages) - 1] != PathChar)
        {
#ifdef LINUX
          if (mkdir (Messages, 0750) == 0)
#else
          if (mkdir (Messages) == 0)
#endif
            logtext ("Messages Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Messages[strlen (Messages)] = PathChar;
        }
      else
        {
          Messages[strlen (Messages) - 1] = 0;
#ifdef LINUX
          if (mkdir (Messages, 0750) == 0)
#else
          if (mkdir (Messages) == 0)
#endif
            logtext ("Messages Directory not found - Creating", 1, YES);
          else
            makedirerror ();
          Messages[strlen (Messages)] = PathChar;
        }
    }
  // test copyright
  if (strcmp (CopyRight, "NONE") == 0)
    logtext ("No Copyright statement - None appended", 5, YES);
  else
    {
      if (strchr (CopyRight, PathChar) == NULL)
        {
          memset (buffer, 0, sizeof (buffer));
          strcpy (buffer, CopyRight);
          memset (CopyRight, 0, sizeof (CopyRight));
          sprintf (CopyRight, "%s%s", Master, buffer);
        }
      temp = fopen (CopyRight, "rt");
      if (temp == NULL)
        logtext ("Copyright File not found - None appended", 4, YES);
      else
        fclose (temp);
    }
  // test prolog
  if (strcmp (Prolog, "NONE") == 0)
    logtext ("No Prolog statement - None appended", 5, YES);
  else
    {
      if (strchr (Prolog, PathChar) == NULL)
        {
          memset (buffer, 0, sizeof (buffer));
          strcpy (buffer, Prolog);
          memset (Prolog, 0, sizeof (Prolog));
          sprintf (Prolog, "%s%s", Master, buffer);
        }
      temp = fopen (Prolog, "rt");
      if (temp == NULL)
        logtext ("Prolog File not found - None appended", 4, YES);
      else
        fclose (temp);
    }
  // test epilog
  if (strcmp (Epilog, "NONE") == 0)
    logtext ("No Epilog statement - None appended", 5, YES);
  else
    {
      if (strchr (Epilog, PathChar) == NULL)
        {
          memset (buffer, 0, sizeof (buffer));
          strcpy (buffer, Epilog);
          memset (Epilog, 0, sizeof (Epilog));
          sprintf (Epilog, "%s%s", Master, buffer);
        }
      temp = fopen (Epilog, "rt");
      if (temp == NULL)
        logtext ("Epilog File not found - None appended", 4, YES);
      else
        fclose (temp);
    }
  // test comments
  if (Comments[0] == 0 || strcmp (Comments, "NONE") == 0)
    logtext ("No Comments statement - Nothing done.", 5, YES);

  // test file after testing make dir (to account for slashes
  if (expSourceFile[0] != 0)
    {
      strcpy (segfile[0].FileName, expSourceFile);
      logtext ("Using Make Directive source file", 5, YES);
    }
  else
    {
      logtext ("Make Directive does not contain a source file", 5, YES);
      strcpy (segfile[0].FileName, "data.tmp");
    }

  if (MAKETYPE >= 2)
    {
      segfile[0].Net = MAKENUMBER;      // hardcode
      segfile[0].Node = 0;      // hardcode
    }
  else
    {
      segfile[0].Net = MAKENET; // hardcode
      segfile[0].Node = MAKENUMBER;     // hardcode
    }
  segfile[0].Zone = MAKEZONE;   // hardcode
  segfile[0].SegmentType = MAKETYPE;    // hardcode

  strcpy (segfile[0].AltNotify, NetAddress);

  if (PVTlvl == 0)
    {
      logtext ("Default - Private nodes are restricted to NETWORK/HUB level.",
               5, YES);
      PVTlvl = 3;
    }
  else
    {
      switch (PVTlvl)
        {
        case 0:
          PVTlvl = 3;
          logtext ("Private nodes are restricted to NETWORK/HUB level.", 5,
                   YES);
          break;
        case 1:
          logtext ("Private nodes are restricted to HUB level.", 5, YES);
          break;
        case 2:
        case 3:
          PVTlvl = 3;
          logtext ("Private nodes are restricted to NETWORK/HUB level.", 5,
                   YES);
          break;
          break;
        case 4:
          logtext
            ("Private nodes are restricted to REGION/NETWORK/HUB level.", 5,
             YES);
          break;
        case 5:
          logtext ("Private nodes are not restricted.", 5, YES);
          break;
        case 6:
        case 7:
        default:
          PVTlvl = 3;
          logtext ("Private nodes are restricted to NETWORK/HUB level.", 5,
                   YES);
          break;
        }
    }

  free (CWD);

}

void
makedirerror (void)
{
  switch (errno)
    {
    case 1:
      logtext ("Critcal error can not create requested directory", 0, YES);
      logwrite (CFE_ABORT, 0);
      logwrite (SYS_STOP, 0);
      closelog ();              // close logfile
      exit (255);
      break;
    case 5:
      break;                    //BC 4.5++ returns this if mkdir() and dir already exists
      logtext ("Memory Problem.", 5, YES);
      logwrite (CFE_ABORT, 0);
      logwrite (SYS_STOP, 0);
      closelog ();              // close logfile
      exit (255);
      break;
    case 6:
      //logtext("Directory Exists or Permission denied.",5,YES);
      break;
    case 12:
      logtext ("No Space left on DISK.", 0, YES);
      logwrite (CFE_ABORT, 0);
      logwrite (SYS_STOP, 0);
      closelog ();              // close logfile
      exit (255);
      break;
    default:
      break;
    }

}


void
printctrlinfo (void)
{
  short bcnt = 0;

  printf ("Test            : %c\n", TEST);
  printf ("Process         : %c\n", PROCESS);
  printf ("Network Name    : %s\n", NetWorkName);
  printf ("Merge Path      : %s\n", MergePath);
  printf ("Control File    : %s\n", ControlFile);
  printf ("Make            : %s [%d] [%s]\n", SegmentType[MAKETYPE].String,
          MAKENUMBER, expSourceFile);
  printf ("Process         : %s\n", Process_day);
  printf ("Publish         : %s\n", Publish_day);
  printf ("Private         : %s\n", PVTDisposition[DispPVT].String);
  printf ("MinPhone        : %d\n", MinPhone);
  printf ("MaxPhone        : %d\n", MaxPhone);
  printf ("BaudRates       : ");
  while (bauds[bcnt] != 0)
    {
      printf ("%ld,", bauds[bcnt++]);
    }
  printf ("\n");

  printf ("Master          : %s\n", Master);
  printf ("Uploads         : %s\n", Uploads);
  printf ("MailFiles       : %s\n", MailFiles);
  printf ("Update          : %s\n", Update);
  printf ("BadFiles        : %s\n", BadFiles);
  printf ("OutFile         : %s\n", OutFile);
  printf ("OutPath         : %s\n", OutPath);
  printf ("Threshold       : %ld %ld\n", threshold.arc_size,
          threshold.diff_size);
  printf ("Arc             : %d\n", ARC);
  printf ("OutDiff         : %s\n", OutDiff);
  printf ("Cleanup         : %c\n", CLEANUP);
  printf ("NetAddress      : %s\n", NetAddress);
  printf ("Messages        : %s\n", Messages);
  printf ("Submit          : %s ", SubAddress);
  if (mnotify[3].Crash == 'Y')
    printf ("Crash,");
  if (mnotify[3].Hold == 'Y')
    printf ("Hold,");
  if (mnotify[0].Intl == 'Y')
    printf ("Intl,");
  if (mnotify[3].Normal == 'Y')
    printf ("Normal,");
  printf ("\n");

  printf ("Notify          : ");
  if (mnotify[0].active == 'Y')
    {
      printf ("Receipt- ");
      if (mnotify[0].Crash == 'Y')
        printf ("Crash,");
      if (mnotify[0].Hold == 'Y')
        printf ("Hold,");
      if (mnotify[0].Intl == 'Y')
        printf ("Intl,");
      if (mnotify[0].Normal == 'Y')
        printf ("Normal,");
    }

  if (mnotify[1].active == 'Y')
    {
      printf ("Errors- ");
      if (mnotify[1].Crash == 'Y')
        printf ("Crash,");
      if (mnotify[1].Hold == 'Y')
        printf ("Hold,");
      if (mnotify[1].Intl == 'Y')
        printf ("Intl,");
      if (mnotify[1].Normal == 'Y')
        printf ("Normal,");
    }

  if (mnotify[2].active == 'Y')
    {
      printf ("Self- ");
      if (mnotify[2].Crash == 'Y')
        printf ("Crash,");
      if (mnotify[2].Hold == 'Y')
        printf ("Hold,");
      if (mnotify[2].Intl == 'Y')
        printf ("Intl,");
      if (mnotify[2].Normal == 'Y')
        printf ("Normal,");
    }
  printf ("\n");

  printf ("Copyright       : %s\n", CopyRight);
  printf ("Prolog          : %s\n", Prolog);
  printf ("Epilog          : %s\n", Epilog);
  printf ("Comments        : %s\n", Comments);
  printf ("Segment Files   : ");

  for (bcnt = 0; bcnt <= sfilecnt; bcnt++)
    {
      printf ("%s,", segfile[bcnt].FileName);
    }

  printf ("\n");

}

void
initglobals (void)
{
  memset (NetWorkName, 0, sizeof (NetWorkName));
  memset (MergePath, 0, sizeof (MergePath));
  memset (ControlFile, 0, sizeof (ControlFile));
  memset (Master, 0, sizeof (Master));
  memset (OutPath, 0, sizeof (OutPath));
  memset (Messages, 0, sizeof (Messages));
  memset (Uploads, 0, sizeof (Uploads));
  memset (MailFiles, 0, sizeof (MailFiles));
  memset (BadFiles, 0, sizeof (BadFiles));
  memset (OutFile, 0, sizeof (OutFile));
  memset (OutDiff, 0, sizeof (OutDiff));
  memset (SourceFile, 0, sizeof (SourceFile));

  memset (CopyRight, 0, sizeof (CopyRight));
  strcpy (CopyRight, "cpyright.txt");
  memset (Prolog, 0, sizeof (Prolog));
  strcpy (Prolog, "prolog.txt");
  memset (Epilog, 0, sizeof (Epilog));
  strcpy (Epilog, "epilog.txt");
  memset (Comments, 0, sizeof (Comments));

  MinPhone = 0;
  MaxPhone = 99;
  memset (BaudRates, 0, sizeof (BaudRates));
  memset (Process_day, 0, sizeof (Process_day));
  memset (Publish_day, 0, sizeof (Publish_day));
  memset (expSourceFile, 0, sizeof (expSourceFile));

//  memset(bauds,0,sizeof(bauds));
  memset (mnotify, 'N', sizeof (mnotify));
  DispPVT = 6;

  // segment file vars
  sfilecnt = 1;

  memset (segfile, 0, sizeof (segfile));

  strcpy (segfile[0].FileName, "data.tmp");

  TEST = 'N';
  PROCESS = 'N';
  DISP = 'N';
  CLEANUP = 'N';
  FORCE = 'N';
  FLAGCHK = 'N';
  FLAGCHKAUTO = 'N';
  WARNINGS = 'N';
  TESTALL = 'N';
  STATS = 'N';
  LASTDATES = 'N';
  NOTIFY = 'N';
  ISLATE = 'N';
  threshold.arc_size = -1;
  threshold.diff_size = -1;
}

short
is_proc_day (short file_day)
{
  struct dosdate_t date;

  _dos_getdate (&date);

  if (date.dayofweek == file_day)
    return (0);
  else
    return (1);

}
