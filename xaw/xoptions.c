/*
 * xoptions.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
#include <X11/Xaw/Scrollbar.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include "common.h"
#include "backend.h"
#include "xboard.h"
#include "dialogs.h"
#include "menus.h"
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
UnCaret ()
{
    Arg args[2];

    if(previous) {
	XtSetArg(args[0], XtNdisplayCaret, False);
	XtSetValues(previous, args, 1);
    }
    previous = NULL;
}

void
SetFocus (Widget w, XtPointer data, XEvent *event, Boolean *b)
{
    Arg args[2];
    char *s;
    int j;

    UnCaret();
    XtSetArg(args[0], XtNstring, &s);
    XtGetValues(w, args, 1);
    j = 1;
    XtSetArg(args[0], XtNdisplayCaret, True);
    if(!strchr(s, '\n') && strlen(s) < 80) XtSetArg(args[1], XtNinsertPosition, strlen(s)), j++;
    XtSetValues(w, args, j);
    XtSetKeyboardFocus((Widget) data, w);
    previous = w;
}

void
BoardFocus ()
{
    XtSetKeyboardFocus(shellWidget, formWidget);
}

//--------------------------- Engine-specific options menu ----------------------------------

int dialogError;
Option *dialogOptions[NrOfDialogs];

static Arg layoutArgs[] = {
    { XtNborderWidth, 0 },
    { XtNdefaultDistance, 0 },
};

static Arg formArgs[] = {
    { XtNborderWidth, 0 },
    { XtNresizable, (XtArgVal) True },
};

void
CursorAtEnd (Option *opt)
{
}

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
SetWidgetLabel (Option *opt, char *buf)
{
    Arg arg;
    XtSetArg(arg, XtNlabel, (XtArgVal) buf);
    XtSetValues(opt->handle, &arg, 1);
}

void
SetDialogTitle (DialogClass dlg, char *title)
{
    Arg args[16];
    XtSetArg(args[0], XtNtitle, title);
    XtSetValues(shells[dlg], args, 1);
}

void
LoadListBox (Option *opt, char *emptyText, int n1, int n2)
{
    static char *dummyList[2];
    dummyList[0] = emptyText; // empty listboxes tend to crash X, so display user-supplied warning string instead
    XawListChange(opt->handle, *(char**)opt->target ? opt->target : dummyList, 0, 0, True);
//printf("listbox data = %x\n", opt->target);
}

int
ReadScroll (Option *opt, float *top, float *bottom)
{   // retreives fractions of top and bottom of thumb
    Arg args[16];
    Widget w = XtParent(opt->handle); // viewport
    Widget v = XtNameToWidget(w, "vertical");
    int j=0;
    float h;
    if(!v) return FALSE; // no scroll bar
    XtSetArg(args[j], XtNshown, &h); j++;
    XtSetArg(args[j], XtNtopOfThumb, top); j++;
    XtGetValues(v, args, j);
    *bottom = *top + h;
    return TRUE;
}

void
SetScroll (Option *opt, float f)
{   // sets top of thumb to given fraction
    static char *params[3] = { "", "Continuous", "Proportional" };
    static XEvent event;
    Widget w = XtParent(opt->handle); // viewport
    Widget v = XtNameToWidget(w, "vertical");
    if(!v) return; // no scroll bar
    XtCallActionProc(v, "StartScroll", &event, params+1, 1);
    XawScrollbarSetThumb(v, f, -1.0);
    XtCallActionProc(v, "NotifyThumb", &event, params, 0);
//    XtCallActionProc(v, "NotifyScroll", &event, params+2, 1);
    XtCallActionProc(v, "EndScroll", &event, params, 0);
}

void
HighlightListBoxItem (Option *opt, int nr)
{
    XawListHighlight(opt->handle, nr);
}

void
HighlightWithScroll (Option *opt, int sel, int max)
{
    float top, bottom, f, g;
    HighlightListBoxItem(opt, sel);
    if(!ReadScroll(opt, &top, &bottom)) return; // no scroll bar
    bottom = bottom*max - 1.f;
    f = g = top;
    top *= max;
    if(sel > (top + 3*bottom)/4) f = (sel - 0.75f*(bottom-top))/max; else
    if(sel < (3*top + bottom)/4) f = (sel - 0.25f*(bottom-top))/max;
    if(f < 0.f) f = 0.; if(f + 1.f/max > 1.f) f = 1. - 1./max;
    if(f != g) SetScroll(opt, f);
}

int
SelectedListBoxItem (Option *opt)
{
    XawListReturnStruct *rs;
    rs = XawListShowCurrent(opt->handle);
    return rs->list_index;
}

void
HighlightText (Option *opt, int start, int end, Boolean on)
{
    if(on)
	XawTextSetSelection( opt->handle, start, end ); // for lack of a better method, use selection for highighting
    else
	XawTextSetSelection( opt->handle, 0, 0 );
}

void
FocusOnWidget (Option *opt, DialogClass dlg)
{
    UnCaret();
    XtSetKeyboardFocus(shells[dlg], opt->handle);
}

void
SetIconName (DialogClass dlg, char *name)
{
	Arg args[16];
	int j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) name);  j++;
//	XtSetArg(args[j], XtNtitle, (XtArgVal) name);  j++;
	XtSetValues(shells[dlg], args, j);
}

static void
CheckCallback (Widget ww, XtPointer client_data, XEvent *event, Boolean *b)
{
    int s, data = (intptr_t) client_data;
    Option *opt = dialogOptions[data >> 8] + (data & 255);

    if(opt->type == Label) { ((ButtonCallback*) opt->target)(data&255); return; }

    GetWidgetState(opt, &s);
    SetWidgetState(opt, !s);
}

static void
SpinCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    String name, val;
    Arg args[16];
    char buf[MSG_SIZ], *p;
    int j = 0; // Initialisation is necessary because the text value may be non-numeric causing the scanf conversion to fail
    int data = (intptr_t) client_data;
    Option *opt = dialogOptions[data >> 8] + (data & 255);

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    GetWidgetText(opt, &val);
    sscanf(val, "%d", &j);
    if (strcmp(name, _("browse")) == 0) {
	char *q=val, *r;
	for(r = ""; *q; q++) if(*q == '.') r = q; else if(*q == '/') r = ""; // last dot after last slash
	if(!strcmp(r, "") && !currentCps && opt->type == FileName && opt->textValue)
		r = opt->textValue;
	Browse(data>>8, opt->name, NULL, r, opt->type == PathName, "", &p, (FILE**) opt);
	return;
    } else
    if (strcmp(name, "+") == 0) {
	if(++j > opt->max) return;
    } else
    if (strcmp(name, "-") == 0) {
	if(--j < opt->min) return;
    } else return;
    snprintf(buf, MSG_SIZ,  "%d", j);
    SetWidgetText(opt, buf, TransientDlg);
}

static void
ComboSelect (Widget w, caddr_t addr, caddr_t index) // callback for all combo items
{
    Arg args[16];
    Option *opt = dialogOptions[((intptr_t)addr)>>24]; // applicable option list
    int i = ((intptr_t)addr)>>16 & 255; // option number
    int j = 0xFFFF & (intptr_t) addr;

    values[i] = j; // store selected value in Option struct, for retrieval at OK

    if(opt[i].type == Graph || opt[i].min & COMBO_CALLBACK && (!currentCps || shellUp[BrowserDlg])) {
	((ButtonCallback*) opt[i].target)(i);
	return;
    }

    if(opt[i].min & NO_GETTEXT)
      XtSetArg(args[0], XtNlabel, ((char**)opt[i].choice)[j]);
    else
      XtSetArg(args[0], XtNlabel, _(((char**)opt[i].choice)[j]));

    XtSetValues(opt[i].handle, args, 1);
}

Widget
CreateMenuItem (Widget menu, char *msg, XtCallbackProc CB, int n)
{
    int j=0;
    Widget entry;
    Arg args[16];
    XtSetArg(args[j], XtNleftMargin, 20);   j++;
    XtSetArg(args[j], XtNrightMargin, 20);  j++;
    if(!strcmp(msg, "----")) { XtCreateManagedWidget(msg, smeLineObjectClass, menu, args, j); return NULL; }
    XtSetArg(args[j], XtNlabel, msg);
    entry = XtCreateManagedWidget("item", smeBSBObjectClass, menu, args, j+1);
    XtAddCallback(entry, XtNcallback, CB, (caddr_t)(intptr_t) n);
    return entry;
}

char *
format_accel (char *input)
{
  char *output;
  char *key,*test;

  output = strdup("");

  if( strstr(input, "<Ctrl>") )
    {
      output = realloc(output, strlen(output) + strlen(_("Ctrl"))+2);
      strncat(output, _("Ctrl"), strlen(_("Ctrl")) +1);
      strncat(output, "+", 1);
    };
  if( strstr(input, "<Alt>") )
    {
      output = realloc(output, strlen(output) + strlen(_("Alt"))+2);
      strncat(output, _("Alt"), strlen(_("Alt")) +1);
      strncat(output, "+", 1);
    };
  if( strstr(input, "<Shift>") )
    {
      output = realloc(output, strlen(output) + strlen(_("Shift"))+2);
      strncat(output, _("Shift"), strlen(_("Shift")) +1);
      strncat(output, "+", 1);
    };

  test = strrchr(input, '>');
  if ( test==NULL )
    key = strdup(input);
  else
    key = strdup(++test); // remove ">"
  if(strlen(key) == 1) key[0] = ToUpper(key[0]);

  output = realloc(output, strlen(output) + strlen(_(key))+2);
  strncat(output, _(key), strlen(_(key)) +1);

  free(key);
  return output;
}

int
pixlen (char *s)
{
#if 0
    int dummy;
    XCharStruct overall;
    XTextExtents(messageFontStruct, s, strlen(s), &dummy, &dummy, &dummy, &overall);
    return overall.width;
#else
    float tot = 0;
    while(*s) switch(*s++) {
	case '.': tot += 0.45; break;
	case ' ': tot += 0.55; break;
	case 'i': tot += 0.45; break;
	case 'l': tot += 0.45; break;
	case 'j': tot += 0.45; break;
	case 'f': tot += 0.45; break;
	case 'I': tot += 0.45; break;
	case 't': tot += 0.45; break;
	case 'k': tot += 0.83; break;
	case 's': tot += 0.83; break;
	case 'x': tot += 0.83; break;
	case 'z': tot += 0.83; break;
	case 'r': tot += 0.55; break;
	case 'w': tot += 1.3; break;
	case 'm': tot += 1.3; break;
	case 'A': tot += 1.3; break;
	case 'C': tot += 1.3; break;
	case 'D': tot += 1.3; break;
	case 'G': tot += 1.3; break;
	case 'H': tot += 1.3; break;
	case 'N': tot += 1.3; break;
	case 'V': tot += 1.3; break;
	case 'X': tot += 1.3; break;
	case 'Y': tot += 1.3; break;
	case 'Z': tot += 1.3; break;
	case 'M': tot += 1.6; break;
	case 'W': tot += 1.6; break;
	case 'B': tot += 1.1; break;
	case 'E': tot += 1.1; break;
	case 'F': tot += 1.1; break;
	case 'K': tot += 1.1; break;
	case 'P': tot += 1.1; break;
	case 'R': tot += 1.1; break;
	case 'S': tot += 1.1; break;
	case 'O': tot += 1.4; break;
	case 'Q': tot += 1.4; break;
	default:  tot++;
    }
    return tot;
#endif
}

static Widget
CreateComboPopup (Widget parent, Option *opt, int n, int fromList, int def)
{   // fromList determines if the item texts are taken from a list of strings, or from a menu table
    int i;
    Widget menu, entry;
    Arg arg;
    MenuItem *mb = (MenuItem *) opt->choice;
    char **list = (char **) opt->choice;
    int maxlength=0, menuLen[1000];


    if(list[0] == NULL) return NULL; // avoid empty menus, as they cause crash
    menu = XtCreatePopupShell(opt->name, simpleMenuWidgetClass, parent, NULL, 0);

    if(!fromList)
      for (i=0; mb[i].string; i++) if(mb[i].accel) {
	int len = pixlen(_(mb[i].string));
	menuLen[i] = len;
	if (maxlength < len )
	  maxlength = len;
      }

    for (i=0; 1; i++)
      {
	char *msg = fromList ? list[i] : mb[i].string;
	char *label=NULL;

	if(!msg) break;

	if(!fromList && mb[i].accel)
	  {
	    char *menuname = opt->min & NO_GETTEXT ? msg : _(msg);
	    char *accel = format_accel(mb[i].accel);
	    size_t len;
//	    int fill = maxlength - strlen(menuname) +2+strlen(accel);
	    int fill = (maxlength - menuLen[i] + 3)*1.8;

	    len = strlen(menuname)+fill+strlen(accel)+1;
	    label = malloc(len);

	    snprintf(label,len,"%s%*s%s",menuname,fill," ",accel);
	    free(accel);
	  }
	else
	  label = strdup(opt->min & NO_GETTEXT ? msg : _(msg));

	entry = CreateMenuItem(menu, label, (XtCallbackProc) ComboSelect, (n<<16)+i);
	if(!fromList) mb[i].handle = (void*) entry; // save item ID, for enabling / checkmarking
	if(i==def) {
	    XtSetArg(arg, XtNpopupOnEntry, entry);
	    XtSetValues(menu, &arg, 1);
	}
	free(label);
      }
      return menu;
}

char moveTypeInTranslations[] =
    "<Key>Return: TypeInProc(1) \n"
    "<Key>Escape: TypeInProc(0) \n";
extern char filterTranslations[];
extern char gameListTranslations[];
extern char memoTranslations[];


char *translationTable[] = { // beware: order is essential!
   historyTranslations, commentTranslations, moveTypeInTranslations, ICSInputTranslations,
   filterTranslations, gameListTranslations, memoTranslations
};

void
AddHandler (Option *opt, DialogClass dlg, int nr)
{
    XtOverrideTranslations(opt->handle, XtParseTranslationTable(translationTable[nr]));
}

//----------------------------Generic dialog --------------------------------------------

// cloned from Engine Settings dialog (and later merged with it)

Widget shells[NrOfDialogs];
DialogClass parents[NrOfDialogs];
WindowPlacement *wp[NrOfDialogs] = { // Beware! Order must correspond to DialogClass enum
    NULL, &wpComment, &wpTags, NULL, NULL, NULL, NULL, &wpMoveHistory, &wpGameList, &wpEngineOutput, &wpEvalGraph,
    NULL, NULL, NULL, NULL, /*&wpMain*/ NULL
};

int
DialogExists (DialogClass n)
{   // accessor for use in back-end
    return shells[n] != NULL;
}

void
RaiseWindow (DialogClass dlg)
{
    static XEvent xev;
    Window root = RootWindow(xDisplay, DefaultScreen(xDisplay));
    Atom atom = XInternAtom (xDisplay, "_NET_ACTIVE_WINDOW", False);

    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.display = xDisplay;
    xev.xclient.window = XtWindow(shells[dlg]);
    xev.xclient.message_type = atom;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = CurrentTime;

    XSendEvent (xDisplay,
          root, False,
          SubstructureRedirectMask | SubstructureNotifyMask,
          &xev);

    XFlush(xDisplay);
    XSync(xDisplay, False);
}

int
PopDown (DialogClass n)
{   // pops down any dialog created by GenericPopUp (or returns False if it wasn't up), unmarks any associated marked menu
    int j;
    Arg args[10];
    Dimension windowH, windowW; Position windowX, windowY;
    if (!shellUp[n] || !shells[n]) return 0;
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
    shellUp[n]--; // count rather than clear
    if(n == 0 || n >= PromoDlg) XtDestroyWidget(shells[n]), shells[n] = NULL;
    if(marked[n]) {
	MarkMenuItem(marked[n], False);
	marked[n] = NULL;
    }
    if(!n && n != BrowserDlg) currentCps = NULL; // if an Engine Settings dialog was up, we must be popping it down now
    currentOption = dialogOptions[TransientDlg]; // just in case a transient dialog was up (to allow its check and combo callbacks to work)
    RaiseWindow(parents[n]);
    if(parents[n] == BoardWindow) XtSetKeyboardFocus(shellWidget, formWidget);
    return 1;
}

void
GenericPopDown (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // to cause popdown through a translation (Delete Window button!)
    int dlg = atoi(prms[0]);
    Widget sh = shells[dlg];
    if(shellUp[BrowserDlg] && dlg != BrowserDlg || dialogError) return; // prevent closing dialog when it has an open file-browse daughter
    shells[dlg] = w;
    PopDown(dlg);
    shells[dlg] = sh; // restore
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
{       // sets the color of a widget
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
{   // for detecting a typed change in color
    char buf[10];
    if ( (XLookupString(&(event->xkey), buf, 2, NULL, NULL) == 1) && *buf == '\r' )
	RefreshColor((int)(intptr_t) data, 0);
}

static void
GraphEventProc(Widget widget, caddr_t client_data, XEvent *event)
{   // handle expose and mouse events on Graph widget
    Dimension w, h;
    Arg args[16];
    int j, button=10, f=1, sizing=0;
    Option *opt, *graph = (Option *) client_data;
    PointerCallback *userHandler = graph->target;

    if (!XtIsRealized(widget)) return;

    switch(event->type) {
	case Expose: // make handling of expose events generic, just copying from memory buffer (->choice) to display (->textValue)
	    /* Get window size */
	    j = 0;
	    XtSetArg(args[j], XtNwidth, &w); j++;
	    XtSetArg(args[j], XtNheight, &h); j++;
	    XtGetValues(widget, args, j);

	    if(w < graph->max || w > graph->max + 1 || h != graph->value) { // use width fudge of 1 pixel
		if(((XExposeEvent*)event)->count >= 0) { // suppress sizing on expose for ordered redraw in response to sizing.
		    sizing = 1;
		    graph->max = w; graph->value = h; // note: old values are kept if we we don't exceed width fudge
		}
	    } else w = graph->max;

	    if(sizing && ((XExposeEvent*)event)->count > 0) { graph->max = 0; return; } // don't bother if further exposure is pending during resize
	    if(!graph->textValue || sizing) { // create surfaces of new size for display widget
		if(graph->textValue) cairo_surface_destroy((cairo_surface_t *)graph->textValue);
		graph->textValue = (char*) cairo_xlib_surface_create(xDisplay, XtWindow(widget), DefaultVisual(xDisplay, 0), w, h);
	    }
	    if(sizing) { // the memory buffer was already created in GenericPopup(),
			 // to give drawing routines opportunity to use it before first expose event
			 // (which are only processed when main gets to the event loop, so after all init!)
			 // so only change when size is no longer good
		cairo_t *cr;
		if(graph->choice) cairo_surface_destroy((cairo_surface_t *) graph->choice);
		graph->choice = (char**) cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
		// paint white, to prevent weirdness when people maximize window and drag pieces over space next to board
		cr = cairo_create ((cairo_surface_t *) graph->choice);
		cairo_rectangle (cr, 0, 0, w, h);
		cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
		cairo_fill(cr);
		cairo_destroy (cr);
		break;
	    }
	    w = ((XExposeEvent*)event)->width;
	    if(((XExposeEvent*)event)->x + w > graph->max) w--; // cut off fudge pixel
	    if(w) ExposeRedraw(graph, ((XExposeEvent*)event)->x, ((XExposeEvent*)event)->y, w, ((XExposeEvent*)event)->height);
	    return;
	case MotionNotify:
	    f = 0;
	    w = ((XButtonEvent*)event)->x; h = ((XButtonEvent*)event)->y;
	    break;
	case ButtonRelease:
	    f = -1; // release indicated by negative button numbers
	case ButtonPress:
	    w = ((XButtonEvent*)event)->x; h = ((XButtonEvent*)event)->y;
	    switch(((XButtonEvent*)event)->button) {
		case Button1: button = 1; break;
		case Button2: button = 2; break;
		case Button3: button = 3; break;
		case Button4: button = 4; break;
		case Button5: button = 5; break;
	    }
    }
    button *= f;
    opt = userHandler(button, w, h);
    if(opt) { // user callback specifies a context menu; pop it up
	XUngrabPointer(xDisplay, CurrentTime);
	XtCallActionProc(widget, "XawPositionSimpleMenu", event, &(opt->name), 1);
	XtPopupSpringLoaded(opt->handle);
    }
    XSync(xDisplay, False);
}

void
GraphExpose (Option *opt, int x, int y, int w, int h)
{
  XExposeEvent e;
  if(!opt->handle) return;
  e.x = x; e.y = y; e.width = w; e.height = h; e.count = -1; e.type = Expose; // count = -1: kludge to suppress sizing
  GraphEventProc(opt->handle, (caddr_t) opt, (XEvent *) &e); // fake expose event
}

static void
GenericCallback (Widget w, XtPointer client_data, XtPointer call_data)
{   // all Buttons in a dialog (including OK, cancel) invoke this
    String name;
    Arg args[16];
    char buf[MSG_SIZ];
    int data = (intptr_t) client_data;
    DialogClass dlg;
    Widget sh = XtParent(XtParent(XtParent(w))), oldSh;

    currentOption = dialogOptions[dlg=data>>16]; data &= 0xFFFF;
    oldSh = shells[dlg]; shells[dlg] = sh; // bow to reality
    if (data == 30000) { // cancel
        PopDown(dlg);
    } else
    if (data == 30001) { // save buttons imply OK
        if(GenericReadout(currentOption, -1)) PopDown(dlg); // calls OK-proc after full readout, but no popdown if it returns false
    } else

    if(currentCps && dlg != BrowserDlg) {
	XtSetArg(args[0], XtNlabel, &name);
	XtGetValues(w, args, 1);
	if(currentOption[data].type == SaveButton) GenericReadout(currentOption, -1);
	snprintf(buf, MSG_SIZ,  "option %s\n", name);
	SendToProgram(buf, currentCps);
    } else ((ButtonCallback*) currentOption[data].target)(data);

    shells[dlg] = oldSh; // in case of multiple instances, restore previous (as this one could be popped down now)
}

void
TabProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // for transfering focus to the next text-edit
    Option *opt;
    for(opt = currentOption; opt->type != EndMark; opt++) {
	if(opt->handle == w) {
	    while(++opt) {
		if(opt->type == EndMark) opt = currentOption; // wrap
		if(opt->handle == w) return; // full circle
		if(opt->type == TextBox || opt->type == Spin || opt->type == Fractional || opt->type == FileName || opt->type == PathName) {
		    SetFocus(opt->handle, XtParent(XtParent(XtParent(w))), NULL, 0);
		    return;
	        }
	    }
	}
    }
}

void
WheelProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // for scrolling a widget seen through a viewport with the mouse wheel (ListBox!)
    int j=0, n = atoi(prms[0]);
    static char *params[3] = { "", "Continuous", "Proportional" };
    Arg args[16];
    float h, top;
    Widget v;
    if(!n) { // transient dialogs also use this for list-selection callback
	n = prms[1][0]-'0';
	Option *opt=dialogOptions[prms[2][0]-'A'] + n;
	if(opt->textValue) ((ListBoxCallback*) opt->textValue)(n, SelectedListBoxItem(opt));
	return;
    }
    v = XtNameToWidget(XtParent(w), "vertical");
    if(!v) return;
    XtSetArg(args[j], XtNshown, &h); j++;
    XtSetArg(args[j], XtNtopOfThumb, &top); j++;
    XtGetValues(v, args, j);
    top += 0.1f*h*n; if(top < 0.f) top = 0.;
    XtCallActionProc(v, "StartScroll", event, params+1, 1);
    XawScrollbarSetThumb(v, top, -1.0);
    XtCallActionProc(v, "NotifyThumb", event, params, 0);
//    XtCallActionProc(w, "NotifyScroll", event, params+2, 1);
    XtCallActionProc(v, "EndScroll", event, params, 0);
}

static char *oneLiner  =
   "<Key>Return: redraw-display() \n \
    <Key>Tab: TabProc() \n ";
static char scrollTranslations[] =
   "<Btn1Up>(2): WheelProc(0 0 A) \n \
    <Btn4Down>: WheelProc(-1) \n \
    <Btn5Down>: WheelProc(1) \n ";

static void
SqueezeIntoBox (Option *opt, int nr, int width)
{   // size buttons in bar to fit, clipping button names where necessary
    int i, wtot = 0;
    Dimension widths[20], oldWidths[20];
    Arg arg;
    for(i=1; i<nr; i++) {
	XtSetArg(arg, XtNwidth, &widths[i]);
	XtGetValues(opt[i].handle, &arg, 1);
	wtot +=  oldWidths[i] = widths[i];
    }
    opt->min = wtot;
    if(width <= 0) return;
    while(wtot > width) {
	int wmax=0, imax=0;
	for(i=1; i<nr; i++) if(widths[i] > wmax) wmax = widths[imax=i];
	widths[imax]--;
	wtot--;
    }
    for(i=1; i<nr; i++) if(widths[i] != oldWidths[i]) {
	XtSetArg(arg, XtNwidth, widths[i]);
	XtSetValues(opt[i].handle, &arg, 1);
    }
    opt->min = wtot;
}

int
SetPositionAndSize (Arg *args, Widget leftNeigbor, Widget topNeigbor, int b, int w, int h, int chaining)
{   // sizing and positioning most widgets have in common
    int j = 0;
    // first position the widget w.r.t. earlier ones
    if(chaining & 1) { // same row: position w.r.t. last (on current row) and lastrow
	XtSetArg(args[j], XtNfromVert, topNeigbor); j++;
	XtSetArg(args[j], XtNfromHoriz, leftNeigbor); j++;
    } else // otherwise it goes at left margin (which is default), below the previous element
	XtSetArg(args[j], XtNfromVert, leftNeigbor),  j++;
    // arrange chaining ('2'-bit indicates top and bottom chain the same)
    if((chaining & 14) == 6) XtSetArg(args[j], XtNtop,    XtChainBottom), j++;
    if((chaining & 14) == 10) XtSetArg(args[j], XtNbottom, XtChainTop ), j++;
    if(chaining & 4) XtSetArg(args[j], XtNbottom, XtChainBottom ), j++;
    if(chaining & 8) XtSetArg(args[j], XtNtop,    XtChainTop), j++;
    if(chaining & 0x10) XtSetArg(args[j], XtNright, XtChainRight), j++;
    if(chaining & 0x20) XtSetArg(args[j], XtNleft,  XtChainRight), j++;
    if(chaining & 0x40) XtSetArg(args[j], XtNright, XtChainLeft ), j++;
    if(chaining & 0x80) XtSetArg(args[j], XtNleft,  XtChainLeft ), j++;
    // set size (if given)
    if(w) XtSetArg(args[j], XtNwidth, w), j++;
    if(h) XtSetArg(args[j], XtNheight, h),  j++;
    // color
    if(!appData.monoMode) {
	if(!b && appData.dialogColor[0]) XtSetArg(args[j], XtNbackground, dialogColor),  j++;
	if(b == 3 && appData.buttonColor[0]) XtSetArg(args[j], XtNbackground, buttonColor),  j++;
    }
    if(b == 3) b = 1;
    // border
    XtSetArg(args[j], XtNborderWidth, b);  j++;
    return j;
}

int
GenericPopUp (Option *option, char *title, DialogClass dlgNr, DialogClass parent, int modal, int top)
{
    Arg args[24];
    Widget popup, layout, dialog=NULL, edit=NULL, form,  last, b_ok, b_cancel, previousPane = NULL, textField = NULL, oldForm, oldLastRow, oldForeLast;
    Window root, child;
    int x, y, i, j, height=999, width=1, h, c, w, shrink=FALSE, stack = 0, box, chain;
    int win_x, win_y, maxWidth, maxTextWidth;
    unsigned int mask;
    char def[MSG_SIZ], *msg, engineDlg = (currentCps != NULL && dlgNr != BrowserDlg);
    static char pane[6] = "paneX";
    Widget texts[100], forelast = NULL, anchor, widest, lastrow = NULL, browse = NULL;
    Dimension bWidth = 50;

    if(dlgNr < PromoDlg && shellUp[dlgNr]) return 0; // already up
    if(dlgNr && dlgNr < PromoDlg && shells[dlgNr]) { // reusable, and used before (but popped down)
	XtPopup(shells[dlgNr], XtGrabNone);
	shellUp[dlgNr] = True;
	return 0;
    }

    dialogOptions[dlgNr] = option; // make available to callback
    // post currentOption globally, so Spin and Combo callbacks can already use it
    // WARNING: this kludge does not work for persistent dialogs, so that these cannot have spin or combo controls!
    currentOption = option;

    if(engineDlg) { // Settings popup for engine: format through heuristic
	int n = currentCps->nrOptions;
	if(n > 50) width = 4; else if(n>24) width = 2; else width = 1;
	height = n / width + 1;
	if(n && (currentOption[n-1].type == Button || currentOption[n-1].type == SaveButton)) currentOption[n].min = SAME_ROW; // OK on same line
	currentOption[n].type = EndMark; currentOption[n].target = NULL; // delimit list by callback-less end mark
    }
     i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    shells[BoardWindow] = shellWidget; parents[dlgNr] = parent;

    if(dlgNr == BoardWindow) popup = shellWidget; else
    popup = shells[dlgNr] =
      XtCreatePopupShell(title, !top || !appData.topLevel ? transientShellWidgetClass : topLevelShellWidgetClass,
                                                           shells[parent], args, i);

    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));
    if(!appData.monoMode && appData.dialogColor[0]) XtSetArg(args[0], XtNbackground, dialogColor);
    XtSetValues(layout, args, 1);

  for(c=0; c<width; c++) {
    pane[4] = 'A'+c;
    form =
      XtCreateManagedWidget(pane, formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    j=0;
    XtSetArg(args[j], stack ? XtNfromVert : XtNfromHoriz, previousPane);  j++;
    if(!appData.monoMode && appData.dialogColor[0]) XtSetArg(args[j], XtNbackground, dialogColor),  j++;
    XtSetValues(form, args, j);
    lastrow = forelast = NULL;
    previousPane = form;

    last = widest = NULL; anchor = lastrow;
    for(h=0; h<height || c == width-1; h++) {
	i = h + c*height;
	if(option[i].type == EndMark) break;
	if(option[i].type == -1) continue;
	lastrow = forelast;
	forelast = last;
	switch(option[i].type) {
	  case Fractional:
	    snprintf(def, MSG_SIZ,  "%.2f", *(float*)option[i].target);
	    option[i].value = *(float*)option[i].target;
	    goto tBox;
	  case Spin:
	    if(!engineDlg) option[i].value = *(int*)option[i].target;
	    snprintf(def, MSG_SIZ,  "%d", option[i].value);
	  case TextBox:
	  case FileName:
	  case PathName:
          tBox:
	    if(option[i].name[0]) { // prefixed by label with option name
		j = SetPositionAndSize(args, last, lastrow, 0 /* border */,
				       0 /* w */, textHeight /* h */, 0xC0 /* chain to left edge */);
		XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
		XtSetArg(args[j], XtNlabel, _(option[i].name));  j++;
		texts[h] = dialog = XtCreateManagedWidget(option[i].name, labelWidgetClass, form, args, j);
	    } else texts[h] = dialog = NULL; // kludge to position from left margin
	    w = option[i].type == Spin || option[i].type == Fractional ? 70 : option[i].max ? option[i].max : 205;
	    if(option[i].type == FileName || option[i].type == PathName) w -= 55;
	    if(squareSize > 33) w += (squareSize - 33)/2;
	    j = SetPositionAndSize(args, dialog, last, 1 /* border */,
				   w /* w */, option[i].type == TextBox ? option[i].value : 0 /* h */, 0x91 /* chain full width */);
	    if(option[i].type == TextBox) { // decorations for multi-line text-edits
		if(option[i].min & T_VSCRL) { XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++; }
		if(option[i].min & T_HSCRL) { XtSetArg(args[j], XtNscrollHorizontal, XawtextScrollAlways);  j++; }
		if(option[i].min & T_FILL)  { XtSetArg(args[j], XtNautoFill, True);  j++; }
		if(option[i].min & T_WRAP)  { XtSetArg(args[j], XtNwrap, XawtextWrapWord); j++; }
		if(option[i].min & T_TOP)   { XtSetArg(args[j], XtNtop, XtChainTop); j++;
		    if(!option[i].value) {    XtSetArg(args[j], XtNbottom, XtChainTop); j++;
					      XtSetValues(dialog, args+j-2, 2);
		    }
		}
	    } else shrink = TRUE;
	    XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	    XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
	    XtSetArg(args[j], XtNdisplayCaret, False);  j++;
	    XtSetArg(args[j], XtNresizable, True);  j++;
	    XtSetArg(args[j], XtNinsertPosition, 9999);  j++;
	    XtSetArg(args[j], XtNstring, option[i].type==Spin || option[i].type==Fractional ? def :
				engineDlg ? option[i].textValue : *(char**)option[i].target);  j++;
	    edit = last;
	    option[i].handle = (void*)
		(textField = last = XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j));
	    XtAddEventHandler(last, ButtonPressMask, False, SetFocus, (XtPointer) popup); // gets focus on mouse click
	    if(option[i].min == 0 || option[i].type != TextBox)
		XtOverrideTranslations(last, XtParseTranslationTable(oneLiner)); // standard handler for <Enter> and <Tab>

	    if(option[i].type == TextBox || option[i].type == Fractional) break;

	    // add increment and decrement controls for spin
	    if(option[i].type == FileName || option[i].type == PathName) {
		msg = _("browse"); w = 0; // automatically scale to width of text
		j = textHeight ? textHeight : 0;
	    } else {
		w = 20; msg = "+"; j = textHeight/2; // spin button
	    }
	    j = SetPositionAndSize(args, last, edit, 3 /* border */,
				   w /* w */, j /* h */, 0x31 /* chain to right edge */);
	    edit = XtCreateManagedWidget(msg, commandWidgetClass, form, args, j);
	    XtAddCallback(edit, XtNcallback, SpinCallback, (XtPointer)(intptr_t) i + 256*dlgNr);
	    if(w == 0) browse = edit;

	    if(option[i].type != Spin) break;

	    j = SetPositionAndSize(args, last, edit, 3 /* border */,
				   20 /* w */, textHeight/2 /* h */, 0x31 /* chain to right edge */);
	    XtSetArg(args[j], XtNvertDistance, -1);  j++;
	    last = XtCreateManagedWidget("-", commandWidgetClass, form, args, j);
	    XtAddCallback(last, XtNcallback, SpinCallback, (XtPointer)(intptr_t) i + 256*dlgNr);
	    break;
	  case CheckBox:
	    if(!engineDlg) option[i].value = *(Boolean*)option[i].target; // where checkbox callback uses it
	    j = SetPositionAndSize(args, last, lastrow, 1 /* border */,
				   textHeight/2 /* w */, textHeight/2 /* h */, 0xC0 /* chain both to left edge */);
	    XtSetArg(args[j], XtNvertDistance, (textHeight+2)/4 + 3);  j++;
	    XtSetArg(args[j], XtNstate, option[i].value);  j++;
	    lastrow  = last;
	    option[i].handle = (void*)
		(last = XtCreateManagedWidget(" ", toggleWidgetClass, form, args, j));
	    j = SetPositionAndSize(args, last, lastrow, 0 /* border */,
				   option[i].max /* w */, textHeight /* h */, 0xC1 /* chain */);
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    XtSetArg(args[j], XtNlabel, _(option[i].name));  j++;
	    last = XtCreateManagedWidget("label", commandWidgetClass, form, args, j);
	    // make clicking the text toggle checkbox
	    XtAddEventHandler(last, ButtonPressMask, False, CheckCallback, (XtPointer)(intptr_t) i + 256*dlgNr);
	    shrink = TRUE; // following buttons must get text height
	    break;
	  case Icon:
	  case Label:
	    msg = option[i].name;
	    if(!msg) break;
	    chain = option[i].min;
	    if(chain & SAME_ROW) forelast = lastrow; else shrink = FALSE;
	    j = SetPositionAndSize(args, last, lastrow, (chain & 2) != 0 /* border */,
				   option[i].max /* w */, shrink ? textHeight : 0 /* h */, chain | 2 /* chain */);
#if ENABLE_NLS
	    if(option[i].choice) XtSetArg(args[j], XtNfontSet, *(XFontSet*)option[i].choice), j++;
#else
	    if(option[i].choice) XtSetArg(args[j], XtNfont, (XFontStruct*)option[i].choice), j++;
#endif
	    XtSetArg(args[j], XtNresizable, False);  j++;
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    XtSetArg(args[j], XtNlabel, _(msg));  j++;
	    option[i].handle = (void*) (last = XtCreateManagedWidget("label", labelWidgetClass, form, args, j));
	    if(option[i].target) // allow user to specify event handler for button presses
		XtAddEventHandler(last, ButtonPressMask, False, CheckCallback, (XtPointer)(intptr_t) i + 256*dlgNr);
	    break;
	  case SaveButton:
	  case Button:
	    if(option[i].min & SAME_ROW) {
		chain = 0x31; // 0011.0001 = both left and right side to right edge
		forelast = lastrow;
	    } else chain = 0, shrink = FALSE;
	    j = SetPositionAndSize(args, last, lastrow, 3 /* border */,
				   option[i].max /* w */, shrink ? textHeight : 0 /* h */, option[i].min & 0xE | chain /* chain */);
	    XtSetArg(args[j], XtNlabel, _(option[i].name));  j++;
	    if(option[i].textValue) { // special for buttons of New Variant dialog
		XtSetArg(args[j], XtNsensitive, option[i].value >= 0 && (appData.noChessProgram
					 || strstr(first.variants, VariantName(option[i].value)))); j++;
		XtSetArg(args[j], XtNborderWidth, (gameInfo.variant == option[i].value)+1); j++;
	    }
	    option[i].handle = (void*)
		(dialog = last = XtCreateManagedWidget(option[i].name, commandWidgetClass, form, args, j));
	    if(option[i].choice && ((char*)option[i].choice)[0] == '#' && !engineDlg) { // for the color picker default-reset
		SetColor( *(char**) option[i-1].target, &option[i]);
		XtAddEventHandler(option[i-1].handle, KeyReleaseMask, False, ColorChanged, (XtPointer)(intptr_t) i-1);
	    }
	    XtAddCallback(last, XtNcallback, GenericCallback, (XtPointer)(intptr_t) i + (dlgNr<<16)); // invokes user callback
	    if(option[i].textValue) SetColor( option[i].textValue, &option[i]); // for new-variant buttons
	    break;
	  case ComboBox:
	    j = SetPositionAndSize(args, last, lastrow, 0 /* border */,
				   0 /* w */, textHeight /* h */, 0xC0 /* chain both sides to left edge */);
	    XtSetArg(args[j], XtNjustify, XtJustifyLeft);  j++;
	    XtSetArg(args[j], XtNlabel, _(option[i].name));  j++;
	    texts[h] = dialog = XtCreateManagedWidget(option[i].name, labelWidgetClass, form, args, j);

	    if(option[i].min & COMBO_CALLBACK) msg = _(option[i].name); else {
	      if(!engineDlg) SetCurrentComboSelection(option+i);
	      msg=_(((char**)option[i].choice)[option[i].value]);
	    }

	    j = SetPositionAndSize(args, dialog, last, (option[i].min & 2) == 0 /* border */,
				   option[i].max && !engineDlg ? option[i].max : 100 /* w */,
				   textHeight /* h */, 0x91 /* chain */); // same row as its label!
	    XtSetArg(args[j], XtNmenuName, XtNewString(option[i].name));  j++;
	    XtSetArg(args[j], XtNlabel, msg);  j++;
	    shrink = TRUE;
	    option[i].handle = (void*)
		(last = XtCreateManagedWidget(" ", menuButtonWidgetClass, form, args, j));
	    CreateComboPopup(last, option + i, i + 256*dlgNr, TRUE, -1);
	    values[i] = option[i].value;
	    break;
	  case ListBox:
	    // Listbox goes in viewport, as needed for game list
	    if(option[i].min & SAME_ROW) forelast = lastrow;
	    j = SetPositionAndSize(args, last, lastrow, 1 /* border */,
				   option[i].max /* w */, option[i].value /* h */, option[i].min /* chain */);
	    XtSetArg(args[j], XtNresizable, False);  j++;
	    XtSetArg(args[j], XtNallowVert, True); j++; // scoll direction
	    last =
	      XtCreateManagedWidget("viewport", viewportWidgetClass, form, args, j);
	    j = 0; // now list itself
	    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
	    XtSetArg(args[j], XtNforceColumns, True);  j++;
	    XtSetArg(args[j], XtNverticalList, True);  j++;
	    option[i].handle = (void*)
	        (edit = XtCreateManagedWidget("list", listWidgetClass, last, args, j));
	    XawListChange(option[i].handle, option[i].target, 0, 0, True);
	    XawListHighlight(option[i].handle, 0);
	    scrollTranslations[25] = '0' + i;
	    scrollTranslations[27] = 'A' + dlgNr;
	    XtOverrideTranslations(edit, XtParseTranslationTable(scrollTranslations)); // for mouse-wheel
	    break;
	  case Graph:
	    j = SetPositionAndSize(args, last, lastrow, 0 /* border */,
				   option[i].max /* w */, option[i].value /* h */, option[i].min /* chain */);
	    option[i].handle = (void*)
		(last = XtCreateManagedWidget("graph", widgetClass, form, args, j));
	    XtAddEventHandler(last, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask, False,
		      (XtEventHandler) GraphEventProc, &option[i]); // mandatory user-supplied expose handler
	    if(option[i].min & SAME_ROW) last = forelast, forelast = lastrow;
	    option[i].choice = (char**) cairo_image_surface_create (CAIRO_FORMAT_ARGB32, option[i].max, option[i].value); // image buffer
	    break;
	  case PopUp: // note: used only after Graph, so 'last' refers to the Graph widget
	    option[i].handle = (void*) CreateComboPopup(last, option + i, i + 256*dlgNr, TRUE, option[i].value);
	    break;
	  case BarBegin:
	  case BoxBegin:
	    if(option[i].min & SAME_ROW) forelast = lastrow;
	    j = SetPositionAndSize(args, last, lastrow, 0 /* border */,
				   0 /* w */, 0 /* h */, option[i].min /* chain */);
	    XtSetArg(args[j], XtNorientation, XtorientHorizontal);  j++;
	    XtSetArg(args[j], XtNvSpace, 0);                        j++;
	    option[box=i].handle = (void*)
		(last = XtCreateWidget("box", boxWidgetClass, form, args, j));
	    oldForm = form; form = last; oldLastRow = lastrow; oldForeLast = forelast;
	    lastrow = NULL; last = NULL;
	    break;
	  case DropDown:
	    j = SetPositionAndSize(args, last, lastrow, 0 /* border */,
				   0 /* w */, 0 /* h */, 1 /* chain (always on same row) */);
	    forelast = lastrow;
	    msg = _(option[i].name); // write name on the menu button
	    XtSetArg(args[j], XtNmenuName, XtNewString(option[i].name));  j++;
	    XtSetArg(args[j], XtNlabel, msg);  j++;
	    option[i].handle = (void*)
		(last = XtCreateManagedWidget(option[i].name, menuButtonWidgetClass, form, args, j));
	    option[i].textValue = (char*) CreateComboPopup(last, option + i, i + 256*dlgNr, FALSE, -1);
	    break;
	  case BarEnd:
	  case BoxEnd:
	    XtManageChildren(&form, 1);
	    SqueezeIntoBox(&option[box], i-box, option[box].max);
	    if(option[i].target) ((ButtonCallback*)option[i].target)(box); // callback that can make sizing decisions
	    last = form; lastrow = oldLastRow; form = oldForm; forelast = oldForeLast;
	    break;
	  case Break:
	    width++;
	    height = i+1;
	    stack = !(option[i].min & SAME_ROW);
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
    for(h=0; h<height || c == width-1; h++) {
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
    for(h=0; h<height || c == width-1; h++) {
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

    if(option[i].min & SAME_ROW) { // even when OK suppressed this EndMark bit can request chaining of last row to bottom
	for(j=i-1; option[j+1].min & SAME_ROW; j--) {
	    XtSetArg(args[0], XtNtop, XtChainBottom);
	    XtSetArg(args[1], XtNbottom, XtChainBottom);
	    XtSetValues(option[j].handle, args, 2);
	}
	if((option[j].type == TextBox || option[j].type == ListBox) && option[j].name[0] == NULLCHAR) {
	    Widget w = option[j].handle;
	    if(option[j].type == ListBox) w = XtParent(w); // for listbox we must chain viewport
	    XtSetArg(args[0], XtNbottom, XtChainBottom);
	    XtSetValues(w, args, 1);
	}
	lastrow = forelast;
    } else shrink = FALSE, lastrow = last, last = widest ? widest : dialog;
    j = SetPositionAndSize(args, last, anchor ? anchor : lastrow, 3 /* border */,
			   0 /* w */, shrink ? textHeight : 0 /* h */, 0x37 /* chain: right, bottom and use both neighbors */);

  if(!(option[i].min & NO_OK)) {
    option[i].handle = b_ok = XtCreateManagedWidget(_("OK"), commandWidgetClass, form, args, j);
    XtAddCallback(b_ok, XtNcallback, GenericCallback, (XtPointer)(intptr_t) (30001 + (dlgNr<<16)));
    if(!(option[i].min & NO_CANCEL)) {
      XtSetArg(args[1], XtNfromHoriz, b_ok); // overwrites!
      b_cancel = XtCreateManagedWidget(_("cancel"), commandWidgetClass, form, args, j);
      XtAddCallback(b_cancel, XtNcallback, GenericCallback, (XtPointer)(intptr_t) (30000 + (dlgNr<<16)));
    }
  }

    XtRealizeWidget(popup);
    if(dlgNr != BoardWindow) { // assign close button, and position w.r.t. pointer, if not main window
	XSetWMProtocols(xDisplay, XtWindow(popup), &wm_delete_window, 1);
	snprintf(def, MSG_SIZ, "<Message>WM_PROTOCOLS: GenericPopDown(\"%d\") \n", dlgNr);
	XtAugmentTranslations(popup, XtParseTranslationTable(def));
	XQueryPointer(xDisplay, xBoardWindow, &root, &child,
			&x, &y, &win_x, &win_y, &mask);

	XtSetArg(args[0], XtNx, x - 10);
	XtSetArg(args[1], XtNy, y - 30);
	XtSetValues(popup, args, 2);
    }
    XtPopup(popup, modal ? XtGrabExclusive : XtGrabNone);
    shellUp[dlgNr]++; // count rather than flag
    previous = NULL;
    if(textField) SetFocus(textField, popup, (XEvent*) NULL, False);
    if(dlgNr && wp[dlgNr]) { // if persistent window-info available, reposition
	j = 0;
	if(wp[dlgNr]->width > 0 && wp[dlgNr]->height > 0) {
	  XtSetArg(args[j], XtNheight, (Dimension) (wp[dlgNr]->height));  j++;
	  XtSetArg(args[j], XtNwidth,  (Dimension) (wp[dlgNr]->width));  j++;
	}
	if(wp[dlgNr]->x > 0 && wp[dlgNr]->y > 0) {
	  XtSetArg(args[j], XtNx, (Position) (wp[dlgNr]->x));  j++;
	  XtSetArg(args[j], XtNy, (Position) (wp[dlgNr]->y));  j++;
	}
	if(j) XtSetValues(popup, args, j);
    }
    RaiseWindow(dlgNr);
    return 1; // tells caller he must do initialization (e.g. add specific event handlers)
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
//    XSetInputFocus(xDisplay, XtWindow(opt->handle), RevertToPointerRoot, CurrentTime);
}

void
TypeInProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // can be used as handler for any text edit in any dialog (from GenericPopUp, that is)
    int n = prms[0][0] - '0';
    Widget sh = XtParent(XtParent(XtParent(w))); // popup shell

    if(n<2) { // Enter or Esc typed from primed text widget: treat as if dialog OK or cancel button hit.
	int dlgNr; // figure out what the dialog number is by comparing shells (because we must pass it :( )
	for(dlgNr=0; dlgNr<NrOfDialogs; dlgNr++) if(shellUp[dlgNr] && shells[dlgNr] == sh)
	    GenericCallback (w, (XtPointer)(intptr_t) (30000 + n + (dlgNr<<16)), NULL);
    }
}

void
HardSetFocus (Option *opt)
{
    XSetInputFocus(xDisplay, XtWindow(opt->handle), RevertToPointerRoot, CurrentTime);
}

void
FileNamePopUpWrapper (char *label, char *def, char *filter, FileProc proc, Boolean pathFlag, char *openMode, char **openName, FILE **openFP)
{
    Browse(BoardWindow, label, (def[0] ? def : NULL), filter, False, openMode, openName, openFP);
}
