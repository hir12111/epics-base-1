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
 *	Author Jeffrey O. Hill
 *	johill@lanl.gov
 *	505 665 1831
 */

#ifdef _MSC_VER
#   pragma warning(disable:4355)
#endif

#define epicsAssertAuthor "Jeff Hill johill@lanl.gov"

#define epicsExportSharedSymbols
#include "iocinf.h"
#include "oldAccess.h"
#include "cac.h"

epicsSingleton < tsFreeList < struct oldChannelNotify, 1024 > > oldChannelNotify::pFreeList;

extern "C" void cacNoopAccesRightsHandler ( struct access_rights_handler_args )
{
}

oldChannelNotify::oldChannelNotify ( ca_client_context & cacIn, const char *pName, 
        caCh * pConnCallBackIn, void * pPrivateIn, capri priority ) :
    cacCtx ( cacIn ), 
    io ( cacIn.createChannel ( pName, *this, priority ) ), 
    pConnCallBack ( pConnCallBackIn ), 
    pPrivate ( pPrivateIn ), pAccessRightsFunc ( cacNoopAccesRightsHandler ),
    ioSeqNo ( cacIn.sequenceNumberOfOutstandingIO () ),
    currentlyConnected ( false ), prevConnected ( false )
{
    // no need to worry about a connect preempting here because
    // the connect sequence will not start untill initiateConnect()
    // is called
    if ( pConnCallBackIn == 0 ) {
        this->cacCtx.incrementOutstandingIO ( cacIn.sequenceNumberOfOutstandingIO () );
    }
}

oldChannelNotify::~oldChannelNotify ()
{
    delete & this->io;

    // no need to worry about a connect preempting here because
    // the nciu has been deleted
    if ( this->pConnCallBack == 0 && ! this->currentlyConnected ) {
        this->cacCtx.decrementOutstandingIO ( this->ioSeqNo );
    }
}

void oldChannelNotify::setPrivatePointer ( void *pPrivateIn )
{
    this->pPrivate = pPrivateIn;
}

void * oldChannelNotify::privatePointer () const
{
    return this->pPrivate;
}

int oldChannelNotify::replaceAccessRightsEvent ( caArh *pfunc )
{
    // The order of the following is significant to guarantee that the
    // access rights handler is always gets called even if the channel connects
    // while this is running. There is some very small chance that the
    // handler could be called twice here with the same access rights state, but 
    // that will not upset the application.
    this->pAccessRightsFunc = pfunc ? pfunc : cacNoopAccesRightsHandler;
    if ( this->currentlyConnected ) {
        struct access_rights_handler_args args;
        args.chid = this;
        caAccessRights tmp = this->io.accessRights ();
        args.ar.read_access = tmp.readPermit ();
        args.ar.write_access = tmp.writePermit ();
        ( *pfunc ) ( args );
    }
    return ECA_NORMAL;
}

int oldChannelNotify::changeConnCallBack ( caCh * pfunc )
{
    epicsGuard < callbackMutex > callbackGuard =
            this->cacCtx.callbackGuardFactory ();
 
    if ( ! this->currentlyConnected ) {
         if ( pfunc ) { 
            if ( ! this->pConnCallBack ) {
                this->cacCtx.decrementOutstandingIO ( this->ioSeqNo );
            }
        }
        else {
            if ( this->pConnCallBack ) {
                this->cacCtx.incrementOutstandingIO ( this->ioSeqNo );
            }
        }
    }
    this->pConnCallBack = pfunc;

    return ECA_NORMAL;
}

void oldChannelNotify::connectNotify ()
{
    this->currentlyConnected = true;
    this->prevConnected = true;
    if ( this->pConnCallBack ) {
        struct connection_handler_args  args;
        args.chid = this;
        args.op = CA_OP_CONN_UP;
        ( *this->pConnCallBack ) ( args );
        
    }
    else {
        this->cacCtx.decrementOutstandingIO ( this->ioSeqNo );
    }

}

void oldChannelNotify::disconnectNotify ()
{
    this->currentlyConnected = false;
    if ( this->pConnCallBack ) {
        struct connection_handler_args args;
        args.chid = this;
        args.op = CA_OP_CONN_DOWN;
        ( *this->pConnCallBack ) ( args );
    }
    else {
        this->cacCtx.incrementOutstandingIO ( this->ioSeqNo );
    }
}

void oldChannelNotify::accessRightsNotify ( const caAccessRights &ar )
{
    struct access_rights_handler_args args;
    args.chid = this;
    args.ar.read_access = ar.readPermit();
    args.ar.write_access = ar.writePermit();
    ( *this->pAccessRightsFunc ) ( args );
}

void oldChannelNotify::exception ( int status, const char *pContext )
{
    this->cacCtx.exception ( status, pContext, __FILE__, __LINE__ );
}

void oldChannelNotify::readException ( int status, const char *pContext,
    unsigned type, arrayElementCount count, void * /* pValue */ )
{
    this->cacCtx.exception ( status, pContext, 
        __FILE__, __LINE__, *this, type, count, CA_OP_GET );
}

void oldChannelNotify::writeException ( int status, const char *pContext,
    unsigned type, arrayElementCount count )
{
    this->cacCtx.exception ( status, pContext, 
        __FILE__, __LINE__, *this, type, count, CA_OP_PUT );
}

void * oldChannelNotify::operator new ( size_t size )
{
    return oldChannelNotify::pFreeList->allocate ( size );
}

void oldChannelNotify::operator delete ( void *pCadaver, size_t size )
{
    oldChannelNotify::pFreeList->release ( pCadaver, size );
}
