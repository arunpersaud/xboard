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

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  

#include "common.h"
#include "backend.h"
#include "xboard.h"
#include "xboard2.h"
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

#ifdef TODO_GTK
static Widget previous = NULL;
#endif
static Option *currentOption;
static Boolean browserUp;

void
UnCaret ()
{
#ifdef TODO_GTK
    Arg args[2];

    if(previous) {
	XtSetArg(args[0], XtNdisplayCaret, False);
	XtSetValues(previous, args, 1);
    }
    previous = NULL;
#endif
}

#ifdef TODO_GTK
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
#endif

void
BoardFocus ()
{
#ifdef TODO_GTK
    XtSetKeyboardFocus(shellWidget, formWidget);
#endif
}

//--------------------------- Engine-specific options menu ----------------------------------

int dialogError;
Option *dialogOptions[NrOfDialogs];

#ifdef TODO_GTK
static Arg layoutArgs[] = {
    { XtNborderWidth, 0 },
    { XtNdefaultDistance, 0 },
};

static Arg formArgs[] = {
    { XtNborderWidth, 0 },
    { XtNresizable, (XtArgVal) True },
};
#endif

void
MarkMenuItem (char *menuRef, int state)
{
    MenuItem *item = MenuNameToItem(menuRef);

    if(item) {
        ((GtkCheckMenuItem *) (item->handle))->active = state;
    }
}

void GetWidgetTextGTK(GtkWidget *w, char **buf)
{        
    GtkTextIter start;
    GtkTextIter end;    

    if (GTK_IS_ENTRY(w)) {
	*buf = gtk_entry_get_text(GTK_ENTRY (w));
    } else
    if (GTK_IS_TEXT_BUFFER(w)) {
        gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(w), &start);
        gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(w), &end);
        *buf = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(w), &start, &end, FALSE);
    }
    else {
        printf("error in GetWidgetText, invalid widget\n");
        *buf = NULL; 
    }
}

void
GetWidgetText (Option *opt, char **buf)
{
    int x;
    static char val[12];
    switch(opt->type) {
      case Fractional:
      case FileName:
      case PathName:
      case TextBox: GetWidgetTextGTK((GtkWidget *) opt->handle, buf); break;
      case Spin:
	x = gtk_spin_button_get_value (GTK_SPIN_BUTTON(opt->handle));                   
	snprintf(val, 12, "%d", x); *buf = val;
	break;
      default:
	printf("unexpected case (%d) in GetWidgetText\n", opt->type);
	*buf = NULL;
    }
}

void SetSpinValue(Option *opt, char *val, int n)
{    
    if (opt->type == Spin)
      {
        if (!strcmp(val, _("Unused")))
           gtk_widget_set_sensitive(opt->handle, FALSE);
        else
          {
            gtk_widget_set_sensitive(opt->handle, TRUE);      
            gtk_spin_button_set_value(opt->handle, atoi(val));
          }
      }
    else
      printf("error in SetSpinValue, unknown type %d\n", opt->type);    
}

void SetWidgetTextGTK(GtkWidget *w, char *text)
{
    if (GTK_IS_ENTRY(w)) {
	gtk_entry_set_text (GTK_ENTRY (w), text);
    } else
    if (GTK_IS_TEXT_BUFFER(w)) {
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(w), text, -1);
    } else
	printf("error: SetWidgetTextGTK arg is neitherGtkEntry nor GtkTextBuffer\n");
}

void
SetWidgetText (Option *opt, char *buf, int n)
{
    switch(opt->type) {
      case Fractional:
      case FileName:
      case PathName:
      case TextBox: SetWidgetTextGTK((GtkWidget *) opt->handle, buf); break;
      case Spin: SetSpinValue(opt, buf, n); break;
      default:
	printf("unexpected case (%d) in GetWidgetText\n", opt->type);
    }
#ifdef TODO_GTK
// focus is automatic in GTK?
    if(n >= 0) SetFocus(opt->handle, shells[n], NULL, False);
#endif
}

void
GetWidgetState (Option *opt, int *state)
{
    *state = gtk_toggle_button_get_active(opt->handle);
}

void
SetWidgetState (Option *opt, int state)
{
    gtk_toggle_button_set_active(opt->handle, state);
}

void
SetWidgetLabel (Option *opt, char *buf)
{
    gtk_label_set_text(opt->handle, buf);
}

void
SetDialogTitle (DialogClass dlg, char *title)
{
    gtk_window_set_title(GTK_WINDOW(shells[dlg]), title);
}

void
SetListBoxItem (GtkListStore *store, int n, char *msg)
{
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_indices(n, -1);
    gtk_tree_model_get_iter(GTK_TREE_MODEL (store), &iter, path);
    gtk_tree_path_free(path);
    gtk_list_store_set(store, &iter, 0, msg, -1);
}

void
LoadListBox (Option *opt, char *emptyText, int n1, int n2)
{
    char **data = (char **) (opt->target);
    GtkWidget *list = (GtkWidget *) (opt->handle);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
    GtkListStore *store = GTK_LIST_STORE(model);
    GtkTreeIter iter;
 
    if(n1 >= 0 && n2 >= 0) {
	SetListBoxItem(store, n1, data[n1]);
	SetListBoxItem(store, n2, data[n2]);
	return;
    }

    if (gtk_tree_model_get_iter_first(model, &iter)) 
	gtk_list_store_clear(store);

    while(*data) { // add elements to listbox one by one
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, *data++, -1); // 0 = first column
    }
}

void
HighlightItem (Option *opt, int index, int scroll)
{
    char *value, **data = (char **) (opt->target);
    GtkWidget *list = (GtkWidget *) (opt->handle);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
    GtkListStore *store = GTK_LIST_STORE(model);
    GtkTreePath *path = gtk_tree_path_new_from_indices(index, -1);
    GtkTreeIter iter;
    gtk_tree_selection_select_path(selection, path);
    if(scroll) gtk_tree_view_scroll_to_cell(list, path, NULL, 0, 0, 0);
    gtk_tree_path_free(path);
}

void
HighlightListBoxItem (Option *opt, int index)
{
    HighlightItem (opt, index, FALSE);
}

void
HighlightWithScroll (Option *opt, int index, int max)
{
    HighlightItem (opt, index, TRUE); // ignore max
}

int
SelectedListBoxItem (Option *opt)
{
    int i;
    char *value, **data = (char **) (opt->target);
    GtkWidget *list = (GtkWidget *) (opt->handle);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));

    GtkTreeModel *model;
    GtkTreeIter iter;
    if (!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter)) return -1;
    gtk_tree_model_get(model, &iter, 0, &value,  -1);
    for(i=0; data[i]; i++) if(!strcmp(data[i], value)) return i;
    g_free(value);
    return -1;
}

void
FocusOnWidget (Option *opt, DialogClass dlg)
{
    UnCaret();
#ifdef TODO_GTK
    XtSetKeyboardFocus(shells[dlg], opt->handle);
#endif
    gtk_widget_grab_focus(opt->handle);
}

void
SetIconName (DialogClass dlg, char *name)
{
#ifdef TODO_GTK
	Arg args[16];
	int j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) name);  j++;
//	XtSetArg(args[j], XtNtitle, (XtArgVal) name);  j++;
	XtSetValues(shells[dlg], args, j);
#endif
}

void ComboSelect(GtkWidget *widget, gpointer addr)
{
    Option *opt = dialogOptions[((intptr_t)addr)>>8]; // applicable option list
    gint i = ((intptr_t)addr) & 255; // option number
    gint g;

    g = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));    
    values[i] = g; // store in temporary, for transfer at OK

#if TODO_GTK
// Note: setting text on button is probably automatic
// Is this still needed? Could be all comboboxes that needed a callbak are now listboxes!
#endif
    if(opt[i].type == Graph || opt[i].min & COMBO_CALLBACK && (!currentCps || shellUp[BrowserDlg])) {
	((ButtonCallback*) opt[i].target)(i);
	return;
    }
}

#ifdef TODO_GTK
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
#endif

static void
MenuSelect (gpointer addr) // callback for all combo items
{
    Option *opt = dialogOptions[((intptr_t)addr)>>24]; // applicable option list
    int i = ((intptr_t)addr)>>16 & 255; // option number
    int j = 0xFFFF & (intptr_t) addr;

    values[i] = j; // store selected value in Option struct, for retrieval at OK
    ((ButtonCallback*) opt[i].target)(i);
}

static GtkWidget *
CreateMenuPopup (Option *opt, int n, int def)
{   // fromList determines if the item texts are taken from a list of strings, or from a menu table
    int i;
    GtkWidget *menu, *entry;
    MenuItem *mb = (MenuItem *) opt->choice;

    menu = gtk_menu_new();
//    menu = XtCreatePopupShell(opt->name, simpleMenuWidgetClass, parent, NULL, 0);
    for (i=0; 1; i++) 
      {
	char *msg = mb[i].string;
	if(!msg) break;
	if(strcmp(msg, "----")) { // 
	  if(!(opt->min & NO_GETTEXT)) msg = _(msg);
	  if(mb[i].handle) {
	    entry = gtk_check_menu_item_new_with_label(msg); // should be used for items that can be checkmarked
	    if(mb[i].handle == RADIO) gtk_check_menu_item_set_draw_as_radio(entry, True);
	  } else
	    entry = gtk_menu_item_new_with_label(msg);
	  gtk_signal_connect_object (GTK_OBJECT (entry), "activate", GTK_SIGNAL_FUNC(MenuSelect), (gpointer) (n<<16)+i);
	  gtk_widget_show(entry);
	} else entry = gtk_separator_menu_item_new();
	gtk_menu_append(GTK_MENU (menu), entry);
//CreateMenuItem(menu, opt->min & NO_GETTEXT ? msg : _(msg), (XtCallbackProc) ComboSelect, (n<<16)+i);
	mb[i].handle = (void*) entry; // save item ID, for enabling / checkmarking
//	if(i==def) {
//	    XtSetArg(arg, XtNpopupOnEntry, entry);
//	    XtSetValues(menu, &arg, 1);
//	}
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

Option *typeIn; // kludge to distinguish type-in callback from input-box callback

static gboolean
ICSKeyEvent(GtkWidget *widget, GdkEventKey *event, gpointer g)
{
    Option *opt = (Option *) g;
    if(opt == typeIn) {
	if(event->keyval == GDK_Return) {
	    char *val;
	    GetWidgetText(opt, &val);
	    TypeInDoneEvent(val);
	    PopDown(TransientDlg);
	    return TRUE;
	}
	return FALSE;
    }

    switch(event->keyval) {
      case GDK_Return: IcsKey(0); return TRUE;
      case GDK_Up:     IcsKey(1); return TRUE;
      case GDK_Down:  IcsKey(-1); return TRUE;
      default: return FALSE;
    }
}

int shiftState, controlState;

static gboolean
TypeInProc (GtkWidget *widget, GdkEventKey *event, gpointer gdata)
{   // This callback catches key presses on text-entries, and uses <Enter> and <Esc> as synonyms for dialog OK or Cancel
    // *** kludge alert *** If a dialog does want some other action, like sending the line typed in the text-entry to an ICS,
    // it should define an OK handler that does so, and returns FALSE to suppress the popdown.
    int n = (intptr_t) gdata;
    int dlg = n >> 16;
    Option *opt;
    n &= 0xFFFF;
    opt = &dialogOptions[dlg][n];

    if(opt == icsBox) return ICSKeyEvent(event->keyval); // Intercept ICS Input Box, which needs special treatment

    shiftState = event->state & GDK_SHIFT_MASK;
    controlState = event->state & GDK_CONTROL_MASK;
    switch(event->keyval) {
      case GDK_Return:
	if(GenericReadout(dialogOptions[dlg], -1)) PopDown(dlg);
	break;
      case GDK_Escape:
	PopDown(dlg);
	break;
      default:
	return FALSE;
    }
    return TRUE;
}

void
HighlightText (Option *opt, int from, int to, Boolean highlight)
{
#   define INIT 0x8000
    static GtkTextIter start, end;

    if(!(opt->min & INIT)) {
	opt->min |= INIT; // each memo its own init flag!
	gtk_text_buffer_create_tag(opt->handle, "highlight", "background", "yellow", NULL);
	gtk_text_buffer_create_tag(opt->handle, "normal", "background", "white", NULL);
    }
    gtk_text_buffer_get_iter_at_offset(opt->handle, &start, from);
    gtk_text_buffer_get_iter_at_offset(opt->handle, &end, to);
    gtk_text_buffer_apply_tag_by_name(opt->handle, highlight ? "highlight" : "normal", &start, &end);
}

int
ShiftKeys ()
{   // bassic primitive for determining if modifier keys are pressed
    return 3*(shiftState != 0) + 0xC*(controlState != 0); // rely on what last mouse button press left us
}

static gboolean
GameListEvent(GtkWidget *widget, GdkEvent *event, gpointer gdata)
{
    int n = (int) gdata;

    if(n == 4) {
	if(((GdkEventKey *) event)->keyval != GDK_Return) return FALSE;
	SetFilter();
	return TRUE;
    }

    if(event->type == GDK_KEY_PRESS) {
	int ctrl = (((GdkEventKey *) event)->state & GDK_CONTROL_MASK) != 0;
	switch(((GdkEventKey *) event)->keyval) {
	  case GDK_Up: GameListClicks(-1 - 2*ctrl); return TRUE;
	  case GDK_Left: GameListClicks(-1); return TRUE;
	  case GDK_Down: GameListClicks(1 + 2*ctrl); return TRUE;
	  case GDK_Right: GameListClicks(1); return TRUE;
	  case GDK_Prior: GameListClicks(-4); return TRUE;
	  case GDK_Next: GameListClicks(4); return TRUE;
	  case GDK_Home: GameListClicks(-2); return TRUE;
	  case GDK_End: GameListClicks(2); return TRUE;
	  case GDK_Return: GameListClicks(0); return TRUE;
	  default: return FALSE;
	}
    }
    if(event->type != GDK_2BUTTON_PRESS || ((GdkEventButton *) event)->button != 1) return FALSE;
    GameListClicks(0);
    return TRUE;
}

static gboolean
MemoEvent(GtkWidget *widget, GdkEvent *event, gpointer gdata)
{   // handle mouse clicks on text widgets that need it
    int w, h;
    int button=10, f=1;
    Option *opt, *memo = (Option *) gdata;
    MemoCallback *userHandler = (MemoCallback *) memo->choice;
    GdkEventButton *bevent = (GdkEventButton *) event;
    GdkEventMotion *mevent = (GdkEventMotion *) event;
    GtkTextIter start, end;
    String val = NULL;
    gboolean res;
    gint index, x, y;

    if(memo->type == Label) { ((ButtonCallback*) memo->target)(memo->value); return TRUE; } // only clock widgets use this

    switch(event->type) { // figure out what's up
	case GDK_MOTION_NOTIFY:
	    f = 0;
	    w = mevent->x; h = mevent->y;
	    break;
	case GDK_BUTTON_RELEASE:
	    f = -1; // release indicated by negative button numbers
	    w = bevent->x; h = bevent->y;
	    button = bevent->button;
	    break;
	case GDK_BUTTON_PRESS:
	    w = bevent->x; h = bevent->y;
	    button = bevent->button;
	    shiftState = bevent->state & GDK_SHIFT_MASK;
	    controlState = bevent->state & GDK_CONTROL_MASK;
// GTK_TODO: is this really the most efficient way to get the character at the mouse cursor???
	    gtk_text_view_window_to_buffer_coords(widget, GTK_TEXT_WINDOW_WIDGET, w, h, &x, &y);
	    gtk_text_view_get_iter_at_location(widget, &start, x, y);
	    gtk_text_buffer_place_cursor(memo->handle, &start);
	    /* get cursor position into index */
	    g_object_get(memo->handle, "cursor-position", &index, NULL);
	    /* get text from textbuffer */
	    gtk_text_buffer_get_start_iter (memo->handle, &start);
	    gtk_text_buffer_get_end_iter (memo->handle, &end);
	    val = gtk_text_buffer_get_text (memo->handle, &start, &end, FALSE); 
	    break;
	default:
	    return FALSE; // should not happen
    }
    button *= f;
    // hand click parameters as well as text & location to user
    res = (userHandler) (memo, button, w, h, val, index);
    if(val) g_free(val);
    return res;
}

void
AddHandler (Option *opt, DialogClass dlg, int nr)
{
    switch(nr) {
      case 0: // history (now uses generic textview callback)
      case 1: // comment (likewise)
	break;
      case 2: // move type-in
	typeIn = opt;
      case 3: // input box
	g_signal_connect(opt->handle, "key-press-event", G_CALLBACK (ICSKeyEvent), (gpointer) opt);
	break;
      case 5: // game list
	g_signal_connect(opt->handle, "button-press-event", G_CALLBACK (GameListEvent), (gpointer) 0 );
      case 4: // game-list filter
	g_signal_connect(opt->handle, "key-press-event", G_CALLBACK (GameListEvent), (gpointer) nr );
	break;
      case 6: // engine output (uses generic textview callback)
	break;
    }
}

//----------------------------Generic dialog --------------------------------------------

// cloned from Engine Settings dialog (and later merged with it)

GtkWidget *shells[NrOfDialogs];
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
#ifdef TODO_GTK
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
          root, False,static gboolean
MemoEvent(GtkWidget *widget, GdkEvent *event, gpointer gdata)

          SubstructureRedirectMask | SubstructureNotifyMask,
          &xev);

    XFlush(xDisplay); 
    XSync(xDisplay, False);
#endif
}

int
PopDown (DialogClass n)
{
    //Arg args[10];    
    
    if (!shellUp[n] || !shells[n]) return 0;    
#ifdef TODO_GTK
// Not sure this is still used
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
#endif
    
    gtk_widget_hide(shells[n]);
    shellUp[n]--; // count rather than clear

    if(n == 0 || n >= PromoDlg) {
        gtk_widget_destroy(shells[n]);
        shells[n] = NULL;
    }

    if(marked[n]) {
	MarkMenuItem(marked[n], False);
	marked[n] = NULL;
    }

    if(!n) currentCps = NULL; // if an Engine Settings dialog was up, we must be popping it down now
    currentOption = dialogOptions[TransientDlg]; // just in case a transient dialog was up (to allow its check and combo callbacks to work)
#ifdef TODO_GTK
    RaiseWindow(parents[n]); // automatic in GTK?
    if(parents[n] == BoardWindow) XtSetKeyboardFocus(shellWidget, formWidget); // also automatic???
#endif
    return 1;
}

/* GTK callback used when OK/cancel clicked in genericpopup for non-modal dialog */
gboolean GenericPopDown(w, resptype, gdata)
     GtkWidget *w;
     GtkResponseType  resptype;
     gpointer  gdata;
{
    DialogClass dlg = (intptr_t) gdata; /* dialog number dlgnr */
    GtkWidget *sh = shells[dlg];

    currentOption = dialogOptions[dlg];

#ifdef TODO_GTK
// I guess BrowserDlg will be abandoned, as GTK has a better browser of its own
    if(shellUp[BrowserDlg] && dlg != BrowserDlg || dialogError) return True; // prevent closing dialog when it has an open file-browse daughter
#else
    if(browserUp || dialogError && dlg != FatalDlg) return True; // prevent closing dialog when it has an open file-browse or error-popup daughter
#endif
    shells[dlg] = w; // make sure we pop down the right one in case of multiple instances

    /* OK pressed */    
    if (resptype == GTK_RESPONSE_ACCEPT) {
        if (GenericReadout(currentOption, -1)) PopDown(dlg);
        return TRUE;
    } else
    /* cancel pressed */
    {
	if(dlg == BoardWindow) ExitEvent(0);
	PopDown(dlg);
    }
    shells[dlg] = sh; // restore
    return TRUE;
}

int AppendText(Option *opt, char *s)
{    
    char *v;
    int len;
    GtkTextIter end;    
  
    GetWidgetTextGTK(opt->handle, &v);
    len = strlen(v);
    g_free(v);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(opt->handle), &end);
    gtk_text_buffer_insert(opt->handle, &end, s, -1);

    return len;
}

void
SetColor (char *colorName, Option *box)
{       // sets the color of a widget
    GdkColor color;

    /* set the colour of the colour button to the colour that will be used */
    gdk_color_parse( colorName, &color );
    gtk_widget_modify_bg ( GTK_WIDGET(box->handle), GTK_STATE_NORMAL, &color );
}

#ifdef TODO_GTK
void
ColorChanged (Widget w, XtPointer data, XEvent *event, Boolean *b)
{   // for detecting a typed change in color
    char buf[10];
    if ( (XLookupString(&(event->xkey), buf, 2, NULL, NULL) == 1) && *buf == '\r' )
	RefreshColor((int)(intptr_t) data, 0);
}
#endif

static void
GraphEventProc(GtkWidget *widget, GdkEvent *event, gpointer gdata)
{   // handle expose and mouse events on Graph widget
    int w, h;
    int j, button=10, f=1, sizing=0;
    Option *opt, *graph = (Option *) gdata;
    PointerCallback *userHandler = graph->target;
    GdkEventExpose *eevent = (GdkEventExpose *) event;
    GdkEventButton *bevent = (GdkEventButton *) event;
    GdkEventMotion *mevent = (GdkEventMotion *) event;
    cairo_t *cr;

//    if (!XtIsRealized(widget)) return;

    switch(event->type) {
	case GDK_EXPOSE: // make handling of expose events generic, just copying from memory buffer (->choice) to display (->textValue)
	    /* Get window size */
#ifdef TODO_GTK
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
		if(graph->choice) cairo_surface_destroy((cairo_surface_t *) graph->choice);
		graph->choice = (char**) cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
		break;
	    }
#endif
	    w = eevent->area.width;
	    if(eevent->area.x + w > graph->max) w--; // cut off fudge pixel
	    cr = gdk_cairo_create(((GtkWidget *) (graph->handle))->window);
	    cairo_set_source_surface(cr, (cairo_surface_t *) graph->choice, 0, 0);
	    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	    cairo_rectangle(cr, eevent->area.x, eevent->area.y, w, eevent->area.height);
	    cairo_fill(cr);
	    cairo_destroy(cr);
	default:
	    return;
	case GDK_MOTION_NOTIFY:
	    f = 0;
	    w = mevent->x; h = mevent->y;
	    break;
	case GDK_BUTTON_RELEASE:
	    f = -1; // release indicated by negative button numbers
	case GDK_BUTTON_PRESS:
	    w = bevent->x; h = bevent->y;
	    button = bevent->button;
	    shiftState = bevent->state & GDK_SHIFT_MASK;
	    controlState = bevent->state & GDK_CONTROL_MASK;
    }
    button *= f;

    opt = userHandler(button, w, h);
#ifdef TODO_GTK
    if(opt) { // user callback specifies a context menu; pop it up
	XUngrabPointer(xDisplay, CurrentTime);
	XtCallActionProc(widget, "XawPositionSimpleMenu", event, &(opt->name), 1);
	XtPopupSpringLoaded(opt->handle);
    }
    XSync(xDisplay, False);
#endif
}

void
GraphExpose (Option *opt, int x, int y, int w, int h)
{
#if 0
  GdkRectangle r;
  r.x = x; r.y = y; r.width = w; r.height = h;
  gdk_window_invalidate_rect(((GtkWidget *)(opt->handle))->window, &r, FALSE);
#endif
  GdkEventExpose e;
  if(!opt->handle) return;
  e.area.x = x; e.area.y = y; e.area.width = w; e.area.height = h; e.count = -1; e.type = GDK_EXPOSE; // count = -1: kludge to suppress sizing
  GraphEventProc(opt->handle, (GdkEvent *) &e, (gpointer) opt); // fake expose event
}

void GenericCallback(GtkWidget *widget, gpointer gdata)
{
    const gchar *name;
    char buf[MSG_SIZ];    
    int data = (intptr_t) gdata;   
    DialogClass dlg;
#ifdef TODO_GTK
    GtkWidget *sh = XtParent(XtParent(XtParent(w))), *oldSh;
#else
    GtkWidget *sh, *oldSh;
#endif

    currentOption = dialogOptions[dlg=data>>16]; data &= 0xFFFF;
#ifndef TODO_GTK
    sh = shells[dlg]; // make following line a no-op, as we haven't found out what the real shell is yet (breaks multiple popups of same type!)
#endif
    oldSh = shells[dlg]; shells[dlg] = sh; // bow to reality

    if (data == 30000) { // cancel
        PopDown(dlg); 
    } else
    if (data == 30001) { // save buttons imply OK
        if(GenericReadout(currentOption, -1)) PopDown(dlg); // calls OK-proc after full readout, but no popdown if it returns false
    } else

    if(currentCps) {
        name = gtk_button_get_label (GTK_BUTTON(widget));         
	if(currentOption[data].type == SaveButton) GenericReadout(currentOption, -1);
        snprintf(buf, MSG_SIZ,  "option %s\n", name);
        SendToProgram(buf, currentCps);
    } else ((ButtonCallback*) currentOption[data].target)(data);   

    shells[dlg] = oldSh; // in case of multiple instances, restore previous (as this one could be popped down now)
}

void BrowseGTK(GtkWidget *widget, gpointer gdata)
{
    GtkWidget *entry;
    GtkWidget *dialog;
    GtkFileFilter *gtkfilter;
    GtkFileFilter *gtkfilter_all;
    int opt_i = (intptr_t) gdata;
    GtkFileChooserAction fc_action;
  
    gtkfilter     = gtk_file_filter_new();
    gtkfilter_all = gtk_file_filter_new();

    char fileext[10] = "*";

    /* select file or folder depending on option_type */
    if (currentOption[opt_i].type == PathName)
        fc_action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    else
        fc_action = GTK_FILE_CHOOSER_ACTION_OPEN;

    dialog = gtk_file_chooser_dialog_new ("Open File",
                      NULL,
                      fc_action,
                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                      NULL);

    /* one filter to show everything */
    gtk_file_filter_add_pattern(gtkfilter_all, "*");
    gtk_file_filter_set_name   (gtkfilter_all, "All Files");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),gtkfilter_all);
    
    /* filter for specific filetypes e.g. pgn or fen */
    if (currentOption[opt_i].textValue != NULL && (strcmp(currentOption[opt_i].textValue, "") != 0) )    
      {          
        strcat(fileext, currentOption[opt_i].textValue);    
        gtk_file_filter_add_pattern(gtkfilter, fileext);
        gtk_file_filter_set_name (gtkfilter, currentOption[opt_i].textValue);
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog),gtkfilter);
        /* activate filter */
        gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(dialog),gtkfilter);
      }
    else
      gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(dialog),gtkfilter_all);       

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
      {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));             
        entry = currentOption[opt_i].handle;
        gtk_entry_set_text (GTK_ENTRY (entry), filename);        
        g_free (filename);

      }
    gtk_widget_destroy (dialog);
    dialog = NULL;
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
#ifdef TODO_GTK
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
#endif
}

#ifdef TODO_GTK
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
#endif

static int
SameRow (Option *opt)
{
    return (opt->min & SAME_ROW && (opt->type == Button || opt->type == SaveButton || opt->type == Label || opt->type == ListBox));
}

static void
Pack (GtkWidget *hbox, GtkWidget *table, GtkWidget *entry, int left, int right, int top)
{
    if(hbox) gtk_box_pack_start(GTK_BOX (hbox), entry, TRUE, TRUE, 0);
    else     gtk_table_attach_defaults(GTK_TABLE(table), entry, left, right, top, top+1);
}

int
GenericPopUp (Option *option, char *title, DialogClass dlgNr, DialogClass parent, int modal, int topLevel)
{    
    GtkWidget *dialog = NULL;
    gint       w;
    GtkWidget *label;
    GtkWidget *box;
    GtkWidget *checkbutton;
    GtkWidget *entry;
    GtkWidget *hbox = NULL;    
    GtkWidget *button;
    GtkWidget *table;
    GtkWidget *spinner;    
    GtkAdjustment *spinner_adj;
    GtkWidget *combobox;
    GtkWidget *textview;
    GtkTextBuffer *textbuffer;           
    GdkColor color;     
    GtkWidget *actionarea;
    GtkWidget *sw;    
    GtkWidget *list;    
    GtkWidget *graph;    
    GtkWidget *menuButton;    
    GtkWidget *menuBar;    
    GtkWidget *menu;    

    int i, j, arraysize, left, top, height=999, width=1, boxStart;    
    char def[MSG_SIZ], *msg, engineDlg = (currentCps != NULL && dlgNr != BrowserDlg);

    if(dlgNr < PromoDlg && shellUp[dlgNr]) return 0; // already up

    if(dlgNr && dlgNr < PromoDlg && shells[dlgNr]) { // reusable, and used before (but popped down)
        gtk_widget_show(shells[dlgNr]);
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
//	if(n && (currentOption[n-1].type == Button || currentOption[n-1].type == SaveButton)) currentOption[n].min = SAME_ROW; // OK on same line
        currentOption[n].type = EndMark; currentOption[n].target = NULL; // delimit list by callback-less end mark
    }    

    parents[dlgNr] = parent;
#ifdef TODO_GTK
    shells[BoardWindow] = shellWidget; parents[dlgNr] = parent;

    if(dlgNr == BoardWindow) dialog = shellWidget; else
    dialog =
      XtCreatePopupShell(title, !top || !appData.topLevel ? transientShellWidgetClass : topLevelShellWidgetClass,
                                                           shells[parent], args, i);
#endif
    dialog = gtk_dialog_new_with_buttons( title,
                                      GTK_WINDOW(shells[parent]),
				      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR |
                                          (modal ? GTK_DIALOG_MODAL : 0),
                                      GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                      NULL );      

    shells[dlgNr] = dialog;
    box = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );
    gtk_box_set_spacing(GTK_BOX(box), 5);    

    arraysize = 0;
    for (i=0;option[i].type != EndMark;i++) {
        arraysize++;   
    }

    table = gtk_table_new(arraysize, 3, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table), 20);
    left = 0;
    top = -1;    

    for (i=0;option[i].type != EndMark;i++) {
	if(option[i].type == -1) continue;
        top++;
        if (top >= height) {
            top = 0;
            left = left + 3;
            gtk_table_resize(GTK_TABLE(table), height, left + 3);   
        }                
        if(!SameRow(&option[i])) {
	    if(SameRow(&option[i+1])) {
		// make sure hbox is always available when we have more options on same row
                hbox = gtk_hbox_new (option[i].type == Button && option[i].textValue, 0);
                if (strcmp(option[i].name, "") == 0 || option[i].type == Label || option[i].type == Button)
                    // for Label and Button name is contained inside option
                    gtk_table_attach_defaults(GTK_TABLE(table), hbox, left, left+3, top, top+1);
                else
                    gtk_table_attach_defaults(GTK_TABLE(table), hbox, left+1, left+3, top, top+1);
	    } else hbox = NULL; //and also make sure no hbox exists if only singl option on row
        }
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
            label = gtk_label_new(option[i].name);
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

            /* width */
            w = option[i].type == Spin || option[i].type == Fractional ? 70 : option[i].max ? option[i].max : 205;
	    if(option[i].type == FileName || option[i].type == PathName) w -= 55;

            if (option[i].type==TextBox && option[i].value > 80){                
                textview = gtk_text_view_new();                
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), option[i].min & T_WRAP ? GTK_WRAP_WORD : GTK_WRAP_NONE);
#ifdef TODO_GTK
		if(option[i].min & T_FILL)  { XtSetArg(args[j], XtNautoFill, True);  j++; }
		if(option[i].min & T_TOP)   { XtSetArg(args[j], XtNtop, XtChainTop); j++;
#endif
                /* add textview to scrolled window so we have vertical scroll bar */
                sw = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                               option[i].min & T_HSCRL ? GTK_POLICY_ALWAYS : GTK_POLICY_AUTOMATIC,
                                               option[i].min & T_VSCRL ? GTK_POLICY_ALWAYS : GTK_POLICY_NEVER);
                gtk_container_add(GTK_CONTAINER(sw), textview);
                gtk_widget_set_size_request(GTK_WIDGET(sw), w, -1);
 
                textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));                
                gtk_widget_set_size_request(textview, -1, option[i].min);
                /* check if label is empty */ 
                if (strcmp(option[i].name,"") != 0) {
                    gtk_table_attach_defaults(GTK_TABLE(table), label, left, left+1, top, top+1);
                    Pack(hbox, table, sw, left+1, left+3, top);
                }
                else {
                    /* no label so let textview occupy all columns */
                    Pack(hbox, table, sw, left, left+3, top);
                } 
                if ( *(char**)option[i].target != NULL )
                    gtk_text_buffer_set_text (textbuffer, *(char**)option[i].target, -1);
                else
                    gtk_text_buffer_set_text (textbuffer, "", -1); 
                option[i].handle = (void*)textbuffer;
                option[i].textValue = (char*)textview;
		if(option[i].choice) { // textviews can request a handler for mouse events in the choice field
		    g_signal_connect(textview, "button-press-event", G_CALLBACK (MemoEvent), (gpointer) &option[i] );
		    g_signal_connect(textview, "button-release-event", G_CALLBACK (MemoEvent), (gpointer) &option[i] );
		    g_signal_connect(textview, "motion-notify-event", G_CALLBACK (MemoEvent), (gpointer) &option[i] );
		}
                break; 
            }

            entry = gtk_entry_new();

            if (option[i].type==Spin || option[i].type==Fractional)
                gtk_entry_set_text (GTK_ENTRY (entry), def);
            else if (currentCps)
                gtk_entry_set_text (GTK_ENTRY (entry), option[i].textValue);
            else if ( *(char**)option[i].target != NULL )
                gtk_entry_set_text (GTK_ENTRY (entry), *(char**)option[i].target);            

            //gtk_entry_set_width_chars (GTK_ENTRY (entry), 18);
            gtk_entry_set_max_length (GTK_ENTRY (entry), w);

            // left, right, top, bottom
            if (strcmp(option[i].name, "") != 0) gtk_table_attach_defaults(GTK_TABLE(table), label, left, left+1, top, top+1);
            //gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, i, i+1);            

            if (option[i].type == Spin) {                
                spinner_adj = (GtkAdjustment *) gtk_adjustment_new (option[i].value, option[i].min, option[i].max, 1.0, 0.0, 0.0);
                spinner = gtk_spin_button_new (spinner_adj, 1.0, 0);
                gtk_table_attach_defaults(GTK_TABLE(table), spinner, left+1, left+3, top, top+1);
                option[i].handle = (void*)spinner;
            }
            else if (option[i].type == FileName || option[i].type == PathName) {
                gtk_table_attach_defaults(GTK_TABLE(table), entry, left+1, left+2, top, top+1);
                button = gtk_button_new_with_label ("Browse");
                gtk_table_attach_defaults(GTK_TABLE(table), button, left+2, left+3, top, top+1);
                g_signal_connect (button, "clicked", G_CALLBACK (BrowseGTK), (gpointer)(intptr_t) i);
                option[i].handle = (void*)entry;                 
            }
            else {
                Pack(hbox, table, entry, left + (strcmp(option[i].name, "") != 0), left+3, top);
                option[i].handle = (void*)entry;
            }                        		
            break;
          case CheckBox:
            checkbutton = gtk_check_button_new_with_label(option[i].name);            
            if(!currentCps) option[i].value = *(Boolean*)option[i].target;
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton), option[i].value);
            gtk_table_attach_defaults(GTK_TABLE(table), checkbutton, left, left+3, top, top+1);                            
            option[i].handle = (void *)checkbutton;            
            break; 
	  case Label:            
            option[i].handle = (void *) (label = gtk_label_new(option[i].name));
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	    if(option[i].min & BORDER) {
		GtkWidget *frame = gtk_frame_new(NULL);
                gtk_container_add(GTK_CONTAINER(frame), label);
		label = frame;
	    }
            Pack(hbox, table, label, left, left+3, top);
	    if(option[i].target) { // allow user to specify event handler for button presses
		gtk_widget_add_events(GTK_WIDGET(label), GDK_BUTTON_PRESS_MASK);
		g_signal_connect(label, "button-press-event", G_CALLBACK(MemoEvent), (gpointer) &option[i]);
	    }
	    break;
          case SaveButton:
          case Button:
            button = gtk_button_new_with_label (option[i].name);

            /* set button color on view board dialog */
            if(option[i].choice && ((char*)option[i].choice)[0] == '#' && !currentCps) {
                gdk_color_parse( *(char**) option[i-1].target, &color );
                gtk_widget_modify_bg ( GTK_WIDGET(button), GTK_STATE_NORMAL, &color );
	    }

            /* set button color on new variant dialog */
            if(option[i].textValue) {
                gdk_color_parse( option[i].textValue, &color );
                gtk_widget_modify_bg ( GTK_WIDGET(button), GTK_STATE_NORMAL, &color );
                gtk_widget_set_sensitive(button, appData.noChessProgram || option[i].value < 0
					 || strstr(first.variants, VariantName(option[i].value)));                 
            }
            
            Pack(hbox, table, button, left, left+1, top);
            g_signal_connect (button, "clicked", G_CALLBACK (GenericCallback), (gpointer)(intptr_t) i + (dlgNr<<16));           
            option[i].handle = (void*)button;            
            break;  
	  case ComboBox:
            label = gtk_label_new(option[i].name);
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            gtk_table_attach_defaults(GTK_TABLE(table), label, left, left+1, top, top+1);

            combobox = gtk_combo_box_new_text();            

            for(j=0;;j++) {
               if (  ((char **) option[i].textValue)[j] == NULL) break;
               gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), ((char **) option[i].choice)[j]);                          
            }

            if(currentCps)
                option[i].choice = (char**) option[i].textValue;
            else {
                for(j=0; option[i].choice[j]; j++) {                
                    if(*(char**)option[i].target && !strcmp(*(char**)option[i].target, ((char**)(option[i].textValue))[j])) break;
                }
                /* If choice is NULL set to first */
                if (option[i].choice[j] == NULL)
                   option[i].value = 0;
                else 
                   option[i].value = j;
            }

            //option[i].value = j + (option[i].choice[j] == NULL);            
            gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), option[i].value); 
            
            Pack(hbox, table, combobox, left+1, left+3, top);
            g_signal_connect(G_OBJECT(combobox), "changed", G_CALLBACK(ComboSelect), (gpointer) (intptr_t) (i + 256*dlgNr));

            option[i].handle = (void*)combobox;
            values[i] = option[i].value;            
            break;
	  case ListBox:
            {
                GtkCellRenderer *renderer;
                GtkTreeViewColumn *column;
                GtkListStore *store;

                option[i].handle = (void *) (list = gtk_tree_view_new());
                gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);
                renderer = gtk_cell_renderer_text_new();
                column = gtk_tree_view_column_new_with_attributes("List Items", renderer, "text", 0, NULL);
                gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
                store = gtk_list_store_new(1, G_TYPE_STRING); // 1 column of text
                gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
                g_object_unref(store);
                LoadListBox(&option[i], "?", -1, -1);
		HighlightListBoxItem(&option[i], 0);

                /* add listbox to scrolled window so we have vertical scroll bar */
                sw = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
                gtk_container_add(GTK_CONTAINER(sw), list);
                gtk_widget_set_size_request(GTK_WIDGET(sw), option[i].max ? option[i].max : -1, option[i].value ? option[i].value : -1);
 
                /* never has label, so let listbox occupy all columns */
                Pack(hbox, table, sw, left, left+3, top);
            }
	    break;
	  case Graph:
	    option[i].handle = (void*) (graph = gtk_drawing_area_new());
            gtk_widget_set_size_request(graph, option[i].max, option[i].value);
//	    gtk_drawing_area_size(graph, option[i].max, option[i].value);
            gtk_table_attach_defaults(GTK_TABLE(table), graph, left, left+3, top, top+1);
            g_signal_connect (graph, "expose-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
	    gtk_widget_add_events(GTK_WIDGET(graph), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
            g_signal_connect (graph, "button-press-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
            g_signal_connect (graph, "button-release-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
            g_signal_connect (graph, "motion-notify-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);

#ifdef TODO_GTK
	    XtAddEventHandler(last, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask, False,
		      (XtEventHandler) GraphEventProc, &option[i]); // mandatory user-supplied expose handler
	    if(option[i].min & SAME_ROW) last = forelast, forelast = lastrow;
#endif
	    option[i].choice = (char**) cairo_image_surface_create (CAIRO_FORMAT_ARGB32, option[i].max, option[i].value); // image buffer
	    break;
#ifdef TODO_GTK
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
#endif
	  case DropDown:
	    msg = _(option[i].name); // write name on the menu button
//	    XtSetArg(args[j], XtNmenuName, XtNewString(option[i].name));  j++;
//	    XtSetArg(args[j], XtNlabel, msg);  j++;
	    option[i].handle = (void*)
		(menuButton = gtk_menu_item_new_with_label(msg));
	    gtk_widget_show(menuButton);
	    option[i].textValue = (char*) (menu = CreateMenuPopup(option + i, i + 256*dlgNr, -1));
	    gtk_menu_item_set_submenu(GTK_MENU_ITEM (menuButton), menu);
	    gtk_menu_bar_append (GTK_MENU_BAR (menuBar), menuButton);

	    break;
	  case BarBegin:
	    menuBar = gtk_menu_bar_new ();
	    gtk_widget_show (menuBar);
	  case BoxBegin:
	    boxStart = i;
	    break;
	  case BarEnd:
            gtk_table_attach_defaults(GTK_TABLE(table), menuBar, left, left+1, top, top+1);
	  case BoxEnd:
//	    XtManageChildren(&form, 1);
//	    SqueezeIntoBox(&option[boxStart], i-boxStart, option[boxStart].max);
	    if(option[i].target) ((ButtonCallback*)option[i].target)(boxStart); // callback that can make sizing decisions
	    break;
	  case Break:
            if(option[i].min & SAME_ROW) top = height; // force next option to start in a new column
            break; 
	default:
	    printf("GenericPopUp: unexpected case in switch. i=%d type=%d name=%s.\n", i, option[i].type, option[i].name);
	    break;
	}        
    }

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                        table, TRUE, TRUE, 0);    

    /* Show dialog */
    gtk_widget_show_all( dialog );    

    /* hide OK/cancel buttons */
    if((option[i].min & 2)) {
        actionarea = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
        gtk_widget_hide(actionarea);
    }

    g_signal_connect (dialog, "response",
                      G_CALLBACK (GenericPopDown),
                      (gpointer)(intptr_t) dlgNr);
    g_signal_connect (dialog, "delete-event",
                      G_CALLBACK (GenericPopDown),
                      (gpointer)(intptr_t) dlgNr);
    shellUp[dlgNr]++;

#ifdef TODO_GTK
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
		XtSetArg(args[j], XtNsensitive, appData.noChessProgram || option[i].value < 0
					 || strstr(first.variants, VariantName(option[i].value))); j++;
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
    if(dlgNr && wp[dlgNr] && wp[dlgNr]->width > 0) { // if persistent window-info available, reposition
	j = 0;
	XtSetArg(args[j], XtNheight, (Dimension) (wp[dlgNr]->height));  j++;
	XtSetArg(args[j], XtNwidth,  (Dimension) (wp[dlgNr]->width));  j++;
	XtSetArg(args[j], XtNx, (Position) (wp[dlgNr]->x));  j++;
	XtSetArg(args[j], XtNy, (Position) (wp[dlgNr]->y));  j++;
	XtSetValues(popup, args, j);
    }
    RaiseWindow(dlgNr);
#endif
    return 1; // tells caller he must do initialization (e.g. add specific event handlers)
}

/* function called when the data to Paste is ready */
#ifdef TODO_GTK
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
#endif

void
SendText (int n)
{
#ifdef TODO_GTK
    char *p = (char*) textOptions[n].choice;
    if(strstr(p, "$name")) {
	XtGetSelectionValue(menuBarWidget,
	  XA_PRIMARY, XA_STRING,
	  /* (XtSelectionCallbackProc) */ SendTextCB,
	  (XtPointer) (intptr_t) n, /* client_data passed to PastePositionCB */
	  CurrentTime
	);
    } else SendString(p);
#endif
}

void
SetInsertPos (Option *opt, int pos)
{
#ifdef TODO_GTK
    Arg args[16];
    XtSetArg(args[0], XtNinsertPosition, pos);
    XtSetValues(opt->handle, args, 1);
//    SetFocus(opt->handle, shells[InputBoxDlg], NULL, False); // No idea why this does not work, and the following is needed:
//    XSetInputFocus(xDisplay, XtWindow(opt->handle), RevertToPointerRoot, CurrentTime);
#endif
}

#ifdef TODO_GTK
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
#endif

void
HardSetFocus (Option *opt)
{
#ifdef TODO_GTK
    XSetInputFocus(xDisplay, XtWindow(opt->handle), RevertToPointerRoot, CurrentTime);
#endif
}


