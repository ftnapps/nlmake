; This is the FLAGS.CTL, the flag check control file.
;
; Regular flags are entered with the token FLAGS (case insensitive).
;
; User flags are entered with the token UFLAGS (case insensitive)
; BUT MUST NOT START WITH A "U" Example UREC would be just REC.
; You may use as many tokens (FLAG or UFLAG) as you want.
;
; Lines that begin with ";" are considered comment lines and can appear
; anywhere in this file
;
; ===============================================
;   A: OPERATING CONDITION FLAGS:
;
;   These flags define the online times and point status.
;   CM = Continuous Mail: can receive mail calls 24 hours a day
;        The absence of CM means that only Zone Mail Hour is observed
;   MO = Mail Only: no publicly available BBS available
;   LO = Listed Only: Only accepts mail calls from nodes in the nodelist (i.e.,
;                     no points)
FLAGS  CM,MO,LO
;
; ===============================================
;  B. MODEM FLAGS:
;    The following flags define modem protocols supported. For details, refer
;    to the nodelist and/or your NC, RC, or ZC.
;
FLAGS  V21,V22,V29,V32,V32b,V32T,V33,V34,HST,H14,H16,H96,MAX,PEP
FLAGS  CSP,ZYX,VFC,Z19,V90C,V90S,X2C,X2S
;
;     The following flags define type of error correction available. A
;     separate error correction flag should not be used when the error
;     correction type can be determined by the modem flag. For instance
;     a modem flag of HST implies MNP.
FLAGS  MNP,V42
;
; ===============================================
;  C: COMPRESSION FLAGS:
;
FLAGS  MN,V42b
;
;  D: FILE/UPDATE REQUEST FLAGS:
;    The following table is from the nodelist:
;  |--------------------------------------------------|
;  |      |         Bark        |        WaZOO        |
;  |      |---------------------|---------------------|
;  |      |   File   |  Update  |   File   |  Update  |
;  | Flag | Requests | Requests | Requests | Requests |
;  |------|----------|----------|----------|----------|
;  | XA   |    Yes   |    Yes   |    Yes   |    Yes   |
;  | XB   |    Yes   |    Yes   |    Yes   |    No    |
;  | XC   |    Yes   |    No    |    Yes   |    Yes   |
;  | XP   |    Yes   |    Yes   |    No    |    No    |
;  | XR   |    Yes   |    No    |    Yes   |    No    |
;  | XW   |    No    |    No    |    Yes   |    No    |
;  | XX   |    No    |    No    |    Yes   |    Yes   |
;  |--------------------------------------------------|
;
;
;  The following software is qualified to
;  use the appropriate file request flag
;  according to information provided by
;  developers:
;
;  |-----------------------------------|
;  | Flag      Software Package        |
;  |-----------------------------------|
;  |  XA  Frontdoor   1.99b and lower  |
;  |      Frontdoor   2.01  and higher |
;  |      Dutchie     2.90c            |
;  |      Binkleyterm 2.1   and higher |
;  |      D'Bridge    1.2   and lower  |
;  |      TIMS                         |
;  |-----------------------------------|
;  |  XB  Binkleyterm 2.0              |
;  |      Dutchie     2.90b            |
;  |-----------------------------------|
;  |  XC  Opus        1.1              |
;  |-----------------------------------|
;  |  XP  Seadog                       |
;  |-----------------------------------|
;  |  XR  Opus        1.03             |
;  |      Platinum Xpress              |
;  |-----------------------------------|
;  |  XW  Fido        12N   and higher |
;  |      Tabby                        |
;  |-----------------------------------|
;  |  XX  D'Bridge    1.30  and higher |
;  |      Frontdoor   1.99c/2.00       |
;  |      InterMail   2.01             |
;  |      McMail      1.00             |
;  |-----------------------------------|
;  |  None       QMM                   |
;  |-----------------------------------|
;
;
;
FLAGS  XA,XB,XC,XP,XR,XW,XX
;
; ===============================================
;  E: GATEWAY FLAG:
;  The * denotes a non-pattern variable length tail
FLAGS G*
;
; ===============================================
;  F: MAIL PERIOD FLAGS:
; #? allows combinations of the following
FLAGS  #01,#02,#08,#09,#18,#20,#?
;
; Example #01#02 is valid. #01#21 would be invalid since #21 is not listed
;
; ===============================================
;  G: ISDN CAPABILTY FLAGS:
FLAGS  ISDN,X75,V120H,V120L,V110H,V110L
;
; ===============================================
;  H: INTERNET CAPABILITY FLAGS:
; Testing will allow a ":" after these flags to include additional info
;
FLAGS  IBN,IFC,ITN,IVM,IFT,ITX,IUC,IMI,ISE,IP,IEM
;
; ===============================================
;  I: SYSTEM ONLINE TIME USERFLAGS
;  Allows Tyz flags.
;  Valid range for y and z is A to X and a to x inclusive
;  Creates a blanket allowance
FLAGS Tyz
; Some example combinations (do not include these as regular flags -- there
; are 576 of them in total)
;  TAA,TAa,TAb,TAB,TAc,TAC,TAd,TAD,TAe,TAE,TAf,TAF,TAg,TAG,TAh,TAH
;  TAi,TAI,TAj,TAJ,TAK,TAk,TAl,TAL,TAm,TAM,TAn,TAN,TAo,TAO,TAp,TAP
;  TAq,TAQ,TAr,TAR,TAs,TAS,TAt,TAT,TAu,TAU,TAv,TAV,TAW,TAw,TAx,TAX
;
;
; ===============================================
;  A. FORMAT OF USER FLAGS
;     U,x..x
;
; Note: there has been some dispute over whether the U should be followed by a
; comma or whether the user flag can come immediately, e.g., UNEC or U,NEC
; NLMake will accept either format.
;
; ===============================================
;    B: MAIL ORIENTED USER FLAGS: (the U is infered by the token UFLAGS)
; PN* matches any numeric value, e.g., PN10999, for displaying private networks
UFLAGS  ZEC,REC,NEC,SDS,SMH,NC,PN*
;
; ===============================================
; Redundant Flags
;
; Not all are here!  You may add more if you wish, but note the format.
; To prevent redundant checking comment out the RFLAGS lines.
; RFLAGS true_value, <redundant values>
RFLAGS  V32b,<V32,MNP>
RFLAGS  V42b,<V42,MNP>
RFLAGS  HST,<MNP>
; ===============================================
; Suggest replacements for bad or old flags
; U before the name denotes a U flag
;
OFLAGS  UTELNET,<ITN>
OFLAGS  BND,<IBN>
OFLAGS  TEL,<ITN>
OFLAGS  TELNET,<ITN>
OFLAGS  VMD,<IVM>
OFLAGS  TCP,<IP>
;
