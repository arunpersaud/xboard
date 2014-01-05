/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011, 2012, 2013,
 * 2014 Free Software Foundation, Inc.
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
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "dialogs.h"
#include "xboard.h"
#include "engineoutput.h"
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


/* Module variables */
static int currentPV;
static Pixmap icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle
static Widget memoWidget;


static void
ReadIcon (char *pixData[], int iconNr, Widget w)
{
    int r;

	if ((r=XpmCreatePixmapFromData(xDisplay, XtWindow(w),
				       pixData,
				       &(icons[iconNr]),
				       NULL, NULL /*&attr*/)) != 0) {
	  fprintf(stderr, _("Error %d loading icon image\n"), r);
	  exit(1);
	}
}

void
InitEngineOutput (Option *opt, Option *memo2)
{	// front-end, because it must have access to the pixmaps
	Widget w = opt->handle;
	memoWidget = memo2->handle;

        ReadIcon(WHITE_14,   nColorWhite, w);
        ReadIcon(BLACK_14,   nColorBlack, w);
        ReadIcon(UNKNOWN_14, nColorUnknown, w);

        ReadIcon(CLEAR_14,   nClear, w);
        ReadIcon(PONDER_14,  nPondering, w);
        ReadIcon(THINK_14,   nThinking, w);
        ReadIcon(ANALYZE_14, nAnalyzing, w);
}

void
DrawWidgetIcon (Option *opt, int nIcon)
{   // as we are already in X front-end, so do X-stuff here
    Arg arg;
    XtSetArg(arg, XtNleftBitmap, (XtArgVal) icons[nIcon]);
    XtSetValues(opt->handle, &arg, 1);
}

void
InsertIntoMemo (int which, char * text, int where)
{
	XawTextBlock t;
	Widget edit;

	/* the backend adds \r\n, which is needed for winboard,
	 * for xboard we delete them again over here */
	if(t.ptr = strchr(text, '\r')) *t.ptr = ' ';

	t.ptr = text; t.firstPos = 0; t.length = strlen(text); t.format = XawFmt8Bit;
	edit = XtNameToWidget(shells[EngOutDlg], which ? "*paneB.text" : "*paneA.text");
	XawTextReplace(edit, where, where, &t);
	if(where < highTextStart[which]) { // [HGM] multiPVdisplay: move highlighting
	    int len = strlen(text);
	    highTextStart[which] += len; highTextEnd[which] += len;
	    XawTextSetSelection( edit, highTextStart[which], highTextEnd[which] );
	}
}

//--------------------------------- PV walking ---------------------------------------

char memoTranslations[] =
":Ctrl<Key>c: CopyMemoProc() \n \
<Btn3Motion>: HandlePV() \n \
Shift<Btn3Down>: select-start() extend-end(PRIMARY) SelectPV(1) \n \
Any<Btn3Down>: select-start() extend-end(PRIMARY) SelectPV(0) \n \
<Btn3Up>: StopPV() \n";

void
SelectPV (Widget w, XEvent * event, String * params, Cardinal * nParams)
{	// [HGM] pv: translate click to PV line, and load it for display
	String val;
	int start, end;
	XawTextPosition index, dummy;
	int x, y;
	Arg arg;

	x = event->xmotion.x; y = event->xmotion.y;
	currentPV = (w != memoWidget);
	XawTextGetSelectionPos(w, &index, &dummy);
	XtSetArg(arg, XtNstring, &val);
	XtGetValues(w, &arg, 1);
	shiftKey = strcmp(params[0], "0");
	if(LoadMultiPV(x, y, val, index, &start, &end, currentPV)) {
	    XawTextSetSelection( w, start, end );
	    highTextStart[currentPV] = start; highTextEnd[currentPV] = end;
	}
}

void
StopPV (Widget w, XEvent * event, String * params, Cardinal * nParams)
{	// [HGM] pv: on right-button release, stop displaying PV
        XawTextUnsetSelection( w );
        highTextStart[currentPV] = highTextEnd[currentPV] = 0;
        UnLoadPV();
        XtCallActionProc(w, "beginning-of-file", event, NULL, 0);
}

//------------------------- Ctrl-C copying of memo texts ---------------------------

// Awfull code: first read our own primary selection into selected_fen_position,
//              and then transfer ownership of this to the clipboard, so that the
//              copy-position callback can fetch it there when somebody pastes it
// Worst of all is that I only added it because I did not know how to copy primary:
// my laptop has no middle button. Ctrl-C might not be needed at all... [HGM]

// cloned from CopyPositionProc. Abuse selected_fen_position to hold selection

Boolean SendPositionSelection(Widget w, Atom *selection, Atom *target,
		 Atom *type_return, XtPointer *value_return,
		 unsigned long *length_return, int *format_return); // from xboard.c

static void
MemoCB (Widget w, XtPointer client_data, Atom *selection,
	Atom *type, XtPointer value, unsigned long *len, int *format)
{
  if (value==NULL || *len==0) return; /* nothing had been selected to copy */
  selected_fen_position = value;
  selected_fen_position[*len]='\0'; /* normally this string is terminated, but be safe */
    XtOwnSelection(menuBarWidget, XA_CLIPBOARD(xDisplay),
		   CurrentTime,
		   SendPositionSelection,
		   NULL/* lose_ownership_proc */ ,
		   NULL/* transfer_done_proc */);
}

void
CopyMemoProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    if(appData.pasteSelection) return;
    if (selected_fen_position) free(selected_fen_position);
    XtGetSelectionValue(menuBarWidget,
      XA_PRIMARY, XA_STRING,
      /* (XtSelectionCallbackProc) */ MemoCB,
      NULL, /* client_data passed to PastePositionCB */

      /* better to use the time field from the event that triggered the
       * call to this function, but that isn't trivial to get
       */
      CurrentTime
    );
}

//------------------------------- pane switching -----------------------------------

void
ResizeWindowControls (int mode)
{   // another hideous kludge: to have only a single pane, we resize the
    // second to 5 pixels (which makes it too small to display anything)
    Widget form1, form2;
    Arg args[16];
    int j;
    Dimension ew_height, tmp;
    Widget shell = shells[EngOutDlg];

    form1 = XtNameToWidget(shell, "*paneA");
    form2 = XtNameToWidget(shell, "*paneB");

    j = 0;
    XtSetArg(args[j], XtNheight, (XtArgVal) &ew_height); j++;
    XtGetValues(form1, args, j);
    j = 0;
    XtSetArg(args[j], XtNheight, (XtArgVal) &tmp); j++;
    XtGetValues(form2, args, j);
    ew_height += tmp; // total height

    if(mode==0) {
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) 5); j++;
	XtSetValues(form2, args, j);
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) (ew_height-5)); j++;
	XtSetValues(form1, args, j);
    } else {
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) (ew_height/2)); j++;
	XtSetValues(form1, args, j);
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) (ew_height/2)); j++;
	XtSetValues(form2, args, j);
    }
}
