; Compress.CTL file for NLMake/LNX  4/10/2001
; archive filenames ("%f") are in quotes to protect wildcards from shell
;
Archiver ARC
Add             arc a %a "%f"
Extract         arc e %a "%f"
Extension       A??
Ident           0,1a
FILENAME
End
;
Archiver        ZIP
Extension       Z??
Ident           0,504b0304                      ; "PK^c^d"
Add             zip %a "%f"
Extract         unzip %a "%f"~
End
;
Archiver        ARJ
Extension       J??
Ident           0,60ea
Add             arj a -e+ %a "%f"
Extract         arj e %a "%f"
End
;
Archiver        RAR
Extension       R??
Ident           0,526172211a0700
Add             rar a -ee -md64 -ep -y -std -c- %a "%f"
Extract         rar e -o- -y -std -c- %a "%f"
End
;
