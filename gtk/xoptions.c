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

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#ifdef OSXAPP
#  include <gtkmacintegration/gtkosxapplication.h>
#endif

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

void GetWidgetTextGTK(GtkWidget *w, char **buf)
{
    GtkTextIter start;
    GtkTextIter end;

    if (GTK_IS_ENTRY(w)) {
	*buf = (char *) gtk_entry_get_text(GTK_ENTRY (w));
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
    if(opt->type == Button) // Chat window uses this routine for changing button labels
	gtk_button_set_label(opt->handle, buf);
    else
	gtk_label_set_text(opt->handle, buf);
}

void
SetDialogTitle (DialogClass dlg, char *title)
{
    gtk_window_set_title(GTK_WINDOW(shells[dlg]), title);
}

void
SetWidgetFont (GtkWidget *w, char **s)
{
    PangoFontDescription *pfd;
    if (!s || !*s || !**s) return; // uses no font, no font spec or empty font spec
    pfd = pango_font_description_from_string(*s);
    gtk_widget_modify_font(w, pfd);
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
    GtkWidget *list = (GtkWidget *) (opt->handle);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    GtkTreePath *path = gtk_tree_path_new_from_indices(index, -1);
    gtk_tree_selection_select_path(selection, path);
    if(scroll) gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(list), path, NULL, 0, 0, 0);
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

void
ScrollToCursor (Option *opt, int caretPos)
{
    static GtkTextIter iter;
    GtkTextMark *mark = gtk_text_buffer_get_mark((GtkTextBuffer *) opt->handle, "scrollmark");
    gtk_text_buffer_get_iter_at_offset((GtkTextBuffer *) opt->handle, &iter, caretPos);
    gtk_text_buffer_move_mark((GtkTextBuffer *) opt->handle, mark, &iter);
    gtk_text_view_scroll_to_mark((GtkTextView *) opt->textValue, mark, 0.0, 0, 0.5, 0.5);
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
    if(dlg) gtk_window_present(GTK_WINDOW(shells[dlg]));
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
#ifdef OSXAPP
	if(!strcmp(msg, "Quit ")) continue;             // Quit item will appear automatically in App menu
	if(!strcmp(msg, "About XBoard")) msg = "About"; // 'XBoard' will be appended automatically when moved to App menu 1st item
#endif
        if(!strcmp(msg, "ICS Input Box")) { mb[i].handle = NULL; continue; } // suppress ICS Input Box in GTK
	if(strcmp(msg, "----")) { //
	  if(!(opt->min & NO_GETTEXT)) msg = _(msg);
	  if(mb[i].handle) {
	    entry = gtk_check_menu_item_new_with_label(msg); // should be used for items that can be checkmarked
	    if(mb[i].handle == RADIO) gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(entry), True);
	  } else
	    entry = gtk_menu_item_new_with_label(msg);
	  gtk_signal_connect_object (GTK_OBJECT (entry), "activate", GTK_SIGNAL_FUNC(MenuSelect), (gpointer) (intptr_t) ((n<<16)+i));
	  if(mb[i].accel) {
	    guint accelerator_key;
	    GdkModifierType accelerator_mods;

	    gtk_accelerator_parse(mb[i].accel, &accelerator_key, &accelerator_mods);
#ifdef OSXAPP
   	    if(accelerator_mods & GDK_CONTROL_MASK) {  // in OSX use Meta where Linux uses Ctrl
		accelerator_mods &= ~GDK_CONTROL_MASK; // clear Ctrl flag
		accelerator_mods |= GDK_META_MASK;     // set Meta flag
	    }
#endif
	    gtk_widget_add_accelerator (GTK_WIDGET(entry), "activate",GtkAccelerators,
					accelerator_key, accelerator_mods, GTK_ACCEL_VISIBLE);
	  }
	} else entry = gtk_separator_menu_item_new();
	gtk_widget_show(entry);
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

Option *icsBox; // kludge to distinguish type-in callback from input-box callback

void
CursorAtEnd (Option *opt)
{
    gtk_editable_set_position(opt->handle, -1);
}

static gboolean
ICSKeyEvent (int keyval)
{   // TODO_GTK: arrow-handling should really be integrated in type-in proc, and this should be a backe-end OK handler
    switch(keyval) {
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
      case 'n':       return (controlState && IcsHist(14, opt, dlg));
      case 'o':       return (controlState && IcsHist(15, opt, dlg));
      case GDK_Tab:   IcsHist(10, opt, dlg); break;
      case GDK_Up:     IcsHist(1, opt, dlg); break;
      case GDK_Down:  IcsHist(-1, opt, dlg); break;
      case GDK_Return:
	if(GenericReadout(dialogOptions[dlg], -1)) PopDown(dlg);
	break;
      case GDK_Escape:
	if(!IcsHist(33, opt, dlg)) PopDown(dlg);
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

static char **names;
static int curFG, curBG, curAttr;
static GdkColor backgroundColor;

void
SetTextColor(char **cnames, int fg, int bg, int attr)
{
    if(fg < 0) fg = 0; if(bg < 0) bg = 7;
    names = cnames; curFG = fg; curBG = bg, curAttr = attr;
    if(attr == -2) { // background color of ICS console.
	gdk_color_parse(cnames[bg&7], &backgroundColor);
	curAttr = 0;
    }
}

void
AppendColorized (Option *opt, char *s, int count)
{
    static GtkTextIter end;
    static GtkTextTag *fgTags[8], *bgTags[8], *font, *bold, *normal, *attr = NULL;

    if(!font) {
	font = gtk_text_buffer_create_tag(opt->handle, NULL, "font", appData.icsFont, NULL);
	gtk_widget_modify_base(GTK_WIDGET(opt->textValue), GTK_STATE_NORMAL, &backgroundColor);
    }

    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(opt->handle), &end);

    if(names) {
      if(curAttr == 1) {
	if(!bold) bold = gtk_text_buffer_create_tag(opt->handle, NULL, "weight", PANGO_WEIGHT_BOLD, NULL);
        attr = bold;
      } else {
	if(!normal) normal = gtk_text_buffer_create_tag(opt->handle, NULL, "weight", PANGO_WEIGHT_NORMAL, NULL);
        attr = normal;
      }
      if(!fgTags[curFG]) {
	fgTags[curFG] = gtk_text_buffer_create_tag(opt->handle, NULL, "foreground", names[curFG], NULL);
      }
      if(!bgTags[curBG]) {
	bgTags[curBG] = gtk_text_buffer_create_tag(opt->handle, NULL, "background", names[curBG], NULL);
      }
      gtk_text_buffer_insert_with_tags(opt->handle, &end, s, count, fgTags[curFG], bgTags[curBG], font, attr, NULL);
    } else
      gtk_text_buffer_insert_with_tags(opt->handle, &end, s, count, font, NULL);

}

void
Show (Option *opt, int hide)
{
    if(hide) gtk_widget_hide(opt->handle);
    else     gtk_widget_show(opt->handle);
}

int
ShiftKeys ()
{   // bassic primitive for determining if modifier keys are pressed
    return 3*(shiftState != 0) + 0xC*(controlState != 0); // rely on what last mouse button press left us
}

static gboolean
GameListEvent(GtkWidget *widget, GdkEvent *event, gpointer gdata)
{
    int n = (intptr_t) gdata;

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
    Option *memo = (Option *) gdata;
    MemoCallback *userHandler = (MemoCallback *) memo->choice;
    GdkEventButton *bevent = (GdkEventButton *) event;
    GdkEventMotion *mevent = (GdkEventMotion *) event;
    GtkTextIter start, end;
    String val = NULL;
    gboolean res;
    gint index = 0, x, y;

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
	    if(memo->type == Label) { // only clock widgets use this
		((ButtonCallback*) memo->target)(button == 1 ? memo->value : -memo->value);
		return TRUE;
	    }
	    if(memo->value == 250 // kludge to recognize ICS Console and Chat panes
	     && gtk_text_buffer_get_selection_bounds(memo->handle, NULL, NULL) ) {
printf("*** selected\n");
	        gtk_text_buffer_get_selection_bounds(memo->handle, &start, &end); // only return selected text
		index = -1; // kludge to indicate omething was selected
	    } else {
// GTK_TODO: is this really the most efficient way to get the character at the mouse cursor???
		gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_WIDGET, w, h, &x, &y);
		gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &start, x, y);
		gtk_text_buffer_place_cursor(memo->handle, &start);
		/* get cursor position into index */
		g_object_get(memo->handle, "cursor-position", &index, NULL);
		/* take complete contents */
		gtk_text_buffer_get_start_iter (memo->handle, &start);
		gtk_text_buffer_get_end_iter (memo->handle, &end);
	    }
	    /* get text from textbuffer */
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
      case 3: // input box
	icsBox = opt;
      case 2: // move type-in
	g_signal_connect(opt->handle, "key-press-event", G_CALLBACK (TypeInProc), (gpointer) (dlg<<16 | (opt - dialogOptions[dlg])));
	break;
      case 5: // game list
	g_signal_connect(opt->handle, "button-press-event", G_CALLBACK (GameListEvent), (gpointer) 0 );
      case 4: // game-list filter
	g_signal_connect(opt->handle, "key-press-event", G_CALLBACK (GameListEvent), (gpointer) (intptr_t) nr );
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
    NULL, &wpComment, &wpTags, &wpTextMenu, NULL, &wpConsole, &wpDualBoard, &wpMoveHistory, &wpGameList, &wpEngineOutput, &wpEvalGraph,
    NULL, NULL, NULL, NULL, &wpMain
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
    if(n && wp[n]) { // remember position
	GetActualPlacement(shells[n], wp[n]);
    }

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
    if(browserUp || dialogError && dlg != FatalDlg || dlg == MasterDlg && shellUp[TransientDlg])
	return True; // prevent closing dialog when it has an open file-browse, transient or error-popup daughter
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
    int button=10, f=1, sizing=0;
    Option *opt, *graph = (Option *) gdata;
    PointerCallback *userHandler = graph->target;
    GdkEventExpose *eevent = (GdkEventExpose *) event;
    GdkEventButton *bevent = (GdkEventButton *) event;
    GdkEventMotion *mevent = (GdkEventMotion *) event;
    GdkEventScroll *sevent = (GdkEventScroll *) event;
    GtkAllocation a;
    cairo_t *cr;

//    if (!XtIsRealized(widget)) return;

    switch(event->type) {
	case GDK_EXPOSE: // make handling of expose events generic, just copying from memory buffer (->choice) to display (->textValue)
	    /* Get window size */
	    gtk_widget_get_allocation(widget, &a);
	    w = a.width; h = a.height;
//printf("expose %dx%d @ (%d,%d): %dx%d @(%d,%d)\n", w, h, a.x, a.y, eevent->area.width, eevent->area.height, eevent->area.x, eevent->area.y);
#ifdef TODO_GTK
	    j = 0;
	    XtSetArg(args[j], XtNwidth, &w); j++;
	    XtSetArg(args[j], XtNheight, &h); j++;
	    XtGetValues(widget, args, j);
#endif
	    if(w < graph->max || w > graph->max + 1 || h != graph->value) { // use width fudge of 1 pixel
		if(eevent->count >= 0) { // suppress sizing on expose for ordered redraw in response to sizing.
		    sizing = 1;
		    graph->max = w; graph->value = h; // note: old values are kept if we we don't exceed width fudge
		}
	    } else w = graph->max;
	    if(sizing && eevent->count > 0) { graph->max = 0; return; } // don't bother if further exposure is pending during resize
#ifdef TODO_GTK
	    if(!graph->textValue || sizing) { // create surfaces of new size for display widget
		if(graph->textValue) cairo_surface_destroy((cairo_surface_t *)graph->textValue);
		graph->textValue = (char*) cairo_xlib_surface_create(xDisplay, XtWindow(widget), DefaultVisual(xDisplay, 0), w, h);
	    }
#endif
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
	    w = eevent->area.width;
	    if(eevent->area.x + w > graph->max) w--; // cut off fudge pixel
	    cr = gdk_cairo_create(((GtkWidget *) (graph->handle))->window);
	    cairo_set_source_surface(cr, (cairo_surface_t *) graph->choice, 0, 0);
//cairo_set_source_rgb(cr, 1, 0, 0);
	    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	    cairo_rectangle(cr, eevent->area.x, eevent->area.y, w, eevent->area.height);
	    cairo_fill(cr);
	    cairo_destroy(cr);
	default:
	    return;
	case GDK_SCROLL:
	    if(sevent->direction == GDK_SCROLL_UP) button = 4;
	    if(sevent->direction == GDK_SCROLL_DOWN) button = 5;
	    break;
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

    char fileext[MSG_SIZ];

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
    if (currentOption[opt_i].textValue != NULL)
      {
        char *q, *p = currentOption[opt_i].textValue;
        gtk_file_filter_set_name (gtkfilter, p);
        while(*p) {
          snprintf(fileext, MSG_SIZ, "*%s", p);
          while(*p) if(*p++ == ' ')  break;
          for(q=fileext; *q; q++) if(*q == ' ') { *q = NULLCHAR; break; }
          gtk_file_filter_add_pattern(gtkfilter, fileext);
        }
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

gboolean
ListCallback (GtkWidget *widget, GdkEventButton *event, gpointer gdata)
{
    int n = (intptr_t) gdata & 0xFFFF;
    int dlg = (intptr_t) gdata >> 16;
    Option *opt = dialogOptions[dlg] + n;

    if(event->type != GDK_2BUTTON_PRESS || event->button != 1) return FALSE;
    ((ListBoxCallback*) opt->textValue)(n, SelectedListBoxItem(opt));
    return TRUE;
}

#ifdef TODO_GTK
// This is needed for color pickers?
static char *oneLiner  =
   "<Key>Return: redraw-display() \n \
    <Key>Tab: TabProc() \n ";
#endif

#ifdef TODO_GTK
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
#endif

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
TableWidth (Option *opt)
{   // Hideous work-around! If the table is 3 columns, but 2 & 3 are always occupied together, the fixing of the width of column 1 does not work
    while(opt->type != EndMark && opt->type != Break)
	if(opt->type == FileName || opt->type == PathName || opt++->type == BarBegin) return 3; // This table needs browse button
    return 2; // no browse button;
}

static int
SameRow (Option *opt)
{
    return (opt->min & SAME_ROW && (opt->type == Button || opt->type == SaveButton || opt->type == Label
				 || opt->type == ListBox || opt->type == BoxBegin || opt->type == Icon || opt->type == Graph));
}

static void
Pack (GtkWidget *hbox, GtkWidget *table, GtkWidget *entry, int left, int right, int top, GtkAttachOptions vExpand)
{
    if(hbox) gtk_box_pack_start(GTK_BOX (hbox), entry, TRUE, TRUE, 0);
    else     gtk_table_attach(GTK_TABLE(table), entry, left, right, top, top+1,
				GTK_FILL | GTK_EXPAND, GTK_FILL | vExpand, 2, 1);
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
    GtkWidget *oldHbox = NULL, *hbox = NULL;
    GtkWidget *pane = NULL;
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
    GtkWidget *menuBar = NULL;
    GtkWidget *menu;

    int i, j, arraysize, left, top, height=999, width=1, boxStart=0, breakType = 0, r;
    char def[MSG_SIZ], *msg, engineDlg = (currentCps != NULL && dlgNr != BrowserDlg);
    gboolean expandable = FALSE;

    if(dlgNr < PromoDlg && shellUp[dlgNr]) return 0; // already up

    if(dlgNr && dlgNr < PromoDlg && shells[dlgNr]) { // reusable, and used before (but popped down)
        gtk_widget_show(shells[dlgNr]);
        shellUp[dlgNr] = True;
	if(wp[dlgNr]) gtk_window_move(GTK_WINDOW(shells[dlgNr]), wp[dlgNr]->x, wp[dlgNr]->y);
        return 0;
    }
    if(dlgNr == TransientDlg && parent == BoardWindow && shellUp[MasterDlg]) parent = MasterDlg; // MasterDlg can always take role of main window

    dialogOptions[dlgNr] = option; // make available to callback
    // post currentOption globally, so Spin and Combo callbacks can already use it
    // WARNING: this kludge does not work for persistent dialogs, so that these cannot have spin or combo controls!
    currentOption = option;

    if(engineDlg) { // Settings popup for engine: format through heuristic
        int n = currentCps->nrOptions;
//        if(n > 50) width = 4; else if(n>24) width = 2; else width = 1;
	width = n / 20 + 1;
        height = n / width + 1;
if(appData.debugMode) printf("n=%d, h=%d, w=%d\n",n,height,width);
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

    if(topLevel)
      {
	dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	box = gtk_vbox_new(FALSE,0);
	gtk_container_add (GTK_CONTAINER (dialog), box);
      }
    else
      {
	dialog = gtk_dialog_new_with_buttons( title,
					      GTK_WINDOW(shells[parent]),
					      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR |
					      (modal ? GTK_DIALOG_MODAL : 0),
					      GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					      NULL );
	box = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );
      }

    shells[dlgNr] = dialog;
//    gtk_box_set_spacing(GTK_BOX(box), 5);

    arraysize = 0;
    for (i=0;option[i].type != EndMark;i++) {
        arraysize++;
    }

    table = gtk_table_new(arraysize, r=TableWidth(option), FALSE);
    left = 0;
    top = -1;

    for (i=0;option[i].type != EndMark;i++) {
	if(option[i].type == Skip) continue;
        top++;
//printf("option =%2d, top =%2d\n", i, top);
        if (top >= height || breakType) {
            gtk_table_resize(GTK_TABLE(table), top - (breakType != 0), r);
	    if(!pane) { // multi-column: put tables in intermediate hbox
		if(breakType & SAME_ROW || engineDlg)
		    pane =  gtk_hbox_new (FALSE, 0);
		else
		    pane =  gtk_vbox_new (FALSE, 0);
		gtk_box_set_spacing(GTK_BOX(pane), 5 + 5*breakType);
		gtk_box_pack_start (GTK_BOX (/*GTK_DIALOG (dialog)->vbox*/box), pane, TRUE, TRUE, 0);
	    }
	    gtk_box_pack_start (GTK_BOX (pane), table, expandable, TRUE, 0);
	    table = gtk_table_new(arraysize - i, r=TableWidth(option + i), FALSE);
            top = breakType = 0; expandable = FALSE;
        }
        if(!SameRow(&option[i])) {
	    if(SameRow(&option[i+1])) {
		GtkAttachOptions x = GTK_FILL;
		// make sure hbox is always available when we have more options on same row
                hbox = gtk_hbox_new (option[i].type == Button && option[i].textValue || option[i].type == Graph, 0);
		if(!currentCps && option[i].value > 80) x |= GTK_EXPAND; // only vertically extended widgets should size vertically
                if (strcmp(option[i].name, "") == 0 || option[i].type == Label || option[i].type == Button)
                    // for Label and Button name is contained inside option
                    gtk_table_attach(GTK_TABLE(table), hbox, left, left+r, top, top+1, GTK_FILL | GTK_EXPAND, x, 2, 1);
                else
                    gtk_table_attach(GTK_TABLE(table), hbox, left+1, left+r, top, top+1, GTK_FILL | GTK_EXPAND, x, 2, 1);
	    } else hbox = NULL; //and also make sure no hbox exists if only singl option on row
        } else top--;
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
                GtkTextIter iter;
                expandable = TRUE;
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
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_OUT);

                textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                /* check if label is empty */
                if (strcmp(option[i].name,"") != 0) {
                    gtk_table_attach(GTK_TABLE(table), label, left, left+1, top, top+1, GTK_FILL, GTK_FILL, 2, 1);
                    Pack(hbox, table, sw, left+1, left+r, top, 0);
                }
                else {
                    /* no label so let textview occupy all columns */
                    Pack(hbox, table, sw, left, left+r, top, GTK_EXPAND);
                }
                SetWidgetFont(textview, option[i].font);
                if ( *(char**)option[i].target != NULL )
                    gtk_text_buffer_set_text (textbuffer, *(char**)option[i].target, -1);
                else
                    gtk_text_buffer_set_text (textbuffer, "", -1);
                option[i].handle = (void*)textbuffer;
                option[i].textValue = (char*)textview;
                gtk_text_buffer_get_iter_at_offset(textbuffer, &iter, -1);
                gtk_text_buffer_create_mark(textbuffer, "scrollmark", &iter, FALSE); // permanent mark
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
            if (strcmp(option[i].name, "") != 0)
                gtk_table_attach(GTK_TABLE(table), label, left, left+1, top, top+1, GTK_FILL, GTK_FILL, 2, 1); // leading names do not expand

            if (option[i].type == Spin) {
                spinner_adj = (GtkAdjustment *) gtk_adjustment_new (option[i].value, option[i].min, option[i].max, 1.0, 0.0, 0.0);
                spinner = gtk_spin_button_new (spinner_adj, 1.0, 0);
                gtk_table_attach(GTK_TABLE(table), spinner, left+1, left+r, top, top+1, GTK_FILL | GTK_EXPAND, GTK_FILL, 2, 1);
                option[i].handle = (void*)spinner;
            }
            else if (option[i].type == FileName || option[i].type == PathName) {
                gtk_table_attach(GTK_TABLE(table), entry, left+1, left+2, top, top+1, GTK_FILL | GTK_EXPAND, GTK_FILL, 2, 1);
                button = gtk_button_new_with_label ("Browse");
                gtk_table_attach(GTK_TABLE(table), button, left+2, left+r, top, top+1, GTK_FILL, GTK_FILL, 2, 1); // Browse button does not expand
                g_signal_connect (button, "clicked", G_CALLBACK (BrowseGTK), (gpointer)(intptr_t) i);
                option[i].handle = (void*)entry;
            }
            else {
                Pack(hbox, table, entry, left + (strcmp(option[i].name, "") != 0), left+r, top, 0);
                option[i].handle = (void*)entry;
            }
            break;
          case CheckBox:
            checkbutton = gtk_check_button_new_with_label(option[i].name);
            if(!currentCps) option[i].value = *(Boolean*)option[i].target;
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton), option[i].value);
            gtk_table_attach(GTK_TABLE(table), checkbutton, left, left+r, top, top+1, GTK_FILL | GTK_EXPAND, GTK_FILL, 2, 0);
            option[i].handle = (void *)checkbutton;
            break;
	  case Icon:
            option[i].handle = (void *) (label = gtk_image_new_from_pixbuf(NULL));
            gtk_widget_set_size_request(label, option[i].max ? option[i].max : -1, -1);
            Pack(hbox, table, label, left, left+2, top, 0);
            break;
	  case Label:
            option[i].handle = (void *) (label = gtk_label_new(option[i].name));
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            SetWidgetFont(label, option[i].font);
	    if(option[i].min & BORDER) {
		GtkWidget *frame = gtk_frame_new(NULL);
                gtk_container_add(GTK_CONTAINER(frame), label);
		label = frame;
	    }
            gtk_widget_set_size_request(label, option[i].max ? option[i].max : -1, -1);
	    if(option[i].target) { // allow user to specify event handler for button presses
		button = gtk_event_box_new();
                gtk_container_add(GTK_CONTAINER(button), label);
		label = button;
		gtk_widget_add_events(GTK_WIDGET(label), GDK_BUTTON_PRESS_MASK);
		g_signal_connect(label, "button-press-event", G_CALLBACK(MemoEvent), (gpointer) &option[i]);
		gtk_widget_set_sensitive(label, TRUE);
	    }
            Pack(hbox, table, label, left, left+3, top, 0);
	    break;
          case SaveButton:
          case Button:
            button = gtk_button_new_with_label (option[i].name);
            SetWidgetFont(gtk_bin_get_child(GTK_BIN(button)), option[i].font);

            /* set button color on view board dialog */
            if(option[i].choice && ((char*)option[i].choice)[0] == '#' && !currentCps) {
                gdk_color_parse( *(char**) option[i-1].target, &color );
                gtk_widget_modify_bg ( GTK_WIDGET(button), GTK_STATE_NORMAL, &color );
	    }

            /* set button color on new variant dialog */
            if(option[i].textValue) {
                static char *b = "Bold";
                gdk_color_parse( option[i].textValue, &color );
                gtk_widget_modify_bg ( GTK_WIDGET(button), GTK_STATE_NORMAL, &color );
                gtk_widget_set_sensitive(button, option[i].value >= 0 && (appData.noChessProgram
					 || strstr(first.variants, VariantName(option[i].value))));
                if(engineVariant[100] ? !strcmp(engineVariant+100, option[i].name) : 
                   gameInfo.variant ? option[i].value == gameInfo.variant : !strcmp(option[i].name, "Normal"))
                    SetWidgetFont(gtk_bin_get_child(GTK_BIN(button)), &b);
            }

            Pack(hbox, table, button, left, left+1, top, 0);
            g_signal_connect (button, "clicked", G_CALLBACK (GenericCallback), (gpointer)(intptr_t) i + (dlgNr<<16));
            option[i].handle = (void*)button;
            break;
	  case ComboBox:
            label = gtk_label_new(option[i].name);
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            gtk_table_attach(GTK_TABLE(table), label, left, left+1, top, top+1, GTK_FILL, GTK_FILL, 2, 1);

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

            Pack(hbox, table, combobox, left+1, left+r, top, 0);
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
                SetWidgetFont(option[i].handle, option[i].font);
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
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_OUT);

                if(option[i].textValue) // generic callback for double-clicking listbox item
                    g_signal_connect(list, "button-press-event", G_CALLBACK(ListCallback), (gpointer) (intptr_t) (dlgNr<<16 | i) );

                /* never has label, so let listbox occupy all columns */
                Pack(hbox, table, sw, left, left+r, top, GTK_EXPAND);
                expandable = TRUE;
            }
	    break;
	  case Graph:
	    option[i].handle = (void*) (graph = gtk_drawing_area_new());
            gtk_widget_set_size_request(graph, option[i].max, option[i].value);
	    if(0){ GtkAllocation a;
		a.x = 0; a.y = 0; a.width = option[i].max, a.height = option[i].value;
		gtk_widget_set_allocation(graph, &a);
	    }
            g_signal_connect (graph, "expose-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
	    gtk_widget_add_events(GTK_WIDGET(graph), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
            g_signal_connect (graph, "button-press-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
            g_signal_connect (graph, "button-release-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
            g_signal_connect (graph, "motion-notify-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
            g_signal_connect (graph, "scroll-event", G_CALLBACK (GraphEventProc), (gpointer) &option[i]);
	    if(option[i].min & FIX_H) { // logo
		GtkWidget *frame = gtk_aspect_frame_new(NULL, 0.5, 0.5, option[i].max/(float)option[i].value, FALSE);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
                gtk_container_add(GTK_CONTAINER(frame), graph);
		graph = frame;
	    }
            Pack(hbox, table, graph, left, left+r, top, GTK_EXPAND);
            expandable = TRUE;

#ifdef TODO_GTK
	    if(option[i].min & SAME_ROW) last = forelast, forelast = lastrow;
#endif
	    option[i].choice = (char**) cairo_image_surface_create (CAIRO_FORMAT_ARGB32, option[i].max, option[i].value); // image buffer
	    break;
#ifdef TODO_GTK
	  case PopUp: // note: used only after Graph, so 'last' refers to the Graph widget
	    option[i].handle = (void*) CreateComboPopup(last, option + i, i + 256*dlgNr, TRUE, option[i].value);
	    break;
#endif
	  case DropDown:
	    top--;
	    msg = _(option[i].name); // write name on the menu button
#ifndef OSXAPP
	    if(tinyLayout) { strcpy(def, msg); def[tinyLayout] = NULLCHAR; msg = def; } // clip menu text to keep menu bar small
#endif
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
	    boxStart = i;
	    break;
	  case BoxBegin:
	    option[i+1].min |= SAME_ROW; // kludge to suppress allocation of new hbox
	    oldHbox = hbox;
	    option[i].handle = (void*) (hbox = gtk_hbox_new(FALSE, 0)); // hbox to collect buttons
	    gtk_box_pack_start(GTK_BOX (oldHbox), hbox, FALSE, TRUE, 0); // *** Beware! Assumes button bar always on same row with other! ***
//            gtk_table_attach(GTK_TABLE(table), hbox, left+2, left+3, top, top+1, GTK_FILL | GTK_SHRINK, GTK_FILL, 2, 1);
	    boxStart = i;
	    break;
	  case BarEnd:
	    top--;
#ifndef OSXAPP
            gtk_table_attach(GTK_TABLE(table), menuBar, left, left+r, top, top+1, GTK_FILL | GTK_EXPAND, GTK_FILL, 2, 1);

	    if(option[i].target) ((ButtonCallback*)option[i].target)(boxStart); // callback that can make sizing decisions
#else
	    top--; // in OSX menu bar is not put in window, so also don't count it
	    {   // in stead, offer it to OSX, and move About item to top of App menu
		GtkosxApplication *theApp = g_object_new(GTKOSX_TYPE_APPLICATION, NULL);
		extern MenuItem helpMenu[]; // oh, well... Adding items in help menu breaks this anyway
		gtk_widget_hide (menuBar);
		gtkosx_application_set_menu_bar(theApp, GTK_MENU_SHELL(menuBar));
		gtkosx_application_insert_app_menu_item(theApp, GTK_MENU_ITEM(helpMenu[8].handle), 0); // hack
		gtkosx_application_sync_menubar(theApp);
	    }
#endif
	    break;
	  case BoxEnd:
//	    XtManageChildren(&form, 1);
//	    SqueezeIntoBox(&option[boxStart], i-boxStart, option[boxStart].max);
	    hbox = oldHbox; top--;
	    if(option[i].target) ((ButtonCallback*)option[i].target)(boxStart); // callback that can make sizing decisions
	    break;
	  case Break:
            breakType = option[i].min & SAME_ROW | BORDER; // kludge to flag we must break
	    option[i].handle = table;
            break;

	  case PopUp:
	    top--;
            break;
	default:
	    printf("GenericPopUp: unexpected case in switch. i=%d type=%d name=%s.\n", i, option[i].type, option[i].name);
	    break;
	}
    }

    gtk_table_resize(GTK_TABLE(table), top+1, r);
    if(dlgNr == BoardWindow && appData.fixedSize) { // inhibit sizing
	GtkWidget *h = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (h), table, TRUE, FALSE, 2);
	table = h;
    }
    if(pane)
	gtk_box_pack_start (GTK_BOX (pane), table, expandable, TRUE, 0);
    else
	gtk_box_pack_start (GTK_BOX (/*GTK_DIALOG (dialog)->vbox*/box), table, TRUE, TRUE, 0);

    option[i].handle = (void *) table; // remember last table in EndMark handle (for hiding Engine-Output pane).

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_NONE);

    /* Show dialog */
    gtk_widget_show_all( dialog );

    /* hide OK/cancel buttons */
    if(!topLevel)
      {
	if((option[i].min & NO_OK)) {
	  actionarea = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
	  gtk_widget_hide(actionarea);
	} else if((option[i].min & NO_CANCEL)) {
	  button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_REJECT);
	  gtk_widget_hide(button);
	}
        g_signal_connect (dialog, "response",
                      G_CALLBACK (GenericPopDown),
                      (gpointer)(intptr_t) dlgNr);
      }

    g_signal_connect (dialog, "delete-event",
                      G_CALLBACK (GenericPopDown),
                      (gpointer)(intptr_t) dlgNr);
    shellUp[dlgNr]++;

    if(dlgNr && wp[dlgNr]) { // if persistent window-info available, reposition
      if(wp[dlgNr]->x > 0 && wp[dlgNr]->y > 0)
	gtk_window_move(GTK_WINDOW(dialog), wp[dlgNr]->x, wp[dlgNr]->y);
      if(wp[dlgNr]->width > 0 && wp[dlgNr]->height > 0)
	gtk_window_resize(GTK_WINDOW(dialog), wp[dlgNr]->width, wp[dlgNr]->height);
    }

    for(i=0; option[i].type != EndMark; i++) if(option[i].type == Graph)
	gtk_widget_set_size_request(option[i].handle, -1, -1); // remove size requests after realization, so user can shrink

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
    char *p = (char*) textOptions[n].choice;
#ifdef TODO_GTK
    if(strstr(p, "$name")) {
	XtGetSelectionValue(menuBarWidget,
	  XA_PRIMARY, XA_STRING,
	  /* (XtSelectionCallbackProc) */ SendTextCB,
	  (XtPointer) (intptr_t) n, /* client_data passed to PastePositionCB */
	  CurrentTime
	);
    } else
#endif
    SendString(p);
}

void
SetInsertPos (Option *opt, int pos)
{
    if(opt->value > 80) ScrollToCursor(opt, pos);
    else gtk_editable_set_position(GTK_EDITABLE(opt->handle), pos);
}

void
HardSetFocus (Option *opt, DialogClass dlg)
{
    FocusOnWidget(opt, dlg);
}
