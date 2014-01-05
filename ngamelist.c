/*
 * ngamelist.c -- Game list window, Xt-independent front-end code for XBoard
 *
 * Copyright 1995, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "dialogs.h"
#include "menus.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


static char filterString[MSG_SIZ];
static int listLength, wins, losses, draws, page;
int narrowFlag;


typedef struct {
    short int x, y;
    short int w, h;
    FILE *fp;
    char *filename;
    char **strings;
} GameListClosure;
static GameListClosure *glc = NULL;

static char *filterPtr;
static char *list[1003];
static int listEnd;

static int GameListPrepare P((int byPos, int narrow));
static void GameListReplace P((int page));
static void GL_Button P((int n));

static Option gamesOptions[] = {
{ 200,  LR|TB,     400, NULL, (void*) list,       NULL, NULL, ListBox, "" },
{   0,  0,         100, NULL, (void*) &filterPtr, "", NULL, TextBox, "" },
{   4,  SAME_ROW,    0, NULL, (void*) &GL_Button, NULL, NULL, Button, N_("find position") },
{   2,  SAME_ROW,    0, NULL, (void*) &GL_Button, NULL, NULL, Button, N_("narrow") }, // buttons referred to by ID in value (=first) field!
{   3,  SAME_ROW,    0, NULL, (void*) &GL_Button, NULL, NULL, Button, N_("thresholds") },
{   9,  SAME_ROW,    0, NULL, (void*) &GL_Button, NULL, NULL, Button, N_("tags") },
{   5,  SAME_ROW,    0, NULL, (void*) &GL_Button, NULL, NULL, Button, N_("next") },
{   6,  SAME_ROW,    0, NULL, (void*) &GL_Button, NULL, NULL, Button, N_("close") },
{   0,  SAME_ROW | NO_OK, 0, NULL, NULL, "", NULL, EndMark , "" }
};

static void
GL_Button (int n)
{
    int index;
    n = gamesOptions[n].value; // use marker in option rather than n itself, for more easy adding/deletng of buttons
    if (n == 6) { // close
	PopDown(GameListDlg);
	return;
    }
    if (n == 3) { // thresholds
	LoadOptionsPopUp(GameListDlg);
	return;
    }
    if (n == 9) { // tags
	GameListOptionsPopUp(GameListDlg);
	return;
    }
    index = SelectedListBoxItem(&gamesOptions[0]);
    if (n == 7) { // load
	if (index < 0) {
	    DisplayError(_("No game selected"), 0);
	    return;
	}
    } else if (n == 5) { // next
	index++;
	if (index >= listLength || !list[index]) {
	    DisplayError(_("Can't go forward any further"), 0);
	    return;
	}
	HighlightWithScroll(&gamesOptions[0], index, listEnd);
    } else if (n == 8) { // prev
	index--;
	if (index < 0) {
	    DisplayError(_("Can't back up any further"), 0);
	    return;
	}
	HighlightWithScroll(&gamesOptions[0], index, listEnd);
    } else if (n == 2 || // narrow
               n == 4) { // find position
	char *text;
	GetWidgetText(&gamesOptions[1], &text);
        safeStrCpy(filterString, text, sizeof(filterString)/sizeof(filterString[0]));
        GameListPrepare(True, n == 2); GameListReplace(0);
        return;
    }

    index = atoi(list[index])-1; // [HGM] filter: read true index from sequence nr of line
    if (cmailMsgLoaded) {
	CmailLoadGame(glc->fp, index + 1, glc->filename, True);
    } else {
	LoadGame(glc->fp, index + 1, glc->filename, True);
    }
}

static int
GameListCreate (char *name)
{
    int new;
    if(new = GenericPopUp(gamesOptions, name, GameListDlg, BoardWindow, NONMODAL, appData.topLevel))
	AddHandler(&gamesOptions[1], GameListDlg, 4),
	AddHandler(&gamesOptions[0], GameListDlg, 5);
    FocusOnWidget(&gamesOptions[0], GameListDlg);
    return new;
}

static int
GameListPrepare (int byPos, int narrow)
{   // [HGM] filter: put in separate routine, to make callable from call-back
    int nstrings;
    ListGame *lg;
    char **st, *line;
    TimeMark t, t2;

    GetTimeMark(&t);
    if(st = glc->strings) while(*st) free(*st++);
    nstrings = ((ListGame *) gameList.tailPred)->number;
    glc->strings = (char **) malloc((nstrings + 1) * sizeof(char *));
    st = glc->strings;
    lg = (ListGame *) gameList.head;
    listLength = wins = losses = draws = 0;
    if(byPos) InitSearch();
    while (nstrings--) {
	int pos = -1;
	if(!narrow || lg->position >= 0) { // only consider already selected positions when narrowing
	  line = GameListLine(lg->number, &lg->gameInfo);
	  if((filterString[0] == NULLCHAR || SearchPattern( line, filterString )) && (!byPos || (pos=GameContainsPosition(glc->fp, lg)) >= 0) ) {
            *st++ = line; // [HGM] filter: make adding line conditional.
	    listLength++;
            if( lg->gameInfo.result == WhiteWins ) wins++; else
            if( lg->gameInfo.result == BlackWins ) losses++; else
            if( lg->gameInfo.result == GameIsDrawn ) draws++;
	    if(!byPos) pos = 0; // indicate selected
	  }
	}
	if(lg->number % 2000 == 0) {
	    char buf[MSG_SIZ];
	    snprintf(buf, MSG_SIZ, _("Scanning through games (%d)"), lg->number);
	    DisplayTitle(buf);
	}
	lg->position = pos;
	lg = (ListGame *) lg->node.succ;
    }
    if(appData.debugMode) { GetTimeMark(&t2);printf("GameListPrepare %ld msec\n", SubtractTimeMarks(&t2,&t)); }
    DisplayTitle("XBoard");
    *st = NULL;
    return listLength;
}

static void
GameListReplace (int page)
{
  // filter: put in separate routine, to make callable from call-back
  char buf[MSG_SIZ], **st=list;
  int i;

  if(page) *st++ = _("previous page"); else if(listLength > 1000) *st++ = "";
  for(i=0; i<1000; i++) if( !(*st++ = glc->strings[page+i]) ) { st--; break; }
  listEnd = st - list;
  if(page + 1000 <= listLength) *st++ = _("next page");
  *st = NULL;

  LoadListBox(&gamesOptions[0], _("no games matched your request"), -1, -1);
  HighlightWithScroll(&gamesOptions[0], listEnd > 1000, listEnd);
  snprintf(buf, MSG_SIZ, _("%s - %d/%d games (%d-%d-%d)"), glc->filename, listLength, ((ListGame *) gameList.tailPred)->number, wins, losses, draws);
  SetDialogTitle(GameListDlg, buf);
}

void
GameListPopUp (FILE *fp, char *filename)
{
    if (glc == NULL) {
	glc = (GameListClosure *) calloc(1, sizeof(GameListClosure));
	glc->x = glc->y = -1;
	glc->filename = NULL;
    }

    GameListPrepare(False, False); // [HGM] filter: code put in separate routine

    glc->fp = fp;

    if (glc->filename != NULL) free(glc->filename);
    glc->filename = StrSave(filename);

    if (!GameListCreate(filename))
	SetIconName(GameListDlg, filename);

    page = 0;
    GameListReplace(0); // [HGM] filter: code put in separate routine, and also called to set title
    MarkMenu("View.GameList", GameListDlg);
}

FILE *
GameFile ()
{
  return glc ? glc->fp : NULL;
}

void
GameListDestroy ()
{
    if (glc == NULL) return;
    PopDown(GameListDlg);
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
ShowGameListProc ()
{
    if (glc == NULL) {
	DisplayError(_("There is no game list"), 0);
	return;
    }
    if (shellUp[GameListDlg]) {
	PopDown(GameListDlg);
	return;
    }
    GenericPopUp(NULL, NULL, GameListDlg, BoardWindow, NONMODAL, appData.topLevel); // first two args ignored when shell exists!
    MarkMenu("View.GameList", GameListDlg);
    GameListHighlight(lastLoadGameNumber);
}

int
GameListClicks (int direction)
{
    int index;

    if (glc == NULL || listLength == 0) return 1;
    if(direction == 100) { FocusOnWidget(&gamesOptions[0], GameListDlg); return 1; }
    index = SelectedListBoxItem(&gamesOptions[0]);

    if (index < 0) return 1;
    if(page && (index == 0 && direction < 1 || direction == -4)) {
        page -= 1000;
        if(page < 0) page = 0; // safety
        GameListReplace(page);
	return 1;
    }
    if(index == 1001 && direction >= 0 || listEnd == 1001 && direction == 4) {
        page += 1000;
        GameListReplace(page);
	return 1;
    }

    if(direction != 0) {
	int doLoad = abs(direction) == 3;
	if(doLoad) direction /= 3;
	index += direction;
	if(direction < -1) index = 0;
	if(direction >  1) index = listEnd-1;
	if(index < 0 || index >= listEnd) return 1;
	HighlightWithScroll(&gamesOptions[0], index, listEnd);
	if(!doLoad) return 1;
    }
    index = atoi(list[index])-1; // [HGM] filter: read true index from sequence nr of line
    if (cmailMsgLoaded) {
	CmailLoadGame(glc->fp, index + 1, glc->filename, True);
    } else {
	LoadGame(glc->fp, index + 1, glc->filename, True);
    }
    return 0;
}

void
SetFilter ()
{
        char *name;
	GetWidgetText(&gamesOptions[1], &name);
        safeStrCpy(filterString, name, sizeof(filterString)/sizeof(filterString[0]));
        GameListPrepare(False, False); GameListReplace(0);
	UnCaret(); // filter text-edit
	FocusOnWidget(&gamesOptions[0], GameListDlg); // listbox
}

void
GameListHighlight (int index)
{
    int i=0; char **st;
    if (!shellUp[GameListDlg]) return;
    st = list;
    while(*st && atoi(*st)<index) st++,i++;
    HighlightWithScroll(&gamesOptions[0], i, listEnd);
}

int
SaveGameListAsText (FILE *f)
{
    ListGame * lg = (ListGame *) gameList.head;
    int nItem;

    if( !glc || ((ListGame *) gameList.tailPred)->number <= 0 ) {
      DisplayError(_("Game list not loaded or empty"), 0);
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
