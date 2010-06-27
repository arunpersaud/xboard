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
#include <glib.h>

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
extern GtkWidget               *GUI_TagBox;

extern GameInfo gameInfo;

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
  gtk_widget_hide (GUI_EditTags);
  return;
}

void 
TagsPopUp(GameInfo *gameInfo, char *msg)
{
  EditTagsPopUp(gameInfo);
  return;
}

/* helper */
void 
TagToGameInfo(label, content, gameInfo)
     char *label;
     char *content;
     GameInfo *gameInfo;
{
  int success=FALSE;
  
  if (StrCaseCmp(label, "Event") == 0) 
	success = StrSavePtr(content, &gameInfo->event) != NULL;
  else if (StrCaseCmp(label, "Site") == 0)
    success = StrSavePtr(content, &gameInfo->site) != NULL;
  else if (StrCaseCmp(label, "Date") == 0) 
    success = StrSavePtr(content, &gameInfo->date) != NULL;
  else if (StrCaseCmp(label, "Round") == 0) 
    success = StrSavePtr(content, &gameInfo->round) != NULL;
  else if (StrCaseCmp(label, "White") == 0) 
    success = StrSavePtr(content, &gameInfo->white) != NULL;
  else if (StrCaseCmp(label, "Black") == 0) 
    success = StrSavePtr(content, &gameInfo->black) != NULL;
  else if (StrCaseCmp(label, "WhiteElo")==0)
    {
      success = TRUE;
      gameInfo->whiteRating = atoi( content );
    } 
  else if (StrCaseCmp(label, "BlackElo")==0)
    {
      success = TRUE;
      gameInfo->blackRating = atoi( content );
    }
  else if (StrCaseCmp(label, "Result") == 0) 
    {
      if (strcmp(content, "1-0") == 0)
	gameInfo->result = WhiteWins;
      else if (strcmp(content, "0-1") == 0)
	gameInfo->result = BlackWins;
      else if (strcmp(content, "1/2-1/2") == 0)
	gameInfo->result = GameIsDrawn;
      else
	gameInfo->result = GameUnfinished;
      success = TRUE;
    } 
  else if (StrCaseCmp(label, "TimeControl") == 0) 
    success = StrSavePtr(content, &gameInfo->timeControl) != NULL;
  else if (StrCaseCmp(label, "Variant") == 0) 
    {
        /* xboard-defined extension */
        gameInfo->variant = StringToVariant(content);
	success = TRUE;
    }
  if(! success)
    printf("Warning: problem converting Tags, please file a bug report\n");

  return;
}
/* end helper */

/* Callbacks */
void
EditTagClearProc(GtkObject *object, gpointer user_data)
{
  /* go through all tags in the widget and clear the text input boxes */
  
  GList *hboxes;
  GList *outer, *inner;  /* iterator for for-loops */

  hboxes = gtk_container_get_children (GTK_CONTAINER( GUI_TagBox) );

  for ( outer = g_list_first(hboxes); outer != NULL; outer = g_list_next(outer))
    {
      GList *tags;
      tags = gtk_container_get_children (GTK_CONTAINER( outer->data) );

      for ( inner = g_list_first(tags); inner != NULL; inner = g_list_next(inner))
	{
	  if(GTK_IS_ENTRY(inner->data))
	    gtk_entry_set_text(GTK_ENTRY(inner->data),"");
	}
      g_list_free(tags);
    }
  g_list_free(hboxes);

  return;
}

void
EditTagCancelProc(GtkObject *object, gpointer user_data)
{
  TagsPopDown();

  /* go through all tags in the widget and clear the text input boxes */
  
  GList *hboxes;
  GList *outer, *inner;  /* iterator for for-loops */

  hboxes = gtk_container_get_children (GTK_CONTAINER( GUI_TagBox) );

  for ( outer = g_list_first(hboxes); outer != NULL; outer = g_list_next(outer))
    {
      GList *tags;
      tags = gtk_container_get_children (GTK_CONTAINER( outer->data) );

      for ( inner = g_list_first(tags); inner != NULL; inner = g_list_next(inner))
	{
	  if(GTK_IS_ENTRY(inner->data))
	    gtk_entry_set_text(GTK_ENTRY(inner->data),"");
	}
      g_list_free(tags);
    }
  g_list_free(hboxes);


  return;
}

void
EditTagOKProc(GtkObject *object, gpointer user_data)
{
  G_CONST_RETURN gchar *label;
  G_CONST_RETURN gchar *content;
  

  /* go through all tags in the widget and clear the text input boxes */
  
  GList *hboxes;
  GList *outer, *inner;  /* iterator for for-loops */

  printf("need to OK new TAGS\n");


  hboxes = gtk_container_get_children (GTK_CONTAINER( GUI_TagBox) );

  for ( outer = g_list_first(hboxes); outer != NULL; outer = g_list_next(outer))
    {
      label   = "unknown";
      content = "?";
      
      GList *tags;
      tags = gtk_container_get_children (GTK_CONTAINER( outer->data) );

      for ( inner = g_list_first(tags); inner != NULL; inner = g_list_next(inner))
	{
	  if(GTK_IS_LABEL(inner->data))
	    label = gtk_label_get_text(GTK_LABEL(inner->data));
	  if(GTK_IS_ENTRY(inner->data))
	    content = gtk_entry_get_text(GTK_ENTRY(inner->data));
	}
      printf ("New TAG: %s -> %s\n",label,content);
//      TagToGameInfo(label,content,gameInfo);
      g_list_free(tags);
    }
  g_list_free(hboxes);
  

  TagsPopDown();

  return;
}

/* end Callbacks */


void 
EditTagsPopUp(gameInfo)
     GameInfo *gameInfo;
{
  
  GtkWidget *tmp;
  char Elo[12];


  /* remove old tags that we already added */
  gtk_container_foreach(GTK_CONTAINER (GUI_TagBox),(GtkCallback) (gtk_widget_destroy),NULL);


  /* add new ones */
  tmp = GTK_WIDGET ( GTK_create_tag("Event", gameInfo->event) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);

  tmp = GTK_WIDGET ( GTK_create_tag("Site", gameInfo->site) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);

  tmp = GTK_WIDGET ( GTK_create_tag("Date", gameInfo->date) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);

  tmp = GTK_WIDGET ( GTK_create_tag("Round", gameInfo->round) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);
    
  tmp = GTK_WIDGET ( GTK_create_tag("White", gameInfo->white) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);
    
  tmp = GTK_WIDGET ( GTK_create_tag("Black", gameInfo->black) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);
    
  tmp = GTK_WIDGET ( GTK_create_tag("Result", PGNResult(gameInfo->result)) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);
    
  sprintf(Elo,"%11d",gameInfo->whiteRating);
  tmp = GTK_WIDGET ( GTK_create_tag("WhiteElo", Elo) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);

  sprintf(Elo,"%11d",gameInfo->blackRating);
  tmp = GTK_WIDGET ( GTK_create_tag("BlackElo", Elo) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);

  tmp = GTK_WIDGET ( GTK_create_tag("TimeControl", gameInfo->timeControl) );
  gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);

  if (gameInfo->variant != VariantNormal) 
    {
      tmp = GTK_WIDGET ( GTK_create_tag("Variant", gameInfo->variant) );
      gtk_box_pack_start(GTK_BOX(GUI_TagBox), tmp, TRUE, FALSE, 4);
    }

  /* realize widget */
  gtk_widget_show_all (GUI_EditTags);

  return;
}

GtkWidget *
GTK_create_tag(gchar *tagname, gchar *tagcontent )
{
  GtkWidget *box;

  GtkWidget *label;  

  GtkWidget *content;
  GtkEntryBuffer *contentbuffer;

  box = gtk_hbox_new(1,1);
  
  label = gtk_label_new (tagname);
  gtk_label_set_justify(GTK_LABEL(label),  GTK_JUSTIFY_RIGHT);

  contentbuffer = gtk_entry_buffer_new(tagcontent,sizeof(tagcontent));
  content =  gtk_entry_new_with_buffer (contentbuffer);     

  gtk_box_pack_start(GTK_BOX(box), label, TRUE, FALSE,  2);
  gtk_box_pack_start(GTK_BOX(box), content, TRUE, FALSE,  2);


  return box;
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
