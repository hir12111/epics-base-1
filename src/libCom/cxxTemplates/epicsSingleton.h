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
 *  Author: Jeff O. Hill
 *
 */

#ifndef epicsSingleton_h
#define epicsSingleton_h

#include "shareLib.h"
#include "epicsMutex.h"
#include "epicsGuard.h"
#include "epicsThread.h"

template < class TYPE >
class epicsSingleton {
public:
    epicsSingleton ();
    ~epicsSingleton ();
    TYPE * operator -> ();
    const TYPE * operator -> () const;
    TYPE & operator * ();
    const TYPE & operator * () const;
    TYPE * instance ();
    const TYPE * instance () const;
    bool isInstantiated () const;
private:
    TYPE * pSingleton;
    void factory ();
};

template < class TYPE >
inline epicsSingleton<TYPE>::epicsSingleton () : pSingleton ( 0 )
{
}

template < class TYPE >
inline epicsSingleton<TYPE>::~epicsSingleton ()
{
    // this causes problems when there are file scope objects that reference the
    // singleton in their destructors
    // delete this->pSingleton;
}

epicsShareFunc epicsMutex & epicsSingletonPrivateMutex ();

template < class TYPE >
inline void epicsSingleton<TYPE>::factory ()
{
    epicsGuard < epicsMutex > guard ( epicsSingletonPrivateMutex() );
    if ( ! this->pSingleton ) {
        this->pSingleton = new TYPE;
    }
}

template < class TYPE >
inline TYPE * epicsSingleton<TYPE>::operator -> ()
{
    if ( ! this->pSingleton ) {
        this->factory ();
    }
    return this->pSingleton;
}

template < class TYPE >
inline const TYPE * epicsSingleton<TYPE>::operator -> () const
{
	epicsSingleton<TYPE> *p = const_cast < epicsSingleton<TYPE> * > ( this );
    return const_cast <const TYPE *> ( p->operator -> () );
}

template < class TYPE >
inline TYPE & epicsSingleton<TYPE>::operator * ()
{
    return * this->operator -> ();
}

template < class TYPE >
inline const TYPE & epicsSingleton<TYPE>::operator * () const
{
    return * this->operator -> ();
}

template < class TYPE >
inline TYPE * epicsSingleton<TYPE>::instance ()
{
    return this->operator -> ();
}

template < class TYPE >
inline const TYPE * epicsSingleton<TYPE>::instance () const
{
    return this->operator -> ();
}

template < class TYPE >
inline bool epicsSingleton<TYPE>::isInstantiated () const
{
    return ( this->pSingleton ? true : false );
}

#endif // epicsSingleton_h

