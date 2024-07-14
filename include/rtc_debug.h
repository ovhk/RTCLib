/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc_debug.h
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_debug.h,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.3   Feb 16 2005 16:12:32   VANHOUCKE
 Update
 
    Rev 1.2   Jan 27 2005 18:01:28   VANHOUCKE
 update
 
    Rev 1.1   Dec 24 2004 15:44:22   VANHOUCKE
 Update
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef __RTC_DEBUG__
#define __RTC_DEBUG__
#pragma once

/* Cette MACRO permet de ne pas faire les appels de fonctions de debug au niveau Pre-Processeur */
#ifdef USE_RTC_DEBUG
#	ifndef RTC_KERNEL_DEBUG_LEVEL
#		define RTC_KERNEL_DEBUG_LEVEL DL_MAX
#	endif
#	define D(level, fct)									\
			{												\
				if (level <= RTC_KERNEL_DEBUG_LEVEL) fct;	\
			}												
#else
#	define D(level, fct)
#endif

#ifdef USE_RTC_DEBUG
#	pragma message ("@INFO: [RTC] MODE DEBUG ON")
#	if RTC_KERNEL_DEBUG_LEVEL == DL_MAX
#		pragma message ("@INFO: [RTC] RTC_KERNEL_DEBUG_LEVEL : DL_MAX")
#	elif RTC_KERNEL_DEBUG_LEVEL == DL_MIN
#		pragma message ("@INFO: [RTC] RTC_KERNEL_DEBUG_LEVEL : DL_MIN")
#	else
#		pragma message ("@INFO: [RTC] RTC_KERNEL_DEBUG_LEVEL : between DL_MIN and DL_MAX")
#	endif
#else
#	pragma message ("@INFO: [RTC] MODE DEBUG OFF")
#endif

/**
 * Prototypes des fonctions de debug
 */
void RTC_DEBUG_INIT(void);
void RTC_DEBUG_DEINIT(void);
void RTC_DEBUG_PRINTF(const char * format, ...);

#endif
