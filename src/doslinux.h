/* DOS-style file and date/time structures for use in Linux port of NLMake */
#include <dirent.h>

/* _dos_getdate/_dossetdate and _dos_gettime/_dos_settime structures */

struct dosdate_t {
    unsigned char day;      /* 1-31 */
    unsigned char month;        /* 1-12 */
    unsigned int year;      /* 1980-2099 */
    unsigned char dayofweek;    /* 0-6, 0=Sunday */
    };

struct dostime_t {
    unsigned char hour; /* 0-23 */
    unsigned char minute;   /* 0-59 */
    unsigned char second;   /* 0-59 */
    unsigned char hsecond;  /* 0-99 */
    };

/* _dos_findfirst structure */

struct find_t
{
 DIR *dir;
 struct dirent *dd;
 size_t size;
 unsigned wr_date;
 char *name;
 char path[80],fname[80];
};

// original DOS struct was:
// char reserved[21];
// char attrib;
// unsigned wr_time;
// unsigned wr_date;
// long size;
// char name[13];

void _dos_getdate(struct dosdate_t *);
void _dos_gettime(struct dostime_t *);
unsigned _dos_findfirst(char *, unsigned, struct find_t *);
unsigned _dos_findnext(struct find_t *);
unsigned _dos_getftime(int, unsigned short *, unsigned short *);
unsigned _dos_open(const char *, unsigned, int *);
unsigned _dos_close(int);

void _makepath(const char *path, char *drive, char *dir, char *file, char *ext);
void _splitpath(const char *path, char *drive, char *dir, char *file, char *ext);

char *strupr(char * string);
char *itoa( int value, char *string, int radix );

#define EZERO 0
#define _A_NORMAL 0

/* map spawnlp() calls to UNIX-style system() calls*/
#define P_WAIT 0
