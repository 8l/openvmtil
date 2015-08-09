
#include "../includes/cfrtil.h"

byte CharTable []= { // from maru
  /*  00 nul */	0,
  /*  01 soh */	0,
  /*  02 stx */	0,
  /*  03 etx */	0,
  /*  04 eot */	0,
  /*  05 enq */	0,
  /*  06 ack */	0,
  /*  07 bel */	0,
  /*  08 bs  */	0,
  /*  09 ht  */	0,
  /*  0a nl  */	CHAR_PRINT | CHAR_BLANK,
  /*  0b vt  */	0,
  /*  0c np  */	CHAR_PRINT | CHAR_BLANK,
  /*  0d cr  */	CHAR_PRINT | CHAR_BLANK,
  /*  0e so  */	0,
  /*  0f si  */	0,
  /*  10 dle */	0,
  /*  11 dc1 */	0,
  /*  12 dc2 */	0,
  /*  13 dc3 */	0,
  /*  14 dc4 */	0,
  /*  15 nak */	0,
  /*  16 syn */	0,
  /*  17 etb */	0,
  /*  18 can */	0,
  /*  19 em  */	0,
  /*  1a sub */	0,
  /*  1b esc */	0,
  /*  1c fs  */	0,
  /*  1d gs  */	0,
  /*  1e rs  */	0,
  /*  1f us  */	0,
  /*  20 sp  */	CHAR_PRINT | CHAR_BLANK,
  /*  21  !  */	CHAR_PRINT | CHAR_LETTER,
  /*  22  "  */	CHAR_PRINT,
  /*  23  #  */	CHAR_PRINT | CHAR_LETTER,
  /*  24  $  */	CHAR_PRINT | CHAR_LETTER,
  /*  25  %  */	CHAR_PRINT | CHAR_LETTER,
  /*  26  &  */	CHAR_PRINT | CHAR_LETTER,
  /*  27  '  */	CHAR_PRINT,
  /*  28  (  */	CHAR_PRINT,
  /*  29  )  */	CHAR_PRINT,
  /*  2a  *  */	CHAR_PRINT | CHAR_LETTER,
  /*  2b  +  */	CHAR_PRINT | CHAR_LETTER,
  /*  2c  ,  */	CHAR_PRINT | CHAR_LETTER,
  /*  2d  -  */	CHAR_PRINT | CHAR_LETTER,
  /*  2e  .  */	CHAR_PRINT | CHAR_LETTER,
  /*  2f  /  */	CHAR_PRINT | CHAR_LETTER,
  /*  30  0  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  31  1  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  32  2  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  33  3  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  34  4  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  35  5  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  36  6  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  37  7  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  38  8  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  39  9  */	CHAR_PRINT | CHAR_DIGIT10 | CHAR_DIGIT16,
  /*  3a  :  */	CHAR_PRINT | CHAR_LETTER,
  /*  3b  ;  */	CHAR_PRINT,
  /*  3c  <  */	CHAR_PRINT | CHAR_LETTER,
  /*  3d  =  */	CHAR_PRINT | CHAR_LETTER,
  /*  3e  >  */	CHAR_PRINT | CHAR_LETTER,
  /*  3f  ?  */	CHAR_PRINT | CHAR_LETTER,
  /*  40  @  */	CHAR_PRINT | CHAR_LETTER,
  /*  41  A  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  42  B  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  43  C  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  44  D  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  45  E  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  46  F  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  47  G  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  48  H  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  49  I  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  4a  J  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  4b  K  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  4c  L  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  4d  M  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  4e  N  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  4f  O  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  50  P  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  51  Q  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  52  R  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  53  S  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  54  T  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  55  U  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  56  V  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  57  W  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  58  X  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  59  Y  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  5a  Z  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  5b  [  */	CHAR_PRINT,
  /*  5c  \  */	CHAR_PRINT | CHAR_LETTER,
  /*  5d  ]  */	CHAR_PRINT,
  /*  5e  ^  */	CHAR_PRINT | CHAR_LETTER,
  /*  5f  _  */	CHAR_PRINT | CHAR_LETTER,
  /*  60  `  */	CHAR_PRINT,
  /*  61  a  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  62  b  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  63  c  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  64  d  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  65  e  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  66  f  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA | CHAR_DIGIT16,
  /*  67  g  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  68  h  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  69  i  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  6a  j  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  6b  k  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  6c  l  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  6d  m  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  6e  n  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  6f  o  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  70  p  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  71  q  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  72  r  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  73  s  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  74  t  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  75  u  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  76  v  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  77  w  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  78  x  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  79  y  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  7a  z  */	CHAR_PRINT | CHAR_LETTER | CHAR_ALPHA,
  /*  7b  {  */	CHAR_PRINT,
  /*  7c  | */	CHAR_PRINT | CHAR_LETTER,
  /*  7d  }  */	CHAR_PRINT,
  /*  7e  ~  */	CHAR_PRINT | CHAR_LETTER,
  /*  7f del */	0,
};

Boolean
CharTable_IsCharType ( byte c, int32 type )
{
    return CharTable [ c ] & type ;
}
int32
CharSet_IsDelimiter ( CharSet *cset, byte ch )
{
    return cset [ ch ] & CHAR_DELIMITER ;
}

void
CharSet_SetChar ( CharSet *cset, byte ch )
{
    cset [ ch ] |= CHAR_DELIMITER ;
}

void
CharSet_UnSetChar ( CharSet *cset, byte ch )
{
    cset [ ch ] &= ~ CHAR_DELIMITER ;
}

CharSet *
CharSet_Init ( CharSet *cset, int32 size, byte * initString )
{
    int32 i ;
    memset ( cset, 0, size ) ;
    //if ( size >= sizeof (CharTable) ) memcpy ( cset, &CharTable, sizeof( CharTable) ) ;
    for ( i = 0 ; initString [i] ; i ++ ) cset [ initString [ i ] ] |= CHAR_DELIMITER ;
    cset [0] |= CHAR_DELIMITER ; // default 0 is always a delimiter
    return cset ;
}

CharSet *
CharSet_Allocate ( int32 size, int32 allocType )
{
    CharSet * acset = ( CharSet * ) _Mem_Allocate ( size, allocType ? allocType : SESSION ) ;
    return acset ;
}

CharSet *
_CharSet_New ( byte * initString, int32 size, int32 allocType )
{
    CharSet * cset = CharSet_Allocate ( size, allocType ) ;
    CharSet_Init ( cset, size, initString ) ;
    return cset ;
}

CharSet *
CharSet_New ( byte * initString, int32 allocType )
{
    return _CharSet_New ( initString, 128, allocType ) ;
}


