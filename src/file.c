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
#include <fcntl.h>
#include "records.h"
#include "logdef.h"


extern char OutPath[];
extern char Master[];
extern char Update[];
extern short getJDate(short type);
extern short sfilecnt;
extern SEGFILE segfile[];
extern LINEPRMS Months[];
extern LINEPRMS SegmentType[];
extern char Publish_day[];
extern char * extchr(char * string, char dot);

// macros
#define YEAR(t)   (((t & 0xFE00) >> 9) + 1980)
#define MONTH(t)  ((t & 0x01E0) >> 5)
#define DAY(t)    (t & 0x001F)
#define HOUR(t)   ((t & 0xF800) >> 11)
#define MINUTE(t) ((t & 0x07E0) >> 5)
#define SECOND(t) ((t & 0x001F) << 1)
#define SETDATE(d,m,y)  (((y-1980) << 9) + (m << 5) + d)
#define SETTIME(h,m,s)  ((h << 11) + (m << 5) + (s >> 1))

// externs
extern void logtext(char *string, short indicator, short dateon);
extern void fix_proc_date(struct dosdate_t *date);
extern short FindMostCurr(char *FileName);
extern short getJDate(short type);

// prototype
long filesize(FILE *fp);
long calc_eof(char *filename);
short is_file_there(char *filename);
short file_age(char *filename);
short comp_compile_date(void);
void get_file_date(char *filename, char *date_stamp);


void copyfile(char *filename,char *destination)
{
        //char buffer[255];
        FILE *inf, *otf;
        char *mbptr;
        long size_read=0;
        unsigned long fsize=0L;

        inf = fopen (filename, "rb");
        if(inf == NULL) return;
        fsize = filesize(inf);
        //_heapshrink();

                //printf("calculated file size <%ld>\n",fsize);

#ifdef DOS
        mbptr = (char *) _fmalloc(6000);
#else
        mbptr = (char *) malloc(10000);
#endif

        if(mbptr == NULL)
        {
                fclose(inf);
                printf("Insufficent memory to copy <%ld>\n",fsize);
                exit(254);
        }
  //            printf("allocated memory to copy <%ld>\n",fsize);
        otf = fopen (destination, "wb");

        do {
        memset(mbptr,0,sizeof(mbptr));
   size_read = fread (mbptr, 1,sizeof(mbptr),inf);
   fwrite (mbptr, 1,size_read,otf);
        } while(size_read != 0);
        fclose(inf);
        fclose(otf);

#ifdef DOS
        _ffree(mbptr);
#endif
#ifdef OS2
        free(mbptr);
#endif
#ifdef WIN
        free(mbptr);
#endif
#ifdef LINUX
        free(mbptr);
#endif

}

void movefile(char *filename,char *destination)
{
        //char buffer[255];
        char finaldest[255];
        char destdrv[5];
        char destpath[255];
        char destfname[9];
        char destext[5];
        char sourdrv[5];
        char sourpath[255];
        char sourfname[9];
        char sourext[5];
        FILE *temp;

        if(destination[0] == 0)
        {
                temp = fopen (filename, "rt");
                        if(temp != NULL)
                        {
                   fclose(temp);
                        remove(filename);
                        }
          return;
        }

        _splitpath(filename,sourdrv,sourpath,sourfname,sourext);
        _splitpath(destination,destdrv,destpath,destfname,destext);

   _makepath(finaldest,destdrv,destpath,sourfname,sourext);
  //    sprintf(buffer,"copy %s %s >> NULL", filename, finaldest);
  //    printf("Buffer : %s\n",buffer);

        temp = fopen (filename, "rb");
        if(temp == NULL) return;
        fclose(temp);
        copyfile(filename,finaldest);
//  add_eof(finaldest);

        temp = fopen (finaldest, "rt");

        if(temp != NULL)
        {
                remove(filename);
                fclose(temp);
        }
}

void deletefile(char *filename)
{
        FILE *temp;

        temp = fopen (filename, "rt");
        if(temp != NULL)
        {
         fclose(temp);
         remove(filename);
        }
        remove("NULL");
}

void clean_dir(char *filename)
{
  long rc;
  char cleaname[255];
  struct find_t fileinfo;
  struct dosdate_t date;
  short jfriday, lastfriday;
  char *dot;
  char ascjfriday[5];

          dot = extchr (filename, '.');
          if (dot == NULL) return;

                sprintf(cleaname,"%s",filename);
                dot = extchr (cleaname, '.');
                if (dot != NULL) strcpy(dot,".*");
                else strcpy(cleaname,".*");

  rc = _dos_findfirst (cleaname, _A_NORMAL, &fileinfo);

  //printf("\nfind first %s\n",cleaname);

  jfriday = getJDate(1);
  lastfriday = jfriday - 7;
        if(lastfriday <= 0)
        {
        _dos_getdate (&date);
        date.year--;
        if((date.year % 4) == 0)
        lastfriday += 366;
        else
        lastfriday += 365;
        }

        itoa(jfriday,ascjfriday,10);
        if(jfriday < 100)
        {
                sprintf(ascjfriday,"%0.3d",jfriday);
        }

  if (rc == 0)
    {
      while (rc == 0)
        {
          dot = extchr (fileinfo.name, '.');
          if(strncmp(filename,Master,strlen(Master)) == 0)
     sprintf(cleaname,"%s%s",Master,fileinfo.name);
          else
     sprintf(cleaname,"%s%s",OutPath,fileinfo.name);
//     printf("\nfind  %s\n",cleaname);
          if (dot != NULL)
          {
            dot++;    // deal with outfiles
          if (atoi (dot) != lastfriday && atoi (dot) != jfriday)
          {
       //printf("\nfind  %c\n",*dot);
                if (isalpha(*dot)) // deal with archives
                {
                        dot++;
       //printf("\nfind  %c\n",*dot);
       //printf("\nfind  %s\n",ascjfriday);
           if (strncmp(dot,ascjfriday+1,2) != 0)
                deletefile(cleaname);
                } else
                deletefile(cleaname);
          }
          }
          // delete
          rc = _dos_findnext (&fileinfo);
        }
    }

}

short filecomp(char *filename1, char *filename2)
{
        FILE *file1, *file2;
        char str1[255];
        char str2[255];

        file1 = fopen(filename1,"rb");
        file2 = fopen(filename2,"rb");

        if(file1 == NULL || file2 == NULL)
        {
        if (file1) fclose(file1);
        if (file2) fclose(file2);
        return(1);
        }

  //    printf("filename 1: %s\n",filename1);
  //    printf("filename 2: %s\n",filename2);
        // ignore first line of file ;)
      fgets (str1, sizeof(str1), file1);
      fgets (str2, sizeof(str2), file2);

        while(1)
        {
      memset (str1, 0,sizeof(str1));
      fgets (str1, sizeof(str1), file1);
      memset (str2, 0, sizeof(str2));
      fgets (str2, sizeof(str2), file2);
                if(strcmp(str1,str2) != 0)
                {
                fclose(file1);
                fclose(file2);
                return(1);
                }
                if(str1[0] == 0 && str2[0] == 0) break;
        }

        fclose(file1);
        fclose(file2);

        return(0);
}

long filesize(FILE *fp)
{
        long size_of_file;

        fseek(fp,0L,SEEK_END);
        size_of_file = ftell(fp);
        fseek(fp,0L,SEEK_SET);

  // printf("file size before return <%ld>\n",size_of_file);

        return(size_of_file);

}

short is_file_there(char *filename)
{
        FILE *temp;

        temp = fopen (filename, "rt");
        if(temp == NULL) return(1);

        fclose(temp);
        return(0);

}

long calc_eof(char *filename)
{
        FILE *temp;
        long eof;

        temp = fopen (filename, "rt");
        if(temp == NULL) return(0);

        fseek (temp, 0L, SEEK_END);
        eof = ftell (temp);
        fclose(temp);

        return(eof);

}

void add_eof(char *filename)
{
        FILE *temp;
        char ch = 0x1A;

        temp = fopen (filename, "rb+");
        if(temp == NULL) return;

        fseek (temp, 0L, SEEK_END);
                fputc(ch,temp);

        fclose(temp);
}
#ifdef LINUX
void touch_file_date(char *filename)
{
  char cmd[80];
  strcpy(cmd,"touch ");
  strcat(cmd,filename);
  system(cmd);
}
#else
void touch_file_date(char *filename)
{
        int      handle;
   unsigned short date, time;
  struct dosdate_t cdate;
  struct dostime_t ctime;

  _dos_getdate (&cdate);
  _dos_gettime (&ctime);

           if( _dos_open( filename, O_RDWR, &handle ) != 0 ) {
             logtext( "Unable to set file time stamp",2,YES);
           } else {
             logtext( "Reset file time stamp",5,YES);
                                 // set today date time here!
                                 date = SETDATE(cdate.day,cdate.month,cdate.year);
                                 time = SETTIME(ctime.hour,ctime.minute,ctime.second);
             //time = (12 << 11) + (0 << 5) + 0;
             _dos_setftime( handle, date, time );
             _dos_close( handle );
           }

}
#endif

void list_file_dates(char *filename)
{
        short bcnt;
        char namebuf[255];
        char date_stamp[100];
        FILE *segmentfile;
        FILE *stats2;
        struct dosdate_t date;


        _dos_getdate (&date);
        fix_proc_date(&date);

        stats2 = fopen (filename, "wt");
        if(stats2 == NULL)
                {
      logtext( "Unable to open file lastin.txt",0,YES);
                return;
                }

        fprintf(stats2,"Segment file Dates for the week of %s, %s %0.2d, %d -- Day number %0.3d \n\n",
                                                                                                                        Publish_day,
                                                                                                                        Months[date.month].String,
                                                                                                                        date.day,
                                                                                                                        date.year,
                                                                                                                        getJDate(1));

        fprintf(stats2,"Last Processed Files:\n\n");

   for(bcnt=1;bcnt<=sfilecnt;bcnt++)
  {
        sprintf(namebuf,"%s%s",Master,segfile[bcnt].FileName);
        if(strchr(segfile[bcnt].FileName,'.') == NULL)
        FindMostCurr(namebuf);
        segmentfile = fopen (namebuf, "rt");
        if(segmentfile != NULL)
        {
        fclose(segmentfile);
        get_file_date(namebuf,date_stamp);
        if(segfile[bcnt].SegmentType >= 2)
        fprintf(stats2,"%-8.8s %-5d segment was last modified %s\n",
                                        SegmentType[segfile[bcnt].SegmentType].String,segfile[bcnt].Net,date_stamp);
        else
        fprintf(stats2,"%-8.8s %-5d segment was last modified %s",
                                SegmentType[segfile[bcnt].SegmentType].String,segfile[bcnt].Net,date_stamp);
        } else
                {
                if(segfile[bcnt].SegmentType >= 2)
                fprintf(stats2,"%-8.8s %-5d segment is not found in master directory\n",
                                        SegmentType[segfile[bcnt].SegmentType].String,segfile[bcnt].Net);
                else
                fprintf(stats2,"%-8.8s %-5d segment is not found in master directory\n",
                                        SegmentType[segfile[bcnt].SegmentType].String,segfile[bcnt].Node);
                }

  }

        fprintf(stats2,"\nUpdates to be processed:\n\n");

        for(bcnt=1;bcnt<=sfilecnt;bcnt++)
  {
        sprintf(namebuf,"%s%s",Update,segfile[bcnt].FileName);
        if(strchr(segfile[bcnt].FileName,'.') == NULL)
        FindMostCurr(namebuf);
        segmentfile = fopen (namebuf, "rt");
        if(segmentfile != NULL)
        {
        fclose(segmentfile);
        get_file_date(namebuf,date_stamp);
        if(segfile[bcnt].SegmentType >= 2)
        fprintf(stats2,"%-8.8s %-5d segment was last modified %s\n",
                                        SegmentType[segfile[bcnt].SegmentType].String,segfile[bcnt].Net,date_stamp);
        else
        fprintf(stats2,"%-8.8s %-5d segment was last modified %s",
                                SegmentType[segfile[bcnt].SegmentType].String,segfile[bcnt].Net,date_stamp);
        }

  }

  fclose(stats2);
  logtext( "Created lastin.txt",2,YES);
}

void get_file_date(char *filename, char *date_stamp)
{
        int      handle;
   unsigned short date, time;

        date_stamp[0] = 0;

           if( _dos_open( filename, O_RDWR, &handle ) != 0 ) {
             logtext( "Unable to get file time stamp",2,YES);
           } else {
             logtext( "Retrieved file time stamp",6,YES);

             _dos_getftime( handle, &date, &time );
             sprintf(date_stamp,"%0.2d/%0.2d/%d @ %0.2d:%0.2d:%0.2d",
                     MONTH(date), DAY(date), YEAR(date),
                     HOUR(time), MINUTE(time), SECOND(time) );
             _dos_close( handle );
           }
}

short file_age(char *filename)
{
        int      handle;
   unsigned short date, time;
        unsigned short Today_JDate=0, File_JDate=0, dayscnt = 367;
        struct dosdate_t cdate;
        _dos_getdate (&cdate);


           if( _dos_open( filename, O_RDWR, &handle ) != 0 ) {
             logtext( "Unable to get file time stamp",2,YES);
           } else {
             logtext( "Retrieved file time stamp",6,YES);

             _dos_getftime( handle, &date, &time );

  switch(MONTH(date))
  {
                case 1:
                File_JDate = DAY(date);
                break;
                case 2:
                File_JDate += 31;
                File_JDate += DAY(date);
                break;
                case 3:
                if((YEAR(date) % 4) == 0)
                File_JDate += 60;
                else
                File_JDate += 59;
                File_JDate += DAY(date);
                break;
                case 4:
                if((YEAR(date) % 4) == 0)
                File_JDate += 91;
                else
                File_JDate += 90;
                File_JDate += DAY(date);
                break;
                case 5:
                if((YEAR(date) % 4) == 0)
                File_JDate += 121;
                else
                File_JDate += 120;
                File_JDate += DAY(date);
                break;
                case 6:
                if((YEAR(date) % 4) == 0)
                File_JDate += 152;
                else
                File_JDate += 151;
                File_JDate += DAY(date);
                break;
                case 7:
                if((YEAR(date) % 4) == 0)
                File_JDate += 182;
                else
                File_JDate += 181;
                File_JDate += DAY(date);
                break;
                case 8:
                if((YEAR(date) % 4) == 0)
                File_JDate += 213;
                else
                File_JDate += 212;
                File_JDate += DAY(date);
                break;
                case 9:
                if((YEAR(date) % 4) == 0)
                File_JDate += 244;
                else
                File_JDate += 243;
                File_JDate += DAY(date);
                break;
                case 10:
                if((YEAR(date) % 4) == 0)
                File_JDate += 274;
                else
                File_JDate += 273;
                File_JDate += DAY(date);
                break;
                case 11:
                if((YEAR(date) % 4) == 0)
                File_JDate += 305;
                else
                File_JDate += 304;
                File_JDate += DAY(date);
                break;
                case 12:
                if((YEAR(date) % 4) == 0)
                File_JDate += 335;
                else
                File_JDate += 334;
                File_JDate += DAY(date);
                break;
  }
                        Today_JDate = getJDate(0);

                                          if(cdate.year == YEAR(date)) // This year
                                          {
                                                dayscnt = Today_JDate - File_JDate;
                                          } else
                                          if(cdate.year == YEAR(date)+1) // last year
                                          {
                                                if((YEAR(date) % 4) == 0) // was leap
                                                dayscnt = Today_JDate + 366 - File_JDate;
                                                else
                                                dayscnt = Today_JDate + 365 - File_JDate;
                                          }

             _dos_close( handle );
           }

                          if(dayscnt <= 365)
                          return(dayscnt);
                          else
                          return(365);

}


void fix_compile_date(void)
{
#ifndef LINUX
        int      handle;
        unsigned short date, time;

           if( _dos_open( "flags.ctl", O_RDWR, &handle ) != 0 ) {
             logtext( "Unable to get file time stamp",2,YES);
           } else {
             _dos_getftime( handle, &date, &time );
             _dos_close( handle );
           if( _dos_open( "quick.lst", O_RDWR, &handle ) != 0 ) {
             logtext( "Unable to set file time stamp",2,YES);
                 } else {
                  logtext( "Reset file time stamp",5,YES);

                  _dos_setftime( handle, date, time );
                  _dos_close( handle );
                 }
           }
#endif
}

short comp_compile_date(void)
{
        int      handle;
   unsigned short Fdate=1, Ftime=1;
   unsigned short Qdate=0, Qtime=0;

           if( _dos_open( "flags.ctl", O_RDWR, &handle ) != 0 ) {
             logtext( "Unable to read flags.ctl date",2,YES);
           } else {
             logtext( "Checking for update of FLAGS lists.",5,YES);

             _dos_getftime( handle, &Fdate, &Ftime );
             _dos_close( handle );
           }

           if( _dos_open( "quick.lst", O_RDWR, &handle ) != 0 ) {
             logtext( "Unable to read quick.lst date",2,YES);
           } else {
             _dos_getftime( handle, &Qdate, &Qtime );
             _dos_close( handle );
           }

         if(Qdate == Fdate && Qtime == Ftime)
         {
    logtext( "Compile of flags.ctl is not required (same dates).",5,YES);
         return(0);
         }

    logtext( "Compile of flags.ctl is required.",5,YES);
         return(1);

}
