/*
 * Evaluation graph
 *
 * Author: Alessandro Scotti (Dec 2005)
 * Translated to X by H.G.Muller (Nov 2009)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010 Free Software Foundation, Inc.
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

// [HGM] pixmaps of some ICONS used in the engine-outut window
#include "pixmaps/WHITE_14.xpm"
#include "pixmaps/BLACK_14.xpm"
#include "pixmaps/CLEAR_14.xpm"
#include "pixmaps/UNKNOWN_14.xpm"
#include "pixmaps/THINKING_14.xpm"
#include "pixmaps/PONDER_14.xpm"
#include "pixmaps/ANALYZING_14.xpm"

#ifdef SNAP
#include "wsnap.h"
#endif

#define _LL_ 100

typedef void (*DrawFunc)();
DrawFunc ChooseDrawFunc();

// imports from xboard.c
extern Widget formWidget, shellWidget, boardWidget, menuBarWidget;
extern Display *xDisplay;
extern Window xBoardWindow;
extern int squareSize;
extern Pixmap xMarkPixmap, wIconPixmap, bIconPixmap;
extern char *layoutName;
extern int lineGap;

Pixmap icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle
Widget outputField[2][7]; // [HGM] front-end array to translate output field to window handle

/* Imports from backend.c */

/* Imports from xboard.c */
extern Arg layoutArgs[2], formArgs[2], messageArgs[4];
extern GC coordGC;

//extern WindowPlacement wpEvalGraph;

/* Module variables */

static Display *zDisplay;
static Window promoWindow;
static int nrow, ncol;
extern Board promoBoard;

static GC blackBrush;

Widget promoShell;

void PromoPopDown()
{
  return;
  //  TODO
  
    if (!promoShell) return;
    XtPopdown(promoShell);
    XtDestroyWidget(promoShell);
    promoShell = NULL;
}

// front-end
static Pixel MakeColor(char *color )
{
    XrmValue vFrom, vTo;

    vFrom.addr = (caddr_t) color;
    vFrom.size = strlen(color);
    XtConvert(promoShell, XtRString, &vFrom, XtRPixel, &vTo);
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

    return XtGetGC(promoShell, value_mask, &gc_values);
}

void DrawPromoSquare(row, column, piece)
     int row, column;
     ChessSquare piece;
{
    int square_color, x, y;
    DrawFunc drawfunc;

    x = lineGap + column * (squareSize + lineGap);
    y = lineGap + row * (squareSize + lineGap);

    square_color = 1;

    if (piece == EmptySquare) {
	BlankSquare(x, y, square_color, piece, promoWindow);
    } else {
     //	drawfunc = ChooseDrawFunc();
	drawfunc(piece, square_color, x, y, promoWindow);
    }

}

// front-end. Create pens, device context and buffer bitmap for global use, copy result to display
// The back-end part n the middle has been taken out and moed to PainEvalGraph()
static void DisplayPromoDialog()
{
    int j, r, f;
    int width;
    int height;
    Dimension w, h;
    Arg args[6];

    /* Get client area */
    j = 0;
    XtSetArg(args[j], XtNwidth, &w); j++;
    XtSetArg(args[j], XtNheight, &h); j++;
    XtGetValues(promoShell, args, j);
    width = w;
    height = h;

    /* Create or recreate paint box if needed */
    if( width != nWidthPB || height != nHeightPB ) {

        nWidthPB = width;
        nHeightPB = height;
    }

    XFillRectangle(zDisplay, promoWindow, blackBrush, 0, 0, w, h);
    for(r = 0; r<nrow; r++) for(f=0; f<ncol; f++) 
	DrawPromoSquare(r, f, promoBoard[r][f+BOARD_LEFT]);

    XSync(zDisplay, False);
}

static void InitializePromoDialog()
{ int i;    XtGCMask value_mask = GCLineWidth | GCLineStyle | GCForeground
      | GCBackground | GCFunction | GCPlaneMask;
    XGCValues gc_values;

    blackBrush = CreateGC(3, "#000000", "#000000", LineSolid);
}

void PromoClick(widget, unused, event)
     Widget widget;
     caddr_t unused;
     XEvent *event;
{
    int boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);
    int boardWidth  = lineGap + BOARD_WIDTH  * (squareSize + lineGap);
    int boardLeft   = BOARD_LEFT  * (squareSize + lineGap);
    int x = event->xbutton.x;
    int y = event->xbutton.y;

    // translate click to as if it was on main board near a1
    if(flipView) x = boardWidth - x; else y = boardHeight - y;

    if( widget) {
	if(event->type == ButtonPress)   LeftClick(Press,   x + boardLeft, y);
	if(event->type == ButtonRelease) RightClick(Release, x + boardLeft, y, &y, &y);
    }
    PromoPopDown();
}

// This (cloned from EventProc in xboard.c) is needed as event handler, to prevent
// the graph being wiped out after covering / uncovering by other windows.
void PromoEventProc(widget, unused, event)
     Widget widget;
     caddr_t unused;
     XEvent *event;
{
    if (!XtIsRealized(widget))
      return;

    switch (event->type) {
      case Expose:
	if (event->xexpose.count > 0) return;  /* no clipping is done */
	DisplayPromoDialog();
	break;
      default:
	return;
    }
}

void PromoCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Widget w2;
    Arg args[16];
    char buf[80];
    VariantClass v;
    
    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    if (strcmp(name, _("clear board")) == 0) {
	EditPositionMenuEvent(ClearBoard, 0, 0);
    } else
    if (strcmp(name, _("grant rights")) == 0) {
	EditPositionMenuEvent(GrantRights, fromX, fromY);
    } else
    if (strcmp(name, _("revoke rights")) == 0) {
	EditPositionMenuEvent(NoRights, fromX, fromY);
    }
    PromoPopDown();
}

Widget PromoCreate(name, x, y, clear)
     char *name;
     int x, y;
     Boolean clear;
{
  Widget *tmp=NULL;
  return *tmp;
  //  TODO

    Arg args[16];
    Widget shell, layout, form, panel, b_clear, b_grant, b_revoke;
    int j, h, w;

    // define form within layout within shell.
    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
    shell =
      XtCreatePopupShell(name, transientShellWidgetClass,
			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    panel =
      XtCreateManagedWidget("panel", formWidgetClass, form,
			    formArgs, XtNumber(formArgs));
    // make sure width is known in advance, for better placement of child widgets
    j = 0;
    XtSetArg(args[j], XtNwidth,  (XtArgVal) (w = ncol*squareSize + (ncol+1)*lineGap)); j++;
    XtSetArg(args[j], XtNheight, (XtArgVal) (h = nrow*squareSize + (nrow+1)*lineGap)); j++;
    XtSetValues(panel, args, j);

    if(clear) { // piece menu: add buttons
	j=0;
	XtSetArg(args[j], XtNfromVert, panel);  j++;
	b_clear = XtCreateManagedWidget(_("clear board"), commandWidgetClass, form, args, j);
	XtAddCallback(b_clear, XtNcallback, PromoCallback, (XtPointer) 0);

	j=0;
	XtSetArg(args[j], XtNfromHoriz, b_clear);  j++;
	XtSetArg(args[j], XtNfromVert, panel);  j++;
	b_grant = XtCreateManagedWidget(_("grant rights"), commandWidgetClass, form, args, j);
	XtAddCallback(b_grant, XtNcallback, PromoCallback, (XtPointer) 0);

	j=0;
	XtSetArg(args[j], XtNfromHoriz, b_grant);  j++;
	XtSetArg(args[j], XtNfromVert, panel);  j++;
	b_revoke = XtCreateManagedWidget(_("revoke rights"), commandWidgetClass, form, args, j);
	XtAddCallback(b_revoke, XtNcallback, PromoCallback, (XtPointer) 0);
    }

    if(y >= 0) {
	Dimension bx, by;
	if(y > h) y -= h - 5; else y += 60;
	x -= w/2; if(x<10) x = 10;
	j = 0;
	XtSetArg(args[j], XtNx, &bx); j++;
	XtSetArg(args[j], XtNy, &by); j++;
	XtGetValues(boardWidget, args, j);
	j = 0;
	XtSetArg(args[j], XtNx, (XtArgVal) x+bx); j++;
	XtSetArg(args[j], XtNy, (XtArgVal) y+by); j++;
	XtSetValues(shell, args, j);
    }

    XtRealizeWidget(shell);

    zDisplay = XtDisplay(shell);
    promoWindow = XtWindow(panel);
    XtAddEventHandler(panel, ExposureMask, False,
		      (XtEventHandler) PromoEventProc, NULL);
    XtAddEventHandler(panel, ButtonPressMask|ButtonReleaseMask, False,
		      (XtEventHandler) PromoClick, NULL);

    return shell;
}

void 
PromoDialog(int h, int w, Board b, Boolean clear, char *title, int x, int y)
{
    Arg args[16];
    int j;
    Widget edit;
    static int  needInit = TRUE;

    nrow = h; ncol = w;

    if (promoShell == NULL) {

	promoShell =
	  PromoCreate(title, x, y, clear);
	XtRealizeWidget(promoShell);
	//	CatchDeleteWindow(promoShell, "PromoPopDown");
	InitializePromoDialog();
    }

    XtPopup(promoShell, XtGrabExclusive);
    XSync(zDisplay, False);
//    DisplayPromoDialog();
}

