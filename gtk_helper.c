/*
 * gtk_helper.c -- helper functions for the GTK frontend of XBoard
 *
 * Copyright 2011 Free Software Foundation, Inc.
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
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "common.h"

gchar* get_glade_filename(filename)
     char *filename;
{
  gchar *gladefilename=NULL;

  gladefilename = g_build_filename ("gtk/",filename, NULL);
  if (g_file_test (gladefilename, G_FILE_TEST_EXISTS) == FALSE)
    {
      g_free(gladefilename);

      gladefilename = g_build_filename (GLADEDIR,filename, NULL);
      if (g_file_test (gladefilename, G_FILE_TEST_EXISTS) == FALSE)
	{
	  g_free(gladefilename);
	  printf ("Error: can not find ui-file for %s\n",filename);
	  return NULL;
	}
    }

  return gladefilename;
}

void save_window_placement(window, placement)
     GtkWindow *window;
     WindowPlacement *placement;
{
  gint x,y,w,h;

  if(!window) return;

  gtk_window_get_position(window,&x,&y);
  gtk_window_get_size(window,&w,&h);

  placement->x = x;
  placement->y = y;
  placement->width  = w;
  placement->height = h;

  return;
}

void restore_window_placement(window, placement)
     GtkWindow *window;
     WindowPlacement *placement;
{
  gint x = placement->x;
  gint y = placement->y;
  gint w = placement->width;
  gint h = placement->height;

  gtk_window_set_default_size(window, w,h);

  gtk_window_move   ( window, x,y);
  gtk_window_resize ( window, w,h);

  return;
}
