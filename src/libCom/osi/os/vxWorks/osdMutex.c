/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* os/vxWorks/osdMutex.c */

/* Author:  Marty Kraimer Date:    25AUG99 */

#include <vxWorks.h>
#include <semLib.h>
#include <time.h>
#include <objLib.h>
#include <sysLib.h>
/* The following not defined in an vxWorks header */
int sysClkRateGet(void);


#include "epicsMutex.h"

epicsMutexId epicsMutexOsdCreate(void)
{
    return((epicsMutexId)
        semMCreate(SEM_DELETE_SAFE|SEM_INVERSION_SAFE|SEM_Q_PRIORITY));
}

void epicsMutexOsdDestroy(epicsMutexId id)
{
    semDelete((SEM_ID)id);
}

epicsMutexLockStatus epicsMutexLockWithTimeout(
    epicsMutexId id, double timeOut)
{
    int status;
    int ticks;

    if(timeOut<=0.0) {
        ticks = 0;
    } else {
        ticks = timeOut*sysClkRateGet();
        if(ticks<=0) ticks = 1;
    }
    status = semTake((SEM_ID)id,ticks);
    if(status==OK) return(epicsMutexLockOK);
    if(errno==S_objLib_OBJ_TIMEOUT) return(epicsMutexLockTimeout);
    return(epicsMutexLockError);
}

epicsMutexLockStatus epicsMutexTryLock(epicsMutexId id)
{
    int status;
    status = semTake((SEM_ID)id,NO_WAIT);
    if(status==OK) return(epicsMutexLockOK);
    if(errno==S_objLib_OBJ_UNAVAILABLE) return(epicsMutexLockTimeout);
    return(epicsMutexLockError);
}

void epicsMutexShow(epicsMutexId id,unsigned int level)
{
    semShow((SEM_ID)id,level);
}
