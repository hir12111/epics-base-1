/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
#ifndef _EPICS_READLINE_H
#define _EPICS_READLINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <shareLib.h>
#include <stdio.h>

epicsShareFunc char * epicsShareAPI epicsReadline (FILE *fp, const char *prompt);
epicsShareFunc void epicsShareAPI epicsStifleHistory (int n);
epicsShareFunc void epicsShareAPI epicsAddHistory (const char *line);
epicsShareFunc void epicsShareAPI epicsBindKeys (void);

#ifdef __cplusplus
}
#endif

#endif /* _EPICS_READLINE_H */
