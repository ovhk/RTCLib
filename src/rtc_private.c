/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc_private.c
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_private.c,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.4   Apr 05 2005 13:27:54   VANHOUCKE
 update
 
    Rev 1.2   Feb 21 2005 17:06:18   VANHOUCKE
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
#include "../include/rtc_kernel.h"							/* Format des données propre à RTC */

/*--------------- FUNCTIONS : ---------------*/

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
void LockInRegion( void )
{
	if (	( Kernel->Region.bSafeRegionActive == TRUE ) 
		&& ( Kernel->Region.dwInRegion != 0 )
		&&	( Kernel->Region.dwTaskNoInRegion != CurrentTask() ) 
		)
	{
		DWORD dwRes = 0;

		dwRes = WaitForSingleObject( Kernel->Region.hRegionEvent, INFINITE );

		switch ( dwRes )
		{
		case WAIT_FAILED:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [LockInRegion(%d)] Exit : WAIT_FAILED", __FILE__, __LINE__, Kernel->Region.dwInRegion));
			break;

		case WAIT_ABANDONED:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [LockInRegion(%d)] Exit : WAIT_ABANDONED", __FILE__, __LINE__, Kernel->Region.dwInRegion));
			break;
		
		case WAIT_OBJECT_0:
			break;

		default:
			D(DL_MIN, RTC_DEBUG_PRINTF("%s - %d - [LockInRegion(%d)] Exit : Unknown Error", __FILE__, __LINE__, Kernel->Region.dwInRegion));
		}
	}
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
void UnSafeRegion(void)
{
#pragma message ("???? : Y a t'il autre chose à faire ????")
	Kernel->Region.bSafeRegionActive = FALSE;
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
int RtcToWindowsPriority( int priority )
{
	#if ( RTC_PRIORITY_CLASS == REALTIME_PRIORITY_CLASS )
		/*
			16 REALTIME_PRIORITY_CLASS THREAD_PRIORITY_IDLE 
			17 REALTIME_PRIORITY_CLASS -7 
			18 REALTIME_PRIORITY_CLASS -6 
			19 REALTIME_PRIORITY_CLASS -5 
			20 REALTIME_PRIORITY_CLASS -4 
			21 REALTIME_PRIORITY_CLASS -3 
			22 REALTIME_PRIORITY_CLASS THREAD_PRIORITY_LOWEST 
			23 REALTIME_PRIORITY_CLASS THREAD_PRIORITY_BELOW_NORMAL 
			24 REALTIME_PRIORITY_CLASS THREAD_PRIORITY_NORMAL 
			25 REALTIME_PRIORITY_CLASS THREAD_PRIORITY_ABOVE_NORMAL 
			26 REALTIME_PRIORITY_CLASS THREAD_PRIORITY_HIGHEST 
			27 REALTIME_PRIORITY_CLASS 3 
			28 REALTIME_PRIORITY_CLASS 4 
			29 REALTIME_PRIORITY_CLASS 5 
			30 REALTIME_PRIORITY_CLASS 6 
			31 REALTIME_PRIORITY_CLASS THREAD_PRIORITY_TIME_CRITICAL 
		*/
		switch ( priority )
		{
		case 2:
			return THREAD_PRIORITY_LOWEST + RTC_PRIORITY_OFFSET;
		case 3:
			return THREAD_PRIORITY_BELOW_NORMAL + RTC_PRIORITY_OFFSET;
		case 4:
			return THREAD_PRIORITY_NORMAL + RTC_PRIORITY_OFFSET;
		case 5:
			return THREAD_PRIORITY_ABOVE_NORMAL + RTC_PRIORITY_OFFSET;
		case 6:
			return THREAD_PRIORITY_HIGHEST + RTC_PRIORITY_OFFSET;
		case 7:
			return 3 + RTC_PRIORITY_OFFSET;
		case 8:
			return 4 + RTC_PRIORITY_OFFSET;
		case 9:
			return 5 + RTC_PRIORITY_OFFSET;
		case 10:
			return 6 + RTC_PRIORITY_OFFSET;
		default: 
			OutputDebugString("-------Unknown Priority");
			return THREAD_PRIORITY_IDLE;
		}
	#elif ( RTC_PRIORITY_CLASS == HIGH_PRIORITY_CLASS )
		/*
			1 HIGH_PRIORITY_CLASS THREAD_PRIORITY_IDLE 
			11 HIGH_PRIORITY_CLASS THREAD_PRIORITY_LOWEST 
			12 HIGH_PRIORITY_CLASS THREAD_PRIORITY_BELOW_NORMAL 
			13 HIGH_PRIORITY_CLASS THREAD_PRIORITY_NORMAL 
			14 HIGH_PRIORITY_CLASS THREAD_PRIORITY_ABOVE_NORMAL 
			15 HIGH_PRIORITY_CLASS THREAD_PRIORITY_HIGHEST 
			15 HIGH_PRIORITY_CLASS THREAD_PRIORITY_TIME_CRITICAL 
		*/
		switch ( priority )
		{
		case 2:
			return THREAD_PRIORITY_LOWEST + RTC_PRIORITY_OFFSET;
		case 3:
			return THREAD_PRIORITY_BELOW_NORMAL + RTC_PRIORITY_OFFSET;
		case 4:
			return THREAD_PRIORITY_NORMAL + RTC_PRIORITY_OFFSET;
		case 5:
			return THREAD_PRIORITY_NORMAL + RTC_PRIORITY_OFFSET;
		case 6:
			return THREAD_PRIORITY_ABOVE_NORMAL + RTC_PRIORITY_OFFSET;
		case 7:
			return THREAD_PRIORITY_ABOVE_NORMAL + RTC_PRIORITY_OFFSET;
		case 10:
			return THREAD_PRIORITY_HIGHEST + RTC_PRIORITY_OFFSET;
		default: 
			OutputDebugString("-------Unknown Priority");
			return THREAD_PRIORITY_IDLE;
		}
	#endif
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
#ifdef USE_MULTIMEDIA_TIMER

	#pragma warning ( disable : 4312 ) /* désactive un warning issue du cast 32 bits vers 64 bits : (RtcCallBackFct) dwUser */
	void CALLBACK TimerFunction(UINT wTimerID, UINT msg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
	{
		PDELAY tmp = (PDELAY) dwUser;

		tmp->FunctionPtr();				/* Appel de la fonction de callback utilisateur */
		tmp->Status = Expired;			/* Mise à jour du status */
	} 
	#pragma warning ( default : 4312 ) /* résactive un warning issue du cast 32 bits vers 64 bits */

#elif defined USE_QUEUE_TIMER

	#pragma warning ( disable : 4312 ) /* désactive un warning issue du cast 32 bits vers 64 bits : (RtcCallBackFct) dwUser */
	void CALLBACK TimerProc(void * lpParameter, BOOLEAN TimerOrWaitFired)
	{
		PDELAY tmp = (PDELAY) lpParameter;

		__try
		{
			__try
			{
				EnterCriticalSection( &tmp->csTimerLock );

				D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TimerProc(FirstDelay=%d)] call", __FILE__, __LINE__, tmp->FirstDelay));

				tmp->FunctionPtr();				/* Appel de la fonction de callback utilisateur */
				tmp->Status = Expired;			/* Mise à jour du status */
			}
			__finally
			{
				LeaveCriticalSection( &tmp->csTimerLock );
			}
		}
		__except( ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
		{
			D(DL_MAX, RTC_DEBUG_PRINTF("%s - %d - [TimerProc()] Error exception", __FILE__, __LINE__));	
			printf( "[TimerProc] callback function exception\n" );
			
			LeaveCriticalSection( &tmp->csTimerLock );
		}
	}
	#pragma warning ( default : 4312 ) /* résactive un warning issue du cast 32 bits vers 64 bits */

#endif