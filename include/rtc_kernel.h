/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	kernel.h
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_kernel.h,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.2   Feb 22 2005 12:16:00   DAUDIER
 Protection de EnterRegion par section critique  LockInRegionInRegion

    Rev 1.1   Feb 16 2005 16:12:34   VANHOUCKE
 Update

    Rev 1.0   Feb 16 2005 16:10:10   VANHOUCKE
 Initial revision.

    Rev 1.6   Jan 27 2005 18:01:22   VANHOUCKE
 update

    Rev 1.5   Jan 19 2005 18:54:42   VANHOUCKE
 Ajout de la structure Region dans Kernel

    Rev 1.4   Dec 24 2004 15:44:22   VANHOUCKE
 Update

    Rev 1.3   Dec 23 2004 13:49:12   VANHOUCKE
 Ajout de la structure du debugger

    Rev 1.2   Dec 21 2004 08:58:04   VANHOUCKE
 modif commentaire

    Rev 1.1   Dec 08 2004 17:11:36   VANHOUCKE
 Inclusion de ios86c.h

    Rev 1.0   Dec 03 2004 09:02:50   VANHOUCKE
 Initial revision.

* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef __RTC_KERNEL__
#define __RTC_KERNEL__
#pragma once

/*--------------- INCLUDES : ---------------*/

#define	_WIN32_WINNT 0x0501 /* Windows XP Developpement */
#include <windows.h>
#define  STRSAFE_LIB 
#include <StrSafe.h>

#include <stack.h>	/* DataStructureLib.lib */

#include <base86c.h> /* RTC */
#include <xec86c.h>	/* RTC */
#include <ios86c.h>	/* RTC */

#if !defined(USE_MULTIMEDIA_TIMER) && !defined(USE_QUEUE_TIMER)
#	pragma message( "@ERROR: Timer non configure : USE_MULTIMEDIA_TIMER ou USE_QUEUE_TIMER")
#endif

#if defined(USE_MULTIMEDIA_TIMER) && defined(USE_QUEUE_TIMER)
#	pragma message( "@ERROR: Timer non configure : USE_MULTIMEDIA_TIMER ou USE_QUEUE_TIMER")
#	undef USE_MULTIMEDIA_TIMER
#	undef USE_QUEUE_TIMER
#endif

/*--------------- TYPEDEFS : ---------------*/

typedef Word (__stdcall * RtcCallBackFct)();			/* Prototype de fonction de callback RTC */

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

/**
 * Définition d'un timer
 */
typedef	struct
{
#ifdef USE_MULTIMEDIA_TIMER
#	pragma message( "@INFO: [RTC] USE_MULTIMEDIA_TIMER" )
	MMRESULT				idEvent;						/* Identifiant de l'évènement du timer */
	MMRESULT				mmTimer;						/* Identifiant du timer */
	UINT					uDelay;						/* Temps de claquage du timer */
#elif defined(USE_QUEUE_TIMER)
#	pragma message( "@INFO: [RTC] USE_QUEUE_TIMER" )
	HANDLE				hDelay;						/* Handle sur le timer */
#endif

	Word					FirstDelay;					/* durée du timer */
	Word					Period;						/* Périodicité du timer */
	RtcCallBackFct		FunctionPtr;				/* Fonction de callback utilisateur */

	CRITICAL_SECTION	csTimerLock;

	DelayStatus			Status;						/* Etat actuel du timer */

} DELAY, * PDELAY;

/**
 * Définition d'un sémaphore
 */
typedef	struct
{
	HANDLE					hSemaphore;					/* Handle du sémaphore */
	CRITICAL_SECTION		csSemaphore;				/* Section critique de protection des compteurs */
	DWORD						dwCounter;					/* Compteur du sémaphore : nombre de sémaphore pris */
	DWORD						dwCounterMax;				/* nombre maximum de sémaphore pris */
	Word						LastTaskP;					/* Numéro de la tâche qui a fait le dernier P */
} SEMAPHORE, * PSEMAPHORE;

/**
 * Définition d'une mailbox
 */
typedef	struct
{
	Stack						pile;							/* Pile FIFO pour les pointeurs de message */
	CRITICAL_SECTION		csMailBox;					/* Section critique de protection de la pile */
	HANDLE					hEvent;						/* Hande de l'évènement */
	Word						Capacity;					/* Capacité de la mailbox -------> NON UTILISE */

	Word						LastTaskSend;				/* Numéro de la tâche qui a fait le dernier Send */
	Word						LastTaskReceive;			/* Numéro de la tâche qui a fait le dernier Receive */
} MAILBOX, * PMAILBOX;

/**
 * Définition d'un message (NON UTILISE)
 */
typedef	struct
{
	BYTE	*					Message;						/* Données du message */
	SIZE_T					Taille;						/* Taille du message */
	UINT						MailBoxNumber;				/* Numéro de la mailbox ou se situe le message */
	Word						noTaskSend;					/* Numéro de la tâche qui a fait le Send */
} MESSAGE, * PMESSAGE;

/**
 * Définition d'une tâche
 */
typedef struct
{
	struct _WindowsThread
	{
		LPSECURITY_ATTRIBUTES	lpThreadAttributes;	/* Attribut du Thread */
		SIZE_T						dwStackSize;			/* Taille de la pile */
		LPTHREAD_START_ROUTINE	lpStartAddress;		/* Pointeur vers la routine (fonction) du Thread */
		LPVOID						lpParameter;			/* Paramètres passés à la routine du Thread */
		DWORD							dwCreationFlags;		/* Paramètres de création/lancement du Thread */
		DWORD							ThreadId;				/* Identifiant du Thread */
		HANDLE						hThread;					/* Handle sur le Thread */
		HANDLE						hEvent;					/* Handle sur l'évènement */
		tEvent						EventData;				/* Données de l'évènement */
		CRITICAL_SECTION			csEvent;					/* Section Critique pour protéger les données de l'évènement */
	} WindowsThread;											/* Information propre à Windows */

  struct _RtcTask
  {
		UINT							TaskNumber;				/* Numéro de la tâche */
		UINT							Priority;				/* Priorité de la tâche */
		PUINT							StackPtr;				/* Pointeur vers la pile de la tâche -------> NON UTILISE */
		UINT							DataSegment;			/* Segment de données -------> NON UTILISE */
		BOOL							TaskUsesNDP;			/* Utilisation Co-Processeur: 8087 ou 80287 -------> NON UTILISE */
		void							(* Hook)();				/*  -------> NON UTILISE */
		ULONG							HookParam;				/*  -------> NON UTILISE */
		PULONG						pTCBAddr;				/*  -------> NON UTILISE */

		TaskStatus					Status;
		TaskStatus					StatusBeforeRegion;

  } RtcTask;													/* Informations propre à RTC */

} TACHE, * PTACHE;

/**
 * Définition du Kernel
 */
typedef struct
{
	size_t						szMaxKernelSize;			/* Taille maximum du Kernel calculé à partir de ConfigTable */

	struct
	{
		CRITICAL_SECTION			csCriticalRegion;				/* Section Critique utilisé pour les régions critiques */
		DWORD							dwInRegion;						/* Compteur de région */
		Word							dwTaskNoInRegion;				/* Numéro de la tâche qui est en région */
		BOOL							bSafeRegionActive;			/* Active/désactive la protection des régions */
		HANDLE						hRegionEvent;					/* Handle sur l'évènement utilisé pour la protection des régions  */
		CRITICAL_SECTION			csCriticalRegion_InRegion;	/* Section Critique utilisé pour la protection des régions en région */
	} Region;

	struct tConfigTable *	ConfigTable;				/* Copy du tableau de configuration */
	PTACHE	   *				pTaches;						/* Liste de Tâches */
	PSEMAPHORE *				pSemaphores;				/* Liste de Sémaphores */
	PDELAY     *				pDelays;						/* Liste de Timers */
	PMAILBOX   *				pMailBoxes;					/* Liste de MailBox */
	PMESSAGE   *				pMessage;					/* Liste de Message -------> NON UTILISE */
	BOOL							bKernelOk;					/* Utilisé pour savoir si le kernel a été correctement initialisé */

	DoubleWord *				BKG_TCBAddr;				/* Non utilisé */
	DoubleWord *				CLK_TCBAddr;				/* Non utilisé */

#ifdef USE_MULTIMEDIA_TIMER
	TIMECAPS		MultimediaTimerCapability;				/* Capacité du timer (dépend +- du hard) */
	DWORD			MultimediaTimerResolution;				/* Résolution du timer */
#endif

} KERNEL, * PKERNEL;

/*--------------- VARIABLES : ---------------*/

extern PKERNEL Kernel;

#endif __RTC_KERNEL__