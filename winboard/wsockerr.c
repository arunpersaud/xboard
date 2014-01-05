/*
 * wsockerr.c
 *
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

/* Windows sockets error map */
/* These messages ought to be in the Windows message catalog! */

#include <windows.h>
#include <winsock.h>
#include "wsockerr.h"

ErrorMap errmap[] =
{ {WSAEINTR, "Interrupted system call"},
  {WSAEBADF, "Bad file number"},
  {WSAEACCES, "Permission denied"},
  {WSAEFAULT, "Bad address"},
  {WSAEINVAL, "Invalid argument"},
  {WSAEMFILE, "Too many open files"},
  {WSAEWOULDBLOCK, "Operation would block"},
  {WSAEINPROGRESS, "Operation now in progress"},
  {WSAEALREADY, "Operation already in progress"},
  {WSAENOTSOCK, "Socket operation on non-socket"},
  {WSAEMSGSIZE, "Message too long"},
  {WSAEPROTOTYPE, "Protocol wrong type for socket"},
  {WSAENOPROTOOPT, "Protocol not available"},
  {WSAEPROTONOSUPPORT, "Protocol not supported"},
  {WSAESOCKTNOSUPPORT, "Socket type not supported"},
  {WSAEOPNOTSUPP, "Operation not supported on socket"},
  {WSAEPFNOSUPPORT, "Protocol family not supported"},
  {WSAEAFNOSUPPORT, "Address family not supported by protocol family"},
  {WSAEADDRINUSE, "Address already in use"},
  {WSAEADDRNOTAVAIL, "Can't assign requested address"},
  {WSAENETDOWN, "Network is down"},
  {WSAENETUNREACH, "Network is unreachable"},
  {WSAENETRESET, "Network dropped connection on reset"},
  {WSAECONNABORTED, "Software caused connection abort"},
  {WSAECONNRESET, "Connection reset by peer"},
  {WSAENOBUFS, "No buffer space available"},
  {WSAEISCONN, "Socket is already connected"},
  {WSAENOTCONN, "Socket is not connected"},
  {WSAESHUTDOWN, "Can't send after socket shutdown"},
  {WSAETOOMANYREFS, "Too many references: can't splice"},
  {WSAETIMEDOUT, "Connection timed out"},
  {WSAECONNREFUSED, "Connection refused"},
  {WSAELOOP, "Too many levels of symbolic links"},
  {WSAENAMETOOLONG, "File name too long"},
  {WSAEHOSTDOWN, "Host is down"},
  {WSAEHOSTUNREACH, "No route to host"},
  {WSAENOTEMPTY, "Directory not empty"},
  {WSAEPROCLIM, "Too many processes"},
  {WSAEUSERS, "Too many users"},
  {WSAEDQUOT, "Disc quota exceeded"},
  {WSAESTALE, "Stale NFS file handle"},
  {WSAEREMOTE, "Too many levels of remote in path"},
  {WSAEDISCON, "Undocumented Winsock error code WSAEDISCON"},
  {WSASYSNOTREADY, "Winsock subsystem unusable"},
  {WSAVERNOTSUPPORTED, "Required Winsock version is not supported"},
  {WSANOTINITIALISED, "Winsock not initialized"},
  {WSAHOST_NOT_FOUND, "Host name not found by name server (authoritative)"},
  {WSATRY_AGAIN, "Host name not found by name server (nonauthoritative), or name server failure"},
  {WSANO_RECOVERY, "Nonrecoverable name server error"},
  {WSANO_DATA, "Host name has no address data of required type"},
  {0, NULL}
};
