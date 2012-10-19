/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include <gtk/gtk.h>

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

// [HGM] pixmaps of some ICONS used in the engine-outut window
#include "pixmaps/WHITE_14.xpm"
#include "pixmaps/BLACK_14.xpm"
#include "pixmaps/CLEAR_14.xpm"
#include "pixmaps/UNKNOWN_14.xpm"
#include "pixmaps/THINKING_14.xpm"
#include "pixmaps/PONDER_14.xpm"
#include "pixmaps/ANALYZING_14.xpm"

extern Option engoutOptions[]; // must go in header, but which?

/* Module variables */
static int currentPV, highTextStart[2], highTextEnd[2];
#ifdef TODO_GTK
static Pixmap icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle
static Widget memoWidget;
#endif
static void *memoWidget;

#ifdef TODO_GTK
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
#endif

void
InitEngineOutput (Option *opt, Option *memo2)
{	// front-end, because it must have access to the pixmaps
#ifdef TODO_GTK
	Widget w = opt->handle;
	memoWidget = memo2->handle;

        ReadIcon(WHITE_14,   nColorWhite, w);
        ReadIcon(BLACK_14,   nColorBlack, w);
        ReadIcon(UNKNOWN_14, nColorUnknown, w);

        ReadIcon(CLEAR_14,   nClear, w);
        ReadIcon(PONDER_14,  nPondering, w);
        ReadIcon(THINK_14,   nThinking, w);
        ReadIcon(ANALYZE_14, nAnalyzing, w);
#endif
}

void
DrawWidgetIcon (Option *opt, int nIcon)
{   // as we are already in X front-end, so do X-stuff here
#ifdef TODO_GTK
    gchar widgetname[50];

    if( nIcon != 0 ) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(opt->handle), GDK_PIXBUF(iconsGTK[nIcon]));
    }
#endif
}

void
InsertIntoMemo (int which, char * text, int where)
{
    char *p;
    GtkTextIter start;
 
    /* the backend adds \r\n, which is needed for winboard,
     * for xboard we delete them again over here */
    if(p = strchr(text, '\r')) *p = ' ';

    GtkTextBuffer *tb = (GtkTextBuffer *) (engoutOptions[which ? 12 : 5].handle);
//    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tb), &start);
    gtk_text_buffer_get_iter_at_offset(tb, &start, where);
    gtk_text_buffer_insert(tb, &start, text, -1);
    if(where < highTextStart[which]) { // [HGM] multiPVdisplay: move highlighting
	int len = strlen(text);
	highTextStart[which] += len; highTextEnd[which] += len;
    }
}

//--------------------------------- PV walking ---------------------------------------

char memoTranslations[] =
":Ctrl<Key>c: CopyMemoProc() \n \
<Btn3Motion>: HandlePV() \n \
Shift<Btn3Down>: select-start() extend-end() SelectPV(1) \n \
Any<Btn3Down>: select-start() extend-end() SelectPV(0) \n \
<Btn3Up>: StopPV() \n";

//------------------------------- pane switching -----------------------------------

void
ResizeWindowControls (int mode)
{   // another hideous kludge: to have only a single pane, we resize the
    // second to 5 pixels (which makes it too small to display anything)
    if(mode) gtk_widget_show(engoutOptions[13].handle);
    else     gtk_widget_hide(engoutOptions[13].handle);
}

