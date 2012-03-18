/*
 * xoptions.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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
#include "dialogs.h"
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
static Option *currentOption;

void
SetFocus (Widget w, XtPointer data, XEvent *event, Boolean *b)
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
    if(!strchr(s, '\n') && strlen(s) < 80) XtSetArg(args[1], XtNinsertPosition, strlen(s)), j++;
    XtSetValues(w, args, j);
    XtSetKeyboardFocus((Widget) data, w);
    previous = w;
}

//--------------------------- Engine-specific options menu ----------------------------------

int dialogError;
static Boolean browserUp;

void
GetWidgetText (Option *opt, char **buf)
{
    Arg arg;
    XtSetArg(arg, XtNstring, buf);
    XtGetValues(opt->handle, &arg, 1);
}

void
SetWidgetText (Option *opt, char *buf, int n)
{
    Arg arg;
    XtSetArg(arg, XtNstring, buf);
    XtSetValues(opt->handle, &arg, 1);
    if(n >= 0) SetFocus(opt->handle, shells[n], NULL, False);
}

void
GetWidgetState (Option *opt, int *state)
{
    Arg arg;
    XtSetArg(arg, XtNstate, state);
    XtGetValues(opt->handle, &arg, 1);
}

void
SetWidgetState (Option *opt, int state)
{
    Arg arg;
    XtSetArg(arg, XtNstate, state);
    XtSetValues(opt->handle, &arg, 1);
}

void
SetDialogTitle (DialogClass dlg, char *title)
{
    Arg args[16];
    XtSetArg(args[0], XtNtitle, title);
    XtSetValues(shells[dlg], args, 1);
}

static void
CheckCallback (Widget ww, XtPointer data, XEvent *event, Boolean *b)
{
    Widget w = currentOption[(int)(intptr_t)data].handle;
    Boolean s;
    Arg args[16];

    XtSetArg(args[0], XtNstate, &s);
    XtGetValues(w, args, 1);
    SetWidgetState(&currentOption[(int)(intptr_t)data], !s);
}

static void
SpinCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    String name, val;
    Arg args[16];
    char buf[MSG_SIZ], *p;
    int j = 0; // Initialiasation is necessary because the text value may be non-numeric causing the scanf conversion to fail
    int data = (intptr_t) client_data;

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    GetWidgetText(&currentOption[data], &val);
    sscanf(val, "%d", &j);
    if (strcmp(name, _("browse")) == 0) {
	char *q=val, *r;
	for(r = ""; *q; q++) if(*q == '.') r = q; else if(*q == '/') r = ""; // last dot after last slash
	if(!strcmp(r, "") && !currentCps && currentOption[data].type == FileName && currentOption[data].textValue)
		r = currentOption[data].textValue;
	browserUp = True;
	if(XsraSelFile(shells[TransientDlg], currentOption[data].name, NULL, NULL, "", "", r,
			 	  currentOption[data].type == PathName ? "p" : "f", NULL, &p)) {
		int len = strlen(p);
		if(len && p[len-1] == '/') p[len-1] = NULLCHAR;
		XtSetArg(args[0], XtNstring, p);
		XtSetValues(currentOption[data].handle, args, 1);
	}
	browserUp = False;
	SetFocus(currentOption[data].handle, shells[TransientDlg], (XEvent*) NULL, False);
	return;
    } else
    if (strcmp(name, "+") == 0) {
	if(++j > currentOption[data].max) return;
    } else
    if (strcmp(name, "-") == 0) {
	if(--j < currentOption[data].min) return;
    } else return;
    snprintf(buf, MSG_SIZ,  "%d", j);
    SetWidgetText(&currentOption[data], buf, TransientDlg);
}

static void
ComboSelect (Widget w, caddr_t addr, caddr_t index) // callback for all combo items
{
    Arg args[16];
    int i = ((intptr_t)addr)>>8;
    int j = 255 & (intptr_t) addr;

    values[i] = j; // store in temporary, for transfer at OK

    if(currentOption[i].min & NO_GETTEXT)
      XtSetArg(args[0], XtNlabel, ((char**)currentOption[i].textValue)[j]);
    else
      XtSetArg(args[0], XtNlabel, _(((char**)currentOption[i].textValue)[j]));

    XtSetValues(currentOption[i].handle, args, 1);

    if(currentOption[i].min & COMBO_CALLBACK && !currentCps && comboCallback) (comboCallback)(i);
}

static void
CreateComboPopup (Widget parent, Option *option, int n)
{
    int i=0, j;
    Widget menu, entry;
    Arg args[16];
    char **mb = (char **) option->textValue;

    if(mb[0] == NULL) return; // avoid empty menus, as they cause crash
    menu = XtCreatePopupShell(option->name, simpleMenuWidgetClass,
			      parent, NULL, 0);
    j = 0;
    XtSetArg(args[j], XtNwidth, 100);  j++;
    while (mb[i] != NULL) 
      {
	if (option->min & NO_GETTEXT)
	  XtSetArg(args[j], XtNlabel, mb[i]);
	else
	  XtSetArg(args[j], XtNlabel, _(mb[i]));
	entry = XtCreateManagedWidget((String) mb[i], smeBSBObjectClass,
				      menu, args, j+1);
	XtAddCallback(entry, XtNcallback,
		      (XtCallbackProc) ComboSelect,
		      (caddr_t)(intptr_t) (256*n+i));
	i++;
      }
}

char moveTypeInTranslations[] =
    "<Key>Return: TypeInProc(1) \n"
    "<Key>Escape: TypeInProc(0) \n";


char *translationTable[] = {
   historyTranslations, commentTranslations, moveTypeInTranslations, ICSInputTranslations,
};

void
AddHandler (Option *opt, int nr)
{
    XtOverrideTranslations(opt->handle, XtParseTranslationTable(translationTable[nr]));
}

//----------------------------Generic dialog --------------------------------------------

// cloned from Engine Settings dialog (and later merged with it)

Widget shells[NrOfDialogs];
WindowPlacement *wp[NrOfDialogs] = { NULL, &wpComment, &wpTags, NULL, NULL, NULL, NULL, &wpMoveHistory };
static Option *dialogOptions[NrOfDialogs];

int
DialogExists (DialogClass n)
{
    return shells[n] != NULL;
}

int
PopDown (DialogClass n)
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
	MarkMenuItem(marked[n], False);
	marked[n] = NULL;
    }
    if(!n) currentCps = NULL; // if an Engine Settings dialog was up, we must be popping it down now
    return 1;
}

void
GenericPopDown (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    if(browserUp || dialogError) return; // prevent closing dialog when it has an open file-browse daughter
    PopDown(prms[0][0] - '0');
}

int
AppendText (Option *opt, char *s)
{
    XawTextBlock t;
    char *v;
    int len;
    GetWidgetText(opt, &v);
    len = strlen(v);
    t.ptr = s; t.firstPos = 0; t.length = strlen(s); t.format = XawFmt8Bit;
    XawTextReplace(opt->handle, len, len, &t);
    return len;
}

void
SetColor (char *colorName, Option *box)
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
	} else buttonColor = timerBackgroundPixel;
	XtSetArg(args[0], XtNbackground, buttonColor);;
	XtSetValues(box->handle, args, 1);
}

void
ColorChanged (Widget w, XtPointer data, XEvent *event, Boolean *b)
{
    char buf[10];
    if ( (XLookupString(&(event->xkey), buf, 2, NULL, NULL) == 1) && *buf == '\r' )
	RefreshColor((int)(intptr_t) data, 0);
}

void
GenericCallback (Widget w, XtPointer client_data, XtPointer call_data)
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
        if(GenericReadout(currentOption, -1)) PopDown(data);
        return;
    }
    if(currentCps) {
	if(currentOption[data].type == SaveButton) GenericReadout(currentOption, -1);
	snprintf(buf, MSG_SIZ,  "option %s\n", name);
	SendToProgram(buf, currentCps);
    } else ((ButtonCallback*) currentOption[data].target)(data);
}

static char *oneLiner  = "<Key>Return:	redraw-display()\n";

int
GenericPopUp (Option *option, char *title, DialogClass dlgNr)
{
    Arg args[16];
    Widget popup, layout, dialog=NULL, edit=NULL, form,  last, b_ok, b_cancel, leftMargin = NULL, textField = NULL;
    Window root, child;
    int x, y, i, j, height=999, width=1, h, c, w, shrink=FALSE;
    int win_x, win_y, maxWidth, maxTextWidth;
    unsigned int mask;
    char def[MSG_SIZ], *msg;
    static char pane[6] = "paneX";
    Widget texts[100], forelast = NULL, anchor, widest, lastrow = NULL, browse = NULL;
    Dimension bWidth = 50;

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
	if(n && (currentOption[n-1].type == Button || currentOption[n-1].type == SaveButton)) currentOption[n].min = SAME_ROW; // OK on same line
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
	    XtSetArg(args[j], XtNheight, textHeight),  j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    XtSetArg(args[j], XtNlabel, _(option[i].name));  j++;
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
	    } else shrink = TRUE;
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
		msg = _("browse"); w = 0;
		/* automatically scale to width of text */
		XtSetArg(args[j], XtNwidth, (XtArgVal) NULL );  j++;
		if(textHeight) XtSetArg(args[j], XtNheight, textHeight),  j++;
	    } else {
		w = 20; msg = "+";
		XtSetArg(args[j], XtNheight, textHeight/2);  j++;
		XtSetArg(args[j], XtNwidth,   w);  j++;
	    }
	    edit = XtCreateManagedWidget(msg, commandWidgetClass, form, args, j);
	    XtAddCallback(edit, XtNcallback, SpinCallback, (XtPointer)(intptr_t) i);
	    if(w == 0) browse = edit;

	    if(option[i].type != Spin) break;

	    j=0;
	    XtSetArg(args[j], XtNfromVert, edit);  j++;
	    XtSetArg(args[j], XtNfromHoriz, last);  j++;
	    XtSetArg(args[j], XtNvertDistance, -1);  j++;
	    XtSetArg(args[j], XtNheight, textHeight/2);  j++;
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
	    XtSetArg(args[j], XtNvertDistance, (textHeight+2)/4 + 3);  j++;
	    XtSetArg(args[j], XtNwidth, textHeight/2);  j++;
	    XtSetArg(args[j], XtNheight, textHeight/2);  j++;
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
	    if(option[i].type != Label) XtSetArg(args[j], XtNheight, textHeight),  j++, shrink = TRUE;
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    XtSetArg(args[j], XtNlabel, _(msg));  j++;
	    last = XtCreateManagedWidget(msg, labelWidgetClass, form, args, j);
	    if(option[i].type == CheckBox)
		XtAddEventHandler(last, ButtonPressMask, False, CheckCallback, (XtPointer)(intptr_t) i);
	    break;
	  case SaveButton:
	  case Button:
	    j=0;
	    if(option[i].min & SAME_ROW) {
		XtSetArg(args[j], XtNfromVert, lastrow);  j++;
		XtSetArg(args[j], XtNfromHoriz, last);  j++;
		XtSetArg(args[j], XtNleft, XtChainRight); j++;
		XtSetArg(args[j], XtNright, XtChainRight); j++;
		if(shrink) XtSetArg(args[j], XtNheight, textHeight),  j++;
	    } else {
		XtSetArg(args[j], XtNfromVert, last);  j++;
		XtSetArg(args[j], XtNfromHoriz, NULL);  j++; lastrow = forelast;
		shrink = FALSE;
	    }
	    XtSetArg(args[j], XtNlabel, _(option[i].name));  j++;
	    if(option[i].max) { XtSetArg(args[j], XtNwidth, option[i].max);  j++; }
	    if(option[i].textValue) { // special for buttons of New Variant dialog
		XtSetArg(args[j], XtNsensitive, appData.noChessProgram || option[i].value < 0
					 || strstr(first.variants, VariantName(option[i].value))); j++;
		XtSetArg(args[j], XtNborderWidth, (gameInfo.variant == option[i].value)+1); j++;
	    }
	    option[i].handle = (void*)
		(dialog = last = XtCreateManagedWidget(option[i].name, commandWidgetClass, form, args, j));
	    if(option[i].choice && ((char*)option[i].choice)[0] == '#' && !currentCps) {
		SetColor( *(char**) option[i-1].target, &option[i]);
		XtAddEventHandler(option[i-1].handle, KeyReleaseMask, False, ColorChanged, (XtPointer)(intptr_t) i-1);
	    }
	    XtAddCallback(last, XtNcallback, GenericCallback,
			  (XtPointer)(intptr_t) i + (dlgNr<<16));
	    if(option[i].textValue) SetColor( option[i].textValue, &option[i]);
	    forelast = lastrow; // next button can go on same row
	    break;
	  case ComboBox:
	    j=0;
	    XtSetArg(args[j], XtNfromVert, last);  j++;
	    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	    XtSetArg(args[j], XtNright, XtChainLeft); j++;
	    XtSetArg(args[j], XtNheight, textHeight),  j++;
	    XtSetArg(args[j], XtNborderWidth, 0);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    XtSetArg(args[j], XtNlabel, _(option[i].name));  j++;
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
	    XtSetArg(args[j], XtNlabel, _(((char**)option[i].textValue)[option[i].value]));  j++;
	    XtSetArg(args[j], XtNheight, textHeight),  j++;
	    shrink = TRUE;
	    option[i].handle = (void*)
		(last = XtCreateManagedWidget(" ", menuButtonWidgetClass, form, args, j));
	    CreateComboPopup(last, option + i, i);
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
    if(browse != NULL) {
	j=0;
	XtSetArg(args[j], XtNwidth, &bWidth);  j++;
	XtGetValues(browse, args, j);
    }
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
	if(!texts[h]) continue; // Note: texts[h] can be undefined (giving errors in valgrind), but then both if's below will be false.
	j=0;
	if(option[i].type == Spin) {
	    XtSetArg(args[j], XtNwidth, maxWidth);  j++;
	    XtSetValues(texts[h], args, j);
	} else
	if(option[i].type == TextBox || option[i].type == ComboBox || option[i].type == PathName || option[i].type == FileName) {
	    XtSetArg(args[j], XtNwidth, maxTextWidth);  j++;
	    XtSetValues(texts[h], args, j);
	    if(bWidth != 50 && (option[i].type == FileName || option[i].type == PathName)) {
		int tWidth = (option[i].max ? option[i].max : 205) - 5 - bWidth;
		j = 0;
		XtSetArg(args[j], XtNwidth, tWidth);  j++;
		XtSetValues(option[i].handle, args, j);
	    }
	}
    }
  }

  if(!(option[i].min & NO_OK)) {
    j=0;
    if(option[i].min & SAME_ROW) {
	for(j=i-1; option[j+1].min & SAME_ROW && option[j].type == Button; j--) {
	    XtSetArg(args[0], XtNtop, XtChainBottom);
	    XtSetArg(args[1], XtNbottom, XtChainBottom);
	    XtSetValues(option[j].handle, args, 2);
	}
	if(option[j].type == TextBox && option[j].name[0] == NULLCHAR) {
	    XtSetArg(args[0], XtNbottom, XtChainBottom);
	    XtSetValues(option[j].handle, args, 1);
	}
	j = 0;
	XtSetArg(args[j], XtNfromHoriz, last); last = forelast;
    } else shrink = FALSE,
    XtSetArg(args[j], XtNfromHoriz, widest ? widest : dialog);  j++;
    XtSetArg(args[j], XtNfromVert, anchor ? anchor : last);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainRight);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    if(shrink) XtSetArg(args[j], XtNheight, textHeight),  j++;
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


/* function called when the data to Paste is ready */
static void
SendTextCB (Widget w, XtPointer client_data, Atom *selection,
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

void
SendText (int n)
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

void
SetInsertPos (Option *opt, int pos)
{
    Arg args[16];
    XtSetArg(args[0], XtNinsertPosition, pos);
    XtSetValues(opt->handle, args, 1);
//    SetFocus(opt->handle, shells[InputBoxDlg], NULL, False); // No idea why this does not work, and the following is needed:
    XSetInputFocus(xDisplay, XtWindow(opt->handle), RevertToPointerRoot, CurrentTime);
}

void
TypeInProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    char *val;

    if(prms[0][0] == '1') {
	GetWidgetText(&boxOptions[0], &val);
	TypeInDoneEvent(val);
    }
    PopDown(TransientDlg);
}

void
HardSetFocus (Option *opt)
{
    XSetInputFocus(xDisplay, XtWindow(opt->handle), RevertToPointerRoot, CurrentTime);
}


