/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/*
 *	    $Id$
 *
 *	    WIN32 specific initialisation for bsd sockets,
 *	    based on Chris Timossi's base/src/ca/windows_depend.c,
 *      and also further additions by Kay Kasemir when this was in
 *	    dllmain.cc
 *
 *      7-1-97  -joh-
 *
 */

/*
 * ANSI C
 */
#include <stdio.h>
#include <stdlib.h>

/*
 * WIN32 specific
 */
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN 
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

/*
 * EPICS
 */
#define epicsExportSharedSymbols
#include "osiSock.h"
#include "errlog.h"
#include "epicsVersion.h"

/*
 * osiLocalAddr ()
 */
epicsShareFunc osiSockAddr epicsShareAPI osiLocalAddr ( SOCKET socket )
{
	static osiSockAddr  addr;
	static char     	init;
	int             	status;
	INTERFACE_INFO		*pIfinfo;
	INTERFACE_INFO      *pIfinfoList;
	unsigned			nelem;
	DWORD				numifs;
	DWORD				cbBytesReturned;

	if (init) {
		return addr;
	}

    init = 1;
    addr.sa.sa_family = AF_UNSPEC;

	/* only valid for winsock 2 and above */
	if (wsaMajorVersion() < 2 ) {
		return addr;
	}

	nelem = 10;
	pIfinfoList = (INTERFACE_INFO *) calloc ( nelem, sizeof (INTERFACE_INFO) );
	if (!pIfinfoList) {
		errlogPrintf ("calloc failed\n");
		return addr;
    }

	status = WSAIoctl (socket, SIO_GET_INTERFACE_LIST, NULL, 0,
						(LPVOID)pIfinfoList, nelem*sizeof(INTERFACE_INFO),
						&cbBytesReturned, NULL, NULL);

	if (status != 0 || cbBytesReturned == 0) {
		errlogPrintf ("WSAIoctl SIO_GET_INTERFACE_LIST failed %d\n",WSAGetLastError());
		free (pIfinfoList);		
		return addr;
    }

	numifs = cbBytesReturned / sizeof(INTERFACE_INFO);
	for (pIfinfo = pIfinfoList; pIfinfo < (pIfinfoList+numifs); pIfinfo++){

		/*
		 * dont use interfaces that have been disabled
		 */
		if (!(pIfinfo->iiFlags & IFF_UP)) {
			continue;
		}

		/*
		 * dont use the loop back interface
		 */
		if (pIfinfo->iiFlags & IFF_LOOPBACK) {
			continue;
		}

		addr.sa = pIfinfo->iiAddress.Address;

		/* Work around MS Winsock2 bugs */
        if (addr.sa.sa_family == 0) {
            addr.sa.sa_family = AF_INET;
        }

		free (pIfinfoList);
        return addr;
	}

    free (pIfinfoList);
	return addr;
}

/*
 * osiSockDiscoverBroadcastAddresses ()
 */
epicsShareFunc void epicsShareAPI osiSockDiscoverBroadcastAddresses
     (ELLLIST *pList, SOCKET socket, const osiSockAddr *pMatchAddr)
{
	int             	status;
	INTERFACE_INFO      *pIfinfo;
	INTERFACE_INFO      *pIfinfoList;
	unsigned			nelem;
	int					numifs;
	DWORD				cbBytesReturned;
    osiSockAddrNode     *pNewNode;

	/* only valid for winsock 2 and above */
	if (wsaMajorVersion() < 2 ) {
		fprintf(stderr, "Need to set EPICS_CA_AUTO_ADDR_LIST=NO for winsock 1\n");
		return;
	}

	nelem = 10;
	pIfinfoList = (INTERFACE_INFO *) calloc(nelem, sizeof(INTERFACE_INFO));
	if(!pIfinfoList){
		return;
	}

	status = WSAIoctl (socket, SIO_GET_INTERFACE_LIST, 
						NULL, 0,
						(LPVOID)pIfinfoList, nelem*sizeof(INTERFACE_INFO),
						&cbBytesReturned, NULL, NULL);

	if (status != 0 || cbBytesReturned == 0) {
		fprintf(stderr, "WSAIoctl SIO_GET_INTERFACE_LIST failed %d\n",WSAGetLastError());
		free(pIfinfoList);		
		return;
	}

	numifs = cbBytesReturned/sizeof(INTERFACE_INFO);
	for (pIfinfo = pIfinfoList; pIfinfo < (pIfinfoList+numifs); pIfinfo++){

		/*
		 * dont bother with interfaces that have been disabled
		 */
		if (!(pIfinfo->iiFlags & IFF_UP)) {
			continue;
		}

		/*
		 * dont use the loop back interface
		 */
		if (pIfinfo->iiFlags & IFF_LOOPBACK) {
			continue;
		}

		/*
		 * work around WS2 bug
		 */
		if (pIfinfo->iiAddress.Address.sa_family != AF_INET) {
			if (pIfinfo->iiAddress.Address.sa_family == 0) {
				pIfinfo->iiAddress.Address.sa_family = AF_INET;
			}
		}

		/*
		 * if it isnt a wildcarded interface then look for
		 * an exact match
		 */
        if (pMatchAddr->sa.sa_family != AF_UNSPEC) {
            if (pIfinfo->iiAddress.Address.sa_family != pMatchAddr->sa.sa_family) {
                continue;
            }
            if (pIfinfo->iiAddress.Address.sa_family != AF_INET) {
                continue;
            }
            if (pMatchAddr->sa.sa_family != AF_INET) {
                continue;
            }
		    if (pMatchAddr->ia.sin_addr.s_addr != htonl(INADDR_ANY)) {
			    if (pIfinfo->iiAddress.AddressIn.sin_addr.s_addr != pMatchAddr->ia.sin_addr.s_addr) {
				    continue;
			    }
		    }
        }

        pNewNode = (osiSockAddrNode *) calloc (1, sizeof(*pNewNode));
        if (pNewNode==NULL) {
            errlogPrintf ("osiSockDiscoverBroadcastAddresses(): no memory available for configuration\n");
            return;
        }

        if (pIfinfo->iiAddress.Address.sa_family == AF_INET && pIfinfo->iiFlags & IFF_BROADCAST) {
            const unsigned mask = pIfinfo->iiNetmask.AddressIn.sin_addr.s_addr;
            const unsigned bcast = pIfinfo->iiBroadcastAddress.AddressIn.sin_addr.s_addr;
            const unsigned addr = pIfinfo->iiAddress.AddressIn.sin_addr.s_addr;
            unsigned result = (addr & mask) | (bcast &~mask);
            pNewNode->addr.ia.sin_family = AF_INET;
            pNewNode->addr.ia.sin_addr.s_addr = result;
            pNewNode->addr.ia.sin_port = 0u;
        } 
        else {
            pNewNode->addr.sa = pIfinfo->iiBroadcastAddress.Address;
        }

		/*
		 * LOCK applied externally
		 */
        ellAdd (pList, &pNewNode->node);
	}

	free (pIfinfoList);
}
