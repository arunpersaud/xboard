/*
 * xedittags.c -- Tags edit window, part of X front end for XBoard
 *
 * Copyright 1995, 2009, 2010 Free Software Foundation, Inc.
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
#include <gtk/gtk.h>


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
#include "xedittags.h"
#include "gettext.h"

extern GtkWidget               *GUI_EditTags;
extern GtkWidget               *GUI_EditTagsTextArea;

Widget tagsShell, editTagsShell;

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

void 
TagsPopDown()
{
  return;
}

void 
TagsPopUp(char *tags, char *msg)
{
  EditTagsPopUp(tags);
  return;
}


//void EditTagsCallback(w, client_data, call_data)
//     Widget w;
//     XtPointer client_data, call_data;
//{
//    String name, val;
//    Arg args[16];
//    int j;
//    Widget textw;
//
//    j = 0;
//    XtSetArg(args[j], XtNlabel, &name);  j++;
//    XtGetValues(w, args, j);
//    
//
//    if (strcmp(name, _("ok")) == 0) {
//    /* ok: get values, update, close */
//	textw = XtNameToWidget(editTagsShell, "*form.text");
//	j = 0;
//	XtSetArg(args[j], XtNstring, &val); j++;
//	XtGetValues(textw, args, j);
//	ReplaceTags(val, &gameInfo);
//	TagsPopDown();
//    } else if (strcmp(name, _("cancel")) == 0) {
//      /* close */
//	TagsPopDown();
//    } else if (strcmp(name, _("clear")) == 0) {
//      /* clear all */
//	textw = XtNameToWidget(editTagsShell, "*form.text");
//	XtCallActionProc(textw, "select-all", NULL, NULL, 0);
//	XtCallActionProc(textw, "kill-selection", NULL, NULL, 0);
//    }
//}


void 
EditTagsPopUp(tags)
     char *tags;
{
  GtkWidget *label;  
  
  /* add the text to the dialog */
  
  label = gtk_label_new (tags);

  /* remove old tags that we already added */
  gtk_container_foreach(GTK_CONTAINER (GUI_EditTagsTextArea),G_CALLBACK (gtk_widget_destroy),NULL);
  
  /* TODO replace this with a version where you can edit and a callback for the edit button that saves the new tags*/
  gtk_container_add (GTK_CONTAINER (GUI_EditTagsTextArea), label);
  
  /* realize widget */
  gtk_widget_show_all (GUI_EditTags);

  return;
}

void
EditTagsProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  /* always call EditTagsEvent, which calls the function to popup the window */
  EditTagsEvent();
  return;
}

void
EditTagsHideProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  /* hide everything */
  gtk_widget_hide_all(GUI_EditTags);
  return;
}
