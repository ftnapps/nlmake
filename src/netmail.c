#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#ifdef LINUX
#include "doslinux.h"
#else
#include <dos.h>
#endif
#include "records.h"
#include "logdef.h"


#define                 KLUDGE 0x01
#define                 RCVED  0x04
#define         MM16    0xA001  /* crc-16 mask */
#define         MMTT    0x1021  /* crc-ccitt mask */
#define     MBUFSIZE 50000

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
short mupdcrcr (unsigned short crc, short c, unsigned short mask);
short mupdcrc (unsigned short crc, short c, unsigned short mask);
void byte_convert (char *DByte, short number);


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


void
send_netmail (char *subject, short SFI, char Type)
{
  FILE *ptr;
  FILE *ptr1;
  char messagepath[255];
  char sysop[36];
  short multimessage = 0;

  short cnt, rc, eof = 0, mattrib = 0;
  unsigned short crc16, crctt;
  char intl = 0;
  short msgnum = 0;
  char msgcrc[25];
  char msgintl[25];
  char DByte[5];
  char *front;
  char *dot;
  char *msg;

  struct dosdate_t date;        //date
  struct dostime_t time;        //time

  struct find_t fileinfo;

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

#ifdef DOS
  front = (char *) _fmalloc (MBUFSIZE);
#else
  front = (char *) malloc (MBUFSIZE);
#endif

  if (front == NULL)
    {
      printf ("malloc error\n");
      exit (0);
    }

  msg = front;
  memset (msg, 0, MBUFSIZE);

  // Write fromName, toName & subject to msg
  memmove (msg, ProgName, strlen (ProgName));
  msg += 36;
  memmove (msg, sysop, strlen (sysop));
  msg += 36;
  if (strlen (subject) <= 71)
    memmove (msg, subject, strlen (subject));
  else
    memmove (msg, subject, 71);
  msg += 72;

  // Write date/time to msg
  _dos_getdate (&date);
  _dos_gettime (&time);
  sprintf (msg, "%02d %s %02d  %02d:%02d:%02d",
             date.day, Months[date.month].Single, date.year % 100,
             time.hour, time.minute, time.second);
  msg += 22;

  if (Type == 3 || segfile[SFI].AltNotify[0] != 0)
    byte_convert (DByte, SUBNODE);      // submit node
  else
    byte_convert (DByte, segfile[SFI].Node);
  memmove (msg, DByte, 2);      // Dest Net
  msg += 2;
  // *msg = 0xA6;                        // Orig net (168)
  byte_convert (DByte, MAKENODE);
  memmove (msg, DByte, 2);      // Dest Net
  msg += 4;

  //*msg = 0xA7;                        // Orig Node       (174)
  byte_convert (DByte, MAKENET);
  memmove (msg, DByte, 2);      // Dest Net
  msg += 2;

  if (Type == 3 || segfile[SFI].AltNotify[0] != 0)
    byte_convert (DByte, SUBNET);       // submit net
  else
    byte_convert (DByte, segfile[SFI].Net);
  memmove (msg, DByte, 2);      // Dest Net
  msg += 11;                    // left off here

  *(msg++) = 0x00;              // Leave here

  //*(msg++) = 0x83;            // Attribs  low order

  //*(msg++) = 0x01;            // Attribs  high order

  if (mnotify[Type].Crash == 'Y')
    mattrib += 2;
  if (mnotify[Type].Hold == 'Y')
    mattrib += 512;
  if (mnotify[Type].Normal == 'Y')
    mattrib = 0;
  if (mnotify[Type].Intl == 'Y')
    intl = 1;
  mattrib += 128;               // Set Kill sent
  mattrib += 256;               // Set Local
  mattrib += 1;                 // Set Local


  if (Type != 3)
    sprintf (msgintl, "INTL %d:%d/%d %d:%d/%d", segfile[SFI].Zone,
             segfile[SFI].Net, segfile[SFI].Node, MAKEZONE, MAKENET,
             MAKENODE);
  else
    {
      sprintf (msgintl, "INTL %d:%d/%d %d:%d/%d", SUBZONE, SUBNET, SUBNODE,
               MAKEZONE, MAKENET, MAKENODE);
      mattrib += 16;            // Set File Attach
    }

  byte_convert (DByte, mattrib);        // submit net
  memmove (msg, DByte, 2);      // Dest Net

  msg += 2;

  *(msg++) = 0x00;              // Reply

  *(msg++) = 0x00;              // Leave here

  /*     for(cnt=0;cnt<=4;cnt++)
     {
     printf("%d",cnt);
     *(msg+=1) = 0xFF;
     } */

  //msg += 1;
  *msg = (char) 0x01;

  crc16 = crctt = 0;
  for (cnt = 0; cnt <= 164; cnt++)
    {
      crc16 = mupdcrcr (crc16, *front + cnt, MM16);
      crctt = mupdcrc (crctt, *front + cnt, MMTT);
    }
  // stuff intl here
  sprintf (msgcrc, "MSGID: %d:%d/%d %04X%04X", MAKEZONE, MAKENET, MAKENODE,
           crc16, crctt);
  memmove (msg += 1, msgcrc, strlen (msgcrc));  // Message ID CRC
  *(msg += strlen (msgcrc)) = 0x0D;     // end message ID
  msg++;

  if (intl == 1)
    {
      *msg = (char) 0x01;
      memmove (msg += 1, msgintl, strlen (msgintl));    // Message ID CRC
      *(msg += strlen (msgintl)) = 0x0D;        // end message ID
      msg++;
    }


  ptr1 = fopen ("msgtext.tmp", "rb");
  if (ptr1 != NULL)
    {
      fseek (ptr1, 0L, SEEK_END);
      eof = ftell (ptr1);
      fseek (ptr1, 0L, SEEK_SET);
    }
  while (multimessage == 0)
    {
      if (multimessage == 1)
        break;
      if (ptr1 != NULL)
        {
          if ((MBUFSIZE - (msg - front)) >= eof)
            {
              fread (msg, eof, 1, ptr1);
              msg += eof;
              multimessage = 1;
            }
          else
            {
              fread (msg, MBUFSIZE - (msg - front), 1, ptr1);
              eof -= MBUFSIZE - (msg - front);
              msg += MBUFSIZE - (msg - front);
              multimessage = 0;
            }
        }
      else
        {
          switch (Type)
            {
            case 0:
              sprintf (msg, "\rSegment file received \rNo Errors Detected\r");
              eof = strlen (msg);
              break;
            case 1:
              sprintf (msg,
                       "\rLocal Segment File Update\rNo Errors Detected\r");
              eof = strlen (msg);
              break;
            case 2:
              sprintf (msg, "\rSegment File Received \rNo Errors Detected\r");
              eof = strlen (msg);
              break;
            case 3:
              sprintf (msg, "\rSegment Update\r\r---\r");
              eof = strlen (msg);
              break;
            default:
              sprintf (msg, "\rSegment Update\r\r---\r");
              eof = strlen (msg);
              break;
            }
          msg += eof;
          multimessage = 1;
        }

      sprintf (messagepath, "%s*.msg", Messages);
      rc = _dos_findfirst (messagepath, _A_NORMAL, &fileinfo);

      if (rc == 0)
        {
          while (rc == 0)
            {
              dot = extchr (fileinfo.name, '.');
              if (dot != NULL)
                *dot = 0;
              else
                break;
              if (atoi (fileinfo.name) > msgnum)
                msgnum = atoi (fileinfo.name);
              rc = _dos_findnext (&fileinfo);
            }
        }

      sprintf (messagepath, "%s%d.msg", Messages, msgnum + 1);


      ptr = fopen (messagepath, "wb+");

      if (ptr == NULL)
        {
          logtext ("Message path not found. ", 1, YES);
          return;
        }

      fwrite (front, sizeof (char), (long) (msg - front), ptr);
      fclose (ptr);
      if (ptr1)
        fclose (ptr1);
    }
  deletefile ("msgtext.tmp");
#ifdef DOS
  _ffree (front);
#else
  free (front);
#endif

}


short
mupdcrcr (unsigned short crc, short c, unsigned short mask)
{
  short i;
  for (i = 0; i < 8; i++)
    {
      if ((crc ^ c) & 1)
        crc = (crc >> 1) ^ mask;
      else
        crc >>= 1;
      c >>= 1;
    }
  return (short) crc;
}

short
mupdcrc (unsigned short crc, short c, unsigned short mask)
{
  short i;
  c <<= 8;
  for (i = 0; i < 8; i++)
    {
      if ((crc ^ c) & 0x8000)
        crc = (crc << 1) ^ mask;
      else
        crc <<= 1;
      c <<= 1;
    }
  return (short) crc;
}

void
byte_convert (char *DByte, short number)
{
  *(DByte + 1) = (char) (number / 256);
  *DByte = (char) (number % 256);

//      printf("netmail : %X %X",*DByte,*(DByte+1));
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
