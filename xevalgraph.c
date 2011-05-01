/*
 * Evaluation graph
 *
 * Author: Alessandro Scotti (Dec 2005)
 * Translated to X by H.G.Muller (Nov 2009)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011 Free Software Foundation, Inc.
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

//extern WindowPlacement wpEvalGraph;

Position evalGraphX = -1, evalGraphY = -1;
Dimension evalGraphW, evalGraphH;
Widget evalGraphShell;
static int evalGraphDialogUp;

/* Module variables */

char *crWhite = "#FFFFB0";
char *crBlack = "#AD5D3D";
static Display *yDisplay;
static Window eGraphWindow;

static GC pens[6]; // [HGM] put all pens in one array
static GC hbrHist[3];

#if 0
static HDC hdcPB = NULL;
static HBITMAP hbmPB = NULL;
#endif

// [HGM] front-end, added as wrapper to avoid use of LineTo and MoveToEx in other routines (so they can be back-end)
void DrawSegment( int x, int y, int *lastX, int *lastY, int penType )
{
  static int curX, curY;

  if(penType != PEN_NONE)
    XDrawLine(yDisplay, eGraphWindow, pens[penType], curX, curY, x, y);
  if(lastX != NULL) { *lastX = curX; *lastY = curY; }
  curX = x; curY = y;
}

// front-end wrapper for drawing functions to do rectangles
void DrawRectangle( int left, int top, int right, int bottom, int side, int style )
{
    XFillRectangle(yDisplay, eGraphWindow, hbrHist[side], left, top, right-left, bottom-top);
    if(style != FILLED)
      XDrawRectangle(yDisplay, eGraphWindow, pens[PEN_BLACK], left, top, right-left-1, bottom-top-1);
}

// front-end wrapper for putting text in graph
void DrawEvalText(char *buf, int cbBuf, int y)
{
    // the magic constants 7 and 5 should really be derived from the font size somehow
    XDrawString(yDisplay, eGraphWindow, coordGC, MarginX - 2 - 7*cbBuf, y+5, buf, cbBuf);
}

// front-end
static Pixel MakeColor(char *color )
{
    XrmValue vFrom, vTo;

    vFrom.addr = (caddr_t) color;
    vFrom.size = strlen(color);
    XtConvert(evalGraphShell, XtRString, &vFrom, XtRPixel, &vTo);
    // test for NULL?

    return *(Pixel *) vTo.addr;
}

static GC CreateGC(int width, char *fg, char *bg, int style)
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

    return XtGetGC(evalGraphShell, value_mask, &gc_values);
}

// front-end. Create pens, device context and buffer bitmap for global use, copy result to display
// The back-end part n the middle has been taken out and moed to PainEvalGraph()
static void DisplayEvalGraph()
{
    int j;
    int width;
    int height;
    Dimension w, h;
    Arg args[6];

    /* Get client area */
    j = 0;
    XtSetArg(args[j], XtNwidth, &w); j++;
    XtSetArg(args[j], XtNheight, &h); j++;
    XtGetValues(evalGraphShell, args, j);
    width = w;
    height = h;

    /* Create or recreate paint box if needed */
    if( width != nWidthPB || height != nHeightPB ) {

        nWidthPB = width;
        nHeightPB = height;
    }

    // back-end painting; calls back front-end primitives for lines, rectangles and text
    PaintEvalGraph();

    XSync(yDisplay, False);
}

static void InitializeEvalGraph()
{
  pens[PEN_BLACK]      = CreateGC(1, "black", "black", LineSolid);
  pens[PEN_DOTTED]     = CreateGC(1, "#A0A0A0", "#A0A0A0", LineOnOffDash);
  pens[PEN_BLUEDOTTED] = CreateGC(1, "#0000FF", "#0000FF", LineOnOffDash);
  pens[PEN_BOLD]       = CreateGC(3, crWhite, crWhite, LineSolid);
  pens[PEN_BOLD+1]     = CreateGC(3, crBlack, crBlack, LineSolid);
  hbrHist[0] = CreateGC(3, crWhite, crWhite, LineSolid);
  hbrHist[1] = CreateGC(3, crBlack, crBlack, LineSolid);
  hbrHist[2] = CreateGC(3, "#E0E0F0", "#E0E0F0", LineSolid);; // background (a bit blueish, for contrst with yellow curve)
}

void EvalClick(widget, unused, event)
     Widget widget;
     caddr_t unused;
     XEvent *event;
{
        if( widget && event->type == ButtonPress ) {
            int index = GetMoveIndexFromPoint( event->xbutton.x, event->xbutton.y );

            if( index >= 0 && index < currLast ) {
                ToNrEvent( index + 1 );
            }
        }
}

// This (cloned from EventProc in xboard.c) is needed as event handler, to prevent
// the graph being wiped out after covering / uncovering by other windows.
void EvalEventProc(widget, unused, event)
     Widget widget;
     caddr_t unused;
     XEvent *event;
{
    if (!XtIsRealized(widget))
      return;

    switch (event->type) {
      case Expose:
	if (event->xexpose.count > 0) return;  /* no clipping is done */
	DisplayEvalGraph();
	break;
      default:
	return;
    }
}
// The following routines are mutated clones of the commentPopUp routines

Widget EvalGraphCreate(name)
     char *name;
{
    Arg args[16];
    Widget shell, layout, form;
    Dimension bw_width, bw_height;
    int j;

    // get board width
    j = 0;
    XtSetArg(args[j], XtNwidth,  &bw_width);  j++;
    XtSetArg(args[j], XtNheight, &bw_height);  j++;
    XtGetValues(boardWidget, args, j);

    // define form within layout within shell.
    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
    shell =
#if TOPLEVEL
     XtCreatePopupShell(name, topLevelShellWidgetClass,
#else
      XtCreatePopupShell(name, transientShellWidgetClass,
#endif
			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    // divide window vertically into two equal parts, by creating two forms
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    // make sure width is known in advance, for better placement of child widgets
    j = 0;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) bw_width-16); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) bw_height/4); j++;
    XtSetValues(shell, args, j);

    XtRealizeWidget(shell);

    if(wpEvalGraph.width > 0) {
      evalGraphW = wpEvalGraph.width;
      evalGraphH = wpEvalGraph.height;
      evalGraphX = wpEvalGraph.x;
      evalGraphY = wpEvalGraph.y;
    }

    if (evalGraphX == -1) {
	int xx, yy;
	Window junk;
	evalGraphH = bw_height/4;
	evalGraphW = bw_width-16;

	XSync(xDisplay, False);
#ifdef NOTDEF
	/* This code seems to tickle an X bug if it is executed too soon
	   after xboard starts up.  The coordinates get transformed as if
	   the main window was positioned at (0, 0).
	   */
	XtTranslateCoords(shellWidget,
			  (bw_width - evalGraphW) / 2, 0 - evalGraphH / 2,
			  &evalGraphX, &evalGraphY);
#else  /*!NOTDEF*/
        XTranslateCoordinates(xDisplay, XtWindow(shellWidget),
			      RootWindowOfScreen(XtScreen(shellWidget)),
			      (bw_width - evalGraphW) / 2, 0 - evalGraphH / 2,
			      &xx, &yy, &junk);
	evalGraphX = xx;
	evalGraphY = yy;
#endif /*!NOTDEF*/
	if (evalGraphY < 0) evalGraphY = 0; /*avoid positioning top offscreen*/
    }
    j = 0;
    XtSetArg(args[j], XtNheight, evalGraphH);  j++;
    XtSetArg(args[j], XtNwidth, evalGraphW);  j++;
    XtSetArg(args[j], XtNx, evalGraphX);  j++;
    XtSetArg(args[j], XtNy, evalGraphY);  j++;
    XtSetValues(shell, args, j);

    yDisplay = XtDisplay(shell);
    eGraphWindow = XtWindow(form);
    XtAddEventHandler(form, ExposureMask, False,
		      (XtEventHandler) EvalEventProc, NULL);
    XtAddEventHandler(form, ButtonPressMask, False,
		      (XtEventHandler) EvalClick, NULL);

    return shell;
}

void
EvalGraphPopUp()
{
    Arg args[16];
    int j;
    static int  needInit = TRUE;
    static char *title = _("Evaluation graph");

    if (evalGraphShell == NULL) {

	evalGraphShell =
	  EvalGraphCreate(title);
	XtRealizeWidget(evalGraphShell);
	CatchDeleteWindow(evalGraphShell, "EvalGraphPopDown");
	if( needInit ) {
	    InitializeEvalGraph();
	    needInit = FALSE;
	}
    } else {
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(evalGraphShell, args, j);
    }

    XtPopup(evalGraphShell, XtGrabNone);
    XSync(yDisplay, False);

    j=0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Evaluation Graph"),
		args, j);

    evalGraphDialogUp = True;
//    ShowThinkingEvent(); // [HGM] thinking: might need to prompt engine for thinking output
}

void EvalGraphPopDown()
{
    Arg args[16];
    int j;

    if (!evalGraphDialogUp) return;
    j = 0;
    XtSetArg(args[j], XtNx, &evalGraphX); j++;
    XtSetArg(args[j], XtNy, &evalGraphY); j++;
    XtSetArg(args[j], XtNwidth, &evalGraphW); j++;
    XtSetArg(args[j], XtNheight, &evalGraphH); j++;
    XtGetValues(evalGraphShell, args, j);
    wpEvalGraph.x = evalGraphX - 4;
    wpEvalGraph.y = evalGraphY - 23;
    wpEvalGraph.width = evalGraphW;
    wpEvalGraph.height = evalGraphH;
    XtPopdown(evalGraphShell);
    XSync(xDisplay, False);
    j=0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Evaluation Graph"),
		args, j);

    evalGraphDialogUp = False;
//    ShowThinkingEvent(); // [HGM] thinking: might need to shut off thinking output
}

Boolean EvalGraphIsUp()
{
    return evalGraphDialogUp;
}

int EvalGraphDialogExists()
{
    return evalGraphShell != NULL;
}

void
EvalGraphProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
  if (evalGraphDialogUp) {
    EvalGraphPopDown();
  } else {
    EvalGraphPopUp();
  }
}
// This function is the interface to the back-end. It is currently called through the front-end,
// though, where it shares the HistorySet() wrapper with MoveHistorySet(). Once all front-ends
// support the eval graph, it would be more logical to call it directly from the back-end.
void EvalGraphSet( int first, int last, int current, ChessProgramStats_Move * pvInfo )
{
    /* [AS] Danger! For now we rely on the pvInfo parameter being a static variable! */

    currFirst = first;
    currLast = last;
    currCurrent = current;
    currPvInfo = pvInfo;

    if( evalGraphShell ) {
        DisplayEvalGraph();
    }
}
