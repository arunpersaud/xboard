/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include "xboard.h"
#include "engineoutput.h"
#include "gettext.h"
#include "gtk_helper.h"


#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


// [HGM] pixmaps of some ICONS used in the engine-outut window
//#include "pixmaps/WHITE_14.xpm"
//#include "pixmaps/BLACK_14.xpm"
//#include "pixmaps/CLEAR_14.xpm"
//#include "pixmaps/UNKNOWN_14.xpm"
//#include "pixmaps/THINKING_14.xpm"
//#include "pixmaps/PONDER_14.xpm"
//#include "pixmaps/ANALYZING_14.xpm"

#ifdef SNAP
#include "wsnap.h"
#endif

#define _LL_ 100

Pixmap icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle
Widget outputField[2][7]; // [HGM] front-end array to translate output field to window handle
GtkWidget *outputFieldGTK[2][7]; // [HGM] front-end array to translate output field to window handle

static GtkBuilder *builder=NULL;
static GError *gtkerror=NULL; 
GtkWidget *engineOutputShellGTK=NULL;

void EngineOutputPopDown();
void engineOutputPopUp();
int  EngineOutputIsUp();
void SetEngineColorIcon( int which );
gboolean HandlePVGTK P((GtkWidget *widget, GdkEventMotion *eventmotion, gpointer data));
GtkWidget *GetBoardWidget P((void));

//extern WindowPlacement wpEngineOutput;

Position engineOutputX = -1, engineOutputY = -1;
Dimension engineOutputW, engineOutputH;
Widget engineOutputShell;
static int engineOutputDialogUp;

/* Module variables */
int  windowMode = 1;
static int currentPV, highTextStart[2], highTextEnd[2];

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

//static void UpdateControls( EngineOutputData * ed );

void ReadIcon(char *pixData[], int iconNr)
{
    int r;

//	if ((r=XpmCreatePixmapFromData(xDisplay, XtWindow(outputField[0][nColorIcon]),
//				       pixData,
//				       &(icons[iconNr]),
//				       NULL, NULL /*&attr*/)) != 0) {
//	  fprintf(stderr, _("Error %d loading icon image\n"), r);
//	  exit(1);
//	}
}

static void InitializeEngineOutput()
{
//        ReadIcon(WHITE_14,   nColorWhite);
//        ReadIcon(BLACK_14,   nColorBlack);
//        ReadIcon(UNKNOWN_14, nColorUnknown);
//
//        ReadIcon(CLEAR_14,   nClear);
//        ReadIcon(PONDER_14,  nPondering);
//        ReadIcon(THINK_14,   nThinking);
//        ReadIcon(ANALYZE_14, nAnalyzing);
}

void DoSetWindowText(int which, int field, char *s_label)
{
    // which = 0 for 1st engine, 1 for second
    // field = 3 for engine name, 5 for NPS     
    if (field != 3 && field != 5) return;
    if (!GTK_IS_LABEL(outputFieldGTK[which][field])) return;    
    gtk_label_set_text(GTK_LABEL(outputFieldGTK[which][field]), s_label);
}

void SetEngineOutputTitle(char *title)
{
    gtk_window_set_title(GTK_WINDOW(engineOutputShellGTK), title);
}

void InsertIntoMemo( int which, char * text, int where )
{
    gchar widgetname[50];
    GtkTextIter start;    
    
    strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
    GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);    
 
    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editGTK));            
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tb), &start);
    gtk_text_buffer_insert(tb, &start, text, -1);
}

void SetIcon( int which, int field, int nIcon )
{
    Arg arg;

    if( nIcon != 0 ) {
//	XtSetArg(arg, XtNleftBitmap, (XtArgVal) icons[nIcon]);
//	XtSetValues(outputField[which][field], &arg, 1);
    }
}

void DoClearMemo(int which)
{
    gchar widgetname[50];   
    
    strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
    GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);     
 
    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editGTK));        
    gtk_text_buffer_set_text(tb, "", -1);
}


// Key/Mouse Button Actions

// Ctrl<Key>c                     - copy selected text to clipboard
// Right Button down+mouse motion - Walk thru the PV on the board
// Shift+right button click       - Select the PV and show it on the board
// Right button down              - Select the PV and show it on the board
// Right button release           - unselect the PV, revert board to normal

static
gboolean ButtonReleaseCB(w, eventbutton, gptr)
     GtkWidget *w;
     GdkEventButton  *eventbutton;
     gpointer  gptr;
{	// [HGM] pv: on right-button release, stop displaying PV
    GtkTextBuffer *tb;    
    GtkTextIter iter;

    /* if not right-button release then return  */
    if (eventbutton->button !=3) return False;

    tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));

    // unselect text
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tb), &iter);
    gtk_text_buffer_select_range(GTK_TEXT_BUFFER(tb), &iter, &iter);

    highTextStart[currentPV] = highTextEnd[currentPV] = 0;
    UnLoadPV();
    return True;
}

static
gboolean ButtonPressCB(w, eventbutton, gptr)
     GtkWidget *w;
     GdkEventButton  *eventbutton;
     gpointer  gptr;
{
    GtkTextBuffer *tb;
    GtkTextIter start;
    GtkTextIter end;
    GtkTextIter bound;
    GtkTextIter insert;
    GtkTextIter iter;
    GtkTextIter iter2;
    gint x, y, index;
    String val;

    /* if not a right-click then return  */
    if (eventbutton->button !=3) return False;

    x = (int)eventbutton->x; y = (int)eventbutton->y;
    currentPV = (w == outputFieldGTK[1][nMemo]);

    tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));

    /* get cursor position into index */
    gint bufx,bufy;
    gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(w), GTK_TEXT_WINDOW_TEXT, x, y, &bufx, &bufy);   

    gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(w), &iter2, bufx, bufy);
    index = gtk_text_iter_get_offset(&iter2);   

    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tb), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tb), &end);
    val = gtk_text_buffer_get_text(tb, &start, &end, False);

    if (eventbutton->state & GDK_SHIFT_MASK) {
        shiftKey = 1; 
    } else {
        shiftKey = 0; 
    }

    gint gstart, gend;
    if(LoadMultiPV(x, y, val, index, &gstart, &gend)) {
        gtk_text_iter_set_offset(&start, gstart);
        gtk_text_iter_set_offset(&end, gend);
        gtk_text_buffer_select_range(GTK_TEXT_BUFFER(tb), &start, &end);
        highTextStart[currentPV] = gstart; highTextEnd[currentPV] = gend;
    }

    return True; /* don't propagate right click to default handler */   
}

gboolean HandlePVGTK(w, eventmotion, gptr)
     GtkWidget *w;
     GdkEventMotion  *eventmotion;
     gpointer  gptr;
{   // [HGM] pv: walk PV
    int squareSize = GetSquareSize();
    int lineGap = GetLineGapGTK();
    if ( ! (eventmotion->state & GDK_BUTTON3_MASK) ) return True;
    MovePV(eventmotion->x, eventmotion->y, lineGap + BOARD_HEIGHT * (squareSize + lineGap));
    return True;
}

// The following routines are mutated clones of the commentPopUp routines
void PositionControlSetGTK(which)
    int which;
{
    gchar widgetname[50];
    strcpy(widgetname, which == 0 ? "Engine1Colorlabel" : "Engine2Colorlabel");    
    GtkWidget *ColorWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ColorWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nColorIcon] = ColorWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Namelabel" : "Engine2Namelabel"); 
    GtkWidget *NameWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!NameWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nLabel] = NameWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Modelabel" : "Engine2Modelabel"); 
    GtkWidget *ModeWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ModeWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nStateIcon] = ModeWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Movelabel" : "Engine2Movelabel");
    GtkWidget *MoveWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ModeWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nStateData] = MoveWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Nodeslabel" : "Engine2Nodeslabel");
    GtkWidget *NodesWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!NodesWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nLabelNPS] = NodesWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
    GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nMemo] = editGTK;

    g_signal_connect(GTK_TEXT_VIEW(editGTK), "button-press-event",
                     G_CALLBACK(ButtonPressCB),
                     NULL);

    g_signal_connect(GTK_TEXT_VIEW(editGTK), "button-release-event",
                     G_CALLBACK(ButtonReleaseCB),
                     NULL);

    g_signal_connect(GTK_TEXT_VIEW(editGTK), "motion-notify-event",
                     G_CALLBACK(HandlePVGTK),
                     NULL);

    PangoTabArray *tabs = pango_tab_array_new_with_positions(4, True,
                            PANGO_TAB_LEFT, 60,
                            PANGO_TAB_LEFT, 120,
                            PANGO_TAB_LEFT, 190,
                            PANGO_TAB_LEFT, 260); 
    gtk_text_view_set_tabs(GTK_TEXT_VIEW(editGTK), tabs);

}

GtkWidget *EngineOutputCreateGTK(name, text)
     char *name, *text;
{
    GtkWidget *shell=NULL;
    gint bw_width, bw_height;
    GtkWidget *boardwidget;

    builder = gtk_builder_new();
    gchar *filename = get_glade_filename ("engineoutput.glade");
    if(!gtk_builder_add_from_file (builder, filename, &gtkerror)) {      
      if(gtkerror)
        printf ("Error: %d %s\n",gtkerror->code,gtkerror->message);
    }
    shell = GTK_WIDGET(gtk_builder_get_object(builder, "EngineOutput"));
    if(!shell) printf("Error: Failed to get engineoutput object with gtk_builder\n");    
    PositionControlSetGTK(0);
    PositionControlSetGTK(1);
    
    /* set old window size and position if available */
    if(wpEngineOutput.width > 0) {
        restore_window_placement(GTK_WINDOW(shell), &wpEngineOutput);
    } else { // set width, height of engine output window based on board size
        boardwidget = GetBoardWidget();   // get boardwidget width, height   
        gdk_drawable_get_size(boardwidget->window, &bw_width, &bw_height);
        engineOutputW = bw_width-16;
        engineOutputH = bw_height/2;
        gtk_window_resize(GTK_WINDOW(shell), engineOutputW, engineOutputH);    
    }       

    return shell;
}

void ResizeWindowControls(mode)
	int mode;
{
    GtkWidget *vpaned=NULL;

    vpaned = GTK_WIDGET(gtk_builder_get_object(builder, "EngineOutputVPaned"));
    if(!vpaned) printf("Error: Failed to get vpaned object with gtk_builder\n");

    // 1 engine so let it have the whole window
    if(mode==0) {
        gtk_paned_set_position(GTK_PANED(vpaned), vpaned->allocation.height);
    } else { // two engines so split the engine output window 50/50
        gtk_paned_set_position(GTK_PANED(vpaned), vpaned->allocation.height / 2);
    }
}

static
gboolean DeleteCB(w, event, gdata)
     GtkWidget *w;
     GdkEvent  *event;
     gpointer  gdata;
{
    EngineOutputPopDown();
    return True;
}

void
EngineOutputPopUp()
{
    Arg args[16];
    int j;
    Widget edit;
    static int  needInit = TRUE;
    static char *title = N_("Engine output"), *text = N_("This feature is experimental");    

    if (engineOutputShellGTK == NULL) {
        engineOutputShellGTK = EngineOutputCreateGTK(_(title), _(text));

        g_signal_connect(engineOutputShellGTK, "delete-event",
                          G_CALLBACK(DeleteCB),
                          NULL);

        gtk_widget_show_all(engineOutputShellGTK);
        ResizeWindowControls(1); // ensure pane separator is halfway down window
    } 
    
    SetCheckMenuItemActive(NULL, 100, True); // set GTK menu item to checked

    engineOutputDialogUp = True;
    ShowThinkingEvent(); // [HGM] thinking: might need to prompt engine for thinking output
}

void EngineOutputPopDown()
{
    Arg args[16];
    int j;

    if (!engineOutputDialogUp) return;
    DoClearMemo(1);

    /* lets save the position and size*/
    save_window_placement(GTK_WINDOW(engineOutputShellGTK), &wpEngineOutput);    

    gtk_widget_destroy(engineOutputShellGTK);
    engineOutputShellGTK = NULL;

    SetCheckMenuItemActive(NULL, 100, False); // set GTK menu item to unchecked
    engineOutputDialogUp = False;
    ShowThinkingEvent(); // [HGM] thinking: might need to shut off thinking output
}

int EngineOutputIsUp()
{
    return engineOutputDialogUp;
}

int EngineOutputDialogExists()
{
    return engineOutputShellGTK != NULL;
}

void EngineOutputProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if (engineOutputDialogUp) {
    EngineOutputPopDown();
  } else {
    EngineOutputPopUp();
  }
}
