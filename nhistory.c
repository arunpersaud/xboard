/*
 * New (WinBoard-style) Move history for XBoard
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
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * ------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
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
void RefreshMemoContent P((void));
void MemoContentUpdated P((void));

// variables in xoptions.c
extern Option historyOptions[];

// ------------- low-level front-end actions called by MoveHistory back-end -----------------

void
ClearHistoryMemo ()
{
    SetWidgetText(&historyOptions[0], "", HistoryDlg);
}

// the bold argument says 0 = normal, 1 = bold typeface
// the colorNr argument says 0 = font-default, 1 = gray
int
AppendToHistoryMemo (char * text, int bold, int colorNr)
{
    return AppendText(&historyOptions[0], text); // for now ignore bold & color stuff, as Xaw cannot handle that
}

void
HighlightMove (int from, int to, Boolean highlight)
{
    HighlightText (&historyOptions[0], from, to, highlight);
}

char *historyText;

int
SelectMove (Option *opt, int n, int x, int y, char *text, int index)
{
	if(n != 3 && n != 1) return FALSE; // only on button-1 and 3 press
	FindMoveByCharIndex( index ); // [HGM] also does the actual moving to it, now
	return (n == 3);  // suppress context menu for button 3, but allow selection with button 1
}

Option historyOptions[] = {
{ 200, T_VSCRL | T_FILL | T_WRAP | T_TOP, 400, NULL, (void*) &historyText, NULL, (char**) &SelectMove, TextBox, "" },
{   0,           NO_OK,             0, NULL, (void*) NULL, "", NULL, EndMark , "" }
};

void
ScrollToCurrent (int caretPos)
{
    ScrollToCursor(&historyOptions[0], caretPos);
}

// ------------ standard entry points into MoveHistory code -----------

Boolean
MoveHistoryIsUp ()
{
    return shellUp[HistoryDlg];
}

Boolean
MoveHistoryDialogExists ()
{
    return DialogExists(HistoryDlg);
}

void
HistoryPopUp ()
{
    if(GenericPopUp(historyOptions, _("Move list"), HistoryDlg, BoardWindow, NONMODAL, appData.topLevel))
	AddHandler(&historyOptions[0], HistoryDlg, 0);
    MarkMenu("View.MoveHistory", HistoryDlg);
}

void
HistoryShowProc ()
{
  if (!shellUp[HistoryDlg]) {
    ASSIGN(historyText, "");
    HistoryPopUp();
    RefreshMemoContent();
    MemoContentUpdated();
  } else PopDown(HistoryDlg);
  ToNrEvent(currentMove);
}
