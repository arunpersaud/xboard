/*
 * xhistory.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000, 2009, 2010 Free Software Foundation, Inc.
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

#include "config.h"

#include <gtk/gtk.h>
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

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "xhistory.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

extern GtkWidget               *GUI_History;
extern GtkListStore            *LIST_MoveHistory;
extern GtkTreeView             *TREE_History;

String dots=" ... ";
Position gameHistoryX, gameHistoryY;
Dimension gameHistoryW, gameHistoryH;

void
HistoryPopDown(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  /* hides the history window */

  gtk_widget_hide (GUI_History);
  return;
}

void 
HistoryMoveProc(window, event, data)
     GtkWindow *window;
     GdkEvent *event;
     gpointer data;
{
  int to; /* the move we want to go to */
  
  /* check if the mouse was clicked */
  if(event->type == GDK_BUTTON_PRESS)
    {	
      GtkTreeViewColumn *column;
      GtkTreePath *path;
      GList *cols;
      gint *indices;
      gint row,col;
      
      /* can we convert this into an element of the history list? */
      if(gtk_tree_view_get_path_at_pos(TREE_History,
				       (gint)event->button.x, (gint)event->button.y,
				       &path,&column,NULL,NULL))
	{
	  /* find out which row and column the user clicked on */
	  indices = gtk_tree_path_get_indices(path);
	  row     = indices[0];
	  cols    = gtk_tree_view_get_columns(GTK_TREE_VIEW(column->tree_view));
	  col     = g_list_index(cols,(gpointer)column);
	  g_list_free(cols);
	  gtk_tree_path_free(path);
	  
	  printf("DEBUG: row %d col %d\n",row,col);


	  if(col)
	    {
	      /* user didn't click on the move number */

	      to = 2*row + col;
	      printf("DEBUG: going to %d\n",to);fflush(stdout);

	      /* set board to that move */
	      ToNrEvent(to);
	    }
	}
    }
  return;
}


void HistorySet(char movelist[][2*MOVE_LEN],int first,int last,int current)
{
  int i,b,m;
  char movewhite[2*MOVE_LEN],moveblack[2*MOVE_LEN],move[2*MOVE_LEN];
  GtkTreeIter iter;

  /* TODO need to add highlights for current move */
  /* TODO need to add navigation by keyboard or mouse (double click on move) */

  strcpy(movewhite,"");
  strcpy(moveblack,"");

  /* first clear everything, do we need this? */
  gtk_list_store_clear(LIST_MoveHistory);

  /* copy move list into history window */

  /* go through all moves */
  for(i=0;i<last;i++) 
    {
      /* test if there is a move */
      if(movelist[i][0]) 
	{
	  /* only copy everything before a  ' ' */
	  char* p = strchr(movelist[i], ' ');
	  if (p) 
	    {
	      strncpy(move, movelist[i], p-movelist[i]);
	      move[p-movelist[i]] = NULLCHAR;
	    } 
	  else 
	    {
	      strcpy(move,movelist[i]);
	    }
	} 
      else
	strcpy(move,dots);
      
      if((i%2)==0) 
	{
	  /* white move */
	  strcpy(movewhite,move);
	}
      else
	{
	  /* black move */
	  strcpy(moveblack,move);

	  /* save move */
	  gtk_list_store_append (LIST_MoveHistory, &iter);
	  gtk_list_store_set (LIST_MoveHistory, &iter,
			      0, (i/2 +1),
			      1, movewhite,
			      2, moveblack,
			      -1);

	  strcpy(movewhite,"");
	  strcpy(moveblack,"");
	};
    }

  /* check if there is a white move left */
  if(movewhite[0])
    {
      strcpy(moveblack,"");

      /* save move */
      gtk_list_store_append (LIST_MoveHistory, &iter);
      gtk_list_store_set (LIST_MoveHistory, &iter,
			  0, (i/2 +1),
			  1, movewhite,
			  2, moveblack,
			  -1);
    };


  //TODO
  //  EvalGraphSet( first, last, current, pvInfoList ); // piggy-backed
  
  return;
}

void HistoryCreate()
{
    String trstr=
             "<Key>Up: BackwardProc() \n \
             <Key>Left: BackwardProc() \n \
             <Key>Down: ForwardProc() \n \
             <Key>Right: ForwardProc() \n";

    return;
//    if(wpMoveHistory.width > 0) {
//      gameHistoryW = wpMoveHistory.width;
//      gameHistoryH = wpMoveHistory.height;
//      gameHistoryX = wpMoveHistory.x;
//      gameHistoryY = wpMoveHistory.y;
//    }
//
//  // [HGM] restore old position
//  if(gameHistoryW > 0) {
//  j = 0;
//    XtSetArg(args[j], XtNx, gameHistoryX);  j++;
//  XtSetArg(args[j], XtNy, gameHistoryY);  j++;
//    XtSetArg(args[j], XtNwidth, gameHistoryW);  j++;
//    XtSetArg(args[j], XtNheight, gameHistoryH);  j++;
//  XtSetValues(hist->sh, args, j);
//  }
}

void
HistoryPopUp()
{
  /* show history window */

  gtk_widget_show (GUI_History);
  return;
}


void
HistoryShowProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  HistoryCreate();
  HistoryPopUp();
  //TODO:  ToNrEvent(currentMove);

  return;
}

Boolean
MoveHistoryIsUp()
{
  /* return status of history window */
  
  return gtk_widget_get_visible (GUI_History);
}
