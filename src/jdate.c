#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef LINUX
#include "doslinux.h"
#define stricmp strcasecmp
#else
#include <dos.h>
#endif
#include "records.h"
#include "logdef.h"


extern LINEPRMS WeekDays[];
extern char Publish_day[];

short getJDate(short type)
{
  struct dosdate_t date;
  short jdate = 0, jfriday =0, i=0;

  _dos_getdate (&date);

 // date.year = 2000;

  switch(date.month)
  {
		case 1:
		jdate = date.day;
		break;
		case 2:
		jdate += 31;
		jdate += date.day;
		break;
		case 3:
		if((date.year % 4) == 0)
		jdate += 60;
		else
		jdate += 59;
		jdate += date.day;
		break;
		case 4:
		if((date.year % 4) == 0)
		jdate += 91;
		else
		jdate += 90;
		jdate += date.day;
		break;
		case 5:
		if((date.year % 4) == 0)
		jdate += 121;
		else
		jdate += 120;
		jdate += date.day;
		break;
		case 6:
		if((date.year % 4) == 0)
		jdate += 152;
		else
		jdate += 151;
		jdate += date.day;
		break;
		case 7:
		if((date.year % 4) == 0)
		jdate += 182;
		else
		jdate += 181;
		jdate += date.day;
		break;
		case 8:
		if((date.year % 4) == 0)
		jdate += 213;
		else
		jdate += 212;
		jdate += date.day;
		break;
		case 9:
		if((date.year % 4) == 0)
		jdate += 244;
		else
		jdate += 243;
		jdate += date.day;
		break;
		case 10:
		if((date.year % 4) == 0)
		jdate += 274;
		else
		jdate += 273;
		jdate += date.day;
		break;
		case 11:
		if((date.year % 4) == 0)
		jdate += 305;
		else
		jdate += 304;
		jdate += date.day;
		break;
		case 12:
		if((date.year % 4) == 0)
		jdate += 335;
		else
		jdate += 334;
		jdate += date.day;
		break;
  }

  for(i=0;i<=6;i++)
  {
	if(stricmp(Publish_day,WeekDays[i].String) == 0)
	break;
  }

			if((i - date.dayofweek) >= 0)
				jfriday = jdate + i - date.dayofweek;
			else
				jfriday = jdate + i - date.dayofweek + 7;

  //printf("today is -> %d Friday is ->%d",jdate, jfriday);
  if(type == 1)
  return(jfriday);
  else
  return(jdate);

}

void fix_proc_date(struct dosdate_t *date)
{
  short i, jfriday = 0, year = 0;

  for(i=0;i<=6;i++)
  {
	if(stricmp(Publish_day,WeekDays[i].String) == 0)
	break;
  }

			if((i - date->dayofweek) >= 0)
				jfriday = date->day + i - date->dayofweek;
			else
				jfriday = date->day + i - date->dayofweek + 7;


		if(jfriday <= 28)
		{
		date->day = jfriday;
		return;
		}

		switch(date->month)
		{
			case 1:  // jan
			if(jfriday >= 32)
			{
				jfriday -= 31;
				date->month = 2;
			}
			break;
			case 2: // feb
			if((date->year % 4) == 0) // is leap
			{
				if(jfriday >= 30)
				{
					jfriday -= 29;
					date->month = 3;
				}
			} else
				if(jfriday >= 29)
				{
					jfriday -= 28;
					date->month = 3;
				}
			break;
			case 3:  // mar
			if(jfriday >= 32)
			{
				jfriday -= 31;
				date->month = 4;
			}
			break;
			case 4:  // april
			if(jfriday >= 31)
			{
				jfriday -= 30;
				date->month = 5;
			}
			break;
			case 5:  // may
			if(jfriday >= 32)
			{
				jfriday -= 31;
				date->month = 6;
			}
			break;
			case 6:  // june
			if(jfriday >= 31)
			{
				jfriday -= 30;
				date->month = 7;
			}
			break;
			case 7:  // jul
			if(jfriday >= 32)
			{
				jfriday -= 31;
				date->month = 8;
			}
			break;
			case 8: // aug
			if(jfriday >= 32)
			{
				jfriday -= 31;
				date->month = 9;
			}
			break;
			case 9:  // sep
			if(jfriday >= 31)
			{
				jfriday -= 30;
				date->month = 10;
			}
			break;
			case 10: // oct
			if(jfriday >= 32)
			{
				jfriday -= 31;
				date->month = 11;
			}
			break;
			case 11:  // nov
			if(jfriday >= 31)
			{
				jfriday -= 30;
				date->month = 12;
			}
			break;
			case 12:  // dec
			if(jfriday >= 32)
			{
				jfriday -= 31;
				date->month = 1;
				year = date->year;
				year ++;
				date->year = year;
			}
			break;
		}

		date->day = jfriday;

}


