/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

#include <string.h>
#include <stdio.h>

#ifndef vxWorks
#error this is a vxWorks specific source code
#endif

#include <hostLib.h>
#include <inetLib.h>

#define epicsExportSharedSymbols
#include "osiSock.h"

int osiSockAttach()
{
	return 1;
}

void osiSockRelease()
{
}

/*
 * ipAddrToHostName
 */
epicsShareFunc unsigned epicsShareAPI ipAddrToHostName 
            (const struct in_addr *pAddr, char *pBuf, unsigned bufSize)
{
	int				status;
	int				errnoCopy = errno;
    unsigned        len;

	if (bufSize<1) {
		return 0;
	}

    if (bufSize>MAXHOSTNAMELEN) {
	    status = hostGetByAddr ((int)pAddr->s_addr, pBuf);
	    if (status==OK) {
            pBuf[MAXHOSTNAMELEN] = '\0';
            len = strlen (pBuf);
        }
        else {
            len = 0;
        }
    }
    else {
	    char name[MAXHOSTNAMELEN+1];
	    status = hostGetByAddr (pAddr->s_addr, name);
	    if (status==OK) {
            strncpy (pBuf, name, bufSize);
            pBuf[bufSize-1] = '\0';
            len = strlen (pBuf);
	    }
        else {
            len = 0;
        }
    }

	errno = errnoCopy;

    return len;
}

/*
 * hostToIPAddr ()
 */
epicsShareFunc int epicsShareAPI hostToIPAddr 
				(const char *pHostName, struct in_addr *pIPA)
{
	int addr;

	addr = hostGetByName ((char *)pHostName);
	if (addr==ERROR) {
		/*
		 * return indicating an error
		 */
		return -1;
	}

	pIPA->s_addr = (unsigned long) addr;

	/*
	 * success
	 */
	return 0;
}

