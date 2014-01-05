/*
 * usounds.c -- sound handling for XBoard (through external player)
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

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

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

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "common.h"
#include "frontend.h"


void
PlaySoundFile (char *name)
{
  if (*name == NULLCHAR) {
    return;
  } else if (strcmp(name, "$") == 0) {
    putc(BELLCHAR, stderr);
  } else {
    char buf[2048];
    char *prefix = "", *sep = "";
    if(appData.soundProgram[0] == NULLCHAR) return;
    if(!strchr(name, '/')) { prefix = appData.soundDirectory; sep = "/"; }
    snprintf(buf, sizeof(buf), "%s '%s%s%s' &", appData.soundProgram, prefix, sep, name);
    system(buf);
  }
}

void
RingBell ()
{
  PlaySoundFile(appData.soundMove);
}

void
PlayIcsWinSound ()
{
  PlaySoundFile(appData.soundIcsWin);
}

void
PlayIcsLossSound ()
{
  PlaySoundFile(appData.soundIcsLoss);
}

void
PlayIcsDrawSound ()
{
  PlaySoundFile(appData.soundIcsDraw);
}

void
PlayIcsUnfinishedSound ()
{
  PlaySoundFile(appData.soundIcsUnfinished);
}

void
PlayAlarmSound ()
{
  PlaySoundFile(appData.soundIcsAlarm);
}

void
PlayTellSound ()
{
  PlaySoundFile(appData.soundTell);
}

void
PlaySoundForColor (ColorClass cc)
{
    switch (cc) {
    case ColorShout:
      PlaySoundFile(appData.soundShout);
      break;
    case ColorSShout:
      PlaySoundFile(appData.soundSShout);
      break;
    case ColorChannel1:
      PlaySoundFile(appData.soundChannel1);
      break;
    case ColorChannel:
      PlaySoundFile(appData.soundChannel);
      break;
    case ColorKibitz:
      PlaySoundFile(appData.soundKibitz);
      break;
    case ColorTell:
      PlaySoundFile(appData.soundTell);
      break;
    case ColorChallenge:
      PlaySoundFile(appData.soundChallenge);
      break;
    case ColorRequest:
      PlaySoundFile(appData.soundRequest);
      break;
    case ColorSeek:
      PlaySoundFile(appData.soundSeek);
      break;
    case ColorNormal:
    case ColorNone:
    default:
      break;
    }
}
