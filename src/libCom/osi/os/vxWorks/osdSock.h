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
 * vxWorks specific socket include
 */

#ifndef osdSockH
#define osdSockH

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>

#include <sys/types.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <sockLib.h>
#include <ioLib.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ioLib.h>
#include <hostLib.h>
#include <selectLib.h>
/*This following is not defined in any vxWorks header files*/
int sysClkRateGet(void);

#ifdef __cplusplus
}
#endif
 
typedef int                     SOCKET;
#define INVALID_SOCKET		(-1)
#define SOCKERRNO               errno
#define SOCKERRSTR(ERRNO_IN)    (strerror(ERRNO_IN))
#define socket_close(S)         close(S)
#define SD_BOTH 2

/*
 * it is quite lame on WRS's part to assume that
 * a ptr is always the same as an int
 */
#define socket_ioctl(A,B,C)     ioctl(A,B,(int)C)
typedef int osiSockIoctl_t;
typedef int osiSocklen_t;

#define FD_IN_FDSET(FD) ((FD)<FD_SETSIZE&&(FD)>=0)

#define SOCK_EWOULDBLOCK EWOULDBLOCK
#define SOCK_ENOBUFS ENOBUFS
#define SOCK_ECONNRESET ECONNRESET
#define SOCK_ETIMEDOUT ETIMEDOUT
#define SOCK_EADDRINUSE EADDRINUSE
#define SOCK_ECONNREFUSED ECONNREFUSED
#define SOCK_ECONNABORTED ECONNABORTED
#define SOCK_EINPROGRESS EINPROGRESS
#define SOCK_EISCONN EISCONN
#define SOCK_EALREADY EALREADY
#define SOCK_EINVAL EINVAL
#define SOCK_EINTR EINTR
#define SOCK_EPIPE EPIPE
#define SOCK_EMFILE EMFILE
#define SOCK_SHUTDOWN ESHUTDOWN
#define SOCK_ENOTSOCK ENOTSOCK
#define SOCK_EBADF EBADF

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7F000001
#endif

#if ( defined (BSD) && ( BSD >= 44 ) )
#   define ifreq_size(pifreq) (pifreq->ifr_addr.sa_len + sizeof(pifreq->ifr_name))
#else
#   define ifreq_size(pifreq) sizeof(*pifreq)
#endif

#endif /*osdSockH*/
 

