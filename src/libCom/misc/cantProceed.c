/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* callocMustSucceed.c */

/* Author:  Marty Kraimer Date:    04JAN99 */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#define epicsExportSharedSymbols
#include "errlog.h"
#include "cantProceed.h"
#include "epicsThread.h"

epicsShareFunc void * epicsShareAPI callocMustSucceed(size_t count, size_t size, const char *errorMessage)
{
    void *mem = calloc(count,size);
    if(mem==0) {
        errlogPrintf("%s callocMustSucceed failed count %d size %d\n",
            errorMessage,count,size);
        cantProceed(0);
    }
    return(mem);
}

epicsShareFunc void * epicsShareAPI mallocMustSucceed(size_t size, const char *errorMessage)
{
    void *mem = malloc(size);
    if(mem==0) {
        errlogPrintf("%s mallocMustSucceed failed size %d\n",
            errorMessage,size);
        cantProceed(0);
    }
    return(mem);
}

epicsShareFunc void epicsShareAPI cantProceed(const char *errorMessage)
{
    if(errorMessage) errlogPrintf("fatal error: %s\n",errorMessage);
    else errlogPrintf("fatal error\n");
    errlogFlush();
    epicsThreadSleep(1.0);
    epicsThreadSuspendSelf();
}
