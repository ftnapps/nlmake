#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#define stricmp strcasecmp
#endif

#include "records.h"
#include "logdef.h"

extern LINEPRMS WeekDays[];
extern char Publish_day[];

// Get current day_of_year (if type == 0) or day_of_year for next publish day (if type == 1)
short
getJDate (short type)
{
  short jdate, jfriday, i;
  time_t utime;
  struct tm *tm;

  time (&utime);
  tm = localtime (&utime);

  jdate = tm->tm_yday;

  for (i = 0; i <= 6; i++)
    {
      if (stricmp (Publish_day, WeekDays[i].String) == 0)
        break;
    }

  if ((i - tm->tm_wday) >= 0)
    jfriday = jdate + i - tm->tm_wday;
  else
    jfriday = jdate + i - tm->tm_wday + 7;

  //printf("today is -> %d Friday is ->%d",jdate, jfriday);
  if (type == 1)
    return (jfriday);
  else
    return (jdate);

}


/* Get next publishing date */
void
fix_proc_date (struct tm *date)
{
  short i, jfriday;

  // Get day_of_week for publish
  for (i = 0; i <= 6; i++)
    {
      if (stricmp (Publish_day, WeekDays[i].String) == 0)
        break;
    }

  // Get day_of_month for publish
  if ((i - date->tm_wday) >= 0)
    jfriday = date->tm_mday + i - date->tm_wday;
  else
    jfriday = date->tm_mday + i - date->tm_wday + 7;


  // No adjustment of day & month needed
  if (jfriday <= 28)
    {
      date->tm_mday = jfriday;
      return;
    }

  // Adjust day & month if we've fallen into next month...
  switch (date->tm_mon)
    {
    case 1:                     // jan
      if (jfriday >= 32)
        {
          jfriday -= 31;
          date->tm_mon++;
        }
      break;
    case 2:                     // feb
      if ((date->tm_year % 4) == 0)        // is leap
        {
          if (jfriday >= 30)
            {
              jfriday -= 29;
              date->tm_mon++;
            }
        }
      else if (jfriday >= 29)
        {
          jfriday -= 28;
          date->tm_mon++;
        }
      break;
    case 3:                     // mar
      if (jfriday >= 32)
        {
          jfriday -= 31;
          date->tm_mon++;
        }
      break;
    case 4:                     // april
      if (jfriday >= 31)
        {
          jfriday -= 30;
          date->tm_mon++;
        }
      break;
    case 5:                     // may
      if (jfriday >= 32)
        {
          jfriday -= 31;
          date->tm_mon++;
        }
      break;
    case 6:                     // june
      if (jfriday >= 31)
        {
          jfriday -= 30;
          date->tm_mon++;
        }
      break;
    case 7:                     // jul
      if (jfriday >= 32)
        {
          jfriday -= 31;
          date->tm_mon++;
        }
      break;
    case 8:                     // aug
      if (jfriday >= 32)
        {
          jfriday -= 31;
          date->tm_mon++;
        }
      break;
    case 9:                     // sep
      if (jfriday >= 31)
        {
          jfriday -= 30;
          date->tm_mon++;
        }
      break;
    case 10:                    // oct
      if (jfriday >= 32)
        {
          jfriday -= 31;
          date->tm_mon++;
        }
      break;
    case 11:                    // nov
      if (jfriday >= 31)
        {
          jfriday -= 30;
          date->tm_mon++;
        }
      break;
    case 12:                    // dec
      if (jfriday >= 32)
        {
          jfriday -= 31;
          date->tm_mon -= 12;
          date->tm_year++;
        }
      break;
    }

  date->tm_mday = jfriday;

}
