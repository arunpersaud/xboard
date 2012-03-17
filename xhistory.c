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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/cursorfont.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xhistory.h"
#include "xboard.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

// templates for calls into back-end (= history.c; should be moved to history.h header shared with it!)
void RefreshMemoContent P((void));
void MemoContentUpdated P((void));
void FindMoveByCharIndex P(( int char_index ));

// variables in xoptions.c
extern Option historyOptions[];

// ------------- low-level front-end actions called by MoveHistory back-end -----------------

void
HighlightMove (int from, int to, Boolean highlight)
{
    if(highlight)
	XawTextSetSelection( historyOptions[0].handle, from, to ); // for lack of a better method, use selection for highighting
}

void
ClearHistoryMemo ()
{
    ClearTextWidget(&historyOptions[0]);
}

// the bold argument says 0 = normal, 1 = bold typeface
// the colorNr argument says 0 = font-default, 1 = gray
int
AppendToHistoryMemo (char * text, int bold, int colorNr)
{
    return AppendText(&historyOptions[0], text); // for now ignore bold & color stuff, as Xaw cannot handle that
}

void
ScrollToCurrent (int caretPos)
{
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
}


// ------------------------------ callbacks --------------------------

char *historyText;
char historyTranslations[] =
"<Btn3Down>: select-start() \n \
<Btn3Up>: extend-end() SelectMove() \n";

void
SelectMove (Widget w, XEvent * event, String * params, Cardinal * nParams)
{
	XawTextPosition index, dummy;

	XawTextGetSelectionPos(w, &index, &dummy);
	FindMoveByCharIndex( index ); // [HGM] also does the actual moving to it, now
}

Option historyOptions[] = {
{ 0xD, 200, 400, NULL, (void*) &historyText, "", NULL, TextBox, "" },
{   0,  2,    0, NULL, (void*) NULL, "", NULL, EndMark , "" }
};

// ------------ standard entry points into MoveHistory code -----------

Boolean
MoveHistoryIsUp ()
{
    return shellUp[7];
}

Boolean
MoveHistoryDialogExists ()
{
    return shells[7] != NULL;
}

void
HistoryPopUp ()
{
    if(GenericPopUp(historyOptions, _("Move list"), 7))
	XtOverrideTranslations(historyOptions[0].handle, XtParseTranslationTable(historyTranslations));
    MarkMenu("menuView.Show Move History", 7);
}

void
HistoryShowProc ()
{
  if (!shellUp[7]) {
    ASSIGN(historyText, "");
    HistoryPopUp();
    RefreshMemoContent();
    MemoContentUpdated();
  } else PopDown(7);
  ToNrEvent(currentMove);
}
