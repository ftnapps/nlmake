#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <errno.h>
#endif

#include "records.h"
#include "logdef.h"
//#include "textstr.inc" // Processing commands in string form


#define FTSC_MAXLEN 157         // Maximum FTSC-0005 nodelist line length

#ifdef __DOS__
#define MAXNODES 5000           // Per segment
#else
#define MAXNODES 30000          // Per segment
#endif

#ifdef __linux__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif


// extern
extern SEGFILE segfile[];
extern char OutFile[];
extern char OutDiff[];
extern char CopyRight[];
extern char Prolog[];
extern char Epilog[];
extern char Comments[];
extern short MinPhone, MaxPhone;
extern long bauds[];
extern char NetAddress[];
extern char SubAddress[];
extern char NetWorkName[];
extern short sfilecnt;
extern char Master[];
extern char Update[];
extern char BadFiles[];
extern char MailFiles[];
extern char Uploads[];
extern char MAKETYPE;
extern char CLEANUP;
extern char CLEANUPOUT;
extern char FLAGCHK;
extern char FLAGCHKAUTO;
extern char WARNINGS;
extern char PVTlvl;
extern char FORCE;
extern char STATS;
extern LINEPRMS WeekDays[];
extern LINEPRMS Months[];
extern LINEPRMS SegmentType[];
extern LINEPRMS FieldOneType[];
extern char *errnostr[];
extern char Process_day[];
extern char Publish_day[];
extern short MAKEZONE;
extern char DispPVT;
extern char NOTIFY;
extern short Dlineoff;
extern short Flineoff;
extern char expSourceFile[];
extern char ControlFile[];
extern MSG mnotify[];
extern short MAKENUMBER;
extern short MAKENET;
extern short MAXAGE;
extern char ISLATE;
extern char *extchr (char *string, char dot);


// extern functions
extern void get_proc_date (char *buf, int maxlen);
extern short FindMostCurr (char *FileName);
//extern unsigned long getcrc(char *str,unsigned long crctt);
extern unsigned short getcrc (char *filename, long offset);
extern void movefile (char *filename, char *destination);
extern void deletefile (char *filename);
extern short compress (char *filename);
extern short decompress (char *filename);
extern short create_diff (char *fdlename);
extern short FindArch (char *FileName);
extern short fcn_threshold (char *filename);
extern void clean_dir (char *filename);
extern short apply_diff (char *filename, short type);


// extern functions
extern void logwrite (short message, short indicator);
extern void logtext (char *string, short indicator, short dateon);
extern short getJDate (short type);
extern void send_netmail (char *subject, short SFI, char Type);
extern void netmail_text (char *message);
extern short filecomp (char *filename1, char *filename2);
extern short checkflags (char *allflags);
extern short open_stat_file (char *filename);
extern void stat_text (char *info);
extern void close_stat_file (void);
extern long calc_eof (char *filename);
extern void add_eof (char *filename);
extern void touch_file_date (char *filename);
extern void get_file_date (char *filename, char *date_stamp);
extern short file_age (char *filename);


// memory for flags check
extern void loadquicklst (void);
extern void freequicklst (void);

// temp debug
extern void openlog (void);
extern void closelog (void);



// prototypes
short is_node_there (short current);
short checkphone (void);
short checkbaud (void);
void logbadflags (short email, short stats);
void copy_info_files (short type);
void clear_nodes_list (void);
void kill_spaces (void);
void ExtractInfo (void);
void add_crc (unsigned short crctt, char *filename);

// Globals
// Node number database and index file
//   - Field 1   - Field 2
char *Fieldone, *Fieldtwo;
char comma[5];
//   Database field - Field 3     - Field 4     - Field 5
char *node_number, *bbs_name, *location, *sysop_name;
//   - Field 6       - Field 7      - Field  8
char *phone_number, *baud_rate, flags[255];

char zone_phone[80], region_phone[80], host_phone[80], hub_phone[80];

long nodes[MAXNODES];
short curnode = 0;
short curseg = 0;
short curhost = 0;
short curhostype = 0;

char *str;
FILE *segmentfile, *outfilep, *comntsp;
long eof = 0;
short proc_type;
short headeroffset = 0;
char *badflags;
char *caseflags;
char *goodflags;
char *dupeflags;
char *rplcdflags;
char *redunflags;

void
init_flags_lists (void)
{
  badflags = (char *) malloc (256);
  caseflags = (char *) malloc (256);
  goodflags = (char *) malloc (256);
  dupeflags = (char *) malloc (256);
  rplcdflags = (char *) malloc (512);
  redunflags = (char *) malloc (256);
}

void
free_flags_lists (void)
{
  free (badflags);
  free (caseflags);
  free (goodflags);
  free (dupeflags);
  free (rplcdflags);
  free (redunflags);
}


/*
This is the function where we compile all segments into an "outfile"
*/
short
process_segment (void)
{
  short bcnt;
  char date[50];
  char logline[255];
  short errorlvl = 0;
  short linecnt;
  char errorcomnt[255];
  short errcnt;
  short lineistype;
  char namebuf[255];
  char *ptr;
  short comptype;
  short phonecnt;
  short flgerr = 0;


  outfilep = fopen (OutFile, "wb+");

  if (Comments[0] != 0)
    comntsp = fopen (Comments, "wt+");

  if (outfilep == NULL)
    {
      if (errno <= 39)
        sprintf (logline, "Critial Error - Can not open OUTFILE <%s>",
                 errnostr[errno]);
      else
        sprintf (logline, "Critial Error - Can not open OUTFILE <?>");
      logtext (logline, 0, YES);
      errorlvl = 254;
      return (errorlvl);
    }



  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
    {
      loadquicklst ();
      init_flags_lists ();
    }

  str = (char *) malloc (MAXSTR);

  memset (str, 0, sizeof (str));

  // set CRC line

  get_proc_date (date, sizeof(date));

  //memset(str,0,254);
  str[0] = 0;
  sprintf (str, ";A %s Nodelist for %s : 00000\r\n", NetWorkName, date);


  headeroffset = strlen (str) - 1;
  fprintf (outfilep, str);

  // copyright
  copy_info_files (0);
  // prolog
  copy_info_files (1);

  for (bcnt = 0; bcnt <= sfilecnt; bcnt++)
    {                           // end known
      //memset(namebuf,0,254);
      namebuf[0] = 0;

      if (bcnt != 0 || expSourceFile[0] != 0)
        {                       //
          sprintf (namebuf, "%s%s", Master, segfile[bcnt].FileName);
          if (strchr (segfile[bcnt].FileName, '.') == NULL)
            FindMostCurr (namebuf);
        }                       //
      else
        sprintf (namebuf, "%s", segfile[bcnt].FileName);

      if (bcnt != 0)
        {                       //
          if (segfile[bcnt].SegmentType <= 1)
            {
              printf ("Processing %-8.8s %-5d -- file %s\n",
                      SegmentType[segfile[bcnt].SegmentType].String,
                      segfile[bcnt].Node, namebuf);
              sprintf (logline, "Processing %-8.8s %-5d -- file %s",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Node, namebuf);
            }
          else
            {
              printf ("Processing %-8.8s %-5d -- file %s\n",
                      SegmentType[segfile[bcnt].SegmentType].String,
                      segfile[bcnt].Net, namebuf);
              sprintf (logline, "Processing %-8.8s %-5d -- file %s",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Net, namebuf);
            }
          logtext (logline, 1, YES);
        }
      else                      //
        {                       //
          printf ("Make       %-8.8s %-5d \n",
                  SegmentType[segfile[bcnt].SegmentType].String, MAKENUMBER);
          sprintf (logline, "Make %-8.8s %-5d",
                   SegmentType[segfile[bcnt].SegmentType].String, MAKENUMBER);
          logtext (logline, 1, YES);
        }                       //
      if (comntsp != NULL && bcnt != 0)
        {
          if (segfile[bcnt].SegmentType <= 1)
            sprintf (logline, "\nComments from %-8.8s %-5d -- file %s\n",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Node, namebuf);
          else
            sprintf (logline, "\nComments from  %-8.8s %-5d -- file %s\n",
                     SegmentType[segfile[bcnt].SegmentType].String,
                     segfile[bcnt].Net, namebuf);
          fprintf (comntsp, "%s\n", logline);
        }
      eof = calc_eof (namebuf) - 1;


      /* Open a segment */
      segmentfile = fopen (namebuf, "rb");

      if (segmentfile != NULL)
        {                       // end found
          //fseek (segmentfile, 0L, SEEK_END);
          //eof = ftell (segmentfile);
          //fseek (segmentfile, 0L, SEEK_SET);
          linecnt = 0;
          proc_type = 0;        // reset at start of every file
          clear_nodes_list ();

          while (1)
            {                   // end found
              if (eof <= ftell (segmentfile))
                break;
              //memset (str, 0, 254);
              str[0] = 0;

              /* Read a line from the segment */
              fgets (str, MAXSTR, segmentfile);
              linecnt++;

              /* If the line is a comment, save it to the comments file */
              if (str[0] == ';' && strlen (str) >= 3)
		{
		  if (comntsp != NULL)
		    fprintf (comntsp, "%s", str);

		  /* Pass through error lines */
		  if (str[1] == 'E')
		    fprintf (outfilep, "%s", str);
		}

              /* Is this line a good data line...? */
              if (str[0] != 0 && str[0] != 26 && str[0] != '\r' && str[0] != '\n' && str[0] != ';')
                {
                  kill_spaces ();
                  ExtractInfo ();
                  errorcomnt[0] = 0;
                  errcnt = 0;
                  //check for line too long
                  if (strlen (str) > 2 + FTSC_MAXLEN)   //must be <=157 chrs, not counting cr/lf
                    {
                      errcnt++;
                      strcat (errorcomnt, "<Line too long>");
                    }
                  //    printf("curnode %d\n",curnode);
                  if (is_node_there (curnode) == 1)
                    {
                      errcnt++;
                      strcat (errorcomnt, "<Duplicate NodeNumber> ");
                    }

                  if (segfile[bcnt].SegmentType != proc_type)
                    {

                      logtext ("Error Segment file not correct type", 0, YES);
                      sprintf (logline, "%s is configured as %s but is %s",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               SegmentType[proc_type].String);
                      logtext (logline, 0, YES);
                      errorlvl = 2;     // abort fatal
                      break;
                    }
                  //memset(errorcomnt,NULL,254);

                  if (segfile[bcnt].Net != curseg
                      && segfile[bcnt].SegmentType >= 2)
                    {
                      logtext ("Error Segment file not correct number", 0,
                               YES);
                      sprintf (logline,
                               "%s is configured as %s %d but is %s %d",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               segfile[bcnt].Net,
                               SegmentType[proc_type].String, curseg);
                      logtext (logline, 0, YES);
                      errorlvl = 2;     // abort fatal
                      //bcnt = sfilecnt + 1;
                      break;
                    }
                  else
                    if (segfile[bcnt].Node != curseg
                        && segfile[bcnt].SegmentType <= 1)
                    {
                      logtext ("Error Segment file not correct number", 0,
                               YES);
                      sprintf (logline,
                               "%s is configured as %s %d but is %s %d",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               segfile[bcnt].Node,
                               SegmentType[proc_type].String, curseg);
                      logtext (logline, 0, YES);
                      errorlvl = 2;     // abort fatal
                      //bcnt = sfilecnt + 1;
                      break;
                    }

                  // test field 1
                  lineistype = 0;
                  if (*Fieldone != 0)
                    while (FieldOneType[lineistype].String != NULL)
                      {
                        if (strnicmp
                            (Fieldone, FieldOneType[lineistype].Single,
                             strlen (FieldOneType[lineistype].Single)) == 0)
                          {
                            break;
                          }
                        lineistype++;
                      }

                  if (*Fieldone != 0)
                    if (FieldOneType[lineistype].String == NULL)
                      {
                        errcnt++;
                        strcat (errorcomnt, "<Unknown Type> ");
                      }

                  if (lineistype == 1)
                    {           //
                      switch (DispPVT)
                        {
                        case 0:
                          errcnt++;
                          strcat (errorcomnt,
                                  "<PVT not allowed at any time> ");
                          break;
                        case 1:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                              break;
                            }
                          if (hub_phone[0] != 0)
                            {
                              phone_number = hub_phone;
                              Fieldone = comma;
                            }
                          break;
                        case 2:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                              break;
                            }
                          if (host_phone[0] != 0)
                            {
                              phone_number = host_phone;
                              Fieldone = comma;
                            }
                          break;
                        case 3:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                              break;
                            }
                          if (host_phone[0] != 0)
                            {
                              phone_number = host_phone;
                              Fieldone = comma;
                            }
                          break;
                        case 4:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                              break;
                            }
                          if (region_phone[0] != 0)
                            {
                              phone_number = region_phone;
                              Fieldone = comma;
                            }
                          break;
                        case 5:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                              break;
                            }
                          if (zone_phone[0] != 0)
                            {
                              phone_number = zone_phone;
                              Fieldone = comma;
                            }
                          break;
                        case 6:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                            }
                          break;
                        }
                    }

                  if (MinPhone != 0)
                    {           //
                      phonecnt = checkphone ();
                      if (phonecnt != MinPhone)
                        {
                          if (phonecnt < MinPhone)
                            {
                              if (WARNINGS == 'N')
                                errcnt++;
                              strcat (errorcomnt,
                                      "<Phone Number too few parts> ");
                            }
                          else if (phonecnt > MaxPhone)
                            {
                              if (WARNINGS == 'N')
                                errcnt++;
                              strcat (errorcomnt,
                                      "<Phone Number too many parts> ");
                            }
                        }
                    }           //

                  if (checkbaud () != 0)
                    {           //
                      if (WARNINGS == 'N')
                        errcnt++;
                      strcat (errorcomnt, "<Invalid Baud Rate> ");
                    }           //


                  // check flags
                  // check Uflags
                  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
                    if ((flgerr = checkflags (flags)) == 1)
                      {
                        strcat (errorcomnt, "<Flags Error> ");
                      }

                  if (errcnt == 0)
                    {
                      if (errorcomnt[0] != 0)
                        {
                          if (bcnt == 0)
                            {
                              if (expSourceFile[0] == 0)
                                sprintf (logline,
                                         "Warning Segment DATA in File : %-13.13s Line :%d has errors",
                                         ControlFile, (linecnt + Dlineoff));
                              else
                                sprintf (logline,
                                         "Warning Segment File : %-13.13s Line :%d has errors",
                                         segfile[bcnt].FileName, linecnt);
                            }
                          else
                            sprintf (logline,
                                     "Warning Segment File : %-13.13s Line :%d has errors",
                                     segfile[bcnt].FileName, linecnt);

                          logtext (logline, 1, YES);
                          logtext (errorcomnt, 2, YES);
                          if (flgerr == 1)
                            logbadflags (NO, NO);
                          {
                          }

                        }
                      //fprintf (outfilep, "%s", str);
                      if (FLAGCHKAUTO != 'Y')
                        fprintf (outfilep, "%s,%s,%s,%s,%s,%s,%s%s\r\n",
                                 Fieldone, Fieldtwo, bbs_name,
                                 location, sysop_name,
                                 phone_number, baud_rate, flags);
                      else
                        fprintf (outfilep, "%s,%s,%s,%s,%s,%s,%s%s\r\n",
                                 Fieldone, Fieldtwo, bbs_name,
                                 location, sysop_name,
                                 phone_number, baud_rate, goodflags);
                    }
                  else
                    {
                      fprintf (outfilep, ";E %s\r\n", errorcomnt);
                      fprintf (outfilep, ";E %s,%s,%s,%s,%s,%s,%s%s\r\n",
                               Fieldone, Fieldtwo, bbs_name,
                               location, sysop_name,
                               phone_number, baud_rate, flags);
                      if (errorlvl <= 1)
                        errorlvl = 1;   // abort here not fatal
                      if (bcnt == 0)
                        {
                          if (expSourceFile[0] == 0)
                            sprintf (logline,
                                     "Error   Segment DATA in File : %-13.13s Line :%d commented out",
                                     ControlFile, (linecnt + Dlineoff));
                          else
                            sprintf (logline,
                                     "Error   Segment File : %-13.13s Line :%d commented out",
                                     expSourceFile, linecnt);
                        }
                      else
                        sprintf (logline,
                                 "Error   Segment File : %-13.13s Line :%d commented out",
                                 segfile[bcnt].FileName, linecnt);
                      logtext (logline, 1, YES);
                      logtext (errorcomnt, 2, YES);
                      if (flgerr == 1)
                        logbadflags (NO, NO);
                    }

                }               // End if comment
            }                   // While(1)

          fclose (segmentfile);

        }
      else                      // null file (not found)
        {                       //
          logtext ("Error Segment file not found", 0, YES);
          logtext (namebuf, 0, NO);
          errorlvl = 2;         // abort here fatal or not?
          // bcnt = sfilecnt + 1;
        }                       //

    }                           // For loop

  //printf("end input\n");


  // epilog
  if (errorlvl <= 1)
    copy_info_files (2);
  // comments
//       if(errorlvl <= 1)
  //  copy_info_files(3);

  fclose (outfilep);
  if (comntsp)
    fclose (comntsp);


  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
    {
      freequicklst ();
      free_flags_lists ();
    }

  free (str);

  // crc insert routine
  add_crc (getcrc (OutFile, headeroffset), OutFile);
  add_eof (OutFile);

  // compress with threshold
  comptype = fcn_threshold (OutFile);
  //printf("Threshold %d\n",comptype);
  strcpy (namebuf, OutFile);
  if (errorlvl <= 1)
    switch (comptype)
      {
      case 0:                   // Uncompressed Segment
        if (SubAddress[0] != 0)
          {
            if (filecomp (OutFile, "outfile.tmp") == 1 || FORCE == 'Y')
              {
                send_netmail (namebuf, bcnt, 3);
                logtext ("Segment file sent", 0, YES);
              }
            else
              logtext ("No Changes Segment file not sent", 0, YES);
          }
        break;
      case 1:                   // Compressed Segment
        if (compress (namebuf) != 0)
          {
            if (errorlvl <= 1)
              errorlvl = 2;     // errors
            logtext ("Compressor failure - Check compress.ctl", 0, YES);
          }
        else
          {
            if (SubAddress[0] != 0)
              {
                if (filecomp (OutFile, "outfile.tmp") == 1 || FORCE == 'Y')
                  {
                    send_netmail (namebuf, bcnt, 3);
                    logtext ("Segment file sent", 0, YES);
                  }
                else
                  logtext ("No Changes Segment file not sent", 0, YES);
              }
          }
        break;
      case 2:                   // Uncompressed Diff
        ptr = strchr (namebuf, '.');    // Apend .D?? extension
        ptr++;
        *ptr = 'D';
        if (create_diff (namebuf) == 1)
          {
            if (errorlvl <= 1)
              errorlvl = 2;     // errors
            break;
          }
        else if (SubAddress[0] != 0)
          {
            if (filecomp (OutFile, "outfile.tmp") == 1 || FORCE == 'Y')
              {
                send_netmail (namebuf, bcnt, 3);
                logtext ("Segment Diff file sent", 0, YES);
              }
            else
              logtext ("No Changes Segment Diff not sent", 0, YES);
          }
        break;
      case 3:                   // Compressed Diff
        ptr = strchr (namebuf, '.');    // Apend .D?? extension
        ptr++;
        *ptr = 'D';
        if (create_diff (namebuf) == 1)
          {
            if (errorlvl <= 1)
              errorlvl = 2;     // errors
            break;
          }
        else if (compress (namebuf) != 0)
          {
            if (errorlvl <= 1)
              errorlvl = 2;     // abort fatal
            logtext ("Compressor failure - Check compress.ctl", 0, YES);
          }
        else if (SubAddress[0] != 0)
          {
            if (filecomp (OutFile, "outfile.tmp") == 1 || FORCE == 'Y')
              {
                send_netmail (namebuf, bcnt, 3);
                logtext ("Segment Diff file sent", 0, YES);
              }
            else
              logtext ("No Changes Segment Diff not sent", 0, YES);
          }
        break;
      default:
        if (SubAddress[0] != 0)
          send_netmail (OutFile, bcnt, 3);
        break;
      }

  // Forced diff
  if (OutDiff[0] != 0)
    {
      comptype = fcn_threshold (OutDiff);
      if (errorlvl <= 1)
        if (create_diff (OutDiff) == 1)
          {
            if (errorlvl <= 1)
              errorlvl = 2;     // abort fatal
          }

      // test if we compress outdiff
      comptype = fcn_threshold (OutDiff);
      switch (comptype)
        {
        case 0:         // Uncompressed Segment
        case 2:         // Uncompressed Diff
          break;
        case 1:         // Compressed Output
        case 3:         // Compressed Diff
          if (compress (OutDiff) != 0)
            {
              if (errorlvl <= 1)
                errorlvl = 2;   // abort fatal
              logtext ("Compressor failure - Check compress.ctl", 0, YES);
            }
          break;
        default:
          break;
        }
    }

  // force cleanup
  if (CLEANUPOUT == 'Y')
    clean_dir (OutFile);

  return (errorlvl);

}

void
kill_spaces (void)
{
  char *chptr;
  char cset[] =
    {
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()\"\\\'?><,.+=-_`~|/ :;{}[]\n"
};
  unsigned int space;

  chptr = strchr (str, ' ');
  while (chptr != NULL)
    {
      if (chptr == NULL)
        break;
      if (*(chptr - 1) == ',' || *(chptr + 1) == ',' || *(chptr + 1) == ' ')
        memmove (chptr, chptr + 1, (strlen (chptr + 1) + 1));
      else
        *chptr = '_';
      chptr = strchr (str, ' ');
    }

// printf("spn %d strlen %d\n",strspn(str,cset),strlen(str));

  /*
  if (strspn (str, cset) != strlen (str))
    for (space = 0; space < strlen (str); space++)
      {
        // if (str[space] & 0x80)
        //   str[space] = '?';
        // if (str[space] < 0x20)
	//   str[space] = '?';
      }
  */
}

void
clear_nodes_list (void)
{
  memset (nodes, -1, sizeof (nodes));
  //printf ("clear_nodes_list\n");
}

short
is_node_there (short current)
{
  short i, host = 0;
  short retvalue = 0;
  long local;
  //char magic[40];
  //char zone[5];

  if (nodes[0] == -1)           // set file type
    {
      if (str[0] != ',')
        while (SegmentType[host].String != NULL)
          {
            if (strnicmp
                (str, SegmentType[host].String,
                 strlen (SegmentType[host].String)) == 0)
              {
                proc_type = host;
                break;
              }
            else
              host++;
          }
      if (proc_type == 2)
        proc_type = 3;
      curseg = atol (Fieldtwo);
    }
  //printf("current %d\n",current);
  //printf("%d\n",current);

  local = current + (10000 * curhost) + (100000000 * MAKEZONE);
  //printf("output %d\n",local);

  //printf(magic,"%d%d%d",MAKEZONE,curhost,current);
  //sprintf(magic,"%d%04d%04d",MAKEZONE,curhost,current);
  //printf("magic %s\n",magic);
  //local = atol(magic);
  //printf("local %uld\n",local);

  for (i = 0; i < MAXNODES; i++)
    {
      if (local == nodes[i])
        {
          retvalue = 1;         // Found
          break;
        }
      if (nodes[i] == -1)       // Not found
        {
          retvalue = 0;         // not Found
          nodes[i] = local;     // add
          break;
        }
    }

  return (retvalue);

}


void
ExtractInfo (void)
{
  short host = 0;
  char *p;
  char *delims = { ",\n\r" };

  if (str[0] != ',')
    while (SegmentType[host].String != NULL)
      {
        if (strnicmp
            (str, SegmentType[host].String,
             strlen (SegmentType[host].String)) == 0)
          {
            curhostype = host;
            break;
          }
        else
          host++;
      }

  if (str[0] != ',')
    {
      Fieldone = strtok (str, delims);  // 1
      //printf("\n%s\n",Fieldone);
      Fieldtwo = strtok (NULL, delims); //2
      //printf("%s\n",Fieldtwo);
    }
  else
    {
      comma[0] = 0;
      Fieldone = comma;
      Fieldtwo = strtok (str, delims);  //2
      //printf("%s\n",Fieldtwo);
    }
  //bbs name
  bbs_name = strtok (NULL, delims);     //3
  //printf("%s\n",bbs_name);
  //
  // location
  location = strtok (NULL, delims);     //4
  //printf("%s\n",location);

  // Get Sysop Name
  sysop_name = strtok (NULL, delims);   //5
  //printf("%s\n",sysop_name);

  // Get Phone Number
  phone_number = strtok (NULL, delims); //6
  //printf("%s\n",phone_number);
  if (Fieldtwo)
    curnode = atol (Fieldtwo);
  else
    return;
  // printf("curnode %d\n",curnode);
  // set parameters based on what the segment type is???
  switch (host)
    {
    case 0:                     // node
      if (proc_type == 0 && outfilep != NULL)
        {
          fprintf (outfilep, ";\r\n");
        }
      if (curhost == 0)
        curhost = MAKENET;
      break;
    case 1:                     // hub
      if (DispPVT == 1)
        strcpy (hub_phone, phone_number);
      if (proc_type == 1 && outfilep != NULL)
        {
          fprintf (outfilep, ";\r\n");
        }
      if (curhost == 0)
        curhost = MAKENET;
      break;
    case 2:                     // host
    case 3:                     // net
      if (outfilep != NULL)
        {
          fprintf (outfilep, ";\r\n");
        }
      if (DispPVT == 3)
        strcpy (host_phone, phone_number);
      curnode = 0;
      curhost = atoi (Fieldtwo);
      curhostype = host;
      break;
    case 4:                     //region
      if (outfilep != NULL)
        {
          fprintf (outfilep, ";\r\n");
        }
      if (DispPVT == 4)
        strcpy (region_phone, phone_number);
      curnode = 0;
      curhost = atoi (Fieldtwo);
      curhostype = host;
      break;
    case 5:                     // zone
      if (outfilep != NULL)
        {
          fprintf (outfilep, ";\r\n");
        }
      if (DispPVT == 5)
        strcpy (zone_phone, phone_number);
      curnode = 0;
      curhost = atoi (Fieldtwo);
      curhostype = host;
      break;
    case 6:                     //comp
      break;
    default:
      break;
    }
  // get baud
  baud_rate = strtok (NULL, delims);    // 7
  //printf("%s\n",baud_rate);

  memset (flags, 0, sizeof (flags));

  p = strtok (NULL, delims);

  while (p != NULL)
    {
      if (p != NULL)
        {
          strncat (flags, ",", 1);
          strncat (flags, p, strlen (p));
        }
      p = strtok (NULL, delims);
    }
  //    printf("%s\n",flags);

}

// check sub-routines

short
checkphone (void)
{
  short partscnt = 0;
  short i = 0;

  if (!phone_number)
    return 0;
  while (phone_number[i++] != 0)
    {
      if (phone_number[i] == '-')
        partscnt++;
    }

  if (*Fieldone != 0)
    {
      if (strnicmp (Fieldone, "PVT", 3) == 0
          && stricmp (phone_number, "-Unpublished-") == 0)
        return (MinPhone);
      if (strnicmp (Fieldone, "DOWN", 4) == 0
          && stricmp (phone_number, "-Unpublished-") == 0)
        return (MinPhone);
      if (strnicmp (Fieldone, "HOLD", 4) == 0
          && stricmp (phone_number, "-Unpublished-") == 0)
        return (MinPhone);
    }

  return (partscnt + 1);

}

short
checkbaud (void)
{

  long bps;
  short cntr = 0;

  bps = atol (baud_rate);

  while (bauds[cntr] != 0)
    {
      if (bauds[cntr] == bps)
        return (0);
      if (bauds[cntr] == 0)
        return (1);
      cntr++;
    }

  return (1);
}


void
copy_info_files (short type)
{
  FILE *info;
  char *ins_date;
  char asc_year[10];
  char linetype[] = { "ASAS        " };
  time_t utime;
  struct tm *tm;

  time (&utime);
  tm = localtime (&utime);
  strftime(asc_year, 10, "%Y", tm);

  switch (type)
    {
    case 0:
      eof = calc_eof (CopyRight);
      info = fopen (CopyRight, "rt");
      break;
    case 1:
      eof = calc_eof (Prolog);
      info = fopen (Prolog, "rt");
      break;
    case 2:
      eof = calc_eof (Epilog);
      info = fopen (Epilog, "rt");
      break;
    case 3:
      eof = calc_eof (Comments);
      info = fopen (Comments, "rt");
      break;
    default:
      return;

      // break;
    }

  if (info != NULL)
    {

      //fseek (info, 0L, SEEK_END);
      //eof = ftell (info);
      //    fseek (info, 0L, SEEK_SET);
      while (1)
        {
          //memset (str, 0, 254);
          str[0] = 0;
          if (fgets (str, MAXSTR, info) == NULL)
            break;

          ins_date = strstr (str, "####");
          if (ins_date != NULL)
            {
              strncpy (ins_date, asc_year, 4);
            }
          if (str[0] == ';')
            {
              fprintf (outfilep, "%s", str);
            }
          else
            {
              fprintf (outfilep, ";%c %s", linetype[type], str);        //HERE
            }

          // log
          if (eof == ftell (info))
            break;
        }
    }
}

void
add_crc (unsigned short crctt, char *filename)
{
  FILE *fp;
  char localstr[256];

  fp = fopen (filename, "rb+");
  fgets (localstr, 255, fp);
  fseek (fp, (long) (strlen (localstr) - 7), SEEK_SET);
  fprintf (fp, "%05hu", (unsigned short) crctt);
  //printf("CRC : %0.5u\n",crctt);
  fclose (fp);
}

short
test_crc (char *filename, char *logline)
{
  FILE *fp;
  unsigned short listedcrc, actualcrc;
  char crc[10];
  char localstr[256];

  memset (crc, 0, sizeof (crc));

  fp = fopen (filename, "rb+");

  if (fp == NULL)
    return (2);

  fgets (localstr, 255, fp);

  //fseek (fp,0L, SEEK_SET);

  fseek (fp, (long) (strlen (localstr) - 7), SEEK_SET);
  fread (crc, sizeof (char), 5, fp);


  listedcrc = atoi (crc);
  fclose (fp);
  fp = NULL;

  actualcrc = getcrc (filename, strlen (localstr) - 1);

  //printf("CRC L:%0.5hu A:%0.5hu\n",listedcrc,actualcrc);

  if (listedcrc != 0)
    {
      if (listedcrc != actualcrc)
        {
          sprintf (logline, "CRC Fail File: %05hu Actual: %05hu", listedcrc,
                   actualcrc);
//              printf("CRC Error!\n");
          if (fp)
            fclose (fp);
          return (1);
        }
    }
  else
    {
      sprintf (logline, "CRC Disabled!");
      if (fp)
        fclose (fp);
      return (0);
      //     printf("CRC Disabled!\n");
    }

  sprintf (logline, "CRC Check File: %05hu Actual: %05hu", listedcrc,
           actualcrc);
  if (fp)
    fclose (fp);
  return (0);
}

// Test segment files
short
test_segment (void)
{
  short bcnt;
  char logline[255];
  short errorlvl = 0;
  short linecnt;
  char errorcomnt[255];
  char date_stamp[100];
  short totalerrcnt;
  short errcnt;
  short lineistype;
  char namebuf[255];
  short phonecnt;
  short flgerr = 0;
  char *dot;


  str = (char *) malloc (MAXSTR);

  memset (str, 0, sizeof (str));

  if (STATS == 'Y')
    {
      strcpy (namebuf, ControlFile);
      dot = extchr (namebuf, '.');
      strcpy (dot, ".err");
      if (open_stat_file (namebuf) == 1)
        STATS = 'N';
    }

  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
    {
      loadquicklst ();
      init_flags_lists ();
    }

  // For each segment...?
  for (bcnt = 0; bcnt <= sfilecnt; bcnt++)
    {
      //memset(namebuf,0,254);
      namebuf[0] = 0;

      if (bcnt != 0 || expSourceFile[0] != 0)
        {
          // Try getting segment from update directory
          sprintf (namebuf, "%s%s", Update, segfile[bcnt].FileName);
          if (strchr (segfile[bcnt].FileName, '.') == NULL)
            FindMostCurr (namebuf);

          segmentfile = fopen (namebuf, "rt");
          if (segmentfile == NULL)
            {
              // Use segment from master directory
              //fclose(segmentfile); - no, it's null!
              sprintf (namebuf, "%s%s", Master, segfile[bcnt].FileName);
              if (strchr (segfile[bcnt].FileName, '.') == NULL)
                FindMostCurr (namebuf);
            }
        }
      else
        sprintf (namebuf, "%s", segfile[bcnt].FileName);

      if (bcnt != 0)
        {
          if (segfile[bcnt].SegmentType <= 1)
            {
              printf ("Testing    %-8.8s %-5d -- file %s\n",
                      SegmentType[segfile[bcnt].SegmentType].String,
                      segfile[bcnt].Node, namebuf);
              sprintf (logline, "Testing    %-8.8s %-5d -- file %s",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Node, namebuf);
            }
          else
            {
              printf ("Testing    %-8.8s %-5d -- file %s\n",
                      SegmentType[segfile[bcnt].SegmentType].String,
                      segfile[bcnt].Net, namebuf);
              sprintf (logline, "Testing    %-8.8s %-5d -- file %s",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Net, namebuf);
            }
          logtext (logline, 1, YES);
        }
      else
        {
          printf ("Testing    %-8.8s %-5d \n",
                  SegmentType[segfile[bcnt].SegmentType].String, MAKENUMBER);
          sprintf (logline, "Testing    %-8.8s %-5d ",
                   SegmentType[segfile[bcnt].SegmentType].String, MAKENUMBER);
          logtext (logline, 1, YES);
        }


      if (STATS == 'Y' || NOTIFY == 'Y')
        {
//        if (segmentfile) fclose(segmentfile);  //This was causing a segfault under Linux.  Needs testing and return if required.
          get_file_date (namebuf, date_stamp);
          sprintf (logline, "Processing %-8.8s %-5d last received on %s",
                   SegmentType[segfile[bcnt].SegmentType].String,
                   segfile[bcnt].Net, date_stamp);
          if (STATS == 'Y')
            stat_text (logline);
          if (NOTIFY == 'Y')
            netmail_text (logline);
          segmentfile = fopen (namebuf, "rt");
        }

//        if (segmentfile) fclose(segmentfile);  //This was causing a segfault under Linux.  Needs testing and return if required.
      eof = calc_eof (namebuf);
      segmentfile = fopen (namebuf, "rb");

      if (segmentfile != NULL)
        {
          //fseek (segmentfile, 0L, SEEK_END);
          //eof = ftell (segmentfile);
          //fseek (segmentfile, 0L, SEEK_SET);
          linecnt = 0;
          proc_type = 0;        // reset at start of every file
          clear_nodes_list ();
          deletefile ("msgtemp.txt");
          totalerrcnt = 0;
          while (1)
            {                   // end found
              if (eof == ftell (segmentfile))
                {
                  if (bcnt == 0)
                    {
                      if (mnotify[2].active == 'Y')
                        {
                          if (totalerrcnt == 0)
                            netmail_text ("\rNo Errors Detected");
                          send_netmail ("Local Data Segment", bcnt, 2);
                        }
                      deletefile ("msgtemp.txt");
                    }
                  if (STATS == 'Y')
                    {
                      if (totalerrcnt == 0)
                        stat_text ("\nNo Errors Detected");
                      stat_text ("\n--End--\n");
                    }
                  if (bcnt != 0 && NOTIFY == 'Y')
                    {
                      if (totalerrcnt == 0)
                        {
                          netmail_text ("\rNo Errors Detected");
                          netmail_text ("\r--End--\r");
                          send_netmail ("Notification", bcnt, 0);
                        }
                      else
                        {
                          netmail_text ("\rErrors Detected");
                          netmail_text ("\r--End--\r");
                          send_netmail ("Error Notification", bcnt, 1);
                        }
                    }
                  deletefile ("msgtemp.txt");
                  break;
                }
              //memset (str, 0, 254);
              str[0] = 0;
              fgets (str, MAXSTR, segmentfile);
              //printf ("%s", str);
              linecnt++;
              if (str[0] != ';' && str[0] != 0 && str[0] != 26)
                {
                  ExtractInfo ();
                  errorcomnt[0] = 0;
                  errcnt = 0;

                  if (is_node_there (curnode) == 1)
                    {
                      errcnt++;
                      strcat (errorcomnt, "<Duplicate NodeNumber> ");
                    }
                  if (segfile[bcnt].SegmentType != proc_type)
                    {

                      logtext ("Error Segment file not correct type", 0, YES);
                      sprintf (logline, "%s is configured as %s but is %s",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               SegmentType[proc_type].String);
                      logtext (logline, 0, YES);
                      errorlvl = 2;     // abort fatal
                      fclose (segmentfile);
                      break;
                    }
                  //memset(errorcomnt,0,254);

                  if (segfile[bcnt].Net != curseg
                      && segfile[bcnt].SegmentType >= 2)
                    {
                      logtext ("Error Segment file not correct number", 0,
                               YES);
                      sprintf (logline,
                               "%s is configured as %s %d but is %s %d",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               segfile[bcnt].Net,
                               SegmentType[proc_type].String, curseg);
                      logtext (logline, 0, YES);
                      errorlvl = 2;     // abort fatal
                      break;
                    }
                  else
                    if (segfile[bcnt].Node != curseg
                        && segfile[bcnt].SegmentType <= 1)
                    {
                      logtext ("Error Segment file not correct number", 0,
                               YES);
                      sprintf (logline,
                               "%s is configured as %s %d but is %s %d",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               segfile[bcnt].Node,
                               SegmentType[proc_type].String, curseg);
                      logtext (logline, 0, YES);
                      errorlvl = 2;     // abort fatal
                      break;
                    }
                  else
                    // test field 1
                    lineistype = 0;
                  if (*Fieldone != 0)
                    while (FieldOneType[lineistype].String != NULL)
                      {
                        if (strnicmp
                            (Fieldone, FieldOneType[lineistype].Single,
                             strlen (FieldOneType[lineistype].Single)) == 0)
                          {
                            break;
                          }
                        lineistype++;
                      }

                  if (*Fieldone != 0)
                    if (FieldOneType[lineistype].String == NULL)
                      {
                        errcnt++;
                        strcat (errorcomnt, "<Unknown Type> ");
                      }

                  if (lineistype == 1)
                    {           //
                      switch (DispPVT)
                        {
                        case 0:
                          errcnt++;
                          strcat (errorcomnt,
                                  "<PVT not allowed at any time> ");
                          break;
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                            }
                          break;
                        }
                    }

                  if (MinPhone != 0)
                    {           //
                      phonecnt = checkphone ();
                      if (phonecnt != MinPhone)
                        {
                          if (phonecnt < MinPhone)
                            {
                              if (WARNINGS == 'N')
                                errcnt++;
                              strcat (errorcomnt,
                                      "<Phone Number too few parts> ");
                            }
                          else if (phonecnt > MaxPhone)
                            {
                              if (WARNINGS == 'N')
                                errcnt++;
                              strcat (errorcomnt,
                                      "<Phone Number too many parts> ");
                            }
                        }
                    }           //

                  if (checkbaud () != 0)
                    {           //
                      if (WARNINGS == 'N')
                        errcnt++;
                      strcat (errorcomnt, "<Invalid Baud Rate> ");
                    }           //


                  // check flags
                  // check Uflags
                  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
                    if ((flgerr = checkflags (flags)) == 1)
                      {
                        strcat (errorcomnt, "<Flags Error> ");
                        totalerrcnt++;
                      }

                  totalerrcnt += errcnt;        // calc all errors

                  if (errcnt == 0)
                    {
                      if (errorcomnt[0] != 0)
                        {
                          if (bcnt == 0)
                            {
                              if (expSourceFile[0] == 0)
                                sprintf (logline,
                                         "Warning Segment DATA in File : %-13.13s Line :%d has errors",
                                         ControlFile, (linecnt + Dlineoff));
                              else
                                sprintf (logline,
                                         "Warning Segment File : %-13.13s Line :%d has errors",
                                         segfile[bcnt].FileName, linecnt);
                              if (NOTIFY != 'Y')
                                netmail_text (logline);
                              logtext (logline, 0, YES);
                              sprintf (logline, "%s,%s,%s,%s,%s,%s,%s%s",
                                       Fieldone, Fieldtwo, bbs_name,
                                       location, sysop_name,
                                       phone_number, baud_rate, flags);
                              if (NOTIFY != 'Y')
                                netmail_text (logline);
                              if (NOTIFY != 'Y')
                                netmail_text (errorcomnt);
                            }
                          else
                            {
                              sprintf (logline,
                                       "Warning Segment File : %-13.13s Line :%d has errors",
                                       segfile[bcnt].FileName, linecnt);
                              logtext (logline, 0, YES);
                            }
                          logtext (errorcomnt, 2, YES);
                          if (STATS == 'Y')
                            {
                              sprintf (logline, "\n-- %d:%d/%d --", MAKEZONE,
                                       curhost, curnode);
                              stat_text (logline);
                              sprintf (logline, "Warning %s", errorcomnt);
                              stat_text (logline);
                              sprintf (logline, "%s,%s,%s,%s,%s",
                                       Fieldone, Fieldtwo, bbs_name,
                                       location, sysop_name);
                              stat_text (logline);
                              sprintf (logline, ",%s,%s%s",
                                       phone_number, baud_rate, flags);
                              stat_text (logline);
                            }
                          if (NOTIFY == 'Y')
                            {
                              sprintf (logline, "\r-- %d:%d/%d --", MAKEZONE,
                                       curhost, curnode);
                              netmail_text (logline);
                              sprintf (logline, "Warning %s", errorcomnt);
                              netmail_text (logline);
                              sprintf (logline, "%s,%s,%s,%s,%s",
                                       Fieldone, Fieldtwo, bbs_name,
                                       location, sysop_name);
                              netmail_text (logline);
                              sprintf (logline, ",%s,%s%s",
                                       phone_number, baud_rate, flags);
                              netmail_text (logline);
                            }
                          if (flgerr == 1)
                            logbadflags (NOTIFY, STATS);
                        }
                    }
                  else
                    {
                      if (bcnt == 0)
                        {
                          if (expSourceFile[0] == 0)
                            sprintf (logline,
                                     "Error   Segment DATA in File : %-13.13s Line :%d will be commented out",
                                     ControlFile, (linecnt + Dlineoff));
                          else
                            sprintf (logline,
                                     "Error   Segment File : %-13.13s Line :%d will be commented out",
                                     segfile[bcnt].FileName, linecnt);
                          if (NOTIFY != 'Y')
                            netmail_text (logline);
                          logtext (logline, 0, YES);
                          //logtext (errorcomnt, 2, YES);
                          sprintf (logline, "%s,%s,%s,%s,%s,%s,%s%s\n",
                                   Fieldone, Fieldtwo, bbs_name,
                                   location, sysop_name,
                                   phone_number, baud_rate, flags);
                          if (NOTIFY != 'Y')
                            netmail_text (logline);
                          if (flgerr == 1)
                            if (NOTIFY != 'Y')
                              logbadflags (YES, STATS);
                        }
                      else
                        {
                          sprintf (logline,
                                   "Error   Segment File : %-13.13s Line :%d will be commented out",
                                   segfile[bcnt].FileName, linecnt);
                          logtext (logline, 0, YES);
                        }
                      logtext (errorcomnt, 2, YES);
                      if (STATS == 'Y')
                        {
                          sprintf (logline, "\n-- %d:%d/%d --", MAKEZONE,
                                   curhost, curnode);
                          stat_text (logline);
                          sprintf (logline, "Error %s", errorcomnt);
                          stat_text (logline);
                          sprintf (logline, "%s,%s,%s,%s,%s",
                                   Fieldone, Fieldtwo, bbs_name,
                                   location, sysop_name);
                          stat_text (logline);
                          sprintf (logline, ",%s,%s%s",
                                   phone_number, baud_rate, flags);
                          stat_text (logline);
                        }
                      if (NOTIFY == 'Y')
                        {
                          sprintf (logline, "\n-- %d:%d/%d --", MAKEZONE,
                                   curhost, curnode);
                          netmail_text (logline);
                          sprintf (logline, "Error %s", errorcomnt);
                          netmail_text (logline);
                          sprintf (logline, "%s,%s,%s,%s,%s",
                                   Fieldone, Fieldtwo, bbs_name,
                                   location, sysop_name);
                          netmail_text (logline);
                          sprintf (logline, ",%s,%s%s",
                                   phone_number, baud_rate, flags);
                          netmail_text (logline);
                        }
                      if (errorlvl <= 1)
                        errorlvl = 1;   // abort here not fatal
                      if (flgerr == 1)
                        logbadflags (NOTIFY, STATS);
                    }

                }               // End if comment
            }                   // While(1)

          fclose (segmentfile);
        }
      else
        {
          logtext ("Error Segment file not found", 0, YES);
          logtext (namebuf, 0, NO);
          if (STATS == 'Y')
            {
              stat_text ("Error Segment file not found");
              stat_text ("\n--End--\n");
            }
          if (NOTIFY == 'Y')
            {
              netmail_text ("Error Segment file not found");
              netmail_text ("\n--End--\n");
            }
          errorlvl = 2;         // abort fatal
        }

    }

  if (STATS == 'Y')
    close_stat_file ();


  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
    {
      freequicklst ();
      free_flags_lists ();
    }
  free (str);

  return (errorlvl);

}

short
process_new (void)
{
  short bcnt;
  char logline[255];
  char date_stamp[100];
  char CRCLine[100];
  short errorlvl = 0;
  short linecnt;
  char errorcomnt[255];
  short errcnt;
  short totalerrcnt = 0;
  short lineistype;
  short phonecnt;
  short flgerr = 0;
  short prcssed = YES;
  char namebuf[255];
  short ageis;

  str = (char *) malloc (MAXSTR);
  memset (str, 0, sizeof (str));

  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
    {
      loadquicklst ();
      init_flags_lists ();
    }


  for (bcnt = 1; bcnt <= sfilecnt; bcnt++)
    {
      //memset(namebuf,0,254);

      namebuf[0] = 0;

      sprintf (namebuf, "%s%s", MailFiles, segfile[bcnt].FileName);
      if (strchr (segfile[bcnt].FileName, '.') == NULL)
        {
          if (FindArch (namebuf) == 0)
            {
              if (decompress (namebuf) == 0)
                {
                  deletefile (namebuf);
                  sprintf (namebuf, "%s%s", MailFiles,
                           segfile[bcnt].FileName);
                  FindMostCurr (namebuf);
                }
              else
                {
                  sprintf (namebuf, "%s%s", MailFiles,
                           segfile[bcnt].FileName);
                  FindMostCurr (namebuf);
                }
            }
          else
            {
              FindMostCurr (namebuf);
            }
        }

      segmentfile = fopen (namebuf, "rt");

      if (segmentfile == NULL)
        {
          //fclose(segmentfile); - no, it's NULL
          sprintf (namebuf, "%s%s", Uploads, segfile[bcnt].FileName);
          if (strchr (segfile[bcnt].FileName, '.') == NULL)
            {
              if (FindArch (namebuf) == 0)
                {
                  if (decompress (namebuf) == 0)
                    {
                      deletefile (namebuf);
                      sprintf (namebuf, "%s%s", Uploads,
                               segfile[bcnt].FileName);
                      FindMostCurr (namebuf);
                    }
                  else
                    {
                      sprintf (namebuf, "%s%s", Uploads,
                               segfile[bcnt].FileName);
                      FindMostCurr (namebuf);
                    }
                }
              else
                {
                  FindMostCurr (namebuf);
                }
            }
        }

      // see if there is a diff
      if (segmentfile == NULL)
        {
          if (strchr (segfile[bcnt].FileName, '.') == NULL)
            apply_diff (segfile[bcnt].FileName, bcnt);
          else
            {
              //memset(namebuf,0,254);
              namebuf[0] = 0;
              sprintf (namebuf, "%s%s", MailFiles, segfile[bcnt].FileName);
              FindMostCurr (namebuf);
              segmentfile = fopen (namebuf, "rt");
            }
        }


      if (segmentfile != NULL)
        {
          fclose (segmentfile);
          segmentfile = NULL;
          if (test_crc (namebuf, CRCLine) == 0)
            {
              segmentfile = fopen (namebuf, "rt");
            }
          else
            {
              if (mnotify[1].active == 'Y')
                {
                  netmail_text ("\rFile not processed! \r\r---\r");
                  send_netmail ("File Receipt! CRC Errors!", bcnt, 1);
                  deletefile ("msgtext.tmp");
                }               ///
              if (segfile[bcnt].SegmentType <= 1)
                {
                  sprintf (logline, "Processing %-8.8s %-5d -- %s",
                           SegmentType[segfile[bcnt].SegmentType].String,
                           segfile[bcnt].Node, CRCLine);
                  logtext (logline, 0, YES);
                }
              else
                {
                  sprintf (logline, "Processing %-8.8s %-5d -- %s",
                           SegmentType[segfile[bcnt].SegmentType].String,
                           segfile[bcnt].Net, CRCLine);
                  logtext (logline, 0, YES);
                }
              if (segmentfile)
                fclose (segmentfile);
              movefile (namebuf, BadFiles);
              printf ("Moving %s to %s\n", namebuf, BadFiles);
              sprintf (logline, "Moving %s to %s", namebuf, BadFiles);
              logtext (logline, 1, YES);
            }
//              }

        }

      if (segmentfile)
        fclose (segmentfile);
      eof = calc_eof (namebuf);
      segmentfile = fopen (namebuf, "rt");


      if (segmentfile != NULL)
        {
          if (segfile[bcnt].SegmentType <= 1)
            {
              printf ("Processing %-8.8s %-5d -- file %s\n",
                      SegmentType[segfile[bcnt].SegmentType].String,
                      segfile[bcnt].Node, namebuf);
              sprintf (logline, "Processing %-8.8s %-5d -- file %s",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Node, namebuf);
              logtext (logline, 1, YES);
              logtext (CRCLine, 2, YES);
              sprintf (logline, "Processing %-8.8s %-5d",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Node);
              netmail_text (logline);
            }
          else
            {
              printf ("Processing %-8.8s %-5d -- file %s\n",
                      SegmentType[segfile[bcnt].SegmentType].String,
                      segfile[bcnt].Net, namebuf);
              sprintf (logline, "Processing %-8.8s %-5d -- file %s",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Net, namebuf);
              logtext (logline, 1, YES);
              logtext (CRCLine, 2, YES);
              sprintf (logline, "Processing %-8.8s %-5d",
                       SegmentType[segfile[bcnt].SegmentType].String,
                       segfile[bcnt].Net);
              netmail_text (logline);
            }

          //fseek (segmentfile, 0L, SEEK_END);
          //eof = ftell (segmentfile);
          //fseek (segmentfile, 0L, SEEK_SET);
          linecnt = 0;
          proc_type = 0;        // reset at start of every file
          clear_nodes_list ();

          while (1)
            {                   // end found
              if (eof == ftell (segmentfile))
                {
                  if (totalerrcnt == 0)
                    {
                      if (mnotify[0].active == 'Y')
                        {
                          if (prcssed == NO)
                            {
                              netmail_text
                                ("\rFile not processed! \r\r---\r");
                              send_netmail ("File Receipt : Critial Errors",
                                            bcnt, 1);
                            }
                          else
                            {
                              netmail_text ("\rNo Errors Detected");
                              netmail_text ("\rFile processed! \r\r---\r");
                              send_netmail ("File Receipt", bcnt, 0);
                            }
                        }
                    }
                  else
                    {
                      if (mnotify[1].active == 'Y')
                        {
                          if (prcssed == NO)
                            {
                              netmail_text
                                ("\rFile not processed! \r\r---\r");
                              send_netmail ("File Receipt : Critial Errors!",
                                            bcnt, 1);
                            }
                          else
                            {
                              netmail_text ("\rFile processed! \r\r---\r");
                              send_netmail ("File Receipt : Errors Found",
                                            bcnt, 1);
                            }
                        }
                    }
                  deletefile ("msgtext.tmp");   // delete if not sent
                  totalerrcnt = 0;
                  break;
                }
              //memset (str, 0, sizeof(str)-1);
              str[0] = 0;
              fgets (str, MAXSTR, segmentfile);
              // printf ("%s", str);
              linecnt++;
              if (str[0] != ';' && str[0] != 0 && str[0] != 26)
                {
                  ExtractInfo ();
                  errorcomnt[0] = 0;
                  errcnt = 0;

                  if (is_node_there (curnode) == 1)
                    {
                      errcnt++;
                      strcat (errorcomnt, "<Duplicate NodeNumber> ");
                    }
                  if (segfile[bcnt].SegmentType != proc_type)
                    {

                      logtext ("Error Segment file not correct type", 0, YES);
                      sprintf (logline, "%s is configured as %s but is %s",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               SegmentType[proc_type].String);
                      logtext (logline, 0, YES);
                      if (errorlvl <= 1)
                        errorlvl = 1;   // abort non fatal
                      prcssed = NO;
                      printf ("Moving %s to %s\n", namebuf, BadFiles);
                      sprintf (logline, "Moving %s to %s", namebuf, BadFiles);
                      fclose (segmentfile);
                      movefile (namebuf, BadFiles);
                      break;
                    }
                  //memset(errorcomnt,0,254);

                  if (segfile[bcnt].Net != curseg
                      && segfile[bcnt].SegmentType >= 2)
                    {
                      logtext ("Error Segment file not correct number", 0,
                               YES);
                      sprintf (logline,
                               "%s is configured as %s %d but is %s %d",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               segfile[bcnt].Net,
                               SegmentType[proc_type].String, curseg);
                      logtext (logline, 0, YES);
                      if (errorlvl <= 1)
                        errorlvl = 1;   // abort non fatal
                      prcssed = NO;
                      movefile (namebuf, BadFiles);
                      printf ("Moving %s to %s\n", namebuf, BadFiles);
                      sprintf (logline, "Moving %s to %s", namebuf, BadFiles);
                      logtext (logline, 1, YES);
                    }
                  else
                    if (segfile[bcnt].Node != curseg
                        && segfile[bcnt].SegmentType <= 1)
                    {
                      logtext ("Error Segment file not correct number", 0,
                               YES);
                      sprintf (logline,
                               "%s is configured as %s %d but is %s %d",
                               segfile[bcnt].FileName,
                               SegmentType[segfile[bcnt].SegmentType].String,
                               segfile[bcnt].Node,
                               SegmentType[proc_type].String, curseg);
                      logtext (logline, 0, YES);
                      if (errorlvl <= 1)
                        errorlvl = 1;   // abort non fatal
                      prcssed = NO;
                      movefile (namebuf, BadFiles);
                      printf ("Moving %s to %s\n", namebuf, BadFiles);
                      sprintf (logline, "Moving %s to %s", namebuf, BadFiles);
                      logtext (logline, 1, YES);
                    }


                  // test field 1
                  lineistype = 0;
                  if (*Fieldone != 0)
                    while (FieldOneType[lineistype].String != NULL)
                      {
                        if (strnicmp
                            (Fieldone, FieldOneType[lineistype].Single,
                             strlen (FieldOneType[lineistype].Single)) == 0)
                          {
                            break;
                          }
                        lineistype++;
                      }


                  if (*Fieldone != 0)
                    if (FieldOneType[lineistype].String == NULL)
                      {
                        errcnt++;
                        strcat (errorcomnt, "<Unknown Type> ");
                      }
                  if (lineistype == 1)
                    {           //
                      switch (DispPVT)
                        {
                        case 0:
                          errcnt++;
                          strcat (errorcomnt,
                                  "<PVT not allowed at any time> ");
                          break;
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                          if (curhostype > PVTlvl)
                            {
                              errcnt++;
                              strcat (errorcomnt,
                                      "<PVT not allowed above the ");
                              strcat (errorcomnt, SegmentType[PVTlvl].String);
                              strcat (errorcomnt, " level> ");
                            }
                          break;
                        }
                    }


                  if (MinPhone != 0)
                    {           //
                      phonecnt = checkphone ();
                      if (phonecnt != MinPhone)
                        {
                          if (phonecnt < MinPhone)
                            {
                              if (WARNINGS == 'N')
                                errcnt++;
                              strcat (errorcomnt,
                                      "<Phone Number too few parts> ");
                            }
                          else if (phonecnt > MaxPhone)
                            {
                              if (WARNINGS == 'N')
                                errcnt++;
                              strcat (errorcomnt,
                                      "<Phone Number too many parts> ");
                            }
                        }
                    }           //

                  if (checkbaud () != 0)
                    {           //
                      if (WARNINGS == 'N')
                        errcnt++;
                      strcat (errorcomnt, "<Invalid Baud Rate> ");
                    }           //

                  // check flags
                  // check Uflags
                  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
                    if ((flgerr = checkflags (flags)) == 1)
                      {
                        strcat (errorcomnt, "<Flags Error> ");
                      }

                  totalerrcnt += errcnt;
                  totalerrcnt += flgerr;
                  if (errcnt == 0)
                    {
                      if (errorcomnt[0] != 0)
                        {
                          sprintf (logline,
                                   "Warning Segment File : %-13.13s Line :%d has errors",
                                   segfile[bcnt].FileName, linecnt);
                          logtext (logline, 0, YES);
                          logtext (errorcomnt, 2, YES);
                          //memset(logline,0,254);
                          sprintf (logline, "\r-- %d:%d/%d --", MAKEZONE,
                                   curhost, curnode);
                          netmail_text (logline);
                          sprintf (logline, "Warning %s", errorcomnt);
                          netmail_text (logline);
                          sprintf (logline, "%s,%s,%s,%s,%s",
                                   Fieldone, Fieldtwo, bbs_name,
                                   location, sysop_name);
                          netmail_text (logline);
                          sprintf (logline, ",%s,%s%s",
                                   phone_number, baud_rate, flags);
                          netmail_text (logline);
                          if (flgerr == 1)
                            logbadflags (YES, NO);
                          //send_netmail("Sysop",bcnt,0);
                        }
                    }
                  else
                    {
                      if (errorlvl <= 1)
                        errorlvl = 1;   // abort here not fatal
                      sprintf (logline,
                               "Error   Segment File : %-13.13s Line :%d will be commented out",
                               segfile[bcnt].FileName, linecnt);
                      logtext (logline, 0, YES);
                      logtext (errorcomnt, 2, YES);
                      sprintf (logline, "\r-- %d:%d/%d --", MAKEZONE, curhost,
                               curnode);
                      netmail_text (logline);
                      sprintf (logline, "Error %s", errorcomnt);
                      netmail_text (logline);
                      //memset(logline,0,254);
                      sprintf (logline, "%s,%s,%s,%s,%s",
                               Fieldone, Fieldtwo, bbs_name,
                               location, sysop_name);
                      netmail_text (logline);
                      sprintf (logline, ",%s,%s%s",
                               phone_number, baud_rate, flags);
                      netmail_text (logline);
                      if (flgerr == 1)
                        logbadflags (YES, NO);
                      // send_netmail("Sysop",bcnt,0);
                    }

                }               // End if comment
            }                   // While(1)



          if (segmentfile != NULL)
            {
              fclose (segmentfile);
              printf ("Moving %s to %s\n", namebuf, Update);
              sprintf (logline, "Moving %s to %s", namebuf, Update);
              logtext (logline, 1, YES);
              touch_file_date (namebuf);        // touch up date
              movefile (namebuf, Update);

              if (CLEANUP == 'Y')
                {
                  if (strchr (segfile[bcnt].FileName, '.') == NULL)
                    {
                      sprintf (namebuf, "%s%s.", Master,
                               segfile[bcnt].FileName);
                      clean_dir (namebuf);
                    }
                }
            }
        }
      else
        {
          sprintf (namebuf, "%s%s", Master, segfile[bcnt].FileName);
          if (strchr (segfile[bcnt].FileName, '.') == NULL)
            FindMostCurr (namebuf);
          get_file_date (namebuf, date_stamp);
          if (date_stamp[0] != 0)
            {
              if (segfile[bcnt].SegmentType >= 2)
                sprintf (logline,
                         "%-8.8s %-5d: No New Segment File. Last in: %s",
                         SegmentType[segfile[bcnt].SegmentType].String,
                         segfile[bcnt].Net, date_stamp);
              else
                sprintf (logline,
                         "%-8.8s %-5d: No New Segment File. Last in: %s",
                         SegmentType[segfile[bcnt].SegmentType].String,
                         segfile[bcnt].Node, date_stamp);
              logtext (logline, 4, YES);
              // run date test
              if (MAXAGE != 0)
                {
                  ageis = file_age (namebuf);
                  if (MAXAGE <= ageis)
                    {
                      if (segfile[bcnt].SegmentType >= 2)
                        sprintf (logline,
                                 "%-8.8s %-5d: Segment File Age Warning %d days old",
                                 SegmentType[segfile[bcnt].SegmentType].
                                 String, segfile[bcnt].Net, ageis);
                      else
                        sprintf (logline,
                                 "%-8.8s %-5d: Segment File Age Warning %d days old",
                                 SegmentType[segfile[bcnt].SegmentType].
                                 String, segfile[bcnt].Node, ageis);
                      logtext (logline, 5, YES);
                      if (ISLATE == 'Y')
                        {
                          sprintf (logline,
                                   "Your segment is %d days old\rPlease Submit a new one as soon as possible.",
                                   ageis);
                          netmail_text (logline);
                          netmail_text ("\r--End--\r");
                          send_netmail ("Error Notification", bcnt, 1);
                        }
                    }
                }
            }
        }

    }

  if (FLAGCHK == 'Y' || FLAGCHKAUTO == 'Y')
    {
      freequicklst ();
      free_flags_lists ();
    }
  free (str);

  return (errorlvl);
}

void
copynew (void)
{
  short bcnt;
  char namebuf[255];
  char delnamebuf[255];
  char buffer[255];
  short filecnt = 0;

  netmail_text ("\rFiles received since last process run:\r");

  for (bcnt = 1; bcnt <= sfilecnt; bcnt++)
    {
      sprintf (namebuf, "%s%s", Update, segfile[bcnt].FileName);
      if (strchr (segfile[bcnt].FileName, '.') == NULL)
        FindMostCurr (namebuf);
      segmentfile = fopen (namebuf, "rt");
      if (segmentfile != NULL)
        {
          fclose (segmentfile);
          if (CLEANUP == 'Y' && stricmp (Update, Master) != 0)
            {
              sprintf (delnamebuf, "%s%s.*", Master, segfile[bcnt].FileName);
              deletefile (delnamebuf);
            }
          movefile (namebuf, Master);
          sprintf (buffer, "%s\r", namebuf);
          netmail_text (buffer);
          filecnt++;
        }
    }

  if (mnotify[2].active == 'Y')
    {
      if (filecnt == 0)
        netmail_text ("\rNONE\r");
      netmail_text ("\r--End--\r");
      send_netmail ("Segment Receive Notification", 0, 2);
    }
  else
    deletefile ("msgtext.tmp");

}

void
logbadflags (short email, short stats)
{
  char logline[255];

  if (*badflags != 0)
    {
      sprintf (logline, "Unknown Flags                : %s", badflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

  if (*caseflags != 0)
    {
      sprintf (logline, "Warning, Case error          : %s", caseflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

  if (*dupeflags != 0)
    {
      sprintf (logline, "Duplicate Flags              : %s", dupeflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

  if (*redunflags != 0)
    {
      sprintf (logline, "Redundant Flags              : %s", redunflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

  if (*rplcdflags != 0 && FLAGCHK == 'Y')
    {
      sprintf (logline, "Change Requested (old -> new): %s", rplcdflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

  if (*rplcdflags != 0 && FLAGCHKAUTO == 'Y')
    {
      sprintf (logline, "Replaced(Current -> Correct) : %s", rplcdflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

  if (*goodflags != 0 && FLAGCHKAUTO == 'Y')
    {
      sprintf (logline, "Auto Corrected Flags         : %s", goodflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

  if (*goodflags != 0 && FLAGCHK == 'Y')
    {
      sprintf (logline, "Suggested Flags              : %s", goodflags);
      logtext (logline, 3, YES);
      if (email == 1 || email == 'Y')
        netmail_text (logline);
      if (stats == 'Y')
        stat_text (logline);
    }

}
