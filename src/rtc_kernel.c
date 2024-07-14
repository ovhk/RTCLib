/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc.c
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_kernel.c,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.7   May 03 2005 11:08:04   VANHOUCKE
 update
 
    Rev 1.6   Apr 28 2005 11:04:16   VANHOUCKE
 update
 
    Rev 1.5   Apr 05 2005 15:47:58   VANHOUCKE
 update
 
    Rev 1.4   Apr 05 2005 13:27:54   VANHOUCKE
 update
 
    Rev 1.2   Feb 21 2005 17:07:16   VANHOUCKE
 update
 
    Rev 1.1   Feb 16 2005 16:13:18   VANHOUCKE
 Update
 
    Rev 1.0   Feb 16 2005 16:09:32   VANHOUCKE
 Initial revision.
 
    Rev 1.10   Jan 27 2005 18:01:32   VANHOUCKE
 update
 
    Rev 1.9   Jan 19 2005 18:57:02   VANHOUCKE
 - Correction de bugs
 - Ajout de la protection des régions
 
    Rev 1.8   Jan 17 2005 11:31:00   VANHOUCKE
 Correction de EnterRegion et LeaveRegion
 
    Rev 1.7   Jan 11 2005 13:22:58   VANHOUCKE
 Optimisation des CurrentTask
 Ajout des status de retour
 
    Rev 1.6   Jan 03 2005 10:54:46   VANHOUCKE
 Suppression de warnings par des pragmas
 Correction de la fonction de callback des timers multimedia
 
    Rev 1.5   Dec 24 2004 15:43:52   VANHOUCKE
 Ajout du thread de debug du kernel RTC
 Ajout de status
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES : ---------------*/

#include "../include/rtc_config.h"							/* Configuration de la compilation */
#include "../include/rtc_debug.h"							/* Outils de debug */
#include "../include/rtc_kernel.h"							/* Format des données propre à RTC */
#include "../include/rtc_debugger.h"						/* Format des données du debugger RTC */

#pragma message ("@TODO: [Start/Stop Delay] Multimedia et queue timer fonctionnent un peu differemment")

/*--------------- VARIABLES : ---------------*/

// <summary>Handle sur le Tas RTC</summary>
static HANDLE	hMemoryHeap = INVALID_HANDLE_VALUE; /* Handle sur le Tas RTC */

// <summary>Pointeur vers les données du kernel</summary>
PKERNEL Kernel = NULL;						/* Pointeur vers les données du Kernel */

/*--------------- FUNCTIONS : ---------------*/

/* Fonction privée */
void LockInRegion( void );
int RtcToWindowsPriority( int priority );

#ifdef USE_MULTIMEDIA_TIMER
	void CALLBACK TimerFunction( UINT wTimerID, UINT msg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2 );
#elif defined USE_QUEUE_TIMER
	void CALLBACK TimerProc( void * lpParameter, BOOLEAN TimerOrWaitFired );
#endif

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
// <summary>
// Point d'entré de l'initialisation du kernel RTC
// </summary>
// <remarks>Cette fonction doit être appelée avant tout autre appel RTC</remarks>
// <param name="pConfigTable">Tableau de configuration</param>
// <param name="pBKG_TCBAddr">pBKG_TCBAddr</param>
// <param name="pCLK_TCBAddr">pCLK_TCBAddr</param>
// <returns>cOK</returns>
far InitKernel(
		struct tConfigTable far * pConfigTable,
		DoubleWord far *          pBKG_TCBAddr,
		DoubleWord far *          pCLK_TCBAddr
)
{
	UINT					i								= 0;
	SIZE_T				szMaxKernelSize			= 0;
	SIZE_T				szInitialKernelSize		= 0;

#ifdef USE_MULTIMEDIA_TIMER
	MMRESULT mmTimerCap = 0;
#endif

	/* Initialisation du debug */
	D(DL_MIN, RTC_DEBUG_INIT());

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Enter", __FILE__, __LINE__));	

	/* Test de la table de configuration */
	if (pConfigTable == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] pConfigTable == NULL", __FILE__, __LINE__));
		return cExceptionOccurred;
	}

	szMaxKernelSize =			sizeof(KERNEL)													+ 
									sizeof(struct tConfigTable)								+ 
									sizeof(PTACHE)			* pConfigTable->NbTasks			+
									sizeof(TACHE)			* pConfigTable->NbTasks			+
									sizeof(PDELAY)			* pConfigTable->NbDelays		+
									sizeof(DELAY)			* pConfigTable->NbDelays		+
									sizeof(PSEMAPHORE)	* pConfigTable->NbSemaphores	+
									sizeof(SEMAPHORE)		* pConfigTable->NbSemaphores  +
									sizeof(PMAILBOX)		* pConfigTable->NbMailBoxes	+
									sizeof(MAILBOX)		* pConfigTable->NbMailBoxes;

	szInitialKernelSize =	sizeof(KERNEL)													+ 
									sizeof(struct tConfigTable)								+ 
									sizeof(PTACHE)			* pConfigTable->NbTasks			+
									sizeof(PDELAY)			* pConfigTable->NbDelays		+
									sizeof(PSEMAPHORE)	* pConfigTable->NbSemaphores	+
									sizeof(PMAILBOX)		* pConfigTable->NbMailBoxes;

	//hMemoryHeap = HeapCreate(0, szInitialKernelSize, szMaxKernelSize);
	hMemoryHeap = HeapCreate( 0, szInitialKernelSize, 0 );

	/* Test du HeapCreate */
	if (hMemoryHeap == INVALID_HANDLE_VALUE ||
		 hMemoryHeap == NULL) 
	{ 
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Kernel HeapCreate error | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
		return cExceptionOccurred;
	}

	/* Allocation du Kernel */
	Kernel = (PKERNEL) HeapAlloc( hMemoryHeap, HEAP_ZERO_MEMORY, sizeof( KERNEL ) );

	/* Test du HeapAlloc */
	if (Kernel == NULL) 
	{ 
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Kernel HeapAlloc error | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
		return cExceptionOccurred;
	}

	/* Allocation de tConfigTable */
	Kernel->ConfigTable = (struct tConfigTable *) HeapAlloc( hMemoryHeap, HEAP_ZERO_MEMORY, sizeof( struct tConfigTable ) );

	/* Test du HeapAlloc */
	if ( Kernel->ConfigTable == NULL ) 
	{ 
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Kernel->ConfigTable HeapAlloc error | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
		return cExceptionOccurred;
	}

	/* Copie de la table de configuration (pour le partage) */
	CopyMemory( Kernel->ConfigTable, pConfigTable, sizeof( struct tConfigTable ) );

	/* Initialisation du Kernel */
   Kernel->BKG_TCBAddr					= pBKG_TCBAddr = NULL;
	Kernel->CLK_TCBAddr					= pCLK_TCBAddr = NULL;
	Kernel->bKernelOk						= FALSE;
	/* Initialisation de la taille maximum du Kernel */
	Kernel->szMaxKernelSize				= szMaxKernelSize;

	RtcDebugger.dwSharedDataSize		=	sizeof(DEBUGGER_SHARED_DATA)											+
													sizeof(SHAREDDATA_TASK)			* pConfigTable->NbTasks			+
													sizeof(SHAREDDATA_DELAY)		* pConfigTable->NbDelays		+
													sizeof(SHAREDDATA_SEMAPHORE)	* pConfigTable->NbSemaphores	+
													sizeof(SHAREDDATA_MAILBOX)		* pConfigTable->NbMailBoxes;

	RtcDebugger.bDebuggerExit			= FALSE;
	RtcDebugger.hThread					= INVALID_HANDLE_VALUE;
	RtcDebugger.lpThreadAttributes	= NULL;
	RtcDebugger.dwStackSize				= 0;
	RtcDebugger.lpStartAddress			= (LPTHREAD_START_ROUTINE) ThreadRtcDebugger;
	RtcDebugger.lpParameter				= NULL;
	RtcDebugger.dwCreationFlags		= 0;			/* Start immediately */
	RtcDebugger.ThreadId					= 0;

	/* Création du thread de debug RTC */
	RtcDebugger.hThread = CreateThread( RtcDebugger.lpThreadAttributes,
													RtcDebugger.dwStackSize,
													RtcDebugger.lpStartAddress,
													RtcDebugger.lpParameter,
													RtcDebugger.dwCreationFlags,
													&RtcDebugger.ThreadId
													);

	/* Test du CreateThread */
	if (RtcDebugger.hThread == INVALID_HANDLE_VALUE ||
		 RtcDebugger.hThread == NULL) 
	{ 
		OutputDebugString("Erreur CreateThread");
		/* En cas d'erreur, en continu... */
	}

	/* Test de la taille du DataSegment */
	if ( (((Kernel->ConfigTable->MaxPriority + 1 ) * 2 )
				+ ( ( Kernel->ConfigTable->NbTasks + 2 ) * ( 36 + Kernel->ConfigTable->TCBExtSize ) )
				+ ( Kernel->ConfigTable->NbSemaphores * 8 )
				+ ( Kernel->ConfigTable->NbMailBoxes * 8 )
				+ ( Kernel->ConfigTable->NbMessages * 8 )
				+ ( Kernel->ConfigTable->NbDelays * 8 )
				+ 38) > Kernel->ConfigTable->DataSegmentSize)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit cDataSegmentOverFlow : > Kernel->ConfigTable->DataSegmentSize", __FILE__, __LINE__));
		return cDataSegmentOverFlow;
	}

	/* Test de la taille du DataSegment */
	if (Kernel->ConfigTable->DataSegmentSize > 0xFFFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit cDataSegmentOverFlow : Kernel->ConfigTable->DataSegmentSize > 0xFFFF", __FILE__, __LINE__));
		return cDataSegmentOverFlow;
	}

	/* Test si la priorité est comprise entre 0 et <MaxPriority> */
	if (Kernel->ConfigTable->ClockTaskPriority < 0 || Kernel->ConfigTable->ClockTaskPriority > Kernel->ConfigTable->MaxPriority)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit cBadClockTkPriority", __FILE__, __LINE__));
		return cBadClockTkPriority;
	}

#pragma message("[InitKernel] TODO: A TESTER en utilisateur !!!!!!!!!!!!!!!!!!!!!!!!!!")
	///* Attribution des droits utilisateurs pour utiliser les fortes priorités de process/thread et l'ajustement de l'heure/date */
	//{
	//	BOOL					bRes		= FALSE;
	//	HANDLE				hToken	= INVALID_HANDLE_VALUE;
	//	DWORD					dwSize	= sizeof( TOKEN_PRIVILEGES );
	//	TOKEN_PRIVILEGES	tp;				/* token provileges */
	//	TOKEN_PRIVILEGES	oldtp;			/* old token privileges */
	//	LUID					luidSystemTime;  
	//	LUID					luidIncBasePriority;

	//	bRes = OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken );

	//	/* Test de OpenProcessToken */
	//	if ( bRes == FALSE )
	//	{
	//		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : OpenProcessToken() == FALSE | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
	//		return cExceptionOccurred;
	//	}

	//	/* get the LUID (Locally Unique Identifier) of the privilege you want to adjust. */
	//	bRes = LookupPrivilegeValue( NULL, SE_SYSTEMTIME_NAME, &luidSystemTime );

	//	/* Test de LookupPrivilegeValue */
	//	if ( bRes == FALSE )
	//	{
	//		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : LookupPrivilegeValue() SE_SYSTEMTIME_NAME == FALSE | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
	//		CloseHandle( hToken );
	//		return cExceptionOccurred;
	//	}

	//	/* get the LUID (Locally Unique Identifier) of the privilege you want to adjust. */
	//	bRes = LookupPrivilegeValue( NULL, SE_INC_BASE_PRIORITY_NAME, &luidIncBasePriority );

	//	/* Test de LookupPrivilegeValue */
	//	if ( bRes == FALSE )
	//	{
	//		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : LookupPrivilegeValue() SE_INC_BASE_PRIORITY_NAME == FALSE | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
	//		CloseHandle( hToken );
	//		return cExceptionOccurred;
	//	}

		/**
		 * Pas bonne gestion de la taille mémoire !!!!!!!!!!!!!!!!!!!!!
		 */
		//ZeroMemory( &tp, sizeof( TOKEN_PRIVILEGES ) );
		//tp.PrivilegeCount					= 2;
		//tp.Privileges[0].Luid			= luidSystemTime;
		//tp.Privileges[0].Attributes	= SE_PRIVILEGE_ENABLED;
		//tp.Privileges[1].Luid			= luidIncBasePriority;
		//tp.Privileges[1].Attributes	= SE_PRIVILEGE_ENABLED;

		///* djust the tokens. */
		//bRes = AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof( TOKEN_PRIVILEGES ), &oldtp, &dwSize );

		///* Test de AdjustTokenPrivileges */
		//if ( bRes == FALSE )
		//{
		//	D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : AdjustTokenPrivileges() == FALSE | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
		//	CloseHandle( hToken );
		//	return cExceptionOccurred;
		//}
		
	//	CloseHandle( hToken );
	//}

	/* Classe de priorité */
	/* Initialisation de la classe de priorité */
	{
		BOOL bRes = FALSE;

		bRes = SetPriorityClass( GetCurrentProcess(), RTC_PRIORITY_CLASS );

		/* Test de SetPriorityClass */
		if ( bRes == FALSE )
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : SetPriorityClass() == FALSE | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
			return cExceptionOccurred;
		}
	}

	/* TACHES */
	/* Test si le nombre de tâche est compris entre 0 et 0xFFFF */
	if ( Kernel->ConfigTable->NbTasks <= 0 || Kernel->ConfigTable->NbTasks > 0xFFFF )
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : Kernel->ConfigTable->NbTasks <= 0 || Kernel->ConfigTable->NbTasks > 0xFFFF", __FILE__, __LINE__));
		return cExceptionOccurred;
	}

	/* On alloue un tableau de pointeur de tâche */
	Kernel->pTaches = (PTACHE *) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, Kernel->ConfigTable->NbTasks * sizeof(PTACHE));

	/* Test du HeapAlloc */
	if (Kernel->pTaches == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : HeapAlloc Kernel->pTaches == NULL", __FILE__, __LINE__));
		return cDataSegmentOverFlow;
	}

	/* MAILBOXES */
	/* Test si le nombre de mailbox est compris entre 0 et 0xFFFF */
	if (Kernel->ConfigTable->NbMailBoxes <= 0 || Kernel->ConfigTable->NbMailBoxes > 0xFFFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : Kernel->ConfigTable->NbMailBoxes <= 0 || Kernel->ConfigTable->NbMailBoxes > 0xFFFF", __FILE__, __LINE__));
		return cExceptionOccurred;
	}
	
	/* On alloue un tableau de pointeur de semaphore */
	Kernel->pMailBoxes = (PMAILBOX *) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, Kernel->ConfigTable->NbMailBoxes * sizeof(PMAILBOX));

	/* Test du HeapAlloc */
	if (Kernel->pMailBoxes == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : HeapAlloc Kernel->pMailBoxes == NULL | GetLastError=%d", __FILE__, __LINE__, GetLastError()));
		return cDataSegmentOverFlow;
	}

	/* SEMAPHORES */
	/* Test si le nombre de sémaphore est compris entre 0 et 0xFFFF */
	if (Kernel->ConfigTable->NbSemaphores <= 0 || Kernel->ConfigTable->NbSemaphores > 0xFFFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : Kernel->ConfigTable->NbSemaphores <= 0 || Kernel->ConfigTable->NbSemaphores > 0xFFFF", __FILE__, __LINE__));
		return cExceptionOccurred;
	}
	
	/* On alloue un tableau de pointeur de semaphore */
	Kernel->pSemaphores = (PSEMAPHORE *) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, Kernel->ConfigTable->NbSemaphores * sizeof(PSEMAPHORE));

	/* Test du HeapAlloc */
	if (Kernel->pSemaphores == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : HeapAlloc Kernel->pSemaphores == NULL", __FILE__, __LINE__));
		return cDataSegmentOverFlow;
	}

	/* DELAYS */
	/* Test si le nombre de timer est compris entre 0 et 0xFFFF */
	if (Kernel->ConfigTable->NbDelays <= 0 || Kernel->ConfigTable->NbDelays > 0xFFFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : Kernel->ConfigTable->NbDelays <= 0 || Kernel->ConfigTable->NbDelays > 0xFFFF", __FILE__, __LINE__));
		return cExceptionOccurred;
	}
	
	/* On alloue un tableau de pointeur de timers */
	Kernel->pDelays = (PDELAY *) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, Kernel->ConfigTable->NbDelays * sizeof(PDELAY));

	/* Test du HeapAlloc */
	if (Kernel->pDelays == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : HeapAlloc Kernel->pDelays == NULL", __FILE__, __LINE__));
		return cDataSegmentOverFlow;
	}

#ifdef USE_MULTIMEDIA_TIMER
	// Set resolution to the minimum supported by the system
	mmTimerCap  = timeGetDevCaps( &Kernel->MultimediaTimerCapability, sizeof( TIMECAPS ) );

	/* Test de timeGetDevCaps */
	if (mmTimerCap != TIMERR_NOERROR)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitKernel)] Exit : mmTimerCap != TIMERR_NOERROR", __FILE__, __LINE__));
		return cExceptionOccurred;
	}

	/* Initialisation de la résolution du timer multimédia (dépend +- du hard) */
	Kernel->MultimediaTimerResolution = min(max(Kernel->MultimediaTimerCapability.wPeriodMin, 0), Kernel->MultimediaTimerCapability.wPeriodMax);
#endif

	/* REGIONS CRITIQUE */
	/* Initialisation de la section critique de la région critique de RTC */
	InitializeCriticalSection(&Kernel->Region.csCriticalRegion);
	Kernel->Region.dwInRegion				= 0;		/* Au lancement, on n'est pas en région */
	Kernel->Region.bSafeRegionActive		= TRUE;	/* On active la protection des régions (UnSafeRegion() pour désactiver) */
	Kernel->Region.dwTaskNoInRegion		= -2;		/* On n'est pas en région donc pas de tâche associée */
	Kernel->Region.hRegionEvent			= CreateEvent(NULL, TRUE, TRUE, NULL);

	if (Kernel->Region.hRegionEvent == INVALID_HANDLE_VALUE ||
		 Kernel->Region.hRegionEvent == NULL) 
	{
		D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit : RegionEvent == NULL", __FILE__, __LINE__));
		return cExceptionOccurred;
	}


	/* Le kernel a été correctement initialisé */
	Kernel->bKernelOk = TRUE;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitKernel] Exit normaly", __FILE__, __LINE__));

	return cOK;
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
void DeinitKernel()
{
	unsigned int i = 0;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [DeinitKernel] Entry", __FILE__, __LINE__));

	printf( "[DeinitKernel] Arret des timers..." );

	/* Libération des Delays */
	for (i = 0; i < Kernel->ConfigTable->NbDelays; i++)
	{
		//MMRESULT			idEvent;
		//MMRESULT			mmTimer;
		//HANDLE			hDelay;	
		if ( Kernel->pDelays[i] != NULL )
		{
			StopDelay( i );
			//DeleteTimerQueueTimer( NULL, Kernel->pDelays[i]->hDelay, INVALID_HANDLE_VALUE );
//			CloseHandle( Kernel->pDelays[i]->hDelay ); 
//			DeleteCriticalSection( &Kernel->pDelays[i]->csTimerLock ); // fait planter si un timer claque après cette ligne
//			HeapFree( hMemoryHeap, 0, Kernel->pDelays[i] );
		}
	}

	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	printf( "\t\t\t\tOK\n" );
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );

	printf( "[DeinitKernel] Arret des taches..." );

	/* Libération des tâches */
	for (i = 0; i < Kernel->ConfigTable->NbTasks; i++)
	{
		if ( Kernel->pTaches[i] != NULL )
		{
			StopTask( i );
			TerminateThread( Kernel->pTaches[i]->WindowsThread.hThread, 0 );
			WaitForSingleObject( Kernel->pTaches[i]->WindowsThread.hThread, INFINITE );
			CloseHandle( Kernel->pTaches[i]->WindowsThread.hThread );
			CloseHandle( Kernel->pTaches[i]->WindowsThread.hEvent );
			DeleteCriticalSection( &Kernel->pTaches[i]->WindowsThread.csEvent );
			HeapFree( hMemoryHeap, 0, Kernel->pTaches[i] );
		}
	}

	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	printf( "\t\t\t\tOK\n" );
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );

	printf( "[DeinitKernel] Liberation des boites aux lettres..." );

	/* Libération des MailBoxes */
	for (i = 0; i < Kernel->ConfigTable->NbMailBoxes; i++)
	{
		if ( Kernel->pMailBoxes[i] != NULL )
		{
			CloseHandle( Kernel->pMailBoxes[i]->hEvent );
			stack_destroy( &Kernel->pMailBoxes[i]->pile );
			DeleteCriticalSection( &Kernel->pMailBoxes[i]->csMailBox );
			HeapFree( hMemoryHeap, 0, Kernel->pMailBoxes[i] );
		}
	}

	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	printf( "\t\tOK\n" );
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );

	printf( "[DeinitKernel] Liberation des semaphores..." );

	/* Libération des Semaphores */
	for (i = 0; i < Kernel->ConfigTable->NbSemaphores; i++)
	{
		if ( Kernel->pSemaphores[i] != NULL )
		{
			CloseHandle( Kernel->pSemaphores[i]->hSemaphore );
			DeleteCriticalSection( &Kernel->pSemaphores[i]->csSemaphore );
			HeapFree( hMemoryHeap, 0, Kernel->pSemaphores[i] );
		}
	}

	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	printf( "\t\t\tOK\n" );
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );


	printf( "[DeinitKernel] Arret du thread de debug..." );

	/* Libération du Debugger */
	RtcDebugger.bDebuggerExit = TRUE;
	WaitForSingleObject( RtcDebugger.hThread, 1500 );
	CloseHandle( RtcDebugger.hThread );

	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	printf( "\t\t\tOK\n" );
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );


	printf( "[DeinitKernel] Liberation des regions RTC..." );

	/* Libération des Régions */
	DeleteCriticalSection( &Kernel->Region.csCriticalRegion );
	CloseHandle( Kernel->Region.hRegionEvent );

	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	printf( "\t\t\tOK\n" );
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );


	printf( "[DeinitKernel] Liberation de la memoire du Kernel RTC..." );

	/* Libération du Kernel */
	HeapFree( hMemoryHeap, 0, Kernel->pTaches );
	HeapFree( hMemoryHeap, 0, Kernel->pMailBoxes );
	HeapFree( hMemoryHeap, 0, Kernel->pSemaphores );
	HeapFree( hMemoryHeap, 0, Kernel->pDelays );
	HeapFree( hMemoryHeap, 0, Kernel->ConfigTable );
	// MultimediaTimerCapability
	HeapFree( hMemoryHeap, 0, Kernel );

	HeapDestroy(hMemoryHeap);

	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY );
	printf( "\tOK\n" );
	SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [DeinitKernel] Exit normaly", __FILE__, __LINE__));

	D(DL_MIN, RTC_DEBUG_DEINIT());
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
far InitTask(
     Word				TaskNumber,             /* range 0..NbTasks-1 */
     Word				Priority,               /* range 0..MaxPriority */
     Word (far *		StartAddress)(),		/* pointer to the fonction which
												/* contains the code of the task */
     Word far *			StackPtr,				/* pointer to the end of the stack */
     Word				DataSegment,			/* initial DS register value for the task */
     Boolean			TaskUsesNDP,			/* Numeric Data Processor: 8087 or 80287 */
												/* = 1 if the task uses NDP, = 0 otherwise */
///	 void (far *)       /* Hook */ (),
	 void (far *		Hook)(),
	 DoubleWord         HookParam,
	 DoubleWord far *   pTCBAddr
)
{
	BOOL	bRes = FALSE;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Enter", __FILE__, __LINE__, TaskNumber));

	/* On vérifie que le Kernel a bien été initialisé */
	if (Kernel->bKernelOk == FALSE)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit : Kernel->bKernelOk == FALSE", __FILE__, __LINE__, TaskNumber));
		return cExceptionOccurred;
	}

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber > Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber));
		return cBadTaskNumber;
	}

	/* On vérifie que la priorité est comprise entre 0 et <MaxPriority> */
	if (Priority < 0 || Priority >= Kernel->ConfigTable->MaxPriority)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit : Priority < 0 || Priority >= Kernel->ConfigTable->MaxPriority", __FILE__, __LINE__, TaskNumber));
		return cBadPriority;
	}

	/* on alloue la mémoire pour une tâche */
	Kernel->pTaches[TaskNumber] = (PTACHE) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, sizeof(TACHE));

	/* Test du HeapAlloc */
	if (Kernel->pTaches[TaskNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit : HeapAlloc Kernel->pTaches[TaskNumber] == NULL", __FILE__, __LINE__, TaskNumber));
		return cTaskNotInitialized;
	}

	/* Tache RTC */
	Kernel->pTaches[TaskNumber]->RtcTask.TaskNumber						= TaskNumber;
	Kernel->pTaches[TaskNumber]->RtcTask.Priority						= Priority;
	Kernel->pTaches[TaskNumber]->RtcTask.DataSegment					= DataSegment;
	Kernel->pTaches[TaskNumber]->RtcTask.StackPtr						= StackPtr;
	Kernel->pTaches[TaskNumber]->RtcTask.TaskUsesNDP					= TaskUsesNDP;
	Kernel->pTaches[TaskNumber]->RtcTask.Hook								= Hook;
	Kernel->pTaches[TaskNumber]->RtcTask.HookParam						= HookParam;
	Kernel->pTaches[TaskNumber]->RtcTask.pTCBAddr						= pTCBAddr;
	Kernel->pTaches[TaskNumber]->RtcTask.Status							= NonOperational;
	Kernel->pTaches[TaskNumber]->RtcTask.StatusBeforeRegion			= NonOperational;

	/* Thread Windows */
	Kernel->pTaches[TaskNumber]->WindowsThread.dwCreationFlags		= CREATE_SUSPENDED;
	Kernel->pTaches[TaskNumber]->WindowsThread.dwStackSize			= RTC_TASK_STACK_SIZE;
	Kernel->pTaches[TaskNumber]->WindowsThread.lpParameter			= NULL;
	Kernel->pTaches[TaskNumber]->WindowsThread.lpStartAddress		= (LPTHREAD_START_ROUTINE) StartAddress;
	Kernel->pTaches[TaskNumber]->WindowsThread.lpThreadAttributes	= NULL;
	Kernel->pTaches[TaskNumber]->WindowsThread.ThreadId				= 0;
	Kernel->pTaches[TaskNumber]->WindowsThread.EventData				= 0x00000000;

	/* Initialisation de la section critique associé aux données de l'évènement */
	InitializeCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	/* Création du thread Windows */
	Kernel->pTaches[TaskNumber]->WindowsThread.hThread	
		
								= CreateThread(Kernel->pTaches[TaskNumber]->WindowsThread.lpThreadAttributes,
													Kernel->pTaches[TaskNumber]->WindowsThread.dwStackSize,
													Kernel->pTaches[TaskNumber]->WindowsThread.lpStartAddress,
													Kernel->pTaches[TaskNumber]->WindowsThread.lpParameter,
													Kernel->pTaches[TaskNumber]->WindowsThread.dwCreationFlags,
													&Kernel->pTaches[TaskNumber]->WindowsThread.ThreadId
													);

	/* On test la création du thread */
	if (Kernel->pTaches[TaskNumber]->WindowsThread.hThread == INVALID_HANDLE_VALUE ||
		 Kernel->pTaches[TaskNumber]->WindowsThread.hThread == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit : CreateThread Kernel->pTaches[TaskNumber]->WindowsThread.hThread == INVALID_HANDLE_VALUE | GetLastError=%d", __FILE__, __LINE__, TaskNumber, GetLastError()));
		return cTaskNotInitialized;
	}

	/* Création de l'évènement associé à la tâche */
	Kernel->pTaches[TaskNumber]->WindowsThread.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	if (Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE ||
		 Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit : CreateEvent Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE | GetLastError=%d", __FILE__, __LINE__, TaskNumber, GetLastError()));
		return cTaskNotInitialized;
	}

	/* Initialisation de la priorité de la tâche */
	bRes = SetThreadPriority( Kernel->pTaches[TaskNumber]->WindowsThread.hThread, RtcToWindowsPriority( Priority ) );

	/* Test de SetThreadPriority */
	if ( bRes == FALSE )
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit : SetThreadPriority() == FALSE | GetLastError=%d", __FILE__, __LINE__, TaskNumber, GetLastError()));
		return cTaskNotInitialized;
	}

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitTask(%d)] Exit normaly", __FILE__, __LINE__, TaskNumber));

	return cOK;
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
far StartTask(
     Word TaskNumber
)
{
	DWORD	ret = 0;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StartTask(%d)] Enter", __FILE__, __LINE__, TaskNumber));

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartTask(%d)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber));
		return cBadTaskNumber;
	}

	/* On test si la tâche a été allouée */
	if (Kernel->pTaches[TaskNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartTask(%d)] Exit : Kernel->pTaches[TaskNumber] == NULL", __FILE__, __LINE__, TaskNumber));
		return cBadTaskNumber;
	}

	//if ( Kernel->dwInRegion == 0 )
	//{
		/* On lance le thread */
		ret = ResumeThread(Kernel->pTaches[TaskNumber]->WindowsThread.hThread);

		/* Test du lancement du thread */
		if (ret == -1)
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartTask(%d)] Exit : ResumeThread() == -1 | GetLastError=%d", __FILE__, __LINE__, TaskNumber, GetLastError()));
			return cExceptionOccurred;
		}

		/* Mise à jour de l'état de la tâche */
		Kernel->pTaches[TaskNumber]->RtcTask.Status = Ready;
	//} 
	//else
	//{
	//	/* Mise à jour de l'état de la tâche */
	//	Kernel->pTaches[TaskNumber]->RtcTask.Status = Suspend;
	//}

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StartTask(%d)] Exit normaly", __FILE__, __LINE__, TaskNumber));

	return cOK;
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
far StopTask(
     Word TaskNumber
)
{
	DWORD ret = 0;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StopTask(%d)] Enter", __FILE__, __LINE__, TaskNumber));

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopTask(%d)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber));
		return cBadTaskNumber;
	}

	/* On test si la tâche a été allouée */
	if (Kernel->pTaches[TaskNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopTask(%d)] Exit : Kernel->pTaches[TaskNumber] == NULL", __FILE__, __LINE__, TaskNumber));
		return cBadTaskNumber;
	}

	/* On arrete le thread */
	ret = SuspendThread(Kernel->pTaches[TaskNumber]->WindowsThread.hThread);

	/* test de l'arret du thread */
	if (ret == -1)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopTask(%d)] Exit : SuspendThread() == -1 | GetLastError=%d", __FILE__, __LINE__, TaskNumber, GetLastError()));
		return cExceptionOccurred;
	}

	/* Mise à jour de l'état de la tâche */
	Kernel->pTaches[TaskNumber]->RtcTask.Status = NonOperational;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StopTask(%d)] Exit normaly", __FILE__, __LINE__, TaskNumber));

	return cOK;
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
far Terminate( void )
{
	Word wRet = cExceptionOccurred;
	Word CurTask = CurrentTask();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [Terminate(%d)] Enter", __FILE__, __LINE__, CurTask));

	/* On termine brutalement le thread courant */
	wRet = (TRUE == TerminateThread(GetCurrentThread(), 0)) ? cOK : cExceptionOccurred;

	/* Mise à jour de l'état de la tâche */
	Kernel->pTaches[CurTask]->RtcTask.Status = NonOperational;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [Terminate] Exit normaly", __FILE__, __LINE__));

	return wRet;
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
Word far CurrentTask( void )
{
	DWORD	current = 0;
	UINT	i		= 0;

//	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [CurrentTask] Enter", __FILE__, __LINE__));

	/* Récupération de l'identifiant du thread courant */
	current = GetCurrentThreadId();

	/* On liste toutes les tâches à la recherche de cet identifiant */
	for (i = 0; i < Kernel->ConfigTable->NbTasks; i++)
	{
		/* on vérifie que la tâche est bien allouée */
		if (Kernel->pTaches[i] != NULL)
		{
			if (current == Kernel->pTaches[i]->WindowsThread.ThreadId)
			{
				//D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [CurrentTask] Exit normaly", __FILE__, __LINE__));
				return i;	/* On retourne le numéro de la tâche courante */
			}
		}
	}

	D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [CurrentTask] Exit : Tâche non trouvée", __FILE__, __LINE__));

#pragma message ("@INFO: [CurrentTask] Ne peut pas retourner Clock (-2) Task")
	/* returns TaskNumber for an user defined task */
	/*         -1     for the BackGround Task      */
	/*         -2     for the Clock Task           */
	return -1;
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

tTaskState far TaskState(
     Word TaskNumber
)
{
	tTaskState status = -1;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TaskState(TaskNumber=%d)] Enter", __FILE__, __LINE__, TaskNumber));

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [TaskState(TaskNumber=%d)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber));
		return cBadTaskNumber;
	}

	/* On test si la tâche a été allouée */
	if (Kernel->pTaches[TaskNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [TaskState(TaskNumber=%d)] Exit : Kernel->pTaches[TaskNumber] == NULL", __FILE__, __LINE__, TaskNumber));
		return cBadTaskNumber;
	}

	/* On récupère le status */
	status = (TaskNumber == CurrentTask()) ? cCurrent : (tTaskState) Kernel->pTaches[TaskNumber]->RtcTask.Status;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TaskState(TaskNumber=%d)] Exit normaly (status=%d)", __FILE__, __LINE__, TaskNumber, status));

	/* returns 0 = cCurrent        */
	/*         1 = cReady          */
	/*         2 = cWaiting        */
	/*         3 = cNonOperational */	
	return status;
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
Word far TaskPriority(
     Word TaskNumber  /* -1 may be used to refer to current task */
)
{
	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TaskPriority] Enter : TaskNumber='%d'", __FILE__, __LINE__, TaskNumber));

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks>, -1 pour la tache courante */
	if (TaskNumber < -1 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [TaskPriority] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__));
		return cBadTaskNumber;
	}

	/* On test si on veut récupérer la priorité de la tache courante */
	if (TaskNumber == -1)
	{
		TaskNumber = CurrentTask();
	}

	/* On test si la tâche a été allouée */
	if (Kernel->pTaches[TaskNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [TaskPriority] Exit : Kernel->pTaches[TaskNumber] == NULL", __FILE__, __LINE__));
		return cBadTaskNumber;
	}

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TaskPriority] Exit normaly", __FILE__, __LINE__));

	return Kernel->pTaches[TaskNumber]->RtcTask.Priority; /* returns Priority */
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
far ChangePriority(
     Word TaskNumber,   /* -1 may be used to refer to current task */
     Word NewPriority
)
{
	BOOL bRes = FALSE;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ChangePriority] Enter", __FILE__, __LINE__));

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks>, -1 pour la tache courante */
	if (TaskNumber < -1 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [ChangePriority] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__));
		return cBadTaskNumber;
	}

	/* On vérifie que la priorité est comprise entre 0 et <MaxPriority> */
	if (NewPriority < 0 || NewPriority >= Kernel->ConfigTable->MaxPriority)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [ChangePriority] Exit : NewPriority < 0 || NewPriority >= Kernel->ConfigTable->MaxPriority", __FILE__, __LINE__));
		return cBadPriority;
	}

	/* On test si on veut changer la priorité de la tache courante */
	if (TaskNumber == -1)
	{
		TaskNumber = CurrentTask();
	}

	/* On test si la tâche a été allouée */
	if (Kernel->pTaches[TaskNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [ChangePriority] Exit : Kernel->pTaches[TaskNumber] == NULL", __FILE__, __LINE__));
		return cBadTaskNumber;
	}

	bRes = SetThreadPriority(Kernel->pTaches[TaskNumber]->WindowsThread.hThread, RtcToWindowsPriority(NewPriority));

	/* On modifie la priorité de la tâche */
	if ( bRes == FALSE )
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [ChangePriority] Exit : SetThreadPriority() == FALSE - %d", __FILE__, __LINE__, GetLastError()));
		return cTaskNotInitialized;
	}

	/* On mets à jour la priorité RTC */
    Kernel->pTaches[TaskNumber]->RtcTask.Priority = NewPriority;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ChangePriority] Exit normaly", __FILE__, __LINE__));

	return cOK;
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
far ForceExceptionHandler(
     Word TaskNumber,
     Word (far * ExceptionHandlerPtr)(),
     Word Parameter
)
{
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ForceExceptionHandler] Enter", __FILE__, __LINE__));

	/* XXX */

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ForceExceptionHandler] Exit normaly", __FILE__, __LINE__));

	return cOK; // cCurrTask;
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
far ExitExceptionHandler( void )
{
	/* XXX */
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ExitExceptionHandler] Enter and Exit", __FILE__, __LINE__));
	return (Word) 0;
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
Word far DataSegment( void )
{
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [DataSegment] Enter and Exit", __FILE__, __LINE__));
	/* XXX */
	return (Word) 0;
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
far EnterRegion( void )
{
	DWORD	dwRet		= 0;
	DWORD	current	= 0;
	//Word	ret		= 0;
	unsigned int i = 0;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [EnterRegion(%d)] Enter", __FILE__, __LINE__, Kernel->Region.dwInRegion));

	EnterCriticalSection(&Kernel->Region.csCriticalRegion);

	if (Kernel->Region.dwInRegion == 0)
	{
		BOOL bRes = FALSE;
		DWORD toto = 0;

		/* Récupération de l'identifiant de la tâche courante */
		Kernel->Region.dwTaskNoInRegion = CurrentTask();
		
		bRes = ResetEvent( Kernel->Region.hRegionEvent );

		if ( bRes == FALSE )
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [EnterRegion(%d)] Exit : ResetEvent == FALSE - GetLastError=%d", __FILE__, __LINE__, Kernel->Region.dwInRegion, GetLastError()));
			LeaveCriticalSection(&Kernel->Region.csCriticalRegion);
			return cExceptionOccurred;
		}
	}

	Kernel->Region.dwInRegion++;

	LeaveCriticalSection(&Kernel->Region.csCriticalRegion);

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [EnterRegion(%d)] Exit normaly", __FILE__, __LINE__, Kernel->Region.dwInRegion));

	return cOK;
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
far LeaveRegion( void )
{
	DWORD	current	= 0;
	unsigned int i = 0;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [LeaveRegion(%d)] Enter", __FILE__, __LINE__, Kernel->Region.dwInRegion));

	EnterCriticalSection(&Kernel->Region.csCriticalRegion);

	if (Kernel->Region.dwInRegion == 0)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [LeaveRegion(%d)] Exit : InRegion == 0", __FILE__, __LINE__, Kernel->Region.dwInRegion));
		LeaveCriticalSection(&Kernel->Region.csCriticalRegion);
		return cExceptionOccurred;
	}

	Kernel->Region.dwInRegion--;

	if (Kernel->Region.dwInRegion == 0)
	{
		BOOL bRes = FALSE;

		bRes = SetEvent( Kernel->Region.hRegionEvent );

		if ( bRes == FALSE )
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [LeaveRegion(%d)] Exit : SetEvent == FALSE - GetLastError=%d", __FILE__, __LINE__, Kernel->Region.dwInRegion, GetLastError()));
			LeaveCriticalSection(&Kernel->Region.csCriticalRegion);
			return cExceptionOccurred;
		}

		Kernel->Region.dwTaskNoInRegion = -2;
	}

	LeaveCriticalSection(&Kernel->Region.csCriticalRegion);

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [LeaveRegion(%d)] Exit normaly", __FILE__, __LINE__, Kernel->Region.dwInRegion));

	return cOK;
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
far SignalEvent(
     Word TaskNumber,
     tEvent EventNumber   /* range 0..15 */
)
{
	BOOL				bRet		= FALSE;
	unsigned int	EventBit = (1 << EventNumber);

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d, fromtask=%d, bit=%x)] Enter", __FILE__, __LINE__, TaskNumber, EventNumber, CurrentTask(), EventBit));

	/* On vérifie que le numéro de la tâche est compris entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber, EventNumber));
		return cBadTaskNumber;
	}

	/* On test si la tâche a été allouée */
	if (Kernel->pTaches[TaskNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] Exit : Kernel->pTaches[TaskNumber] == NULL", __FILE__, __LINE__, TaskNumber, EventNumber));
		return cBadTaskNumber;
	}

	/* On vérifie que le numéro de l'évènement est compris entre 0 et sizeof(tEvent) * 8 */
	if (EventNumber < 0 || EventNumber >= 32)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] Exit : EventNumber < 0 || EventNumber >= 0xFFFFFFFF", __FILE__, __LINE__, TaskNumber, EventNumber));
		return cBadTaskNumber;
	}

	/* l'évènement a été cloturé */
	if (Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE ||
		Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] Exit : Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE", __FILE__, __LINE__, TaskNumber, EventNumber));
		return cBadTaskNumber;
	}

	/* On test pour savoir si l'évènement à déjà été signalé */
	if ((Kernel->pTaches[TaskNumber]->WindowsThread.EventData & EventBit) == EventBit)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] EventData=%x | Event already signaled", __FILE__, __LINE__, TaskNumber, EventNumber, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
	}
	
	EnterCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	/* Mise à jour des données de l'évènement */
	Kernel->pTaches[TaskNumber]->WindowsThread.EventData |= EventBit;

	D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] EventData=%x", __FILE__, __LINE__, TaskNumber, EventNumber, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));

	/* On envoi l'évènement */
	bRet = SetEvent(Kernel->pTaches[TaskNumber]->WindowsThread.hEvent);

	LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	/* On test l'envoi de l'évènement */
	if ( bRet == FALSE )
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] Exit : SetEvent() == FALSE | GetLastError=%d", __FILE__, __LINE__, TaskNumber, EventNumber, GetLastError()));
		return cExceptionOccurred;
	}

    D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [SignalEvent(task=%d, eventno=%d)] Exit normaly", __FILE__, __LINE__, TaskNumber, EventNumber));

	return cOK;
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
Word far WaitEventsMs(
     tEventList EventsAwaited,
     tTimeOut TimeOut,
     tEventList far * pEventsOccurred
)
{
	DWORD	dwRet			= 0;
	DWORD	timeout		= (TimeOut == 0) ? INFINITE : TimeOut;
	Word	TaskNumber	= CurrentTask();
	Word	ret			= cExceptionOccurred;
	BOOL	bFlagLoop	= FALSE;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Enter", __FILE__, __LINE__, TaskNumber, EventsAwaited));

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber, EventsAwaited));
		return cBadTaskNumber;
	}

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (EventsAwaited < 0 || EventsAwaited > 0xFFFFFFFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : EventsAwaited < 0 || EventsAwaited >= 0xFFFFFFFF", __FILE__, __LINE__, TaskNumber, EventsAwaited));
		return cBadTaskNumber;
	}

	/* l'évènement a été cloturé */
	if (Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE ||
		Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE", __FILE__, __LINE__, TaskNumber, EventsAwaited));
		return cBadTaskNumber;
	}

	/* On test si la variable passée en paramètre a été allouée*/
	if (pEventsOccurred == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : pEventsOccurred == NULL", __FILE__, __LINE__, TaskNumber));
		return cExceptionOccurred;
	}
	
	/* Initialisation de la variable passée en paramètre */
	*pEventsOccurred = 0x00000000;

	EnterCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	/* On test si les évènements courants sont ceux attendu */
	if ((Kernel->pTaches[TaskNumber]->WindowsThread.EventData & EventsAwaited) != 0x00000000)
	{
		/* On recopie les évènements arrivés dans la variable passée en paramètre */
		*pEventsOccurred = Kernel->pTaches[TaskNumber]->WindowsThread.EventData & EventsAwaited;
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x, pEventsOccurred=%x)] Events == EventsAwaited -> pas de WaitForSingleObject", __FILE__, __LINE__, TaskNumber, EventsAwaited, *pEventsOccurred));
		LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);
		LockInRegion();
		return cOK;	/* Pas besoin d'attendre */
	}

	LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	do
	{
		/* Mise à jour de l'état de la tâche */
		Kernel->pTaches[TaskNumber]->RtcTask.Status = Waiting;

		bFlagLoop = FALSE;

		/* On attend l'évènement */
		dwRet = WaitForSingleObject(Kernel->pTaches[TaskNumber]->WindowsThread.hEvent, timeout);

		/* Mise à jour de l'état de la tâche */
		Kernel->pTaches[TaskNumber]->RtcTask.Status = Ready;

		/* On test l'object que l'on a recu */
		switch (dwRet)
		{
		case WAIT_ABANDONED:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : WaitForSingleObject == WAIT_ABANDONED | GetLastError=%d", __FILE__, __LINE__, TaskNumber, EventsAwaited));
			ret = cTimeOut;
			break;

		case WAIT_TIMEOUT:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : WaitForSingleObject == WAIT_TIMEOUT", __FILE__, __LINE__, TaskNumber, EventsAwaited));
			ret = cTimeOut;
			break;

		case WAIT_OBJECT_0:	/* Les évènements sont arrivées */
			EnterCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);
			*pEventsOccurred = Kernel->pTaches[TaskNumber]->WindowsThread.EventData & EventsAwaited;

			if ((Kernel->pTaches[TaskNumber]->WindowsThread.EventData & EventsAwaited) != 0x00000000)
			{
				/* Si c'est les évènements que l'on attendait, on les remets à zéro */
				D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x, pEventsOccurred=%d)] Events == EventsAwaited", __FILE__, __LINE__, TaskNumber, EventsAwaited, *pEventsOccurred));
				ret = cOK;
			}
			else
			{
				D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x, pEventsOccurred=%x, EventData=%x)] Events != EventsAwaited", __FILE__, __LINE__, TaskNumber, EventsAwaited, *pEventsOccurred, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
				bFlagLoop = TRUE; /* On veux boucler */
			}

			LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);
			break;

		case WAIT_FAILED:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : WaitForSingleObject == WAIT_FAILED | GetLastError=%d", __FILE__, __LINE__, TaskNumber, EventsAwaited, GetLastError()));
			ret = cExceptionOccurred;
			break;

		default:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x)] Exit : WaitForSingleObject == %d | GetLastError=%d", __FILE__, __LINE__, TaskNumber, EventsAwaited, dwRet, GetLastError()));
			ret = cExceptionOccurred;
			break;
		}

	} while ( bFlagLoop == TRUE );


	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [WaitEvents(task=%d, EventsAwaited=%x, pEventsOccurred=%d)] Exit normaly", __FILE__, __LINE__, TaskNumber, EventsAwaited, *pEventsOccurred));

	LockInRegion();

	return ret; /* returns status = cOK or cTimeOut or cExceptionOccurred */
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
Word far WaitEvents(
     tEventList EventsAwaited,
     tTimeOut TimeOut,
     tEventList far * pEventsOccurred
)
{
	return WaitEventsMs( EventsAwaited, TimeOut * RTCTICKTOMS, pEventsOccurred );
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
Boolean far EventsOccurred(
     tEventList EventList
)
{
	Word TaskNumber = CurrentTask();
	BOOL bRet		= FALSE;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [EventsOccurred(task=%d, EventData=%d)] Enter", __FILE__, __LINE__, TaskNumber, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
	
	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [EventsOccurred(task=%d, EventData=%d)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
		return FALSE;
	}

	EnterCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (EventList < 0 || EventList >= 0xFFFFFFFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [EventsOccurred(task=%d, EventData=%d)] Exit : EventList < 0 || EventList >= 0xFFFFFFFF", __FILE__, __LINE__, TaskNumber, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
		LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);
		return FALSE;
	}

	/* l'évènement a été cloturé */
	if (Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE ||
		Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [EventsOccurred(task=%d, EventData=%d)] Exit : Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE", __FILE__, __LINE__, TaskNumber, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
		LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);
		return FALSE;
	}

	/* On test si les évènements demandés sont apparu */
	bRet = ((Kernel->pTaches[TaskNumber]->WindowsThread.EventData ^ EventList) == 0) ? TRUE : FALSE;

	LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [EventsOccurred(task=%d, EventData=%d)] Exit normaly", __FILE__, __LINE__, TaskNumber, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));

	return bRet;
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
far ClearEvents(
     tEventList EventList
)
{
	BOOL bRet		= FALSE;
	Word TaskNumber = CurrentTask();

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ClearEvents(task=%d, EventList=%d, EventData=%d)] Enter", __FILE__, __LINE__, TaskNumber, EventList, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
	
	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [ClearEvents(task=%d, EventList=%d, EventData=%d)] Exit : TaskNumber < 0 || TaskNumber >= Kernel->ConfigTable->NbTasks", __FILE__, __LINE__, TaskNumber, EventList, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
		return cExceptionOccurred;
	}

	/* On vérifie que le numéro de la tâche est comprise entre 0 et <NbTasks> */
	if (EventList < 0 || EventList >= 0xFFFFFFFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [ClearEvents(task=%d, EventList=%d, EventData=%d)] Exit : EventList < 0 || EventList >= 0xFFFFFFFF", __FILE__, __LINE__, TaskNumber, EventList, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
		return cExceptionOccurred;
	}

	/* l'évènement a été cloturé */
	if (Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE ||
		Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [ClearEvents(task=%d, EventList=%d, EventData=%d)] Exit : Kernel->pTaches[TaskNumber]->WindowsThread.hEvent == INVALID_HANDLE_VALUE", __FILE__, __LINE__, TaskNumber, EventList, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));
		return cExceptionOccurred;
	}

	EnterCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	/* On remets tout à zéro */
	Kernel->pTaches[TaskNumber]->WindowsThread.EventData &= ~EventList;

	LeaveCriticalSection(&Kernel->pTaches[TaskNumber]->WindowsThread.csEvent);

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ClearEvents(task=%d, EventList=%d, EventData=%d)] Exit normaly", __FILE__, __LINE__, TaskNumber, EventList, Kernel->pTaches[TaskNumber]->WindowsThread.EventData));

	return cOK;
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
far InitSemaphore(
     Word SemaphoreNumber,  /* range 0..NbSemaphores-1 */
     Word CountMaxValue     /* range 0..32767 */
)
{
	PSEMAPHORE tmp = NULL;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Enter", __FILE__, __LINE__, SemaphoreNumber));

	/* On vérifie que le Kernel a bien été initialisé */
	if (Kernel->bKernelOk == FALSE)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit : Kernel->bKernelOk == FALSE", __FILE__, __LINE__, SemaphoreNumber));
		return cExceptionOccurred;
	}

	/* On vérifie que CountMaxValue est compris entre 0 et 0x7FFF */
	if (CountMaxValue < 0 || CountMaxValue > 0x7FFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit : CountMaxValue < 0 || CountMaxValue > 0x7FFF", __FILE__, __LINE__, SemaphoreNumber));
		return cNegMaxValue;
	}

	/* On vérifie que SemaphoreNumber est compris entre 0 et <NbSemaphores> */
	if (SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit : SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores", __FILE__, __LINE__, SemaphoreNumber));
		return cBadSemaphoreNumber;
	}

	/* le semaphore a déjà été créé */
	if (Kernel->pSemaphores[SemaphoreNumber] != NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit : Le semaphore a déjà été créé", __FILE__, __LINE__, SemaphoreNumber));
		return cOK;
	}

	/* on alloue la mémoire pour un semaphore */
	Kernel->pSemaphores[SemaphoreNumber] = (PSEMAPHORE) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, sizeof(SEMAPHORE));

	/* On test le HeapAlloc */
	if (Kernel->pSemaphores[SemaphoreNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit : HeapAlloc Kernel->pSemaphores[SemaphoreNumber] == NULL", __FILE__, __LINE__, SemaphoreNumber));
		return cExceptionOccurred;
	}

	/* Initialise la section critique */
	InitializeCriticalSection(&Kernel->pSemaphores[SemaphoreNumber]->csSemaphore);

	/* On créé le sémaphore */
	Kernel->pSemaphores[SemaphoreNumber]->hSemaphore = CreateSemaphore( 
																		NULL,					// default security attributes
																		0,						// initial count
																		CountMaxValue,		// maximum count
																		NULL);				// unnamed semaphore

	/* Initialisation du compteur */
	Kernel->pSemaphores[SemaphoreNumber]->dwCounter		= 0;
	Kernel->pSemaphores[SemaphoreNumber]->dwCounterMax = CountMaxValue;

	/* On test la création du sémaphore */
	if (Kernel->pSemaphores[SemaphoreNumber]->hSemaphore == INVALID_HANDLE_VALUE ||
		 Kernel->pSemaphores[SemaphoreNumber]->hSemaphore == NULL) 
	{
		/* On test si on a lancé trop de sémaphore pour Windows */
		if (ERROR_TOO_MANY_SEMAPHORES == GetLastError())
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit : ERROR_TOO_MANY_SEMAPHORES == GetLastError()", __FILE__, __LINE__, SemaphoreNumber));
			return cNegMaxValue;
		}

		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit : Kernel->pSemaphores[SemaphoreNumber]->hSemaphore == INVALID_HANDLE_VALUE", __FILE__, __LINE__, SemaphoreNumber));
        return cExceptionOccurred;
	}
	
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitSemaphore(%d)] Exit normaly", __FILE__, __LINE__, SemaphoreNumber));

	return cOK;
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
Word far V(
     Word SemaphoreNumber
)
{
	BOOL ret = FALSE;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [V(SemaphoreNumber=%d)] Enter", __FILE__, __LINE__, SemaphoreNumber ));

	/* On vérifie que SemaphoreNumber est compris entre 0 et <NbSemaphores> */
	if (SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [V(SemaphoreNumber=%d)] Exit : SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores", __FILE__, __LINE__, SemaphoreNumber ));
		return cBadSemaphoreNumber;
	}

	/* le semaphore n'existe pas */
	if (Kernel->pSemaphores[SemaphoreNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [V(SemaphoreNumber=%d)] Exit : Kernel->pSemaphores[SemaphoreNumber] == NULL", __FILE__, __LINE__, SemaphoreNumber ));
		return cBadSemaphoreNumber;
	}

	EnterCriticalSection(&Kernel->pSemaphores[SemaphoreNumber]->csSemaphore);

	/* On libère le sémaphore */
	ret = ReleaseSemaphore( 
							Kernel->pSemaphores[SemaphoreNumber]->hSemaphore,	// handle to semaphore
							1,													// increase count by one
							NULL);      // not interested in previous count

	/* On test ReleaseSemaphore */
	if (ret == FALSE && ERROR_TOO_MANY_POSTS == GetLastError())
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [V(SemaphoreNumber=%d, Counter=%d)] Exit : ReleaseSemaphore == ERROR_TOO_MANY_POSTS", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
		LeaveCriticalSection(&Kernel->pSemaphores[SemaphoreNumber]->csSemaphore);
		return cSemOverFlow;
	}

	/* On test ReleaseSemaphore */
	if (ret == FALSE)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [V(SemaphoreNumber=%d, Counter=%d)] Exit : ReleaseSemaphore ret == FALSE | GetlastError=%d", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter, GetLastError()));
		LeaveCriticalSection(&Kernel->pSemaphores[SemaphoreNumber]->csSemaphore);
		return cBadSemaphoreNumber;
	}

	/* On met à jour le compteur */
	Kernel->pSemaphores[SemaphoreNumber]->dwCounter++;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [V(SemaphoreNumber=%d, Counter=%d)] Exit normaly", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));

	LeaveCriticalSection(&Kernel->pSemaphores[SemaphoreNumber]->csSemaphore);

	return cOK; /* returns status = cOK or cSemOverFlow */
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
Word far PMs(
     Word     SemaphoreNumber,
     tTimeOut TimeOut
)
{
	DWORD ret		= 0; 
	Word  wRet		= 0;
	DWORD timeout	= (TimeOut == 0) ? INFINITE : TimeOut;
	Word wCurrTask = CurrentTask();

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d)] Enter", __FILE__, __LINE__, SemaphoreNumber));

	/* On vérifie que SemaphoreNumber est compris entre 0 et <NbSemaphores> */
	if (SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d)] Exit : SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores", __FILE__, __LINE__, SemaphoreNumber));
		return cBadSemaphoreNumber;
	}

	/* le semaphore n'existe pas */
	if (Kernel->pSemaphores[SemaphoreNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d)] Exit : Kernel->pSemaphores[SemaphoreNumber] == NULL", __FILE__, __LINE__, SemaphoreNumber));
		return cBadSemaphoreNumber;
	}	

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d, Counter=%d)] Info", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));

	//if (wCurrTask == -1)
	//{
	//	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d)] Exit : cWaitInBackGroundTask", __FILE__, __LINE__, SemaphoreNumber));
	//	return cWaitInBackGroundTask;
	//}

	if (Kernel->Region.dwInRegion != 0)
	{
		D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d)] Exit : cWaitInRegion", __FILE__, __LINE__, SemaphoreNumber));
		return cWaitInRegion;
	}

	/* Mise à jour de l'état de la tache si la tâche existe */
	if ( ( Kernel->pTaches[wCurrTask] != NULL ) && ( wCurrTask != -1 ) )
	{
		Kernel->pTaches[wCurrTask]->RtcTask.Status = Waiting_P;
	}

	/* On attend le sémaphore */
	ret = WaitForSingleObject( 
								Kernel->pSemaphores[SemaphoreNumber]->hSemaphore,	// handle to semaphore
								timeout												// zero-second time-out interval
								);											

	/* Mise à jour de l'état de la tache si la tâche existe */
	if ( ( Kernel->pTaches[wCurrTask] != NULL ) && ( wCurrTask != -1 ) )
	{
		Kernel->pTaches[wCurrTask]->RtcTask.Status = Ready;
	}

	/* On test l'object que l'on a recu */
	switch (ret) 
	{ 
		/* C'est bien notre sémaphore */
		case WAIT_OBJECT_0:
			
			EnterCriticalSection(&Kernel->pSemaphores[SemaphoreNumber]->csSemaphore);

			/* On met à jour le compteur */
			Kernel->pSemaphores[SemaphoreNumber]->dwCounter--;
			Kernel->pSemaphores[SemaphoreNumber]->LastTaskP = wCurrTask;
			
			D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d, Counter=%d)] Exit normaly", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
			
			LeaveCriticalSection(&Kernel->pSemaphores[SemaphoreNumber]->csSemaphore);

			wRet = cOK;
			break;

		/* Timeout */
		case WAIT_TIMEOUT: 
			D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d, Counter=%d)] Exit normaly (timeout)", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
			wRet = cTimeOut;
			break;

		/* Erreur */
		default:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [P(SemaphoreNumber=%d, Counter=%d)] Exit with error", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
			wRet = cExceptionOccurred;
			break;
	}

	LockInRegion();

	return wRet; /* returns status = cOK or cTimeOut or cExceptionOccurred */
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
Word far P(
     Word     SemaphoreNumber,
     tTimeOut TimeOut
)
{
	return PMs( SemaphoreNumber, TimeOut * RTCTICKTOMS );
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
Word far PWithPrio(
     Word     SemaphoreNumber,
     tTimeOut TimeOut
)
{
#pragma message ("@INFO: [PWithPrio] Pas implemente -> identique a P")
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [PWithPrio] Enter, call P and Exit", __FILE__, __LINE__));
	/* PWithPrio is not implemented */
	return P(SemaphoreNumber, TimeOut); /* returns status = cOK or cTimeOut or cExceptionOccurred */
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
Word far TestP(
     Word SemaphoreNumber
)
{
	DWORD ret			= 0;
	Word	wCurrTask	= CurrentTask();

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TestP(SemaphoreNumber=%d, Counter=%d)] Enter", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));

	/* On vérifie que SemaphoreNumber est compris entre 0 et <NbSemaphores> */
	if (SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [TestP(SemaphoreNumber=%d, Counter=%d)] Exit : SemaphoreNumber < 0 || SemaphoreNumber >= Kernel->ConfigTable->NbSemaphores", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
		return cBadSemaphoreNumber;
	}

	/* On test si le sémaphore est alloué */
	if (Kernel->pSemaphores[SemaphoreNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [TestP(SemaphoreNumber=%d, Counter=%d)] Exit : Kernel->pSemaphores[SemaphoreNumber] == NULL", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
		return cBadSemaphoreNumber;
	}

	/* Mise à jour de l'état de la tache si la tâche existe */
	if (Kernel->pTaches[wCurrTask] != NULL)
	{
		Kernel->pTaches[wCurrTask]->RtcTask.Status = Waiting_TP;
	}

	/* On test sans attendre l'état du sémaphore */
	ret = WaitForSingleObject(Kernel->pSemaphores[SemaphoreNumber]->hSemaphore, 0);

	/* Mise à jour de l'état de la tache si la tâche existe */
	if (Kernel->pTaches[wCurrTask] != NULL)
	{
		Kernel->pTaches[wCurrTask]->RtcTask.Status = Ready;
	}

	/* On test l'object que l'on a recu */
	switch (ret)
	{
	/* Le sémaphore n'est pas disponible */
	case WAIT_ABANDONED:
		D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TestP(SemaphoreNumber=%d, Counter=%d)] Exit normaly (abandoned)", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
		return cSemNonAvailable;

	/* Le sémaphore est disponible */
	case WAIT_OBJECT_0:
		Kernel->pSemaphores[SemaphoreNumber]->dwCounter--;
		D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TestP(SemaphoreNumber=%d, Counter=%d)] Exit normaly", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
		return cOK;

	/* Timeout */
	case WAIT_TIMEOUT: 
		D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TestP(SemaphoreNumber=%d, Counter=%d)] Exit normaly (cSemNonAvailable)", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));
		return cSemNonAvailable;

	/* Erreur */
	default:
		break;
	}

	D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [TestP(SemaphoreNumber=%d, Counter=%d)] Exit : error", __FILE__, __LINE__, SemaphoreNumber, Kernel->pSemaphores[SemaphoreNumber]->dwCounter));

	return cExceptionOccurred; /* returns status = cOK or cSemNonAvailable */
}
      
/* STACK Fonction de destruction de nos données */
/*
void _MailBox_destroy(void * data)
{
}
*/
/* STACK Fonction de debug*/
/*
void _MailBox_debug(void * data)
{
	printf(" %d |", * (int *) data);
}
*/

/* STACK Fonction de comparaison de donnée 
= 0 -> val1 = val2
< 0 -> val1 < val2
> 0 -> val1 > val2
*/
/*
int _MailBox_comp(const void * val1, const void * val2)
{
#pragma warning ( disable : 4311 )
	int a = (int) val1;
	int b = (int) val2;
#pragma warning ( default : 4311 )

	return (int) a - b;
}
*/

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
far InitMailBox(
     Word MailBoxNumber,   /* range 0..NbMailBoxes-1 */
     Word Capacity         /* range 0..32767 */
)
{
	BOOL bRet = FALSE;

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitMailBox(%d)] Enter", __FILE__, __LINE__, MailBoxNumber));

	/* On vérifie que MailBoxNumber est compris entre 0 et <NbMailBoxes> */
	if (MailBoxNumber < 0 || MailBoxNumber >= Kernel->ConfigTable->NbMailBoxes)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitMailBox(%d)] Exit : MailBoxNumber < 0 || MailBoxNumber >= Kernel->ConfigTable->NbMailBoxes", __FILE__, __LINE__, MailBoxNumber));
		return cBadMailBoxNumber;
	}

	/* On vérifie que Capacity est compris entre 0 et 0x7FFF */
	/* NOTE : Capacity n'est pas utilisé */
	if (Capacity < 0 || Capacity > 0x7FFF)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitMailBox(%d)] Exit : Capacity < 0 || Capacity > 0x7FFF", __FILE__, __LINE__, MailBoxNumber));
		return cNegCapacity;
	}

	/* la mailbox a déjà été créé */
	if (Kernel->pMailBoxes[MailBoxNumber] != NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitMailBox(%d)] Exit : La mailbox a déjà été créé", __FILE__, __LINE__, MailBoxNumber));
		return cOK;
	}

	/* on alloue la mémoire pour la mailbox */
	Kernel->pMailBoxes[MailBoxNumber] = (PMAILBOX) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, sizeof(MAILBOX));

	/* On test HeapAlloc */
	if (Kernel->pMailBoxes[MailBoxNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitMailBox(%d)] Exit : HeapAlloc Kernel->pMailBoxes[MailBoxNumber] == NULL", __FILE__, __LINE__, MailBoxNumber));
		return cExceptionOccurred;
	}

	/* On initialise Capacity */
	Kernel->pMailBoxes[MailBoxNumber]->Capacity = Capacity;

	/* On initialise la section critique associée à la pile */
	InitializeCriticalSection(&Kernel->pMailBoxes[MailBoxNumber]->csMailBox);

	/* Creation de l'évènement associé à la MailBox */
	Kernel->pMailBoxes[MailBoxNumber]->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	/* On test le CreateEvent */
	if (Kernel->pMailBoxes[MailBoxNumber]->hEvent == INVALID_HANDLE_VALUE ||
		Kernel->pMailBoxes[MailBoxNumber]->hEvent == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [InitMailBox(%d)] Exit : CreateEvent Kernel->pMailBoxes[MailBoxNumber]->hEvent == INVALID_HANDLE_VALUE | GetLastError=%d", __FILE__, __LINE__, MailBoxNumber, GetLastError()));
		return cTaskNotInitialized;
	}

	/* Initialisation de la pile (DataStructureLib.lib) */
	stack_init(&Kernel->pMailBoxes[MailBoxNumber]->pile, NULL, NULL);
	
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [InitMailBox(%d)] Exit normaly", __FILE__, __LINE__, MailBoxNumber));

	return cOK;
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
Word far Send(
     Word  MailBoxNumber,
     DoubleWord Message
)
{
	BOOL	bRet		= 0;
	DWORD	dwLen		= 0;
	DWORD	dwMsgSize	= 0;
#pragma warning ( disable : 4312 )
	LPVOID	Msg			= (LPVOID) Message;
#pragma warning ( default : 4312 )

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [Send(%d)] Enter", __FILE__, __LINE__, MailBoxNumber));

	/* On vérifie que MailBoxNumber est compris entre 0 et <NbMailBoxes> */
	if (MailBoxNumber < 0 || MailBoxNumber >= Kernel->ConfigTable->NbMailBoxes)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Send(%d)] Exit : MailBoxNumber < 0 || MailBoxNumber >= Kernel->ConfigTable->NbMailBoxes", __FILE__, __LINE__, MailBoxNumber));
		return cBadMailBoxNumber;
	}

	/* On test si la mailbox a été allouée */
	if (Kernel->pMailBoxes[MailBoxNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Send(%d)] Exit : Kernel->pMailBoxes[MailBoxNumber] == NULL", __FILE__, __LINE__, MailBoxNumber));
		return cBadMailBoxNumber;
	}

	EnterCriticalSection(&Kernel->pMailBoxes[MailBoxNumber]->csMailBox);

	/* PUSH dans la pile */
	stack_push(&Kernel->pMailBoxes[MailBoxNumber]->pile, Msg);

	/* Envoi de l'évènement */
	bRet = SetEvent(Kernel->pMailBoxes[MailBoxNumber]->hEvent);

	/* On test l'envoi */
	if (bRet == FALSE)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Send(%d)] Exit : SetEvent bRet == FALSE", __FILE__, __LINE__, MailBoxNumber));
		LeaveCriticalSection(&Kernel->pMailBoxes[MailBoxNumber]->csMailBox);
		return cExceptionOccurred;
	}

	/* Mise à jour du numéro de la dernière tache qui à fait un Send */
	Kernel->pMailBoxes[MailBoxNumber]->LastTaskSend = CurrentTask();

	LeaveCriticalSection(&Kernel->pMailBoxes[MailBoxNumber]->csMailBox);

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [Send(%d)] Exit normaly", __FILE__, __LINE__, MailBoxNumber));

	return cOK; /* returns status = cOK or cFull */
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
Word far SendWithPrio(
     Word  MailBoxNumber,
     DoubleWord Message,
     Word  MessagePriority  /* range 0..65535 */
)
{
#pragma message ("@INFO: [SendWithPrio] Pas implemente -> identique a Send")
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [SendWithPrio] Enter, call Send and Exit)", __FILE__, __LINE__));
	return Send(MailBoxNumber, Message);
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
Word far Receive(
     Word  MailBoxNumber,
     DoubleWord far * pMessage,  /* pointer to the output variable */
     tTimeOut   TimeOut
)
{
	BOOL	bRet			= 0;
	DWORD	dwLen			= 0;
	DWORD	dwMsgSize	= 0;
	DWORD	dwTimeOut   = ( TimeOut == 0 ) ? INFINITE : TimeOut;// * RTCTICKTOMS; /* On est déjà en milliseconde !!! */
	DWORD	dwRet			= 0;
	Word	ret			= 0;
	Word CurTask		= CurrentTask();

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d)] Enter", __FILE__, __LINE__, MailBoxNumber));

	/* Utilisé pour TestReceive */
	dwTimeOut = (dwTimeOut == -2) ? 0 : dwTimeOut;

	/* On vérifie que MailBoxNumber est compris entre 0 et <NbMailBoxes> */
	if (MailBoxNumber < 0 || MailBoxNumber >= Kernel->ConfigTable->NbMailBoxes)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d)] Exit : MailBoxNumber < 0 || MailBoxNumber >= Kernel->ConfigTable->NbMailBoxes", __FILE__, __LINE__, MailBoxNumber));
		return cBadMailBoxNumber;
	}

	/* On test si la mailbox a été allouée */
	if (Kernel->pMailBoxes[MailBoxNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d)] Exit : Kernel->pMailBoxes[MailBoxNumber] == NULL", __FILE__, __LINE__, MailBoxNumber));
		return cBadMailBoxNumber;
	}

	if (CurTask == -1)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d)] Exit : WaitInBackGroundTask", __FILE__, __LINE__, MailBoxNumber));
		return cWaitInBackGroundTask;
	}

	*((void **)pMessage) = NULL;

	/* Mise à jour de l'état de la tâche */
	Kernel->pTaches[CurTask]->RtcTask.Status = Waiting_R;

	/* On attend l'évènement */
	dwRet = WaitForSingleObject( Kernel->pMailBoxes[MailBoxNumber]->hEvent, dwTimeOut );

	/* Mise à jour de l'état de la tâche */
	Kernel->pTaches[CurTask]->RtcTask.Status = Ready;

	/* On test l'object que l'on a recu */
	switch (dwRet)
	{
	/* Pas de message */
	/* NOTE : A TESTER */
	case WAIT_ABANDONED:
		ret = cNoMessPending;
		break;

	/* Timeout */
	case WAIT_TIMEOUT:
		ret = cTimeOut;
		break;

	/* On a recu un message */
	case WAIT_OBJECT_0:
		EnterCriticalSection(&Kernel->pMailBoxes[MailBoxNumber]->csMailBox);
		/* POP dans la pile */
		stack_fifo_pop(&Kernel->pMailBoxes[MailBoxNumber]->pile, (void **) pMessage);
		/* Mise à jour du numéro de la dernière tache qui à fait un Receive */
		Kernel->pMailBoxes[MailBoxNumber]->LastTaskReceive = CurTask;

		/* On reset l'évènement que si la pile est vide */
		if ( stack_size(&Kernel->pMailBoxes[MailBoxNumber]->pile) == 0 )
		{
			BOOL bRes = FALSE;

			bRes = ResetEvent(Kernel->pMailBoxes[MailBoxNumber]->hEvent);

			if (bRes == FALSE)
			{
				D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d)] Exit : ResetEvent == FALSE - %d", __FILE__, __LINE__, MailBoxNumber, GetLastError()));
				ret = cExceptionOccurred;
				break;
			}
		}

		LeaveCriticalSection(&Kernel->pMailBoxes[MailBoxNumber]->csMailBox);
		ret = cOK;
		break;

	/* Erreur */
	case WAIT_FAILED:
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d)] Exit : WAIT_FAILED - %d", __FILE__, __LINE__, MailBoxNumber, GetLastError()));
		ret = cExceptionOccurred;
		break;

	/* Erreur */
	default:
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d)] Exit : Unknown Error - %d", __FILE__, __LINE__, MailBoxNumber, GetLastError()));
		ret = cExceptionOccurred;
		break;
	}

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [Receive(%d,dwRet=%d)] Exit normaly", __FILE__, __LINE__, MailBoxNumber, dwRet));

	LockInRegion();

	return ret; /* returns status = cOK or cTimeOut or cExceptionOccurred */
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
Word far TestReceive(
     Word  MailBoxNumber,
     DoubleWord far * pMessage    /* pointer to the output variable */
)
{
	DWORD	dwRet		= 0;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TestReceive(%d)] Enter (appel Receive)", __FILE__, __LINE__, MailBoxNumber));

	/* NOTE : On ne test pas les paramètres d'entré, Receive le fait déjà */

	/* On fait un Receive non bloquant (-2) */
	dwRet = Receive(MailBoxNumber, pMessage, -2);

	dwRet = ( dwRet == cTimeOut ) ? cNoMessPending : dwRet;

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TestReceive(%d,dwRet=%d)] Exit normaly", __FILE__, __LINE__, MailBoxNumber, dwRet));
	
	return dwRet; /* returns status = cOK or cNoMessPending */
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
far StartDelay(
     Word DelayNumber,				/* range 0..NbDelays-1    */
     Word FirstDelay,				/* range 0..65535         */
     Word Period,						/* 0 = non periodical */
     Word (far * FunctionPtr)()	/* pointer to the function which is to be   */
											/* activated at the expiration of the delay */
)
{
#ifdef USE_MULTIMEDIA_TIMER
#elif defined USE_QUEUE_TIMER
	BOOL success = FALSE;
#endif

	LockInRegion();

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Enter", __FILE__, __LINE__, DelayNumber));

	/* On vérifie que DelayNumber est compris entre 0 et <NbDelays> */
	if (DelayNumber < 0 || DelayNumber >= Kernel->ConfigTable->NbDelays)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : DelayNumber < 0 || DelayNumber >= Kernel->ConfigTable->NbDelays", __FILE__, __LINE__, DelayNumber));
		return cBadDelayNumber;
	}

	/* On vérifie que FunctionPtr n'est pas NULL */
	if ( FunctionPtr == NULL )
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : FunctionPtr == NULL", __FILE__, __LINE__, DelayNumber));
		return cExceptionOccurred;
	}

	/* On vérifie que FirstDelay est supérieur à 0 */
	if ( FirstDelay < 0 )
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : FirstDelay < 0", __FILE__, __LINE__, DelayNumber));
		return cExceptionOccurred;
	}

	if ( FirstDelay > 0xFFFF )
	{
		FirstDelay = 0xFFFF;
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : FirstDelay = 0xFFFF", __FILE__, __LINE__, DelayNumber));
	}

	/* Si FirstDelay = 0, on doit attendre 1 tick */
	FirstDelay = (FirstDelay == 0) ? 1 : FirstDelay;

	/* On test si la période est bien supérieur à zéro */
	if (Period < 0)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : Period < 0", __FILE__, __LINE__, DelayNumber));
		return cExceptionOccurred;
	}

	/* le timer n'existe pas */
	if (Kernel->pDelays[DelayNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Info : Le timer n'existe pas, on le créé", __FILE__, __LINE__, DelayNumber));
		Kernel->pDelays[DelayNumber] = (PDELAY) HeapAlloc(hMemoryHeap, HEAP_ZERO_MEMORY, sizeof(DELAY));

		/* On test HeapAlloc */
		if (Kernel->pDelays[DelayNumber] == NULL)
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : HeapAlloc Kernel->pDelays[DelayNumber] == NULL", __FILE__, __LINE__, DelayNumber));
			return cExceptionOccurred;
		}

		InitializeCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);
	}

	EnterCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);

	Kernel->pDelays[DelayNumber]->FirstDelay	= FirstDelay;
	Kernel->pDelays[DelayNumber]->Period		= Period;
	Kernel->pDelays[DelayNumber]->FunctionPtr = (RtcCallBackFct) FunctionPtr;

#ifdef USE_MULTIMEDIA_TIMER

	/* On initialise la tempo */
	Kernel->pDelays[DelayNumber]->uDelay = FirstDelay * RTCTICKTOMS;

	/* On créé le timer */
	Kernel->pDelays[DelayNumber]->mmTimer = timeBeginPeriod(Kernel->MultimediaTimerResolution); 

	/* On test timeBeginPeriod */
	if (Kernel->pDelays[DelayNumber]->mmTimer != TIMERR_NOERROR)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : Kernel->pDelays[DelayNumber]->mmTimer != TIMERR_NOERROR", __FILE__, __LINE__, DelayNumber));
		LeaveCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);
		return cExceptionOccurred;
	}

#pragma warning ( disable : 4311 ) /* désactive un warning issue du cast 64 bits vers 32 bits : (DWORD) FunctionPtr*/

	/* On initialise la fonction de callback et on lance le timer  */
	if (Period == 0)
	{
		/* ONE SHOT */
		Kernel->pDelays[DelayNumber]->idEvent = timeSetEvent(Kernel->pDelays[DelayNumber]->uDelay, Kernel->MultimediaTimerResolution, (LPTIMECALLBACK) TimerFunction, (DWORD_PTR) Kernel->pDelays[DelayNumber], TIME_ONESHOT | TIME_CALLBACK_FUNCTION); 
	}
	else
	{
		/* PERIODIC */
		Kernel->pDelays[DelayNumber]->idEvent = timeSetEvent(Kernel->pDelays[DelayNumber]->uDelay, Kernel->MultimediaTimerResolution, (LPTIMECALLBACK) TimerFunction, (DWORD_PTR) Kernel->pDelays[DelayNumber], TIME_PERIODIC | TIME_CALLBACK_FUNCTION); 
	}

#pragma warning ( default : 4311 ) /* résactive un warning issue du cast 64 bits vers 32 bits */

	/* On test timeSetEvent */
	if (Kernel->pDelays[DelayNumber]->idEvent == (MMRESULT) 0)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : Kernel->pDelays[DelayNumber]->idEvent == (MMRESULT) 0", __FILE__, __LINE__, DelayNumber));
		/* On arrete le timer */
		timeEndPeriod(Kernel->pDelays[DelayNumber]->uDelay);
		LeaveCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);
		return cExceptionOccurred;
	}

#elif defined USE_QUEUE_TIMER

	/* Création du timer */
	success = CreateTimerQueueTimer( &Kernel->pDelays[DelayNumber]->hDelay,	NULL, TimerProc, (void *) Kernel->pDelays[DelayNumber], FirstDelay * RTCTICKTOMS, Period * RTCTICKTOMS, WT_EXECUTEINTIMERTHREAD );

	/* Test de la création du timer */
	if (success == FALSE)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : CreateTimerQueueTimer success == FALSE", __FILE__, __LINE__, DelayNumber));
		LeaveCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);
		return cExceptionOccurred;
	}
	
	/* On test le Handle du timer */
	if (Kernel->pDelays[DelayNumber]->hDelay == INVALID_HANDLE_VALUE || 
		 Kernel->pDelays[DelayNumber]->hDelay == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit : CreateTimerQueueTimer Kernel->pDelays[DelayNumber]->hDelay == INVALID_HANDLE_VALUE || Kernel->pDelays[DelayNumber].hDelay == NULL", __FILE__, __LINE__, DelayNumber));
		LeaveCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);
		return cExceptionOccurred;
	}

#endif

	Kernel->pDelays[DelayNumber]->Status = Launched;

	LeaveCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StartDelay(DelayNumber=%d)] Exit normaly", __FILE__, __LINE__, DelayNumber));

	return cOK;
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
Word far StopDelay(
     Word  DelayNumber
)
{
#pragma message ("@INFO: [StopDelay] On ne peut pas recuperer le temps restant")

#ifdef USE_MULTIMEDIA_TIMER
	MMRESULT	mmRes	= 0;
#elif defined USE_QUEUE_TIMER
	BOOL		bRet	= FALSE;
#endif

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Enter", __FILE__, __LINE__, DelayNumber));

	/* On vérifie que DelayNumber est compris entre 0 et <NbDelays> */
	if (DelayNumber < 0 || DelayNumber >= Kernel->ConfigTable->NbDelays)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : DelayNumber < 0 || DelayNumber >= Kernel->ConfigTable->NbDelays", __FILE__, __LINE__, DelayNumber));
		LockInRegion();
		return cBadDelayNumber;
	}

	/* On vérifie que le timer a été alloué */
	if (Kernel->pDelays[DelayNumber] == NULL)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : Kernel->pDelays[DelayNumber] == NULL", __FILE__, __LINE__, DelayNumber));
		LockInRegion();
		return cBadDelayNumber;
	}

#ifdef USE_MULTIMEDIA_TIMER

	/* On test l'identifiant de l'évènement du timer */
	if (Kernel->pDelays[DelayNumber]->idEvent == (MMRESULT) 0)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : Kernel->pDelays[DelayNumber]->idEvent == (MMRESULT) 0", __FILE__, __LINE__, DelayNumber));
		LockInRegion();
		return cExceptionOccurred;
	}

	/* NOTE : On en a pas besoin, on peut le supprimer de la structure */
	if (Kernel->pDelays[DelayNumber]->mmTimer != TIMERR_NOERROR)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : Kernel->pDelays[DelayNumber]->mmTimer != TIMERR_NOERROR", __FILE__, __LINE__, DelayNumber));
		LockInRegion();
		return cExceptionOccurred;
	}

	/* On arrête le timer */
	mmRes = timeEndPeriod(Kernel->pDelays[DelayNumber]->uDelay);
	
	if (mmRes != TIMERR_NOERROR)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : timeEndPeriod mmRes != TIMERR_NOERROR", __FILE__, __LINE__, DelayNumber));
		LockInRegion();
		return cExceptionOccurred;
	}

	/* On tue l'évènement associé au timer */
	mmRes = timeKillEvent(Kernel->pDelays[DelayNumber]->idEvent);

	/* On test timeKillEvent */
	if (mmRes != TIMERR_NOERROR)
	{
		D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : timeKillEvent mmRes != TIMERR_NOERROR", __FILE__, __LINE__, DelayNumber));
		LockInRegion();
		return cExceptionOccurred;
	}

#elif defined USE_QUEUE_TIMER
	
	if ( Kernel->pDelays[DelayNumber]->Status == Launched )
	{
		/* On test le Handle du timer */
		if (Kernel->pDelays[DelayNumber]->hDelay == INVALID_HANDLE_VALUE ||
			Kernel->pDelays[DelayNumber]->hDelay == NULL)
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : Kernel->pDelays[DelayNumber]->hDelay == INVALID_HANDLE_VALUE", __FILE__, __LINE__, DelayNumber));
			LockInRegion();
			return cExceptionOccurred;
		}

		EnterCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);

		/* On tue le timer */
		bRet = DeleteTimerQueueTimer(NULL, Kernel->pDelays[DelayNumber]->hDelay, NULL);

		LeaveCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);

		/* On test DeleteTimerQueueTimer */
		if (bRet == FALSE)
		{
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit : DeleteTimerQueueTimer bRet == FALSE | GetLastError=%d", __FILE__, __LINE__, DelayNumber, GetLastError()));
			LockInRegion();
			return cExceptionOccurred;
		}
	}

#endif

	EnterCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);

	Kernel->pDelays[DelayNumber]->Status = Stopped;

	LeaveCriticalSection(&Kernel->pDelays[DelayNumber]->csTimerLock);

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [StopDelay(DelayNumber=%d)] Exit normaly", __FILE__, __LINE__, DelayNumber));

	LockInRegion();

	return 0;	/* returns RestOfDelay; = 0 if delay already stopped or expired */
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
Word ConnectDelay(
     Word  * pDelayNumber,
	  unsigned short DataSegment,
	  void (far * FunctionPtr)()
)
{
#pragma message ("@INFO: [ConnectDelay] Pas implemente")
	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ConnectDelay] Enter", __FILE__, __LINE__));

	D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [ConnectDelay] Exit normaly", __FILE__, __LINE__));

	return cOK;
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
void AllocObj(
					tStatus * 	pStatus,
					Word			Password,
					tObjId *		pObjId,
					Word			TypObj 
					)
{
	*pStatus = cOK;
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
void FreeObj(	tStatus  *	pStatus,
					Word			Password,
					tObjId		ObjId,
					Word			TypObj
				)
{
	*pStatus = cOK;
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
void ChgeCntObj(	tStatus   *	pStatus,
						Word			Password,
						tObjId	 *	pObjId,
						Word			TypObj
					)
{
	*pStatus = cOK;
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
void ChObjPassword(	tStatus	 *	pStatus,
							Word			OldPassword,
							Word			NewPassword,
							tObjId		ObjId,
							Word			TypObj
						)
{
	*pStatus = cOK;
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
void SetObjName(	tStatus	 *	pStatus,
						tObjName  *	pObjName,
						tObjId		ObjId,
						Word			TypObj
					)
{
	*pStatus = cOK;
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
void GetObjName(
					tStatus   * pStatus,
					tObjName  *	pObjName,
					tObjId		ObjId,
					Word			TypObj
					)
{
	*pStatus = cOK;
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
void GetObjId(
					tStatus  *	pStatus,
					tObjName  *	pObjName,
					tObjId	 *	pObjId,
					Word			TypObj
				)
{
	*pStatus = cOK;
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
void CheckObjId(	tStatus	 *	pStatus,
						tObjId		ObjId,
						Word			TypObj
					)
{
	*pStatus = cOK;
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
void InitSegPool(	tStatus		*	pStatus,
						DoubleWord		MemSize,
						Address			MemAddress,
						Int				SegPool)
{
	*pStatus = cOK;
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
void CheckPool(	tStatus  *	pStatus,
						Int				SegPool
					)
{
	*pStatus = cOK;
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
void SegAllocate(	tStatus 			*	pStatus,
						tObjId			*	pSegId,
						Address 			*	pSegAddr,
						DoubleWord			SegSize,
						Int					SegPool
					)
{
	*pStatus = cOK;
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
void SegReserve(	tStatus		*	pStatus,
						tObjId		*	pSegId,
						Address			SegAddr,
						DoubleWord		SegSize,
						int				SegPool
					)
{
	*pStatus = cOK;
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
void SegFree(	tStatus	 *	pStatus,
					tObjId		SegId
				)
{
	*pStatus = cOK;
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
void DDrvInstall(	tStatus	 *	pStatus,
						DoubleWord	CallAid
					)
{
	*pStatus = cOK;
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
void DDrvResources(	tStatus		 *	pStatus,
							DoubleWord	 *	pDataSize,
							tObjNumTable	pObjNumTable,
							void 			*	pDDConfigTable,
							Int				MaxD,
							DoubleWord		CallAid
						)
{
	*pStatus = cOK;
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
void DDrvInit(	tStatus	 *	pStatus,
					tObjId	 *	pDDrvId,
					Address		DataAddr,
					tObjName  *	pDDrvName,
					DoubleWord	CallAid
				)
{
	*pStatus = cOK;
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
void DDrvGetInfo(	tStatus		*	pStatus,
						tDDrvInfo	*	pDDrvInfo,
						tObjId		*	pDDrvId,
						tObjName		*	pDDrvName
					)
{
	*pStatus = cOK;
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
void DDrvControl(	tStatus	 *	pStatus,
						void		 *	ControlPB,
						tObjId		DDrvId
						)
{
	*pStatus = cOK;
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
void UnitCreate(	tStatus		 *	pStatus,
						tObjId		 *	pUnitId,
						tUnitDescr	 *	pUnitDescr,
						tObjName	    *	pUnitName
					)
{
	*pStatus = cOK;
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
void UnitDelete(	tStatus	 *	pStatus,
						tObjId		UnitId
					)
{
	*pStatus = cOK;
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
void UnitGetInfo(	tStatus		 *	pStatus,
						Byte			 *	pUnitState,
						tUnitDescr	 *	pUnitDescr,
						tObjId		 *	pUnitId,
						tObjName		 *	pUnitName
						)
{
	*pStatus = cOK;
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
void PhyUClose(	tStatus	 *	pStatus,
						Address		pQIOPB,
						tObjId		UnitId
					)
{
	*pStatus = cOK;
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
void UnitOpen(	tStatus		*	pStatus,
					tObjId		*	pUnitId,
					tObjName		*	pUnitName
				)
{
	*pStatus = cOK;
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
void QIO(	tStatus	 *	pStatus,
				Address		pQIOPB,
				tObjId		UnitId
			)
{
	*pStatus = cOK;
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
void QIOW(	tStatus  *	pStatus,
				Address		pQIOPB,
				tObjId		UnitId
			)
{
	*pStatus = cOK;
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
void QIOCancel(tStatus	 *	pStatus,
					tObjId		UnitId
					)
{
	*pStatus = cOK;
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
void IOSInstall(tStatus * pStatus)
{
	*pStatus = cOK;
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
void IOSResources(	tStatus 				*	pStatus,
							DoubleWord			*	pDataSize,
							tObjNumTable 		*	pObjNumTable,
							tIOSConfigTable	*	pIOSConfigTable
						)
{
	*pStatus = cOK;
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
void InitIOS(	tStatus	 *	pStatus,
					Address		DataAd
				)
{
	*pStatus = cOK;
}