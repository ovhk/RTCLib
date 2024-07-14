/*********************************************************/
/* Basic constants and types declarations for all        */
/* RealTime Craft products C language interface packages */
/*********************************************************/
#define    TRUE            1
#define    FALSE           0

/*===================================================*/
/*                                                   */
/*     Structure of the Status                       */
/*                                                   */
/*   F  E  D  C  B  A  9  8  7  6  5  4  3  2  1  0  */
/* +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+ */
/* |Tp|Rd| Product Code |    Error   Code          | */
/* +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+ */
/*   Tp=0 : Don't Trap,   Tp=1 : Must Trap           */
/*   Rd : Reserved                                   */
/*   ProductCode=31 : no product-encoded status      */
/*   Status=0 : OK                                   */
/*                                                   */
/*   ProductCodes:                                   */
/*       XEC      00000                              */
/*       IOS      00001                              */
/*       FMS      00010                              */
/*       PEMM     00100                              */
/*===================================================*/

#define  cOK         0
#define  cBadOpCode -24831 /* MustTrap, AllProduct, 1 */

typedef char          Char;       /* 8 bits character */
typedef unsigned char Byte;       /* unsigned 8 bits word */

typedef Byte          Boolean;    /* 0 = false, 1 = true */
typedef char          ShortInt;   /* signed 8 bits integer */

typedef int           Int;        /* signed 16 bits integer */
typedef unsigned int  Word;       /* unsigned 16 bits word  */

typedef long          LongInt;    /* signed 32 bits integer */
typedef unsigned long DoubleWord; /* unsigned 32 bits word  */

typedef Byte * Address;

typedef Word tStatus;
