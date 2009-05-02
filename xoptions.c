/*
 * xoptions.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000 Free Software Foundation, Inc.
 *
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
 * ------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ------------------------------------------------------------------------
 *
 * See the file ChangeLog for a revision history.
 */

// [HGM] this file is the counterpart of woptions.c, containing xboard popup menus
// similar to those of WinBoard, to set the most common options interactively.

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
#include <X11/Xaw/Toggle.h>

#include "common.h"
#include "backend.h"
#include "xboard.h"

extern Widget formWidget, shellWidget, boardWidget, menuBarWidget;
extern Display *xDisplay;
extern int squareSize;
extern Pixmap xMarkPixmap;
extern char *layoutName;
extern Window xBoardWindow;
extern Arg layoutArgs[2], formArgs[2];
Pixel timerForegroundPixel, timerBackgroundPixel;

// [HGM] the following code for makng menu popups was cloned from the FileNamePopUp routines

static Widget previous = NULL;

void SetFocus(Widget w, XtPointer data, XEvent *event, Boolean *b)
{
    Arg args;

    if(previous) {
	XtSetArg(args, XtNdisplayCaret, False);
	XtSetValues(previous, &args, 1);
    }
    XtSetArg(args, XtNdisplayCaret, True);
    XtSetValues(w, &args, 1);
    XtSetKeyboardFocus((Widget) data, w);
    previous = w;
}

//--------------------------- New Shuffle Game --------------------------------------------
extern int shuffleOpenings;
extern int startedFromPositionFile;
int shuffleUp;
Widget shuffleShell;

void ShufflePopDown()
{
    if (!shuffleUp) return;
    XtPopdown(shuffleShell);
    XtDestroyWidget(shuffleShell);
    shuffleUp = False;
    ModeHighlight();
}

void ShuffleCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Widget w2;
    Arg args[16];
    char buf[80];
    
    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    if (strcmp(name, "cancel") == 0) {
        ShufflePopDown();
        return;
    }
    if (strcmp(name, "off") == 0) {
        ShufflePopDown();
	shuffleOpenings = False; // [HGM] should be moved to New Variant menu, once we have it!
	ResetGameEvent();
	AnalysisPopDown();
        return;
    }
    if (strcmp(name, "random") == 0) {
	sprintf(buf, "%d", rand());
	XtSetArg(args[0],XtNvalue, buf); // erase bad (non-numeric) value
	XtSetValues(XtParent(w), args, 1);
        return;
    }
    if (strcmp(name, "ok") == 0) {
	int nr; String name;
        name = XawDialogGetValueString(w2 = XtParent(w));
	if(sscanf(name ,"%d",&nr) != 1) {
	    sprintf(buf, "%d", appData.defaultFrcPosition);
	    XtSetArg(args[0],XtNvalue, buf); // erase bad (non-numeric) value
	    XtSetValues(w2, args, 1);
	    return;
	}
	appData.defaultFrcPosition = nr;
	shuffleOpenings = True;
        ShufflePopDown();
	ResetGameEvent();
	AnalysisPopDown();
        return;
    }
}

void ShufflePopUp()
{
    Arg args[16];
    Widget popup, layout, dialog, edit;
    Window root, child;
    int x, y, i;
    int win_x, win_y;
    unsigned int mask;
    char def[80];
    
    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    shuffleShell = popup =
      XtCreatePopupShell("New Shuffle Game", transientShellWidgetClass,
			 shellWidget, args, i);
    
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  
    sprintf(def, "%d\n", appData.defaultFrcPosition);
    i = 0;
    XtSetArg(args[i], XtNlabel, "Start-position number:"); i++;
    XtSetArg(args[i], XtNvalue, def); i++;
    XtSetArg(args[i], XtNborderWidth, 0); i++;
    dialog = XtCreateManagedWidget("Shuffle", dialogWidgetClass,
				   layout, args, i);
    
//    XtSetArg(args[0], XtNeditType, XawtextEdit);  // [HGM] can't get edit to work decently
//    XtSetArg(args[1], XtNuseStringInPlace, False);
//    XtSetValues(dialog, args, 2);

    XawDialogAddButton(dialog, "ok", ShuffleCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, "cancel", ShuffleCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, "random", ShuffleCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, "off", ShuffleCallback, (XtPointer) dialog);
    
    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "ShufflePopDown");
    
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);
    
    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);
    
    XtPopup(popup, XtGrabExclusive);
    shuffleUp = True;
    
    edit = XtNameToWidget(dialog, "*value");

    XtSetKeyboardFocus(popup, edit);
}

void ShuffleMenuProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
//    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
//	Reset(FALSE, TRUE);
//    }
    ShufflePopUp();
}

//--------------------------- Time-Control Menu Popup ----------------------------------
int TimeControlUp;
Widget TimeControlShell;
int tcInc;
Widget tcMess1, tcMess2, tcData, tcTime, tcOdds1, tcOdds2;
int tcIncrement, tcMoves;

void TimeControlPopDown()
{
    if (!TimeControlUp) return;
    XtPopdown(TimeControlShell);
    XtDestroyWidget(TimeControlShell);
    TimeControlUp = False;
    ModeHighlight();
}

void TimeControlCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name, txt;
    Widget w2;
    Arg args[16];
    char buf[80];
    int j;

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    if (strcmp(name, "classical") == 0) {
	if(!tcInc) return;
	j=0;
	XtSetArg(args[j], XtNlabel, "minutes for each"); j++;
	XtSetValues(tcMess1, args, j);
	j=0;
	XtSetArg(args[j], XtNlabel, "moves"); j++;
	XtSetValues(tcMess2, args, j);
	j=0;
	XtSetArg(args[j], XtNstring, &name); j++;
	XtGetValues(tcData, args, j);
	tcIncrement = 0; sscanf(name, "%d", &tcIncrement);
	sprintf(buf, "%d", tcMoves);
	j=0;
	XtSetArg(args[j], XtNstring, buf); j++;
	XtSetValues(tcData, args, j);
	tcInc = False;
        return;
    }
    if (strcmp(name, "incremental") == 0) {
	if(tcInc) return;
	j=0;
	XtSetArg(args[j], XtNlabel, "minutes, plus"); j++;
	XtSetValues(tcMess1, args, j);
	j=0;
	XtSetArg(args[j], XtNlabel, "sec/move"); j++;
	XtSetValues(tcMess2, args, j);
	j=0;
	XtSetArg(args[j], XtNstring, &name); j++;
	XtGetValues(tcData, args, j);
	tcMoves = appData.movesPerSession; sscanf(name, "%d", &tcMoves);
	sprintf(buf, "%d", tcIncrement);
	j=0;
	XtSetArg(args[j], XtNstring, buf); j++;
	XtSetValues(tcData, args, j);
	tcInc = True;
        return;
    }
    if (strcmp(name, " OK ") == 0) {
	int inc, mps, tc, ok;
	XtSetArg(args[0], XtNstring, &txt);
	XtGetValues(tcData, args, 1);
	if(tcInc) {
	    ok = sscanf(txt, "%d", &inc); mps = 0;
	    if(!ok && txt[0] == 0) { inc = 0; ok = 1; } // accept empty string as zero
	    ok &= (inc >= 0);
	} else {
	    ok = sscanf(txt, "%d", &mps); inc = -1;
	    ok &= (mps > 0);
	}
	if(ok != 1) {
	    XtSetArg(args[0], XtNstring, ""); // erase any offending input
	    XtSetValues(tcData, args, 1);
	    return;
	}
	XtSetArg(args[0], XtNstring, &txt);
	XtGetValues(tcTime, args, 1);
	if(!ParseTimeControl(txt, inc, mps)) {
	    XtSetArg(args[0], XtNstring, ""); // erase any offending input
	    XtSetValues(tcTime, args, 1);
	    DisplayError("Bad Time-Control String", 0);
	    return;
	}
	appData.movesPerSession = mps;
	appData.timeIncrement = inc;
	appData.timeControl = strdup(txt);
	XtSetArg(args[0], XtNstring, &txt);
	XtGetValues(tcOdds1, args, 1);
	appData.firstTimeOdds = first.timeOdds 
		= (sscanf(txt, "%d", &j) == 1 && j > 0) ? j : 1;
	XtGetValues(tcOdds2, args, 1);
	appData.secondTimeOdds = second.timeOdds 
		= (sscanf(txt, "%d", &j) == 1 && j > 0) ? j : 1;

	Reset(True, True);
        TimeControlPopDown();
        return;
    }
}

void TimeControlPopUp()
{
    Arg args[16];
    Widget popup, layout, form, edit, b_ok, b_cancel, b_clas, b_inc, mess;
    Window root, child;
    int x, y, i, j;
    int win_x, win_y;
    unsigned int mask;
    char def[80];
    
    tcInc = (appData.timeIncrement >= 0);
    tcMoves = appData.movesPerSession; tcIncrement = appData.timeIncrement;
    if(!tcInc) tcIncrement = 0;
    sprintf(def, "%d", tcInc ? tcIncrement : tcMoves);

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
//    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    TimeControlShell = popup =
      XtCreatePopupShell("TimeControl Menu", transientShellWidgetClass,
			 shellWidget, args, i);
    
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  
    form =
      XtCreateManagedWidget(layoutName, formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
  
    j = 0;
//    XtSetArg(args[j], XtNwidth,     (XtArgVal) 300); j++;
//    XtSetArg(args[j], XtNheight,    (XtArgVal) 85); j++;
    XtSetValues(popup, args, j);

    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, appData.timeControl);  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  85);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcTime = XtCreateManagedWidget("TC", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(tcTime, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, tcInc ? "   minutes, plus   " : "minutes for each"); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNfromHoriz, tcTime); j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
  //  XtSetArg(args[j], XtNwidth,  100);  j++;
  //  XtSetArg(args[j], XtNheight, 20);  j++;
    tcMess1 = XtCreateManagedWidget("TCtext", labelWidgetClass, form, args, j);

    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNfromHoriz, tcMess1); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, def);  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  40);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcData = XtCreateManagedWidget("MPS", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(tcData, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, tcInc ? "sec/move" : "moves     "); j++;
    XtSetArg(args[j], XtNjustify, XtJustifyLeft); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNfromHoriz, tcData); j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
//    XtSetArg(args[j], XtNwidth,  80);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcMess2 = XtCreateManagedWidget("MPStext", labelWidgetClass,
				   form, args, j);

    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNfromVert, tcTime); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, "1");  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  40);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcOdds1 = XtCreateManagedWidget("Odds1", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(tcOdds1, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNfromVert, tcTime); j++;
    XtSetArg(args[j], XtNfromHoriz, tcOdds1); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, "1");  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  40);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcOdds2 = XtCreateManagedWidget("Odds2", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(tcOdds2, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, "Engine #1 and #2 Time-Odds Factors"); j++;
    XtSetArg(args[j], XtNjustify, XtJustifyLeft); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNfromVert, tcTime); j++;
    XtSetArg(args[j], XtNfromHoriz, tcOdds2); j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
//    XtSetArg(args[j], XtNwidth,  200);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    mess = XtCreateManagedWidget("Oddstext", labelWidgetClass,
				   form, args, j);
    j=0;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    b_clas= XtCreateManagedWidget("classical", commandWidgetClass,
				   form, args, j);   
    XtAddCallback(b_clas, XtNcallback, TimeControlCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_clas);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    b_inc = XtCreateManagedWidget("incremental", commandWidgetClass,
				   form, args, j);   
    XtAddCallback(b_inc, XtNcallback, TimeControlCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNfromHoriz, tcData);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_ok= XtCreateManagedWidget(" OK ", commandWidgetClass,
				   form, args, j);   
    XtAddCallback(b_ok, XtNcallback, TimeControlCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_ok);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_cancel= XtCreateManagedWidget("cancel", commandWidgetClass,
				   form, args, j);   
    XtAddCallback(b_cancel, XtNcallback, TimeControlPopDown, (XtPointer) 0);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "TimeControlPopDown");
    
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);
    
    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);
    
    XtPopup(popup, XtGrabExclusive);
    TimeControlUp = True;
    
    previous = NULL;
    SetFocus(tcTime, popup, (XEvent*) NULL, False);
//    XtSetKeyboardFocus(popup, tcTime);
}

void TimeControlProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   TimeControlPopUp();
}

//--------------------------- Engine-Options Menu Popup ----------------------------------
int EngineUp;
Widget EngineShell;
extern int adjudicateLossThreshold;

Widget engDrawMoves, engThreshold, engRule, engRepeat;

void EnginePopDown()
{
    if (!EngineUp) return;
    XtPopdown(EngineShell);
    XtDestroyWidget(EngineShell);
    EngineUp = False;
    ModeHighlight();
}

int ReadToggle(Widget w)
{
    Arg args; Boolean res;

    XtSetArg(args, XtNstate, &res);
    XtGetValues(w, &args, 1);

    return res;
}

Widget w1, w2, w3, w4, w5, w6, w7, w8;

void EngineCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Widget s2;
    Arg args[16];
    char buf[80];
    int j;
    
    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    if (strcmp(name, "OK") == 0) {
	// read all switches
	appData.periodicUpdates = ReadToggle(w1);
//	appData.hideThinkingFromHuman = ReadToggle(w2);
	appData.firstScoreIsAbsolute  = ReadToggle(w3);
	appData.secondScoreIsAbsolute = ReadToggle(w4);
	appData.testClaims    = ReadToggle(w5);
	appData.checkMates    = ReadToggle(w6);
	appData.materialDraws = ReadToggle(w7);
	appData.trivialDraws  = ReadToggle(w8);

	// adjust setting in other menu for duplicates 
	// (perhaps duplicates should be removed from general Option Menu?)
//	XtSetArg(args[0], XtNleftBitmap, appData.showThinking ? xMarkPixmap : None);
//	XtSetValues(XtNameToWidget(menuBarWidget,
//				   "menuOptions.Show Thinking"), args, 1);

	// read out numeric controls, simply ignore bad formats for now
	XtSetArg(args[0], XtNstring, &name);
	XtGetValues(engDrawMoves, args, 1);
	if(sscanf(name, "%d", &j) == 1) appData.adjudicateDrawMoves = j;
	XtGetValues(engThreshold, args, 1);
	if(sscanf(name, "%d", &j) == 1) 
		adjudicateLossThreshold = appData.adjudicateLossThreshold = -j; // inverted!
	XtGetValues(engRule, args, 1);
	if(sscanf(name, "%d", &j) == 1) appData.ruleMoves = j;
	XtGetValues(engRepeat, args, 1);
	if(sscanf(name, "%d", &j) == 1) appData.drawRepeats = j;

        EnginePopDown();
	ShowThinkingEvent(); // [HGM] thinking: score adjudication might need thinking output
        return;
    }
}

void EnginePopUp()
{
    Arg args[16];
    Widget popup, layout, form, edit, b_ok, b_cancel, b_clas, b_inc, s1; 
    Window root, child;
    int x, y, i, j, width;
    int win_x, win_y;
    unsigned int mask;
    char def[80];
    
    tcInc = (appData.timeIncrement >= 0);
    tcMoves = appData.movesPerSession; tcIncrement = appData.timeIncrement;
    if(!tcInc) tcIncrement = 0;
    sprintf(def, "%d", tcInc ? tcIncrement : tcMoves);

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
//    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    EngineShell = popup =
      XtCreatePopupShell("Adjudications", transientShellWidgetClass,
			 shellWidget, args, i);
    
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  
    form =
      XtCreateManagedWidget(layoutName, formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
  
    j = 0;
//    XtSetArg(args[j], XtNwidth,     (XtArgVal) 250); j++;
//    XtSetArg(args[j], XtNheight,    (XtArgVal) 400); j++;
//    XtSetValues(popup, args, j);

    j = 0;
//    XtSetArg(args[j], XtNwidth,       (XtArgVal) 250); j++;
//    XtSetArg(args[j], XtNheight,      (XtArgVal) 20); j++;
    XtSetArg(args[j], XtNleft,        (XtArgVal) XtChainLeft); j++;
    XtSetArg(args[j], XtNright,       (XtArgVal) XtChainRight); j++;
    XtSetArg(args[j], XtNstate,       appData.periodicUpdates); j++;
//    XtSetArg(args[j], XtNjustify,     (XtArgVal) XtJustifyLeft); j++;
    w1 = XtCreateManagedWidget("Periodic Updates (Analysis Mode)", toggleWidgetClass, form, args, j);

    XtSetArg(args[j], XtNwidth,       (XtArgVal) &width);
    XtGetValues(w1, &args[j], 1);

//    XtSetArg(args[j-1], XtNfromVert,  (XtArgVal) w1);
//    XtSetArg(args[j-3], XtNstate,       appData.hideThinkingFromHuman);
//    w2 = XtCreateManagedWidget("Hide Thinking from Human", toggleWidgetClass, form, args, j);

    XtSetArg(args[j], XtNwidth,       (XtArgVal) width); j++;
    XtSetArg(args[j-2], XtNstate,     appData.firstScoreIsAbsolute);
    XtSetArg(args[j], XtNfromVert,    (XtArgVal) w1); j++;
    w3 = XtCreateManagedWidget("Engine #1 Score is Absolute", toggleWidgetClass, form, args, j);

    XtSetArg(args[j-1], XtNfromVert,  (XtArgVal) w3);
    XtSetArg(args[j-3], XtNstate,       appData.secondScoreIsAbsolute);
    w4 = XtCreateManagedWidget("Engine #2 Score is Absolute", toggleWidgetClass, form, args, j);

    s1 = XtCreateManagedWidget("\nEngine-Engine Adjudications:", labelWidgetClass, form, args, 3);

    XtSetArg(args[j-1], XtNfromVert,  (XtArgVal) s1);
    XtSetArg(args[j-3], XtNstate,       appData.testClaims);
    w5 = XtCreateManagedWidget("Verify Engine Result Claims", toggleWidgetClass, form, args, j);

    XtSetArg(args[j-1], XtNfromVert,  (XtArgVal) w5);
    XtSetArg(args[j-3], XtNstate,       appData.checkMates);
    w6 = XtCreateManagedWidget("Detect All Mates", toggleWidgetClass, form, args, j);

    XtSetArg(args[j-1], XtNfromVert,  (XtArgVal) w6);
    XtSetArg(args[j-3], XtNstate,       appData.materialDraws);
    w7 = XtCreateManagedWidget("Draw when Insuff. Mating Material", toggleWidgetClass, form, args, j);

    XtSetArg(args[j-1], XtNfromVert,  (XtArgVal) w7);
    XtSetArg(args[j-3], XtNstate,       appData.trivialDraws);
    w8 = XtCreateManagedWidget("Adjudicate Trivial Draws", toggleWidgetClass, form, args, j);

    XtSetArg(args[0], XtNfromVert,  (XtArgVal) w4);
    XtSetArg(args[1], XtNborderWidth, (XtArgVal) 0);
    XtSetValues(s1, args, 2);

    sprintf(def, "%d", appData.adjudicateDrawMoves);
    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNfromVert, w8); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, def);  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  60);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    engDrawMoves = XtCreateManagedWidget("Length", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(engDrawMoves, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, " moves maximum, then draw"); j++;
    XtSetArg(args[j], XtNjustify,     (XtArgVal) XtJustifyLeft); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNfromVert, w8); j++;
    XtSetArg(args[j], XtNfromHoriz, engDrawMoves); j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNwidth,  170);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcMess1 = XtCreateManagedWidget("TCtext", labelWidgetClass, form, args, j);

    sprintf(def, "%d", -appData.adjudicateLossThreshold); // inverted!
    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNfromVert, engDrawMoves); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, def);  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  60);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    engThreshold = XtCreateManagedWidget("Threshold", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(engThreshold, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, "-centiPawn lead is win"); j++;
    XtSetArg(args[j], XtNjustify, XtJustifyLeft); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNfromVert, engDrawMoves); j++;
    XtSetArg(args[j], XtNfromHoriz, engThreshold); j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNwidth,  150);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcMess2 = XtCreateManagedWidget("MPStext", labelWidgetClass, form, args, j);

    sprintf(def, "%d", appData.ruleMoves);
    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNfromVert, engThreshold); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, def);  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  30);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    engRule = XtCreateManagedWidget("Rule", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(engRule, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, "-move rule applied"); j++;
    XtSetArg(args[j], XtNjustify,     (XtArgVal) XtJustifyLeft); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNfromVert, engThreshold); j++;
    XtSetArg(args[j], XtNfromHoriz, engRule); j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNwidth,  130);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcMess1 = XtCreateManagedWidget("TCtext", labelWidgetClass, form, args, j);

    sprintf(def, "%d", appData.drawRepeats);
    j= 0;
    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNfromVert, engRule); j++;
    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    XtSetArg(args[j], XtNstring, def);  j++;
    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth,  30);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    engRepeat = XtCreateManagedWidget("Repeats", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(engRepeat, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, "-fold repeat is draw"); j++;
    XtSetArg(args[j], XtNjustify, XtJustifyLeft); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNfromVert, engRule); j++;
    XtSetArg(args[j], XtNfromHoriz, engRepeat); j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNwidth,  130);  j++;
//    XtSetArg(args[j], XtNheight, 20);  j++;
    tcMess2 = XtCreateManagedWidget("MPStext", labelWidgetClass, form, args, j);

    j=0;
    XtSetArg(args[j], XtNfromVert, engRepeat);  j++;
    XtSetArg(args[j], XtNfromHoriz, tcMess2);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_ok= XtCreateManagedWidget("OK", commandWidgetClass, form, args, j);   
    XtAddCallback(b_ok, XtNcallback, EngineCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNfromVert, engRepeat);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_ok);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_cancel= XtCreateManagedWidget("cancel", commandWidgetClass,
				   form, args, j);   
    XtAddCallback(b_cancel, XtNcallback, EnginePopDown, (XtPointer) 0);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "EnginePopDown");
    
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);
    
    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);
    
    XtPopup(popup, XtGrabExclusive);
    EngineUp = True;
    
    previous = NULL;
    SetFocus(engThreshold, popup, (XEvent*) NULL, False);
}

void EngineMenuProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   EnginePopUp();
}

//--------------------------- New-Variant Menu PopUp -----------------------------------
struct NewVarButton {
  char   *name;
  char *color;
  Widget handle;
  VariantClass variant;
};

struct NewVarButton buttonDesc[] = {
    {"normal",            "#FFFFFF", 0, VariantNormal},
    {"FRC",               "#FFFFFF", 0, VariantFischeRandom},
    {"wild castle",       "#FFFFFF", 0, VariantWildCastle},
    {"no castle",         "#FFFFFF", 0, VariantNoCastle},
    {"knightmate",        "#FFFFFF", 0, VariantKnightmate},
    {"berolina",          "#FFFFFF", 0, VariantBerolina},
    {"cylinder",          "#FFFFFF", 0, VariantCylinder},
    {"shatranj",          "#FFFFFF", 0, VariantShatranj},
    {"atomic",            "#FFFFFF", 0, VariantAtomic},
    {"two kings",         "#FFFFFF", 0, VariantTwoKings},
    {"3-checks",          "#FFFFFF", 0, Variant3Check},
    {"suicide",           "#FFFFBF", 0, VariantSuicide},
    {"give-away",         "#FFFFBF", 0, VariantGiveaway},
    {"losers",            "#FFFFBF", 0, VariantLosers},
    {"fairy",             "#BFBFBF", 0, VariantFairy},
    {"Superchess",        "#FFBFBF", 0, VariantSuper},
    {"crazyhouse",        "#FFBFBF", 0, VariantCrazyhouse},
    {"bughouse",          "#FFBFBF", 0, VariantBughouse},
    {"shogi (9x9)",       "#BFFFFF", 0, VariantShogi},
    {"xiangqi (9x10)",    "#BFFFFF", 0, VariantXiangqi},
    {"courier (12x8)",    "#BFFFBF", 0, VariantCourier},
    {"janus (10x8)",      "#BFBFFF", 0, VariantJanus},
    {"Capablanca (10x8)", "#BFBFFF", 0, VariantCapablanca},
    {"CRC (10x8)",        "#BFBFFF", 0, VariantCapaRandom},
#ifdef GOTHIC
    {"Gothic (10x8)",     "#BFBFFF", 0, VariantGothic},
#endif
#ifdef FALCON
    {"Falcon (10x8)",     "#BFBFFF", 0, VariantFalcon},
#endif
    {NULL,                0, 0, (VariantClass) 0}
};

int NewVariantUp;
Widget NewVariantShell;

void NewVariantPopDown()
{
    if (!NewVariantUp) return;
    XtPopdown(NewVariantShell);
    XtDestroyWidget(NewVariantShell);
    NewVariantUp = False;
    ModeHighlight();
}

void NewVariantCallback(w, client_data, call_data)
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
    
    if (strcmp(name, "  OK  ") == 0) {
	int nr = (int) XawToggleGetCurrent(buttonDesc[0].handle) - 1;
	if(nr < 0) return;
	v = buttonDesc[nr].variant;
	if(!appData.noChessProgram) { 
	    char *name = VariantName(v), buf[MSG_SIZ];
	    if (first.protocolVersion > 1 && StrStr(first.variants, name) == NULL) {
		/* [HGM] in protocol 2 we check if variant is suported by engine */
		sprintf(buf, "Variant %s not supported by %s", name, first.tidy);
		DisplayError(buf, 0);
//		NewVariantPopDown();
		return; /* ignore OK if first engine does not support it */
	    } else
	    if (second.initDone && second.protocolVersion > 1 && StrStr(second.variants, name) == NULL) {
		sprintf(buf, "Warning: second engine (%s) does not support this!", second.tidy);
		DisplayError(buf, 0);   /* use of second engine is optional; only warn user */
	    }
	}

	gameInfo.variant = v;
	appData.variant = VariantName(v);

	shuffleOpenings = FALSE; /* [HGM] shuffle: possible shuffle reset when we switch */
	startedFromPositionFile = FALSE; /* [HGM] loadPos: no longer valid in new variant */
	appData.pieceToCharTable = NULL;
	Reset(True, True);
        NewVariantPopDown();
        return;
    }
}

void NewVariantPopUp()
{
    Arg args[16];
    Widget popup, layout, dialog, edit, form, last = NULL, b_ok, b_cancel;
    Window root, child;
    int x, y, i, j;
    int win_x, win_y;
    unsigned int mask;
    char def[80];
    XrmValue vFrom, vTo;

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
//    XtSetArg(args[i], XtNwidth, 250); i++;
//    XtSetArg(args[i], XtNheight, 300); i++;
    NewVariantShell = popup =
      XtCreatePopupShell("NewVariant Menu", transientShellWidgetClass,
			 shellWidget, args, i);
    
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
  
    for(i = 0; buttonDesc[i].name != NULL; i++) {
	Pixel buttonColor;
	if (!appData.monoMode) {
	    vFrom.addr = (caddr_t) buttonDesc[i].color;
	    vFrom.size = strlen(buttonDesc[i].color);
	    XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	    if (vTo.addr == NULL) {
	  	buttonColor = (Pixel) -1;
	    } else {
		buttonColor = *(Pixel *) vTo.addr;
	    }
	}
    
	j = 0;
	XtSetArg(args[j], XtNradioGroup, last); j++;
	XtSetArg(args[j], XtNwidth, 125); j++;
//	XtSetArg(args[j], XtNheight, 16); j++;
	XtSetArg(args[j], XtNfromVert, i == 15 ? NULL : last); j++;
	XtSetArg(args[j], XtNfromHoriz, i < 15 ? NULL : buttonDesc[i-15].handle); j++;
	XtSetArg(args[j], XtNradioData, i+1); j++;
	XtSetArg(args[j], XtNbackground, buttonColor); j++;
	XtSetArg(args[j], XtNstate, gameInfo.variant == buttonDesc[i].variant); j++;
	buttonDesc[i].handle = last =
	    XtCreateManagedWidget(buttonDesc[i].name, toggleWidgetClass, form, args, j);
    }

    j=0;
    XtSetArg(args[j], XtNfromVert, buttonDesc[12].handle);  j++;
    XtSetArg(args[j], XtNfromHoriz, buttonDesc[12].handle);  j++;
    XtSetArg(args[j], XtNheight, 35); j++;
//    XtSetArg(args[j], XtNwidth, 60); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_cancel= XtCreateManagedWidget("CANCEL", commandWidgetClass, form, args, j);   
    XtAddCallback(b_cancel, XtNcallback, NewVariantPopDown, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNfromHoriz, b_cancel);  j++;
    XtSetArg(args[j], XtNfromVert, buttonDesc[12].handle);  j++;
    XtSetArg(args[j], XtNheight, 35); j++;
//    XtSetArg(args[j], XtNwidth, 60); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_ok= XtCreateManagedWidget("  OK  ", commandWidgetClass, form, args, j);   
    XtAddCallback(b_ok, XtNcallback, NewVariantCallback, (XtPointer) 0);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "NewVariantPopDown");
    
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);
    
    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);
    
    XtPopup(popup, XtGrabExclusive);
    NewVariantUp = True;
}

void NewVariantProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   NewVariantPopUp();
}

//--------------------------- UCI Menu Popup ------------------------------------------
int UciUp;
Widget UciShell;

struct UciControl {
  char *name;
  Widget handle;
  void *ptr;
};

struct UciControl controlDesc[] = {
  {"maximum nr of CPUs:", 0, &appData.smpCores},
  {"Polyglot Directory:", 0, &appData.polyglotDir},
  {"Hash Size (MB):",     0, &appData.defaultHashSize},
  {"EGTB Path:",          0, &appData.defaultPathEGTB},
  {"EGTB Cache (MB):",    0, &appData.defaultCacheSizeEGTB},
  {"Polyglot Book:",      0, &appData.polyglotBook},
  {NULL, 0, NULL},
};

void UciPopDown()
{
    if (!UciUp) return;
    XtPopdown(UciShell);
    XtDestroyWidget(UciShell);
    UciUp = False;
    ModeHighlight();
}

void UciCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    char buf[80];
    int oldCores = appData.smpCores, ponder = 0;
    
    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    if (strcmp(name, "OK") == 0) {
	int nr, i, j; String name;
	for(i=0; i<6; i++) {
	    XtSetArg(args[0], XtNstring, &name);
	    XtGetValues(controlDesc[i].handle, args, 1);
	    if(i&1) {
		if(name)
		    *(char**) controlDesc[i].ptr = strdup(name);
	    } else {
		if(sscanf(name, "%d", &j) == 1) 
		    *(int*) controlDesc[i].ptr = j;
	    }
	}
	XtSetArg(args[0], XtNstate, &appData.usePolyglotBook);
	XtGetValues(w1, args, 1);
	XtSetArg(args[0], XtNstate, &appData.firstHasOwnBookUCI);
	XtGetValues(w2, args, 1);
	XtSetArg(args[0], XtNstate, &appData.secondHasOwnBookUCI);
	XtGetValues(w3, args, 1);
	XtSetArg(args[0], XtNstate, &ponder);
	XtGetValues(w4, args, 1);

	// adjust setting in other menu for duplicates 
	// (perhaps duplicates should be removed from general Option Menu?)
	XtSetArg(args[0], XtNleftBitmap, ponder ? xMarkPixmap : None);
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Ponder Next Move"), args, 1);

	// make sure changes are sent to first engine by re-initializing it
	// if it was already started pre-emptively at end of previous game
	if(gameMode == BeginningOfGame) Reset(True, True); else {
	    // Some changed setting need immediate sending always.
	    PonderNextMoveEvent(ponder);
	    if(oldCores != appData.smpCores)
		NewSettingEvent(False, "cores", appData.smpCores);
      }
      UciPopDown();
      return;
    }
}

void UciPopUp()
{
    Arg args[16];
    Widget popup, layout, dialog, edit, form, b_ok, b_cancel, last = NULL, new, upperLeft;
    Window root, child;
    int x, y, i, j;
    int win_x, win_y;
    unsigned int mask;
    char def[80];
    
    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
//    XtSetArg(args[i], XtNwidth, 300); i++;
    UciShell = popup =
      XtCreatePopupShell("Engine Settings", transientShellWidgetClass,
			 shellWidget, args, i);
    
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  
    
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
  
    j = 0;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNheight, 20); j++;
    for(i = 0; controlDesc[i].name != NULL; i++) {
	j = 3;
	XtSetArg(args[j], XtNfromVert, last); j++;
//	XtSetArg(args[j], XtNwidth, 130); j++;
	XtSetArg(args[j], XtNjustify, XtJustifyLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft);  j++;
	XtSetArg(args[j], XtNborderWidth, 0); j++;
	new = XtCreateManagedWidget(controlDesc[i].name, labelWidgetClass, form, args, j);
	if(i==0) upperLeft = new;

	j = 4;
	XtSetArg(args[j], XtNborderWidth, 1); j++;
	XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
	XtSetArg(args[j], XtNdisplayCaret, False);  j++;
	XtSetArg(args[j], XtNright, XtChainRight);  j++;
	XtSetArg(args[j], XtNresizable, True);  j++;
	XtSetArg(args[j], XtNwidth, i&1 ? 245 : 50); j++;
	if(i&1) {
	    XtSetArg(args[j], XtNstring, * (char**) controlDesc[i].ptr ? 
					 * (char**) controlDesc[i].ptr : ""); j++;
	} else {
	    sprintf(def, "%d", * (int*) controlDesc[i].ptr);
	    XtSetArg(args[j], XtNstring, def); j++;
	}
	XtSetArg(args[j], XtNfromHoriz, upperLeft); j++;
	controlDesc[i].handle = last =
	    XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j);
	XtAddEventHandler(last, ButtonPressMask, False, SetFocus, (XtPointer) popup);
    }

    j=0;
    XtSetArg(args[j], XtNfromHoriz, controlDesc[0].handle);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNstate, appData.ponderNextMove);  j++;
    w4 = XtCreateManagedWidget("Ponder", toggleWidgetClass, form, args, j);   

    j=0;
    XtSetArg(args[j], XtNfromVert, last);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    b_ok = XtCreateManagedWidget("OK", commandWidgetClass, form, args, j);   
    XtAddCallback(b_ok, XtNcallback, UciCallback, (XtPointer) 0);

    XtSetArg(args[j], XtNfromHoriz, b_ok);  j++;
    b_cancel = XtCreateManagedWidget("cancel", commandWidgetClass, form, args, j);   
    XtAddCallback(b_cancel, XtNcallback, UciPopDown, (XtPointer) 0);

    j = 5;
    XtSetArg(args[j], XtNfromHoriz, upperLeft);  j++;
    XtSetArg(args[j], XtNstate, appData.usePolyglotBook);  j++;
    w1 = XtCreateManagedWidget(" use book ", toggleWidgetClass, form, args, j);   
//    XtAddCallback(w1, XtNcallback, UciCallback, (XtPointer) 0);

    j = 5;
    XtSetArg(args[j], XtNfromHoriz, w1);  j++;
    XtSetArg(args[j], XtNstate, appData.firstHasOwnBookUCI);  j++;
    w2 = XtCreateManagedWidget("own book 1", toggleWidgetClass, form, args, j);   
//    XtAddCallback(w2, XtNcallback, UciCallback, (XtPointer) 0);

    j = 5;
    XtSetArg(args[j], XtNfromHoriz, w2);  j++;
    XtSetArg(args[j], XtNstate, appData.secondHasOwnBookUCI);  j++;
    w3 = XtCreateManagedWidget("own book 2", toggleWidgetClass, form, args, j);   
//    XtAddCallback(w3, XtNcallback, UciCallback, (XtPointer) 0);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "UciPopDown");
    
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);
    
    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);
    
    XtPopup(popup, XtGrabExclusive);
    UciUp = True;

    previous = NULL;
    SetFocus(controlDesc[2].handle, popup, (XEvent*) NULL, False);
//    XtSetKeyboardFocus(popup, controlDesc[1].handle);
}

void UciMenuProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   UciPopUp();
}

//--------------------------- Engine-specific options menu ----------------------------------

int SettingsUp;
Widget SettingsShell;
int values[MAX_OPTIONS];
ChessProgramState *currentCps;

void SettingsPopDown()
{
    if (!SettingsUp) return;
    XtPopdown(SettingsShell);
    XtDestroyWidget(SettingsShell);
    SettingsUp = False;
    ModeHighlight();
}

void SpinCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name, val;
    Widget w2;
    Arg args[16];
    char buf[MSG_SIZ];
    int i, j;
    
    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    j = 0;
    XtSetArg(args[0], XtNstring, &val);
    XtGetValues(currentCps->option[(int)client_data].handle, args, 1);
    sscanf(val, "%d", &j);
    if (strcmp(name, "+") == 0) {
	if(++j > currentCps->option[(int)client_data].max) return;
    } else
    if (strcmp(name, "-") == 0) {
	if(--j < currentCps->option[(int)client_data].min) return;
    } else return;
    sprintf(buf, "%d", j);
    XtSetArg(args[0], XtNstring, buf);
    XtSetValues(currentCps->option[(int)client_data].handle, args, 1);
}

void SettingsCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name, val;
    Widget w2;
    Arg args[16];
    char buf[MSG_SIZ];
    int i, j;
    
    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    if (strcmp(name, "cancel") == 0) {
        SettingsPopDown();
        return;
    }
    if (strcmp(name, "OK") == 0 || (int)client_data) { // save buttons imply OK
	int nr;

	for(i=0; i<currentCps->nrOptions; i++) { // send all options that had to be OK-ed to engine
	    switch(currentCps->option[i].type) {
		case TextBox:
		    XtSetArg(args[0], XtNstring, &val);
		    XtGetValues(currentCps->option[i].handle, args, 1);
		    if(strcmp(currentCps->option[i].textValue, val)) {
			strcpy(currentCps->option[i].textValue, val);
			sprintf(buf, "option %s %s\n", currentCps->option[i].name, val);
			SendToProgram(buf, currentCps);
		    }
		    break;
		case Spin:
		    XtSetArg(args[0], XtNstring, &val);
		    XtGetValues(currentCps->option[i].handle, args, 1);
		    sscanf(val, "%d", &j);
		    if(j > currentCps->option[i].max) j = currentCps->option[i].max;
		    if(j < currentCps->option[i].min) j = currentCps->option[i].min;
		    if(currentCps->option[i].value != j) {
			currentCps->option[i].value = j;
			sprintf(buf, "option %s %d\n", currentCps->option[i].name, j);
			SendToProgram(buf, currentCps);
		    }
		    break;
		case CheckBox:
		    j = 0;
		    XtSetArg(args[0], XtNstate, &j);
		    XtGetValues(currentCps->option[i].handle, args, 1);
		    if(currentCps->option[i].value != j) {
			currentCps->option[i].value = j;
			sprintf(buf, "option %s %d\n", currentCps->option[i].name, j);
			SendToProgram(buf, currentCps);
		    }
		    break;
		case ComboBox:
		    if(currentCps->option[i].value != values[i]) {
			currentCps->option[i].value = values[i];
			sprintf(buf, "option %s %s\n", currentCps->option[i].name, 
				((char**)currentCps->option[i].textValue)[values[i]]);
			SendToProgram(buf, currentCps);
		    }
		    break;
	    }
	}
	if((int)client_data) { // send save-button command to engine
	    sprintf(buf, "option %s\n", name);
	    SendToProgram(buf, currentCps);
	}
        SettingsPopDown();
        return;
    }
    sprintf(buf, "option %s\n", name);
    SendToProgram(buf, currentCps);
}

void ComboSelect(w, addr, index) // callback for all combo items
     Widget w;
     caddr_t addr;
     caddr_t index;
{
    Arg args[16];
    int i = ((int)addr)>>8;
    int j = 255 & (int) addr;

    values[i] = j; // store in temporary, for transfer at OK
    XtSetArg(args[0], XtNlabel, ((char**)currentCps->option[i].textValue)[j]);
    XtSetValues(currentCps->option[i].handle, args, 1);
}

void CreateComboPopup(parent, name, n, mb)
     Widget parent;
     String name;
     int n;
     char *mb[];
{
    int i=0, j;
    Widget menu, entry;
    Arg args[16];

    menu = XtCreatePopupShell(name, simpleMenuWidgetClass,
			      parent, NULL, 0);
    j = 0;
    XtSetArg(args[j], XtNwidth, 100);  j++;
//    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    while (mb[i] != NULL) {
	    entry = XtCreateManagedWidget(mb[i], smeBSBObjectClass,
					  menu, args, j);
	    XtAddCallback(entry, XtNcallback,
			  (XtCallbackProc) ComboSelect,
			  (caddr_t) (256*n+i));
	i++;
    }
}	

void SettingsPopUp(ChessProgramState *cps)
{
    Arg args[16];
    Widget popup, layout, dialog, edit, form, oldform, last, b_ok, b_cancel;
    Window root, child;
    int x, y, i, j;
    int win_x, win_y;
    unsigned int mask;
    char def[80], *p, *q;

    // to do: start up second engine if needed
    if(!cps->initDone || !cps->nrOptions) return; // nothing to be done
    currentCps = cps;

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    SettingsShell = popup =
      XtCreatePopupShell("Settings Menu", transientShellWidgetClass,
			 shellWidget, args, i);
    
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  
    form =
      XtCreateManagedWidget(layoutName, formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
  
    last = NULL;
    for(i=0; i<cps->nrOptions; i++) {
	Widget box;
	switch(cps->option[i].type) {
	  case Spin:
	    sprintf(def, "%d", cps->option[i].value);
	  case TextBox:
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    dialog = XtCreateManagedWidget(cps->option[i].name, labelWidgetClass, form, args, j);   
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNfromHoriz, dialog);  j++;
	    XtSetArg(args[j], XtNborderWidth, 1); j++;
	    XtSetArg(args[j], XtNwidth, cps->option[i].type == Spin ? 40 : 100); j++;
	    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
	    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
	    XtSetArg(args[j], XtNright, XtChainRight);  j++;
	    XtSetArg(args[j], XtNresizable, True);  j++;
	    XtSetArg(args[j], XtNstring, cps->option[i].type==Spin ? def : cps->option[i].textValue);  j++;
	    edit = last;
	    cps->option[i].handle = (void*)
		(last = XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j));   
	    XtAddEventHandler(box, ButtonPressMask, False, SetFocus, (XtPointer) popup);
	    if(cps->option[i].type == TextBox) break;
	    // add increment and decrement controls for spin
	    j=0;
	    XtSetArg(args[j], XtNfromVert, edit);  j++;
	    XtSetArg(args[j], XtNfromHoriz, box);  j++;
	    XtSetArg(args[j], XtNheight, 10);  j++;
	    XtSetArg(args[j], XtNwidth, 20);  j++;
	    edit = XtCreateManagedWidget("+", commandWidgetClass, form, args, j);
	    XtAddCallback(edit, XtNcallback, SpinCallback, (XtPointer) i);
	    j=0;
	    XtSetArg(args[j], XtNfromVert, edit);  j++;
	    XtSetArg(args[j], XtNfromHoriz, last);  j++;
	    XtSetArg(args[j], XtNheight, 10);  j++;
	    XtSetArg(args[j], XtNwidth, 20);  j++;
	    last = XtCreateManagedWidget("-", commandWidgetClass, form, args, j);
	    XtAddCallback(last, XtNcallback, SpinCallback, (XtPointer) i);
	    break;
	  case CheckBox:
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNwidth, 10);  j++;
	    XtSetArg(args[j], XtNheight, 10);  j++;
	    XtSetArg(args[j], XtNstate, cps->option[i].value);  j++;
	    cps->option[i].handle = (void*) 
		(dialog = XtCreateManagedWidget(" ", toggleWidgetClass, form, args, j));   
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNfromHoriz, dialog);  j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    last = XtCreateManagedWidget(cps->option[i].name, labelWidgetClass, form, args, j);
	    break;
	  case SaveButton:
	  case Button:
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNstate, cps->option[i].value);  j++;
	    cps->option[i].handle = (void*) 
		(last = XtCreateManagedWidget(cps->option[i].name, commandWidgetClass, form, args, j));   
	    XtAddCallback(last, XtNcallback, SettingsCallback, (XtPointer) (cps->option[i].type == SaveButton));
	    break;
	  case ComboBox:
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    dialog = XtCreateManagedWidget(cps->option[i].name, labelWidgetClass, form, args, j);

	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNfromHoriz, dialog);  j++;
	    XtSetArg(args[j], XtNwidth, 100);  j++;
	    XtSetArg(args[j], XtNmenuName, XtNewString(cps->option[i].name));  j++;
	    XtSetArg(args[j], XtNlabel, ((char**)cps->option[i].textValue)[cps->option[i].value]);  j++;
	    cps->option[i].handle = (void*) 
		(last = XtCreateManagedWidget(" ", menuButtonWidgetClass, form, args, j));   
	    CreateComboPopup(last, cps->option[i].name, i, (char **) cps->option[i].textValue);
	    values[i] = cps->option[i].value;
	    break;
	}
    }

    j=0;
    XtSetArg(args[j], XtNfromVert, last);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    b_ok = XtCreateManagedWidget("OK", commandWidgetClass, form, args, j);   
    XtAddCallback(b_ok, XtNcallback, SettingsCallback, (XtPointer) 0);

    XtSetArg(args[j], XtNfromHoriz, b_ok);  j++;
    b_cancel = XtCreateManagedWidget("cancel", commandWidgetClass, form, args, j);   
    XtAddCallback(b_cancel, XtNcallback, SettingsPopDown, (XtPointer) 0);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "SettingsPopDown");
    
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);
    
    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);
    
    XtPopup(popup, XtGrabExclusive);
    SettingsUp = True;

    previous = NULL;
    SetFocus(edit, popup, (XEvent*) NULL, False);
}

void FirstSettingsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   SettingsPopUp(&first);
}

void SecondSettingsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   SettingsPopUp(&second);
}

//--------------------------- General Popup for Cloning ----------------------------------
#if 0
int XXXUp;
Widget XXXShell;

void XXXPopDown()
{
    if (!XXXUp) return;
    XtPopdown(XXXShell);
    XtDestroyWidget(XXXShell);
    XXXUp = False;
    ModeHighlight();
}

void XXXCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Widget w2;
    Arg args[16];
    char buf[80];
    
    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);
    
    if (strcmp(name, "cancel") == 0) {
        XXXPopDown();
        return;
    }
    if (strcmp(name, "ok") == 0) {
	int nr; String name;
        name = XawDialogGetValueString(w2 = XtParent(w));
	if(sscanf(name ,"%d",&nr) != 1) {
	    sprintf(buf, "%d", appData.defaultFrcPosition);
	    XtSetArg(args[0],XtNvalue, buf); // erase bad (non-numeric) value
	    XtSetValues(w2, args, 1);
	    return;
	}
        XXXPopDown();
        return;
    }
}

void XXXPopUp()
{
    Arg args[16];
    Widget popup, layout, dialog, edit;
    Window root, child;
    int x, y, i;
    int win_x, win_y;
    unsigned int mask;
    char def[80];
    
    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    XXXShell = popup =
      XtCreatePopupShell("XXX Menu", transientShellWidgetClass,
			 shellWidget, args, i);
    
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  
    sprintf(def, "%d\n", appData.defaultFrcPosition);
    i = 0;
    XtSetArg(args[i], XtNlabel, ""); i++;
    XtSetArg(args[i], XtNvalue, def); i++;
    XtSetArg(args[i], XtNborderWidth, 0); i++;
    dialog = XtCreateManagedWidget("XXX", dialogWidgetClass,
				   layout, args, i);
    
    XawDialogAddButton(dialog, "ok", XXXCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, "cancel", XXXCallback, (XtPointer) dialog);
    
    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "XXXPopDown");
    
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);
    
    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);
    
    XtPopup(popup, XtGrabExclusive);
    XXXUp = True;
    
    edit = XtNameToWidget(dialog, "*value");

    previous = NULL;
    SetFocus(engThreshold, popup, (XEvent*) NULL, False);
}

void XXXMenuProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   XXXPopUp();
}
#endif

