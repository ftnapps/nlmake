typedef struct COMPRESSTYPE
{
  short compressor;
  unsigned long offset;
  char add[100], extract[100], ident[50], ext;
}
COMPRESSTYPE;
