/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010 Free Software Foundation, Inc.
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
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

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


#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "engineoutput.h"
#include "gettext.h"

extern GtkWidget   *GUI_EngineOutput; /* set in xboard.x */


#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


GdkPixbuf *EngineIcons[8]; // [HGM] this front-end array translates back-end icon indicator to handle

extern GtkWidget *GUI_EngineOutputFields[2][GUI_N]; // [HGM] front-end array to translate output field to window handle

Boolean   engineOutputDialogUp=False;

void EngineOutputPopDown();
void engineOutputPopUp();
int  EngineOutputIsUp();
void SetEngineColorIcon( int which );

/* Module variables */
int  windowMode = 1;

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


void ResizeWindowControls(mode)
	int mode;
{
  /* mode = 0 : hide one output 
   *        1 : show both 
   */

  //TODO

  return;
}


void InitializeEngineOutput()
{ 
  EngineIcons[nColorWhite]   = load_pixbuf("svg/engine-white.svg",14);
  EngineIcons[nColorBlack]   = load_pixbuf("svg/engine-black.svg",14);
  EngineIcons[nColorUnknown] = load_pixbuf("svg/engine-unknown.svg",14);
  EngineIcons[nClear]        = load_pixbuf("svg/engine-clear.svg",14);
  EngineIcons[nPondering]    = load_pixbuf("svg/engine-ponder.svg",14);
  EngineIcons[nThinking]     = load_pixbuf("svg/engine-thinking.svg",14);
  EngineIcons[nAnalyzing]    = load_pixbuf("svg/engine-analyzing.svg",14);
// [HGM] pixmaps of some ICONS used in the engine-outut window
//#include "pixmaps/WHITE_14.xpm"
//#include "pixmaps/BLACK_14.xpm"
//#include "pixmaps/CLEAR_14.xpm"
//#include "pixmaps/UNKNOWN_14.xpm"
//#include "pixmaps/THINKING_14.xpm"
//#include "pixmaps/PONDER_14.xpm"
//#include "pixmaps/ANALYZING_14.xpm"
//
  
  return;
}

void 
DoSetWindowText(int which, int field, char *text)
{ 
  gtk_label_set_text(GTK_LABEL(GUI_EngineOutputFields[which][field]),text);
  return;
}

void InsertIntoMemo( int which, char * text, int where )
{
  GtkTextBuffer *buffer=NULL;
  GtkTextIter iter;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(GUI_EngineOutputFields[which][GUI_TEXT]));
  gtk_text_buffer_get_end_iter ( buffer, &iter );

  gtk_text_buffer_insert(buffer,&iter,text,strlen(text));

  return;
}

void 
SetIcon( int which, int field, int nIcon )
{

  if( nIcon != 0   )
    gtk_image_set_from_pixbuf (GTK_IMAGE(GUI_EngineOutputFields[which][field]),EngineIcons[nIcon]);
  
  return;
}

void DoClearMemo(int which)
{ 
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(GUI_EngineOutputFields[which][GUI_TEXT]));
  
  gtk_text_buffer_set_text(buffer,"",0);

  return;
}

void 
EngineOutputPopUp()
{
  InitializeEngineOutput();

  SetEngineColorIcon( 0 );
  SetEngineColorIcon( 1 );
  SetEngineState( 0, STATE_IDLE, "" );
  SetEngineState( 1, STATE_IDLE, "" );

  engineOutputDialogUp = True;

  gtk_widget_show_all (GUI_EngineOutput);

  // [HGM] thinking: might need to prompt engine for thinking output
  ShowThinkingEvent();
}

void EngineOutputPopDown()
{
  if (!engineOutputDialogUp) return;

  engineOutputDialogUp = False;

  gtk_widget_hide (GUI_EngineOutput);
  return;

  // [HGM] thinking: might need to shut off thinking 
  ShowThinkingEvent(); 
}

int EngineOutputIsUp()
{
    return engineOutputDialogUp;
}

int EngineOutputDialogExists()
{
  return 1;
}

void
EngineOutputProc(GtkObject *object, gpointer user_data)
{
  if (engineOutputDialogUp) 
    EngineOutputPopDown();
  else 
    EngineOutputPopUp();
 
  return;
}
