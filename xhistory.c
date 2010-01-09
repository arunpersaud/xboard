/*
 * xhistory.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000,2009 Free Software Foundation, Inc.
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

String dots=" ... ";
Position gameHistoryX, gameHistoryY;
Dimension gameHistoryW;

void
HistoryPopDown(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  gtk_widget_hide (GUI_History);
  return;
}

void HistoryMoveProc(Widget w, XtPointer closure, XtPointer call_data)
{
    int to;
    /*
    XawListReturnStruct *R = (XawListReturnStruct *) call_data;
    if (w == hist->mvn || w == hist->mvw) {
      to=2*R->list_index-1;
      ToNrEvent(to);
    }
    else if (w == hist->mvb) {
      to=2*R->list_index;
      ToNrEvent(to);
    }
    */
}


void HistorySet(char movelist[][2*MOVE_LEN],int first,int last,int current)
{
  int i,b,m;
  char movewhite[2*MOVE_LEN],moveblack[2*MOVE_LEN],move[2*MOVE_LEN];
  GtkTreeIter iter;

  /* TODO need to add highlights for current move */
  /* TODO need to add navigation by keyboard or mouse (double click on move) */

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
			      0, i,
			      1, movewhite,
			      2, moveblack,
			      -1);

	  strcpy(movewhite,"");
	  strcpy(moveblack,"");
	};
    }
  /* check if ther is a white move left */
  if(movewhite[0])
    {
      i++;
      strcpy(moveblack,"");
      /* save move */
      gtk_list_store_append (LIST_MoveHistory, &iter);
      gtk_list_store_set (LIST_MoveHistory, &iter,
			  0, i,
			  1, movewhite,
			  2, moveblack,
			  -1);
    };
  
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
    /*-------- create the widgets ---------------*/
//    j = 0;
//    XtSetArg(args[j], XtNresizable, True);  j++;
//    XtSetArg(args[j], XtNallowShellResize, True);  j++;
//#if TOPLEVEL
//    hist->sh =
//      XtCreatePopupShell(_("Move list"), topLevelShellWidgetClass,
//			 shellWidget, args, j);
//#else
//    hist->sh =
//      XtCreatePopupShell(_("Move list"), transientShellWidgetClass,
//			 shellWidget, args, j);
//#endif
//    j = 0;
//    XtSetArg(args[j], XtNborderWidth, 0); j++;
//    XtSetArg(args[j], XtNdefaultDistance, 0);  j++;
//      layout =
//      XtCreateManagedWidget(layoutName, formWidgetClass, hist->sh,
//			    args, j);
//
//    j = 0;
//    XtSetArg(args[j], XtNborderWidth, 0); j++;
//    XtSetArg(args[j], XtNresizable, True);  j++;
//
//    form =
//      XtCreateManagedWidget("form", formWidgetClass, layout, args, j);
//     j=0;
//
//    j = 0;
//
//    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
//    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
//    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNright, XtChainRight);  j++;
//
//    XtSetArg(args[j], XtNborderWidth, 1); j++;
//    XtSetArg(args[j], XtNresizable, False);  j++;
//    XtSetArg(args[j], XtNallowVert, True); j++;
//    XtSetArg(args[j], XtNallowHoriz, True);  j++;
//    XtSetArg(args[j], XtNforceBars, False); j++;
//    XtSetArg(args[j], XtNheight, 280); j++;
//    hist->viewport =
//      XtCreateManagedWidget("viewport", viewportWidgetClass,
//			    form, args, j);
//    j=0;
//    XtSetArg(args[j], XtNborderWidth, 0); j++;
//    XtSetArg(args[j], XtNorientation,XtorientHorizontal);j++;
//    hist->vbox =
//      XtCreateManagedWidget("vbox", formWidgetClass, hist->viewport, args, j);
//
//    j=0;
//    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
//    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
//    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
//
//    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
//    XtSetArg(args[j], XtNforceColumns, True);  j++;
//    XtSetArg(args[j], XtNverticalList, True);  j++;
//    XtSetArg(args[j], XtNborderWidth, 0); j++;
//    XtSetArg(args[j], XtNresizable,True);j++;
//    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
//    hist->mvn = XtCreateManagedWidget("movesn", listWidgetClass,
//				      hist->vbox, args, j);
//    XtAddCallback(hist->mvn, XtNcallback, HistoryMoveProc, (XtPointer) hist);
//
//    j=0;
//    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
//    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
//    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNright, XtRubber);  j++;
//
//    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
//    XtSetArg(args[j], XtNforceColumns, True);  j++;
//    XtSetArg(args[j], XtNverticalList, True);  j++;
//    XtSetArg(args[j], XtNborderWidth, 0); j++;
//    XtSetArg(args[j], XtNresizable,True);j++;
//    XtSetArg(args[j], XtNfromHoriz, hist->mvn);  j++;
//    hist->mvw = XtCreateManagedWidget("movesw", listWidgetClass,
//				      hist->vbox, args, j);
//    XtAddCallback(hist->mvw, XtNcallback, HistoryMoveProc, (XtPointer) hist);
//
//    j=0;
//    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
//    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
//    XtSetArg(args[j], XtNleft, XtRubber);  j++;
//    XtSetArg(args[j], XtNright,  XtRubber);  j++;
//
//    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
//    XtSetArg(args[j], XtNforceColumns, True);  j++;
//    XtSetArg(args[j], XtNverticalList, True);  j++;
//    XtSetArg(args[j], XtNborderWidth, 0); j++;
//    XtSetArg(args[j], XtNresizable,True);j++;
//    XtSetArg(args[j], XtNfromHoriz, hist->mvw);  j++;
//    hist->mvb = XtCreateManagedWidget("movesb", listWidgetClass,
//				      hist->vbox, args, j);
//    XtAddCallback(hist->mvb, XtNcallback, HistoryMoveProc, (XtPointer) hist);
//
//    j=0;
//    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
//    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
//    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
//    XtSetArg(args[j], XtNfromVert, hist->viewport);  j++;
//    b_close= XtCreateManagedWidget(_("Close"), commandWidgetClass,
//				   form, args, j);
//    XtAddCallback(b_close, XtNcallback, HistoryPopDown, (XtPointer) 0);
//
//    XtAugmentTranslations(hist->sh,XtParseTranslationTable (trstr));
//
//    XtRealizeWidget(hist->sh);
//    CatchDeleteWindow(hist->sh, "HistoryPopDown");
//
//    for(i=1;i<hist->aNr;i++){
//      strcpy(hist->white[i],dots);
//      strcpy(hist->black[i],"");
//     }
//
//  // [HGM] restore old position
//  j = 0;
//  XtSetArg(args[j], XtNx, &gameHistoryX);  j++;
//  XtSetArg(args[j], XtNy, &gameHistoryY);  j++;
//  XtSetArg(args[j], XtNwidth, &gameHistoryW);  j++;
//  XtGetValues(shellWidget, args, j);
//  j = 0;
//  XtSetArg(args[j], XtNx, gameHistoryX + gameHistoryW);  j++;
//  XtSetArg(args[j], XtNy, gameHistoryY);  j++;
//  XtSetValues(hist->sh, args, j);
//    XtRealizeWidget(hist->sh);
//
//    return hist->sh;
}

void
HistoryPopUp()
{
  //  if(!hist) HistoryCreate();

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

