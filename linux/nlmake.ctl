MASTER      /home/rc12/nlmake/master
UPDATE      /home/rc12/nlmake/update
MAILFILES   /home/rc12/nlmake/mail
;UPLOADS     /home/rc12/nlmake/INBOUND/PROT
BADFILES    /home/rc12/nlmake/bad
OUTPATH     /home/rc12/nlmake/out
MESSAGES    /home/rc12/nlmake/netmail
LOG 5
COMPRESS ARC
ARC 5
NETADDRESS 1:12/0 <RC12>
SUBMIT 1:1/0 <ZC1> Crash
NOTIFY      SELF
NOTIFY      ERRORS
NOTIFY      RECEIPT

FLAGS NOCASE
;AUTO-FLAG   NOCASE ;
;MAXAGE 30
;MERGE       /home/rc12/nlmake/NODELIST
COMMENTS    COMMENTS.TXT
PROCESS     FRIDAY
OUTFILE     REGION12
OUTDIFF     RG12DIFF
COPYRIGHT   CPYRIGHT.TXT
PROLOG      PROLOG.TXT
EPILOG      EPILOG.TXT
;CLEANUP
WARNINGS
;MINPHONE    4
BAUDRATE    300,1200,2400,4800,9600,14400,16800,19200,28800,33600
THRESHOLD   0 -1
MAKE REGION 12

DATA

Region,12,Eastern_Canada,ON_PQ_NB_PE_NS_NF,Carl_Austin_Bennett,1-613-549-5599,9600,CM,XA,V34,IT,IMI,IUC,IBN,ITN:cafe.dyndns.org
,2,Deputy_RC,Ottawa_ON,Steve_Fredette,1-613-742-7612,9600,CM,XA,V32b,V42b,V34
,12,northstar.ccai.com,Ottawa_ON,Ken_Wilson,1-613-739-8634,9600,H16,V34,VFC,V32b,V42b,CM,XA,IBN
,70,The_Pembroke_BBS,Pembroke_ON,Robert_Terpstra,1-613-735-1019,9600,H14,V32b,V42b,XA,CM
,242,Harness_The_Ability,Toronto_ON,Jim_Rysyk,1-416-604-1221,9600,CM,XA,VFC,V32b,V42b

FILES

    Net     163     ottawa.163      163/0       <Jim Brown>
    Net     167     montreal.167    167/0       <Jesse Dooling>
    Net     224     sudbury.224     224/0       <Roger Ouimette>
    Net     229     oshawa.229      229/0       <Dave Hamilton>
    Net     244     hamilton.244    244/0       <Ian Moote>
    Net     246     windsor.246     246/0       <Mason Vye>
    Net     247     niagara.247     247/0       <Hugh Mitchell>
    Net     248     cornwall.248    248/0       <Tammy Wilson>
    Net     249     kingston.249    249/303     <Joe Delahaye>
    Net     250     toronto.250     250/0       <Brent McLaren>
    Net     255     saintjhn.255    255/0       <Terry Davies>
    Net     2401    londonon.401    2401/0      <Rob Ireland>
    Net     2404    lancastr.404    2404/0      <Gerry Calhoun>
