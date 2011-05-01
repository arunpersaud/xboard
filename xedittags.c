/*
 * xedittags.c -- Tags edit window, part of X front end for XBoard
 *
 * Copyright 1995, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include "xedittags.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

Position tagsX = -1, tagsY = -1;
int tagsUp = False, editTagsUp = False;
Widget tagsShell, editTagsShell;

void TagsCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int j;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("close")) == 0) {
	TagsPopDown();
    } else if (strcmp(name, _("edit")) == 0) {
	TagsPopDown();
	EditTagsEvent();
    }
}


void EditTagsCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name, val;
    Arg args[16];
    int j;
    Widget textw;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("ok")) == 0) {
	textw = XtNameToWidget(editTagsShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, &val); j++;
	XtGetValues(textw, args, j);
	ReplaceTags(val, &gameInfo);
	TagsPopDown();
    } else if (strcmp(name, _("cancel")) == 0) {
	TagsPopDown();
    } else if (strcmp(name, _("clear")) == 0) {
	textw = XtNameToWidget(editTagsShell, "*form.text");
	XtCallActionProc(textw, "select-all", NULL, NULL, 0);
	XtCallActionProc(textw, "kill-selection", NULL, NULL, 0);
    }
}

Widget TagsCreate(name, text, msg, mutable, callback)
     char *name, *text, *msg;
     int /*Boolean*/ mutable;
     XtCallbackProc callback;
{
    Arg args[16];
    Widget shell, form, textw, msgw, layout;
    Widget b_ok, b_cancel, b_close, b_edit, b;
    Dimension bw_width, pw_width;
    Dimension pw_height;
    int j, xx, yy;
    Window junk;

    j = 0;
    XtSetArg(args[j], XtNwidth, &bw_width);  j++;
    XtGetValues(boardWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
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
    if (mutable) {
	XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    }
    XtSetArg(args[j], XtNstring, text);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtRubber);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth, bw_width/2);  j++;
    XtSetArg(args[j], XtNheight, bw_width/3);  j++;
    /* !!Work around an apparent bug in XFree86 4.0.1 (X11R6.4.3) */
    XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++;
    XtSetArg(args[j], XtNautoFill, False);  j++;
    textw =
      XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j);

    if (cmailMsgLoaded && !mutable) {
	j = 0;
	XtSetArg(args[j], XtNfromVert, textw);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainRight); j++;
	XtSetArg(args[j], XtNborderWidth, 0); j++;
	XtSetArg(args[j], XtNjustify, XtJustifyLeft); j++;
	XtSetArg(args[j], XtNlabel, msg); j++;
	msgw =
	  XtCreateManagedWidget("msg", labelWidgetClass, form, args, j);
    } else {
	msgw = textw;
    }
    if (mutable) {
	j = 0;
	XtSetArg(args[j], XtNfromVert, msgw);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_ok = b =
	  XtCreateManagedWidget(_("ok"), commandWidgetClass, form, args, j);
	XtAddCallback(b_ok, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, msgw);  j++;
	XtSetArg(args[j], XtNfromHoriz, b);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_cancel = b =
	  XtCreateManagedWidget(_("cancel"), commandWidgetClass, form, args, j);
	XtAddCallback(b_cancel, XtNcallback, callback, (XtPointer) 0);

    } else {
	j = 0;
	XtSetArg(args[j], XtNfromVert, msgw);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_close = b =
	  XtCreateManagedWidget(_("close"), commandWidgetClass, form, args, j);
	XtAddCallback(b_close, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, msgw);  j++;
	XtSetArg(args[j], XtNfromHoriz, b);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_edit = b =
	  XtCreateManagedWidget(_("edit"), commandWidgetClass, form, args, j);
	XtAddCallback(b_edit, XtNcallback, callback, (XtPointer) 0);
    }

    XtRealizeWidget(shell);
    CatchDeleteWindow(shell, "TagsPopDown");

    if (tagsX == -1) {
	j = 0;
	XtSetArg(args[j], XtNwidth, &bw_width);  j++;
	XtGetValues(boardWidget, args, j);
	j = 0;
	XtSetArg(args[j], XtNwidth, &pw_width);  j++;
	XtSetArg(args[j], XtNheight, &pw_height);  j++;
	XtGetValues(shell, args, j);

#ifdef NOTDEF
	/* This code seems to tickle an X bug if it is executed too soon
	   after xboard starts up.  The coordinates get transformed as if
	   the main window was positioned at (0, 0).
	   */
	XtTranslateCoords(boardWidget, (bw_width - pw_width) / 2,
			  0 - pw_height + squareSize / 3, &x, &y);
#else
	XTranslateCoordinates(xDisplay, XtWindow(boardWidget),
			      RootWindowOfScreen(XtScreen(boardWidget)),
			      (bw_width - pw_width) / 2,
			      0 - pw_height + squareSize / 3, &xx, &yy, &junk);
	tagsX = xx;
	tagsY = yy;
#endif
	if (tagsY < 0) tagsY = 0; /*avoid positioning top offscreen*/
    }
    j = 0;
    XtSetArg(args[j], XtNx, tagsX - appData.borderXoffset);  j++;
    XtSetArg(args[j], XtNy, tagsY - appData.borderYoffset);  j++;
    XtSetValues(shell, args, j);
    XtSetKeyboardFocus(shell, textw);

    return shell;
}


void TagsPopUp(tags, msg)
     char *tags, *msg;
{
    Arg args[16];
    int j;
    Widget textw, msgw;

    if (editTagsUp) TagsPopDown();
    if (tagsShell == NULL) {
	tagsShell =
	  TagsCreate(_("Tags"), tags, msg, False, TagsCallback);
    } else {
	textw = XtNameToWidget(tagsShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, tags); j++;
	XtSetValues(textw, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) "Tags");  j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) _("Tags"));  j++;
	XtSetValues(tagsShell, args, j);
	msgw = XtNameToWidget(tagsShell, "*form.msg");
	if (msgw) {
	    j = 0;
	    XtSetArg(args[j], XtNlabel, msg); j++;
	    XtSetValues(msgw, args, j);
	}
    }

    XtPopup(tagsShell, XtGrabNone);
    XSync(xDisplay, False);

    tagsUp = True;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Tags"),
		args, j);
}


void EditTagsPopUp(tags, dest)
     char *tags;
     char **dest;
{
    Widget textw;
    Arg args[16];
    int j;

    if (tagsUp) TagsPopDown();
    if (editTagsShell == NULL) {
	editTagsShell =
	  TagsCreate(_("Edit tags"), tags, NULL, True, EditTagsCallback); 
    } else {
	textw = XtNameToWidget(editTagsShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, tags); j++;
	XtSetValues(textw, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) "Edit Tags");  j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) _("Edit Tags"));  j++;
	XtSetValues(editTagsShell, args, j);
    }

    XtPopup(editTagsShell, XtGrabNone);

    editTagsUp = True;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Tags"),
		args, j);
}

void TagsPopDown()
{
    Arg args[16];
    int j;
    Widget w;

    if (tagsUp) {
	w = tagsShell;
    } else if (editTagsUp) {
	w = editTagsShell;
    } else {
	return;
    }
    j = 0;
    XtSetArg(args[j], XtNx, &tagsX); j++;
    XtSetArg(args[j], XtNy, &tagsY); j++;
    XtGetValues(w, args, j);
    XtPopdown(w);
    XSync(xDisplay, False);
    tagsUp = editTagsUp = False;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Tags"),
		args, j);
}

void
EditTagsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (tagsUp) TagsPopDown();
    if (editTagsUp) {
	TagsPopDown();
    } else {
	EditTagsEvent();
    }
}
