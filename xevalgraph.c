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

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "dialogs.h"
#include "menus.h"
#include "xboard.h"
#include "evalgraph.h"
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

char *crWhite = "#FFFFB0";
char *crBlack = "#AD5D3D";
static Window eGraphWindow;

static GC pens[6]; // [HGM] put all pens in one array
static GC hbrHist[3];

// [HGM] front-end, added as wrapper to avoid use of LineTo and MoveToEx in other routines (so they can be back-end)
void
DrawSegment (int x, int y, int *lastX, int *lastY, int penType)
{
  static int curX, curY;

  if(penType != PEN_NONE)
    XDrawLine(xDisplay, eGraphWindow, pens[penType], curX, curY, x, y);
  if(lastX != NULL) { *lastX = curX; *lastY = curY; }
  curX = x; curY = y;
}

// front-end wrapper for drawing functions to do rectangles
void
DrawRectangle (int left, int top, int right, int bottom, int side, int style)
{
    XFillRectangle(xDisplay, eGraphWindow, hbrHist[side], left, top, right-left, bottom-top);
    if(style != FILLED)
      XDrawRectangle(xDisplay, eGraphWindow, pens[PEN_BLACK], left, top, right-left-1, bottom-top-1);
}

// front-end wrapper for putting text in graph
void
DrawEvalText (char *buf, int cbBuf, int y)
{
    // the magic constants 7 and 5 should really be derived from the font size somehow
    XDrawString(xDisplay, eGraphWindow, coordGC, MarginX - 2 - 7*cbBuf, y+5, buf, cbBuf);
}

// front-end
static Pixel
MakeColor (char *color)
{
    XrmValue vFrom, vTo;

    vFrom.addr = (caddr_t) color;
    vFrom.size = strlen(color);
    XtConvert(shells[EvalGraphDlg], XtRString, &vFrom, XtRPixel, &vTo);
    // test for NULL?

    return *(Pixel *) vTo.addr;
}

static GC
CreateGC (int width, char *fg, char *bg, int style)
{
    XtGCMask value_mask = GCLineWidth | GCLineStyle | GCForeground
      | GCBackground | GCFunction | GCPlaneMask;
    XGCValues gc_values;

    gc_values.plane_mask = AllPlanes;
    gc_values.line_width = width;
    gc_values.line_style = style;
    gc_values.function = GXcopy;

    gc_values.foreground = MakeColor(fg);
    gc_values.background = MakeColor(bg);

    return XtGetGC(shells[EvalGraphDlg], value_mask, &gc_values);
}

static int initDone = FALSE;

static void
InitializeEvalGraph (Option *opt)
{
  eGraphWindow = XtWindow(opt->handle);

  pens[PEN_BLACK]      = CreateGC(1, "black", "black", LineSolid);
  pens[PEN_DOTTED]     = CreateGC(1, "#A0A0A0", "#A0A0A0", LineOnOffDash);
  pens[PEN_BLUEDOTTED] = CreateGC(1, "#0000FF", "#0000FF", LineOnOffDash);
  pens[PEN_BOLD]       = CreateGC(3, crWhite, crWhite, LineSolid);
  pens[PEN_BOLD+1]     = CreateGC(3, crBlack, crBlack, LineSolid);
  hbrHist[0] = CreateGC(3, crWhite, crWhite, LineSolid);
  hbrHist[1] = CreateGC(3, crBlack, crBlack, LineSolid);
  hbrHist[2] = CreateGC(3, "#E0E0F0", "#E0E0F0", LineSolid);; // background (a bit blueish, for contrst with yellow curve)

  initDone = TRUE;
}

// The following stuff is really back-end (but too little to bother with a separate file)

static void
DisplayEvalGraph ()
{   // back-end painting; calls back front-end primitives for lines, rectangles and text
    char *t = MakeEvalTitle(_(title));
    if(t != title && nWidthPB < 340) t = MakeEvalTitle(nWidthPB < 240 ? "" : _("Eval"));
    PaintEvalGraph();
    SetDialogTitle(EvalGraphDlg, t);
}

static void
EvalClick (int x, int y)
{
    int index = GetMoveIndexFromPoint( x, y );

    if( index >= 0 && index < currLast ) ToNrEvent( index + 1 );
}

static Option *
EvalCallback (int button, int x, int y)
{
    if(!initDone) return NULL;

    switch(button) {
	case 10: // expose event
	    /* Create or recreate paint box if needed */
	    nWidthPB = x;
	    nHeightPB = y;
	    DisplayEvalGraph();
	    break;
	case 1: EvalClick(x, y); // left button
	default: break; // other buttons ignored
    }
    return NULL; // no context menu!
}

static Option graphOptions[] = {
{ 150, 0x9C, 300, NULL, (void*) &EvalCallback, NULL, NULL, Graph , "" },
{ 0, 2, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void
EvalGraphPopUp ()
{
    if (GenericPopUp(graphOptions, _(title), EvalGraphDlg, BoardWindow, NONMODAL, 1)) {
	InitializeEvalGraph(&graphOptions[0]); // first time: add callbacks and initialize pens
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

