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
 *      $Id$
 *
 *      Author  Jeffrey O. Hill
 *              johill@lanl.gov
 *              505 665 1831
 */

#include <math.h>

#include "epicsTimer.h"
#include "epicsEvent.h"
#include "epicsAssert.h"
#include "epicsGuard.h"
#include "tsFreeList.h"
#include "epicsSingleton.h"

static const double delayVerifyOffset = 1.0; // sec 

class delayVerify : public epicsTimerNotify {
public:
    delayVerify ( double expectedDelay, epicsTimerQueue & );
    void * operator new ( size_t size );
    void operator delete ( void *pCadaver, size_t size );
    void start ( const epicsTime &expireTime );
    void setBegin ( const epicsTime & );
    double delay () const;
    void checkError () const;
protected:
    virtual ~delayVerify ();
private:
    epicsTimer &timer;
    epicsTime beginStamp;
    epicsTime expireStamp;
    double expectedDelay;
    expireStatus expire ( const epicsTime & );
    static epicsSingleton < tsFreeList < class delayVerify, 0x20 > > pFreeList;
};

static unsigned expireCount;
static epicsEvent expireEvent;

delayVerify::delayVerify ( double expectedDelayIn, epicsTimerQueue &queueIn ) :
    timer ( queueIn.createTimer() ), expectedDelay ( expectedDelayIn )
{
}

delayVerify::~delayVerify ()
{
    this->timer.destroy ();
}

inline void delayVerify::setBegin ( const epicsTime &beginIn )
{
    this->beginStamp = beginIn;
}

inline double delayVerify::delay () const
{
    return delayVerifyOffset + this->expectedDelay;
}

void delayVerify::checkError () const
{
    double actualDelay =  this->expireStamp - this->beginStamp;
    double measuredError = actualDelay - delayVerifyOffset - this->expectedDelay;
    double percentError = measuredError / this->expectedDelay;
    percentError *= 100.0;
    if ( percentError > 0.5 ) {
        printf ( "TEST FAILED timer delay = %g sec, error = %g mSec (%f %%)\n", 
            this->expectedDelay, measuredError * 1000.0, percentError );
    }
}

inline void delayVerify::start ( const epicsTime &expireTime )
{
    this->timer.start ( *this, expireTime );
}

#ifdef _MSC_VER
#   pragma warning ( push )
#   pragma warning ( disable:4660 )
#endif

template class tsFreeList < class delayVerify, 0x20 >;
template class epicsSingleton < tsFreeList < class delayVerify, 0x20 > >;

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

epicsSingleton < tsFreeList < class delayVerify, 0x20 > > delayVerify::pFreeList;

inline void * delayVerify::operator new ( size_t size )
{
    return delayVerify::pFreeList->allocate ( size );
}

inline void delayVerify::operator delete ( void *pCadaver, size_t size )
{
    delayVerify::pFreeList->release ( pCadaver, size );
}

epicsTimerNotify::expireStatus delayVerify::expire ( const epicsTime &currentTime )
{
    this->expireStamp = currentTime;
    if ( --expireCount == 0u ) {
        expireEvent.signal ();
    }
    return noRestart;
}

//
// verify reasonable timer interval accuracy
//
void testAccuracy ()
{
    static const unsigned nTimers = 25u;
    delayVerify *pTimers[nTimers];
    unsigned i;

    epicsTimerQueueActive &queue = 
        epicsTimerQueueActive::allocate ( true, epicsThreadPriorityMax );

    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i] = new delayVerify ( ( nTimers - i ) * 0.1, queue );
        assert ( pTimers[i] );
    }
    expireCount = nTimers;
    for ( i = 0u; i < nTimers; i++ ) {
        epicsTime cur = epicsTime::getCurrent ();
        pTimers[i]->setBegin ( cur );
        pTimers[i]->start ( cur + pTimers[i]->delay () );
    }
    while ( expireCount != 0u ) {
        expireEvent.wait ();
    }
    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i]->checkError ();
    }
    queue.release ();
}

class cancelVerify : public epicsTimerNotify {
public:
    cancelVerify ( epicsTimerQueue & );
    void * operator new ( size_t size );
    void operator delete ( void *pCadaver, size_t size );
    void start ( const epicsTime &expireTime );
    void cancel ();
protected:
    virtual ~cancelVerify ();
private:
    epicsTimer &timer;
    bool failOutIfExpireIsCalled;
    expireStatus expire ( const epicsTime & );
    static epicsSingleton < tsFreeList < class cancelVerify, 0x20 > > pFreeList;
};

cancelVerify::cancelVerify ( epicsTimerQueue &queueIn ) :
    timer ( queueIn.createTimer () ), failOutIfExpireIsCalled ( false )
{
}

cancelVerify::~cancelVerify ()
{
    this->timer.destroy ();
}

inline void cancelVerify::start ( const epicsTime &expireTime )
{
    this->timer.start ( *this, expireTime );
}

#ifdef _MSC_VER
#   pragma warning ( push )
#   pragma warning ( disable:4660 )
#endif

template class tsFreeList < class cancelVerify, 0x20 >;
template class epicsSingleton < tsFreeList < class cancelVerify, 0x20 > >;

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

epicsSingleton < tsFreeList < class cancelVerify, 0x20 > > cancelVerify::pFreeList;

inline void * cancelVerify::operator new ( size_t size )
{
    return cancelVerify::pFreeList->allocate ( size );
}

inline void cancelVerify::operator delete ( void *pCadaver, size_t size )
{
    cancelVerify::pFreeList->release ( pCadaver, size );
}

inline void cancelVerify::cancel ()
{
    this->timer.cancel ();
    this->failOutIfExpireIsCalled = true;
}

epicsTimerNotify::expireStatus cancelVerify::expire ( const epicsTime & )
{
    double root = 3.14159;
    for ( unsigned i = 0u; i < 10000; i++ ) {
        root = sqrt ( root );
    }
    assert ( ! this->failOutIfExpireIsCalled );
    return noRestart;
}

//
// verify that when cancel() is calle dthe timer never runs again
//
void testCancel ()
{
    static const unsigned nTimers = 25u;
    cancelVerify *pTimers[nTimers];
    unsigned i;

    epicsTimerQueueActive &queue = 
        epicsTimerQueueActive::allocate ( true, epicsThreadPriorityMin );

    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i] = new cancelVerify ( queue );
        assert ( pTimers[i] );
    }
    epicsTime cur = epicsTime::getCurrent ();
    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i]->start ( cur + 4.0 );
    }
    epicsThreadSleep ( 5.0 );
    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i]->cancel ();
    }
    epicsThreadSleep ( 1.0 );
    queue.release ();
}

class expireDestroVerify : public epicsTimerNotify {
public:
    expireDestroVerify ( epicsTimerQueue & );
    void * operator new ( size_t size );
    void operator delete ( void *pCadaver, size_t size );
    void start ( const epicsTime &expireTime );
protected:
    virtual ~expireDestroVerify ();
private:
    epicsTimer & timer;
    expireStatus expire ( const epicsTime & );
    static epicsSingleton < tsFreeList < class expireDestroVerify, 0x20 > > pFreeList;
};

expireDestroVerify::expireDestroVerify ( epicsTimerQueue & queueIn ) :
    timer ( queueIn.createTimer () )
{
}

expireDestroVerify::~expireDestroVerify ()
{
    this->timer.destroy ();
}

inline void expireDestroVerify::start ( const epicsTime & expireTime )
{
    this->timer.start ( *this, expireTime );
}

#ifdef _MSC_VER
#   pragma warning ( push )
#   pragma warning ( disable:4660 )
#endif

template class tsFreeList < class expireDestroVerify, 0x20 >;
template class epicsSingleton < tsFreeList < class expireDestroVerify, 0x20 > >;

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

epicsSingleton < tsFreeList < class expireDestroVerify, 0x20 > > expireDestroVerify::pFreeList;

inline void * expireDestroVerify::operator new ( size_t size )
{
    return expireDestroVerify::pFreeList->allocate ( size );
}

inline void expireDestroVerify::operator delete ( void *pCadaver, size_t size )
{
    expireDestroVerify::pFreeList->release ( pCadaver, size );
}

epicsTimerNotify::expireStatus expireDestroVerify::expire ( const epicsTime & )
{
    delete this;
    return noRestart;
}

//
// verify that a timer can be destroyed in expire
//
void testExpireDestroy ()
{
    static const unsigned nTimers = 25u;
    expireDestroVerify *pTimers[nTimers];
    unsigned i;

    epicsTimerQueueActive &queue = 
        epicsTimerQueueActive::allocate ( true, epicsThreadPriorityMin );

    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i] = new expireDestroVerify ( queue );
        assert ( pTimers[i] );
    }
    epicsTime cur = epicsTime::getCurrent ();
    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i]->start ( cur );
    }
    epicsThreadSleep ( 5.0 );
    queue.release ();
}


class periodicVerify : public epicsTimerNotify {
public:
    periodicVerify ( epicsTimerQueue & );
    void * operator new ( size_t size );
    void operator delete ( void *pCadaver, size_t size );
    void start ( const epicsTime &expireTime );
    void cancel ();
    void verifyCount ();
protected:
    virtual ~periodicVerify ();
private:
    epicsTimer &timer;
    unsigned nExpire;
    bool failOutIfExpireIsCalled;
    expireStatus expire ( const epicsTime & );
    static epicsSingleton < tsFreeList < class periodicVerify, 0x20 > > pFreeList;
};

periodicVerify::periodicVerify ( epicsTimerQueue & queueIn ) :
    timer ( queueIn.createTimer () ), nExpire ( 0u ), 
        failOutIfExpireIsCalled ( false )
{
}

periodicVerify::~periodicVerify ()
{
    this->timer.destroy ();
}

inline void periodicVerify::start ( const epicsTime &expireTime )
{
    this->timer.start ( *this, expireTime );
}

#ifdef _MSC_VER
#   pragma warning ( push )
#   pragma warning ( disable:4660 )
#endif

template class tsFreeList < class periodicVerify, 0x20 >;
template class epicsSingleton < tsFreeList < class periodicVerify, 0x20 > >;

#ifdef _MSC_VER
#   pragma warning ( pop )
#endif

epicsSingleton < tsFreeList < class periodicVerify, 0x20 > > periodicVerify::pFreeList;

inline void * periodicVerify::operator new ( size_t size )
{
    return periodicVerify::pFreeList->allocate ( size );
}

inline void periodicVerify::operator delete ( void *pCadaver, size_t size )
{
    periodicVerify::pFreeList->release ( pCadaver, size );
}

inline void periodicVerify::cancel ()
{
    this->timer.cancel ();
    this->failOutIfExpireIsCalled = true;
}

inline void periodicVerify::verifyCount ()
{
    assert ( this->nExpire > 1u );
}

epicsTimerNotify::expireStatus periodicVerify::expire ( const epicsTime & )
{
    this->nExpire++;
    double root = 3.14159;
    for ( unsigned i = 0u; i < 10000; i++ ) {
        root = sqrt ( root );
    }
    assert ( ! this->failOutIfExpireIsCalled );
    double delay = rand ();
    delay = delay / RAND_MAX;
    delay /= 10.0;
    return expireStatus ( restart, delay );
}

//
// verify periodic timers
//
void testPeriodic ()
{
    static const unsigned nTimers = 25u;
    periodicVerify *pTimers[nTimers];
    unsigned i;

    epicsTimerQueueActive &queue = 
        epicsTimerQueueActive::allocate ( true, epicsThreadPriorityMin );

    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i] = new periodicVerify ( queue );
        assert ( pTimers[i] );
    }
    epicsTime cur = epicsTime::getCurrent ();
    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i]->start ( cur );
    }
    epicsThreadSleep ( 5.0 );
    for ( i = 0u; i < nTimers; i++ ) {
        pTimers[i]->verifyCount ();
        pTimers[i]->cancel ();
    }
    epicsThreadSleep ( 1.0 );
    queue.release ();
}

void epicsTimerTest ()
{
    testExpireDestroy ();
    testAccuracy ();
    testCancel ();
    testPeriodic ();
    printf ( "test complete\n" );
}
