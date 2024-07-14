/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc_shared.h
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_shared.h,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'�mulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.1   Feb 16 2005 16:12:18   VANHOUCKE
 Update
 
    Rev 1.0   Dec 24 2004 15:44:46   VANHOUCKE
 Initial revision.
 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef __RTC_SHARED__
#define __RTC_SHARED__
#pragma once

/*--------------- INCLUDES : ---------------*/

#include <windows.h>

/*--------------- TYPEDEFS : ---------------*/

#ifndef __RTC_KERNEL__

	typedef enum
	{
		Launched			= 0,							/* Le timer a d�marr� */
		Expired			= 1,							/* Le timer a claqu� */
		Stopped			= 2							/* Le timer a �t� arr�t� */
	} DelayStatus;										/* Etat actuel du timer */

	typedef enum 
	{
		Current			= 'C',								/* T�che courante */
		Ready				= '*',								/* T�che en cours d'�xecution */
		Waiting			= 'W',								/* T�che en attente WaitEvent*/
		Waiting_P		= 'P',								/* T�che en attente P */
		Waiting_TP		= 'T',								/* T�che en attente TestP */
		Waiting_R		= 'R',								/* T�che en attente Receive */
		NonOperational	= '?',								/* T�che stopp�e ou non lanc�e */
		Suspend			= 'S'									/* T�che en Suspend Region*/
	} TaskStatus;												/* Etat actuel de la t�che */

#endif __RTC_KERNEL__

#pragma pack ( push, 1 )	/* Alignement sur 1 octet */

typedef struct
{
	unsigned int				Priority;				/* Priorit� de la t�che */
	TaskStatus					Status;					/* Etat actuel de la t�che */
	DWORD							ThreadId;				/* Identifiant du Thread */
	unsigned int				EventData;				/* Donn�es de l'�v�nement */
} SHAREDDATA_TASK, * PSHAREDDATA_TASK;

typedef struct
{
	DWORD							dwCounter;				/* Compteur du s�maphore : nombre de s�maphore pris */
	DWORD							dwCounterMax;			/* nombre maximum de s�maphore pris */
	unsigned int				LastTaskP;				/* Num�ro de la t�che qui a fait le dernier P */
} SHAREDDATA_SEMAPHORE, * PSHAREDDATA_SEMAPHORE;

typedef struct
{
	unsigned int				dwNbMessages;			/* Nombre de messages dans la pile */
	unsigned int				LastTaskSend;			/* Num�ro de la t�che qui a fait le dernier Send */
	unsigned int				LastTaskReceive;		/* Num�ro de la t�che qui a fait le dernier Receive */
} SHAREDDATA_MAILBOX, * PSHAREDDATA_MAILBOX;

typedef struct
{
	unsigned int				FirstDelay;				/* dur�e du timer */
	unsigned int				Period;					/* P�riodicit� du timer */
	DelayStatus					Status;					/* Etat actuel du timer */
} SHAREDDATA_DELAY, * PSHAREDDATA_DELAY;

/**
 * D�finition des donn�es partag�es pour le debug
 */
typedef struct
{
   unsigned int				MaxPriority;			/* Priorit� maximum */
   unsigned int				NbTasks;					/* Nombre de t�ches */
   unsigned int				NbSemaphores;			/* Nombre de s�maphores */
   unsigned int				NbMailBoxes;			/* Nombre de mailbox */
   unsigned int				NbDelays;				/* Nombre de timer */

	/* R�gions */
	DWORD							dwInRegion;				/* Compteur de r�gion */
	unsigned int				dwTaskNoInRegion;		/* Num�ro de la t�che qui est en r�gion */
	BOOL							bSafeRegionActive;	/* Active/d�sactive la protection des r�gions */

	/* Offsets par rapport au debut de la structure (DEBUGGER_SHARED_DATA) 
	 * pour acceder aux structures des Tasks, Semaphores, Mailboxes, Delays 
	 */
	DWORD							IndexTasks;				
	DWORD							IndexSemaphores;
	DWORD							IndexMailboxes;
	DWORD							IndexDelays;

} DEBUGGER_SHARED_DATA, * PDEBUGGER_SHARED_DATA; 

#pragma pack ( pop )	/* alignement par d�faut */

#define RTC_SHARED_OBJECT_NAME		TEXT("RtcSharedObject")			/* Nom du Map File */
#define RTC_EVENT_DATA_READY_NAME	TEXT("RtcDebuggerDataReady")	/* Nom de l'�v�nement data ready */
#define RTC_EVENT_GET_DATA_NAME		TEXT("RtcDebuggerGetData")		/* Nom de l'�v�nement get data */

#endif __RTC_SHARED__