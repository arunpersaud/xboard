/*
 * xgamelistopt.c -- Game list options dialog, part of X front end for XBoard
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


extern Widget formWidget, shellWidget, boardWidget, menuBarWidget;
extern Display *xDisplay;
extern int squareSize;
extern Pixmap xMarkPixmap;
extern char *layoutName;
extern char lpUserGLT[];

char gameListOptTranslations[] =
  "<Btn1Up>(2): LoadSelectedProc() \n \
   <Key>Return: LoadSelectedProc() \n";

static Arg layoutArgs[] = {
    { XtNborderWidth, 0 },
    { XtNdefaultDistance, 0 }
};

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

void GLT_GetFromList(int index, char *name)
{
    strcpy(name, strings[index]);
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
    XtSetKeyboardFocus(shellWidget, formWidget);
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
    shell = gameListOptShell =
      XtCreatePopupShell("Game-list options", transientShellWidgetClass,
			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    j = 0;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout, args, j);

    j = 0;
//    XtSetArg(args[j], XtNlist, glc->strings);  j++;
    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
    XtSetArg(args[j], XtNforceColumns, True);  j++;
    XtSetArg(args[j], XtNverticalList, True);  j++;
    listwidg = viewport =
      XtCreateManagedWidget("list", listWidgetClass, form, args, j);
    XawListHighlight(listwidg, 0);
    XtAugmentTranslations(listwidg,
			  XtParseTranslationTable(gameListOptTranslations));

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
    CatchDeleteWindow(shell, "GameListOptionsPopDown");

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


