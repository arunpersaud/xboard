/*
 * xtimer.c -- timing functions for X front end of XBoard
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

#define HIGHDRAG 1

#include "config.h"

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

#include <gtk/gtk.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* not STDC_HEADERS */
extern char *getenv();
# if HAVE_STRING_H
#  include <string.h>
# else /* not HAVE_STRING_H */
#  include <strings.h>
# endif /* not HAVE_STRING_H */
#endif /* not STDC_HEADERS */

#if HAVE_SYS_SYSTEMINFO_H
# include <sys/systeminfo.h>
#endif /* HAVE_SYS_SYSTEMINFO_H */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#include "common.h"
#include "backend.h"
#include "frontend.h"

#ifdef __EMX__
#ifndef HAVE_USLEEP
#define HAVE_USLEEP
#endif
#define usleep(t)   _sleep2(((t)+500)/1000)
#endif

guint delayedEventTimerTag = 0;
DelayedEventCallback delayedEventCallback = 0;

void
FireDelayedEvent(gpointer data)
{
    g_source_remove(delayedEventTimerTag);
    delayedEventTimerTag = 0;
    delayedEventCallback();
}

void
ScheduleDelayedEvent (DelayedEventCallback cb, long millisec)
{
    if(delayedEventTimerTag && delayedEventCallback == cb)
	// [HGM] alive: replace, rather than add or flush identical event
        g_source_remove(delayedEventTimerTag);
    delayedEventCallback = cb;
    delayedEventCallback = cb;
    delayedEventTimerTag = g_timeout_add(millisec,(GSourceFunc) FireDelayedEvent, NULL);
}

DelayedEventCallback
GetDelayedEvent ()
{
  if (delayedEventTimerTag) {
    return delayedEventCallback;
  } else {
    return NULL;
  }
}

void
CancelDelayedEvent ()
{
  if (delayedEventTimerTag) {
    g_source_remove(delayedEventTimerTag);
    delayedEventTimerTag = 0;
  }
}


guint loadGameTimerTag = 0;

int LoadGameTimerRunning()
{
    return loadGameTimerTag != 0;
}

int
StopLoadGameTimer ()
{
    if (loadGameTimerTag != 0) {
	g_source_remove(loadGameTimerTag);
	loadGameTimerTag = 0;
	return TRUE;
    } else {
	return FALSE;
    }
}

void
LoadGameTimerCallback(gpointer data)
{
    g_source_remove(loadGameTimerTag);
    loadGameTimerTag = 0;
    AutoPlayGameLoop();
}

void
StartLoadGameTimer (long millisec)
{
    loadGameTimerTag =
	g_timeout_add( millisec, (GSourceFunc) LoadGameTimerCallback, NULL);
}

guint analysisClockTag = 0;

void
AnalysisClockCallback(gpointer data)
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile
         || appData.icsEngineAnalyze) { // [DM]
	AnalysisPeriodicEvent(0);
    }
}

void
StartAnalysisClock ()
{
    analysisClockTag =
	g_timeout_add( 2000,(GSourceFunc) AnalysisClockCallback, NULL);
}

guint clockTimerTag = 0;

int
ClockTimerRunning ()
{
    return clockTimerTag != 0;
}

int
StopClockTimer ()
{
    if (clockTimerTag != 0)
    {
	g_source_remove(clockTimerTag);
	clockTimerTag = 0;
	return TRUE;
    } else {
	return FALSE;
    }
}

void
ClockTimerCallback(gpointer data)
{
    /* remove timer */
    g_source_remove(clockTimerTag);
    clockTimerTag = 0;

    DecrementClocks();
}

void
StartClockTimer (long millisec)
{
    clockTimerTag = g_timeout_add(millisec,(GSourceFunc) ClockTimerCallback,NULL);
}
