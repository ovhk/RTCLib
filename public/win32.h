#ifndef __WIN32_COMMON__
#define __WIN32_COMMON__

#ifdef WIN32 /* Inclusions spécifiques à Windows */

	#pragma once
	#pragma warning ( disable : 4068 ) /* Supprime les warnings de pragma inconnu */

	#define	_WIN32_WINNT	0x0501 /* Développement Windows XP */
	#include <winsock2.h>
	
	#define  STRSAFE_NO_DEPRECATE	/* On s'autorise à utiliser sprintf etc... */
	#define  STRSAFE_LIB 
	#include <StrSafe.h>

	struct time	/* structure time DOS */
	{
		unsigned char ti_min;     /* Minutes */
		unsigned char ti_hour;    /* Hours */
		unsigned char ti_hund;    /* Hundredths of seconds */
		unsigned char ti_sec;     /* Seconds */
	};

	struct date	/* structure data DOS */
	{
		int	da_year;    /* Year - 1980 */
		char	da_day;     /* Day of the month */
		char	da_mon;     /* Month (1 = Jan) */
	};

	#undef		_far    /* '_far' is an obsolete keyword */
	#define		_far

	#undef		_FAR    /* '_FAR' is an obsolete keyword */
	#define		_FAR

	#undef		far		/* 'far' is an obsolete keyword */
	#define		far

	#undef		FALSE
	#undef		TRUE

	#undef		max
	#undef		min

	#undef		BOOLEAN

	/* WinGDI */
	#undef		TRANSPARENT
	#undef		OPAQUE

#else	 /* Systeme d'exploitation inconnu ! */
	#pragma	message ("Systeme d'exploitation inconnu !")
	//#error Systeme d'exploitation inconnu !
#endif

#endif	/* __WIN32_COMMON__ */
