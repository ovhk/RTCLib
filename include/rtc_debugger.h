/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc_debugger.h
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_debugger.h,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.1   Feb 16 2005 16:12:34   VANHOUCKE
 Update
 
    Rev 1.0   Feb 16 2005 16:10:10   VANHOUCKE
 Initial revision.
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef __RTC_DEBUGGER__
#define __RTC_DEBUGGER__

/*--------------- INCLUDES : ---------------*/

#include "../include/rtc_shared.h"							/* Format des données partagées */

/*--------------- TYPEDEFS : ---------------*/

/**
 * Définition du Debugger
 */
typedef struct
{
	LPSECURITY_ATTRIBUTES	lpThreadAttributes;		/* Attribut du Thread */
	SIZE_T						dwStackSize;				/* Taille de la pile */
	LPTHREAD_START_ROUTINE	lpStartAddress;			/* Pointeur vers la routine (fonction) du Thread */
	LPVOID						lpParameter;				/* Paramètres passés à la routine du Thread */
	DWORD							dwCreationFlags;			/* Paramètres de création/lancement du Thread */
	DWORD							ThreadId;					/* Identifiant du Thread */
	HANDLE						hThread;						/* Handle sur le Thread */

	HANDLE						hMapFile;					/* Handle sur le File Mapping */

	HANDLE						hEventGetData;				/* Handle sur l'évènement RtcDebuggerGetData */
	HANDLE						hEventDataReady;			/* Handle sur l'évènement RtcDebuggerDataReady */

	PDEBUGGER_SHARED_DATA	lpSharedData;				/* Pointeur sur la mémoire partagée */
	DWORD							dwSharedDataSize;			/* Taille de la mémoire partagée */

	BOOL							bDebuggerExit;				/* Arrêt du debugger */
} DEBUGGER, * PDEBUGGER;

/*--------------- FUNCTIONS : ---------------*/

DWORD ThreadRtcDebugger( LPVOID Param );

/*--------------- VARIABLES : ---------------*/

// <summary>Pointeur vers le debugger</summary>
extern DEBUGGER	RtcDebugger; 

#endif __RTC_DEBUGGER__