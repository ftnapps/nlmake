char *errorlevelstr[] =
{
        "no errors encountered",
        "no fatal errors encuntered",
        "one or more fatal errors encountered"
};


char *errnostr[] =
{
        "No error",
        "No such file or directory",
        "Arg list too big",
        "Exec format error",
        "Bad file number",
        "Not enough memory",
        "Permission denied",
        "File exists",
        "Cross-device link",
        "Invalid argument",
        "File table overflow",
        "Too many open files",
        "No space left on device",
        "Argument too large",
        "Result too large",
        "Resource deadlock would occur",
        "Unknown",
        "Resource deadlock would occur",
        "Child does not exist",
        "Resource unavailable, try again",
        "Device or resource busy",
        "File too large",
        "I/O error",
        "Is a directory",
        "Not a directory",
        "Too many links",
        "Block device required",
        "Not a character device",
        "No such device or address",
        "Not owner",
        "Broken pipe",
        "Read-only file system",
        "Illegal seek",
        "No such process",
        "Text file busy",
        "Bad address",
        "Name too long",
        "No such device",
        "No locks available in system",
        "Unknown system call",
        "Directory not empty",
        "Unknown"
};

LINEPRMS WeekDays[] =
{
        {"Sunday", "SUN", 0},
        {"Monday", "MON", 1},
        {"Tuesday", "TUE", 2},
        {"Wednesday", "WED", 3},
        {"Thursday", "THU", 4},
        {"Friday", "FRI", 5},
        {"Saturday", "SAT", 6},
        {NULL, NULL, 99}
};

LINEPRMS Months[] =
{
        {"January",      "Jan", 0},
        {"January",      "Jan", 1},
        {"February",     "Feb", 2},
        {"March",        "Mar", 3},
        {"April",        "Apr", 4},
        {"May",          "May", 5},
        {"June",         "Jun", 6},
        {"July",         "Jul", 7},
        {"August",       "Aug", 8},
        {"September",    "Sep", 9},
        {"October",      "Oct", 10},
        {"November",     "Nov", 11},
        {"December",     "Dec", 12},
        {NULL, NULL, 99}
};

#ifdef __linux__
LINEPRMS LinePrms[] =
{
        {"PROCESS", "-P", 1},
        {"TEST", "-T", 2},
        {"MERGE", "-M", 3},
        {"NAME", "-N", 4},
        {"DISPLAY", "-D", 5},
        {"FORCE", "-F", 6},
        {"HELP", "-?", 7},
        {"HELP", "-H", 7},
        {"COMPILE", "-COM", 8},
        {"TESTALL", "-ALL", 9},
        {"STATS", "-STA", 10},
        {"LAST", "-LAS", 11},
        {"ERRORS", "-ERR", 12},
        {"LATE", "-LAT", 13},
        {NULL, NULL, 99}
};
#else
LINEPRMS LinePrms[] =
{
        {"PROCESS", "/P", 1},
        {"TEST", "/T", 2},
        {"MERGE", "/M", 3},
        {"NAME", "/N", 4},
        {"DISPLAY", "/D", 5},
        {"FORCE", "/F", 6},
        {"HELP", "/?", 7},
        {"HELP", "/H", 7},
        {"COMPILE", "/COM", 8},
        {"TESTALL", "/ALL", 9},
        {"STATS", "/STA", 10},
        {"LAST", "/LAS", 11},
        {"ERRORS", "/ERR", 12},
        {"LATE", "/LAT", 13},
        {NULL, NULL, 99}
};
#endif

LINEPRMS CfilePrms[] =
{
        {"MAKE", "MAK", 1},
        {"NAME", "NAM", 2},
        {"PUBLISH", "PUB", 3},
        {"PROCESS", "PROC", 4},
        {"MERGE", "MER", 5},
        {"PRIVATE", "PRI", 6},
        {"MINPHONE", "MIN", 7},
        {"BAUDRATE", "BAU", 8},
        {"MASTER", "MAS", 9},
        {"UPLOADS", "UPL", 10},
        {"MAILFILES", "MAI", 11},
        {"UPDATE", "UPD", 12},
        {"BADFILES", "BAD", 13},
        {"OUTFILE", "OUTF", 14},
        {"OUTPATH", "OUTP", 15},
        {"THRESHOLD", "THR", 16},
        {"ARC", "ARC", 17},
        {"OUTDIFF", "OUTD", 18},
        {"CLEANUP", "CLE", 19},
        {"NETADDRESS", "NETA", 20},
        {"MESSAGES", "MES", 21},
        {"SUBMIT", "SUB", 22},
        {"NOTIFY", "NOT", 23},
        {"COPYRIGHT", "COP", 24},
        {"PROLOG", "PROL", 25},
        {"EPILOG", "EPI", 26},
        {"COMMENTS", "COMM", 27},
        {"DATA", "DAT", 28},
        {"FILES", "FIL", 29},
        {"LOGLEVEL", "LOG", 30},
        {"COMPRESS", "COMP", 31},
        {"FLAGS", "FLAGS", 32},
        {"AUTO-FLAG", "AUTO-FLAG", 33},
        {"WARNINGS", "WARN", 34},
        {"PVTLEVEL", "PVTL", 35},
        {"MAXAGE", "MAX", 36},
        {NULL, NULL, 99}
};

LINEPRMS MFlagstype[] =
{
        {"CRASH", "CRA", 1},
        {"HOLD", "HOL", 2},
        {"INTL", "INT", 3},
        {"NORMAL", "NOR", 4},
        {NULL, NULL, 99}
};

LINEPRMS NotifyType[] =
{
        {"RECEIPT", "REC", 1},
        {"ERRORS", "ERR", 2},
        {"SELF", "SEL", 3},
        {NULL, NULL, 99}
};

LINEPRMS SegmentType[] =
{
        {"Node", "NOD", 0},
        {"Hub", "HUB", 1},
        {"Host", "HOS", 2},
        {"Network", "NET", 3},
        {"Region", "REG", 4},
        {"Zone", "ZON", 5},
        {"Composite", "COM", 6},
        {"Unknown", "COM", 7},
        {NULL, NULL, 99}
};

LINEPRMS FieldOneType[] =
{
        {"Node", ",", 0},
        {"Private", "PVT", 1},
        {"Down", "DOW", 3},
        {"Hold", "HOL", 4},
        {"Hub", "HUB", 5},
        {"Host", "HOS", 6},
        {"Region", "REG", 6},
        {"Zone", "ZON", 7},
        {NULL, NULL, 99}
};

LINEPRMS PVTDisposition[] =
{
        {"NONE", "NON", 0},
        {"HUB", "HUB", 1},
        {"HOST", "HOS", 3},
        {"HOST", "HOS", 3},     // solves net/host concurency
        {"REGION", "REG", 4},
        {"ZONE", "ZON", 5},
        {"OK", "OK", 6},
        {NULL, NULL, 99}
};

char *compressor_name[] =
{
        "ARC",
        "ZIP",
        "ARJ",
        "RAR",
        "OTHER1",
        "OTHER2",
        NULL
};

char *compressor_verbs[] =
{
        "ADD",                    // 0
        "ARCHIVER",               // 1
        "EXTENSION",              // 2
        "EXTRACT",                // 3
        "IDENT",                  // 4
        "END",                    // 5
        "FILENAME",               // 6
        NULL
};

