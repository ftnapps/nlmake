; Compress.CTL file for NLMake
;
Archiver ARC
;Add             c:\source\nlmake\master\arc2 a %a %f
Add             pkpak -a %a %f
;Extract         c:\source\nlmake\master\arc2 e %a %f
Extract         pkunpak %a %f
Extension       A??
Ident           0,1a
FILENAME
End
;
Archiver        ZIP
Extension       Z??
Ident           0,504b0304                      ; "PK^c^d"
Add             pkzip -a %a %f
Extract         pkunzip -n %a %f
End
;
Archiver        ARJ
Extension       J??
Ident           0,60ea
Add             arj a -e+ %a %f
Extract         arj e %a %f
End
;
Archiver        RAR
Extension       R??
Ident           0,526172211a0700
Add             rar a -ee -md64 -ep -y -std -c- %a %f
Extract         rar e -o- -y -std -c- %a %f
End
;
