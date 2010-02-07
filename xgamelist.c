/*
 * xgamelist.c -- Game list window, part of X front end for XBoard
 *
 * Copyright 1995, 2009, 2010 Free Software Foundation, Inc.
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

void SetFocus P((Widget w, XtPointer data, XEvent *event, Boolean *b));

extern Widget formWidget, shellWidget, boardWidget, menuBarWidget, gameListShell;

extern int squareSize;
extern Pixmap xMarkPixmap;
extern char *layoutName;

static Widget filterText;
static char filterString[MSG_SIZ];
static int listLength;

char gameListTranslations[] =
  "<Btn1Up>(2): LoadSelectedProc(0) \n \
   <Key>Home: LoadSelectedProc(-2) \n \
   <Key>End: LoadSelectedProc(2) \n \
   <Key>Up: LoadSelectedProc(-1) \n \
   <Key>Down: LoadSelectedProc(1) \n \
   <Key>Left: LoadSelectedProc(-1) \n \
   <Key>Right: LoadSelectedProc(1) \n \
   <Key>Return: LoadSelectedProc(0) \n";
char filterTranslations[] =
  "<Key>Return: SetFilterProc() \n";

typedef struct {
    Widget shell;
    Position x, y;
    Dimension w, h;
    Boolean up;
    FILE *fp;
    char *filename;
    char **strings;
} GameListClosure;
static GameListClosure *glc = NULL;

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

static int
GameListPrepare()
{   // [HGM] filter: put in separate routine, to make callable from call-back
    int nstrings;
    ListGame *lg;
    char **st, *line;

    nstrings = ((ListGame *) gameList.tailPred)->number;
    glc->strings = (char **) malloc((nstrings + 1) * sizeof(char *));
    st = glc->strings;
    lg = (ListGame *) gameList.head;
    listLength = 0;
    while (nstrings--) {
	line = GameListLine(lg->number, &lg->gameInfo);
	if(filterString[0] == NULLCHAR || SearchPattern( line, filterString ) ) {
	    *st++ = line; // [HGM] filter: make adding line conditional
	    listLength++;
	}
	lg = (ListGame *) lg->node.succ;
     }
    *st = NULL;
    return listLength;
}

static void
GameListReplace()
{   // [HGM] filter: put in separate routine, to make callable from call-back
    Arg args[16];
    int j;
    Widget listwidg;

	listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
	XawListChange(listwidg, glc->strings, 0, 0, True);
	XawListHighlight(listwidg, 0);
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
	if (index >= listLength) {
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
    } else if (strcmp(name, _("apply")) == 0) {
        String name;
        j = 0;
        XtSetArg(args[j], XtNstring, &name);  j++;
	XtGetValues(filterText, args, j);
        strcpy(filterString, name);
	XawListHighlight(listwidg, 0);
        if(GameListPrepare()) GameListReplace(); // crashes on empty list...
        return;
    }
    index = atoi(glc->strings[index])-1; // [HGM] filter: read true index from sequence nr of line
    if (cmailMsgLoaded) {
	CmailLoadGame(glc->fp, index + 1, glc->filename, True);
    } else {
	LoadGame(glc->fp, index + 1, glc->filename, True);
    }
}

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
    int index, direction = atoi(prms[0]);

    if (glc == NULL) return;
    listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
    rs = XawListShowCurrent(listwidg);
    index = rs->list_index;
    if (index < 0) return;
    if(direction != 0) {
	index += direction; 
	if(direction == -2) index = 0;
	if(direction == 2) index = listLength-1;
	if(index < 0 || index >= listLength) return;
	XawListHighlight(listwidg, index);
	return;
    }
    index = atoi(glc->strings[index])-1; // [HGM] filter: read true index from sequence nr of line
    if (cmailMsgLoaded) {
	CmailLoadGame(glc->fp, index + 1, glc->filename, True);
    } else {
	LoadGame(glc->fp, index + 1, glc->filename, True);
    }
}

void
SetFilterProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
	Arg args[16];
        String name;
	Widget list;
        int j = 0;
        XtSetArg(args[j], XtNstring, &name);  j++;
	XtGetValues(filterText, args, j);
        strcpy(filterString, name);
        if(GameListPrepare()) GameListReplace(); // crashes on empty list...
	list = XtNameToWidget(glc->shell, "*form.viewport.list");
	XawListHighlight(list, 0);
        j = 0;
	XtSetArg(args[j], XtNdisplayCaret, False); j++;
	XtSetValues(filterText, args, j);
	XtSetKeyboardFocus(glc->shell, list);
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
    int i=0; char **st;
    if (glc == NULL || !glc->up) return;
    listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
    st = glc->strings;
    while(*st && atoi(*st)<index) st++,i++;
    XawListHighlight(listwidg, i);
}

Boolean
GameListIsUp()
{
  /* return status of history window */
  
  return gtk_widget_get_visible (GUI_GameList);
}

//--------------------------------- Game-List options dialog ------------------------------------------

Widget gameListOptShell, listwidg;

char *strings[20];
int stringPtr;

void GLT_ClearList()
{
    strings[0] = NULL;
    stringPtr = 0;
}

void GLT_AddToList(char *name)
{
    strings[stringPtr++] = name;
    strings[stringPtr] = NULL;
}

Boolean GLT_GetFromList(int index, char *name)
{
    strcpy(name, strings[index]);
    return TRUE;
}

void GLT_DeSelectList()
{
    XawListChange(listwidg, strings, 0, 0, True);
    XawListHighlight(listwidg, 0);
}

void
GameListOptionsPopDown()
{
    Arg args[16];
    int j;

    if (gameListOptShell == NULL) return;
    XtPopdown(gameListOptShell);
    XtDestroyWidget(gameListOptShell);
    gameListOptShell = 0;
    //    XtSetKeyboardFocus(shellWidget, formWidget);
}

void
GameListOptionsCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int j;
    Widget listwidg;
    XawListReturnStruct *rs;
    int index;
    char *p;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("OK")) == 0) {
	GLT_ParseList();
	appData.gameListTags = strdup(lpUserGLT);
	GameListOptionsPopDown();
	return;
    } else
    if (strcmp(name, _("cancel")) == 0) {
	GameListOptionsPopDown();
	return;
    }
    listwidg = XtNameToWidget(gameListOptShell, "*form.list");
    rs = XawListShowCurrent(listwidg);
    index = rs->list_index;
    if (index < 0) {
	DisplayError(_("No tag selected"), 0);
	return;
    }
    p = strings[index];
    if (strcmp(name, _("down")) == 0) {
        if(index >= strlen(GLT_ALL_TAGS)) return;
	strings[index] = strings[index+1];
	strings[++index] = p;
    } else
    if (strcmp(name, _("up")) == 0) {
        if(index == 0) return;
	strings[index] = strings[index-1];
	strings[--index] = p;
    } else
    if (strcmp(name, _("factory")) == 0) {
	strcpy(lpUserGLT, GLT_DEFAULT_TAGS);
	GLT_TagsToList(lpUserGLT);
	index = 0;
    }
    XawListHighlight(listwidg, index);
}

Widget
GameListOptionsCreate()
{
    Arg args[16];
    Widget shell, form, viewport, layout;
    Widget b_load, b_loadprev, b_loadnext, b_close, b_cancel;
    Dimension fw_width;
    XtPointer client_data = NULL;
    int j;

    j = 0;
    XtSetArg(args[j], XtNwidth, &fw_width);  j++;
    XtGetValues(formWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNallowShellResize, True);  j++;
    //    shell = gameListOptShell =
      //      XtCreatePopupShell("Game-list options", transientShellWidgetClass,
      //			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    j = 0;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout, args, j);

    j = 0;
    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
    XtSetArg(args[j], XtNforceColumns, True);  j++;
    XtSetArg(args[j], XtNverticalList, True);  j++;
    listwidg = viewport =
      XtCreateManagedWidget("list", listWidgetClass, form, args, j);
    XawListHighlight(listwidg, 0);
//    XtAugmentTranslations(listwidg,
//			  XtParseTranslationTable(gameListOptTranslations));

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_load =
      XtCreateManagedWidget(_("factory"), commandWidgetClass, form, args, j);
    XtAddCallback(b_load, XtNcallback, GameListOptionsCallback, client_data);

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_load);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_loadprev =
      XtCreateManagedWidget(_("up"), commandWidgetClass, form, args, j);
    XtAddCallback(b_loadprev, XtNcallback, GameListOptionsCallback, client_data);

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_loadprev);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_loadnext =
      XtCreateManagedWidget(_("down"), commandWidgetClass, form, args, j);
    XtAddCallback(b_loadnext, XtNcallback, GameListOptionsCallback, client_data);

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_loadnext);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_cancel =
      XtCreateManagedWidget(_("cancel"), commandWidgetClass, form, args, j);
    XtAddCallback(b_cancel, XtNcallback, GameListOptionsCallback, client_data);

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_cancel);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_close =
      XtCreateManagedWidget(_("OK"), commandWidgetClass, form, args, j);
    XtAddCallback(b_close, XtNcallback, GameListOptionsCallback, client_data);

    strcpy(lpUserGLT, appData.gameListTags);
    GLT_TagsToList(lpUserGLT);

    XtRealizeWidget(shell);
    //    CatchDeleteWindow(shell, "GameListOptionsPopDown");

    return shell;
}

void
GameListOptionsPopUp(Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    Arg args[16];
    int j, nstrings;
    Widget listwidg;

    if (gameListOptShell == NULL) {
	gameListOptShell = GameListOptionsCreate(); 
    }

    XtPopup(gameListOptShell, XtGrabNone);
}


