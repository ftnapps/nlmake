#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef LINUX
#include <errno.h>
#include "doslinux.h"
#else
#include <dos.h>
#endif
#include "records.h"
#include "logdef.h"

extern short loglvl;
extern char *errnostr[];
FILE *LogFile;
extern LINEPRMS Months[];

// prototype
short openlog (void);
void logtext (char *string, short indicator, short dateon);

LOGMSGS LogMsgs[] = {
  {"INFO MAKE  ", 5, 2},
  {"INFO NAME  ", 5, 2},
  {"INFO PUBLISH  ", 5, 2},
  {"INFO PROCESS  ", 5, 2},
  {"INFO MERGE  ", 5, 2},
  {"INFO PRIVATE  ", 5, 2},
  {"INFO MINPHONE  ", 5, 2},
  {"INFO BAUDRATE  ", 5, 2},
  {"INFO MASTER  ", 5, 2},
  {"INFO UPLOADS  ", 5, 2},
  {"INFO MAILFILES  ", 5, 2},
  {"INFO UPDATE  ", 5, 2},
  {"INFO BADFILES  ", 5, 2},
  {"INFO OUTFILE  ", 5, 2},
  {"INFO OUTPATH  ", 5, 2},
  {"INFO THRESHOLD  ", 5, 2},
  {"INFO ARC  ", 5, 2},
  {"INFO OUTDIFF  ", 5, 2},
  {"INFO CLEANUP  ", 5, 2},
  {"INFO NETADDRESS  ", 5, 2},
  {"INFO MESSAGES  ", 5, 2},
  {"INFO SUBMIT  ", 5, 2},
  {"INFO NOTIFY  ", 5, 2},
  {"INFO COPYRIGHT  ", 5, 2},
  {"INFO PROLOG  ", 5, 2},
  {"INFO EPILOG  ", 5, 2},
  {"INFO COMMENTS  ", 5, 2},
  {"INFO DATA  ", 5, 2},
  {"INFO FILES  ", 5, 2},
  {"INFO FLAGS  ", 5, 2},
  {"INFO PVTlevel  ", 5, 2},
  {"ABORT -- Errors in configuration file", 0, 0},
  {"MAKE control statement error ", 0, 0},
  {"NAME control statement error ", 0, 0},
  {"PUBLISH control statement error ", 0, 0},
  {"PROCESS control statement error ", 0, 0},
  {"MERGE control statement error ", 0, 0},
  {"PRIVATE control statement error ", 0, 0},
  {"MINPHONE control statement error ", 0, 0},
  {"BAUDRATE control statement error ", 0, 0},
  {"MASTER control statement error ", 0, 0},
  {"UPLOADS control statement error ", 0, 0},
  {"MAILFILES control statement error ", 0, 0},
  {"UPDATE control statement error ", 0, 0},
  {"BADFILES control statement error ", 0, 0},
  {"OUTFILE control statement error ", 0, 0},
  {"OUTPATH control statement error ", 0, 0},
  {"THRESHOLD control statement error ", 0, 0},
  {"ARC control statement error ", 0, 0},
  {"OUTDIFF control statement error ", 0, 0},
  {"CLEANUP control statement error ", 0, 0},
  {"NETADDRESS control statement error ", 0, 0},
  {"NETADDRESS Invalid net address ", 0, 0},
  {"MESSAGES control statement error ", 0, 0},
  {"SUBMIT control statement error ", 0, 0},
  {"NOTIFY control statement error ", 0, 0},
  {"COPYRIGHT control statement error ", 0, 0},
  {"PROLOG control statement error ", 0, 0},
  {"EPILOG control statement error ", 0, 0},
  {"COMMENTS control statement error ", 0, 0},
  {"DATA control statement error ", 0, 0},
  {"DATA Comment Line Ignored ", 5, 2},
  {"FILES control statement error ", 0, 0},
  {"FILES error invalid segment type ", 0, 0},
  {"FILES error invalid file name ", 0, 0},
  {"Loglevel Control Line error ", 0, 0},
  {"Compress Control Line error ", 0, 0},
  {"PVTlevel Control Line error ", 0, 0},
  {"Unknown control statement ", 0, 0},
  {"SYSTEM Start-up", 0, 1},
  {"SYSTEM Finish ", 0, 1},
  {"SYSTEM Control File Processing ", 4, 1},
  {"SYSTEM Command Line ", 5, 1},
  {"ABORT -- Errors in Command line", 0, 0},
  {"MAXAGE control statement error", 0, 0},
  {"Error - Max submission FILES reached.", 0, 0},
  {NULL, 99, 0}
};

short
openlog (void)
{
  char logline[255];
  LogFile = fopen ("nlmake.log", "at+");
  if (LogFile == NULL)
    {
      if (errno <= 39)
        sprintf (logline, "Critial Error - Can not open nlmake.log <%s>",
                 errnostr[errno]);
      else
        sprintf (logline, "Critial Error - Can not open nlmake.log <???>");
      logtext (logline, 0, YES);
      return (254);
    }

  return (0);
}

void
closelog (void)
{
  fclose (LogFile);
}

void
logwrite (short message, short indicator)
{
  struct dosdate_t date;
  struct dostime_t time;
  char denote[] = { "!*+:#        " };

  if (LogMsgs[message].loglevel <= loglvl)
    {
      _dos_getdate (&date);
      _dos_gettime (&time);

      if (message == SYS_START)
        fprintf (LogFile, "\n");

      switch (LogMsgs[message].Type)
        {
        case 0:         // Control File Errors
          if (indicator >= 1)
            {
              fprintf (LogFile,
                       "%c %02d-%s-%02d %02d:%02d:%02d - %-45.45s Line #%d\n",
                       denote[LogMsgs[message].loglevel], date.day,
                       Months[date.month].Single, date.year, time.hour,
                       time.minute, time.second, LogMsgs[message].String,
                       indicator);
              printf ("%-45.45s Line #%d\n", LogMsgs[message].String,
                      indicator);
            }
          else
            {
              fprintf (LogFile,
                       "%c %02d-%s-%02d %02d:%02d:%02d - %-45.45s\n",
                       denote[LogMsgs[message].loglevel], date.day,
                       Months[date.month].Single, date.year, time.hour,
                       time.minute, time.second, LogMsgs[message].String);
              printf ("%-45.45s \n", LogMsgs[message].String);
            }
          break;
        case 1:         // System Indicators
          fprintf (LogFile,
                   "%c %02d-%s-%02d %02d:%02d:%02d - %-45.45s\n",
                   denote[LogMsgs[message].loglevel], date.day,
                   Months[date.month].Single, date.year, time.hour,
                   time.minute, time.second, LogMsgs[message].String);
          break;
        case 2:         // Control File Debug
          fprintf (LogFile, "%c %02d-%s-%02d %02d:%02d:%02d - %-18.18s",
                   denote[LogMsgs[message].loglevel],
                   date.day, Months[date.month].Single, date.year,
                   time.hour, time.minute, time.second,
                   LogMsgs[message].String);
          break;

        }

    }

}

void
logtext (char *string, short indicator, short dateon)
{
  struct dosdate_t date;
  struct dostime_t time;
  char denote[] = { "!*+:#        " };

  if (indicator <= loglvl)
    {


      _dos_getdate (&date);
      _dos_gettime (&time);


      if (dateon == YES)
        {
          fprintf (LogFile, "%c %02d-%s-%02d %02d:%02d:%02d - %s\n",
                   denote[indicator],
                   date.day, Months[date.month].Single, date.year,
                   time.hour, time.minute, time.second, string);
        }
      else
        fprintf (LogFile, "%s\n", string);

      if (indicator == 0)
        printf ("%s\n", string);

    }
}
