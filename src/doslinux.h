/* DOS-style file and date/time structures for use in Linux port of NLMake */
#include <sys/types.h>
#include <dirent.h>

/* _dos_findfirst structure */

struct find_t
{
  DIR *dir;
  struct dirent *dd;
  size_t size;
  unsigned wr_date;
  char *name;
  char path[80], fname[80];
};

// original DOS struct was:
// char reserved[21];
// char attrib;
// unsigned wr_time;
// unsigned wr_date;
// long size;
// char name[13];

unsigned _dos_findfirst (char *, unsigned, struct find_t *);
unsigned _dos_findnext (struct find_t *);

#ifdef __linux__
void _makepath (const char *path, char *drive, char *dir, char *file, char *ext);
void _splitpath (const char *path, char *drive, char *dir, char *file, char *ext);
#endif

#define _A_NORMAL 0

char *strupr (char *string);
