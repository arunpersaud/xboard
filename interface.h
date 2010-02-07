/*
 * interface.h -- gtk-interface
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

/* GTK widgets */
GtkBuilder              *builder=NULL;

GtkWidget               *GUI_Window=NULL;
GtkWidget               *GUI_Aspect=NULL;
GtkWidget               *GUI_History=NULL;
GtkWidget               *GUI_GameList=NULL;
GtkWidget               *GUI_Board=NULL;
GtkWidget               *GUI_Whiteclock=NULL;
GtkWidget               *GUI_Blackclock=NULL;
GtkWidget               *GUI_Error=NULL;
GtkWidget               *GUI_Menubar=NULL;
GtkWidget               *GUI_Timer=NULL;
GtkWidget               *GUI_Buttonbar=NULL;
GtkWidget               *GUI_EditTags=NULL;
GtkWidget               *GUI_EditTagsTextArea=NULL;

GtkListStore            *LIST_MoveHistory=NULL;
GtkListStore            *LIST_GameList=NULL;

GtkTreeView             *TREE_History=NULL;
GtkTreeView             *TREE_Game=NULL;

gint                     boardWidth;
gint                     boardHeight;


GdkPixbuf               *WindowIcon=NULL;
GdkPixbuf               *WhiteIcon=NULL;
GdkPixbuf               *BlackIcon=NULL;
#define MAXPIECES 100
GdkPixbuf               *SVGpieces[MAXPIECES];
GdkPixbuf               *SVGLightSquare=NULL;
GdkPixbuf               *SVGDarkSquare=NULL;
GdkPixbuf               *SVGNeutralSquare=NULL;

GdkCursor               *BoardCursor=NULL;


GdkPixbuf *load_pixbuf P((char *filename,int size));
void FileNamePopUp P((char *label, char *def,
		      FileProc proc, char *openMode));
