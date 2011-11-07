/*
 * xgamelist.c -- Game list window, part of GTK front end for XBoard
 *
 * Copyright 1995, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "xgamelist.h"
#include "gettext.h"
#include "gtk_helper.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

//static Widget filterText;
static char filterString[MSG_SIZ];
static int listLength, wins, losses, draws, page;

static GtkWidget        *GUI_GameList=NULL;
static GtkTreeView      *GUI_GameListView=NULL;
static GtkListStore     *LIST_GameListStore=NULL;
static GtkTreeSelection *SEL_GameList=NULL;

static FILE *GameListFp=NULL;
static char *GameListFilename=NULL;

//char gameListTranslations[] =
//  "<Btn1Up>(2): LoadSelectedProc(0) \n \
//   <Key>Home: LoadSelectedProc(-2) \n \
//   <Key>End: LoadSelectedProc(2) \n \
//   <Key>Up: LoadSelectedProc(-1) \n \
//   <Key>Down: LoadSelectedProc(1) \n \
//   <Key>Left: LoadSelectedProc(-1) \n \
//   <Key>Right: LoadSelectedProc(1) \n \
//   <Key>Return: LoadSelectedProc(0) \n";
//char filterTranslations[] =
//  "<Key>Return: SetFilterProc() \n";

static int
GameListPrepare()
{
  int  i=0,nstrings;
  ListGame *lg;
  GtkTreeIter iter;

  if(LIST_GameListStore==NULL) return 0;

  /* first clear everything, do we need this? */
  gtk_list_store_clear(LIST_GameListStore);

  /* fill list with information */
  lg = (ListGame *) gameList.head;
  nstrings = ((ListGame *) gameList.tailPred)->number;
  while (nstrings--)
    {
      GameInfo *gameInfo = &(lg->gameInfo);
      char *event = (gameInfo->event && strcmp(gameInfo->event, "?") != 0) ?
	gameInfo->event : gameInfo->site ? gameInfo->site : "?";
      char *site  = gameInfo->site  ? gameInfo->site : "?";
      char *date  = gameInfo->date  ? gameInfo->date : "?";
      char *round = gameInfo->round ? gameInfo->round : "?";
      char *white = gameInfo->white ? gameInfo->white : "?";
      char *black = gameInfo->black ? gameInfo->black : "?";
      char *timeControl = gameInfo->timeControl ? gameInfo->timeControl : "?";
      int whiteRating = gameInfo->whiteRating;
      int blackRating = gameInfo->blackRating;
      char *outOfBook = gameInfo->outOfBook ? gameInfo->outOfBook : "?";
      char *resultDetails = gameInfo->resultDetails ? gameInfo->resultDetails : "?";
      char *variant = VariantName(gameInfo->variant);

      gtk_list_store_append (LIST_GameListStore, &iter);
      gtk_list_store_set (LIST_GameListStore, &iter,
			  0, lg->number,
                          1, event,
			  2, site,
			  3, date,
			  4, round,
                          5, white,
			  6, whiteRating,
			  7, black,
			  8, blackRating,
			  9, PGNResult(gameInfo->result),
			  10, resultDetails,
			  11, timeControl,
			  12, variant,
			  13, outOfBook,
                          -1);
      lg = (ListGame *) lg->node.succ;
      i++;
    }
  return i;
}

/*
void
GameListCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int j;
    Widget listwidg;
    //XawListReturnStruct *rs;
    int index;

}
*/

void
GameListPopUp(fp, filename)
     FILE *fp;
     char *filename;
{
  GtkBuilder *builder=NULL;
  gchar *gladefilename;

  builder = gtk_builder_new ();
  GError *gtkerror=NULL;

  gladefilename = get_glade_filename ("gamelist.glade");
  if(! gtk_builder_add_from_file (builder, gladefilename, &gtkerror) )
    if(gtkerror)
      printf ("Error: %d %s\n",gtkerror->code,gtkerror->message);

  /* need a reference to the window */
  GUI_GameList = GTK_WIDGET (gtk_builder_get_object (builder, "GameList"));
  if(!GUI_GameList) printf("Error: gtk_builder didn't work (GameList)!\n");

  /* and one to the GameListStore */
  LIST_GameListStore = GTK_LIST_STORE (gtk_builder_get_object (builder, "GameListStore"));
  if(!LIST_GameListStore) printf("Error: gtk_builder didn't work (GameListStore)!\n");

  /* and one to the TreeView */
  GUI_GameListView = GTK_TREE_VIEW (gtk_builder_get_object (builder, "GameListView"));
  if(!GUI_GameListView) printf("Error: gtk_builder didn't work (GameListView)!\n");

  SEL_GameList = gtk_tree_view_get_selection(GUI_GameListView);

  gtk_builder_connect_signals (builder, NULL);
  g_object_unref (G_OBJECT (builder));

  /* make it so that only one item can be selected and is selected the whole time*/
  gtk_tree_selection_set_mode(SEL_GameList,GTK_SELECTION_BROWSE);

  gtk_window_set_title(GTK_WINDOW(GUI_GameList),filename);

  GameListPrepare();
  /* remember the filename, needed for callback */
  GameListFp=fp;
  if(GameListFilename)
    free(GameListFilename);
  GameListFilename = strdup(filename);

  /* set old window size and position if available */
  if(wpGameList.width > 0)
    restore_window_placement(GTK_WINDOW(GUI_GameList), &wpGameList);
  else /* set some defaults */
    {
      gtk_window_set_default_size(GTK_WINDOW(GUI_GameList), squareSize*6, squareSize*3);
      gtk_window_resize ( GTK_WINDOW(GUI_GameList), squareSize*6, squareSize*3);
    }
    page = 0;
    //    GameListReplace(0); // [HGM] filter: code put in separate routine, and also called to set title

  /* show widget and focus*/
  gtk_window_present(GTK_WINDOW(GUI_GameList));
  
  /* GTK-mark gamelist menu as active */
  SetCheckMenuItemActive(NULL, 102, True); 

  return;
}

void
GameListDestroy()
{
  if (LIST_GameListStore == NULL) return;

  GameListPopDown();

  g_object_unref (G_OBJECT (LIST_GameListStore));
  g_object_unref (G_OBJECT (GUI_GameList));
  g_object_unref (G_OBJECT (GUI_GameListView));
  g_object_unref (G_OBJECT (SEL_GameList));

  LIST_GameListStore = NULL;
  GUI_GameList       = NULL;
  SEL_GameList       = NULL;
  GUI_GameListView   = NULL;

  return;
}

void ShowGameListProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    if(GUI_GameList)
      {
	if(((GTK_WIDGET_FLAGS(GUI_GameList) & GTK_MAPPED) != 0)) {
	  gtk_widget_hide(GUI_GameList);
          SetCheckMenuItemActive(NULL, 102, False); // set GTK menu item to unchecked
        }
	else {
	  gtk_widget_show(GUI_GameList);
          SetCheckMenuItemActive(NULL, 102, True); // set GTK menu item to checked
        }
      }
    else
      {
          SetCheckMenuItemActive(NULL, 102, False); // set GTK menu item to unchecked
      }

    if(lastLoadGameNumber)
      GameListHighlight(lastLoadGameNumber);
    
    return;
}

void
LoadSelectedProc (GtkTreeView        *treeview,
		  GtkTreePath        *path,
		  GtkTreeViewColumn  *col,
		  gpointer            userdata)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  model = gtk_tree_view_get_model(treeview);

  if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gint nr;

      gtk_tree_model_get(model, &iter, 0, &nr, -1);

      if (cmailMsgLoaded)
	CmailLoadGame(GameListFp, nr, GameListFilename, True);
      else
	LoadGame(GameListFp, nr, GameListFilename, True);
    }

  return;
}

//void
//LoadSelectedProc(w, event, prms, nprms)
//     Widget w;
//     XEvent *event;
//     String *prms;
//     Cardinal *nprms;
//{
//    Widget listwidg;
//    XawListReturnStruct *rs;
//    int index, direction = atoi(prms[0]);
//
//    if (glc == NULL) return;
//    listwidg = XtNameToWidget(glc->shell, "*form.viewport.list");
//    rs = XawListShowCurrent(listwidg);
//    index = rs->list_index;
//    if (index < 0) return;
//    if(direction != 0) {
//	index += direction;
//	if(direction == -2) index = 0;
//	if(direction == 2) index = listLength-1;
//	if(index < 0 || index >= listLength) return;
//	XawListHighlight(listwidg, index);
//	return;
//    }
//    index = atoi(glc->strings[index])-1; // [HGM] filter: read true index from sequence nr of line
//    if (cmailMsgLoaded) {
//	CmailLoadGame(glc->fp, index + 1, glc->filename, True);
//    } else {
//	LoadGame(glc->fp, index + 1, glc->filename, True);
//    }
//}

/*
void
SetFilterProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
//	Arg args[16];
//        String name;
//	Widget list;
//        int j = 0;
//        XtSetArg(args[j], XtNstring, &name);  j++;
//	XtGetValues(filterText, args, j);
//        safeStrCpy(filterString, name, sizeof(filterString)/sizeof(filterString[0]));
//        GameListPrepare(False); GameListReplace(0);
//	list = XtNameToWidget(glc->shell, "*form.viewport.list");
//	XawListHighlight(list, 0);
//        j = 0;
//	XtSetArg(args[j], XtNdisplayCaret, False); j++;
//	XtSetValues(filterText, args, j);
//	XtSetKeyboardFocus(glc->shell, list);
}
*/

void
GameListPopDown()
{
    //Arg args[16];
    int j;

    if(GUI_GameList == NULL) return;

    /* lets save the position and size*/
    save_window_placement(GTK_WINDOW(GUI_GameList), &wpGameList);
    gtk_widget_hide (GUI_GameList);

    /* GTK-TODO mark as down in menu bar */
    //j = 0;
    //XtSetArg(args[j], XtNleftBitmap, None); j++;
    //XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Game List"),
//		args, j);

    return;
}

void
GameListHighlight(index)
     int index;
{
  GtkTreePath *path = gtk_tree_path_new_from_indices(index);

  if(!GUI_GameListView) return;

  /* set cursor */
  gtk_tree_view_set_cursor (GTK_TREE_VIEW (GUI_GameListView), path,
                            NULL, FALSE);
  /* select path */
  gtk_tree_selection_select_path( SEL_GameList , path);

  return;
}

Boolean
GameListIsUp()
{
  return (!!GUI_GameList);
}

int SaveGameListAsText(FILE *f)
{
    ListGame * lg = (ListGame *) gameList.head;
    int nItem;

    if( ((ListGame *) gameList.tailPred)->number <= 0 ) {
      DisplayError("Game list not loaded or empty", 0);
      return False;
    }

    /* Copy the list into the global memory block */
    if( f != NULL ) {
        lg = (ListGame *) gameList.head;

        for (nItem = 0; nItem < ((ListGame *) gameList.tailPred)->number; nItem++){
            char * st = GameListLineFull(lg->number, &lg->gameInfo);
	    char *line = GameListLine(lg->number, &lg->gameInfo);
	    if(filterString[0] == NULLCHAR || SearchPattern( line, filterString ) )
	            fprintf( f, "%s\n", st );
	    free(st); free(line);
            lg = (ListGame *) lg->node.succ;
        }

        fclose(f);
	return True;
    }
    return False;
}
//--------------------------------- Game-List options dialog ------------------------------------------

//Widget gameListOptShell, listwidg;

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

Boolean GLT_GetFromList(int index, char *name)
{
  safeStrCpy(name, strings[index], MSG_SIZ);
  return TRUE;
}


void GLT_DeSelectList()
{
//    XawListChange(listwidg, strings, 0, 0, True);
//    XawListHighlight(listwidg, 0);
}



/***********************game list option */

void
GameListOptionsPopDown()
{
/*
  if (gameListOptShell == NULL) return;

  XtPopdown(gameListOptShell);
  XtDestroyWidget(gameListOptShell);
  gameListOptShell = 0;
  XtSetKeyboardFocus(shellWidget, formWidget);
*/
}

/*
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
      safeStrCpy(lpUserGLT, GLT_DEFAULT_TAGS, LPUSERGLT_SIZE);
      GLT_TagsToList(lpUserGLT);
      index = 0;
    }
    XawListHighlight(listwidg, index);
}
*/

/*
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
    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
    XtSetArg(args[j], XtNforceColumns, True);  j++;
    XtSetArg(args[j], XtNverticalList, True);  j++;
    listwidg = viewport =
      XtCreateManagedWidget("list", listWidgetClass, form, args, j);
    XawListHighlight(listwidg, 0);
//    XtAugmentTranslations(listwidg,
//			  XtParseTranslationTable(gameListOptTranslations));

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

    safeStrCpy(lpUserGLT, appData.gameListTags, LPUSERGLT_SIZE);
    GLT_TagsToList(lpUserGLT);

    XtRealizeWidget(shell);

    return shell;
}
*/

void GameListOptionsPopUpGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
//  if (gameListOptShell == NULL)
 //   gameListOptShell = GameListOptionsCreate();

 // XtPopup(gameListOptShell, XtGrabNone); 
}


