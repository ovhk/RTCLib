/****************************************/
/* XEC constants and types              */
/****************************************/
 
#define  cTimeOut               1  /* DontTrap, XECProduct,   1 */
#define  cExceptionOccurred     2  /* DontTrap, XECProduct,   2 */
#define  cSemOverFlow           3  /* DontTrap, XECProduct,   3 */
#define  cSemNonAvailable       4  /* DontTrap, XECProduct,   4 */
#define  cFull                  5  /* DontTrap, XECProduct,   5 */
#define  cNoMessPending         6  /* DontTrap, XECProduct,   6 */
 
#define  cBadTaskNumber         -32767  /* MustTrap, XECProduct,   1 */
#define  cBadSemaphoreNumber    -32766  /* MustTrap, XECProduct,   2 */
#define  cBadMailBoxNumber      -32765  /* MustTrap, XECProduct,   3 */
#define  cBadDelayNumber        -32764  /* MustTrap, XECProduct,   4 */
#define  cBadPriority           -32763  /* MustTrap, XECProduct,   5 */
#define  cBadClockTkPriority    -32762  /* MustTrap, XECProduct,   6 */
#define  cDataSegmentOverFlow   -32761  /* MustTrap, XECProduct,   7 */
#define  c8087NotAvailable      -32760  /* MustTrap, XECProduct,   8 */
#define  cNegMaxValue           -32759  /* MustTrap, XECProduct,   9 */
#define  cNegCapacity           -32758  /* MustTrap, XECProduct,  10 */
#define  cCurrTask              -32757  /* MustTrap, XECProduct,  11 */
#define  cDelaySlotAlreadyUsed  -32756  /* MustTrap, XECProduct,  12 */
#define  cTaskNotInitialized    -32755  /* MustTrap, XECProduct,  13 */
#define  cTaskAlreadyStarted    -32754  /* MustTrap, XECProduct,  14 */
#define  cTaskNotOperational    -32753  /* MustTrap, XECProduct,  15 */
#define  cNestingOverFlow       -32752  /* MustTrap, XECProduct,  16 */
#define  cNestingUnderFlow      -32751  /* MustTrap, XECProduct,  17 */
#define  cWaitInRegion          -32750  /* MustTrap, XECProduct,  18 */
#define  cWaitInBackGroundTask  -32749  /* MustTrap, XECProduct,  19 */
#define  cMessOverFlow          -32748  /* MustTrap, XECProduct,  20 */
#define  cTaskAbnormalEnd       -32747  /* MustTrap, XECProduct,  21 */
 
#define  cCurrent        0
#define  cReady          1
#define  cWaiting        2
#define  cNonOperational 3
 
#define  cInfiniteWaiting 0l
 
typedef DoubleWord tTimeOut;
      /* zero : infinite waiting
         other: max waiting time in clock base units (finite waiting) */
 
typedef Word   tTaskState;
      /* 0 = cCurrent, 1 = cReady, 2 = cWaiting, 3 = cNonOperational */
 
typedef Word tEvent;  /* range 0..15 */
 
typedef Word tEventList;
      /* 1 event per bit, event 0 corresponding to the LSB */
 
struct tConfigTable {
   Word MaxPriority,         /* range  0..65535 */
        NbTasks,             /* range  0..65535 */
        NbSemaphores,        /* range  0..65535 */
        NbMailBoxes,         /* range  0..65535 */
        NbMessages,          /* range  0..65535 */
        NbDelays;            /* range  0..65535 */
   Word DataSegmentSize;
   Word far *ClockTaskStackPtr; /* pointer (on 32 bits !) to the end */
                                /* of the stack of the clock task    */
   Word ClockTaskPriority;      /* range 0..MaxPriority */
   Boolean BGTaskUsesNDP;       /* Numeric Data Processor: 8087 or 80287 */
   Boolean		Dummy;
   Word			TCBExtSize;
   void (far *   CLKHook) ();
   DoubleWord	CLKHookParam;
   void (far * BKGHook) ();
   DoubleWord	BKGHookParam;
};
 
/*********************************************************************/
/* XEC interface exported routines                                   */
/*********************************************************************/
 
/****************************************/
/*   Kernel initialization              */
/****************************************/
 
extern far InitKernel(
     struct tConfigTable far * /* pConfigTable */,
	 DoubleWord far *          /* pBKG_TCBAddr */,
	 DoubleWord far *          /* pCLK_TCBAddr */
);
 
/****************************************/
/*   Task management                    */
/****************************************/
 
extern far InitTask(
     Word /* TaskNumber */,             /* range 0..NbTasks-1 */
     Word /* Priority */,               /* range 0..MaxPriority */
     Word (far * /* StartAddress */)(), /* pointer to the fonction which
                                        /* contains the code of the task */
     Word far * /* StackPtr */,         /* pointer to the end of the stack */
     Word /* DataSegment */,     /* initial DS register value for the task */
     Boolean /* TaskUsesNDP */,  /* Numeric Data Processor: 8087 or 80287 */
                                 /* = 1 if the task uses NDP, = 0 otherwise */
	 void (far *)       /* Hook */ (),
	 DoubleWord         /* HookParam */,
	 DoubleWord far *   /* pTCBAddr */
);
 
extern far StartTask(
     Word /* TaskNumber */
);
 
extern far StopTask(
     Word /* TaskNumber */
);
 
extern far Terminate( void );

extern Word far CurrentTask( void );
			       /* returns TaskNumber for an user defined task */
			       /*         -1     for the BackGround Task      */
			       /*         -2     for the Clock Task           */

extern tTaskState far TaskState(
     Word /* TaskNumber */
);
       /* returns 0 = cCurrent      */
       /*         1 = cReady        */
       /*         2 = cWaiting      */
       /*         3 cNonOperational */

extern Word far TaskPriority(
     Word /* TaskNumber */   /* -1 may be used to refer to current task */
);
/* returns Priority */

extern far ChangePriority(
     Word /* TaskNumber */,   /* -1 may be used to refer to current task */
     Word /* NewPriority */
);

extern far ForceExceptionHandler(
     Word /* TaskNumber */,
     Word (far * /* ExceptionHandlerPtr */)(),
     Word /* Parameter */
);

/****************************************/
/*   Critical region management         */
/****************************************/

extern far EnterRegion( void );

extern far LeaveRegion( void );

/****************************************/
/*  Event management                    */
/****************************************/

extern far SignalEvent(
     Word /* TaskNumber */,
     tEvent /* EventNumber */   /* range 0..15 */
);

extern Word far WaitEvents(
     tEventList /* EventsAwaited */,
     tTimeOut /* TimeOut */,
     tEventList far * /* pEventsOccurred */
);
       /* returns status = cOK or cTimeOut or cExceptionOccurred */

extern Boolean far EventsOccurred(
     tEventList /* EventList */
);

extern far ClearEvents(
     tEventList /* EventList */
);

/****************************************/
/*   Semaphore management               */
/****************************************/

extern far InitSemaphore(
     Word /* SemaphoreNumber */,  /* range 0..NbSemaphores-1 */
     Word /* CountMaxValue */     /* range 0..32767 */
);

extern Word far V(
     Word /* SemaphoreNumber */
);
      /* returns status = cOK or cSemOverFlow */

extern Word far P(
     Word     /* SemaphoreNumber */,
     tTimeOut /* TimeOut */
);
      /* returns status = cOK or cTimeOut or cExceptionOccurred */

extern Word far PWithPrio(
     Word     /* SemaphoreNumber */,
     tTimeOut /* TimeOut */
);
      /* returns status = cOK or cTimeOut or cExceptionOccurred */

extern Word far TestP(
     Word /* SemaphoreNumber */
);
      /* returns status = cOK or cSemNonAvailable */

/**********************************************/
/*   Mailbox management                       */
/**********************************************/

extern far InitMailBox(
     Word /* MailBoxNumber */,   /* range 0..NbMailBoxes-1 */
     Word /* Capacity */         /* range 0..32767 */
);

extern Word far Send(
     Word  /* MailBoxNumber */,
     DoubleWord /* Message */
);
      /* returns status = cOK or cFull */

extern Word far SendWithPrio(
     Word  /* MailBoxNumber */,
     DoubleWord /* Message */,
     Word  /* MessagePriority */  /* range 0..65535 */
);
      /* returns status = cOK or cFull */

extern Word far Receive(
     Word  /* MailBoxNumber */,
     DoubleWord far * /* pMessage */,  /* pointer to the output variable */
     tTimeOut   /* TimeOut */
);
      /* returns status = cOK or cTimeOut or cExceptionOccurred */

extern Word far TestReceive(
     Word  /* MailBoxNumber */,
     DoubleWord far * /* pMessage */    /* pointer to the output variable */
);
      /* returns status = cOK or cNoMessPending */

/****************************************/
/*   Delay management                   */
/****************************************/

extern far StartDelay(
     Word /* DelayNumber */,       /* range 0..NbDelays-1    */
     Word /* FirstDelay */,        /* range 0..65535         */
     Word /* Period */,            /* 0 = non periodical */
     Word (far * /* FunctionPtr */)() /* pointer to the function which is to be   */
				   /* activated at the expiration of the delay */
);

extern Word far StopDelay(
     Word  /* DelayNumber */
);
      /* returns RestOfDelay; = 0 if delay already stopped or expired */

/****************************************/
/*   Return from exception              */
/****************************************/

extern far ExitExceptionHandler( void );

/****************************************/
/*   get data segment selector value    */
/****************************************/

extern Word far DataSegment( void );
