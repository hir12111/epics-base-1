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
 */

#define epicsAssertAuthor "Jeff Hill johill@lanl.gov"

#include "iocinf.h"
#include "comBuf.h"

epicsSingleton < tsFreeList < class comBuf, 0x20 > > comBuf::pFreeList;

bool comBuf::flushToWire ( wireSendAdapter & wire )
{
    unsigned occupied = this->occupiedBytes ();
    while ( occupied ) {
        unsigned nBytes = wire.sendBytes ( 
            &this->buf[this->nextReadIndex], occupied );
        if ( nBytes == 0u ) {
            return false;
        }
        this->nextReadIndex += nBytes;
        occupied = this->occupiedBytes ();
    }
    return true;
}

unsigned comBuf::push ( const epicsInt16 * pValue, unsigned nElem )
{
    nElem = this->unoccupiedElem ( sizeof (*pValue), nElem );
    for ( unsigned i = 0u; i < nElem; i++ ) {
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 8u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 0u );
    }
    return nElem;
}

unsigned comBuf::push ( const epicsUInt16 * pValue, unsigned nElem )
{
    nElem = this->unoccupiedElem ( sizeof (*pValue), nElem );
    for ( unsigned i = 0u; i < nElem; i++ ) {
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 8u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 0u );
    }
    return nElem;
}

unsigned comBuf::push ( const epicsInt32 * pValue, unsigned nElem )
{
    nElem = this->unoccupiedElem ( sizeof (*pValue), nElem );
    for ( unsigned i = 0u; i < nElem; i++ ) {
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 24u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 16u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 8u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 0u );
    }
    return nElem;
}

unsigned comBuf::push ( const epicsUInt32 * pValue, unsigned nElem )
{
    nElem = this->unoccupiedElem ( sizeof (*pValue), nElem );
    for ( unsigned i = 0u; i < nElem; i++ ) {
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 24u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 16u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 8u );
        this->buf[this->nextWriteIndex++] = 
            static_cast < epicsUInt8 > ( pValue[i] >> 0u );
    }
    return nElem;
}

unsigned comBuf::push ( const epicsFloat32 * pValue, unsigned nElem )
{
    nElem = this->unoccupiedElem ( sizeof (*pValue), nElem );
    for ( unsigned i = 0u; i < nElem; i++ ) {
        // allow native floating point formats to be converted to IEEE
        osiConvertToWireFormat ( pValue[i], &this->buf[this->nextWriteIndex] );
        this->nextWriteIndex += sizeof ( *pValue );
    }
    return nElem;
}

unsigned comBuf::push ( const epicsFloat64 * pValue, unsigned nElem )
{
    nElem = this->unoccupiedElem ( sizeof (*pValue), nElem );
    for ( unsigned i = 0u; i < nElem; i++ ) {
        // allow native floating point formats to be converted to IEEE
        osiConvertToWireFormat ( pValue[i], &this->buf[this->nextWriteIndex] );
        this->nextWriteIndex += sizeof ( *pValue );
    }
    return nElem;
}

// throwing the exception from a function that isnt inline 
// shrinks the GNU compiled object code
void comBuf::throwInsufficentBytesException ()
{
    throw insufficentBytesAvailable ();
}
