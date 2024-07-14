/**************************************/
/* Object Manager constants and types */
/**************************************/

 
typedef enum {
       cInvalidObject,
/* XEC objects types codes */
       cTaskObject = 0,
       cSemObject,
       cMBxObject,
       cDelayObject,
/* IOS objects types codes */
       cSegObject,
       cDDrvObject,
       cUnitObject,
/* FMS objects types codes */
       cFMSFileObject,
	   cFMSUnitObject,
	   cDiskFileObject,
	   cDiskUnitObject,
	   cTermUnitObject,
/* number of object types */
       cNumObjType }
tObjectType;
 
/********************************/
/* Object manager Status values */  
/********************************/
#define cNmTbFull          (tStatus)258 /* DontTrap, IOSProduct,   2 */
#define cObjIdError        (tStatus)259 /* DontTrap, IOSProduct,   3 */
#define cObjIdFree         (tStatus)260 /* DontTrap, IOSProduct,   4 */
#define cObjIdPassword  (tStatus)-32507 /* MustTrap, IOSProduct,   5 */
#define cTypObjError    (tStatus)-32506 /* MustTrap, IOSProduct,   6 */
#define cAmbiguousName     (tStatus)263 /* DontTrap, IOSProduct,   7 */
#define cStringLonger      (tStatus)264 /* DontTrap, IOSProduct,   8 */
#define cAnonymousObjId    (tStatus)265 /* DontTrap, IOSProduct,   9 */
 
#define cMinInexObject     (tStatus)384 /* DontTrap, IOSProduct, 128 */
#define cMaxInexObject     (tStatus)447 /* DontTrap, IOSProduct, 191 */
#define cMinNoMoreObject   (tStatus)448 /* DontTrap, IOSProduct, 192 */
#define cMaxNoMoreObject   (tStatus)511 /* DontTrap, IOSProduct, 255 */
 
typedef struct {
   Int Index;
   Int Count;
}
   tObjId;
 
#define cObjNameSize 8 /* suggested value */
   /* max number of characters of an object name */
 
typedef struct {
   Int  ObjNameSize;
   char ObjName[cObjNameSize];
}
   tObjName;
 
typedef struct {
       Word       TotalNumber;
       Word       PreAllocated;
}
   tObjNumElement;

typedef tObjNumElement tObjNumTable [cNumObjType];

/***************************************/
/* Object Manager interface procedures */
/***************************************/
 
extern void  cdecl AllocObj
(
	tStatus  * 	pStatus,
    Word			Password,
	tObjId  *	pObjId,
	Word			TypObj );
 
extern void  cdecl FreeObj
(
	tStatus  *	pStatus,
	Word			Password,
	tObjId			ObjId,
	Word			TypObj);
 
extern void  cdecl ChgeCntObj
(
	tStatus  *	pStatus,
	Word			Password,
	tObjId	 *	pObjId,
	Word			TypObj);
 
extern void  cdecl ChObjPassword
(
	tStatus	 *	pStatus,
	Word			OldPassword,
	Word			NewPassword,
	tObjId			ObjId,
	Word			TypObj);

extern void  cdecl SetObjName
(
	tStatus	 *	pStatus,
	tObjName  *	pObjName,
	tObjId			ObjId,
	Word			TypObj);
 
extern void  cdecl GetObjName
(
	tStatus  *	pStatus,
	tObjName  *	pObjName,
	tObjId			ObjId,
	Word			TypObj);

extern void  cdecl GetObjId
(
	tStatus  *	pStatus,
	tObjName  *	pObjName,
	tObjId	 *	pObjId,
	Word			TypObj);
 
extern void  cdecl CheckObjId
(
	tStatus	 *	pStatus,
	tObjId			ObjId,
	Word			TypObj);
 
/***************************************/
/* Segment Manager constants and types */
/***************************************/
 
/* Status values */
#define cSegTooLarge         (tStatus)289 /* DontTrap, IOSProduct,  33 */
#define cSegSzInvalid        (tStatus)290 /* DontTrap, IOSProduct,  34 */
#define cMemFragmentation    (tStatus)291 /* DontTrap, IOSProduct,  35 */
#define cSegOutsideMem       (tStatus)292 /* DontTrap, IOSProduct,  36 */
#define cMemAllocated        (tStatus)293 /* DontTrap, IOSProduct,  37 */
#define cPoolInvalid      (tStatus)-32474 /* MustTrap, IOSProduct,  38 */
#define cPoolAlreadyInit  (tStatus)-32473 /* MustTrap, IOSProduct,  39 */
#define cPoolNotInit      (tStatus)-32472 /* MustTrap, IOSProduct,  40 */
 
/****************************************/
/* Segment Manager interface procedures */
/****************************************/
 
extern void  cdecl InitSegPool
(
	tStatus	 *	pStatus,
	DoubleWord		MemSize,
	Address			MemAddress,
	Int				SegPool);
 
extern void  cdecl CheckPool
(
	tStatus  *	pStatus,
	Int				SegPool);
 
extern void  cdecl SegAllocate
(
	tStatus 	 *	pStatus,
	tObjId		 *	pSegId,
	Address 		*	pSegAddr,
	DoubleWord			SegSize,
	Int					SegPool);
 
extern void  cdecl SegReserve
(
	tStatus  *	pStatus,
	tObjId	 *	pSegId,
	Address			SegAddr,
	DoubleWord		SegSize,
	int				SegPool);
 
extern void  cdecl SegFree
(
	tStatus	 *	pStatus,
	tObjId			SegId);
 
/******************/
/* I/O Supervisor */
/******************/
 
/*****************************/
/* Device drivers management */
/*****************************/
 
typedef struct {
   DoubleWord DDCallAid;
   Int        DDUnitsCreated;
   Word       Reserved1;
}
   tDDrvInfo;
 
/* Device drivers management primitives */

extern void  cdecl DDrvInstall
(
	tStatus	 *	pStatus,
	DoubleWord		CallAid);
 
extern void  cdecl DDrvResources
(
	tStatus		 *	pStatus,
	DoubleWord	 *	pDataSize,
	tObjNumTable		pObjNumTable,
	void 		 *	pDDConfigTable,
	Int					MaxD,
	DoubleWord			CallAid);
 
extern void  cdecl DDrvInit
(
	tStatus	 *	pStatus,
	tObjId	 *	pDDrvId,
	Address			DataAddr,
	tObjName  *	pDDrvName,
	DoubleWord		CallAid);
 
extern void  cdecl DDrvGetInfo
(
	tStatus		 *	pStatus,
	tDDrvInfo	 *	pDDrvInfo,
	tObjId		 *	pDDrvId,
	tObjName	 *	pDDrvName);
 
extern void  cdecl DDrvControl
(
	tStatus	 *	pStatus,
	void	 *	ControlPB,
	tObjId			DDrvId);


/***************************/
/* Unit objects management */
/***************************/
/* Status values */
#define cQIORefused      322 /* DontTrap, IOSProduct,  66 */
#define cUNotOpen        323 /* DontTrap, IOSProduct,  67 */
#define cUNotClosed      324 /* DontTrap, IOSProduct,  68 */
 
/* units states values */
#define cUInexistent  0
#define cUOpen        1
#define cUClosed      2
#define cUBeingClosed 3
 
typedef struct {
   tObjId DDrvId;
   Word   PhyUnit;
   Word   SysIOFun;
   Byte   DevClass;
   Byte   Reserved1;
   Word   Reserved2;
}
   tUnitDescr;
 
typedef struct {
   tObjId TaskId;
   Word   PhyUnit;
   Byte   Event;
   Byte   IOFunction;
   Word   IOStatus;
   Byte   CurrUState;
   Byte   Reserved;
}
   tRootQIOPB;
 
/* Unit management primitives */
extern void  cdecl UnitCreate
(
	tStatus		 *	pStatus,
	tObjId		 *	pUnitId,
	tUnitDescr	 *	pUnitDescr,
	tObjName	 * 	pUnitName);
 
extern void  cdecl UnitDelete
(
	tStatus	 *	pStatus,
	tObjId			UnitId);
 
extern void  cdecl UnitGetInfo
(
	tStatus		 *	pStatus,
	Byte		 *	pUnitState,
	tUnitDescr	 *	pUnitDescr,
	tObjId			*	pUnitId,
	tObjName		*	pUnitName);
 
extern void  cdecl PhyUClose
(
	tStatus	 *	pStatus,
	Address			pQIOPB,
	tObjId			UnitId);

extern void  cdecl UnitOpen
(
	tStatus		 *	pStatus,
	tObjId		 *	pUnitId,
	tObjName	 *	pUnitName);

extern void  cdecl QIO
(
	tStatus	 *	pStatus,
	Address			pQIOPB,
	tObjId			UnitId);
 
extern void  cdecl QIOW
(
	tStatus  *	pStatus,
	Address			pQIOPB,
	tObjId			UnitId);
 
extern void  cdecl QIOCancel
(
	tStatus	 *	pStatus,
	tObjId			UnitId);
 
/********************************************/
/* constants, types and interface functions */
/* used for IOS initialization              */
/********************************************/
/* status values */
#define cDataSegTooShort -24830 /* MustTrap, AllProduct,   2 */
#define cIOSBadConfig    -32447 /* MustTrap, IOSProduct,  65 */
 
typedef struct {
   Int StringRegionSize;
   Int NumObjType;
   Int NumPool;
   Int NumSegment;
}
   tIOSConfigTable;
 
extern void  cdecl IOSInstall
(
	tStatus	 *	pStatus);
 
extern void  cdecl IOSResources
(
	tStatus 		 *	pStatus,
	DoubleWord		 *	pDataSize,
	tObjNumTable 	 *	pObjNumTable,
	tIOSConfigTable	 *	pIOSConfigTable);
 
extern void  cdecl InitIOS
(
	tStatus	 *	pStatus,
	Address			DataAd);
