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

// Get day_of_year for next publish day
int
getJDate (void)
{
  time_t t;
  struct tm *tm;
  int pub_wday, jfriday;

  // Get day_of_week for publish
  for (pub_wday = 0; pub_wday <= 6; pub_wday++)
    {
      if (stricmp (Publish_day, WeekDays[pub_wday].String) == 0)
        break;
    }

  time (&t);
  tm = localtime (&t);

  jfriday = tm->tm_yday + pub_wday - tm->tm_wday + 1;
  if (pub_wday < tm->tm_wday)
    jfriday += 7;

  return (jfriday);
}


/* Get text string describing next publish day */
void get_proc_date (char *buf, int maxlen)
{
  time_t t;
  struct tm *tm;
  int pub_wday, days;

  // Get day_of_week for publish
  for (pub_wday = 0; pub_wday <= 6; pub_wday++)
    {
      if (stricmp (Publish_day, WeekDays[pub_wday].String) == 0)
        break;
    }

  time (&t);
  tm = localtime (&t);

  days = pub_wday - tm->tm_wday;
  if (days < 0)
    days += 7;
  t += days * 24 * 3600;
  tm = localtime (&t);

  strftime (buf, maxlen, "%A, %B %d, %Y -- Day number %j", tm);
}
