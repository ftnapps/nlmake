#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
//#include <mem.h>
#ifndef LINUX
#include <dos.h>
#else
#include "doslinux.h"
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif
#include "flags.h"
#include "logdef.h"

extern char FLAGCHKAUTO;
extern char *badflags;
extern char *caseflags;
extern char *goodflags;
extern char *dupeflags;
extern char *redunflags;
extern char *rplcdflags;
extern char IGNORCASE;


GFLAGS gflag[50];
static char *quicklst;
static char *caselst;
static char *redunlst;
static char *replacelst;
#define LISTSIZE 2500

// externs
extern void netmail_text (char *message);

// Prototype
short is_used (char *current, char *fptr, char *field);
short Ushift_bad (char *quickchk, char umode);
void splitflags (char *flags);
void loadquicklst (void);
void savequicklst (void);
void freequicklst (void);
void set_redundant (char *quickchk, char *pointer, char mode);
void is_redundant (void);
void delete_redundant (char *flags);
void poundflag_test (void);
void tflag_test (void);
void build_flags (char umode);


// valid flags database
char look_for_flag[60];

short flcnt = 0;

void
cre_flags_db (char *filename)
{
  char flagstr[512];
  FILE *flaglst;

  flaglst = fopen (filename, "rt");

#ifdef DOS
  quicklst = (char *) _fmalloc (LISTSIZE);
  redunlst = (char *) _fmalloc (LISTSIZE);
  replacelst = (char *) _fmalloc (LISTSIZE);
  caselst = (char *) _fmalloc (LISTSIZE);
#else
  quicklst = (char *) malloc (LISTSIZE);
  redunlst = (char *) malloc (LISTSIZE);
  replacelst = (char *) malloc (LISTSIZE);
  caselst = (char *) malloc (LISTSIZE);
#endif

  memset (quicklst, 0, sizeof (quicklst));
  memset (redunlst, 0, sizeof (redunlst));
  memset (replacelst, 0, sizeof (replacelst));
  *quicklst = ',';
  *redunlst = ',';
  *replacelst = ',';

  while (1)
    {
      memset (flagstr, 0, sizeof (flagstr));
      if (fgets (flagstr, 254, flaglst) == NULL)
	break;
      //printf("flgstr %d: %s\n",flcnt++,flagstr);
      flcnt++;
      if (flagstr[0] != ';' && flagstr[0] != 0)
	{
	  splitflags (flagstr);
	}
    }

  //      showdb(flagdb, flag_idx, "|");
  savequicklst ();

}

/* estrstr(*haystack,*needle) - find one string in another, allow wildcard */
/* added 4/7/2001 to accommodate flags like ,UMRVIA:* and ,URVIA:* in list */
char *
estrstr (char *haystack, char *needle)
{
  unsigned short i, j, len = strlen (needle);
  if (!len)
    return haystack;		/* if search string empty, return */

  for (i = 0; i < strlen (haystack); i++)	/* search string for target */
    if (haystack[i] == needle[0])	/* 1st chr must match - no wildcards */
      {
	/* check rest of pattern */
	for (j = 0;
	     j < len && haystack[i + j] && haystack[i + j] == needle[j]; j++);
	if (j == len || haystack[i + j] == '*')	/* if match up to first wildcard or */
	  return haystack + i;	/* if first len characters of str found */
      }

  // printf("warning - incorrect flag: %s\n",needle);
  return NULL;			/* if nothing found */
}

void
splitflags (char *flagstr)
{

  char *delims = { " ,\r\n" };
  char *fptr;
  char quickchk[80];

  fptr = strtok (flagstr, delims);	// either flags or uflags

  if (fptr == NULL)
    return;

  if (strnicmp (fptr, "FLAGS", 5) == 0)
    {
      // LOOP HERE
      while (fptr != NULL)
	{
	  fptr = strtok (NULL, delims);	// actual flags
	  if (fptr != NULL)
	    {
	      sprintf (quickchk, ",%s,", fptr);
	      if (strstr (quicklst, quickchk) != NULL)
		{		// log duplicate flag entry in db file
		  printf ("Ignoring duplicate flag <%s> on line %d", fptr,
			  flcnt);
		}
	      else		//
		{
		  if (strlen (fptr) <= 48)
		    {
		      strcat (quicklst, fptr);
		      strcat (quicklst, ",");
		    }
		}
	    }
	}
    }
  else if (strnicmp (fptr, "UFLAGS", 6) == 0)
    {
      // loop here
      while (fptr != NULL)
	{
	  fptr = strtok (NULL, delims);	// actual Uflags
	  if (fptr != NULL)
	    {
	      //     printf("flag :%s <%d>\n",look_for_flag,strlen(look_for_flag));
	      sprintf (quickchk, ",U%s,", fptr);
	      if (strstr (quicklst, quickchk) != NULL)
		{		// log duplicate flag entry in db file
		  printf ("Ignoring duplicate flag <%s> on line %d", fptr,
			  flcnt);
		}
	      else		//
		{
		  if (strlen (fptr) <= 48)
		    {
		      strcat (quicklst, "U");
		      strcat (quicklst, fptr);
		      strcat (quicklst, ",");
		    }
		}
	    }
	}
    }
  else if (strnicmp (fptr, "RFLAGS", 6) == 0)
    {
      fptr = strtok (NULL, delims);	// redundant flags
      if (fptr != NULL)
	{
	  sprintf (quickchk, ",%s,<", fptr);
	  if (strstr (redunlst, quickchk) != NULL)
	    {
	      printf ("Ignoring duplicate redundant flag <%s> on line %d",
		      fptr, flcnt);
	    }
	  else
	    {
	      // forced add
	      strcat (redunlst, fptr);
	      strcat (redunlst, ",<");
	      fptr = strtok (NULL, "<>");	// redundant flags
	      strcat (redunlst, fptr);
	      strcat (redunlst, ">,");
	    }
	}
    }
  else if (strnicmp (fptr, "OFLAGS", 6) == 0)
    {
      fptr = strtok (NULL, delims);	// redundant flags
      if (fptr != NULL)
	{
	  sprintf (quickchk, ",%s,<", fptr);
	  if (strstr (replacelst, quickchk) != NULL)
	    {
	      printf ("Ignoring duplicate replacement flag <%s> on line %d",
		      fptr, flcnt);
	      // logtext
	    }
	  else
	    {
	      // forced add
	      strcat (replacelst, fptr);
	      strcat (replacelst, ",<");
	      fptr = strtok (NULL, "<>");	// redundant flags
	      if (strlen (fptr) <= 49)
		{
		  strcat (replacelst, fptr);
		}
	      strcat (replacelst, ">,");
	    }
	}
    }

}

void
loadquicklst (void)
{
  FILE *qlfp;

#ifdef DOS
  quicklst = (char *) _fmalloc (LISTSIZE);
  redunlst = (char *) _fmalloc (LISTSIZE);
  replacelst = (char *) _fmalloc (LISTSIZE);
  caselst = (char *) _fmalloc (LISTSIZE);
#else
  quicklst = (char *) malloc (LISTSIZE);
  redunlst = (char *) malloc (LISTSIZE);
  replacelst = (char *) malloc (LISTSIZE);
  caselst = (char *) malloc (LISTSIZE);
#endif

  qlfp = fopen ("quick.lst", "rb");
  memset (quicklst, 0, sizeof (quicklst));
  memset (redunlst, 0, sizeof (redunlst));
  memset (replacelst, 0, sizeof (replacelst));
  memset (caselst, 0, sizeof (caselst));
  fread (quicklst, sizeof (char), LISTSIZE, qlfp);
  fread (redunlst, sizeof (char), LISTSIZE, qlfp);
  fread (replacelst, sizeof (char), LISTSIZE, qlfp);
  memcpy (caselst, quicklst, LISTSIZE);
  strupr (caselst);

  fclose (qlfp);

}

void
savequicklst (void)
{

  FILE *qlfp;
  qlfp = fopen ("quick.lst", "wb");
  strcat (quicklst, ",");
  fwrite (quicklst, sizeof (char), LISTSIZE, qlfp);
  fwrite (redunlst, sizeof (char), LISTSIZE, qlfp);
  fwrite (replacelst, sizeof (char), LISTSIZE, qlfp);
  fclose (qlfp);
  freequicklst ();
}

void
freequicklst (void)
{

#ifdef DOS
  _ffree (quicklst);
  _ffree (redunlst);
  _ffree (replacelst);
  _ffree (caselst);
#else
  free (quicklst);
  free (redunlst);
  free (replacelst);
  free (caselst);
#endif
}

short
checkflags (char *allflags)
{
  char localflags[255];
  char quickchk[255];
  char hue = 0;
  char umode = 0;		// 0 = off  1 = ,UREC   2 = ,U,REC  3 = DONE
  char *chptr;
  short luoffset = 0;
  char *delims = { ",\r\n" };
  char *fptr;

  // see if it works
  memset (badflags, 0, 10);
  memset (goodflags, 0, 10);
  memset (caseflags, 0, 10);
  memset (dupeflags, 0, 10);
  memset (redunflags, 0, 10);
  memset (rplcdflags, 0, 10);
  strcpy (localflags, allflags);
  //memset(gflag,0,sizeof(gflag));

  fptr = strtok (localflags, delims);

  while (fptr != NULL)
    {
      if (fptr != NULL)
	{
	  //printf("flgptr :%s \n",fptr);
	  if (*fptr == 'U' || *fptr == 'u')
	    {
	      hue = 'U';
	      if (strlen (fptr) == 1)
		{
		  fptr = strtok (NULL, delims);
		  look_for_flag[0] = hue;
		  look_for_flag[1] = 0;
		  strcat (look_for_flag, fptr);
		  if (umode == 0)
		    umode = 2;
		}
	      else
		{
		  strcpy (look_for_flag, fptr);
		  if (umode == 0)
		    umode = 1;
		}
	    }
	  else
	    {
	      look_for_flag[0] = hue;
	      look_for_flag[1] = 0;
	      strcat (look_for_flag, fptr);
	    }
	  //printf("lookup :%s \n",look_for_flag);
	  if (look_for_flag[0] == 'I' && look_for_flag[3] == ':')
	    look_for_flag[3] = 0;
	  if (look_for_flag[0] == '#')
	    poundflag_test ();
	  if (look_for_flag[0] == 'T' && strlen (look_for_flag) == 3)
	    tflag_test ();
	  if (look_for_flag[0] == 'G')
	    {
	      if (look_for_flag[1] != '*')
		{
		  look_for_flag[1] = '*';
		  look_for_flag[2] = 0;
		}
	      else
		{
		  look_for_flag[1] = 0;
		}
	    }
	  if (strnicmp (look_for_flag, "UPN", 3) == 0)
	    {
	      look_for_flag[3] = '*';
	      look_for_flag[4] = 0;
	    }
	  //printf("lookup :%s \n",look_for_flag);
	  sprintf (quickchk, ",%s,", look_for_flag);

	  if (estrstr (quicklst, quickchk) != NULL)
	    {
	      sprintf (quickchk, ",%s,<", look_for_flag);
	      set_redundant (quickchk, fptr, umode);
	    }
	  else			// check if it's a replaced flag, A mis-labeled U flag or revers
	  if (strstr (caselst, strupr (quickchk)) == NULL)
	    {
	      sprintf (quickchk, ",%s,<", look_for_flag);
	      if (strstr (replacelst, quickchk) != NULL)
		{
		  chptr = strstr (replacelst, quickchk);
		  chptr += strlen (quickchk);
		  memset (quickchk, 0, sizeof (quickchk));
		  memccpy (quickchk, chptr, '>', 80);
		  quickchk[strlen (quickchk) - 1] = 0;
		  strcat (rplcdflags, ",");
		  strcat (rplcdflags, look_for_flag);
		  strcat (rplcdflags, " -> ");
		  strcat (rplcdflags, quickchk);
		  set_redundant (quickchk, quickchk, umode);
		}
	      else
		{
		  // test to see if it is mis-used
		  // A UFlag as Flag or Flag as UFlag
		  if (Ushift_bad (quickchk, umode) == 0)
		    {
		      if (umode == 0 || *fptr == 'U')
			strcat (badflags, ",");
		      else
			strcat (badflags, ",U");
		      strcat (badflags, fptr);
		    }
		  else
		    {
		      strcat (rplcdflags, ",");
		      strcat (rplcdflags, look_for_flag);
		      strcat (rplcdflags, " -> ");
		      strcat (rplcdflags, quickchk);
		      if (quickchk[0] == 'U')
			umode = 2;
		      set_redundant (quickchk, quickchk, umode);
		    }
		}
	    }
	  else
	    {
	      chptr = strstr (caselst, strupr (quickchk));
	      luoffset = chptr - caselst;
	      chptr = quicklst;
	      chptr += (luoffset + 1);
	      memset (quickchk, 0, sizeof (quickchk));
	      memccpy (quickchk, chptr, ',', 50);
	      quickchk[strlen (quickchk) - 1] = 0;
	      if (umode == 0 || *fptr == 'U')
		strcat (caseflags, ",");
	      else
		strcat (caseflags, ",U");
	      strcat (caseflags, fptr);
	      set_redundant (quickchk, quickchk, umode);
	    }

	}			// if null
      fptr = strtok (NULL, delims);	// actual Uflags
    }				// while

  is_redundant ();

  build_flags (umode);


//                printf("Good  Flags : %s\n",goodflags);
//                printf("Bad   Flags : %s\n",badflags);
//                printf("Case  Flags : %s\n",caseflags);
//                printf("Dupe  Flags : %s\n",dupeflags);
//                printf("Redun Flags : %s\n",redunflags);

  if (IGNORCASE == 'Y')
    *caseflags = 0;

  if (*badflags == 0 && *caseflags == 0 &&
      *dupeflags == 0 && *redunflags == 0 && *rplcdflags == 0)
    return (0);


  return (1);
}


short
is_used (char *current, char *fptr, char *field)
{
  short i;
  short retvalue = 0;

  //   printf("testing Flag : %s\n",current);


  for (i = 0; i <= 100; i++)
    {
      //printf("3 Flag <%d> : %s\n",i,gflag[i].flag);
      if (strcmp (gflag[i].flag, current) == 0)
	{
	  retvalue = 1;
	  break;
	}
      if (gflag[i].flag[0] == 0)
	{
	  retvalue = 0;
	  gflag[i].ok = 0;
	  if (*field == '*')
	    gflag[i].dependnc = 0;
	  else
	    gflag[i].dependnc = 1;

	  if (current[0] == 'G' || current[0] == '#')
	    {
	      strcpy (gflag[i].flag, fptr);
	    }
	  else if (current[0] == 'T' && strlen (fptr) == 3)
	    {
	      strcpy (gflag[i].flag, fptr);
	    }
	  else if (current[0] == 'I' && strlen (fptr) == 3)
	    {
	      strcpy (gflag[i].flag, current);
	    }
	  else if (current[0] == 'I' && strlen (fptr) > 3)
	    {
	      strcpy (gflag[i].flag, fptr);
	      memcpy (gflag[i].flag, current, strlen (current));
	    }
	  else if (strnicmp (current, "UPN", 3) == 0)
	    {
	      if (*fptr != 'U')
		{
		  strcpy (gflag[i].flag, "U");
		  strcat (gflag[i].flag, fptr);
		}
	      else
		strcat (gflag[i].flag, fptr);
	    }
	  else
	    {
	      strcpy (gflag[i].flag, current);
	    }
	  break;
	}
    }

  return (retvalue);
}

void
set_redundant (char *quickchk, char *fptr, char umode)
{
  char *chptr;

  if (strstr (redunlst, quickchk) == NULL)
    {
      if (*quickchk == ',')
	strcpy (quickchk, quickchk + 1);
      chptr = strchr (quickchk, ',');
      if (chptr != NULL)
	*chptr = 0;

      if (is_used (quickchk, fptr, "*") == 1)	// need fix for redund
	{
	  if (umode == 0 || *fptr == 'U')
	    strcat (dupeflags, ",");
	  else
	    strcat (dupeflags, ",U");
	  strcat (dupeflags, fptr);
	}
    }
  else
    {
      if (*quickchk == ',')
	strcpy (quickchk, quickchk + 1);
      chptr = strchr (quickchk, ',');
      if (chptr != NULL)
	*chptr = 0;
      if (is_used (quickchk, fptr, "!") == 1)	// need fix for redund
	{
	  if (umode == 0 || *fptr == 'U')
	    strcat (dupeflags, ",");
	  else
	    strcat (dupeflags, ",U");
	  strcat (dupeflags, fptr);
	}
    }

}

void
is_redundant (void)
{
  short i = 0;
  char quickchk[80];
  char *chptr;

  while (gflag[i].flag[0] != 0)
    {
      if (gflag[i].dependnc == 1)
	{
	  //strcpy(look_for_flag,gflag[i].flag);
	  sprintf (quickchk, ",%s,<", gflag[i].flag);
	  if (strstr (redunlst, quickchk) != NULL)
	    {
	      chptr = strstr (redunlst, quickchk);
	      chptr += strlen (quickchk);
	      memset (quickchk, 0, sizeof (quickchk));
	      memccpy (quickchk, chptr, '>', 80);
	      quickchk[strlen (quickchk) - 1] = 0;
	      delete_redundant (quickchk);
	    }
	}
      i++;
    }
}

void
delete_redundant (char *field)
{
  short i = 0;
  char *delims = { ",\r\n" };
  char *fptr;

  fptr = strtok (field, delims);


  while (fptr != NULL)
    {
      while (gflag[i].flag[0] != 0)
	{
	  if (stricmp (gflag[i].flag, fptr) == 0)
	    gflag[i].ok = 1;
	  //    printf("compare %s with %s\n",gflag[i].flag,fptr);
	  i++;
	}
      i = 0;
      fptr = strtok (NULL, delims);
      if (fptr == NULL)
	break;
    }

}

void
tflag_test (void)
{
  char cset[] = { "abcdefghijklmnopqrstuvwxABCDEFGHIJKLMNOPQRSTUVWX" };

  if (look_for_flag[1] == '*')
    {
      look_for_flag[1] = 0;
      return;
    }

  if (strspn (look_for_flag, cset) == strlen (look_for_flag))
    {
      look_for_flag[1] = 'y';
      look_for_flag[2] = 'z';
      look_for_flag[3] = 0;
    }

}

void
poundflag_test (void)
{
  char *pndptr;
  char quickchk[100];

  pndptr = strchr (look_for_flag, '#');

  if (pndptr == NULL || strlen (look_for_flag) == 3)
    return;
  if (look_for_flag[1] == '?')
    {
      look_for_flag[1] = 0;
      return;
    }

  while (pndptr != NULL)
    {
      sprintf (quickchk, ",%3.3s,", pndptr);
      //printf("checking %s\n",quickchk);
      if (strstr (quicklst, quickchk) == NULL)
	break;
      pndptr = strchr (pndptr + 1, '#');
      if (pndptr == NULL)
	{
	  look_for_flag[1] = '?';
	  look_for_flag[2] = 0;
	}
    }

}

short
Ushift_bad (char *quickchk, char umode)
{

  if (look_for_flag[0] != 'U')	// uflag as flag
    {
      sprintf (quickchk, ",U%s,", look_for_flag);
      if (strstr (quicklst, quickchk) != NULL)
	{
	  sprintf (quickchk, "U%s", look_for_flag);
	  return (1);
	}
    }
  else				// flag as uflag
    {
      sprintf (quickchk, ",%s,", look_for_flag + 1);
      if (strstr (quicklst, quickchk) != NULL)
	{
	  sprintf (quickchk, "%s", look_for_flag + 1);
	  return (1);
	}
    }

  return (0);
}

void
build_flags (char umode)
{
  short i = 0;

  // build flags
  while (gflag[i].flag[0] != 0)
    {
      if (gflag[i].ok == 0 && gflag[i].flag[0] != 'U')
	{
	  gflag[i].ok = 0;
	  gflag[i].dependnc = 0;
	  strcat (goodflags, ",");
	  strcat (goodflags, gflag[i].flag);
	  //     gflag[i].flag[0] = 0;
	}
      else if (gflag[i].ok != 0 && gflag[i].flag[0] != 'U')
	{
	  gflag[i].ok = 0;
	  gflag[i].dependnc = 0;
	  strcat (redunflags, ",");
	  strcat (redunflags, gflag[i].flag);
	  //       gflag[i].flag[0] = 0;
	}
      i++;
    }

  // build Uflags
  i = 0;			// reset search

  while (gflag[i].flag[0] != 0)
    {
      if (gflag[i].ok == 0 && gflag[i].flag[0] == 'U')
	{
	  gflag[i].ok = 0;
	  gflag[i].dependnc = 0;
	  if (umode == 1)
	    {
	      strcat (goodflags, ",U");
	      umode = 3;
	    }
	  else if (umode == 2)
	    {
	      strcat (goodflags, ",U,");
	      umode = 3;
	    }
	  else
	    strcat (goodflags, ",");
	  strcat (goodflags, gflag[i].flag + 1);
	  //                      gflag[i].flag[0] = 0;
	}
      else if (gflag[i].ok != 0 && gflag[i].flag[0] == 'U')
	{
	  gflag[i].ok = 0;
	  gflag[i].dependnc = 0;
	  strcat (redunflags, ",");
	  strcat (redunflags, gflag[i].flag);
	  //       gflag[i].flag[0] = 0;
	}
      gflag[i].flag[0] = 0;
      i++;
    }
}
