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
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
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
		Launched			= 0,							/* Le timer a démarré */
		Expired			= 1,							/* Le timer a claqué */
		Stopped			= 2							/* Le timer a été arrété */
	} DelayStatus;										/* Etat actuel du timer */

	typedef enum 
	{
		Current			= 'C',								/* Tâche courante */
		Ready				= '*',								/* Tâche en cours d'éxecution */
		Waiting			= 'W',								/* Tâche en attente WaitEvent*/
		Waiting_P		= 'P',								/* Tâche en attente P */
		Waiting_TP		= 'T',								/* Tâche en attente TestP */
		Waiting_R		= 'R',								/* Tâche en attente Receive */
		NonOperational	= '?',								/* Tâche stoppée ou non lancée */
		Suspend			= 'S'									/* Tâche en Suspend Region*/
	} TaskStatus;												/* Etat actuel de la tâche */

#endif __RTC_KERNEL__

#pragma pack ( push, 1 )	/* Alignement sur 1 octet */

typedef struct
{
	unsigned int				Priority;				/* Priorité de la tâche */
	TaskStatus					Status;					/* Etat actuel de la tâche */
	DWORD							ThreadId;				/* Identifiant du Thread */
	unsigned int				EventData;				/* Données de l'évènement */
} SHAREDDATA_TASK, * PSHAREDDATA_TASK;

typedef struct
{
	DWORD							dwCounter;				/* Compteur du sémaphore : nombre de sémaphore pris */
	DWORD							dwCounterMax;			/* nombre maximum de sémaphore pris */
	unsigned int				LastTaskP;				/* Numéro de la tâche qui a fait le dernier P */
} SHAREDDATA_SEMAPHORE, * PSHAREDDATA_SEMAPHORE;

typedef struct
{
	unsigned int				dwNbMessages;			/* Nombre de messages dans la pile */
	unsigned int				LastTaskSend;			/* Numéro de la tâche qui a fait le dernier Send */
	unsigned int				LastTaskReceive;		/* Numéro de la tâche qui a fait le dernier Receive */
} SHAREDDATA_MAILBOX, * PSHAREDDATA_MAILBOX;

typedef struct
{
	unsigned int				FirstDelay;				/* durée du timer */
	unsigned int				Period;					/* Périodicité du timer */
	DelayStatus					Status;					/* Etat actuel du timer */
} SHAREDDATA_DELAY, * PSHAREDDATA_DELAY;

/**
 * Définition des données partagées pour le debug
 */
typedef struct
{
   unsigned int				MaxPriority;			/* Priorité maximum */
   unsigned int				NbTasks;					/* Nombre de tâches */
   unsigned int				NbSemaphores;			/* Nombre de sémaphores */
   unsigned int				NbMailBoxes;			/* Nombre de mailbox */
   unsigned int				NbDelays;				/* Nombre de timer */

	/* Régions */
	DWORD							dwInRegion;				/* Compteur de région */
	unsigned int				dwTaskNoInRegion;		/* Numéro de la tâche qui est en région */
	BOOL							bSafeRegionActive;	/* Active/désactive la protection des régions */

	/* Offsets par rapport au debut de la structure (DEBUGGER_SHARED_DATA) 
	 * pour acceder aux structures des Tasks, Semaphores, Mailboxes, Delays 
	 */
	DWORD							IndexTasks;				
	DWORD							IndexSemaphores;
	DWORD							IndexMailboxes;
	DWORD							IndexDelays;

} DEBUGGER_SHARED_DATA, * PDEBUGGER_SHARED_DATA; 

#pragma pack ( pop )	/* alignement par défaut */

#define RTC_SHARED_OBJECT_NAME		TEXT("RtcSharedObject")			/* Nom du Map File */
#define RTC_EVENT_DATA_READY_NAME	TEXT("RtcDebuggerDataReady")	/* Nom de l'évènement data ready */
#define RTC_EVENT_GET_DATA_NAME		TEXT("RtcDebuggerGetData")		/* Nom de l'évènement get data */

#endif __RTC_SHARED__