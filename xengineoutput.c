/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009 Free Software Foundation, Inc.
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
// Add xengineo.h later
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

// imports from xboard.c
extern Widget formWidget, shellWidget, boardWidget, menuBarWidget;
extern Display *xDisplay;
extern Window xBoardWindow;
extern int squareSize;
extern Pixmap xMarkPixmap, wIconPixmap, bIconPixmap;
extern char *layoutName;

// temporary kludge to avoid compile errors untill all Windows code has been replaced
#define HICON int *
#define HWND  int *

// [HGM] define numbers to indicate icons, for referring to them in platform-independent way
#define nColorBlack   1
#define nColorWhite   2
#define nColorUnknown 3
#define nClear        4
#define nPondering    5
#define nThinking     6
#define nAnalyzing    7

Pixmap icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle

// [HGM] same for output fields (note that there are two of each type, one per color)
#define nColorIcon 1
#define nStateIcon 2
#define nLabel     3
#define nStateData 4
#define nLabelNPS  5
#define nMemo      6

Widget outputField[2][7]; // [HGM] front-end array to translate output field to window handle

void EngineOutputPopDown();
void engineOutputPopUp(char *title, char *text);
int  EngineOutputIsUp();
static void SetEngineColorIcon( int which );

#define SHOW_PONDERING

/* Imports from backend.c */
char * SavePart(char *str);
extern int opponentKibitzes;

/* Imports from winboard.c */
//extern HWND engineOutputDialog;
extern Arg layoutArgs[2], formArgs[2], messageArgs[4];

//extern WindowPlacement wpEngineOutput;

Position engineOutputX = -1, engineOutputY = -1;
Dimension engineOutputW, engineOutputH;
Widget engineOutputShell;
int engineOutputDialogUp;

/* Module variables */
#define H_MARGIN            2
#define V_MARGIN            2
#define LABEL_V_DISTANCE    1   /* Distance between label and memo */
#define SPLITTER_SIZE       4   /* Distance between first memo and second label */

#define ICON_SIZE           14

#define STATE_UNKNOWN   -1
#define STATE_THINKING   0
#define STATE_IDLE       1
#define STATE_PONDERING  2
#define STATE_ANALYZING  3

static int  windowMode = 1;

static int  needInit = TRUE;

static int  lastDepth[2] = { -1, -1 };
static int  lastForwardMostMove[2] = { -1, -1 };
static int  engineState[2] = { -1, -1 };

typedef struct {
    char * name;
    int which;
    int depth;
    u64 nodes;
    int score;
    int time;
    char * pv;
    char * hint;
    int an_move_index;
    int an_move_count;
} EngineOutputData;

static void VerifyDisplayMode();
static void UpdateControls( EngineOutputData * ed );
static void SetEngineState( int which, int state, char * state_data );

void ReadIcon(char *pixData[], int iconNr)
{
    int r;

	if ((r=XpmCreatePixmapFromData(xDisplay, XtWindow(outputField[0][nColorIcon]),
				       pixData,
				       &(icons[iconNr]),
				       NULL, NULL /*&attr*/)) != 0) {
	  fprintf(stderr, _("Error %d loading icon image\n"), r);
	  exit(1); 
	}	
}

static void InitializeEngineOutput()
{ int i;

        ReadIcon(WHITE_14,   nColorWhite);
        ReadIcon(BLACK_14,   nColorBlack);
        ReadIcon(UNKNOWN_14, nColorUnknown);

        ReadIcon(CLEAR_14,   nClear);
        ReadIcon(PONDER_14,  nPondering);
        ReadIcon(THINK_14,   nThinking);
        ReadIcon(ANALYZE_14, nAnalyzing);
}

void DoSetWindowText(int which, int field, char *s_label)
{ 
	Arg arg;

	XtSetArg(arg, XtNlabel, (XtArgVal) s_label);
	XtSetValues(outputField[which][field], &arg, 1);
}

static void InsertIntoMemo( int which, char * text )
{
	Arg arg; XawTextBlock t; Widget edit;

	t.ptr = text; t.firstPos = 0; t.length = strlen(text); t.format = XawFmt8Bit;
	edit = XtNameToWidget(engineOutputShell, which ? "*form2.text" : "*form.text");
	XawTextReplace(edit, 0, 0, &t);
//	XtSetArg(arg, XtNstring, (XtArgVal) text);
//	XtSetValues(outputField[which][nMemo], &arg, 1);
}

static void SetIcon( int which, int field, int nIcon )
{
    Arg arg;

    if( nIcon != 0 ) {
	XtSetArg(arg, XtNleftBitmap, (XtArgVal) icons[nIcon]);
	XtSetValues(outputField[which][field], &arg, 1);
    }
}

void DoClearMemo(int which)
{ 
    Arg args[16];
    int j;
    Widget edit;

	edit = XtNameToWidget(engineOutputShell, which ? "*form2.text" : "*form.text");
	XtCallActionProc(edit, "select-all", NULL, NULL, 0);
	XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
}

// The following routines are mutated clones of the commentPopUp routines

void PositionControlSet(which, form, bw_width)
     int which;
     Widget form;
     Dimension bw_width;
{
    Arg args[16];
    Widget edit, NameWidget, ColorWidget, ModeWidget, MoveWidget, NodesWidget;
    int j, mutable=1;
    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) ""); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNleft,      XtChainLeft); j++;
    XtSetArg(args[j], XtNright,     XtChainLeft); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) 17); j++;
    outputField[which][nColorIcon] = ColorWidget =
      XtCreateManagedWidget("Color", labelWidgetClass,
		     form, args, j);

    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNjustify,   (XtArgVal) XtJustifyLeft); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) ColorWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNleft,      XtChainLeft); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) bw_width/2 - 57); j++;
    outputField[which][nLabel] = NameWidget =
      XtCreateManagedWidget("Engine", labelWidgetClass,
		     form, args, j);

    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) ""); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) NameWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) 20); j++;
    outputField[which][nStateIcon] = ModeWidget =
      XtCreateManagedWidget("Mode", labelWidgetClass,
		     form, args, j);

    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNjustify,   (XtArgVal) XtJustifyLeft); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) ""); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) ModeWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNright,     XtChainRight); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) bw_width/2 - 102); j++;
    outputField[which][nStateData] = MoveWidget =
      XtCreateManagedWidget("Move", labelWidgetClass,
		     form, args, j);

    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNjustify,   (XtArgVal) XtJustifyRight); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) _("NPS")); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) MoveWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNleft,      XtChainRight); j++;
    XtSetArg(args[j], XtNright,     XtChainRight); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) 100); j++;
    outputField[which][nLabelNPS] = NodesWidget =
      XtCreateManagedWidget("Nodes", labelWidgetClass,
		     form, args, j);

    // create "text" within "form"
    j = 0;
    if (mutable) {
	XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    }
    XtSetArg(args[j], XtNstring, "");  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth, bw_width);  j++; /*force wider than buttons*/
#if 0
    XtSetArg(args[j], XtNscrollVertical, XawtextScrollWhenNeeded);  j++;
#else
    /* !!Work around an apparent bug in XFree86 4.0.1 (X11R6.4.3) */
    XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++;
    XtSetArg(args[j], XtNscrollHorizontal, XawtextScrollWhenNeeded);  j++;
#endif
//    XtSetArg(args[j], XtNautoFill, True);  j++;
//    XtSetArg(args[j], XtNwrap, XawtextWrapWord); j++;
    outputField[which][nMemo] = edit =
      XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j);

    j = 0;
    XtSetArg(args[j], XtNfromVert, ColorWidget); j++;
//    XtSetArg(args[j], XtNresizable, (XtArgVal) True); j++;
    XtSetValues(edit, args, j);
}

Widget EngineOutputCreate(name, text)
     char *name, *text;
{
    Arg args[16];
    Widget shell, layout, form, form2, edit;
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
      XtCreatePopupShell(name, transientShellWidgetClass,
			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    // divide window vertically into two equal parts, by creating two forms
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    form2 =
      XtCreateManagedWidget("form2", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    j = 0;
    XtSetArg(args[j], XtNfromVert,  (XtArgVal) form); j++;
    XtSetValues(form2, args, j);
    // make sure width is known in advance, for better placement of child widgets
    j = 0;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) bw_width-16); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) bw_height/2); j++;
    XtSetValues(shell, args, j);

    // fill up both forms with control elements
    PositionControlSet(0, form,  bw_width);
    PositionControlSet(1, form2, bw_width);

    XtRealizeWidget(shell);

    if (engineOutputX == -1) {
	int xx, yy;
	Window junk;
	Dimension pw_height;
	Dimension ew_height;
#if 0
	j = 0;
	XtSetArg(args[j], XtNheight, &ew_height);  j++;
	XtGetValues(edit, args, j);

	j = 0;
	XtSetArg(args[j], XtNheight, &pw_height);  j++;
	XtGetValues(shell, args, j);
	engineOutputH = pw_height + (lines - 1) * ew_height;
	engineOutputW = bw_width - 16;
#else
	engineOutputH = bw_height/2;
	engineOutputW = bw_width-16;
#endif

	XSync(xDisplay, False);
#ifdef NOTDEF
	/* This code seems to tickle an X bug if it is executed too soon
	   after xboard starts up.  The coordinates get transformed as if
	   the main window was positioned at (0, 0).
	   */
	XtTranslateCoords(shellWidget,
			  (bw_width - engineOutputW) / 2, 0 - engineOutputH / 2,
			  &engineOutputX, &engineOutputY);
#else  /*!NOTDEF*/
        XTranslateCoordinates(xDisplay, XtWindow(shellWidget),
			      RootWindowOfScreen(XtScreen(shellWidget)),
			      (bw_width - engineOutputW) / 2, 0 - engineOutputH / 2,
			      &xx, &yy, &junk);
	engineOutputX = xx;
	engineOutputY = yy;
#endif /*!NOTDEF*/
	if (engineOutputY < 0) engineOutputY = 0; /*avoid positioning top offscreen*/
    }
    j = 0;
    XtSetArg(args[j], XtNheight, engineOutputH);  j++;
    XtSetArg(args[j], XtNwidth, engineOutputW);  j++;
    XtSetArg(args[j], XtNx, engineOutputX);  j++;
    XtSetArg(args[j], XtNy, engineOutputY);  j++;
    XtSetValues(shell, args, j);
//    XtSetKeyboardFocus(shell, edit);

    return shell;
}

void ResizeWindowControls(shell, mode)
	Widget shell;
	int mode;
{
    Widget form1, form2;
    Arg args[16];
    int j;
    Dimension ew_height, tmp;

    form1 = XtNameToWidget(shell, "*form");
    form2 = XtNameToWidget(shell, "*form2");

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

void EngineOutputPopUp(title, text)
     char *title, *text;
{
    Arg args[16];
    int j;
    Widget edit;

    if (engineOutputShell == NULL) {
	engineOutputShell =
	  EngineOutputCreate(title, text);
	XtRealizeWidget(engineOutputShell);
	CatchDeleteWindow(engineOutputShell, "EngineOutputPopDown");
	if( needInit ) {
	    InitializeEngineOutput();
	    needInit = FALSE;
	}
        SetEngineColorIcon( 0 );
        SetEngineColorIcon( 1 );
        SetEngineState( 0, STATE_IDLE, "" );
        SetEngineState( 1, STATE_IDLE, "" );
    } else {
	edit = XtNameToWidget(engineOutputShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, text); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(engineOutputShell, args, j);
    }

    XtPopup(engineOutputShell, XtGrabNone);
    XSync(xDisplay, False);

    j=0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Show Engine Output"),
		args, j);

    engineOutputDialogUp = True;
    ShowThinkingEvent(); // [HGM] thinking: might need to prompt engine for thinking output
}

void EngineOutputPopDown()
{
    Arg args[16];
    int j;

    if (!engineOutputDialogUp) return;
    DoClearMemo(1);
    j = 0;
    XtSetArg(args[j], XtNx, &engineOutputX); j++;
    XtSetArg(args[j], XtNy, &engineOutputY); j++;
    XtSetArg(args[j], XtNwidth, &engineOutputW); j++;
    XtSetArg(args[j], XtNheight, &engineOutputH); j++;
    XtGetValues(engineOutputShell, args, j);
    XtPopdown(engineOutputShell);
    XSync(xDisplay, False);
    j=0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Show Engine Output"),
		args, j);

    engineOutputDialogUp = False;
    ShowThinkingEvent(); // [HGM] thinking: might need to shut off thinking output
}

//------------------------ pure back-end routines -------------------------------


// back end, due to front-end wrapper for SetWindowText, and new SetIcon arguments
static void SetEngineState( int which, int state, char * state_data )
{
    int x_which = 1 - which;

    if( engineState[ which ] != state ) {
        engineState[ which ] = state;

        switch( state ) {
        case STATE_THINKING:
            SetIcon( which, nStateIcon, nThinking );
            if( engineState[ x_which ] == STATE_THINKING ) {
                SetEngineState( x_which, STATE_IDLE, "" );
            }
            break;
        case STATE_PONDERING:
            SetIcon( which, nStateIcon, nPondering );
            break;
        case STATE_ANALYZING:
            SetIcon( which, nStateIcon, nAnalyzing );
            break;
        default:
            SetIcon( which, nStateIcon, nClear );
            break;
        }
    }

    if( state_data != 0 ) {
        DoSetWindowText( which, nStateData, state_data );
    }
}

// back end, now the front-end wrapper ClearMemo is used, and ed no longer contains handles.
void EngineOutputUpdate( FrontEndProgramStats * stats )
{
    EngineOutputData ed;
    int clearMemo = FALSE;
    int which;
    int depth;

    if( stats == 0 ) {
        SetEngineState( 0, STATE_IDLE, "" );
        SetEngineState( 1, STATE_IDLE, "" );
        return;
    }

    if(gameMode == IcsObserving && !appData.icsEngineAnalyze)
	return; // [HGM] kibitz: shut up engine if we are observing an ICS game

    which = stats->which;
    depth = stats->depth;

    if( which < 0 || which > 1 || depth < 0 || stats->time < 0 || stats->pv == 0 ) {
        return;
    }

    if( engineOutputShell == NULL ) {
        return;
    }

    VerifyDisplayMode();

    ed.which = which;
    ed.depth = depth;
    ed.nodes = stats->nodes;
    ed.score = stats->score;
    ed.time = stats->time;
    ed.pv = stats->pv;
    ed.hint = stats->hint;
    ed.an_move_index = stats->an_move_index;
    ed.an_move_count = stats->an_move_count;

    /* Get target control. [HGM] this is moved to front end, which get them from a table */
    if( which == 0 ) {
        ed.name = first.tidy;
    }
    else {
        ed.name = second.tidy;
    }

    /* Clear memo if needed */
    if( lastDepth[which] > depth || (lastDepth[which] == depth && depth <= 1) ) {
        clearMemo = TRUE;
    }

    if( lastForwardMostMove[which] != forwardMostMove ) {
        clearMemo = TRUE;
    }

    if( clearMemo ) DoClearMemo(which);

    /* Update */
    lastDepth[which] = depth == 1 && ed.nodes == 0 ? 0 : depth; // [HGM] info-line kudge
    lastForwardMostMove[which] = forwardMostMove;

    if( ed.pv != 0 && ed.pv[0] == ' ' ) {
        if( strncmp( ed.pv, " no PV", 6 ) == 0 ) { /* Hack on hack! :-O */
            ed.pv = "";
        }
    }

    UpdateControls( &ed );
}

#define ENGINE_COLOR_WHITE      'w'
#define ENGINE_COLOR_BLACK      'b'
#define ENGINE_COLOR_UNKNOWN    ' '

// pure back end
char GetEngineColor( int which )
{
    char result = ENGINE_COLOR_UNKNOWN;

    if( which == 0 || which == 1 ) {
        ChessProgramState * cps;

        switch (gameMode) {
        case MachinePlaysBlack:
        case IcsPlayingBlack:
            result = ENGINE_COLOR_BLACK;
            break;
        case MachinePlaysWhite:
        case IcsPlayingWhite:
            result = ENGINE_COLOR_WHITE;
            break;
        case AnalyzeMode:
        case AnalyzeFile:
            result = WhiteOnMove(forwardMostMove) ? ENGINE_COLOR_WHITE : ENGINE_COLOR_BLACK;
            break;
        case TwoMachinesPlay:
            cps = (which == 0) ? &first : &second;
            result = cps->twoMachinesColor[0];
            result = result == 'w' ? ENGINE_COLOR_WHITE : ENGINE_COLOR_BLACK;
            break;
        }
    }

    return result;
}

// pure back end
char GetActiveEngineColor()
{
    char result = ENGINE_COLOR_UNKNOWN;

    if( gameMode == TwoMachinesPlay ) {
        result = WhiteOnMove(forwardMostMove) ? ENGINE_COLOR_WHITE : ENGINE_COLOR_BLACK;
    }

    return result;
}

// pure back end
static int IsEnginePondering( int which )
{
    int result = FALSE;

    switch (gameMode) {
    case MachinePlaysBlack:
    case IcsPlayingBlack:
        if( WhiteOnMove(forwardMostMove) ) result = TRUE;
        break;
    case MachinePlaysWhite:
    case IcsPlayingWhite:
        if( ! WhiteOnMove(forwardMostMove) ) result = TRUE;
        break;
    case TwoMachinesPlay:
        if( GetActiveEngineColor() != ENGINE_COLOR_UNKNOWN ) {
            if( GetEngineColor( which ) != GetActiveEngineColor() ) result = TRUE;
        }
        break;
    }

    return result;
}

// back end
static void SetDisplayMode( int mode )
{
    if( windowMode != mode ) {
        windowMode = mode;

        ResizeWindowControls( engineOutputShell, mode );
    }
}

// pure back end
void VerifyDisplayMode()
{
    int mode;

    /* Get proper mode for current game */
    switch( gameMode ) {
    case IcsObserving:    // [HGM] ICS analyze
	if(!appData.icsEngineAnalyze) return;
    case AnalyzeMode:
    case AnalyzeFile:
    case MachinePlaysWhite:
    case MachinePlaysBlack:
        mode = 0;
        break;
    case IcsPlayingWhite:
    case IcsPlayingBlack:
        mode = appData.zippyPlay && opponentKibitzes; // [HGM] kibitz
        break;
    case TwoMachinesPlay:
        mode = 1;
        break;
    default:
        /* Do not change */
        return;
    }

    SetDisplayMode( mode );
}

// back end. Determine what icon to se in the color-icon field, and print it
static void SetEngineColorIcon( int which )
{
    char color = GetEngineColor(which);
    int nicon = 0;

    if( color == ENGINE_COLOR_BLACK )
        nicon = nColorBlack;
    else if( color == ENGINE_COLOR_WHITE )
        nicon = nColorWhite;
    else
        nicon = nColorUnknown;

    SetIcon( which, nColorIcon, nicon );
}

#define MAX_NAME_LENGTH 32

// pure back end, now SetWindowText is called via wrapper DoSetWindowText
static void UpdateControls( EngineOutputData * ed )
{
    int isPondering = FALSE;

    char s_label[MAX_NAME_LENGTH + 32];
    
    char * name = ed->name;

    /* Label */
    if( name == 0 || *name == '\0' ) {
        name = "?";
    }

    strncpy( s_label, name, MAX_NAME_LENGTH );
    s_label[ MAX_NAME_LENGTH-1 ] = '\0';

#ifdef SHOW_PONDERING
    if( IsEnginePondering( ed->which ) ) {
        char buf[8];

        buf[0] = '\0';

        if( ed->hint != 0 && *ed->hint != '\0' ) {
            strncpy( buf, ed->hint, sizeof(buf) );
            buf[sizeof(buf)-1] = '\0';
        }
        else if( ed->pv != 0 && *ed->pv != '\0' ) {
            char * sep = strchr( ed->pv, ' ' );
            int buflen = sizeof(buf);

            if( sep != NULL ) {
                buflen = sep - ed->pv + 1;
                if( buflen > sizeof(buf) ) buflen = sizeof(buf);
            }

            strncpy( buf, ed->pv, buflen );
            buf[ buflen-1 ] = '\0';
        }

        SetEngineState( ed->which, STATE_PONDERING, buf );
    }
    else if( gameMode == TwoMachinesPlay ) {
        SetEngineState( ed->which, STATE_THINKING, "" );
    }
    else if( gameMode == AnalyzeMode || gameMode == AnalyzeFile 
	  || gameMode == IcsObserving && appData.icsEngineAnalyze) { // [HGM] ICS-analyze
        char buf[64];
        int time_secs = ed->time / 100;
        int time_mins = time_secs / 60;

        buf[0] = '\0';

        if( ed->an_move_index != 0 && ed->an_move_count != 0 && *ed->hint != '\0' ) {
            char mov[16];

            strncpy( mov, ed->hint, sizeof(mov) );
            mov[ sizeof(mov)-1 ] = '\0';

            sprintf( buf, "%d/%d: %s [%02d:%02d:%02d]", ed->an_move_index, ed->an_move_count, mov, time_mins / 60, time_mins % 60, time_secs % 60 );
        }

        SetEngineState( ed->which, STATE_ANALYZING, buf );
    }
    else {
        SetEngineState( ed->which, STATE_IDLE, "" );
    }
#endif

    DoSetWindowText( ed->which, nLabel, s_label );

    s_label[0] = '\0';

    if( ed->time > 0 && ed->nodes > 0 ) {
        unsigned long nps_100 = ed->nodes / ed->time;

        if( nps_100 < 100000 ) {
            sprintf( s_label, _("NPS: %lu"), nps_100 * 100 );
        }
        else {
            sprintf( s_label, _("NPS: %.1fk"), nps_100 / 10.0 );
        }
    }

    DoSetWindowText( ed->which, nLabelNPS, s_label );

    /* Memo */
    if( ed->pv != 0 && *ed->pv != '\0' ) {
        char s_nodes[24];
        char s_score[16];
        char s_time[24];
        char buf[256];
        int buflen;
        int time_secs = ed->time / 100;
        int time_cent = ed->time % 100;

        /* Nodes */
        if( ed->nodes < 1000000 ) {
            sprintf( s_nodes, u64Display, ed->nodes );
        }
        else {
            sprintf( s_nodes, "%.1fM", u64ToDouble(ed->nodes) / 1000000.0 );
        }

        /* Score */
        if( ed->score > 0 ) {
            sprintf( s_score, "+%.2f", ed->score / 100.0 );
        } else
            sprintf( s_score, "%.2f", ed->score / 100.0 );

        /* Time */
        sprintf( s_time, "%d:%02d.%02d", time_secs / 60, time_secs % 60, time_cent );

        /* Put all together... */
	if(ed->nodes == 0 && ed->score == 0 && ed->time == 0) sprintf( buf, "%3d\t", ed->depth ); else 
        sprintf( buf, "%3d  %s  %s\t%s\t", ed->depth, s_score, s_nodes, s_time );

        /* Add PV */
        buflen = strlen(buf);

        strncpy( buf + buflen, ed->pv, sizeof(buf) - buflen );

        buf[ sizeof(buf) - 3 ] = '\0';

        strcat( buf + buflen, "\n" );

        /* Update memo */
        InsertIntoMemo( ed->which, buf );
    }

    /* Colors */
    SetEngineColorIcon( ed->which );
}

// back end
int EngineOutputIsUp()
{
    return engineOutputDialogUp;
}

void
EngineOutputProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
  if (engineOutputDialogUp) {
    EngineOutputPopDown();
  } else {
    EngineOutputPopUp(_("engine output"),_("This feature is experimental"));
  }
//  ToNrEvent(currentMove);
}

// [HGM] kibitz: write kibitz line; split window for it if necessary
void OutputKibitz(int window, char *text)
{
	if(!EngineOutputIsUp()) return;
	if(!opponentKibitzes) { // on first kibitz of game, clear memos
	    DoClearMemo(1);
	    if(gameMode == IcsObserving) DoClearMemo(0);
	}
	opponentKibitzes = TRUE; // this causes split window DisplayMode in ICS modes.
	VerifyDisplayMode();
	if(gameMode == IcsObserving) {
	    DoSetWindowText(0, nLabel, gameInfo.white);
	    SetIcon( 0, nColorIcon,  nColorWhite);
	    SetIcon( 0, nStateIcon,  nClear);
	}
	DoSetWindowText(1, nLabel, gameMode == IcsPlayingBlack ? gameInfo.white : gameInfo.black); // opponent name
	SetIcon( 1, nColorIcon,  gameMode == IcsPlayingBlack ? nColorWhite : nColorBlack);
	SetIcon( 1, nStateIcon,  nClear);
	InsertIntoMemo(window-1, text);
}
