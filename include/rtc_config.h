/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE :	RTC
* FICHIER :	rtc_config.h
* LANGAGE :	C
* --------------------------------------------------------------------
* MOT-CLE :
* --------------------------------------------------------------------
* RESUME :
* --------------------------------------------------------------------
* DESCRIPTION :
* --------------------------------------------------------------------
* HISTORIQUE :
* $Log: rtc_config.h,v $
* Revision 1.1  2005/07/01 21:06:08  olivier
* Noyau d'émulation RTC + stack TCP (Real Time Craft - Tecsi) sous Windows XP.
*
 
    Rev 1.5   Feb 16 2005 16:12:32   VANHOUCKE
 Update
 
    Rev 1.4   Feb 04 2005 09:51:32   VANHOUCKE
 update
 
    Rev 1.3   Jan 27 2005 18:01:28   VANHOUCKE
 update
 
    Rev 1.2   Jan 19 2005 18:55:20   VANHOUCKE
 Ajout de la config des priorités
 
    Rev 1.1   Dec 24 2004 15:44:22   VANHOUCKE
 Update
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef __RTC_CONFIG__
#define __RTC_CONFIG__
#pragma once

#define	DL_MAX							0xff								/* Niveau maximum de debug */
#define	DL_MIN							0x00								/* Niveau minimum de debug */

#define RTC_PRIORITY_CLASS				REALTIME_PRIORITY_CLASS		/* HIGH_PRIORITY_CLASS ou REALTIME_PRIORITY_CLASS */
#define RTC_PRIORITY_OFFSET			0

/* Deux type de timer sont implémenté : utiliser l'un OU l'autre */
//#define  USE_MULTIMEDIA_TIMER											/* Très précis : tourne sur intérruption (gourmand) */
#define  USE_QUEUE_TIMER													/* Moins précis : sont des objets du kernel Windows */

//#define  USE_RTC_DEBUG													/* Utilisation des traces de debug */
#define  RTC_KERNEL_DEBUG_LEVEL		DL_MAX							/* 0 -> Min de debug (DL_MIN) | 1 -> Max de debug (DL_MAX) */

#define  RTC_KERNEL_DEBUG_FILENAME	"RTC_DEBUG.TXT"				/* Nom de fichier de trace de debug */

#define	RTCTICKTOMS						55									/* Convertion des ticks DOS en ms : 1 tick = 55 ms */

#define	RTC_KERNEL_VERSION			"1.0 beta"						/* Version du Kernel RTC */

#define	RTC_TASK_STACK_SIZE			0x10000							/* Taille de pile des tâches RTC (threads) */

//#define RTC_TCP_RECHERCHE_PROTOCOLE									// Force la fonction tcp86_socket à rechercher  
																					//	si le protocole existe dans le système

#endif __RTC_CONFIG__