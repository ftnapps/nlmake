; Sample NLMake control file
; Uncomment only those line which apply to your situation and then:
; Be sure to check path lines and file names before running
;
; This file is in several parts: Part one is the GLOBAL section where you define
; file names and other common settings.
;
; Part two contains optional settings which you may or may not choose to use.
;
; Part three contains the settings for a small network, a larger network, a
; region, a zone, and a "composite". All sections are commented out so you can
; simply remove the semi-colons from the settings which are closest to your
; situation.

;
; PART ONE:  Global settings (used for all types of operations)
;===========================================

; Files and Paths - change these names to suit your system
;===========================================
;
; Where master files reside (default - current)
MASTER      C:\NLMAKE\MASTER

; Where to save received files 'till processing
UPDATE      C:\NLMAKE\UPDATE

; Where to find new incoming segments
MAILFILES   C:\NLMAKE\INBOUND

; Where your BBS places uploaded files if your Hub calls to deliver a segment
UPLOADS     C:\NLMAKE\UPLOAD

; Optional for files with fatal errors. If this line is commented out, bad
; files will be erased.
BADFILES    C:\NLMAKE\BADFILES

; Alternate outbound directory. If not used, the newly generated segment
; will be in the default directory.
OUTPATH     C:\NLMAKE\MASTER

; Path name to mail server's network mail area
MESSAGES    C:\MSGS

; Log Level -- 0 (least) to 5 (very wordy)
LOG 5

; Outgoing Compression method (the inbound is auto detected)
COMPRESS ARC

; The ARC keyword has been left here for compatibility with Makenl, but is
; ignored. The COMPRESS line now takes precedence if the compress statement
; is present. In other words, this line can be deleted safely, but if NLMAKE
; sees it nothing will happen.
ARC 5

; Your network address here (this is the "from" address when your file is
; sent upstream
NETADDRESS 1:99/0 <Sysop Name>

; Where you send updates, CRASH or HOLD optional
SUBMIT 1:123/456 <Sysop Name> Crash

; Who to notify, why, and how
;=======================================

; Notify Self of errors  (Modified with Crash/Hold/Intl)
NOTIFY      SELF

; Notify Submitters of errors found in segment (if any)
NOTIFY      ERRORS

; Notify Submitters of receipt of segment
NOTIFY      RECEIPT

; PART TWO: OPTIONAL Section
; Keywords in this section are entirely optional. Comment them out (precede
; each keyword with a semi-colon) if you don't want the keyword to be active.

; Dealing with flags
;=======================================
; Check Flags and log errors. By default, FLAGS will check for the
; correct case. Add NOCASE to turn off case checking, then FLAGS will still
; suggest the correct case but won't report it as an error.

FLAGS NOCASE

; Auto Correct flags in output segment --
; CAUTION: NO warning is sent back to the sender, so they won't know what
; happened!!
;AUTO-FLAG NOCASE ;

; AGE of incoming segments
; (in days) commandline /LATE sends a message
; to your hubs if the segment is older than this
MAXAGE 30

; Merges new output with the filename shown here to give FileName.999 for
; temporary use
MERGE       C:\NLMAKE\FILENAME

; Tells NLMake to write the name of each input file to the named comments
; file, followed by any comment statements that appear in that input file.
COMMENTS  COMMENTS.TXT

; PROCESSING
;==============================

; Process day (defaults to Friday)
PROCESS FRIDAY

; Name of your output file (your uplink *C will usually tell you what to use)
OUTFILE ODDNAME

; The next 3 filenames may be any name you wish. If you already have those
; names for your files, you do NOT need to have these statements here, as
; nlmake will include the files if they exist. These three lines are simply
; so you can change the file names if you wish.

; includes cpyright.txt contents at the top of the file (see sample)
COPYRIGHT   CPYRIGHT.TXT

; includes prolog.txt contents after the Copyright text (see sample)
PROLOG      PROLOG.TXT

; includes epilog.txt contents after the end of the file (see sample)
EPILOG      EPILOG.TXT

; Delete old list and diff files
CLEANUP

; WARNINGS will generate warning messages for duplicate node numbers and/or
; phone numbers, but will NOT remove the offending entry.
; NOTE that when this keyword is commented out (default for MAKENL), the
; offending entry will be removed from the output file.
;WARNINGS

; Minimum number of sections required to be present in a node's phone number,
; separated by hyphens (e.g. 4 = 1-800-555-1212)
MINPHONE    4

; You can define as many as you can fit in 200 characters. Do not use any other
; rates than these without instructions from your ZC
BAUDRATE    300,1200,2400,4800,9600,14400,16800,19200,28800,33600

; The first value is the maximum size before archiving of the segment will
; occur. 0 therefore means "always archive". The second value is the maximum
; size before archiving of the diff will occur. -1 therefore means "never send a
; diff".
THRESHOLD  0 -1

; PART THREE: Settings specific to your situation
; ======================================
; Type 1: The Small Network
; Expects to find all data either in this file or another. No segments are being
; received from anyone else -- you're doing it all!

; Define type of build and number
;MAKE NET 99
; Optional: will look in file MYSEG for your data instead of looking for it
; in this file after the word "data"
;MAKE NET 99 MYSEG

; DATA
; All your NET data goes here, as well as your /0 listing info.
; Embedded spaces (after the commas) are ok. There can be no control
; statements after the word "DATA".
;
;NET,99,Somewhere,Jane_Sysop,1-800-555-1212,9600,XA,V34,V32b,V42b,VFC,V32T,CM
;,1,Dunno_Where,Joe_Flunky,1-800-999-9191,9600,XA,V34,V32b,V42b,VFC,V32T,CM,U,REC
;,4,Why_Not?,Jane_Flunky,1-800-999-9192,9600,XA,V34,V32b,V42b,VFC,V32T,CM
;,91,Blue_Note,Flunky_Me,1-800-999-9193,9600,XA,V34,V32b,V42b,VFC,V32T,CM

; ==== END Of Type 1 ===========

; Type 2: The Larger Network
; Expects to find data both in this file and in net segments received from
; others (hubs) in your net

; Defines type of build and number
; MAKE NET 99

; Optional: will look in file MYSEG for your data instead
; of looking for it in this file after the word "data"
; MAKE NET 99 MYSEG

; DATA
; Your NET independent data goes here, as well as your /0 listing info.
; Embedded spaces (after the commas) are ok. There can be no control
; statements after the word "DATA" except for the word "FILES".
;
;Net,99,Somewhere,Jane_Sysop,1-800-555-1212,9600,XA,V34,V32b,V42b,VFC,V32T,CM
;,1,I_Dunno,Joe_Flunky,1-800-999-9191,9600,XA,V34,V32b,V42b,VFC,V32T,CM,U,NEC
;
; The keyword FILES defines the name(s) of hub segments being sent to you for
; inclusion in your net segment.

;FILES
;  Type of Build   Number  Segment Name   Address to Notify   <Name >
;                                                              (optional)
; ===============  ======  ============   =================   =============
;    Hub             1        EASTHUB        99/1              <John Bill>
;    Hub             3        WESTHUB        99/3              <Mary Jane>
;    Hub             99       NORTHHUB       99/99             <Weird One>
;    Hub             1099     NONHUB         99/1099           <Not Me!>

; ==== END Of Type 2 ===========

; Type 3: The Region
; Expects to find data both in this file and in net segments received from
; NCs in your Region

; Defines type of build and number
;MAKE REGION 99

; Optional: will look in file MYSEG for your data instead of looking for it in
; this file after the word "data"
;MAKE REGION 99 MYSEG

; DATA
; Your regional independent data goes here, as well as your /0 listing info.
; Embedded spaces (after the commas) are ok. There can be no control
; statements after the word "DATA" except for the word "FILES".
;
;Region,99,Somewhere,Jane_Sysop,1-800-555-1212,9600,XA,V34,V32b,V42b,VFC,V32T,CM
;,1,Where?,Joe_Flunky,1-800-999-9191,9600,XA,V34,V32b,V42b,VFC,V32T,CM,U,REC
;
; The keyword FILES defines the name(s) of net segments being sent to you for
; inclusion in your region segment.

;FILES
;  Type of Build   Number  Segment Name   Address to Notify   <Name >
;                                                              (optional)
; ===============  ======  ============   =================   =============
;    Net             1105     NET105         1105/0            <John Bill>
;    Net             3550     3550AB         3550/0            <Mary Jane>
;    Net             3553     3553XY         3553/0            <Weird One>
;    Net             3599     MYNET9         3599/0            <Not Me!>

; ==== END Of Type 3 ===========

; Type 4: The Zone
; Expects to find data both in this file and in Region segments received from
; RCs in your Zone

; Defines type of build and number
;MAKE ZONE 99

; Optional: will look in file MYSEG for your data instead of looking for it in
; this file after the word "data"
;MAKE ZONE 99 MYSEG

; DATA
; Your ZONE independent data goes here, as well as your /0 listing info.
; Embedded spaces (after the commas) are ok. There can be no control
; statements after the word "DATA" except for the word "FILES".
;
;ZONE,99,Somewhere,Jane_Sysop,1-800-555-1212,9600,XA,V34,V32b,V42b,VFC,V32T,CM
;,1,Where?,Joe_Flunky,1-800-999-9191,9600,XA,V34,V32b,V42b,VFC,V32T,CM,U,REC
;
;FILES
;  Type of Build   Number  Segment Name   Address to Notify   <Name >
;                                                               (optional)
; ===============  ======  ============   =================   =============
;    REGION         115      Jonny          115/0               <John Bill>
;    REGION         3050     Mary           3050/0              <Mary Jane>
;    REGION         3053     Weird1         3053/0              <Weird One>
;    REGION         3059     OHNO           3099/0              <Not Me!>
;
; ==== END Of Type 4 ===========

; Type 5: The Composite (used to merge zones to produce a world-wide nodelist)

; Defines type of build (there must NOT be a number)
;MAKE Composite
; Your network name - Max 15 Characters
;NAME FidoNet

; Note that you may not need to submit a file upstream, therefore you can
; comment out the SUBMIT statement in the Global section
; There should be no data statement as all data is retrieved from FILES
; There can be no further control statements after this except for the
; word "FILES".
;
;FILES
;  Type of Build   Number  Segment Name   Address to Notify   <Name >
;                                           Optional           (optional)
; ===============  ======  ============   =================   =============
;   ZONE            1        ZONE1          1:115/5            <John Bill>
;   ZONE            2        ZONE2          2:123/4            <Johann>
;   ZONE            3        ZONE3          3:456/7            <Janus>
;   REGION          17       REGION17
;
; will build a composite nodelist containing Zone 1, 2, and 3's primary segments
; and REGION17
