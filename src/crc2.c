/* compute crc's */
/* crc-16 is based on the polynomial x^16+x^15+x^2+1 */
/*  The data is assumed to be fed in from least to most significant bit */
/* crc-ccitt is based on the polynomial x^16+x^12+x^5+1 */
/*  The data is fed in from most to least significant bit */
/* The prescription for determining the mask to use for a given polynomial
	is as follows:
		1.  Represent the polynomial by a 17-bit number
		2.  Assume that the most and least significant bits are 1
		3.  Place the right 16 bits into an integer
		4.  Bit reverse if serial LSB's are sent first
*/
/* Usage : crc2 [filename] */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#define		M16	0xA001		/* crc-16 mask */
#define		MTT	0x1021		/* crc-ccitt mask */

/* function declarations */
unsigned short updcrc(unsigned short,short,unsigned short);
unsigned short updcrcr(unsigned short,short,unsigned short);
void perr(char *);

/* variables */
char filename[100];
unsigned long crc16,crctt;
short ch;
unsigned long num;
FILE *fp;

/* driver */
unsigned short getcrc(char *filename, long offset)
{
	if((fp=fopen(filename,"rb"))==NULL) perr("Can't open file");
   fseek (fp,(short)(offset+1), SEEK_SET);

	num=0L; crc16=crctt=0;
	while((ch=fgetc(fp))!=EOF)
	{
		num++;
		//crc16=updcrcr(crc16,ch,M16);
		if(ch != 0x1A)
		crctt=updcrc(crctt,ch,MTT);
	}
	fclose(fp);
	/*
  printf("\nNumber of bytes = %lu\nCRCTT = %04X",
      num,(unsigned short) crctt);
  printf("\nNumber of bytes = %lu\nCRCTT = %04u",
      num,(unsigned short) crctt);
	 */
    return((unsigned short) crctt);
}

/* update crc */
unsigned short updcrc(unsigned short crc, short c, unsigned short mask)
{
	short i;
	c<<=8;
	for(i=0;i<8;i++)
	{
		if((crc ^ c) & 0x8000) crc=(crc<<1)^mask;
		else crc<<=1;
		c<<=1;
	}
	return crc;
}

/* update crc reverse */
unsigned short updcrcr(unsigned short crc,short c,unsigned short mask)
{
	short i;
	for(i=0;i<8;i++)
	{
		if((crc ^ c) & 1) crc=(crc>>1)^mask;
		else crc>>=1;
		c>>=1;
	}
	return crc;
}

/* error abort */
void perr(char *s)
{
	printf("\n%s",s); exit(1);
}

