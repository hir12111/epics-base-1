/* recStringin.c */
/* share/src/rec $Id$ */

/* recState.c - Record Support Routines for State records
 *
 *      Author: 	Janet Anderson
 *      Date:   	4/23/91
 *
 *      Author:
 *      Date:   mm-dd-yy
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  mm-dd-yy        iii     Comment
 */


#include        <vxWorks.h>
#include        <types.h>
#include        <stdioLib.h>
#include        <lstLib.h>
#include	<strLib.h>

#include        <alarm.h>
#include        <dbAccess.h>
#include        <dbDefs.h>
#include        <dbFldTypes.h>
#include        <devSup.h>
#include        <errMdef.h>
#include        <link.h>
#include        <recSup.h>
#include	<stringinRecord.h>

/* Create RSET - Record Support Entry Table*/
#define report NULL
#define initialize NULL
long init_record();
long process();
#define special NULL
long get_value();
#define cvt_dbaddr NULL
#define get_array_info NULL
#define put_array_info NULL
#define get_units NULL
#define get_precision NULL
#define get_enum_str NULL
#define get_enum_strs NULL
#define put_enum_str NULL
#define get_graphic_double NULL
#define get_control_double NULL
#define get_alarm_double NULL

struct rset stringinRSET={
	RSETNUMBER,
	report,
	initialize,
	init_record,
	process,
	special,
	get_value,
	cvt_dbaddr,
	get_array_info,
	put_array_info,
	get_units,
	get_precision,
	get_enum_str,
	get_enum_strs,
	put_enum_str,
	get_graphic_double,
	get_control_double,
	get_alarm_double };

struct stringindset { /* stringin input dset */
	long		number;
	DEVSUPFUN	dev_report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record; /*returns: (-1,0)=>(failure,success)*/
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_stringin;/*(0,1)=> success, async */
};
void monitor();

static long init_record(pstringin)
    struct stringinRecord	*pstringin;
{
    struct stringindset *pdset;
    long status;

    if(!(pdset = (struct stringindset *)(pstringin->dset))) {
	recGblRecordError(S_dev_noDSET,pstringin,"stringin: init_record");
	return(S_dev_noDSET);
    }
    /* must have read_stringin function defined */
    if( (pdset->number < 5) || (pdset->read_stringin == NULL) ) {
	recGblRecordError(S_dev_missingSup,pstringin,"stringin: init_record");
	return(S_dev_missingSup);
    }
    if( pdset->init_record ) {
	if((status=(*pdset->init_record)(pstringin,process))) return(status);
    }
    return(0);
}

static long process(paddr)
    struct dbAddr	*paddr;
{
    struct stringinRecord	*pstringin=(struct stringinRecord *)(paddr->precord);
	struct stringindset	*pdset = (struct stringindset *)(pstringin->dset);
	long		 status;

	if( (pdset==NULL) || (pdset->read_stringin==NULL) ) {
		pstringin->pact=TRUE;
		recGblRecordError(S_dev_missingSup,pstringin,"read_stringin");
		return(S_dev_missingSup);
	}

	status=(*pdset->read_stringin)(pstringin); /* read the new value */
	pstringin->pact = TRUE;

	/* status is one if an asynchronous record is being processed*/
	if (status==1) return(0);

	tsLocalTime(&pstringin->time);

	/* check event list */
	monitor(pstringin);

	/* process the forward scan link record */
	if (pstringin->flnk.type==DB_LINK) dbScanPassive(pstringin->flnk.value.db_link.pdbAddr);

	pstringin->pact=FALSE;
	return(status);
}


static long get_value(pstringin,pvdes)
    struct stringinRecord             *pstringin;
    struct valueDes     *pvdes;
{
    pvdes->field_type = DBF_STRING;
    pvdes->no_elements=1;
    pvdes->pvalue = (caddr_t)(&pstringin->val[0]);
    return(0);
}


static void monitor(pstringin)
    struct stringinRecord             *pstringin;
{
    unsigned short  monitor_mask;
    short           stat,sevr,nsta,nsev;

    /* get previous stat and sevr  and new stat and sevr*/
    stat=pstringin->stat;
    sevr=pstringin->sevr;
    nsta=pstringin->nsta;
    nsev=pstringin->nsev;
    /*set current stat and sevr*/
    pstringin->stat = nsta;
    pstringin->sevr = nsev;
    pstringin->nsta = 0;
    pstringin->nsev = 0;

    /* Flags which events to fire on the value field */
    monitor_mask = 0;

    /* alarm condition changed this scan */
    if (stat!=nsta || sevr!=nsev) {
            /* post events for alarm condition change*/
            monitor_mask = DBE_ALARM;
            /* post stat and nsev fields */
            db_post_events(pstringin,&pstringin->stat,DBE_VALUE);
            db_post_events(pstringin,&pstringin->sevr,DBE_VALUE);
    }

    if(strncmp(pstringin->oval,pstringin->val,sizeof(pstringin->val))) {
        db_post_events(pstringin,&(pstringin->val[0]),monitor_mask|DBE_VALUE);
	strncpy(pstringin->oval,pstringin->val,sizeof(pstringin->val));
    }
    return;
}
