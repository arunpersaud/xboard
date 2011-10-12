/*
 * New (WinBoard-style) Move history for XBoard
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
#include <stdlib.h>
#include <malloc.h>

#include <gtk/gtk.h>

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
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

// templates for calls into back-end
void RefreshMemoContent P((void));
void MemoContentUpdated P((void));
void FindMoveByCharIndex P(( int char_index ));

int AppendText P((Option *opt, char *s));
int GenericPopUp P((Option *option, char *title, int dlgNr));
void MarkMenu P((char *item, int dlgNr));
void GetWidgetText P((Option *opt, char **buf));
GtkWidget *GetTextView P((GtkWidget *dialog));

extern Option historyOptions[];
extern Widget shells[10];
extern GtkWidget *shellsGTK[10];
extern Boolean shellUp[10];

// ------------- low-level front-end actions called by MoveHistory back-end -----------------

void HighlightMove( int from, int to, Boolean highlight )
{    
    GtkTextIter bound;
    GtkTextIter insert;

    if (!highlight) return;

    /* for lack of a better method, use selection for highighting */
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(historyOptions[0].handle), &bound);
    gtk_text_iter_set_offset(&bound, from);

    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(historyOptions[0].handle), &insert);
    gtk_text_iter_set_offset(&insert, to);     
    gtk_text_buffer_select_range(GTK_TEXT_BUFFER(historyOptions[0].handle), &insert, &bound);    
}

void ClearHistoryMemo()
{    
    ClearTextWidget(historyOptions[0].handle);
}

// the bold argument says 0 = normal, 1 = bold typeface
// the colorNr argument says 0 = font-default, 1 = gray
int AppendToHistoryMemo( char * text, int bold, int colorNr )
{
    Arg args[10];
    return AppendText(&historyOptions[0], text); // for now ignore bold & color stuff, as Xaw cannot handle that
}

void ScrollToCurrent(int caretPos)
{
    Arg args[10];
    char *s;
    int len;
    GtkTextIter iter;

    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(historyOptions[0].handle), &iter);
    gtk_text_iter_set_offset(&iter, caretPos);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(GetTextView(shellsGTK[7])), &iter, 0.1, FALSE, 0.5, 0.5);
    return;
}


// ------------------------------ callbacks --------------------------

char *historyText;

void
SelectMove(tb)
    GtkTextBuffer *tb;
{
    String val;
    gint index;
    GtkTextIter start;
    GtkTextIter end;	

    /* get cursor position into index */
    g_object_get(tb, "cursor-position", &index, NULL);

    /* get text from textbuffer */
    gtk_text_buffer_get_start_iter (tb, &start);
    gtk_text_buffer_get_end_iter (tb, &end);
    val = gtk_text_buffer_get_text (tb, &start, &end, FALSE); 
    FindMoveByCharIndex( index ); // [HGM] also does the actual moving to it, now
}

Option historyOptions[] = {
{ 0xD, 200, 400, NULL, (void*) &historyText, "", NULL, TextBox, "" },
{   0,  2,    0, NULL, (void*) NULL, "", NULL, EndMark , "" }
};

// ------------ standard entry points into MoveHistory code -----------

Boolean MoveHistoryIsUp()
{
    return shellUp[7];
}

Boolean MoveHistoryDialogExists()
{   
    return shellsGTK[7] != NULL;
}

gboolean HistoryPopUpCB(w, eventbutton, gptr)
     GtkWidget *w;
     GdkEventButton  *eventbutton;
     gpointer  gptr;
{
    GtkTextBuffer *tb;
 
    /* if not a right-click then propagate to default handler  */
    if (eventbutton->type != GDK_BUTTON_PRESS || eventbutton->button !=3) return False;

    /* get textbuffer */
    tb = GTK_TEXT_BUFFER(gptr);

    /* user has right clicked in the textbox */
    /* call CommentClick in xboard.c */
    SelectMove(tb);

    return True; /* don't propagate right click to default handler */   
}

void HistoryPopUp()
{
    GtkWidget *textview, *dialog, *w;
    GList *gl, *g;   

    if(!GenericPopUp(historyOptions, _("Move list"), 7)) return;    

    /* Find the dialogs GtkTextView widget */
    textview = GetTextView(shellsGTK[7]);

    if (!textview) return;

    g_signal_connect(GTK_TEXT_VIEW(textview), "button-press-event",
                     G_CALLBACK(HistoryPopUpCB),
                     (gpointer)historyOptions[0].handle);

    MarkMenu("menuView.Show Move History", 7);
    SetCheckMenuItemActive(NULL, 7, True); // set GTK menu item to checked     
}

void HistoryShowProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if (!shellUp[7]) {
    ASSIGN(historyText, "");
    HistoryPopUp();
    RefreshMemoContent();
    MemoContentUpdated();
  } else PopDown(7);
  ToNrEvent(currentMove);
}

void
HistoryShowProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
  if (!shellUp[7]) {
    ASSIGN(historyText, "");
    HistoryPopUp();
    RefreshMemoContent();
    MemoContentUpdated();
  } else PopDown(7);
  ToNrEvent(currentMove);
}

// duplicate of code in winboard.c, so an move to back-end!
void
HistorySet( char movelist[][2*MOVE_LEN], int first, int last, int current )
{
    MoveHistorySet( movelist, first, last, current, pvInfoList );

    EvalGraphSet( first, last, current, pvInfoList );

    MakeEngineOutputTitle();
}

