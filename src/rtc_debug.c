/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc_debug.c
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_debug.c,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.8   Apr 05 2005 13:27:54   VANHOUCKE
 update
 
    Rev 1.7   Feb 21 2005 17:05:34   VANHOUCKE
 update
 
    Rev 1.6   Feb 16 2005 16:13:20   VANHOUCKE
 Update
 
    Rev 1.5   Feb 04 2005 09:52:00   VANHOUCKE
 update
 
    Rev 1.4   Jan 27 2005 18:01:32   VANHOUCKE
 update
 
    Rev 1.3   Dec 24 2004 15:43:10   VANHOUCKE
 Update
* --------------------------------------------------------------------
* $F_HEAD
*/

#define USE_RTC_DEBUG

/*--------------- INCLUDES : ---------------*/

#include "../include/rtc_config.h"
#include "../include/rtc_debug.h"
#include "../include/rtc_kernel.h"

#ifdef	USE_RTC_DEBUG
	static	HANDLE				__hDebug			= INVALID_HANDLE_VALUE;
	static	CRITICAL_SECTION	__csDebug;
	static	DWORD					__dwLen			= 0;
	static	UCHAR					__strBuffer[512];
	static	size_t				__Len				= 0;
#endif	USE_RTC_DEBUG

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :	
* PARAMETRES :
* RETOUR :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* TYPE :
* ROLE :
* --------------------------------------------------------------------
* $F_FCTN
*/
void RTC_DEBUG_INIT()
{
	InitializeCriticalSection(&__csDebug);
	__hDebug = CreateFile(RTC_KERNEL_DEBUG_FILENAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); /* | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING*/
	SetFilePointer(__hDebug, 0, NULL, FILE_END);
	ZeroMemory(__strBuffer, sizeof(__strBuffer));
	StringCchPrintf(__strBuffer, sizeof(__strBuffer), TEXT("RTC Kernel version : %s\r\n\r\n"), RTC_KERNEL_VERSION);
	WriteFile(__hDebug, __strBuffer, sizeof(__strBuffer) * sizeof(TCHAR), &__dwLen, NULL);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :	
* PARAMETRES :
* RETOUR :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* TYPE :
* ROLE :
* --------------------------------------------------------------------
* $F_FCTN
*/
void RTC_DEBUG_DEINIT()
{
	RTC_DEBUG_PRINTF( "\r\n\r\nRTC End\r\n" );
	CloseHandle( __hDebug );
	DeleteCriticalSection( &__csDebug );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :	
* PARAMETRES :
* RETOUR :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* TYPE :
* ROLE :
* --------------------------------------------------------------------
* $F_FCTN
*/
void RTC_DEBUG_PRINTF(const char * format, ...)
{
	va_list argList;

	va_start(argList, format);
	
	ZeroMemory(__strBuffer, sizeof(__strBuffer));
	
	StringCchVPrintf(__strBuffer, sizeof(__strBuffer), format, argList);

	va_end(argList);

	StringCchCat(__strBuffer, sizeof(__strBuffer), TEXT("\r\n"));

	StringCchLength(__strBuffer, sizeof(__strBuffer), &__Len);

	EnterCriticalSection(&__csDebug);

	WriteFile(__hDebug, __strBuffer, (DWORD) __Len * sizeof(TCHAR), &__dwLen, NULL);

	LeaveCriticalSection(&__csDebug);
}
