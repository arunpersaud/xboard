/*
 * Evaluation graph
 *
 * Author: Alessandro Scotti (Dec 2005)
 * Translated to X by H.G.Muller (Nov 2009)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "dialogs.h"
#include "menus.h"
#include "xboard.h"
#include "evalgraph.h"
#include "xevalgraph.h"
#include "draw.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

#include <X11/xpm.h>

#ifdef SNAP
#include "wsnap.h"
#endif

#define _LL_ 100

Pixmap icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle
Widget outputField[2][7]; // [HGM] front-end array to translate output field to window handle
static char *title = N_("Evaluation graph");

//extern WindowPlacement wpEvalGraph;

Position evalGraphX = -1, evalGraphY = -1;
Dimension evalGraphW, evalGraphH;

/* Module variables */

Option *disp;

static Option *EvalCallback P((int button, int x, int y));

static int initDone = FALSE;

static void
InitializeEvalGraph (Option *opt, int w, int h)
{
  if(w == 0) {
    Arg args[10];
    XtSetArg(args[0], XtNwidth, &evalGraphW);
    XtSetArg(args[1], XtNheight, &evalGraphH);
    XtGetValues(opt->handle, args, 2);
    nWidthPB = evalGraphW; nHeightPB = evalGraphH;
  } else nWidthPB = w, nHeightPB = h;

  initDone = TRUE;
}

// The following stuff is really back-end (but too little to bother with a separate file)

static void
EvalClick (int x, int y)
{
    int index = GetMoveIndexFromPoint( x, y );

    if( index >= 0 && index < currLast ) ToNrEvent( index + 1 );
}

static Option graphOptions[] = {
{ 150, 0x9C, 300, NULL, (void*) &EvalCallback, NULL, NULL, Graph , "" },
{ 0, 2, 0, NULL, NULL, "", NULL, EndMark , "" }
};

static void
DisplayEvalGraph ()
{   // back-end painting; calls back front-end primitives for lines, rectangles and text
    char *t = MakeEvalTitle(_(title));
    nWidthPB = disp->max; nHeightPB = disp->value;
    if(t != title && nWidthPB < 340) t = MakeEvalTitle(nWidthPB < 240 ? "" : _("Eval"));
    PaintEvalGraph();
    GraphExpose(graphOptions, 0, 0, nWidthPB, nHeightPB);
    SetDialogTitle(EvalGraphDlg, t);
}

static Option *
EvalCallback (int button, int x, int y)
{
    if(!initDone) return NULL;

    switch(button) {
	case 10: // expose event
	    /* Create or recreate paint box if needed */
	    if(x != nWidthPB || y != nHeightPB) {
		InitializeEvalGraph(&graphOptions[0], x, y);
	    }
	    nWidthPB = x;
	    nHeightPB = y;
	    DisplayEvalGraph();
	    break;
	case 1: EvalClick(x, y); // left button
	default: break; // other buttons ignored
    }
    return NULL; // no context menu!
}

void
EvalGraphPopUp ()
{
    if (GenericPopUp(graphOptions, _(title), EvalGraphDlg, BoardWindow, NONMODAL, 1)) {
	InitializeEvalGraph(&graphOptions[0], 0, 0); // first time: add callbacks and initialize pens
	disp = graphOptions;
    } else {
	SetDialogTitle(EvalGraphDlg, _(title));
	SetIconName(EvalGraphDlg, _(title));
    }

    MarkMenu("View.EvaluationGraph", EvalGraphDlg);

//    ShowThinkingEvent(); // [HGM] thinking: might need to prompt engine for thinking output
}

void
EvalGraphPopDown ()
{
    PopDown(EvalGraphDlg);

//    ShowThinkingEvent(); // [HGM] thinking: might need to shut off thinking output
}

Boolean
EvalGraphIsUp ()
{
    return shellUp[EvalGraphDlg];
}

int
EvalGraphDialogExists ()
{
    return DialogExists(EvalGraphDlg);
}

void
EvalGraphProc ()
{
  if (!PopDown(EvalGraphDlg)) EvalGraphPopUp();
}

// This function is the interface to the back-end.

void
EvalGraphSet (int first, int last, int current, ChessProgramStats_Move * pvInfo)
{
    /* [AS] Danger! For now we rely on the pvInfo parameter being a static variable! */

    currFirst = first;
    currLast = last;
    currCurrent = current;
    currPvInfo = pvInfo;

    if( DialogExists(EvalGraphDlg) ) {
        DisplayEvalGraph();
    }
}

