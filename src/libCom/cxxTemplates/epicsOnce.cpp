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
 *  $Id$
 *
 *                              
 *                    L O S  A L A M O S
 *              Los Alamos National Laboratory
 *               Los Alamos, New Mexico 87545
 *                                  
 *  Copyright, 1986, The Regents of the University of California.
 *                                  
 *           
 *	Author Jeff Hill
 *	johill@lanl.gov
 *	505 665 1831
 */

#define epicsExportSharedSymbols
#include "epicsMutex.h"
#include "epicsSingleton.h"
#include "epicsGuard.h"
#include "epicsOnce.h"
#include "tsFreeList.h"

class epicsOnceImpl : public epicsOnce {
public:
    epicsOnceImpl ( epicsOnceNotify & notifyIn );
    void * operator new ( size_t size );
    void operator delete ( void * pCadaver, size_t size );
private:
    epicsOnceNotify & notify;
    bool onceFlag;
    void destroy ();
    void once ();
	static epicsSingleton < epicsMutex > pMutex;
    static epicsSingleton < tsFreeList < class epicsOnceImpl, 16 > > pFreeList;
};

#ifdef _MSC_VER
#   pragma warning ( push )
#   pragma warning ( disable:4660 )
#endif

template class epicsSingleton < epicsMutex >;
template class tsFreeList < class epicsOnceImpl, 16 >;
template class epicsSingleton < tsFreeList < class epicsOnceImpl, 16 > >;

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

epicsSingleton < epicsMutex > epicsOnceImpl::pMutex;
epicsSingleton < tsFreeList < class epicsOnceImpl, 16 > > epicsOnceImpl::pFreeList;

inline void * epicsOnceImpl::operator new ( size_t size )
{ 
    return epicsOnceImpl::pFreeList->allocate ( size );
}

inline void epicsOnceImpl::operator delete ( void *pCadaver, size_t size )
{ 
    epicsOnceImpl::pFreeList->release ( pCadaver, size );
}

inline epicsOnceImpl::epicsOnceImpl ( epicsOnceNotify & notifyIn ) : 
    notify ( notifyIn ), onceFlag ( false )
{
}

void epicsOnceImpl::once ()
{
    if ( ! this->onceFlag ) {
        epicsGuard < epicsMutex > guard ( *this->pMutex );
        if ( ! this->onceFlag ) {
            this->notify.initialize ();
            this->onceFlag = true;
        }
    }
}

void epicsOnceImpl::destroy ()
{
    delete this;
}

epicsOnce & epicsOnce::create ( epicsOnceNotify & notifyIn )
{
    // free list throws exception in no memory situation
    return * new epicsOnceImpl ( notifyIn );
}

epicsOnce::~epicsOnce ()
{
}

epicsOnceNotify::~epicsOnceNotify ()
{
}

