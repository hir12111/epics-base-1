/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
#ifndef INCregisterRecordDeviceDriverh
#define INCregisterRecordDeviceDriverh

#include "shareLib.h"
struct dbBase;

#ifdef __cplusplus
extern "C" {
#endif

int registerRecordDeviceDriver(struct dbBase *pdbbase);

#ifdef __cplusplus
}
#endif


#endif /* INCregisterRecordDeviceDriverh */
