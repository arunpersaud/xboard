/*
 * xgamelist.c -- Game list window, part of X front end for XBoard
 * $Id: xgamelist.c,v 2.1 2003/10/27 19:21:00 mann Exp $
 *
 * Copyright 1995 Free Software Foundation, Inc.
 *
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
 * ------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ------------------------------------------------------------------------
 *
 * See the file ChangeLog for a revision history.
 */

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

extern Widget formWidget, shellWidget, boardWidget, menuBarWidget;
extern Display *xDisplay;
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
    Arg args[16];
    Widget shell, form, viewport, listwidg, layout;
    Widget b_load, b_loadprev, b_loadnext, b_close;
    Dimension fw_width;
    int j;
    GameListClosure *glc = (GameListClosure *) client_data;

    j = 0;
    XtSetArg(args[j], XtNwidth, &fw_width);  j++;
    XtGetValues(formWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNallowShellResize, True);  j++;
#if TOPLEVEL
    shell =
      XtCreatePopupShell(name, topLevelShellWidgetClass,
			 shellWidget, args, j);
#else
    shell =
      XtCreatePopupShell(name, transientShellWidgetClass,
			 shellWidget, args, j);
#endif
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    j = 0;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout, args, j);

    j = 0;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNresizable, False);  j++;
    XtSetArg(args[j], XtNwidth, fw_width);  j++;
    XtSetArg(args[j], XtNallowVert, True); j++;
    viewport =
      XtCreateManagedWidget("viewport", viewportWidgetClass, form, args, j);

    j = 0;
    XtSetArg(args[j], XtNlist, glc->strings);  j++;
    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
    XtSetArg(args[j], XtNforceColumns, True);  j++;
    XtSetArg(args[j], XtNverticalList, True);  j++;
    listwidg = 
      XtCreateManagedWidget("list", listWidgetClass, viewport, args, j);
    XawListHighlight(listwidg, 0);
    XtAugmentTranslations(listwidg,
			  XtParseTranslationTable(gameListTranslations));

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_load =
      XtCreateManagedWidget("load", commandWidgetClass, form, args, j);
    XtAddCallback(b_load, XtNcallback, callback, client_data);

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_load);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_loadprev =
      XtCreateManagedWidget("prev", commandWidgetClass, form, args, j);
    XtAddCallback(b_loadprev, XtNcallback, callback, client_data);

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_loadprev);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_loadnext =
      XtCreateManagedWidget("next", commandWidgetClass, form, args, j);
    XtAddCallback(b_loadnext, XtNcallback, callback, client_data);

    j = 0;
    XtSetArg(args[j], XtNfromVert, viewport);  j++;
    XtSetArg(args[j], XtNfromHoriz, b_loadnext);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom); j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
    XtSetArg(args[j], XtNleft, XtChainLeft); j++;
    XtSetArg(args[j], XtNright, XtChainLeft); j++;
    b_close =
      XtCreateManagedWidget("close", commandWidgetClass, form, args, j);
    XtAddCallback(b_close, XtNcallback, callback, client_data);

    if (glc->x == -1) {
	Position y1;
	Dimension h1;
	int xx, yy;
	Window junk;

	j = 0;
	XtSetArg(args[j], XtNheight, &h1); j++;
	XtSetArg(args[j], XtNy, &y1); j++;
	XtGetValues(boardWidget, args, j);
	glc->w = fw_width * 3/4;
	glc->h = squareSize * 3;

	XSync(xDisplay, False);
#ifdef NOTDEF
	/* This code seems to tickle an X bug if it is executed too soon
	   after xboard starts up.  The coordinates get transformed as if
	   the main window was positioned at (0, 0).
	*/
	XtTranslateCoords(shellWidget, (fw_width - glc->w) / 2,
			  y1 + (h1 - glc->h + appData.borderYoffset) / 2,
			  &glc->x, &glc->y);
#else /*!NOTDEF*/
        XTranslateCoordinates(xDisplay, XtWindow(shellWidget),
			      RootWindowOfScreen(XtScreen(shellWidget)),
			      (fw_width - glc->w) / 2,
			      y1 + (h1 - glc->h + appData.borderYoffset) / 2,
			      &xx, &yy, &junk);
	glc->x = xx;
	glc->y = yy;
#endif /*!NOTDEF*/
	if (glc->y < 0) glc->y = 0; /*avoid positioning top offscreen*/
    }
    j = 0;
    XtSetArg(args[j], XtNheight, glc->h);  j++;
    XtSetArg(args[j], XtNwidth, glc->w);  j++;
    XtSetArg(args[j], XtNx, glc->x - appData.borderXoffset);  j++;
    XtSetArg(args[j], XtNy, glc->y - appData.borderYoffset);  j++;
    XtSetValues(shell, args, j);

    XtRealizeWidget(shell);
    CatchDeleteWindow(shell, "GameListPopDown");

    return shell;
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

    if (strcmp(name, "close") == 0) {
	GameListPopDown();
	return;
    }
    listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
    rs = XawListShowCurrent(listwidg);
    if (strcmp(name, "load") == 0) {
	index = rs->list_index;
	if (index < 0) {
	    DisplayError("No game selected", 0);
	    return;
	}
    } else if (strcmp(name, "next") == 0) {
	index = rs->list_index + 1;
	if (index >= ((ListGame *) gameList.tailPred)->number) {
	    DisplayError("Can't go forward any further", 0);
	    return;
	}
	XawListHighlight(listwidg, index);
    } else if (strcmp(name, "prev") == 0) {
	index = rs->list_index - 1;
	if (index < 0) {
	    DisplayError("Can't back up any further", 0);
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
    Arg args[16];
    int j, nstrings;
    Widget listwidg;
    ListGame *lg;
    char **st;

    if (glc == NULL) {
	glc = (GameListClosure *) calloc(1, sizeof(GameListClosure));
	glc->x = glc->y = -1;
    }

    if (glc->strings != NULL) {
	st = glc->strings;
	while (*st) {
	    free(*st++);
	}
	free(glc->strings);
    }

    nstrings = ((ListGame *) gameList.tailPred)->number;
    glc->strings = (char **) malloc((nstrings + 1) * sizeof(char *));
    st = glc->strings;
    lg = (ListGame *) gameList.head;
    while (nstrings--) {
	*st++ = GameListLine(lg->number, &lg->gameInfo);
	lg = (ListGame *) lg->node.succ;
     }
    *st = NULL;

    glc->fp = fp;

    if (glc->filename != NULL) free(glc->filename);
    glc->filename = StrSave(filename);

    if (glc->shell == NULL) {
	glc->shell = GameListCreate(filename, GameListCallback, glc); 
    } else {
	listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
	XawListChange(listwidg, glc->strings, 0, 0, True);
	XawListHighlight(listwidg, 0);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) filename);  j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) filename);  j++;
	XtSetValues(glc->shell, args, j);
    }

    XtPopup(glc->shell, XtGrabNone);
    glc->up = True;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Show Game List"),
		args, j);
}

void
GameListDestroy()
{
    if (glc == NULL) return;
    GameListPopDown();
    if (glc->strings != NULL) {
	char **st;
	st = glc->strings;
	while (*st) {
	    free(*st++);
	}
	free(glc->strings);
    }
    free(glc);
    glc = NULL;
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
	DisplayError("There is no game list", 0);
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
    Arg args[16];
    int j;

    if (glc == NULL) return;
    j = 0;
    XtSetArg(args[j], XtNx, &glc->x); j++;
    XtSetArg(args[j], XtNy, &glc->y); j++;
    XtSetArg(args[j], XtNheight, &glc->h); j++;
    XtSetArg(args[j], XtNwidth, &glc->w); j++;
    XtGetValues(glc->shell, args, j);
    XtPopdown(glc->shell);
    XtSetKeyboardFocus(shellWidget, formWidget);
    glc->up = False;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Show Game List"),
		args, j);
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
