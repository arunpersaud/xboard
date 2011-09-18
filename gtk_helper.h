/*
 * gtk_helper.h -- helper functions for the GTK frontend of XBoard
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

#ifndef _GTK_HELPER_H
#define _GTK_HELPER_H 1

gchar *get_glade_filename       P((char *filename));
void   save_window_placement    P((GtkWindow *window,  WindowPlacement *placement));
void   restore_window_placement P((GtkWindow *window,  WindowPlacement *placement));
GdkPixbuf *load_pixbuf          P((char *filename, int size));

#endif


