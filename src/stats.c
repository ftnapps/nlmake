#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef LINUX
#include "doslinux.h"
#else
#include <dos.h>
#endif
#include "logdef.h"

// externs
extern void logtext (char *string, short indicator, short dateon);


FILE *statptr;

void
stat_text (char *info)
{
  fprintf (statptr, "%s\n", info);
}

short
open_stat_file (char *filename)
{
  statptr = fopen (filename, "wt");

  if (statptr == NULL)
    {
      logtext ("Could not open stats file !", 1, YES);
      return (1);
    }
  return (0);

}

void
close_stat_file (void)
{
  fclose (statptr);
}
