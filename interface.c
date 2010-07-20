/*
 * interface.c -- gtk-interface
 *
 * Copyright 2009, 2010 Free Software Foundation, Inc.
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
#include <stdlib.h> // for exit()
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "moves.h"
#include "xboard.h"
#include "childio.h"
#include "xgamelist.h"
#include "xhistory.h"
#include "xedittags.h"
#include "gettext.h"
#include "callback.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


GdkPixbuf *
load_pixbuf(char *filename,int size)
{
  GdkPixbuf *image;

  if(size)
    image = gdk_pixbuf_new_from_file_at_size(filename,size,size,NULL);
  else
    image = gdk_pixbuf_new_from_file(filename,NULL);

  if(image == NULL)
    {
      fprintf(stderr,_("Error: couldn't load file: %s\n"),filename);
      exit(EXIT_FAILURE);
    }
  return image;
}

void
FileNamePopUp(label, def, proc, openMode)
     char *label;
     char *def;
     FileProc proc;
     char *openMode;
{
  /* TODO:
   *   implement look for certain file types
   *   use save/load button depending on what function is calling
   */

  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new (label,
					GTK_WINDOW(GUI_Window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      FILE *f;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      //see loadgamepopup
      f = fopen(filename, openMode);
      if (f == NULL)
	{
	  DisplayError(_("Failed to open file"), errno);
	}
      else
	{
	  /* TODO add indec */
	  (*proc)(f, 0, filename);
	}
      g_free (filename);
    };

  gtk_widget_destroy (dialog);
  ModeHighlight();

  return;

}
