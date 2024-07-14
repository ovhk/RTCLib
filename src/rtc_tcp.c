/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC TCP/IP
* FICHIER :	rtc_tcp.c
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
$Log: rtc_tcp.c,v $
Revision 1.2  2005/07/08 21:00:16  olivier
Correction

Revision 1.1  2005/07/01 21:06:08  olivier
Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.

 
    Rev 1.1   Feb 16 2005 16:13:18   VANHOUCKE
 Update
 
    Rev 1.0   Feb 16 2005 16:09:20   VANHOUCKE
 Initial revision.
 
    Rev 1.5   Feb 04 2005 09:52:26   VANHOUCKE
 update
 
    Rev 1.4   Jan 27 2005 18:01:46   VANHOUCKE
 update
 
    Rev 1.3   Jan 17 2005 10:56:52   VANHOUCKE
 Update
 
    Rev 1.2   Jan 06 2005 12:16:48   VANHOUCKE
 Update
 
    Rev 1.1   Jan 06 2005 10:45:12   VANHOUCKE
 Update
 
    Rev 1.0   Jan 05 2005 10:31:32   VANHOUCKE
 Initial revision.
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES : ---------------*/

#include <win32.h>			/* Windows Default Includes */

#include <base86c.h>			/* RTC */
#include <RtcNet86.h>		/* RTC NET | ucv/dev/inc/RtcNet86.h */

#include "../include/rtc_config.h"

// struct IFDESC est défini dans le RtcNet86.h original
//struct IFDESC				/* network interface description    */
//{                        
//	int	lprotoc;       /* link level protocol              */
//	long	softw_int;     /* for the access to the driver     */
//};

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
int tcp86_sethostbyname ( char * hostname, struct Iid Iaddr )
{
	/* Attribut un nom à une adresse IP */
	return 0;
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
int tcp86_init( struct init_par * params )
{
	int		iRes = 0;
	WSADATA	wsaData;

	iRes = WSAStartup( WINSOCK_VERSION, &wsaData );

	if ( iRes != 0 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return 1;
	}
 
	/* Confirm that the WinSock DLL supports 2.2.			*/
	/* Note that if the DLL supports versions greater		*/
	/* than 2.2 in addition to 2.2, it will still return	*/
	/* 2.2 in wVersion since that is the version we			*/
	/* requested.														*/
 
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup();
		return 2; 
	}

	return 0;
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
int tcp86_socket( int domain, int type, int protocol )
{
#ifdef RTC_TCP_RECHERCHE_PROTOCOLE
	LPWSAPROTOCOL_INFO	wpiBuf = NULL;
	WSAPROTOCOL_INFO		wpiProtocolInfo;
	DWORD		dwSizeProto		= 0;
	int		iFoundProto		= -1;
	DWORD		i					= 0;
	int		iRet				= 0;
#endif RTC_TCP_RECHERCHE_PROTOCOLE

	SOCKET	sSocket			= INVALID_SOCKET;

	if ( (domain <= 0) || (type <= 0) || (protocol < 0) )
	{
		return -1;
	}

#ifdef RTC_TCP_RECHERCHE_PROTOCOLE
	/* Recherche du Protocole */
   
	/* On récupère la taille prise par tous les protocoles */
	iRet = WSAEnumProtocols( NULL, NULL, &dwSizeProto );

	/* Test de WSAEnumProtocols */
	if ( iRet != SOCKET_ERROR ) 
	{
		int iErr = WSAGetLastError();

		OutputDebugString("WSAEnumProtocols == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED: OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress.\n"); break; 
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : Indicates that one of the specified parameters was invalid.\n"); break;
			case WSAENOBUFS:			OutputDebugString("WSAENOBUFS : The buffer length was too small to receive all the relevant WSAPROTOCOL_INFO structures and associated information. Pass in a buffer at least as large as the value returned in lpdwBufferLength.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : One or more of the lpiProtocols, lpProtocolBuffer, or lpdwBufferLength parameters are not a valid part of the user address space.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}
		 
		return -1;  
	}

	/* Allocation du buffer qui va recevoir tous les protocoles (si on ne l'a pas déjà alloué) */
	if ( wpiBuf == NULL ) 
	{
		/* Allocation */
		wpiBuf = (WSAPROTOCOL_INFO *) malloc( (size_t) dwSizeProto );

		/* Test du malloc */
		if ( wpiBuf == NULL ) 
		{
			return -1;
		}
	}

   /* On récupère la liste des protocoles */
	iRet = WSAEnumProtocols( NULL, wpiBuf, &dwSizeProto );

	/* Test de WSAEnumProtocols */
	if ( iRet == SOCKET_ERROR ) 
	{
		int iErr = WSAGetLastError();

		OutputDebugString("WSAEnumProtocols == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED: OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress.\n"); break; 
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : Indicates that one of the specified parameters was invalid.\n"); break;
			case WSAENOBUFS:			OutputDebugString("WSAENOBUFS : The buffer length was too small to receive all the relevant WSAPROTOCOL_INFO structures and associated information. Pass in a buffer at least as large as the value returned in lpdwBufferLength.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : One or more of the lpiProtocols, lpProtocolBuffer, or lpdwBufferLength parameters are not a valid part of the user address space.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}
        
		/* On ne libère pas pour des raisons d'optimisation */
		//free( wpiBuf );
       
		return -1;
    }

	/* Recherche du protocole */
	for ( i = 0; i < (dwSizeProto / sizeof(WSAPROTOCOL_INFO)) ; i++ )
	{
		if ( protocol == 0 )
		{
			if (	(wpiBuf[i].iAddressFamily	== domain)	&&
					(wpiBuf[i].iSocketType		== type)	)
			{
				if ( (wpiBuf[i].dwServiceFlags1 & XP1_QOS_SUPPORTED) == XP1_QOS_SUPPORTED )
				{
					/* Trouvé ! */
					iFoundProto = i;
					break;
				}
			}
		}
		else
		{
			if (	(wpiBuf[i].iAddressFamily	== domain)	&&
					(wpiBuf[i].iSocketType		== type)		&&
					(wpiBuf[i].iProtocol			== protocol) )
			{
				if ( (wpiBuf[i].dwServiceFlags1 & XP1_QOS_SUPPORTED) == XP1_QOS_SUPPORTED )
				{
					/* Trouvé ! */
					iFoundProto = i;
					break;
				}
			}
		}
	}

	if ( iFoundProto == -1 )
	{
		OutputDebugString("On a pas trouve le protocole, on tente d'ouvrir une socket directement...");
		
#endif RTC_TCP_RECHERCHE_PROTOCOLE
		/* Creation de la socket */
		sSocket = WSASocket( domain, type, protocol, NULL, 0, 0 );

#ifdef RTC_TCP_RECHERCHE_PROTOCOLE
	}
	else
	{
		memcpy( &wpiProtocolInfo, &wpiBuf[iFoundProto], sizeof( WSAPROTOCOL_INFO ) );
		
		/* Creation de la socket */
		sSocket = WSASocket( FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, &wpiProtocolInfo, 0, 0 );
	}

	/* Libération de la liste des protocoles */
	/* On ne libère pas pour des raisons d'optimisation */
	free( wpiBuf );

#endif RTC_TCP_RECHERCHE_PROTOCOLE

	/* Test de WSASocket */
	if ( sSocket == INVALID_SOCKET )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("WSASocket == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED:		OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:				OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEAFNOSUPPORT:		OutputDebugString("WSAEAFNOSUPPORT : The specified address family is not supported.\n"); break;
			case WSAEINPROGRESS:			OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAEMFILE:				OutputDebugString("WSAEMFILE : No more socket descriptors are available.\n"); break;
			case WSAENOBUFS:				OutputDebugString("WSAENOBUFS : No buffer space is available. The socket cannot be created.\n"); break;
			case WSAEPROTONOSUPPORT:	OutputDebugString("WSAEPROTONOSUPPORT : The specified protocol is not supported.\n"); break;
			case WSAEPROTOTYPE:			OutputDebugString("WSAEPROTOTYPE : The specified protocol is the wrong type for this socket.\n"); break;
			case WSAESOCKTNOSUPPORT:	OutputDebugString("WSAESOCKTNOSUPPORT : The specified socket type is not supported in this address family.\n"); break;
			case WSAEINVAL:				OutputDebugString("WSAEINVAL : This value is true for any of the following conditions.\n");
													OutputDebugString("The parameter g specified is not valid\n");
													OutputDebugString("The WSAPROTOCOL_INFO structure that lpProtocolInfo points to is incomplete, the contents are invalid or the WSAPROTOCOL_INFO structure has already been used in an earlier duplicate socket operation.\n");
													OutputDebugString("The values specified for members of the socket triple <af, type, and protocol> are individually supported, but the given combination is not.\n"); break;
			case WSAEFAULT:				OutputDebugString("WSAEFAULT : The lpProtocolInfo parameter is not in a valid part of the process address space.\n"); break;
			default:							OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	{
		BOOL bBuf = FALSE;
		int ret = setsockopt( sSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &bBuf, sizeof( BOOL ) );

		if ( ret == -1 ) 
		{
			OutputDebugString("setsockopt() SO_REUSEADDR failed:");
			return -1;
		}
	}

	return (int) sSocket;
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
int tcp86_accept( int s, struct sockaddr * name, int * namelen )
{
	SOCKET sAccept = INVALID_SOCKET;

	if (	(s < 0)				
		||	(name == NULL)		
		||	(namelen == NULL) 
		)
	{
		return -1;
	}

	sAccept = WSAAccept( (SOCKET) s, name, namelen, NULL, 0 );

	/* Test de WSAAccept */
	if ( sAccept == INVALID_SOCKET )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("WSAAccept == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED: OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAECONNREFUSED:	OutputDebugString("WSAECONNREFUSED : The connection request was forcefully rejected as indicated in the return value of the condition function (CF_REJECT).\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : The addrlen parameter is too small or the addr or lpfnCondition are not part of the user address space.\n"); break;
			case WSAEINTR:				OutputDebugString("WSAEINTR : A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress.\n"); break;
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : listen was not invoked prior to WSAAccept, the return value of the condition function is not a valid one, or any case where the specified socket is in an invalid state.\n"); break;
			case WSAEMFILE:			OutputDebugString("WSAEMFILE : The queue is nonempty upon entry to WSAAccept and there are no socket descriptors available.\n"); break;
			case WSAENOBUFS:			OutputDebugString("WSAENOBUFS : No buffer space is available.\n"); break;
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEOPNOTSUPP:		OutputDebugString("WSAEOPNOTSUPP : The referenced socket is not a type that supports connection-oriented service.\n"); break;
			case WSATRY_AGAIN:		OutputDebugString("WSATRY_AGAIN : The acceptance of the connection request was deferred as indicated in the return value of the condition function (CF_DEFER).\n"); break;
			case WSAEWOULDBLOCK:		OutputDebugString("WSAEWOULDBLOCK : The socket is marked as nonblocking and no connections are present to be accepted.\n"); break;
			case WSAEACCES:			OutputDebugString("WSAEACCES : The connection request that was offered has timed out or been withdrawn.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	return (int) sAccept;
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
int tcp86_bind( int s, struct sockaddr * name, int namelen )
{
	int iRes = 0;

	if (	(s < 0)
		||	(name == NULL)
		||	(namelen == 0) 
		)
	{
		return -1;
	}

	iRes = bind( (SOCKET) s,  name, namelen);

	/* Test de bind */
	if ( iRes == SOCKET_ERROR )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("bind == SOCKET_ERROR\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED:	OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEACCES:			OutputDebugString("WSAEACCES : Attempt to connect datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled.\n"); break;
			case WSAEADDRINUSE:		OutputDebugString("WSAEADDRINUSE : A process on the computer is already bound to the same fully-qualified address and the socket has not been marked to allow address reuse with SO_REUSEADDR. For example, the IP address and port are bound in the af_inet case). (See the SO_REUSEADDR socket option under setsockopt.)\n"); break;
			case WSAEADDRNOTAVAIL:	OutputDebugString("WSAEADDRNOTAVAIL : The specified address is not a valid address for this computer.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, or the first two bytes of the memory block specified by name does not match the address family associated with the socket descriptor s.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : The socket is already bound to an address.\n"); break;
			case WSAENOBUFS:			OutputDebugString("WSAENOBUFS : Not enough buffers available, too many connections.\n"); break; 
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	return iRes;
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
int tcp86_connect( int s, struct sockaddr * name, int namelen )
{
	int iRes = 0;

	if (	(s < 0)			||
			(name == NULL)	||
			(namelen == 0) )
	{
		return -1;
	}

	iRes = WSAConnect( (SOCKET) s, name, namelen, NULL, NULL, NULL, NULL );

	/* Test de WSAConnect */
	if ( iRes == INVALID_SOCKET )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("WSAConnect == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED:		OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:				OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEADDRINUSE:			OutputDebugString("WSAEADDRINUSE : The local address of the socket is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during the execution of bind, but could be delayed until this function if the bind function operates on a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function.\n"); break;
			case WSAEINTR:					OutputDebugString("WSAEINTR : The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
			case WSAEINPROGRESS:			OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAEALREADY:				OutputDebugString("WSAEALREADY : A nonblocking connect/WSAConnect call is in progress on the specified socket.\n"); break;
			case WSAEADDRNOTAVAIL:		OutputDebugString("WSAEADDRNOTAVAIL : The remote address is not a valid address (such as ADDR_ANY).\n"); break;
			case WSAEAFNOSUPPORT:		OutputDebugString("WSAEAFNOSUPPORT : Addresses in the specified family cannot be used with this socket.\n"); break;
			case WSAEFAULT:				OutputDebugString("WSAEFAULT : The name or the namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the buffer length for lpCalleeData, lpSQOS, and lpGQOS are too small, or the buffer length for lpCallerData is too large.\n"); break;
			case WSAEINVAL:				OutputDebugString("WSAEINVAL : The parameter s is a listening socket, or the destination address specified is not consistent with that of the constrained group to which the socket belongs, or the lpGQOS parameter is not NULL.\n"); break;
			case WSAEISCONN:				OutputDebugString("WSAEISCONN : The socket is already connected (connection-oriented sockets only).\n"); break;
			case WSAENOBUFS:				OutputDebugString("WSAENOBUFS : No buffer space is available. The socket cannot be connected.\n"); break;
			case WSAENOTSOCK:				OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEOPNOTSUPP:			OutputDebugString("WSAEOPNOTSUPP : The FLOWSPEC structures specified in lpSQOS and lpGQOS cannot be satisfied.\n"); break;
			case WSAEPROTONOSUPPORT:	OutputDebugString("WSAEPROTONOSUPPORT : The lpCallerData parameter is not supported by the service provider.\n"); break;
			case WSAEWOULDBLOCK:			OutputDebugString("WSAEWOULDBLOCK : The socket is marked as nonblocking and the connection cannot be completed immediately.\n"); break;
			case WSAEACCES:				OutputDebugString("WSAEACCES : Attempt to connect datagram socket to broadcast address failed because setsockopt SO_BROADCAST is not enabled.\n"); break;
			
			/* Pour ces 3 cas, on peut réssayer le WSAConnect ! */
			case WSAECONNREFUSED:		OutputDebugString("WSAECONNREFUSED : The attempt to connect was rejected.\n"); break;
			case WSAENETUNREACH:			OutputDebugString("WSAENETUNREACH : The network cannot be reached from this host at this time.\n"); break;
			case WSAETIMEDOUT:			OutputDebugString("WSAETIMEDOUT : Attempt to connect timed out without establishing a connection\n"); break;
			
			default:							OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	return iRes;
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
int tcp86_close( int s )
{
	int iRes = 0;

	if ( s < 0 )
	{
		return -1;
	}

	iRes = closesocket( (SOCKET) s );

	/* Test de closesocket */
	if ( iRes == INVALID_SOCKET )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("closesocket == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED:	OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEWOULDBLOCK:		OutputDebugString("WSAEWOULDBLOCK : The socket is marked as nonblocking and SO_LINGER is set to a nonzero time-out value.\n"); break;
			case WSAEINTR:				OutputDebugString("WSAEINTR : The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	return iRes;
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
int tcp86_listen(int s, int backlog)
{
	int iRes = 0;

	if ( (s < 0) || (backlog <= 0) )
	{
		return -1;
	}

	iRes = listen( (SOCKET) s, backlog );

	/* Test de listen */
	if ( iRes == INVALID_SOCKET )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("listen == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED:	OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEADDRINUSE:		OutputDebugString("WSAEADDRINUSE : The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during execution of the bind function, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function.\n"); break;
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : The socket has not been bound with bind.\n"); break;
			case WSAEISCONN:			OutputDebugString("WSAEISCONN : The socket is already connected.\n"); break;
			case WSAEMFILE:			OutputDebugString("WSAEMFILE : No more socket descriptors are available.\n"); break;
			case WSAENOBUFS:			OutputDebugString("WSAENOBUFS : No buffer space is available.\n"); break;
			case WSAEOPNOTSUPP:		OutputDebugString("WSAEOPNOTSUPP : The referenced socket is not of a type that supports the listen operation.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	return iRes;
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
int tcp86_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout)
{
	int iRes = 0;

	/* nfds ne sert à rien et les autres paramètres peuvent être optionels donc pas de test ! */

	iRes = select( nfds, readfds, writefds, exceptfds, timeout );

	/* Test de select */
	if ( iRes == 0 ) // time out
	{
		return -1;
	}

	if ( iRes == SOCKET_ERROR )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("select == SOCKET_ERROR\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED:	OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : The Windows Sockets implementation was unable to allocate needed resources for its internal operations, or the readfds, writefds, exceptfds, or timeval parameters are not part of the user address space.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : The time-out value is not valid, or all three descriptor parameters were null.\n"); break;
			case WSAEINTR:				OutputDebugString("WSAEINTR : A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : One of the descriptor sets contains an entry that is not a socket.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	return iRes;
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
int tcp86_recv(int s, char * buf, int len, int flags)
{
	int				iRes = 0;
	DWORD				dwReceived = 0;
	DWORD				dwFlags = 0;
	WSABUF			wsaBuf;

	if ( (s < 0) || (buf == NULL) || (len == 0) )
	{
		return -1;
	}

	wsaBuf.buf = (char *) buf;
	wsaBuf.len = (u_long) len;

	ZeroMemory( wsaBuf.buf, wsaBuf.len );

	/////ok//iRes = recv( (SOCKET) s, buf, len, flags );
	iRes = WSARecv( (SOCKET) s, &wsaBuf, 1, &dwReceived, &dwFlags, NULL, NULL );
	
	if ( iRes == SOCKET_ERROR )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("recv == SOCKET_ERROR\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED: OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : The buf parameter is not completely contained in a valid part of the user address space.\n"); break;
			case WSAENOTCONN:			OutputDebugString("WSAENOTCONN : The socket is not connected.\n"); break;
			case WSAEINTR:				OutputDebugString("WSAEINTR : The (blocking) call was canceled through WSACancelBlockingCall.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAENETRESET:		OutputDebugString("WSAENETRESET : The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.\n"); break;
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEOPNOTSUPP:		OutputDebugString("WSAEOPNOTSUPP : MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.\n"); break;
			case WSAESHUTDOWN:		OutputDebugString("WSAESHUTDOWN : The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.\n"); break;
			case WSAEWOULDBLOCK:		OutputDebugString("WSAEWOULDBLOCK : The socket is marked as nonblocking and the receive operation would block.\n"); break;
			case WSAEMSGSIZE:			OutputDebugString("WSAEMSGSIZE : The message was too large to fit into the specified buffer and was truncated.\n"); break;
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.\n"); break;
			case WSAECONNABORTED:	OutputDebugString("WSAECONNABORTED : The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.\n"); break;
			case WSAETIMEDOUT:		OutputDebugString("WSAETIMEDOUT : The connection has been dropped because of a network failure or because the peer system failed to respond.\n"); break;
			case WSAECONNRESET:		OutputDebugString("WSAECONNRESET : The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP 'Port Unreachable' message.\n"); break;
			
			case WSA_IO_PENDING:		OutputDebugString("WSA_IO_PENDING : An overlapped operation was successfully initiated and completion will be indicated at a later time.\n");
											break; 
			default:						OutputDebugString("Unknown Error\n");
											break; 
		}

		WSASetLastError( iErr );

		return -1;
	}

	return dwReceived;
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
int tcp86_recvfrom(int s, char * buf, int len, int flags, struct sockaddr * from, int * fromlen)
{
	int		iRes			= 0;
	DWORD		dwReceived	= 0;
	DWORD		dwFlags		= 0;
	WSABUF	wsaBuf;

	if ( (s < 0) || (buf == NULL) || (len == 0) )
	{
		return -1;
	}

	wsaBuf.buf = (char *) buf;
	wsaBuf.len = (u_long) len;

	ZeroMemory( wsaBuf.buf, wsaBuf.len );

	//iRes = recvfrom( (SOCKET) s, buf, len, flags, from, fromlen );
	iRes = WSARecvFrom( (SOCKET) s, &wsaBuf, 1, &dwReceived, &dwFlags, from, fromlen, NULL, NULL );

	if ( iRes == SOCKET_ERROR )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("recvfrom == SOCKET_ERROR\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED: OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : The buf or from parameters are not part of the user address space, or the fromlen parameter is too small to accommodate the peer address.\n"); break;
			case WSAEISCONN:			OutputDebugString("WSAEISCONN : The socket is connected. This function is not permitted with a connected socket, whether the socket is connection oriented or connectionless.\n"); break;
			case WSAEINTR:				OutputDebugString("WSAEINTR : The (blocking) call was canceled through WSACancelBlockingCall.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAENETRESET:		OutputDebugString("WSAENETRESET : The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.\n"); break;
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEOPNOTSUPP:		OutputDebugString("WSAEOPNOTSUPP : MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.\n"); break;
			case WSAESHUTDOWN:		OutputDebugString("WSAESHUTDOWN : The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.\n"); break;
			case WSAEWOULDBLOCK:		OutputDebugString("WSAEWOULDBLOCK : The socket is marked as nonblocking and the receive operation would block.\n"); break;
			case WSAEMSGSIZE:			OutputDebugString("WSAEMSGSIZE : The message was too large to fit into the specified buffer and was truncated.\n"); break;
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.\n"); break;
			case WSAECONNABORTED:	OutputDebugString("WSAECONNABORTED : The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.\n"); break;
			case WSAETIMEDOUT:		OutputDebugString("WSAETIMEDOUT : The connection has been dropped because of a network failure or because the peer system failed to respond.\n"); break;
			case WSAECONNRESET:		OutputDebugString("WSAECONNRESET : The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP 'Port Unreachable' message.\n"); break;
			default:						OutputDebugString("Unknown Error\n"); break;
		}

		return -1;
	}

	return dwReceived;
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
int tcp86_send(int s, const char * buf, int len, int flags)
{
	int				iRes		= 0;
	DWORD				dwSent	= 0;
	DWORD				dwFlags	= 0;
	WSABUF			wsaBuf;

	if ( ( s < 0 ) || ( buf == NULL ) )
	{
		return -1;
	}

	wsaBuf.buf = (char *) buf;
	wsaBuf.len = (u_long) len;

	///iRes = send( (SOCKET) s, buf, len, flags );
	iRes = WSASend( (SOCKET) s, &wsaBuf, 1, &dwSent, dwFlags , NULL, NULL );

	/* Test de send */
	if ( iRes == SOCKET_ERROR )
	{
		int iErr = WSAGetLastError();

		OutputDebugString("send == SOCKET_ERROR\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED:			OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:					OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEACCES:					OutputDebugString("WSAEACCES : The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address.\n"); break;
			case WSAEINTR:						OutputDebugString("WSAEINTR : A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
			case WSAEINPROGRESS:				OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAEFAULT:					OutputDebugString("WSAEFAULT : The buf parameter is not completely contained in a valid part of the user address space.\n"); break;
			case WSAENETRESET:				OutputDebugString("WSAENETRESET : The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.\n"); break;
			case WSAENOBUFS:					OutputDebugString("WSAENOBUFS : No buffer space is available.\n"); break;
			case WSAENOTCONN:					OutputDebugString("WSAENOTCONN : The socket is not connected.\n"); break;
			case WSAENOTSOCK:					OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEOPNOTSUPP:				OutputDebugString("WSAEOPNOTSUPP : MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations\n"); break;
			case WSAESHUTDOWN:				OutputDebugString("WSAESHUTDOWN : The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.\n"); break;
			case WSAEWOULDBLOCK:				OutputDebugString("WSAEWOULDBLOCK : The socket is marked as nonblocking and the requested operation would block.\n"); break;
			case WSAEMSGSIZE:					OutputDebugString("WSAEMSGSIZE : The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.\n"); break;
			case WSAEHOSTUNREACH:			OutputDebugString("WSAEHOSTUNREACH : The remote host cannot be reached from this host at this time.\n"); break;
			case WSAEINVAL:					OutputDebugString("WSAEINVAL : The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.\n"); break;
			case WSAECONNABORTED:			OutputDebugString("WSAECONNABORTED : The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.\n"); break;
			case WSAECONNRESET:				OutputDebugString("WSAECONNRESET : The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a 'Port Unreachable' ICMP packet. The application should close the socket as it is no longer usable.\n"); break;
			case WSAETIMEDOUT:				OutputDebugString("WSAETIMEDOUT : The connection has been dropped, because of a network failure or because the system on the other end went down without notice.\n"); break;
			case WSA_OPERATION_ABORTED:	OutputDebugString("WSA_OPERATION_ABORTED : The overlapped operation has been canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in WSAIoctl.\n"); break; 

			case WSA_IO_PENDING:				OutputDebugString("WSA_IO_PENDING : An overlapped operation was successfully initiated and completion will be indicated at a later time.\n"); 
													break; 
			default:								OutputDebugString("Unknown Error\n");
													break; 
		}
	}

	return dwSent;
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
int tcp86_sendto(int s, const char * buf, int len, int flags, struct sockaddr * to, int tolen)
{
	int				iRes = 0;
	DWORD				dwSent	= 0;
	DWORD				dwFlags	= 0;
	WSABUF			wsaBuf;

	if ( (s < 0) || (buf == NULL) || (len == 0) || (to == NULL) || (tolen == 0) )
	{
		return -1;
	}

	wsaBuf.buf = (char *) buf;
	wsaBuf.len = (u_long) len;

	//iRes = sendto( (SOCKET) s, buf, len, flags, to, tolen );
	iRes = WSASendTo( (SOCKET) s, &wsaBuf, 1, &dwSent, dwFlags , to, tolen, NULL, NULL);

	/* Test de sendto */
	if ( iRes == INVALID_SOCKET )
	{
		int iErr = WSAGetLastError();
		
		OutputDebugString("sendto == INVALID_SOCKET\nWSAGetLastError : ");

		switch ( iErr )
		{
			case WSANOTINITIALISED: OutputDebugString("WSANOTINITIALISED : A successful WSAStartup call must occur before using this function.\n"); break;
			case WSAENETDOWN:			OutputDebugString("WSAENETDOWN : The network subsystem has failed.\n"); break;
			case WSAEACCES:			OutputDebugString("WSAEACCES : The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address.\n"); break;
			case WSAENETUNREACH:		OutputDebugString("WSAENETUNREACH : The network cannot be reached from this host at this time.\n"); break;
			case WSAEDESTADDRREQ:	OutputDebugString("WSAEDESTADDRREQ : A destination address is required.\n"); break;
			case WSAEAFNOSUPPORT:	OutputDebugString("WSAEAFNOSUPPORT : Addresses in the specified family cannot be used with this socket.\n"); break;
			case WSAEADDRNOTAVAIL:	OutputDebugString("WSAEADDRNOTAVAIL : The remote address is not a valid address, for example, ADDR_ANY.\n"); break;
			case WSAEFAULT:			OutputDebugString("WSAEFAULT : The buf or to parameters are not part of the user address space, or the tolen parameter is too small.\n"); break;
			case WSAEHOSTUNREACH:	OutputDebugString("WSAEHOSTUNREACH : The remote host cannot be reached from this host at this time.\n"); break;
			case WSAEINTR:				OutputDebugString("WSAEINTR : The (blocking) call was canceled through WSACancelBlockingCall.\n"); break;
			case WSAEINPROGRESS:		OutputDebugString("WSAEINPROGRESS : A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
			case WSAENETRESET:		OutputDebugString("WSAENETRESET : The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.\n"); break;
			case WSAENOTSOCK:			OutputDebugString("WSAENOTSOCK : The descriptor is not a socket.\n"); break;
			case WSAEOPNOTSUPP:		OutputDebugString("WSAEOPNOTSUPP : MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations.\n"); break;
			case WSAESHUTDOWN:		OutputDebugString("WSAESHUTDOWN : The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.\n"); break;
			case WSAEWOULDBLOCK:		OutputDebugString("WSAEWOULDBLOCK : The socket is marked as nonblocking and the requested operation would block.\n"); break;
			case WSAENOBUFS:			OutputDebugString("WSAENOBUFS : No buffer space is available.\n"); break;
			case WSAENOTCONN:			OutputDebugString("WSAENOTCONN : The socket is not connected (connection-oriented sockets only).\n"); break;
			case WSAEMSGSIZE:			OutputDebugString("WSAEMSGSIZE : The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.\n"); break;
			case WSAEINVAL:			OutputDebugString("WSAEINVAL : An unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.\n"); break;
			case WSAECONNABORTED:	OutputDebugString("WSAECONNABORTED : The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.\n"); break;
			case WSAETIMEDOUT:		OutputDebugString("WSAETIMEDOUT : The connection has been dropped, because of a network failure or because the system on the other end went down without notice.\n"); break;
			case WSAECONNRESET:		OutputDebugString("WSAECONNRESET : The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a 'Port Unreachable' ICMP packet. The application should close the socket as it is no longer usable.\n"); break;
			
			case WSA_IO_PENDING:		OutputDebugString("WSA_IO_PENDING : An overlapped operation was successfully initiated and completion will be indicated at a later time.\n"); 
											break; 
			default:						OutputDebugString("Unknown Error\n");
											break; 
		}
	}

	return dwSent;
}

/*
 * Fonctions utilisées avec le flag TCP86_0LD_STACK !
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
void tcp86_netinstall ( void )
{
	// utilisé 1 fois dans RTC_NOY.C uniquement si TCP86_0LD_STACK est defini
	return;
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
int tcp86_netinit (	struct tRTCNetControlBlock * pRTCNetControlBlock,
							Word * pBufPool, int * errno )
{
	// utilisé 1 fois dans RTC_NET.C uniquement si TCP86_0LD_STACK est defini
	return 0;
}

/*
 * Fonctions jamais utilisées !
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
int tcp86_netstart ( char * ifname, struct IFDESC ifdesc )
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_ifconfig ( char * ifname, struct IFCUST * ifcust )
{
	/* Jamais utilisé ! */
	return 0;
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
Word tcp86_serveur(void)
{
	/* Aucune Doc ! */
	/* Jamais utilisé ! */
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
Word tcp86_fct_serveur( void )
{
	/* Aucune Doc ! */
	/* Jamais utilisé ! */
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
int tcp86_shutdown(int s, int how)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_ping ( char * hostname, int len )
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_FTPgetput( char * hostname, char * userid, char * passwd, 
							char * buffer, int bufsize, char * file, int * filesize, int mode)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_TFTPget(char * host, char * buffer, int bufsize, char * file, int * filesize, int mode)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_TFTPput(char * host, char * buffer, int bufsize, char * file, int mode)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_stop(void)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_getsockname(int s, struct sockaddr * name, int * namelen)
{
	/* Jamais utilisé ! */
	// getsockname sous Windows
	return 0;
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
int tcp86_getsockopt(int s, int level, int optname, char * optval, int * optlen)
{
	/* Jamais utilisé ! */
	// getsockopt sous Windows
	return 0;
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
int tcp86_setsockopt(int s, int level, int optname, char * optval, int optlen)
{
	/* Jamais utilisé ! */
	// setsockopt sous Windows
	return 0;
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
int tcp86_read(int s, char * buff, int len)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_write(int s, char * buff, int len)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_recvmsg(int s, struct msghdr * msg, int flags)
{
	/* Jamais utilisé ! */
	return 0;
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
int tcp86_sendmsg(int s, struct msghdr * msg, int flags)
{
	/* Jamais utilisé ! */
	return 0;
}