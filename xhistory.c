/*
 * New (WinBoard-style) Move history for XBoard
 *
 * Copyright 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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
 * along with this program. If not, see http://www.gnu.org/licenses/. 
 *
 * ------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "backend.h"
#include "xhistory.h"
#include "dialogs.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

// templates for calls into back-end (= history.c; should be moved to history.h header shared with it!)
void FindMoveByCharIndex P(( int char_index ));

// variables in nhistory.c
extern Option historyOptions[];

// ------------- low-level front-end actions called by MoveHistory back-end -----------------

void
HighlightMove (int from, int to, Boolean highlight)
{
#ifdef TODO_GTK
    if(highlight)
	XawTextSetSelection( historyOptions[0].handle, from, to ); // for lack of a better method, use selection for highighting
#endif
}

void
ScrollToCurrent (int caretPos)
{
#ifdef TODO_GTK
    Arg args[10];
    char *s;
    int len;
    GetWidgetText(&historyOptions[0], &s);
    len = strlen(s);
    if(caretPos < 0 || caretPos > len) caretPos = len;
    if(caretPos > len-30) { // scroll to end, which causes no flicker
      static XEvent event;
      XtCallActionProc(historyOptions[0].handle, "end-of-file", &event, NULL, 0);
      return;
    }
    // the following leads to a very annoying flicker, even when no scrolling is done at all.
    XtSetArg(args[0], XtNinsertPosition, caretPos); // this triggers scrolling in Xaw
    XtSetArg(args[1], XtNdisplayCaret, False);
    XtSetValues(historyOptions[0].handle, args, 2);
#endif
}


// ------------------------------ callbacks --------------------------

char historyTranslations[] =
"<Btn3Down>: select-start() \n \
<Btn3Up>: extend-end() SelectMove() \n";

#ifdef TODO_GTK
void
SelectMove (Widget w, XEvent * event, String * params, Cardinal * nParams)
{
	XawTextPosition index, dummy;

	XawTextGetSelectionPos(w, &index, &dummy);
	FindMoveByCharIndex( index ); // [HGM] also does the actual moving to it, now
}
#endif

