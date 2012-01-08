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

#include <gtk/gtk.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "xedittags.h"
#include "gettext.h"


Position tagsX = -1, tagsY = -1;

void TagsPopUp(tags, msg)
     char *tags, *msg;
{
    NewTagsPopup(tags, cmailMsgLoaded ? msg : NULL);
}


void EditTagsPopUp(tags, dest)
     char *tags;
     char **dest;
{
    NewTagsPopup(tags, NULL);
}

void TagsPopDown()
{
    PopDown(2);
    bookUp = False;
}

G_MODULE_EXPORT void EditTagsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    if (!bookUp && PopDown(2)) {
      /* GTK-TODO: do we need to unset flag in menu that EditTags is up? 
         testing it: seems to be ok without? does this handle a special case?*/
    } else {
	EditTagsEvent();
    }
}
