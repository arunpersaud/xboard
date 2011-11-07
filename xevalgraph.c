/*
 * Evaluation graph
 *
 * Author: Alessandro Scotti (Dec 2005)
 * Translated to X by H.G.Muller (Nov 2009)
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "evalgraph.h"
#include "gettext.h"
#include "gtk_helper.h"

char *crWhite = "#FFFFB0";
char *crBlack = "#AD5D3D";

GtkWidget *GUI_EvalGraph=NULL;
GtkWidget *GUI_EvalGraphDrawingArea=NULL;

GtkWidget *GetBoardWidget P((void));

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

static char *title = "Evaluation graph";

static int evalGraphDialogUp;

void DrawSegment( int x, int y, int *lastX, int *lastY, int penType )
{
  /* combines moveto (using PEN_NONE) and lineto function.
   * uses different line and color types (pens).
   */

  static int curX, curY;
  cairo_t *cr;
  static const double dotted[] = {4.0, 1.0};
  static int len  = sizeof(dotted) / sizeof(dotted[0]);

  /* this is just a move to call, nothing gets drawn, but the last position is updated */
  if(penType == PEN_NONE)
    {
      if(lastX != NULL) { *lastX = curX; *lastY = curY; }
      curX = x; curY = y;
      return;
    }

  cr = gdk_cairo_create (GUI_EvalGraphDrawingArea->window);
  if(!cr) {printf("Error: can not create CR in DrawSegment\n"); return;}

  /* create the path */
  cairo_move_to (cr, curX, curY);
  cairo_line_to (cr, x,y);
  cairo_close_path (cr);

  switch(penType)
    {
    case PEN_BLACK:
      cairo_set_line_width (cr, 1.0);
      cairo_set_source_rgba (cr, 0, 0, 0, 1.0);
      break;
    case PEN_DOTTED:
      cairo_set_line_width (cr, 1.0);
      cairo_set_source_rgba (cr, 0.62745098, 0.62745098, 0.62745098, 1.0);
      cairo_set_dash (cr, dotted, len, 0.0);
      break;
    case PEN_BLUEDOTTED:
      cairo_set_line_width (cr, 1.0);
      cairo_set_source_rgba (cr, 0, 0, 1, 1.0);
      cairo_set_dash (cr, dotted, len, 0.0);
      break;
    case PEN_BOLD:
      cairo_set_line_width (cr, 3.0);
      cairo_set_source_rgba (cr, 1.0, 1.0, 0.69, 1.0);
      break;
    case PEN_BOLD+1:
      cairo_set_line_width (cr, 3.0);
      cairo_set_source_rgba (cr, 0.6784313725490196 ,0.36470588235294116 ,0.23921568627450981, 1.0);
      break;
    }

  cairo_stroke (cr);

  if(lastX != NULL) { *lastX = curX; *lastY = curY; }
  curX = x; curY = y;

  /* free memory */
  cairo_destroy (cr);

  return;
}

// front-end wrapper for drawing functions to do rectangles
void DrawRectangle( int left, int top, int right, int bottom, int side, int style )
{
  cairo_t *cr;

  cr = gdk_cairo_create (GDK_WINDOW(GUI_EvalGraphDrawingArea->window));
  if(!cr) {printf("ERROR: can not create CR in DrawRectangle\n"); return;}

  cairo_rectangle (cr, left, top, right-left, bottom-top);

  switch(side)
    {
    case 0:
      cairo_set_line_width (cr, 3.0);
      cairo_set_source_rgba (cr, 1.0, 1.00, 0.69, 1.0);
      break;
    case 1:
      cairo_set_line_width (cr, 3.0);
      cairo_set_source_rgba (cr,0.6784313725490196 ,0.36470588235294116 ,0.23921568627450981 , 1.0);
      break;
    case 2:
      cairo_set_line_width (cr, 3.0);
      cairo_set_source_rgba (cr,0.8784313725490196,0.8784313725490196,0.9411764705882353, 1.0);
      break;
    }

  cairo_fill (cr);

  if(style != FILLED)
    {
      cairo_rectangle (cr, left, top, right-left-1, bottom-top-1);
      cairo_set_line_width (cr, 1.0);
      cairo_set_source_rgba (cr, 0, 0, 0, 1.0);
      cairo_stroke (cr);
    }

  /* free memory */
  cairo_destroy (cr);

  return;
}

// front-end wrapper for putting text in graph
void DrawEvalText(char *buf, int cbBuf, int y)
{
  cairo_text_extents_t extents;
  cairo_t *cr;

  cr = gdk_cairo_create (GDK_WINDOW(GUI_EvalGraphDrawingArea->window));
  if(!cr) {printf("ERROR: can not create CR in DrawEvalText\n"); return;}

  /* GTK-TODO this has to go into the font-selection */
  cairo_select_font_face (cr, "Sans",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 12.0);


  cairo_text_extents (cr, buf, &extents);

  cairo_move_to (cr, MarginX - 2 - 7*cbBuf, y+5);
  cairo_text_path (cr, buf);
  cairo_set_source_rgb (cr, 0.0, 0.0, 0);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0, 1.0, 0);
  cairo_set_line_width (cr, 0.1);
  cairo_stroke (cr);

  /* free memory */
  cairo_destroy (cr);

  return;
}

// front-end. Create pens, device context and buffer bitmap for global use, copy result to display
// The back-end part n the middle has been taken out and moed to PainEvalGraph()
static void DisplayEvalGraph()
{
  PaintEvalGraph();
  gtk_window_set_title(GTK_WINDOW(GUI_EvalGraph),_(title));
  return;
}

gboolean EvalGraphEventProc(widget, event)
     GtkWidget *widget;
     GdkEvent *event;
{
  /* handle all events from the EvalGraph window */

  int width;
  int height;
  int index;
  gdouble x,y;

  if (!EvalGraphIsUp()) return FALSE;

  switch(event->type)
    {
    case GDK_CONFIGURE:
      /* check for new size */
      width  = ((GdkEventConfigure*)event)->width;
      height = ((GdkEventConfigure*)event)->height;

      /* remember new size, values are used in backend */
      if( width != nWidthPB || height != nHeightPB )
	{
	  nWidthPB  = width;
	  nHeightPB = height;
	};

    case GDK_EXPOSE:
    case GDK_DAMAGE:
      DisplayEvalGraph();
      break;
    case GDK_BUTTON_PRESS:
      x = ((GdkEventButton*)event)->x;
      y = ((GdkEventButton*)event)->y;

      index = GetMoveIndexFromPoint( x,y );

      if( index >= 0 && index < currLast ) {
	ToNrEvent( index + 1 );
      }
      break;
    case GDK_DELETE:
      SetCheckMenuItemActive(NULL, 101, False); // set GTK menu item to unchecked      
      break;
    default:
      break;
    };

  return FALSE;
}

void EvalGraphCreate()
{
  GtkBuilder *builder=NULL;
  char *filename;
  GtkWidget *boardwidget;

  Arg args[16];
  gint bw_width, bw_height;
  int j;

  builder = gtk_builder_new ();
  GError *gtkerror=NULL;

  filename = get_glade_filename ("evalgraph.glade");

  if(! gtk_builder_add_from_file (builder, filename, &gtkerror) )
    {
      if(gtkerror)
	printf ("Error: %d %s\n",gtkerror->code,gtkerror->message);
    }

  /* need a reference to the window */
  GUI_EvalGraph = GTK_WIDGET (gtk_builder_get_object (builder, "EvalGraph"));
  if(!GUI_EvalGraph) printf("Error: gtk_builder didn't work (EvalGraph)!\n");

  /* and one to the drawing area */
  GUI_EvalGraphDrawingArea = GTK_WIDGET (gtk_builder_get_object (builder, "EvalGraphDrawingArea"));
  if(!GUI_EvalGraphDrawingArea) printf("Error: gtk_builder didn't work (EvalGraphDrawingArea)!\n");

  gtk_builder_connect_signals (builder, NULL);
  g_object_unref (G_OBJECT (builder));

  // get board width
  boardwidget = GetBoardWidget();   // get boardwidget width, height   
  gdk_drawable_get_size(boardwidget->window, &bw_width, &bw_height);

  /* GTK-TODO the position should be set relativ to the main window.
   * This will be easier once the main window is a GTK widget, since we
   * can make this window a child and open in relativ to the parent.
   * At the moment it gets open relative to the top left corner */

  if(wpEvalGraph.width > 0)
    restore_window_placement(GTK_WINDOW(GUI_EvalGraph), &wpEvalGraph);
  else /* set some defaults */
    {
      gint w = bw_width-16;
      gint h = bw_height/4;

      gtk_window_set_default_size(GTK_WINDOW(GUI_EvalGraph), w, h);
      gtk_window_resize ( GTK_WINDOW(GUI_EvalGraph), w,h);
    }

  return;
}

void
EvalGraphPopUp()
{
  static int  needInit = TRUE;

  SetCheckMenuItemActive(NULL, 101, True); // set GTK menu item to checked

  if (GUI_EvalGraph == NULL)
    EvalGraphCreate();

  DisplayEvalGraph();
  gtk_widget_show (GUI_EvalGraph);
  evalGraphDialogUp = True;  
  
  return;
}

void EvalGraphPopDown()
{
  SetCheckMenuItemActive(NULL, 101, False); // set GTK menu item to unchecked
  gtk_widget_hide (GUI_EvalGraph);
  evalGraphDialogUp = False;

  save_window_placement(GTK_WINDOW(GUI_EvalGraph), &wpEvalGraph);  

  return;
}

Boolean EvalGraphIsUp()
{
  return evalGraphDialogUp;
}

int EvalGraphDialogExists()
{
  return GUI_EvalGraph != NULL;
}

void EvalGraphProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if (evalGraphDialogUp)
    EvalGraphPopDown();
  else
    EvalGraphPopUp();
}

void
EvalGraphProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
  if (evalGraphDialogUp)
    EvalGraphPopDown();
  else
    EvalGraphPopUp();
}

// This function is the interface to the back-end. It is currently called through the front-end,
// though, where it shares the HistorySet() wrapper with MoveHistorySet(). Once all front-ends
// support the eval graph, it would be more logical to call it directly from the back-end.
void EvalGraphSet( int first, int last, int current, ChessProgramStats_Move * pvInfo )
{
    /* [AS] Danger! For now we rely on the pvInfo parameter being a static variable! */

    currFirst = first;
    currLast  = last;
    currCurrent = current;
    currPvInfo  = pvInfo;

    if( GUI_EvalGraph )
      DisplayEvalGraph();
}
