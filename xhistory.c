/*
 * New (WinBoard-style) Move history for XBoard
 *
 * Copyright 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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
#include <gtk/gtk.h>

#include "common.h"
#include "backend.h"
#include "xhistory.h"
#include "dialogs.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

// templates for calls into back-end (= history.c; should be moved to history.h header shared with it!)
void FindMoveByCharIndex P(( int char_index ));

// variables in nhistory.c
extern Option historyOptions[];

// ------------- low-level front-end actions called by MoveHistory back-end -----------------

void
HighlightMove (int from, int to, Boolean highlight)
{
    static int init = 0;
    static GtkTextIter start, end;

    if(!init) {
	init = 1;
	gtk_text_buffer_create_tag(historyOptions[0].handle, "highlight", "background", "yellow", NULL);
	gtk_text_buffer_create_tag(historyOptions[0].handle, "normal", "background", "white", NULL);
    }
    gtk_text_buffer_get_iter_at_offset(historyOptions[0].handle, &start, from);
    gtk_text_buffer_get_iter_at_offset(historyOptions[0].handle, &end, to);
    gtk_text_buffer_apply_tag_by_name(historyOptions[0].handle, highlight ? "highlight" : "normal", &start, &end);
}

void
ScrollToCurrent (int caretPos)
{
    static GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset((GtkTextBuffer *) historyOptions[0].handle, &iter, caretPos);
    gtk_text_view_scroll_to_iter((GtkTextView *) historyOptions[0].textValue, &iter, 0.0, 0, 0.5, 0.5);
}


// ------------------------------ callbacks --------------------------

char historyTranslations[] =
"<Btn3Down>: select-start() \n \
<Btn3Up>: extend-end() SelectMove() \n";

