/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc_debugger.c
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_debugger.c,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.2   Apr 05 2005 13:27:54   VANHOUCKE
 update
 
    Rev 1.1   Feb 16 2005 16:13:18   VANHOUCKE
 Update
 
    Rev 1.0   Feb 16 2005 16:09:32   VANHOUCKE
 Initial revision.

* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES : ---------------*/

#include "../include/rtc_config.h"							/* Configuration de la compilation */
#include "../include/rtc_debug.h"							/* Outils de debug */
#include "../include/rtc_kernel.h"							/* Format des données du kernel RTC */
#include "../include/rtc_shared.h"							/* Format des données partagées */
#include "../include/rtc_debugger.h"						/* Format des données du debugger RTC */

/*--------------- VARIABLES : ---------------*/

// <summary>Pointeur vers le debugger</summary>
DEBUGGER	RtcDebugger; 

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

DWORD ThreadRtcDebugger( LPVOID Param )
{
	DWORD				dwWaitRes	= 0;
	unsigned int	i				= 0;

	PSHAREDDATA_TASK			pTmpTaches		= NULL;
	PSHAREDDATA_SEMAPHORE	pTmpSemaphores	= NULL;
	PSHAREDDATA_MAILBOX		pTmpMailBoxes	= NULL;
	PSHAREDDATA_DELAY			pTmpDelays		= NULL;

	OutputDebugString( "ThreadRtcDebugger started...\n" );

	/* Creation de l'évènement */
	RtcDebugger.hEventGetData = CreateEvent( NULL, FALSE, FALSE, RTC_EVENT_GET_DATA_NAME );

	/* Test du CreateEvent */
	if (	( RtcDebugger.hEventGetData == INVALID_HANDLE_VALUE ) 
		|| ( RtcDebugger.hEventGetData == NULL ) 
		)
	{ 
		OutputDebugString( "Erreur CreateEvent hEventGetKernel\n" );
		return -1;
	}

	/* Creation de l'évènement */
	RtcDebugger.hEventDataReady = CreateEvent( NULL, FALSE, FALSE, RTC_EVENT_DATA_READY_NAME );

	/* Test du CreateEvent */
	if (	( RtcDebugger.hEventDataReady == INVALID_HANDLE_VALUE ) 
		|| ( RtcDebugger.hEventDataReady == NULL ) 
		)
	{ 
		OutputDebugString( "Erreur CreateEvent hEventGetKernel\n" );
		return -1;
	}

	/* Création du partage inter-process pour des données du kernel */
	RtcDebugger.hMapFile = CreateFileMapping(
											INVALID_HANDLE_VALUE,				// use paging file
											NULL,										// default security 
											PAGE_READWRITE,						// read/write access
											0,											// max. object size 
											RtcDebugger.dwSharedDataSize,		// buffer size  
											RTC_SHARED_OBJECT_NAME				// name of mapping object
										);                
 
	/* Test du CreateFileMapping */
	if (	( RtcDebugger.hMapFile == INVALID_HANDLE_VALUE )
		|| ( RtcDebugger.hMapFile == NULL ) 
		)
	{ 
		OutputDebugString( "Erreur CreateFileMapping\n" );
		return -1;
	}

	/* Allocation et partage des données du kernel */
	RtcDebugger.lpSharedData = (PDEBUGGER_SHARED_DATA) MapViewOfFile(	RtcDebugger.hMapFile,	// handle to mapping object
																							FILE_MAP_ALL_ACCESS,		// read/write permission
																							0, 0, 0						// map entire file
																							);           
 
	/* Test du MapViewOfFile */
	if ( RtcDebugger.lpSharedData == NULL )
	{ 
		CloseHandle( RtcDebugger.hMapFile );
		OutputDebugString( "Erreur MapViewOfFile\n" );
		return -1;
	}

	/* Mise à jour des variables statiques de la structure */
	RtcDebugger.lpSharedData->MaxPriority			= Kernel->ConfigTable->MaxPriority;
	RtcDebugger.lpSharedData->NbDelays				= Kernel->ConfigTable->NbDelays;
	RtcDebugger.lpSharedData->NbMailBoxes			= Kernel->ConfigTable->NbMailBoxes;
	RtcDebugger.lpSharedData->NbSemaphores			= Kernel->ConfigTable->NbSemaphores;
	RtcDebugger.lpSharedData->NbTasks				= Kernel->ConfigTable->NbTasks;

	/* Calculs des offset */
	RtcDebugger.lpSharedData->IndexTasks			= (DWORD) sizeof(DEBUGGER_SHARED_DATA);
	RtcDebugger.lpSharedData->IndexSemaphores		= (DWORD) RtcDebugger.lpSharedData->IndexTasks			+ (RtcDebugger.lpSharedData->NbTasks		* sizeof(SHAREDDATA_TASK));
	RtcDebugger.lpSharedData->IndexMailboxes		= (DWORD) RtcDebugger.lpSharedData->IndexSemaphores	+ (RtcDebugger.lpSharedData->NbSemaphores * sizeof(SHAREDDATA_SEMAPHORE));
	RtcDebugger.lpSharedData->IndexDelays			= (DWORD) RtcDebugger.lpSharedData->IndexMailboxes		+ (RtcDebugger.lpSharedData->NbMailBoxes	* sizeof(SHAREDDATA_MAILBOX));

#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4312 )

	/* Calculs des pointeurs */
	pTmpTaches												= (PSHAREDDATA_TASK)			( (DWORD) RtcDebugger.lpSharedData + RtcDebugger.lpSharedData->IndexTasks );
	pTmpSemaphores											= (PSHAREDDATA_SEMAPHORE)	( (DWORD) RtcDebugger.lpSharedData + RtcDebugger.lpSharedData->IndexSemaphores );
	pTmpMailBoxes											= (PSHAREDDATA_MAILBOX)		( (DWORD) RtcDebugger.lpSharedData + RtcDebugger.lpSharedData->IndexMailboxes );
	pTmpDelays												= (PSHAREDDATA_DELAY)		( (DWORD) RtcDebugger.lpSharedData + RtcDebugger.lpSharedData->IndexDelays );

#pragma warning ( default : 4312 )
#pragma warning ( default : 4311 )

	do
	{
		dwWaitRes = WaitForSingleObject( RtcDebugger.hEventGetData, 1000 );

		switch ( dwWaitRes )
		{
		case WAIT_ABANDONED:
			OutputDebugString( "WAIT_ABANDONED\n" );
			break;

		case WAIT_TIMEOUT:
			OutputDebugString( "WAIT_TIMEOUT\n" );
			break;

		case WAIT_OBJECT_0:
			OutputDebugString( "WAIT_OBJECT_0\n" );
			OutputDebugString( "Updating Shared Data Structure..." );

			RtcDebugger.lpSharedData->bSafeRegionActive	= Kernel->Region.bSafeRegionActive;
			RtcDebugger.lpSharedData->dwInRegion			= Kernel->Region.dwInRegion;
			RtcDebugger.lpSharedData->dwTaskNoInRegion	= Kernel->Region.dwTaskNoInRegion;

			/* On mets à jour la structure partagée */
			for ( i = 0; i < RtcDebugger.lpSharedData->NbTasks; i++ )
			{
				if ( Kernel->pTaches[i] != NULL )
				{
					pTmpTaches[i].ThreadId					= Kernel->pTaches[i]->WindowsThread.ThreadId;
					pTmpTaches[i].EventData					= Kernel->pTaches[i]->WindowsThread.EventData;
					pTmpTaches[i].Priority					= Kernel->pTaches[i]->RtcTask.Priority;
					pTmpTaches[i].Status						= Kernel->pTaches[i]->RtcTask.Status;
				}
			}

			for ( i = 0; i < RtcDebugger.lpSharedData->NbSemaphores; i++ )
			{
				if ( Kernel->pSemaphores[i] != NULL )
				{
					pTmpSemaphores[i].dwCounter			= Kernel->pSemaphores[i]->dwCounter;
					pTmpSemaphores[i].dwCounterMax		= Kernel->pSemaphores[i]->dwCounterMax;
					pTmpSemaphores[i].LastTaskP			= Kernel->pSemaphores[i]->LastTaskP;
				}
			}

			for ( i = 0; i < RtcDebugger.lpSharedData->NbMailBoxes; i++ )
			{
				if ( Kernel->pMailBoxes[i] != NULL )
				{
					pTmpMailBoxes[i].dwNbMessages			= Kernel->pMailBoxes[i]->pile.size;
					pTmpMailBoxes[i].LastTaskReceive		= Kernel->pMailBoxes[i]->LastTaskReceive;
					pTmpMailBoxes[i].LastTaskSend			= Kernel->pMailBoxes[i]->LastTaskSend;
				}
			}

			for ( i = 0; i < RtcDebugger.lpSharedData->NbDelays; i++ )
			{
				if ( Kernel->pDelays[i] != NULL )
				{
					pTmpDelays[i].FirstDelay				= Kernel->pDelays[i]->FirstDelay;
					pTmpDelays[i].Period						= Kernel->pDelays[i]->Period;
					pTmpDelays[i].Status						= Kernel->pDelays[i]->Status;
				}
			}

			FlushViewOfFile( RtcDebugger.lpSharedData, RtcDebugger.dwSharedDataSize );

			OutputDebugString( "   done\nSetEvent Data Ready\n" );
			SetEvent( RtcDebugger.hEventDataReady );		// On envoie l'acknowledge
			break;

		case WAIT_FAILED:
			OutputDebugString( "WAIT_FAILED\n" );
			break;

		default:
			OutputDebugString( "default\n" );
			break;
		}

	} while ( !RtcDebugger.bDebuggerExit );

	UnmapViewOfFile( RtcDebugger.hMapFile );
	CloseHandle( RtcDebugger.hMapFile );
	CloseHandle( RtcDebugger.hEventDataReady );
	CloseHandle( RtcDebugger.hEventGetData );

	OutputDebugString( "ThreadRtcDebugger stop !\n" );

	return 0;
}