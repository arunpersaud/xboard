/*
 * xoptions.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

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
#include <stdint.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
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
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

// [HGM] the following code for makng menu popups was cloned from the FileNamePopUp routines

static Widget previous = NULL;

void SetFocus(Widget w, XtPointer data, XEvent *event, Boolean *b)
{
    Arg args[2];
    char *s;
    int j;

    if(previous) {
	XtSetArg(args[0], XtNdisplayCaret, False);
	XtSetValues(previous, args, 1);
    }
    XtSetArg(args[0], XtNstring, &s);
    XtGetValues(w, args, 1);
    j = 1;
    XtSetArg(args[0], XtNdisplayCaret, True);
    if(!strchr(s, '\n')) XtSetArg(args[1], XtNinsertPosition, strlen(s)), j++;
    XtSetValues(w, args, j);
    XtSetKeyboardFocus((Widget) data, w);
    previous = w;
}

//--------------------------- New Shuffle Game --------------------------------------------
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
    char buf[MSG_SIZ];

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    if (strcmp(name, _("cancel")) == 0) {
        ShufflePopDown();
        return;
    }
    if (strcmp(name, _("off")) == 0) {
        ShufflePopDown();
	shuffleOpenings = False; // [HGM] should be moved to New Variant menu, once we have it!
	ResetGameEvent();
        return;
    }
    if (strcmp(name, _("random")) == 0) {
      snprintf(buf, MSG_SIZ,  "%d", (int) random());
	XtSetArg(args[0],XtNvalue, buf); // erase bad (non-numeric) value
	XtSetValues(XtParent(w), args, 1);
        return;
    }
    if (strcmp(name, _("ok")) == 0) {
	int nr; String name;
        name = XawDialogGetValueString(w2 = XtParent(w));
	if(sscanf(name ,"%d",&nr) != 1) {
	  snprintf(buf, MSG_SIZ,  "%d", appData.defaultFrcPosition);
	    XtSetArg(args[0],XtNvalue, buf); // erase bad (non-numeric) value
	    XtSetValues(w2, args, 1);
	    return;
	}
	appData.defaultFrcPosition = nr;
	shuffleOpenings = True;
        ShufflePopDown();
	ResetGameEvent();
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
    char def[MSG_SIZ];

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    shuffleShell = popup =
      XtCreatePopupShell(_("New Shuffle Game"), transientShellWidgetClass,
			 shellWidget, args, i);

    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));

    snprintf(def, MSG_SIZ,  "%d\n", appData.defaultFrcPosition);
    i = 0;
    XtSetArg(args[i], XtNlabel, _("Start-position number:")); i++;
    XtSetArg(args[i], XtNvalue, def); i++;
    XtSetArg(args[i], XtNborderWidth, 0); i++;
    dialog = XtCreateManagedWidget(_("Shuffle"), dialogWidgetClass,
				   layout, args, i);

//    XtSetArg(args[0], XtNeditType, XawtextEdit);  // [HGM] can't get edit to work decently
//    XtSetArg(args[1], XtNuseStringInPlace, False);
//    XtSetValues(dialog, args, 2);

    XawDialogAddButton(dialog, _("ok"), ShuffleCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, _("cancel"), ShuffleCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, _("random"), ShuffleCallback, (XtPointer) dialog);
    XawDialogAddButton(dialog, _("off"), ShuffleCallback, (XtPointer) dialog);

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
    previous = NULL;
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
    Arg args[16];
    char buf[MSG_SIZ];
    int j;

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    if (strcmp(name, _("classical")) == 0) {
	if(tcInc == 0) return;
	j=0;
	XtSetArg(args[j], XtNlabel, _("minutes for each")); j++;
	XtSetValues(tcMess1, args, j);
	j=0;
	XtSetArg(args[j], XtNlabel, _("moves")); j++;
	XtSetValues(tcMess2, args, j);
	if(tcInc == 1) {
	    j=0;
	    XtSetArg(args[j], XtNstring, &name); j++;
	    XtGetValues(tcData, args, j);
	    tcIncrement = 0; sscanf(name, "%d", &tcIncrement);
	}
	snprintf(buf, MSG_SIZ,  "%d", tcMoves);
	j=0;
	XtSetArg(args[j], XtNstring, buf); j++;
	XtSetValues(tcData, args, j);
	tcInc = 0;
        return;
    }
    if (strcmp(name, _("incremental")) == 0) {
	if(tcInc == 1) return;
	j=0;
	XtSetArg(args[j], XtNlabel, _("minutes, plus")); j++;
	XtSetValues(tcMess1, args, j);
	j=0;
	XtSetArg(args[j], XtNlabel, _("sec/move")); j++;
	XtSetValues(tcMess2, args, j);
	if(tcInc == 0) {
	    j=0;
	    XtSetArg(args[j], XtNstring, &name); j++;
	    XtGetValues(tcData, args, j);
	    tcMoves = appData.movesPerSession; sscanf(name, "%d", &tcMoves);
	}
	snprintf(buf, MSG_SIZ,  "%d", tcIncrement);
	j=0;
	XtSetArg(args[j], XtNstring, buf); j++;
	XtSetValues(tcData, args, j);
	tcInc = 1;
        return;
    }
    if (strcmp(name, _("fixed time")) == 0) {
	if(tcInc == 2) return;
	j=0;
	XtSetArg(args[j], XtNlabel, _("sec/move (max)")); j++;
	XtSetValues(tcMess1, args, j);
	j=0;
	XtSetArg(args[j], XtNlabel, _("")); j++;
	XtSetValues(tcMess2, args, j);
	j=0;
	XtSetArg(args[j], XtNstring, ""); j++;
	XtSetValues(tcData, args, j);
	tcInc = 2;
        return;
    }
    if (strcmp(name, _(" OK ")) == 0) {
	int inc, mps, ok=0;
	XtSetArg(args[0], XtNstring, &txt);
	XtGetValues(tcData, args, 1);
	switch(tcInc) {
	  case 1:
	    ok = sscanf(txt, "%d", &inc); mps = 0;
	    if(!ok && txt[0] == 0) { inc = 0; ok = 1; } // accept empty string as zero
	    ok &= (inc >= 0);
	    break;
	  case 0:
	    ok = sscanf(txt, "%d", &mps); inc = -1;
	    ok &= (mps > 0);
	    break;
	  case 2:
	    ok = 1; inc = -1; mps = 40;
	}
	if(ok != 1) {
	    XtSetArg(args[0], XtNstring, ""); // erase any offending input
	    XtSetValues(tcData, args, 1);
	    return;
	}
	XtSetArg(args[0], XtNstring, &txt);
	XtGetValues(tcTime, args, 1);
	if(tcInc == 2) {
	    if(sscanf(txt, "%d", &inc) != 1) {
		XtSetArg(args[0], XtNstring, ""); // erase any offending input
		XtSetValues(tcTime, args, 1);
		DisplayError(_("Bad Time-Control String"), 0);
		return;
	    }
	    searchTime = inc;
	} else {
	    if(!ParseTimeControl(txt, inc, mps)) {
		XtSetArg(args[0], XtNstring, ""); // erase any offending input
		XtSetValues(tcTime, args, 1);
		DisplayError(_("Bad Time-Control String"), 0);
		return;
	    }
	    searchTime = 0;
	    appData.movesPerSession = mps;
	    appData.timeIncrement = inc;
	    appData.timeControl = strdup(txt);
	}
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
    Widget popup, layout, form,  b_ok, b_cancel, b_clas, b_inc, mess;
    Window root, child;
    int x, y, i, j;
    int win_x, win_y;
    unsigned int mask;
    char def[MSG_SIZ];

    tcInc = searchTime > 0 ? 2 : (appData.timeIncrement >= 0);
    tcMoves = appData.movesPerSession; tcIncrement = appData.timeIncrement;
    if(!tcInc) tcIncrement = 0;
    snprintf(def, MSG_SIZ,  "%d", tcInc ? tcIncrement : tcMoves);

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
//    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    TimeControlShell = popup =
      XtCreatePopupShell(_("TimeControl Menu"), transientShellWidgetClass,
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
    XtSetArg(args[j], XtNinsertPosition, 9999);  j++;
    tcTime = XtCreateManagedWidget("TC", asciiTextWidgetClass, form, args, j);
    XtAddEventHandler(tcTime, ButtonPressMask, False, SetFocus, (XtPointer) popup);

    j= 0;
    XtSetArg(args[j], XtNlabel, tcInc ? tcInc == 2 ? _("sec/move (max)   ") : _("   minutes, plus   ") : _("minutes for each")); j++;
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
    XtSetArg(args[j], XtNlabel, tcInc ? tcInc == 2 ? _("             ") : _("sec/move") : _("moves     ")); j++;
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
    XtSetArg(args[j], XtNlabel, _("Engine #1 and #2 Time-Odds Factors")); j++;
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
    XtSetArg(args[j], XtNstate, tcInc==0); j++;
    b_clas= XtCreateManagedWidget(_("classical"), toggleWidgetClass,
				   form, args, j);
    XtAddCallback(b_clas, XtNcallback, TimeControlCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNradioGroup, b_clas); j++;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_clas);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNstate, tcInc==1); j++;
    b_inc = XtCreateManagedWidget(_("incremental"), toggleWidgetClass,
				   form, args, j);
    XtAddCallback(b_inc, XtNcallback, TimeControlCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNradioGroup, b_inc); j++;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_inc);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNstate, tcInc==2); j++;
    b_inc = XtCreateManagedWidget(_("fixed time"), toggleWidgetClass,
				   form, args, j);
    XtAddCallback(b_inc, XtNcallback, TimeControlCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNfromHoriz, tcData);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_ok= XtCreateManagedWidget(_(" OK "), commandWidgetClass,
				   form, args, j);
    XtAddCallback(b_ok, XtNcallback, TimeControlCallback, (XtPointer) 0);

    j=0;
    XtSetArg(args[j], XtNfromVert, tcOdds1);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_ok);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_cancel= XtCreateManagedWidget(_("cancel"), commandWidgetClass,
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

//--------------------------- Engine-specific options menu ----------------------------------

int values[MAX_OPTIONS];
ChessProgramState *currentCps;
static Option *currentOption;
static Boolean browserUp;

void CheckCallback(Widget ww, XtPointer data, XEvent *event, Boolean *b)
{
    Widget w = currentOption[(int)(intptr_t)data].handle;
    Boolean s;
    Arg args[16];

    XtSetArg(args[0], XtNstate, &s);
    XtGetValues(w, args, 1);
    XtSetArg(args[0], XtNstate, !s);
    XtSetValues(w, args, 1);
}

void SpinCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name, val;
    Arg args[16];
    char buf[MSG_SIZ], *p;
    int j;
    int data = (intptr_t) client_data;

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    j = 0;
    XtSetArg(args[0], XtNstring, &val);
    XtGetValues(currentOption[data].handle, args, 1);
    sscanf(val, "%d", &j);
    if (strcmp(name, "browse") == 0) {
	char *q, *r;
	XtSetArg(args[0], XtNstring, &q);
	XtGetValues(currentOption[data].handle, args, 1);
	for(r = ""; *q; q++) if(*q == '.') r = q; else if(*q == '/') r = ""; // last dot after last slash
	browserUp = True;
	if(XsraSelFile(shells[0], currentOption[data].name, NULL, NULL, "", "", r,
			 	  currentOption[data].type == PathName ? "p" : "f", NULL, &p)) {
		int len = strlen(p);
		if(len && p[len-1] == '/') p[len-1] = NULLCHAR;
		XtSetArg(args[0], XtNstring, p);
		XtSetValues(currentOption[data].handle, args, 1);
	}
	browserUp = False;
	SetFocus(currentOption[data].handle, shells[0], (XEvent*) NULL, False);
	return;
    } else
    if (strcmp(name, "+") == 0) {
	if(++j > currentOption[data].max) return;
    } else
    if (strcmp(name, "-") == 0) {
	if(--j < currentOption[data].min) return;
    } else return;
    snprintf(buf, MSG_SIZ,  "%d", j);
    XtSetArg(args[0], XtNstring, buf);
    XtSetValues(currentOption[data].handle, args, 1);
    SetFocus(currentOption[data].handle, shells[0], NULL, False);
}

void ComboSelect(w, addr, index) // callback for all combo items
     Widget w;
     caddr_t addr;
     caddr_t index;
{
    Arg args[16];
    int i = ((intptr_t)addr)>>8;
    int j = 255 & (intptr_t) addr;

    values[i] = j; // store in temporary, for transfer at OK
    XtSetArg(args[0], XtNlabel, ((char**)currentOption[i].textValue)[j]);
    XtSetValues(currentOption[i].handle, args, 1);
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
			  (caddr_t)(intptr_t) (256*n+i));
	i++;
    }
}


//----------------------------Generic dialog --------------------------------------------

// cloned from Engine Settings dialog (and later merged with it)

typedef void ButtonCallback(int n);

char *trialSound;
static int oldCores, oldPonder;
int MakeColors P((void));
void CreateGCs P((int redo));
void CreateAnyPieces P((void));
void GenericReadout P((int selected));
Widget shells[10];
Widget marked[10];
Boolean shellUp[10];
WindowPlacement *wp[10];
Option *dialogOptions[10];

void MarkMenu(char *item, int dlgNr)
{
    Arg args[2];
    XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    XtSetValues(marked[dlgNr] = XtNameToWidget(menuBarWidget, item), args, 1);
}

int PopDown(int n)
{
    int j;
    Arg args[10];
    Dimension windowH, windowW; Position windowX, windowY;
    if (!shellUp[n]) return 0;
    if(n && wp[n]) { // remember position
	j = 0;
	XtSetArg(args[j], XtNx, &windowX); j++;
	XtSetArg(args[j], XtNy, &windowY); j++;
	XtSetArg(args[j], XtNheight, &windowH); j++;
	XtSetArg(args[j], XtNwidth, &windowW); j++;
	XtGetValues(shells[n], args, j);
	wp[n]->x = windowX;
	wp[n]->x = windowY;
	wp[n]->width  = windowW;
	wp[n]->height = windowH;
    }
    previous = NULL;
    XtPopdown(shells[n]);
    if(n == 0) XtDestroyWidget(shells[n]);
    shellUp[n] = False;
    if(marked[n]) {
	XtSetArg(args[0], XtNleftBitmap, None);
	XtSetValues(marked[n], args, 1);
    }
    if(!n) currentCps = NULL; // if an Engine Settings dialog was up, we must be popping it down now
    return 1;
}

void GenericPopDown(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if(browserUp) return; // prevent closing dialog when it has an open file-browse daughter
    PopDown(prms[0][0] - '0');
}

Option matchOptions[] = {
{ 0,  2, 1000000000, NULL, (void*) &appData.defaultMatchGames, "", NULL, Spin, N_("Default Number of Games in Match:") },
{ 0,  0, 1000000000, NULL, (void*) &appData.matchPause, "", NULL, Spin, N_("Pause between Match Games (msec):") },
{ 0,  0,          0, NULL, (void*) &appData.loadGameFile, "", NULL, FileName, N_("Game File with Opening Lines:") },
{ 0, -2, 1000000000, NULL, (void*) &appData.loadGameIndex, "", NULL, Spin, N_("Game Number (-1 or -2 = Auto-Increment):") },
{ 0,  0,          0, NULL, (void*) &appData.loadPositionFile, "", NULL, FileName, N_("File with Start Positions:") },
{ 0, -2, 1000000000, NULL, (void*) &appData.loadPositionIndex, "", NULL, Spin, N_("Position Number (-1 or -2 = Auto-Increment):") },
{ 0,  0, 1000000000, NULL, (void*) &appData.rewindIndex, "", NULL, Spin, N_("Rewind Index after this many Games (0 = never):") },
{ 0, 0, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void GeneralOptionsOK(int n)
{
	int newPonder = appData.ponderNextMove;
	appData.ponderNextMove = oldPonder;
	PonderNextMoveEvent(newPonder);
}

Option generalOptions[] = {
{ 0,  0, 0, NULL, (void*) &appData.alwaysPromoteToQueen, "", NULL, CheckBox, N_("Always Queen") },
{ 0,  0, 0, NULL, (void*) &appData.animateDragging, "", NULL, CheckBox, N_("Animate Dragging") },
{ 0,  0, 0, NULL, (void*) &appData.animate, "", NULL, CheckBox, N_("Animate Moving") },
{ 0,  0, 0, NULL, (void*) &appData.autoCallFlag, "", NULL, CheckBox, N_("Auto Flag") },
{ 0,  0, 0, NULL, (void*) &appData.autoFlipView, "", NULL, CheckBox, N_("Auto Flip View") },
{ 0,  0, 0, NULL, (void*) &appData.blindfold, "", NULL, CheckBox, N_("Blindfold") },
{ 0,  0, 0, NULL, (void*) &appData.dropMenu, "", NULL, CheckBox, N_("Drop Menu") },
{ 0,  0, 0, NULL, (void*) &appData.highlightDragging, "", NULL, CheckBox, N_("Highlight Dragging (Show Move Targets)") },
{ 0,  0, 0, NULL, (void*) &appData.highlightLastMove, "", NULL, CheckBox, N_("Highlight Last Move") },
{ 0,  0, 0, NULL, (void*) &appData.highlightMoveWithArrow, "", NULL, CheckBox, N_("Highlight with Arrow") },
{ 0,  0, 0, NULL, (void*) &appData.ringBellAfterMoves, "", NULL, CheckBox, N_("Move Sound") },
{ 0,  0, 0, NULL, (void*) &appData.oneClick, "", NULL, CheckBox, N_("One-Click Moving") },
{ 0,  0, 0, NULL, (void*) &appData.periodicUpdates, "", NULL, CheckBox, N_("Periodic Updates (in Analysis Mode)") },
{ 0,  0, 0, NULL, (void*) &appData.ponderNextMove, "", NULL, CheckBox, N_("Ponder Next Move") },
{ 0,  0, 0, NULL, (void*) &appData.popupExitMessage, "", NULL, CheckBox, N_("Popup Exit Messages") },
{ 0,  0, 0, NULL, (void*) &appData.popupMoveErrors, "", NULL, CheckBox, N_("Popup Move Errors") },
{ 0,  0, 0, NULL, (void*) &appData.showCoords, "", NULL, CheckBox, N_("Show Coordinates") },
{ 0,  0, 0, NULL, (void*) &appData.markers, "", NULL, CheckBox, N_("Show Target Squares") },
{ 0,  0, 0, NULL, (void*) &appData.hideThinkingFromHuman, "", NULL, CheckBox, N_("Hide Thinking from Human") },
{ 0,  0, 0, NULL, (void*) &appData.testLegality, "", NULL, CheckBox, N_("Test Legality") },
{ 0, 0, 10, NULL, (void*) &appData.flashCount, "", NULL, Spin, N_("Flash Moves (0 = no flashing):") },
{ 0, 1, 10, NULL, (void*) &appData.flashRate, "", NULL, Spin, N_("Flash Rate (high = fast):") },
{ 0, 5, 100,NULL, (void*) &appData.animSpeed, "", NULL, Spin, N_("Animation Speed (high = slow):") },
{ 0,  0, 0, NULL, (void*) &GeneralOptionsOK, "", NULL, EndMark , "" }
};

void Pick(int n)
{
	VariantClass v = currentOption[n].value;
	if(!appData.noChessProgram) {
	    char *name = VariantName(v), buf[MSG_SIZ];
	    if (first.protocolVersion > 1 && StrStr(first.variants, name) == NULL) {
		/* [HGM] in protocol 2 we check if variant is suported by engine */
	      snprintf(buf, MSG_SIZ,  _("Variant %s not supported by %s"), name, first.tidy);
		DisplayError(buf, 0);
		return; /* ignore OK if first engine does not support it */
	    } else
	    if (second.initDone && second.protocolVersion > 1 && StrStr(second.variants, name) == NULL) {
	      snprintf(buf, MSG_SIZ,  _("Warning: second engine (%s) does not support this!"), second.tidy);
		DisplayError(buf, 0);   /* use of second engine is optional; only warn user */
	    }
	}

	GenericReadout(-1); // make sure ranks and file settings are read

	gameInfo.variant = v;
	appData.variant = VariantName(v);

	shuffleOpenings = FALSE; /* [HGM] shuffle: possible shuffle reset when we switch */
	startedFromPositionFile = FALSE; /* [HGM] loadPos: no longer valid in new variant */
	appData.pieceToCharTable = NULL;
	appData.pieceNickNames = "";
	appData.colorNickNames = "";
	Reset(True, True);
        PopDown(0);
        return;
}

Option variantDescriptors[] = {
{ VariantNormal, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("normal")},
{ VariantFairy, 1, 135, NULL, (void*) &Pick, "#BFBFBF", NULL, Button, N_("fairy")},
{ VariantFischeRandom, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("FRC")},
{ VariantSChess, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Seirawan")},
{ VariantWildCastle, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("wild castle")},
{ VariantSuper, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Superchess")},
{ VariantNoCastle, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("no castle")},
{ VariantCrazyhouse, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("crazyhouse")},
{ VariantKnightmate, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("knightmate")},
{ VariantBughouse, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("bughouse")},
{ VariantBerolina, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("berolina")},
{ VariantShogi, 1, 135, NULL, (void*) &Pick, "#BFFFFF", NULL, Button, N_("shogi (9x9)")},
{ VariantCylinder, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("cylinder")},
{ VariantXiangqi, 1, 135, NULL, (void*) &Pick, "#BFFFFF", NULL, Button, N_("xiangqi (9x10)")},
{ VariantShatranj, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("shatranj")},
{ VariantCourier, 1, 135, NULL, (void*) &Pick, "#BFFFBF", NULL, Button, N_("courier (12x8)")},
{ VariantMakruk, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("makruk")},
{ VariantGreat, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Great Shatranj (10x8)")},
{ VariantAtomic, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("atomic")},
{ VariantCapablanca, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Capablanca (10x8)")},
{ VariantTwoKings, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("two kings")},
{ VariantGothic, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Gothic (10x8)")},
{ Variant3Check, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("3-checks")},
{ VariantJanus, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("janus (10x8)")},
{ VariantSuicide, 0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("suicide")},
{ VariantCapaRandom, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("CRC (10x8)")},
{ VariantGiveaway, 0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("give-away")},
{ VariantSpartan, 1, 135, NULL, (void*) &Pick, "#FF0000", NULL, Button, N_("Spartan")},
{ VariantLosers, 0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("losers")},
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label, N_("Board size ( -1 = default for selected variant):")},
{ 0, -1, BOARD_RANKS-1, NULL, (void*) &appData.NrRanks, "", NULL, Spin, N_("Number of Board Ranks:") },
{ 0, -1, BOARD_FILES, NULL, (void*) &appData.NrFiles, "", NULL, Spin, N_("Number of Board Files:") },
{ 0, -1, BOARD_RANKS-1, NULL, (void*) &appData.holdingsSize, "", NULL, Spin, N_("Holdings Size:") },
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label,
				N_("WARNING: variants with un-orthodox\n"
				  "pieces only have built-in bitmaps\n"
				  "for -boardSize middling, bulky and\n"
				  "petite, and substitute king or amazon\n"
				  "for missing bitmaps. (See manual.)")},
{ 0, 2, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void CommonOptionsOK(int n)
{
	int newPonder = appData.ponderNextMove;
	// make sure changes are sent to first engine by re-initializing it
	// if it was already started pre-emptively at end of previous game
	if(gameMode == BeginningOfGame) Reset(True, True); else {
	    // Some changed setting need immediate sending always.
	    if(oldCores != appData.smpCores)
		NewSettingEvent(False, &(first.maxCores), "cores", appData.smpCores);
	    appData.ponderNextMove = oldPonder;
	    PonderNextMoveEvent(newPonder);
	}
}

Option commonEngineOptions[] = {
{ 0,     0, 0, NULL, (void*) &appData.ponderNextMove, "", NULL, CheckBox, N_("Ponder Next Move") },
{ 0,  0, 1000, NULL, (void*) &appData.smpCores, "", NULL, Spin, N_("Maximum Number of CPUs per Engine:") },
{ 0,     0, 0, NULL, (void*) &appData.polyglotDir, "", NULL, PathName, N_("Polygot Directory:") },
{ 0, 0, 16000, NULL, (void*) &appData.defaultHashSize, "", NULL, Spin, N_("Hash-Table Size (MB):") },
{ 0,     0, 0, NULL, (void*) &appData.defaultPathEGTB, "", NULL, PathName, N_("Nalimov EGTB Path:") },
{ 0,  0, 1000, NULL, (void*) &appData.defaultCacheSizeEGTB, "", NULL, Spin, N_("EGTB Cache Size (MB):") },
{ 0,     0, 0, NULL, (void*) &appData.usePolyglotBook, "", NULL, CheckBox, N_("Use GUI Book") },
{ 0,     0, 0, NULL, (void*) &appData.polyglotBook, "", NULL, FileName, N_("Opening-Book Filename:") },
{ 0,   0, 100, NULL, (void*) &appData.bookDepth, "", NULL, Spin, N_("Book Depth (moves):") },
{ 0,   0, 100, NULL, (void*) &appData.bookStrength, "", NULL, Spin, N_("Book Variety (0) vs. Strength (100):") },
{ 0,     0, 0, NULL, (void*) &appData.firstHasOwnBookUCI, "", NULL, CheckBox, N_("Engine #1 Has Own Book") },
{ 0,     0, 0, NULL, (void*) &appData.secondHasOwnBookUCI, "", NULL, CheckBox, N_("Engine #2 Has Own Book          ") },
{ 0,     1, 0, NULL, (void*) &CommonOptionsOK, "", NULL, EndMark , "" }
};

Option adjudicationOptions[] = {
{ 0, 0,    0, NULL, (void*) &appData.checkMates, "", NULL, CheckBox, N_("Detect all Mates") },
{ 0, 0,    0, NULL, (void*) &appData.testClaims, "", NULL, CheckBox, N_("Verify Engine Result Claims") },
{ 0, 0,    0, NULL, (void*) &appData.materialDraws, "", NULL, CheckBox, N_("Draw if Insufficient Mating Material") },
{ 0, 0,    0, NULL, (void*) &appData.trivialDraws, "", NULL, CheckBox, N_("Adjudicate Trivial Draws (3-Move Delay)") },
{ 0, 0,  100, NULL, (void*) &appData.ruleMoves, "", NULL, Spin, N_("N-Move Rule:") },
{ 0, 0,    6, NULL, (void*) &appData.drawRepeats, "", NULL, Spin, N_("N-fold Repeats:") },
{ 0, 0, 1000, NULL, (void*) &appData.adjudicateDrawMoves, "", NULL, Spin, N_("Draw after N Moves Total:") },
{ 0,-5000, 0, NULL, (void*) &appData.adjudicateLossThreshold, "", NULL, Spin, N_("Win / Loss Threshold:") },
{ 0, 0,    0, NULL, (void*) &first.scoreIsAbsolute, "", NULL, CheckBox, N_("Negate Score of Engine #1") },
{ 0, 0,    0, NULL, (void*) &second.scoreIsAbsolute, "", NULL, CheckBox, N_("Negate Score of Engine #2") },
{ 0, 1,    0, NULL, NULL, "", NULL, EndMark , "" }
};

void IcsOptionsOK(int n)
{
    ParseIcsTextColors();
}

Option icsOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoKibitz, "",  NULL, CheckBox, N_("Auto-Kibitz") },
{ 0, 0, 0, NULL, (void*) &appData.autoComment, "", NULL, CheckBox, N_("Auto-Comment") },
{ 0, 0, 0, NULL, (void*) &appData.autoObserve, "", NULL, CheckBox, N_("Auto-Observe") },
{ 0, 0, 0, NULL, (void*) &appData.autoRaiseBoard, "", NULL, CheckBox, N_("Auto-Raise Board") },
{ 0, 0, 0, NULL, (void*) &appData.bgObserve, "",   NULL, CheckBox, N_("Background Observe while Playing") },
{ 0, 0, 0, NULL, (void*) &appData.dualBoard, "",   NULL, CheckBox, N_("Dual Board for Background-Observed Game") },
{ 0, 0, 0, NULL, (void*) &appData.getMoveList, "", NULL, CheckBox, N_("Get Move List") },
{ 0, 0, 0, NULL, (void*) &appData.quietPlay, "",   NULL, CheckBox, N_("Quiet Play") },
{ 0, 0, 0, NULL, (void*) &appData.seekGraph, "",   NULL, CheckBox, N_("Seek Graph") },
{ 0, 0, 0, NULL, (void*) &appData.autoRefresh, "", NULL, CheckBox, N_("Auto-Refresh Seek Graph") },
{ 0, 0, 0, NULL, (void*) &appData.premove, "",     NULL, CheckBox, N_("Premove") },
{ 0, 0, 0, NULL, (void*) &appData.premoveWhite, "", NULL, CheckBox, N_("Premove for White") },
{ 0, 0, 0, NULL, (void*) &appData.premoveWhiteText, "", NULL, TextBox, N_("First White Move:") },
{ 0, 0, 0, NULL, (void*) &appData.premoveBlack, "", NULL, CheckBox, N_("Premove for Black") },
{ 0, 0, 0, NULL, (void*) &appData.premoveBlackText, "", NULL, TextBox, N_("First Black Move:") },
{ 0, 0, 0, NULL, NULL, NULL, NULL, Break, "" },
{ 0, 0, 0, NULL, (void*) &appData.icsAlarm, "", NULL, CheckBox, N_("Alarm") },
{ 0, 0, 100000000, NULL, (void*) &appData.icsAlarmTime, "", NULL, Spin, N_("Alarm Time (msec):") },
//{ 0, 0, 0, NULL, (void*) &appData.chatBoxes, "", NULL, TextBox, N_("Startup Chat Boxes:") },
{ 0, 0, 0, NULL, (void*) &appData.colorize, "", NULL, CheckBox, N_("Colorize Messages") },
{ 0, 0, 0, NULL, (void*) &appData.colorShout, "", NULL, TextBox, N_("Shout Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorSShout, "", NULL, TextBox, N_("S-Shout Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorChannel1, "", NULL, TextBox, N_("Channel #1 Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorChannel, "", NULL, TextBox, N_("Other Channel Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorKibitz, "", NULL, TextBox, N_("Kibitz Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorTell, "", NULL, TextBox, N_("Tell Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorChallenge, "", NULL, TextBox, N_("Challenge Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorRequest, "", NULL, TextBox, N_("Request Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorSeek, "", NULL, TextBox, N_("Seek Text Colors:") },
{ 0, 0, 0, NULL, (void*) &IcsOptionsOK, "", NULL, EndMark , "" }
};

Option loadOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoDisplayTags, "", NULL, CheckBox, N_("Auto-Display Tags") },
{ 0, 0, 0, NULL, (void*) &appData.autoDisplayComment, "", NULL, CheckBox, N_("Auto-Display Comment") },
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label, N_("Auto-Play speed of loaded games\n(0 = instant, -1 = off):") },
{ 0, -1, 10000000, NULL, (void*) &appData.timeDelay, "", NULL, Fractional, N_("Seconds per Move:") },
{ 0,  0, 0, NULL, NULL, "", NULL, EndMark , "" }
};

Option saveOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoSaveGames, "", NULL, CheckBox, N_("Auto-Save Games") },
{ 0, 0, 0, NULL, (void*) &appData.saveGameFile, "", NULL, FileName,  N_("Save Games on File:") },
{ 0, 0, 0, NULL, (void*) &appData.savePositionFile, "", NULL, FileName,  N_("Save Final Positions on File:") },
{ 0, 0, 0, NULL, (void*) &appData.pgnEventHeader, "", NULL, TextBox,  N_("PGN Event Header:") },
{ 0, 0, 0, NULL, (void*) &appData.oldSaveStyle, "", NULL, CheckBox, N_("Old Save Style (as opposed to PGN)") },
{ 0, 0, 0, NULL, (void*) &appData.saveExtendedInfoInPGN, "", NULL, CheckBox, N_("Save Score/Depth Info in PGN") },
{ 0, 0, 0, NULL, (void*) &appData.saveOutOfBookInfo, "", NULL, CheckBox, N_("Save Out-of-Book Info in PGN           ") },
{ 0, 1, 0, NULL, NULL, "", NULL, EndMark , "" }
};

char *soundNames[] = {
	N_("No Sound"),
	N_("Default Beep"),
	N_("Above WAV File"),
	N_("Car Horn"),
	N_("Cymbal"),
	N_("Ding"),
	N_("Gong"),
	N_("Laser"),
	N_("Penalty"),
	N_("Phone"),
	N_("Pop"),
	N_("Slap"),
	N_("Wood Thunk"),
	NULL,
	N_("User File")
};

char *soundFiles[] = { // sound files corresponding to above names
	"",
	"$",
	"*", // kludge alert: as first thing in the dialog readout this is replaced with the user-given .WAV filename
	"honkhonk.wav",
	"cymbal.wav",
	"ding1.wav",
	"gong.wav",
	"laser.wav",
	"penalty.wav",
	"phone.wav",
	"pop2.wav",
	"slap.wav",
	"woodthunk.wav",
	NULL,
	NULL
};

void Test(int n)
{
    GenericReadout(2);
    if(soundFiles[values[3]]) PlaySound(soundFiles[values[3]]);
}

Option soundOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.soundProgram, "", NULL, TextBox, N_("Sound Program:") },
{ 0, 0, 0, NULL, (void*) &appData.soundDirectory, "", NULL, PathName, N_("Sounds Directory:") },
{ 0, 0, 0, NULL, (void*) (soundFiles+2) /* kludge! */, "", NULL, FileName, N_("User WAV File:") },
{ 0, 0, 0, NULL, (void*) &trialSound, (char*) soundNames, soundFiles, ComboBox, N_("Try-Out Sound:") },
{ 0, 1, 0, NULL, (void*) &Test, NULL, NULL, Button, N_("Play") },
{ 0, 0, 0, NULL, (void*) &appData.soundMove, (char*) soundNames, soundFiles, ComboBox, N_("Move:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsWin, (char*) soundNames, soundFiles, ComboBox, N_("Win:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsLoss, (char*) soundNames, soundFiles, ComboBox, N_("Lose:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsDraw, (char*) soundNames, soundFiles, ComboBox, N_("Draw:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsUnfinished, (char*) soundNames, soundFiles, ComboBox, N_("Unfinished:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsAlarm, (char*) soundNames, soundFiles, ComboBox, N_("Alarm:") },
{ 0, 0, 0, NULL, (void*) &appData.soundShout, (char*) soundNames, soundFiles, ComboBox, N_("Shout:") },
{ 0, 0, 0, NULL, (void*) &appData.soundSShout, (char*) soundNames, soundFiles, ComboBox, N_("S-Shout:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChannel, (char*) soundNames, soundFiles, ComboBox, N_("Channel:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChannel1, (char*) soundNames, soundFiles, ComboBox, N_("Channel 1:") },
{ 0, 0, 0, NULL, (void*) &appData.soundTell, (char*) soundNames, soundFiles, ComboBox, N_("Tell:") },
{ 0, 0, 0, NULL, (void*) &appData.soundKibitz, (char*) soundNames, soundFiles, ComboBox, N_("Kibitz:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChallenge, (char*) soundNames, soundFiles, ComboBox, N_("Challenge:") },
{ 0, 0, 0, NULL, (void*) &appData.soundRequest, (char*) soundNames, soundFiles, ComboBox, N_("Request:") },
{ 0, 0, 0, NULL, (void*) &appData.soundSeek, (char*) soundNames, soundFiles, ComboBox, N_("Seek:") },
{ 0, 1, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void SetColor(char *colorName, Widget box)
{
	Arg args[5];
	Pixel buttonColor;
	XrmValue vFrom, vTo;
	if (!appData.monoMode) {
	    vFrom.addr = (caddr_t) colorName;
	    vFrom.size = strlen(colorName);
	    XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	    if (vTo.addr == NULL) {
	  	buttonColor = (Pixel) -1;
	    } else {
		buttonColor = *(Pixel *) vTo.addr;
	    }
	} else buttonColor = (Pixel) 0;
	XtSetArg(args[0], XtNbackground, buttonColor);;
	XtSetValues(box, args, 1);
}

void SetColorText(int n, char *buf)
{
    Arg args[5];
    XtSetArg(args[0], XtNstring, buf);
    XtSetValues(currentOption[n-1].handle, args, 1);
    SetFocus(currentOption[n-1].handle, shells[0], NULL, False);
    SetColor(buf, currentOption[n].handle);
}

void DefColor(int n)
{
    SetColorText(n, (char*) currentOption[n].choice);
}

void RefreshColor(int source, int n)
{
    int col, j, r, g, b, step = 10;
    char *s, buf[MSG_SIZ]; // color string
    Arg args[5];
    XtSetArg(args[0], XtNstring, &s);
    XtGetValues(currentOption[source].handle, args, 1);
    if(sscanf(s, "#%x", &col) != 1) return;   // malformed
    b = col & 0xFF; g = col & 0xFF00; r = col & 0xFF0000;
    switch(n) {
	case 1: r += 0x10000*step;break;
	case 2: g += 0x100*step;  break;
	case 3: b += step;        break;
	case 4: r -= 0x10000*step; g -= 0x100*step; b -= step; break;
    }
    if(r < 0) r = 0; if(g < 0) g = 0; if(b < 0) b = 0;
    if(r > 0xFF0000) r = 0xFF0000; if(g > 0xFF00) g = 0xFF00; if(b > 0xFF) b = 0xFF;
    col = r | g | b;
    snprintf(buf, MSG_SIZ, "#%06x", col);
    for(j=1; j<7; j++) if(buf[j] >= 'a') buf[j] -= 32; // capitalize
    SetColorText(source+1, buf);
}

void ColorChanged(Widget w, XtPointer data, XEvent *event, Boolean *b)
{
    char buf[10];
    if ( (XLookupString(&(event->xkey), buf, 2, NULL, NULL) == 1) && *buf == '\r' )
	RefreshColor((int)(intptr_t) data, 0);
}

void AdjustColor(int i)
{
    int n = currentOption[i].value;
    RefreshColor(i-n-1, n);
}

void BoardOptionsOK(int n)
{
    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap; else lineGap = defaultLineGap;
    useImages = useImageSqs = 0;
    MakeColors(); CreateGCs(True);
    CreateAnyPieces();
    InitDrawingSizes(-1, 0);
    DrawPosition(True, NULL);
}

Option boardOptions[] = {
{ 0,   0, 70, NULL, (void*) &appData.whitePieceColor, "", NULL, TextBox, N_("White Piece Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#FFFFCC", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.blackPieceColor, "", NULL, TextBox, N_("Black Piece Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#202020", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.lightSquareColor, "", NULL, TextBox, N_("Light Square Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#C8C365", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.darkSquareColor, "", NULL, TextBox, N_("Dark Square Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#77A26D", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.highlightSquareColor, "", NULL, TextBox, N_("Highlight Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#FFFF00", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.premoveHighlightColor, "", NULL, TextBox, N_("Premove Highlight Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#FF0000", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0, 0, 0, NULL, (void*) &appData.upsideDown, "", NULL, CheckBox, N_("Flip Pieces Shogi Style        (Colored buttons restore default)") },
//{ 0, 0, 0, NULL, (void*) &appData.allWhite, "", NULL, CheckBox, N_("Use Outline Pieces for Black") },
{ 0, 0, 0, NULL, (void*) &appData.monoMode, "", NULL, CheckBox, N_("Mono Mode") },
{ 0,-1, 5, NULL, (void*) &appData.overrideLineGap, "", NULL, Spin, N_("Line Gap ( -1 = default for board size):") },
{ 0, 0, 0, NULL, (void*) &appData.liteBackTextureFile, "", NULL, FileName, N_("Light-Squares Texture File:") },
{ 0, 0, 0, NULL, (void*) &appData.darkBackTextureFile, "", NULL, FileName, N_("Dark-Squares Texture File:") },
{ 0, 0, 0, NULL, (void*) &appData.bitmapDirectory, "", NULL, PathName, N_("Directory with Bitmap Pieces:") },
{ 0, 0, 0, NULL, (void*) &appData.pixmapDirectory, "", NULL, PathName, N_("Directory with Pixmap Pieces:") },
{ 0, 0, 0, NULL, (void*) &BoardOptionsOK, "", NULL, EndMark , "" }
};

void GenericReadout(int selected)
{
    int i, j;
    String val;
    Arg args[16];
    char buf[MSG_SIZ], **dest;
    float x;
	for(i=0; ; i++) { // send all options that had to be OK-ed to engine
	    if(selected >= 0) { if(i < selected) continue; else if(i > selected) break; }
	    switch(currentOption[i].type) {
		case TextBox:
		case FileName:
		case PathName:
		    XtSetArg(args[0], XtNstring, &val);
		    XtGetValues(currentOption[i].handle, args, 1);
		    dest = currentCps ? &(currentOption[i].textValue) : (char**) currentOption[i].target;
		    if(*dest == NULL || strcmp(*dest, val)) {
			if(currentCps) {
			    snprintf(buf, MSG_SIZ,  "option %s=%s\n", currentOption[i].name, val);
			    SendToProgram(buf, currentCps);
			} else *dest = currentOption[i].name + 100; // option gets to point to private storage;
			safeStrCpy(*dest, val, MSG_SIZ - (*dest - currentOption[i].name)); // copy text there
		    }
		    break;
		case Spin:
		case Fractional:
		    XtSetArg(args[0], XtNstring, &val);
		    XtGetValues(currentOption[i].handle, args, 1);
		    sscanf(val, "%f", &x);
		    if(x > currentOption[i].max) x = currentOption[i].max;
		    if(x < currentOption[i].min) x = currentOption[i].min;
		    if(currentOption[i].type == Fractional)
			*(float*) currentOption[i].target = x; // engines never have float options!
		    else if(currentOption[i].value != x) {
			currentOption[i].value = x;
			if(currentCps) {
			    snprintf(buf, MSG_SIZ,  "option %s=%.0f\n", currentOption[i].name, x);
			    SendToProgram(buf, currentCps);
			} else *(int*) currentOption[i].target = x;
		    }
		    break;
		case CheckBox:
		    j = 0;
		    XtSetArg(args[0], XtNstate, &j);
		    XtGetValues(currentOption[i].handle, args, 1);
		    if(currentOption[i].value != j) {
			currentOption[i].value = j;
			if(currentCps) {
			    snprintf(buf, MSG_SIZ,  "option %s=%d\n", currentOption[i].name, j);
			    SendToProgram(buf, currentCps);
			} else *(Boolean*) currentOption[i].target = j;
		    }
		    break;
		case ComboBox:
		    val = ((char**)currentOption[i].choice)[values[i]];
		    if(currentCps) {
			if(currentOption[i].value == values[i]) break; // not changed
			currentOption[i].value = values[i];
			snprintf(buf, MSG_SIZ,  "option %s=%s\n", currentOption[i].name,
				((char**)currentOption[i].textValue)[values[i]]);
			SendToProgram(buf, currentCps);
		    } else if(val && (*(char**) currentOption[i].target == NULL || strcmp(*(char**) currentOption[i].target, val))) {
		      if(*(char**) currentOption[i].target) free(*(char**) currentOption[i].target);
		      *(char**) currentOption[i].target = strdup(val);
		    }
		    break;
		case EndMark:
		    if(currentOption[i].target) // callback for implementing necessary actions on OK (like redraw)
			((ButtonCallback*) currentOption[i].target)(i);
		    break;
	    default:
		printf("GenericReadout: unexpected case in switch.\n");
		case Button:
		case SaveButton:
		case Label:
		case Break:
	      break;
	    }
	    if(currentOption[i].type == EndMark) break;
	}
}

void GenericCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    char buf[MSG_SIZ];
    int data = (intptr_t) client_data;

    currentOption = dialogOptions[data>>16]; data &= 0xFFFF;

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    if (strcmp(name, _("cancel")) == 0) {
        PopDown(data);
        return;
    }
    if (strcmp(name, _("OK")) == 0) { // save buttons imply OK
        GenericReadout(-1);
        PopDown(data);
        return;
    }
    if(currentCps) {
	if(currentOption[data].type == SaveButton) GenericReadout(-1);
	snprintf(buf, MSG_SIZ,  "option %s\n", name);
	SendToProgram(buf, currentCps);
    } else ((ButtonCallback*) currentOption[data].target)(data);
}

static char *oneLiner  = "<Key>Return:	redraw-display()\n";

int
GenericPopUp(Option *option, char *title, int dlgNr)
{
    Arg args[16];
    Widget popup, layout, dialog=NULL, edit=NULL, form,  last, b_ok, b_cancel, leftMargin = NULL, textField = NULL;
    Window root, child;
    int x, y, i, j, height=999, width=1, h, c, w;
    int win_x, win_y, maxWidth, maxTextWidth;
    unsigned int mask;
    char def[MSG_SIZ], *msg;
    static char pane[6] = "paneX";
    Widget texts[100], forelast = NULL, anchor, widest, lastrow = NULL;

    if(shellUp[dlgNr]) return 0; // already up
    if(dlgNr && shells[dlgNr]) {
	XtPopup(shells[dlgNr], XtGrabNone);
	shellUp[dlgNr] = True;
	return 0;
    }

    dialogOptions[dlgNr] = option; // make available to callback
    // post currentOption globally, so Spin and Combo callbacks can already use it
    // WARNING: this kludge does not work for persistent dialogs, so that these cannot have spin or combo controls!
    currentOption = option;

    if(currentCps) { // Settings popup for engine: format through heuristic
	int n = currentCps->nrOptions;
	if(!n) { DisplayNote(_("Engine has no options")); currentCps = NULL; return 0; }
	if(n > 50) width = 4; else if(n>24) width = 2; else width = 1;
	height = n / width + 1;
	if(n && currentOption[n-1].type == Button || currentOption[n-1].type == SaveButton) currentOption[n].min = 1; // OK on same line
	currentOption[n].type = EndMark; currentOption[n].target = NULL; // delimit list by callback-less end mark
    }
     i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    popup = shells[dlgNr] =
      XtCreatePopupShell(title, transientShellWidgetClass,
			 shellWidget, args, i);

    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
  for(c=0; c<width; c++) {
    pane[4] = 'A'+c;
    form =
      XtCreateManagedWidget(pane, formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    j=0;
    XtSetArg(args[j], XtNfromHoriz, leftMargin);  j++;
    XtSetValues(form, args, j);
    leftMargin = form;

    last = widest = NULL; anchor = lastrow;
    for(h=0; h<height; h++) {
	i = h + c*height;
	if(option[i].type == EndMark) break;
	lastrow = forelast;
	forelast = last;
	switch(option[i].type) {
	  case Fractional:
	    snprintf(def, MSG_SIZ,  "%.2f", *(float*)option[i].target);
	    option[i].value = *(float*)option[i].target;
	    goto tBox;
	  case Spin:
	    if(!currentCps) option[i].value = *(int*)option[i].target;
	    snprintf(def, MSG_SIZ,  "%d", option[i].value);
	  case TextBox:
	  case FileName:
	  case PathName:
          tBox:
	    if(option[i].name[0]) {
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNright, XtChainLeft); j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    texts[h] =
	    dialog = XtCreateManagedWidget(option[i].name, labelWidgetClass, form, args, j);
	    } else texts[h] = dialog = NULL;
	    w = option[i].type == Spin || option[i].type == Fractional ? 70 : option[i].max ? option[i].max : 205;
	    if(option[i].type == FileName || option[i].type == PathName) w -= 55;
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNfromHoriz, dialog);  j++;
	    XtSetArg(args[j], XtNborderWidth, 1); j++;
	    XtSetArg(args[j], XtNwidth, w); j++;
	    if(option[i].type == TextBox && option[i].min) {
		XtSetArg(args[j], XtNheight, option[i].min); j++;
		if(option[i].value & 1) { XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++; }
		if(option[i].value & 2) { XtSetArg(args[j], XtNscrollHorizontal, XawtextScrollAlways);  j++; }
		if(option[i].value & 4) { XtSetArg(args[j], XtNautoFill, True);  j++; }
		if(option[i].value & 8) { XtSetArg(args[j], XtNwrap, XawtextWrapWord); j++; }
	    }
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
	    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
	    XtSetArg(args[j], XtNright, XtChainRight);  j++;
	    XtSetArg(args[j], XtNresizable, True);  j++;
	    XtSetArg(args[j], XtNinsertPosition, 9999);  j++;
	    XtSetArg(args[j], XtNstring, option[i].type==Spin || option[i].type==Fractional ? def : 
				currentCps ? option[i].textValue : *(char**)option[i].target);  j++;
	    edit = last;
	    option[i].handle = (void*)
		(textField = last = XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j));
	    XtAddEventHandler(last, ButtonPressMask, False, SetFocus, (XtPointer) popup);
	    if(option[i].min == 0 || option[i].type != TextBox)
		XtOverrideTranslations(last, XtParseTranslationTable(oneLiner));

	    if(option[i].type == TextBox || option[i].type == Fractional) break;

	    // add increment and decrement controls for spin
	    j=0;
	    XtSetArg(args[j], XtNfromVert, edit);  j++;
	    XtSetArg(args[j], XtNfromHoriz, last);  j++;
	    XtSetArg(args[j], XtNleft, XtChainRight); j++;
	    XtSetArg(args[j], XtNright, XtChainRight); j++;
	    if(option[i].type == FileName || option[i].type == PathName) {
		w = 50; msg = "browse";
	    } else {
		XtSetArg(args[j], XtNheight, 10);  j++;
		w = 20; msg = "+";
	    }
	    XtSetArg(args[j], XtNwidth, w);  j++;
	    edit = XtCreateManagedWidget(msg, commandWidgetClass, form, args, j);
	    XtAddCallback(edit, XtNcallback, SpinCallback, (XtPointer)(intptr_t) i);

	    if(option[i].type != Spin) break;

	    j=0;
	    XtSetArg(args[j], XtNfromVert, edit);  j++;
	    XtSetArg(args[j], XtNfromHoriz, last);  j++;
	    XtSetArg(args[j], XtNheight, 10);  j++;
	    XtSetArg(args[j], XtNwidth, 20);  j++;
	    XtSetArg(args[j], XtNleft, XtChainRight); j++;
	    XtSetArg(args[j], XtNright, XtChainRight); j++;
	    last = XtCreateManagedWidget("-", commandWidgetClass, form, args, j);
	    XtAddCallback(last, XtNcallback, SpinCallback, (XtPointer)(intptr_t) i);
	    break;
	  case CheckBox:
	    if(!currentCps) option[i].value = *(Boolean*)option[i].target;
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNwidth, 10);  j++;
	    XtSetArg(args[j], XtNheight, 10);  j++;
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNright, XtChainLeft); j++;
	    XtSetArg(args[j], XtNstate, option[i].value);  j++;
	    option[i].handle = (void*)
		(dialog = XtCreateManagedWidget(" ", toggleWidgetClass, form, args, j));
	  case Label:
	    msg = option[i].name;
	    if(*msg == NULLCHAR) msg = option[i].textValue;
	    if(!msg) break;
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNfromHoriz, option[i].type != Label ? dialog : NULL);  j++;
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    last = XtCreateManagedWidget(msg, labelWidgetClass, form, args, j);
	    if(option[i].type == CheckBox)
		XtAddEventHandler(last, ButtonPressMask, False, CheckCallback, (XtPointer)(intptr_t) i);
	    break;
	  case SaveButton:
	  case Button:
	    j=0;
	    XtSetArg(args[j], XtNfromVert, option[i].min & 1 ? lastrow : last);  j++;
	    if(option[i].min & 1) { XtSetArg(args[j], XtNfromHoriz, last);  j++; }
	    else  { XtSetArg(args[j], XtNfromHoriz, NULL);  j++; lastrow = forelast; }
	    if(option[i].max) { XtSetArg(args[j], XtNwidth, option[i].max);  j++; }
	    if(option[i].textValue) { // special for buttons of New Variant dialog
		XtSetArg(args[j], XtNsensitive, appData.noChessProgram || option[i].value < 0
					 || strstr(first.variants, VariantName(option[i].value))); j++;
		XtSetArg(args[j], XtNborderWidth, (gameInfo.variant == option[i].value)+1); j++;
	    }
	    option[i].handle = (void*)
		(dialog = last = XtCreateManagedWidget(option[i].name, commandWidgetClass, form, args, j));
	    if(option[i].choice && ((char*)option[i].choice)[0] == '#' && !currentCps) {
		SetColor( *(char**) option[i-1].target, last);
		XtAddEventHandler(option[i-1].handle, KeyReleaseMask, False, ColorChanged, (XtPointer)(intptr_t) i-1);
	    }
	    XtAddCallback(last, XtNcallback, GenericCallback,
			  (XtPointer)(intptr_t) i + (dlgNr<<16));
	    if(option[i].textValue) SetColor( option[i].textValue, last);
	    forelast = lastrow; // next button can go on same row
	    break;
	  case ComboBox:
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNright, XtChainLeft); j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    texts[h] = dialog = XtCreateManagedWidget(option[i].name, labelWidgetClass, form, args, j);

	    if(currentCps) option[i].choice = (char**) option[i].textValue; else {
	      for(j=0; option[i].choice[j]; j++)
		if(*(char**)option[i].target && !strcmp(*(char**)option[i].target, option[i].choice[j])) break;
	      option[i].value = j + (option[i].choice[j] == NULL);
	    }

	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNfromHoriz, dialog);  j++;
	    XtSetArg(args[j], XtNwidth, option[i].max && !currentCps ? option[i].max : 100);  j++;
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNmenuName, XtNewString(option[i].name));  j++;
	    XtSetArg(args[j], XtNlabel, ((char**)option[i].textValue)[option[i].value]);  j++;
	    option[i].handle = (void*)
		(last = XtCreateManagedWidget(" ", menuButtonWidgetClass, form, args, j));
	    CreateComboPopup(last, option[i].name, i, (char **) option[i].textValue);
	    values[i] = option[i].value;
	    break;
	  case Break:
	    width++;
	    height = i+1;
	    break;
	default:
	    printf("GenericPopUp: unexpected case in switch.\n");
	    break;
	}
    }

    // make an attempt to align all spins and textbox controls
    maxWidth = maxTextWidth = 0;
    for(h=0; h<height; h++) {
	i = h + c*height;
	if(option[i].type == EndMark) break;
	if(option[i].type == Spin || option[i].type == TextBox || option[i].type == ComboBox
				  || option[i].type == PathName || option[i].type == FileName) {
	    Dimension w;
	    if(!texts[h]) continue;
	    j=0;
	    XtSetArg(args[j], XtNwidth, &w);  j++;
	    XtGetValues(texts[h], args, j);
	    if(option[i].type == Spin) {
		if(w > maxWidth) maxWidth = w;
		widest = texts[h];
	    } else {
		if(w > maxTextWidth) maxTextWidth = w;
		if(!widest) widest = texts[h];
	    }
	}
    }
    if(maxTextWidth + 110 < maxWidth)
	 maxTextWidth = maxWidth - 110;
    else maxWidth = maxTextWidth + 110;
    for(h=0; h<height; h++) {
	i = h + c*height;
	if(option[i].type == EndMark) break;
	if(!texts[h]) continue;
	j=0;
	if(option[i].type == Spin) {
	    XtSetArg(args[j], XtNwidth, maxWidth);  j++;
	    XtSetValues(texts[h], args, j);
	} else
	if(option[i].type == TextBox || option[i].type == ComboBox || option[i].type == PathName || option[i].type == FileName) {
	    XtSetArg(args[j], XtNwidth, maxTextWidth);  j++;
	    XtSetValues(texts[h], args, j);
	}
    }
  }

  if(!(option[i].min & 2)) {
    j=0;
    if(option[i].min & 1) { XtSetArg(args[j], XtNfromHoriz, last); last = forelast; } else
    XtSetArg(args[j], XtNfromHoriz, widest ? widest : dialog);  j++;
    XtSetArg(args[j], XtNfromVert, anchor ? anchor : last);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    b_ok = XtCreateManagedWidget(_("OK"), commandWidgetClass, form, args, j);
    XtAddCallback(b_ok, XtNcallback, GenericCallback, (XtPointer)(intptr_t) dlgNr + (dlgNr<<16));

    XtSetArg(args[0], XtNfromHoriz, b_ok);
    b_cancel = XtCreateManagedWidget(_("cancel"), commandWidgetClass, form, args, j);
    XtAddCallback(b_cancel, XtNcallback, GenericCallback, (XtPointer)(intptr_t) dlgNr);
  }

    XtRealizeWidget(popup);
    XSetWMProtocols(xDisplay, XtWindow(popup), &wm_delete_window, 1);
    snprintf(def, MSG_SIZ, "<Message>WM_PROTOCOLS: GenericPopDown(\"%d\") \n", dlgNr);
    XtAugmentTranslations(popup, XtParseTranslationTable(def));
    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);

    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);

    XtPopup(popup, dlgNr ? XtGrabNone : XtGrabExclusive);
    shellUp[dlgNr] = True;
    previous = NULL;
    if(textField)SetFocus(textField, popup, (XEvent*) NULL, False);
    if(dlgNr && wp[dlgNr] && wp[dlgNr]->width > 0) { // if persistent window-info available, reposition
	j = 0;
	XtSetArg(args[j], XtNheight, (Dimension) (wp[dlgNr]->height));  j++;
	XtSetArg(args[j], XtNwidth,  (Dimension) (wp[dlgNr]->width));  j++;
	XtSetArg(args[j], XtNx, (Position) (wp[dlgNr]->x));  j++;
	XtSetArg(args[j], XtNy, (Position) (wp[dlgNr]->y));  j++;
	XtSetValues(popup, args, j);
    }
    return 1;
}


void IcsOptionsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   GenericPopUp(icsOptions, _("ICS Options"), 0);
}

void LoadOptionsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   GenericPopUp(loadOptions, _("Load Game Options"), 0);
}

void SaveOptionsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   GenericPopUp(saveOptions, _("Save Game Options"), 0);
}

void SoundOptionsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   soundFiles[2] = "*";
   GenericPopUp(soundOptions, _("Sound Options"), 0);
}

void BoardOptionsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   GenericPopUp(boardOptions, _("Board Options"), 0);
}

void EngineMenuProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   GenericPopUp(adjudicationOptions, "Adjudicate non-ICS Games", 0);
}

void UciMenuProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   oldCores = appData.smpCores;
   oldPonder = appData.ponderNextMove;
   GenericPopUp(commonEngineOptions, _("Common Engine Settings"), 0);
}

void NewVariantProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   GenericPopUp(variantDescriptors, _("New Variant"), 0);
}

void OptionsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   oldPonder = appData.ponderNextMove;
   GenericPopUp(generalOptions, _("General Options"), 0);
}

void MatchOptionsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   GenericPopUp(matchOptions, _("Match Options"), 0);
}

Option textOptions[100];
void PutText P((char *text, int pos));

void SendString(char *p)
{
    char buf[MSG_SIZ], *q;
    if(q = strstr(p, "$input")) {
	if(!shellUp[4]) return;
	strncpy(buf, p, MSG_SIZ);
	strncpy(buf + (q-p), q+6, MSG_SIZ-(q-p));
	PutText(buf, q-p);
	return;
    }
    snprintf(buf, MSG_SIZ, "%s\n", p);
    SendToICS(buf);
}

/* function called when the data to Paste is ready */
static void
SendTextCB(Widget w, XtPointer client_data, Atom *selection,
	   Atom *type, XtPointer value, unsigned long *len, int *format)
{
  char buf[MSG_SIZ], *p = (char*) textOptions[(int)(intptr_t) client_data].choice, *name = (char*) value, *q;
  if (value==NULL || *len==0) return; /* nothing selected, abort */
  name[*len]='\0';
  strncpy(buf, p, MSG_SIZ);
  q = strstr(p, "$name");
  snprintf(buf + (q-p), MSG_SIZ -(q-p), "%s%s", name, q+5);
  SendString(buf);
  XtFree(value);
}

void SendText(int n)
{
    char *p = (char*) textOptions[n].choice;
    if(strstr(p, "$name")) {
	XtGetSelectionValue(menuBarWidget,
	  XA_PRIMARY, XA_STRING,
	  /* (XtSelectionCallbackProc) */ SendTextCB,
	  (XtPointer) (intptr_t) n, /* client_data passed to PastePositionCB */
	  CurrentTime
	);
    } else SendString(p);
}

void IcsTextProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
   int i=0, j;
   char *p, *q, *r;
   if((p = icsTextMenuString) == NULL) return;
   do {
	q = r = p; while(*p && *p != ';') p++;
	for(j=0; j<p-q; j++) textOptions[i].name[j] = *r++;
	textOptions[i].name[j++] = 0;
	if(!*p) break;
	if(*++p == '\n') p++; // optional linefeed after button-text terminating semicolon
	q = p;
	textOptions[i].choice = (char**) (r = textOptions[i].name + j);
	while(*p && (*p != ';' || p[1] != '\n')) textOptions[i].name[j++] = *p++;
	textOptions[i].name[j++] = 0;
	if(*p) p += 2;
	textOptions[i].max = 135;
	textOptions[i].min = i&1;
	textOptions[i].handle = NULL;
	textOptions[i].target = &SendText;
	textOptions[i].textValue = strstr(r, "$input") ? "#80FF80" : strstr(r, "$name") ? "#FF8080" : "#FFFFFF";
	textOptions[i].type = Button;
   } while(++i < 99 && *p);
   if(i == 0) return;
   textOptions[i].type = EndMark;
   textOptions[i].target = NULL;
   textOptions[i].min = 2;
   MarkMenu("menuView.ICStex", 3);
   GenericPopUp(textOptions, _("ICS text menu"), 3);
}

char *icsText;

Option boxOptions[] = {
{   0, 30,  400, NULL, (void*) &icsText, "", NULL, TextBox, "" },
{   0,  3,    0, NULL, NULL, "", NULL, EndMark , "" }
};

void PutText(char *text, int pos)
{
    Arg args[16];
    char buf[MSG_SIZ], *p;

    if(strstr(text, "$add ") == text) {
	XtSetArg(args[0], XtNstring, &p);
	XtGetValues(boxOptions[0].handle, args, 1);
	snprintf(buf, MSG_SIZ, "%s%s", p, text+5); text = buf;
	pos += strlen(p) - 5;
    }
    XtSetArg(args[0], XtNstring, text);
    XtSetValues(boxOptions[0].handle, args, 1);
    XtSetArg(args[0], XtNinsertPosition, pos);
    XtSetValues(boxOptions[0].handle, args, 1);
//    SetFocus(boxOptions[0].handle, shells[4], NULL, False); // No idea why this does not work, and the following is needed:
    XSetInputFocus(xDisplay, XtWindow(boxOptions[0].handle), RevertToPointerRoot, CurrentTime);
}

void InputBoxPopup()
{
    MarkMenu("menuView.ICS Input Box", 4);
    if(GenericPopUp(boxOptions, _("ICS input box"), 4))
	XtOverrideTranslations(boxOptions[0].handle, XtParseTranslationTable(ICSInputTranslations));
}

void
SettingsPopUp(ChessProgramState *cps)
{
   currentCps = cps;
   GenericPopUp(cps->option, _("Engine Settings"), 0);
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
   if(WaitForSecond(SettingsMenuIfReady)) return;
   SettingsPopUp(&second);
}

//---------------------------- Chat Windows ----------------------------------------------

void OutputChatMessage(int partner, char *mess)
{
    return; // dummy
}

