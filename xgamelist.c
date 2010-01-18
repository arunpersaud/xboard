/*
 * xgamelist.c -- Game list window, part of X front end for XBoard
 *
 * Copyright 1995,2009 Free Software Foundation, Inc.
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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#if USE_XAW3D
#include <X11/Xaw3d/Dialog.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/List.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/SmeLine.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/MenuButton.h>
#include <X11/Xaw3d/Text.h>
#include <X11/Xaw3d/AsciiText.h>
#include <X11/Xaw3d/Viewport.h>
#else
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Viewport.h>
#endif

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "xgamelist.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

extern GtkWidget               *GUI_GameList;
extern GtkListStore            *LIST_GameList;


extern Widget formWidget, boardWidget, menuBarWidget, gameListShell;
extern int squareSize;
extern Pixmap xMarkPixmap;
extern char *layoutName;

char gameListTranslations[] =
  "<Btn1Up>(2): LoadSelectedProc() \n \
   <Key>Return: LoadSelectedProc() \n";

typedef struct {
    Widget shell;
    Position x, y;
    Dimension w, h;
    Boolean up;
    FILE *fp;
    char *filename;
    char **strings;
} GameListClosure;

static Arg layoutArgs[] = {
    { XtNborderWidth, 0 },
    { XtNdefaultDistance, 0 }
};

Widget
GameListCreate(name, callback, client_data)
     char *name;
     XtCallbackProc callback;
     XtPointer client_data;
{
  return;
}

void
GameListCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int j;
    Widget listwidg;
    GameListClosure *glc = (GameListClosure *) client_data;
    XawListReturnStruct *rs;
    int index;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("close")) == 0) {
	GameListPopDown();
	return;
    }
    listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
    rs = XawListShowCurrent(listwidg);
    if (strcmp(name, _("load")) == 0) {
	index = rs->list_index;
	if (index < 0) {
	    DisplayError(_("No game selected"), 0);
	    return;
	}
    } else if (strcmp(name, _("next")) == 0) {
	index = rs->list_index + 1;
	if (index >= ((ListGame *) gameList.tailPred)->number) {
	    DisplayError(_("Can't go forward any further"), 0);
	    return;
	}
	XawListHighlight(listwidg, index);
    } else if (strcmp(name, _("prev")) == 0) {
	index = rs->list_index - 1;
	if (index < 0) {
	    DisplayError(_("Can't back up any further"), 0);
	    return;
	}
	XawListHighlight(listwidg, index);
    }
    if (cmailMsgLoaded) {
	CmailLoadGame(glc->fp, index + 1, glc->filename, True);
    } else {
	LoadGame(glc->fp, index + 1, glc->filename, True);
    }
}

static GameListClosure *glc = NULL;

void
GameListPopUp(fp, filename)
     FILE *fp;
     char *filename;
{
  GtkTreeIter iter;
  int  i=0,nstrings;
  ListGame *lg;
  
  /* first clear everything, do we need this? */
  gtk_list_store_clear(LIST_GameList);

  /* fill list with information */
  lg = (ListGame *) gameList.head;
  nstrings = ((ListGame *) gameList.tailPred)->number;
  while (nstrings--) 
    {
      gtk_list_store_append (LIST_GameList, &iter);
      gtk_list_store_set (LIST_GameList, &iter,
			  0, StrSave(filename),
			  1, GameListLine(lg->number, &lg->gameInfo),
			  2, fp,
			  -1);
      lg = (ListGame *) lg->node.succ;
    }


  /* show widget */
  gtk_widget_show (GUI_GameList);

//    XtPopup(glc->shell, XtGrabNone);
//    glc->up = True;
//    j = 0;
//    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
//    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Show Game List"),
//		args, j);

  return;
}

void
GameListDestroy()
{
  GameListPopDown();

  gtk_list_store_clear(LIST_GameList);
  return;
}

void
ShowGameListProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];
    int j;

    if (glc == NULL) {
	DisplayError(_("There is no game list"), 0);
	return;
    }
    if (glc->up) {
	GameListPopDown();
	return;
    }
    XtPopup(glc->shell, XtGrabNone);
    glc->up = True;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Show Game List"),
		args, j);
}

void
LoadSelectedProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Widget listwidg;
    XawListReturnStruct *rs;
    int index;

    if (glc == NULL) return;
    listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
    rs = XawListShowCurrent(listwidg);
    index = rs->list_index;
    if (index < 0) return;
    if (cmailMsgLoaded) {
	CmailLoadGame(glc->fp, index + 1, glc->filename, True);
    } else {
	LoadGame(glc->fp, index + 1, glc->filename, True);
    }
}

void
GameListPopDown()
{
  /* hides the history window */

  gtk_widget_hide (GUI_GameList);
  return;
}

void
GameListHighlight(index)
     int index;
{
    Widget listwidg;
    if (glc == NULL || !glc->up) return;
    listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
    XawListHighlight(listwidg, index - 1);
}

Boolean
GameListIsUp()
{
  /* return status of history window */
  
  return gtk_widget_get_visible (GUI_GameList);
}
