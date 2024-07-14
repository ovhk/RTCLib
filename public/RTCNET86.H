/****************************************************************************/
/*                                                                          */
/*  RealTime Craft TCP/IP:                                                  */
/*       Constants, Types and API                                           */
/*                                                                          */
/****************************************************************************/
/*
    Copyright (C) 1997 By
    Tecsi
    Tour Aurore, 18 place des Reflets
    92080 Paris la Defense cedex 05
    FRANCE
*/


/****************************************************************************/
/* Internet general definitions                                             */
/****************************************************************************/

#define Iid_SZ 4

struct  Iid {unsigned char c[Iid_SZ];}; /* internal address format          */


/****************************************************************************/
/*  TCP/IP stack initialization                                             */
/****************************************************************************/

/* Run time relocation and linking                                          */
/****************************************************************************/

void    tcp86_netinstall ( void );


/* Resources declaration                                                    */
/****************************************************************************/

#ifndef _OBJNUMTABLE_
#define _OBJNUMTABLE_

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
        cNumObjType
} tObjectType;
 
typedef struct {
        Word            TotalNumber;
        Word            PreAllocated;
} tObjNumElement;

typedef tObjNumElement far tObjNumTable [cNumObjType];

#endif /* _OBJNUMTABLE_ */


/* TCP/IP dimension parameters                                              */
/****************************************************************************/

struct  tTCPConfigTable {
        Word            MaxConnection;
        Word            NetworkInterfaceNum;
        Word            MaxRemoteIPNode;
};


/* resource declaration                                                     */
/****************************************************************************/

long    tcp86_netresource ( struct tTCPConfigTable far * pTCPConfigTable,
                      tObjNumTable far * pObjNumTable );


/* TCP/IP stack initialization                                              */
/****************************************************************************/

#define CfCBSize            48
#define CnCBSize           120
#define IfCBSize           210
#define RTCNetBuffSize    1540

struct  tRTCNetControlBlock {
        Word            TaskNumber;     /* Task Number                      */
        Word            Priority;       /* Taks Priority                    */
        Word far *      StackPtr;       /* Task StackPointer                */
        DoubleWord      TCBAddr;        /* TCB address                      */
        Word            FirstSemaphoreNumber; /* First semaphore number     */
        Word            XecClockHz;     /* Xec Tick Frequency               */
        Word            DelayNumber;    /* Delay number                     */
        Word far *      ControlBlock;   /* RTCNET Control Block             */
        Word            NbRTCNetBuffer; /* Number of buffer in ControlBlock */
};

int     tcp86_netinit ( struct tRTCNetControlBlock far * pRTCNetControlBlock,
                  Word far * pBufPool,
                  int far * errno );


/****************************************************************************/
/*  Network interface initialisation and configuration                      */
/****************************************************************************/

/* Network interface initialization                                         */
/****************************************************************************/

#ifdef __MSDOS__

	#define Ethernet             7          /* link level protocol is Ethernet  */
	#define SLIP                 8          /* link level protocol is SLIP      */
	#define PPP                  10         /* link level protocol is PPP       */

#endif __MSDOS__

struct  IFDESC {                        /* network interface description    */
        int             lprotoc;        /* link level protocol              */
        long            softw_int;      /* for the access to the driver     */
};

int     tcp86_netstart ( char far * ifname, struct IFDESC ifdesc );

/* Network interface configuration                                          */
/****************************************************************************/

#define IFF_UP            0x01          /* interface is up                  */
#define IFF_POINTTOPOINT  0x10          /* interface is point-to-point link */
#define IFF_RUNNING       0x40          /* resources allocated              */
#define IFF_NOARP         0x80          /* no address resolution protocol   */

#define HOSTNAME_SIZ        32          /* hostname max. size               */

struct  IFCUST {
        struct Iid      Iaddr;		/* internal (Internet) address          */
        char            hostname[HOSTNAME_SIZ];  /* local hostname          */
        struct Iid      dst_Iaddr; 	/* destination address (for SLIP)       */
        char            dst_hostname[HOSTNAME_SIZ];  /* destination hostname (for SLIP) */
        int             flag;           /* up/down, broadcast               */
        struct Iid      Imask;          /* address mask, 0 = host part      */
};

int     tcp86_ifconfig ( char far * ifname, struct IFCUST far * ifcust );


/* Logical hostname declaration                                             */
/****************************************************************************/

int     tcp86_sethostbyname ( char far * hostname, struct Iid Iaddr );

/* Return codes                                                             */
/****************************************************************************/

#define cNMGTok              0
#define cNMGTerr            -1          /* configuration error              */


/****************************************************************************/
/*  PING application interface definition                                   */
/****************************************************************************/

int     tcp86_ping ( char far * hostname, int len );

/* Error codes                                                              */
/****************************************************************************/

#define cPINGok              0
#define cPINGerr           -10          /* incorrect parameter              */
#define cPINGreply         -11          /* no reply received                */
#define cPINGconfailed     -12          /* connection failed                */
#define cPINGfunnyresp     -13          /* funny response                   */



/****************************************************************************/
/*  File Transfer application interface definition (FTP and TFTP)           */
/****************************************************************************/

#define ASCII                0          /* file mode: text                  */
#define IMAGE                1          /*            binary                */

/* FTP functions prototypes                                                 */
/****************************************************************************/

int     tcp86_FTPgetput( char far * hostname, char far * userid, char far * passwd, 
                   char far * buffer, int bufsize, char far * file,
                   int far * filesize, int mode);

#define tcp86_FTPget(hostname, userid, passwd, \
               buffer, bufsize, file, filesize, mode) \
        tcp86_FTPgetput(hostname, userid, passwd, \
               buffer, bufsize, file, filesize, mode)

#define tcp86_FTPput(hostname, userid, passwd, buffer, bufsize, file, mode) \
        tcp86_FTPgetput(hostname, userid, passwd, buffer, bufsize, file, 0, mode+2)


/* Return codes                                                             */
/****************************************************************************/

#define cFTPok               0
#define cFTPfailed         -10
#define cFTPBadConnection	-11
#define cFTPBadUser			-12
#define cFTPBadPassword		-13
#define cFTPBadFileType		-14
#define cFTPBadPort			-15
#define cFTPBadOp			-16

/* TFTP functions prototypes                                                */
/****************************************************************************/

int     tcp86_TFTPget(char far * host, char far * buffer, int bufsize, 
                char far * file, int far * filesize, int mode);

int     tcp86_TFTPput(char far * host, char far * buffer, int bufsize,
                char far * file, int mode);

/* Return codes                                                             */
/****************************************************************************/

#define cTFTPok              0
#define cTFTPfailed        -10


/****************************************************************************/
/*  BSD socket interface definition                                         */
/****************************************************************************/

#ifdef __MSDOS__

/* protocol family                                                          */
/****************************************************************************/

#define	PF_INET	             2          /* TCP/IP and related               */


/* socket types                                                             */
/****************************************************************************/

#define	SOCK_STREAM          1          /* stream socket                    */
#define	SOCK_DGRAM           2          /* datagram socket                  */
#define	SOCK_RAW             3          /* raw-protocol interface           */


/* Protocols                                                                */
/****************************************************************************/

#define	IPPROTO_TCP          6          /* tcp                              */
#define	IPPROTO_UDP         17          /* user datagram protocol           */


/* Definition of specific IP addresses                                      */
/****************************************************************************/

#define	INADDR_ANY          (unsigned long)0x00000000


/* options for getsockopt() and setsockopt()                                */
/****************************************************************************/

#define	SOL_SOCKET      0xffff          /* options for socket level         */

#define	SO_DEBUG        0x0001          /* turn on debugging info recording */
#define	SO_REUSEADDR    0x0004          /* allow local address reuse        */
#define	SO_KEEPALIVE    0x0008          /* keep connections alive           */
#define	SO_DONTROUTE    0x0010          /* just use interface addresses     */
#define	SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define	SO_LINGER       0x0080          /* linger on close if data present  */
#define	SO_OOBINLINE    0x0100          /* leave received OOB data in line  */
#define	SO_SNDBUF       0x1001          /* send buffer size                 */
#define	SO_RCVBUF       0x1002          /* receive buffer size              */
#define	SO_ERROR        0x1007          /* get error status and clear       */
#define	SO_TYPE         0x1008          /* get socket type                  */

/* errno values                                                             */
/****************************************************************************/

#define	EINTR                4          /* Interrupted call                 */
#define	EBADF                9          /* Bad socket descriptor            */
#define	EFAULT              14          /* Error occured                    */
#define	EINVAL              22          /* Invalid argument                 */
#define	EDESTADDRREQ        39          /* Destination address required     */
#define	EMSGSIZE            40          /* Message too long                 */
#define	EPROTOTYPE          41          /* Protocol wrong type for socket   */
#define	ENOPROTOOPT         42          /* Protocol not available           */
#define	EPROTONOSUPPORT     43          /* Protocol not supported           */
#define	EOPNOTSUPP          45          /* Operation not supported on socket*/
#define	EPFNOSUPPORT        46          /* Protocol family not supported    */
#define	EADDRINUSE          48          /* Address already in use           */
#define	EADDRNOTAVAIL       49          /* Can't assign requested address   */
#define	ENETUNREACH         51          /* Network is unreachable           */
#define	ENOBUFS             55          /* No buffer space available        */
#define	EISCONN             56          /* Socket is already connected      */
#define	ENOTCONN            57          /* Socket is not connected          */
#define	ETIMEDOUT           60          /* Connection timed out             */
#define	ECONNREFUSED        61          /* Connection refused               */
#define	EHOSTUNREACH        65          /* No route for host                */


/* structures                                                               */
/****************************************************************************/

struct  sockaddr {                      /* generic socket address           */
        unsigned short  sa_family;      /* address family                   */
        char            sa_data[14];    /* up to 14 bytes of direct address */
};

struct  in_addr {                       /* Internet address                 */
        unsigned long   s_addr;
};

struct  sockaddr_in {                   /* Internet socket address          */
        short           sin_family;
        unsigned short  sin_port;
        struct in_addr  sin_addr;
        char            sin_zero[8];
};

#endif __MSDOS__

struct  iovec {                         /* address and length               */
        char far *      iov_base;       /* base                             */
        int             iov_len;        /* size                             */
};

struct  msghdr {                        /* Message header for recvmsg and sendmsg. */
        char far *      msg_name;       /* optional address                 */
        int             msg_namelen;    /* size of address                  */
        struct iovec far * msg_iov;     /* scatter/gather array             */
        int             msg_iovlen;     /* # elements in msg_iov            */
        char far *      msg_accrights;  /* access rights sent/received      */
        int             msg_accrightslen;
};

#ifdef __MSDOS__

#define	MSG_OOB            0x1          /* process out-of-band data         */
#define	MSG_PEEK           0x2          /* peek at incoming message         */
#define	MSG_DONTROUTE      0x4          /* send without using routing tables*/
#define SBYTE                8
#define	FD_SETSIZE         256

typedef struct fd_set {                 /* Bit mask for select()            */
        Byte            fds_bits[FD_SETSIZE/SBYTE];
} fd_set;

/* Pour Optimisation de Taille et de vitesse 
#define SSBY            SBYTE-1
#define	FD_SET(n, p)	((p)->fds_bits[(n)/SBYTE] |= (1<<(SSBY-((n)%SBYTE))))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/SBYTE] &=~(1<<(SSBY-((n)%SBYTE))))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/SBYTE] &  (1<<(SSBY-((n)%SBYTE))))
*/
#define	FD_SET(n, p)	((p)->fds_bits[(n)>>3] |= (1<<(0x07^((n)&0x07))))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)>>3] &=~(1<<(0x07^((n)&0x07))))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)>>3] &  (1<<(0x07^((n)&0x07))))
#define	FD_ZERO(p)      memset((void *)(p), 0, sizeof (fd_set))

struct  timeval {                       /* Timeout format for select()      */
        long            tv_sec;         /* seconds                          */
        long            tv_usec;        /* milliseconds                     */
};

#endif __MSDOS__

/* Socket Interface functions prototypes                                    */
/****************************************************************************/

int     tcp86_accept(int s, struct sockaddr far * name, int far * namelen);
int     tcp86_bind(int s, struct sockaddr far * name, int namelen);
int     tcp86_connect(int s, struct sockaddr far * name, int namelen);
int     tcp86_close(int s);
int     tcp86_getsockname(int s, struct sockaddr far * name, int far * namelen);
int     tcp86_getsockopt(int s, int level, int optname, char far * optval,
                   int far * optlen);
int     tcp86_listen(int s, int backlog);
int     tcp86_select(int nfds, fd_set far * readfds, fd_set far * writefds,
               fd_set far * exceptfds, struct timeval far * timeout);
int     tcp86_setsockopt(int s, int level, int optname, char far * optval,
                   int optlen);
int     tcp86_read(int s, char far * buff, int len);
int     tcp86_recv(int s, char far * buf, int len, int flags);
int     tcp86_recvfrom(int s, char far * buf, int len, int flags,
                 struct sockaddr far * from, int far * fromlen);
int     tcp86_recvmsg(int s, struct msghdr far * msg, int flags);
int     tcp86_send(int s, char far * buf, int len, int flags);
int     tcp86_sendto(int s, char far * buf, int len, int flags,
               struct sockaddr far * to, int tolen);
int     tcp86_sendmsg(int s, struct msghdr far * msg, int flags);
int     tcp86_shutdown(int s, int how);
int     tcp86_socket(int domain, int type, int protocol);
int     tcp86_write(int s, char far * buff, int len);

#define FREEBUFVAR      12
#define CmdGet   		1
int 	tcp86_spy(int var, int cmd, int len, char far * buff);

#ifdef __MSDOS__

	#if !defined(ntohl)
	#define	ntohl(x)   ( ((long)x<<24) | (((long)x&0xff00)<<8) | \
								(x>>8)&0xff00 | ((unsigned long)x>>24) )
	#define	htonl(x)   ( ((long)x<<24) | (((long)x&0xff00)<<8) | \
								(x>>8)&0xff00 | ((unsigned long)x>>24) )
	#endif

	#if !defined(ntohs)
	#define ntohs(x) (((x&0xff) << 8) | ((unsigned short)x >> 8))
	#define htons(x) (((x&0xff) << 8) | ((unsigned short)x >> 8))
	#endif

#endif __MSDOS__

/****************************************************************************/
/*  Trap Numbers                                                            */
/****************************************************************************/

#define cRtcnetBufferOvf      0x8500
#define cRtcnetTooManyConn    0x8501

#define cRtcnetFatalError001  0x8580
#define cRtcnetFatalError002  0x8581
#define cRtcnetFatalError003  0x8582
#define cRtcnetFatalError004  0x8583
#define cRtcnetFatalError005  0x8584
#define cRtcnetFatalError006  0x8585
#define cRtcnetFatalError007  0x8586
#define cRtcnetFatalError008  0x8587
