/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
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
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>

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
#include "backend.h"
#include "dialogs.h"
#include "menus.h"
#include "engineoutput.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


/* Module variables */
int  windowMode = 1;

char *mem1, *mem2; // dummies, as this dialog can never be OK'ed
int highTextStart[2], highTextEnd[2];

int MemoProc P((Option *opt, int n, int x, int y, char *text, int index));

Option engoutOptions[] = {
{  0,  LL|T2T,           17, NULL, NULL, NULL, NULL, Icon, " " },
{  0, L2L|T2T|SAME_ROW, 163, NULL, NULL, NULL, NULL, Label, N_("engine name") },
{  0,     T2T|SAME_ROW,  30, NULL, NULL, NULL, NULL, Icon, " " },
{  0, R2R|T2T|SAME_ROW, 188, NULL, NULL, NULL, NULL, Label, N_("move") },
{  0,  RR|T2T|SAME_ROW,  80, NULL, NULL, NULL, NULL, Label, N_("NPS") },
{200, T_VSCRL | T_TOP,  500, NULL, (void*) &mem1, "", (char**) MemoProc, TextBox, "" },
{  0,         0,         0, NULL, NULL, "", NULL, Break , "" },
{  0,  LL|T2T,           17, NULL, NULL, NULL, NULL, Icon, " " },
{  0, L2L|T2T|SAME_ROW, 163, NULL, NULL, NULL, NULL, Label, N_("engine name") },
{  0,     T2T|SAME_ROW,  30, NULL, NULL, NULL, NULL, Icon, " " },
{  0, R2R|T2T|SAME_ROW, 188, NULL, NULL, NULL, NULL, Label, N_("move") },
{  0,  RR|T2T|SAME_ROW,  80, NULL, NULL, NULL, NULL, Label, N_("NPS") },
{200, T_VSCRL | T_TOP,  500, NULL, (void*) &mem2, "", (char**) MemoProc, TextBox, "" },
{   0,      NO_OK,       0, NULL, NULL, "", NULL, EndMark , "" }
};

int
MemoProc (Option *opt, int n, int x, int y, char *text, int index)
{   // user callback for mouse events in memo
    static int pressed; // keep track of button 3 state
    int start, end, currentPV = (opt != &engoutOptions[5]);

    switch(n) {
      case 0: // pointer motion
	if(!pressed) return FALSE; // only motion with button 3 down is of interest
	MovePV(x, y, 500/*lineGap + BOARD_HEIGHT * (squareSize + lineGap)*/);
	break;
      case 3: // press button 3
	pressed = 1;
	if(LoadMultiPV(x, y, text, index, &start, &end, currentPV)) {
	    highTextStart[currentPV] = start; highTextEnd[currentPV] = end;
	    HighlightText(&engoutOptions[currentPV ? 12 : 5], start, end, TRUE);
	}
	break;
      case -3: // release button 3
	pressed = 0;
        if(highTextStart[currentPV] != highTextEnd[currentPV])
            HighlightText(&engoutOptions[currentPV ? 12 : 5], highTextStart[currentPV], highTextEnd[currentPV], FALSE);
        highTextStart[currentPV] = highTextEnd[currentPV] = 0;
        UnLoadPV();
	break;
      default:
	return FALSE; // not meant for us; do regular event handler
    }
    return TRUE;
}

void
SetIcon (int which, int field, int nIcon)
{   // first call into xengineoutput.c to pick up icon pixmap
    if( nIcon ) DrawWidgetIcon(&engoutOptions[STRIDE*which + field - 1], nIcon);
}

void
DoSetWindowText (int which, int field, char *s_label)
{
	SetWidgetLabel (&engoutOptions[STRIDE*which + field - 1], s_label);
}

void
SetEngineOutputTitle (char *title)
{
	SetDialogTitle(EngOutDlg, title);
}


void
DoClearMemo (int which)
{
      SetWidgetText(&engoutOptions[STRIDE*which + MEMO], "", -1);
}

void
EngineOutputPopUp ()
{
    static int  needInit = TRUE;
    static char *title = N_("Engine output");

    if (GenericPopUp(engoutOptions, _(title), EngOutDlg, BoardWindow, NONMODAL, appData.topLevel)) {
	if(engoutOptions[STRIDE-1].type != Break)
	    DisplayFatalError(_("Mismatch of STRIDE in nengineoutput.c\nChange and recompile!"), 0, 2);
	AddHandler(&engoutOptions[MEMO], EngOutDlg, 6);
	AddHandler(&engoutOptions[MEMO+STRIDE], EngOutDlg, 6);
	if( needInit ) {
	    InitEngineOutput(&engoutOptions[0], &engoutOptions[MEMO]); // make icon bitmaps
	    needInit = FALSE;
	}
        SetEngineColorIcon( 0 );
        SetEngineColorIcon( 1 );
        SetEngineState( 0, STATE_IDLE, "" );
        SetEngineState( 1, STATE_IDLE, "" );
    } else {
	SetIconName(EngOutDlg, _(title));
	SetDialogTitle(EngOutDlg, _(title));
    }

    MarkMenu("View.EngineOutput", EngOutDlg);

    ShowThinkingEvent(); // [HGM] thinking: might need to prompt engine for thinking output
}

int
EngineOutputIsUp ()
{
    return shellUp[EngOutDlg];
}

int
EngineOutputDialogExists ()
{
    return DialogExists(EngOutDlg);
}

void
EngineOutputProc ()
{
  if (!PopDown(EngOutDlg)) EngineOutputPopUp();
}
