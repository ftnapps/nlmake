#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__linux__) || defined(__EMX__)
#include "doslinux.h"
#elif defined(__DOS__) || defined(__NT__) || defined(__OS2__)
#include <dos.h>
#endif

#include "records.h"
#include "logdef.h"


#define                 KLUDGE 0x01
#define                 RCVED  0x04
#define     MBUFSIZE 50000

#ifdef __linux__
#define stricmp strcasecmp
#endif


// externs

extern LINEPRMS Months[];
extern SEGFILE segfile[];
extern short MAKEZONE;
extern short MAKENET;
extern short MAKENODE;
extern char Messages[];
extern char SubAddress[];
extern MSG mnotify[];
extern char OutFile[];
extern char SubNameNotify[];

extern void deletefile (char *filename);
extern void logwrite (short message, short indicator);
extern void logtext (char *string, short indicator, short dateon);
extern char *extchr (char *string, char dot);

extern char ProgName[];

// prototypes
short subbreakaddress (char *address);


short SUBZONE;
short SUBNET;
short SUBNODE;


void
netmail_text (char *message)
{
  FILE *msgtext;

  msgtext = fopen ("msgtext.tmp", "at+");
  fprintf (msgtext, "%s\r", message);

  fclose (msgtext);
}

/* Get serial number for MsgId */
unsigned long lastSerial = 0;
unsigned long
getSerial (void)
{
  unsigned long serial;

  serial = time (NULL);
  if (serial <= lastSerial)
    {
      serial = lastSerial + 1;
    }
  lastSerial = serial;

  return serial;
}

/* Calculate UTC offset - this function is stolen from Tobias Ernst's MsgEd :) */
int
tz_my_offset (void)
{
  time_t now;
  struct tm *tm;

  int gm_minutes;
  long gm_days;
  int local_minutes;
  long local_days;

  tzset ();

  now = time (NULL);

  tm = localtime (&now);
  local_minutes = tm->tm_hour * 60 + tm->tm_min;
  local_days = (long) tm->tm_year * 366L + (long) tm->tm_yday;

  tm = gmtime (&now);
  gm_minutes = tm->tm_hour * 60 + tm->tm_min;
  gm_days = (long) tm->tm_year * 366L + (long) tm->tm_yday;

  if (gm_days < local_days)
    {
      local_minutes += 1440;
    }
  else if (gm_days > local_days)
    {
      gm_minutes += 1440;
    }

  return local_minutes - gm_minutes;
}


void
send_netmail (char *subject, short SFI, char Type)
{
  FILE *ptr;
  FILE *ptr1;
  char messagepath[255];
  char sysop[36];

  short mattrib = 0;
  char *front;
  char *msg;
  int destZone, destNet, destNode;
  int utcOffset;

  time_t utime;
  struct tm *tm;
  DIR *dir;

  front = (char *) malloc (MBUFSIZE);
  if (front == NULL)
    {
      printf ("malloc error\n");
      exit (0);
    }

  msg = front;
  memset (msg, 0, MBUFSIZE);

  if (Type != 3 && segfile[SFI].NameNotify[0] != 0)
    strcpy (sysop, segfile[SFI].NameNotify);
  else
    strcpy (sysop, "Coordinator");

  if (Type == 3 && SubNameNotify[0] != 0)
    strcpy (sysop, SubNameNotify);

  if (Type != 3 && segfile[SFI].AltNotify[0] != 0)
    subbreakaddress (segfile[SFI].AltNotify);
  else
    subbreakaddress (SubAddress);

  if (Type != 3)
    {
      destZone = segfile[SFI].Zone;
      destNet = segfile[SFI].Net;
      destNode = segfile[SFI].Node;
    }
  else
    {
      destZone = SUBZONE;
      destNet = SUBNET;
      destNode = SUBNODE;
    }

  // Write fromName, toName & subject to msg
  snprintf (msg, 36, "%s", ProgName);
  msg += 36;
  snprintf (msg, 36, "%s", sysop);
  msg += 36;
  snprintf (msg, 72, "%s", subject);
  msg += 72;

  // Write date/time to msg
  time (&utime);
  tm = localtime (&utime);
  strftime (msg, 20, "%d %b %y  %H:%M:%S", tm);
  msg += 20;

  msg += 2;                     // skip timesRead

  *msg++ = destNode & 255;
  *msg++ = destNode >> 8;

  *msg++ = MAKENODE & 255;
  *msg++ = MAKENODE >> 8;

  msg += 2;                     // skip cost

  *msg++ = MAKENET & 255;
  *msg++ = MAKENET >> 8;

  *msg++ = destNet & 255;
  *msg++ = destNet >> 8;

  msg += 10;                    // skip zone, point & replyTo

  mattrib |= 1;                 // Set Private
  mattrib |= 128;               // Set Kill sent
  mattrib |= 256;               // Set Local
  //if (mnotify[Type].Normal == 'Y')
  //mattrib = 0;
  if (mnotify[Type].Crash == 'Y')
    mattrib |= 2;
  if (mnotify[Type].Hold == 'Y')
    mattrib |= 512;

  if (Type == 3)
    {
      mattrib |= 16;            // Set File Attach
    }

  *msg++ = mattrib & 255;
  *msg++ = mattrib >> 8;

  msg += 2;                     // skip nextReply


  // Message body begins

  if (mnotify[Type].Intl == 'Y')
    {
      sprintf (msg, "\01INTL %d:%d/%d %d:%d/%d\r", destZone, destNet,
               destNode, MAKEZONE, MAKENET, MAKENODE);
      msg += strlen (msg);
    }

  sprintf (msg, "\01CHRS: ASCII 1\r");
  msg += strlen (msg);

  sprintf (msg, "\01MSGID: %d:%d/%d %08lX\r", MAKEZONE, MAKENET, MAKENODE,
           getSerial ());
  msg += strlen (msg);

  utcOffset = tz_my_offset ();
  sprintf (msg, "\01TZUTC: %.4d\r",
           ((utcOffset / 60) * 100) + (utcOffset % 60));
  msg += strlen (msg);


  // Find highest message number
  if ((dir = opendir (Messages)) != NULL)
    {
      char *dot;
      struct dirent *entry;
      int msgnum = 0;

      while ((entry = readdir (dir)) != NULL)
        {
          int num;

          dot = extchr (entry->d_name, '.');
          if ((dot == NULL) || stricmp (dot + 1, "msg"))
            {
              // not *.msg
              continue;
            }
          *dot = 0;

          num = atoi (entry->d_name);
          if (num > msgnum)
            {
              msgnum = num;
            }
        }

      closedir (dir);
      sprintf (messagepath, "%s%d.msg", Messages, msgnum + 1);
    }
  else
    {
      logtext ("Message path not found. ", 1, YES);
      return;
    }

  ptr = fopen (messagepath, "wb+");
  if (ptr == NULL)
    {
      logtext ("Can not open msg file. ", 1, YES);
      return;
    }


  // write msg header & stuff
  fwrite (front, 1, (long) (msg - front), ptr);


  ptr1 = fopen ("msgtext.tmp", "rb");
  if (ptr1 != NULL)
    {
      size_t r;

      // Process file
      while ((r = fread (front, 1, MBUFSIZE, ptr1)) > 0)
        {
          fwrite (front, 1, r, ptr);
        }
    }
  else
    {
      char *s;

      // File not found - use standard response
      switch (Type)
        {
        case 0:
          s = "\rSegment file received \rNo Errors Detected\r";
          break;
        case 1:
          s = "\rLocal Segment File Update\rNo Errors Detected\r";
          break;
        case 2:
          s = "\rSegment File Received \rNo Errors Detected\r";
          break;
        case 3:
        default:
          s = "\rSegment Update\r\r---\r";
          break;
        }

      fprintf (ptr, "%s", s);
    }

  fprintf (ptr, "%c", 0);

  if (ptr1)
    fclose (ptr1);
  deletefile ("msgtext.tmp");

  fclose (ptr);

  sprintf (front, "Wrote netmail (%s)", messagepath);
  logtext (front, 1, YES);

  free (front);
}

short
subbreakaddress (char *address)
{
  char *pnet, *pnode, *pzone;

  if (strchr (address, ':') != NULL)
    {
      pzone = address;
      if (strchr (address, '/') != NULL)
        {
          pnet = strchr (address, ':');
          pnode = strchr (address, '/');
          pnet++;
          pnode++;
        }
    }
  else
    {
      if (strchr (address, '/') != NULL)
        {
          pnode = strchr (address, '/');
          pnode++;
          pnet = address;
        }
      else
        return (1);
    }

  if (pzone == NULL)
    SUBZONE = MAKEZONE;
  else
    SUBZONE = atoi (pzone);

  if (pnet != NULL)
    SUBNET = atoi (pnet);

  if (pnode != NULL)
    SUBNODE = atoi (pnode);


  //printf("submit address %d:%d/%d",MAKEZONE,MAKENET,MAKENODE);

  return (0);
}
