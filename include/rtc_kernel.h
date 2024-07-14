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
* Noyau d'�mulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
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

/**
 * D�finition d'un timer
 */
typedef	struct
{
#ifdef USE_MULTIMEDIA_TIMER
#	pragma message( "@INFO: [RTC] USE_MULTIMEDIA_TIMER" )
	MMRESULT				idEvent;						/* Identifiant de l'�v�nement du timer */
	MMRESULT				mmTimer;						/* Identifiant du timer */
	UINT					uDelay;						/* Temps de claquage du timer */
#elif defined(USE_QUEUE_TIMER)
#	pragma message( "@INFO: [RTC] USE_QUEUE_TIMER" )
	HANDLE				hDelay;						/* Handle sur le timer */
#endif

	Word					FirstDelay;					/* dur�e du timer */
	Word					Period;						/* P�riodicit� du timer */
	RtcCallBackFct		FunctionPtr;				/* Fonction de callback utilisateur */

	CRITICAL_SECTION	csTimerLock;

	DelayStatus			Status;						/* Etat actuel du timer */

} DELAY, * PDELAY;

/**
 * D�finition d'un s�maphore
 */
typedef	struct
{
	HANDLE					hSemaphore;					/* Handle du s�maphore */
	CRITICAL_SECTION		csSemaphore;				/* Section critique de protection des compteurs */
	DWORD						dwCounter;					/* Compteur du s�maphore : nombre de s�maphore pris */
	DWORD						dwCounterMax;				/* nombre maximum de s�maphore pris */
	Word						LastTaskP;					/* Num�ro de la t�che qui a fait le dernier P */
} SEMAPHORE, * PSEMAPHORE;

/**
 * D�finition d'une mailbox
 */
typedef	struct
{
	Stack						pile;							/* Pile FIFO pour les pointeurs de message */
	CRITICAL_SECTION		csMailBox;					/* Section critique de protection de la pile */
	HANDLE					hEvent;						/* Hande de l'�v�nement */
	Word						Capacity;					/* Capacit� de la mailbox -------> NON UTILISE */

	Word						LastTaskSend;				/* Num�ro de la t�che qui a fait le dernier Send */
	Word						LastTaskReceive;			/* Num�ro de la t�che qui a fait le dernier Receive */
} MAILBOX, * PMAILBOX;

/**
 * D�finition d'un message (NON UTILISE)
 */
typedef	struct
{
	BYTE	*					Message;						/* Donn�es du message */
	SIZE_T					Taille;						/* Taille du message */
	UINT						MailBoxNumber;				/* Num�ro de la mailbox ou se situe le message */
	Word						noTaskSend;					/* Num�ro de la t�che qui a fait le Send */
} MESSAGE, * PMESSAGE;

/**
 * D�finition d'une t�che
 */
typedef struct
{
	struct _WindowsThread
	{
		LPSECURITY_ATTRIBUTES	lpThreadAttributes;	/* Attribut du Thread */
		SIZE_T						dwStackSize;			/* Taille de la pile */
		LPTHREAD_START_ROUTINE	lpStartAddress;		/* Pointeur vers la routine (fonction) du Thread */
		LPVOID						lpParameter;			/* Param�tres pass�s � la routine du Thread */
		DWORD							dwCreationFlags;		/* Param�tres de cr�ation/lancement du Thread */
		DWORD							ThreadId;				/* Identifiant du Thread */
		HANDLE						hThread;					/* Handle sur le Thread */
		HANDLE						hEvent;					/* Handle sur l'�v�nement */
		tEvent						EventData;				/* Donn�es de l'�v�nement */
		CRITICAL_SECTION			csEvent;					/* Section Critique pour prot�ger les donn�es de l'�v�nement */
	} WindowsThread;											/* Information propre � Windows */

  struct _RtcTask
  {
		UINT							TaskNumber;				/* Num�ro de la t�che */
		UINT							Priority;				/* Priorit� de la t�che */
		PUINT							StackPtr;				/* Pointeur vers la pile de la t�che -------> NON UTILISE */
		UINT							DataSegment;			/* Segment de donn�es -------> NON UTILISE */
		BOOL							TaskUsesNDP;			/* Utilisation Co-Processeur: 8087 ou 80287 -------> NON UTILISE */
		void							(* Hook)();				/*  -------> NON UTILISE */
		ULONG							HookParam;				/*  -------> NON UTILISE */
		PULONG						pTCBAddr;				/*  -------> NON UTILISE */

		TaskStatus					Status;
		TaskStatus					StatusBeforeRegion;

  } RtcTask;													/* Informations propre � RTC */

} TACHE, * PTACHE;

/**
 * D�finition du Kernel
 */
typedef struct
{
	size_t						szMaxKernelSize;			/* Taille maximum du Kernel calcul� � partir de ConfigTable */

	struct
	{
		CRITICAL_SECTION			csCriticalRegion;				/* Section Critique utilis� pour les r�gions critiques */
		DWORD							dwInRegion;						/* Compteur de r�gion */
		Word							dwTaskNoInRegion;				/* Num�ro de la t�che qui est en r�gion */
		BOOL							bSafeRegionActive;			/* Active/d�sactive la protection des r�gions */
		HANDLE						hRegionEvent;					/* Handle sur l'�v�nement utilis� pour la protection des r�gions  */
		CRITICAL_SECTION			csCriticalRegion_InRegion;	/* Section Critique utilis� pour la protection des r�gions en r�gion */
	} Region;

	struct tConfigTable *	ConfigTable;				/* Copy du tableau de configuration */
	PTACHE	   *				pTaches;						/* Liste de T�ches */
	PSEMAPHORE *				pSemaphores;				/* Liste de S�maphores */
	PDELAY     *				pDelays;						/* Liste de Timers */
	PMAILBOX   *				pMailBoxes;					/* Liste de MailBox */
	PMESSAGE   *				pMessage;					/* Liste de Message -------> NON UTILISE */
	BOOL							bKernelOk;					/* Utilis� pour savoir si le kernel a �t� correctement initialis� */

	DoubleWord *				BKG_TCBAddr;				/* Non utilis� */
	DoubleWord *				CLK_TCBAddr;				/* Non utilis� */

#ifdef USE_MULTIMEDIA_TIMER
	TIMECAPS		MultimediaTimerCapability;				/* Capacit� du timer (d�pend +- du hard) */
	DWORD			MultimediaTimerResolution;				/* R�solution du timer */
#endif

} KERNEL, * PKERNEL;

/*--------------- VARIABLES : ---------------*/

extern PKERNEL Kernel;

#endif __RTC_KERNEL__