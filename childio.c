/*
 * childio.c -- set up communication with child processes
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
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

/* This file splits into two entirely different pieces of code
   depending on whether USE_PTYS is 1.  The whole reason for all
   the pty nonsense is that select() does not work on pipes in System-V
   derivatives (at least some of them).  This is a problem because
   XtAppAddInput works by adding its argument to a select that is done
   deep inside Xlib.
*/

#include "config.h"

#include <stdio.h>
#include <signal.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "common.h"
#include "frontend.h"
#include "backend.h" /* for safeStrCpy */

#if !USE_PTYS
/* This code is for systems where pipes work properly */

void
SetUpChildIO (int to_prog[2], int from_prog[2])
{
    signal(SIGPIPE, SIG_IGN);
    pipe(to_prog);
    pipe(from_prog);
}

#else /* USE_PTYS == 1 */
/* This code is for all systems where we must use ptys */

#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#if HAVE_STROPTS_H
# include <stropts.h>
#endif /* HAVE_STROPTS_H */
#if HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#else /* not HAVE_SYS_FCNTL_H */
# if HAVE_FCNTL_H
#  include <fcntl.h>
# endif /* HAVE_FCNTL_H */
#endif /* not HAVE_SYS_FCNTL_H */

int PseudoTTY P((char pty_name[]));

int
SetUpChildIO (int to_prog[2], int from_prog[2])
{
    char pty_name[MSG_SIZ];

    if ((to_prog[1] = PseudoTTY(pty_name)) == -1) {
	DisplayFatalError("Can't open pseudo-tty", errno, 1);
	ExitEvent(1);
    }
    from_prog[0] = to_prog[1];
    to_prog[0] = from_prog[1] = open(pty_name, O_RDWR, 0);

#if HAVE_STROPTS_H /* do we really need this??  pipe-like behavior is fine */
    if (ioctl (to_prog[0], I_PUSH, "ptem") == -1 ||
	ioctl (to_prog[0], I_PUSH, "ldterm") == -1 ||
	ioctl (to_prog[0], I_PUSH, "ttcompat") == -1) {
# ifdef NOTDEF /* seems some systems don't have or need ptem and ttcompat */
	DisplayFatalError("Can't ioctl pseudo-tty", errno, 1);
	ExitEvent(1);
# endif /*NOTDEF*/
    }
#endif /* HAVE_STROPTS_H */

}

#if HAVE_GRANTPT
/* This code is for SVR4 */

int
PseudoTTY (char pty_name[])
{
    extern char *ptsname();
    char *ptss;
    int fd;

    fd = open("/dev/ptmx", O_RDWR);
    if (fd < 0) return fd;
    if (grantpt(fd) == -1) return -1;
    if (unlockpt(fd) == -1) return -1;
    if (!(ptss = ptsname(fd))) return -1;
    safeStrCpy(pty_name, ptss, sizeof(pty_name)/sizeof(pty_name[0]));
    return fd;
}

#else /* not HAVE_GRANTPT */
#if HAVE__GETPTY
/* This code is for IRIX */

int
PseudoTTY (char pty_name[])
{
    int fd;
    char *ptyn;

    ptyn = _getpty(&fd, O_RDWR, 0600, 0);
    if (ptyn == NULL) return -1;
    safeStrCpy(pty_name, ptyn, sizeof(pty_name)/sizeof(pty_name[0]));
    return fd;
}

#else /* not HAVE__GETPTY */
#if HAVE_LIBSEQ
/* This code is for Sequent DYNIX/ptx.  Untested. --tpm */

int
PseudoTTY (char pty_name[])
{
    int fd;
    char *slave, *master;

    fd = getpseudotty(&slave, &master);
    if (fd < 0) return fd;
    safeStrCpy(pty_name, slave, sizeof(pty_name)/sizeof(pty_name[0]));
    return fd;
}

#else /* not HAVE_LIBSEQ */
/* This code is for all other systems */
/* The code is adapted from GNU Emacs 19.24 */

#ifndef FIRST_PTY_LETTER
#define FIRST_PTY_LETTER 'p'
#endif
#ifndef LAST_PTY_LETTER
#define LAST_PTY_LETTER 'z'
#endif

int
PseudoTTY (char pty_name[])
{
  struct stat stb;
  register c, i;
  int fd;

  /* Some systems name their pseudoterminals so that there are gaps in
     the usual sequence - for example, on HP9000/S700 systems, there
     are no pseudoterminals with names ending in 'f'.  So we wait for
     three failures in a row before deciding that we've reached the
     end of the ptys.  */
  int failed_count = 0;

#ifdef PTY_ITERATION
  PTY_ITERATION
#else
  for (c = FIRST_PTY_LETTER; c <= LAST_PTY_LETTER; c++)
    for (i = 0; i < 16; i++)
#endif
      {
#ifdef PTY_NAME_SPRINTF
	PTY_NAME_SPRINTF
#else
	  sprintf (pty_name, "/dev/pty%c%x", c, i);
#endif /* no PTY_NAME_SPRINTF */

#ifdef PTY_OPEN
	PTY_OPEN;
#else /* no PTY_OPEN */
	if (stat (pty_name, &stb) < 0)
	  {
	    failed_count++;
	    if (failed_count >= 3)
	      return -1;
	  }
	else
	  failed_count = 0;
	fd = open (pty_name, O_RDWR, 0);
#endif /* no PTY_OPEN */

	if (fd >= 0)
	  {
	    /* check to make certain that both sides are available
	       this avoids a nasty yet stupid bug in rlogins */
#ifdef PTY_TTY_NAME_SPRINTF
	    PTY_TTY_NAME_SPRINTF
#else
	      sprintf (pty_name,  "/dev/tty%c%x", c, i);
#endif /* no PTY_TTY_NAME_SPRINTF */
#ifndef UNIPLUS
	    if (access (pty_name, 6) != 0)
	      {
		close (fd);
		continue;
	      }
#endif /* not UNIPLUS */
#ifdef IBMRTAIX
	      /* On AIX, the parent gets SIGHUP when a pty attached
                 child dies.  So, we ignore SIGHUP once we've started
                 a child on a pty.  Note that this may cause xboard
                 not to die when it should, i.e., when its own
                 controlling tty goes away.
	      */
	      signal(SIGHUP, SIG_IGN);
#endif /* IBMRTAIX */
	    return fd;
	  }
      }
  return -1;
}

#endif /* not HAVE_LIBSEQ */
#endif /* not HAVE__GETPTY */
#endif /* not HAVE_GRANTPT */
#endif /* USE_PTYS */
