typedef struct LINEPRMS
{
	char *String;
	char *Single;
	short Type;
} LINEPRMS;

typedef struct MSG
{
	char active,
		  Crash,
		  Hold,
		  Intl,
		  Normal;
} MSG;

typedef struct THRESHOLD
{
	long  arc_size,
		  diff_size;
} THRESHOLD;

typedef struct SEGFILE
{
	char FileName[13];
	short SegmentType;
	short Zone,
		 Net,
		 Node;
	char AltNotify[15],
	     NameNotify[73];
} SEGFILE;


typedef struct LOGMSGS
{
	char *String;
	short loglevel,
		 Type;
} LOGMSGS;

#define MAXFILELEN 14

