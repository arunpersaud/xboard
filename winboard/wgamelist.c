/*
 * wgamelist.c -- Game list window for WinBoard
 *
 * Copyright 1995, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
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

#include <windows.h> /* required for all Windows applications */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <math.h>
#include <commdlg.h>
#include <dlgs.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "winboard.h"

#include "wsnap.h"

#define _(s) T_(s)

/* Module globals */
static BOOLEAN gameListUp = FALSE;
static FILE* gameFile;
static char* gameFileName = NULL;

struct GameListStats
{
    int white_wins;
    int black_wins;
    int drawn;
    int unfinished;
};

/* [AS] Setup the game list according to the specified filter */
static int GameListToListBox( HWND hDlg, BOOL boReset, char * pszFilter, struct GameListStats * stats, BOOL byPos, BOOL narrow )
{
    ListGame * lg = (ListGame *) gameList.head;
    int nItem;
    char buf[MSG_SIZ];
    BOOL hasFilter = FALSE;
    int count = 0;
    struct GameListStats dummy;

    /* Initialize stats (use a dummy variable if caller not interested in them) */
    if( stats == NULL ) {
        stats = &dummy;
    }

    stats->white_wins = 0;
    stats->black_wins = 0;
    stats->drawn = 0;
    stats->unfinished = 0;

    if( boReset ) {
        SendDlgItemMessage(hDlg, OPT_GameListText, LB_RESETCONTENT, 0, 0);
    }

    if( pszFilter != NULL ) {
        if( strlen( pszFilter ) > 0 ) {
            hasFilter = TRUE;
        }
    }

    if(byPos) InitSearch();

    for (nItem = 0; nItem < ((ListGame *) gameList.tailPred)->number; nItem++){
        char * st = NULL;
        BOOL skip = FALSE;
	int pos = -1;

        if(nItem % 2000 == 0) {
          snprintf(buf, MSG_SIZ, _("Scanning through games (%d)"), nItem);
          SetWindowText(hwndMain, buf);
        }

      if(!narrow || lg->position >= 0) {
        if( hasFilter ) {
            st = GameListLine(lg->number, &lg->gameInfo);
	    if( !SearchPattern( st, pszFilter) ) skip = TRUE;
        }

        if( !skip && byPos) {
            if( (pos = GameContainsPosition(gameFile, lg)) < 0) skip = TRUE;
        }

        if( ! skip ) {
            if(!st) st = GameListLine(lg->number, &lg->gameInfo);
            SendDlgItemMessage(hDlg, OPT_GameListText, LB_ADDSTRING, 0, (LPARAM) st);
            count++;

            /* Update stats */
            if( lg->gameInfo.result == WhiteWins )
                stats->white_wins++;
            else if( lg->gameInfo.result == BlackWins )
                stats->black_wins++;
            else if( lg->gameInfo.result == GameIsDrawn )
                stats->drawn++;
            else
                stats->unfinished++;
	    if(!byPos) pos = 0;
        }
      }

	lg->position = pos;

        if(st) free(st);
        lg = (ListGame *) lg->node.succ;
    }

    SendDlgItemMessage(hDlg, OPT_GameListText, LB_SETCURSEL, 0, 0);
    SetWindowText(hwndMain, "WinBoard");

    return count;
}

/* [AS] Show number of visible (filtered) games and total on window caption */
static int GameListUpdateTitle( HWND hDlg, char * pszTitle, int item_count, int item_total, struct GameListStats * stats )
{
    char buf[256];

    snprintf( buf, sizeof(buf)/sizeof(buf[0]),_("%s - %d/%d games"), pszTitle, item_count, item_total );

    if( stats != 0 ) {
        sprintf( buf+strlen(buf), " (%d-%d-%d)", stats->white_wins, stats->black_wins, stats->drawn );
    }

    SetWindowText( hDlg, buf );

    return 0;
}

#define MAX_FILTER_LENGTH   128

LRESULT CALLBACK
GameListDialog(HWND hDlg, UINT message,	WPARAM wParam, LPARAM lParam)
{
  static char szDlgTitle[64];
  static HANDLE hwndText;
  int nItem;
  RECT rect;
  static int sizeX, sizeY;
  int newSizeX, newSizeY, flags;
  MINMAXINFO *mmi;
  static BOOL filterHasFocus = FALSE;
  int count;
  struct GameListStats stats;
  static SnapData sd;

  switch (message) {
  case WM_INITDIALOG:
    Translate(hDlg, DLG_GameList);
    GetWindowText( hDlg, szDlgTitle, sizeof(szDlgTitle) );
    szDlgTitle[ sizeof(szDlgTitle)-1 ] = '\0';

    if (gameListDialog) {
      SendDlgItemMessage(hDlg, OPT_GameListText, LB_RESETCONTENT, 0, 0);
    }

    /* Initialize the dialog items */
    hwndText = GetDlgItem(hDlg, OPT_TagsText);

    /* Set font */
    SendDlgItemMessage( hDlg, OPT_GameListText, WM_SETFONT, (WPARAM)font[boardSize][GAMELIST_FONT]->hf, MAKELPARAM(TRUE, 0 ));

    count = GameListToListBox( hDlg, gameListDialog ? TRUE : FALSE, NULL, &stats, FALSE, FALSE );

    SendDlgItemMessage( hDlg, IDC_GameListFilter, WM_SETTEXT, 0, (LPARAM) "" );
    SendDlgItemMessage( hDlg, IDC_GameListFilter, EM_SETLIMITTEXT, MAX_FILTER_LENGTH, 0 );

    filterHasFocus = FALSE;

    /* Size and position the dialog */
    if (!gameListDialog) {
      gameListDialog = hDlg;
      flags = SWP_NOZORDER;
      GetClientRect(hDlg, &rect);
      sizeX = rect.right;
      sizeY = rect.bottom;
      if (wpGameList.x != CW_USEDEFAULT && wpGameList.y != CW_USEDEFAULT &&
	  wpGameList.width != CW_USEDEFAULT && wpGameList.height != CW_USEDEFAULT) {
	WINDOWPLACEMENT wp;
	EnsureOnScreen(&wpGameList.x, &wpGameList.y, 0, 0);
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = 0;
	wp.showCmd = SW_SHOW;
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
	wp.rcNormalPosition.left = wpGameList.x;
	wp.rcNormalPosition.right = wpGameList.x + wpGameList.width;
	wp.rcNormalPosition.top = wpGameList.y;
	wp.rcNormalPosition.bottom = wpGameList.y + wpGameList.height;
	SetWindowPlacement(hDlg, &wp);

	GetClientRect(hDlg, &rect);
	newSizeX = rect.right;
	newSizeY = rect.bottom;
        ResizeEditPlusButtons(hDlg, hwndText, sizeX, sizeY,
			      newSizeX, newSizeY);
	sizeX = newSizeX;
	sizeY = newSizeY;
      } else
        GetActualPlacement( gameListDialog, &wpGameList );

    }
      GameListUpdateTitle( hDlg, _("Game List"), count, ((ListGame *) gameList.tailPred)->number, &stats ); // [HGM] always update title
    GameListHighlight(lastLoadGameNumber);
    return FALSE;

  case WM_SIZE:
    newSizeX = LOWORD(lParam);
    newSizeY = HIWORD(lParam);
    ResizeEditPlusButtons(hDlg, GetDlgItem(hDlg, OPT_GameListText),
      sizeX, sizeY, newSizeX, newSizeY);
    sizeX = newSizeX;
    sizeY = newSizeY;
    break;

  case WM_ENTERSIZEMOVE:
    return OnEnterSizeMove( &sd, hDlg, wParam, lParam );

  case WM_SIZING:
    return OnSizing( &sd, hDlg, wParam, lParam );

  case WM_MOVING:
    return OnMoving( &sd, hDlg, wParam, lParam );

  case WM_EXITSIZEMOVE:
    return OnExitSizeMove( &sd, hDlg, wParam, lParam );

  case WM_GETMINMAXINFO:
    /* Prevent resizing window too small */
    mmi = (MINMAXINFO *) lParam;
    mmi->ptMinTrackSize.x = 100;
    mmi->ptMinTrackSize.y = 100;
    break;

  case WM_COMMAND:
      /*
        [AS]
        If <Enter> is pressed while editing the filter, it's better to apply
        the filter rather than selecting the current game.
      */
      if( LOWORD(wParam) == IDC_GameListFilter ) {
          switch( HIWORD(wParam) ) {
          case EN_SETFOCUS:
              filterHasFocus = TRUE;
              break;
          case EN_KILLFOCUS:
              filterHasFocus = FALSE;
              break;
          }
      }

      if( filterHasFocus && (LOWORD(wParam) == IDOK) ) {
          wParam = IDC_GameListDoFilter;
      }
      /* [AS] End command replacement */

    switch (LOWORD(wParam)) {
    case OPT_GameListLoad:
      LoadOptionsPopup(hDlg);
      return TRUE;
    case IDOK:
      nItem = SendDlgItemMessage(hDlg, OPT_GameListText, LB_GETCURSEL, 0, 0);
      if (nItem < 0) {
	/* is this possible? */
	DisplayError(_("No game selected"), 0);
	return TRUE;
      }
      break; /* load the game*/

    case OPT_GameListNext:
      nItem = SendDlgItemMessage(hDlg, OPT_GameListText, LB_GETCURSEL, 0, 0);
      nItem++;
      if (nItem >= ((ListGame *) gameList.tailPred)->number) {
        /* [AS] Removed error message */
	/* DisplayError(_("Can't go forward any further"), 0); */
	return TRUE;
      }
      SendDlgItemMessage(hDlg, OPT_GameListText, LB_SETCURSEL, nItem, 0);
      break; /* load the game*/

    case OPT_GameListPrev:
#if 0
      nItem = SendDlgItemMessage(hDlg, OPT_GameListText, LB_GETCURSEL, 0, 0);
      nItem--;
      if (nItem < 0) {
        /* [AS] Removed error message, added return */
	/* DisplayError(_("Can't back up any further"), 0); */
        return TRUE;
      }
      SendDlgItemMessage(hDlg, OPT_GameListText, LB_SETCURSEL, nItem, 0);
      break; /* load the game*/
#endif
    /* [AS] */
    case OPT_GameListFind:
    case IDC_GameListDoFilter:
        {
            char filter[MAX_FILTER_LENGTH+1];

            if( GetDlgItemText( hDlg, IDC_GameListFilter, filter, sizeof(filter) ) >= 0 ) {
                filter[ sizeof(filter)-1 ] = '\0';
                count = GameListToListBox( hDlg, TRUE, filter, &stats, LOWORD(wParam)!=IDC_GameListDoFilter, LOWORD(wParam)==OPT_GameListNarrow );
                GameListUpdateTitle( hDlg, _("Game List"), count, ((ListGame *) gameList.tailPred)->number, &stats );
            }
        }
        return FALSE;
        break;

    case IDCANCEL:
    case OPT_GameListClose:
      GameListPopDown();
      return TRUE;

    case OPT_GameListText:
      switch (HIWORD(wParam)) {
      case LBN_DBLCLK:
	nItem = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
	break; /* load the game*/

      default:
	return FALSE;
      }
      break;

    default:
      return FALSE;
    }

    /* Load the game */
    {
        /* [AS] Get index from the item itself, because filtering makes original order unuseable. */
        int index = SendDlgItemMessage(hDlg, OPT_GameListText, LB_GETCURSEL, 0, 0);
        char * text;
        LRESULT res;

        if( index < 0 ) {
            return TRUE;
        }

        res = SendDlgItemMessage( hDlg, OPT_GameListText, LB_GETTEXTLEN, index, 0 );

        if( res == LB_ERR ) {
            return TRUE;
        }

        text = (char *) malloc( res+1 );

        res = SendDlgItemMessage( hDlg, OPT_GameListText, LB_GETTEXT, index, (LPARAM)text );

        index = atoi( text );

        nItem = index - 1;

        free( text );
        /* [AS] End: nItem has been "patched" now! */

        if (cmailMsgLoaded) {
            CmailLoadGame(gameFile, nItem + 1, gameFileName, TRUE);
        }
        else {
            LoadGame(gameFile, nItem + 1, gameFileName, TRUE);
	    SetFocus(hwndMain); // [HGM] automatic focus switch
        }
    }

    return TRUE;

  default:
    break;
  }
  return FALSE;
}


VOID GameListPopUp(FILE *fp, char *filename)
{
  FARPROC lpProc;

  gameFile = fp;
  if (gameFileName != filename) {
    if (gameFileName) free(gameFileName);
    gameFileName = StrSave(filename);
  }
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowGameList, MF_CHECKED);
  if (gameListDialog) {
    SendMessage(gameListDialog, WM_INITDIALOG, 0, 0);
    if (!gameListUp) ShowWindow(gameListDialog, SW_SHOW);
    else SetFocus(gameListDialog);
  } else {
    lpProc = MakeProcInstance((FARPROC)GameListDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_GameList),
      hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
  gameListUp = TRUE;
}

FILE *GameFile()
{
  return gameFile;
}

VOID GameListPopDown(void)
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowGameList, MF_UNCHECKED);
  if (gameListDialog) ShowWindow(gameListDialog, SW_HIDE);
  gameListUp = FALSE;
}


VOID GameListHighlight(int index)
{
  char buf[MSG_SIZ];
  int i, res = 0;
  if (gameListDialog == NULL) return;
  for(i=0; res != LB_ERR; i++) {
        res = SendDlgItemMessage( gameListDialog, OPT_GameListText, LB_GETTEXT, i, (LPARAM)buf );
        if(index <= atoi( buf )) break;
  }
  SendDlgItemMessage(gameListDialog, OPT_GameListText,
    LB_SETCURSEL, i, 0);
}


VOID GameListDestroy()
{
  GameListPopDown();
  if (gameFileName) {
    free(gameFileName);
    gameFileName = NULL;
  }
}

VOID ShowGameListProc()
{
  if (gameListUp) {
    if(gameListDialog) SetFocus(gameListDialog);
//    GameListPopDown();
  } else {
    if (gameFileName) {
      GameListPopUp(gameFile, gameFileName);
    } else {
      DisplayError(_("No game list"), 0);
    }
  }
}

HGLOBAL ExportGameListAsText()
{
    HGLOBAL result = NULL;
    LPVOID lpMem = NULL;
    ListGame * lg = (ListGame *) gameList.head;
    int nItem;
    DWORD dwLen = 0;

    if( ! gameFileName || ((ListGame *) gameList.tailPred)->number <= 0 ) {
        DisplayError(_(_("Game list not loaded or empty")), 0);
        return NULL;
    }

    /* Get list size */
    for (nItem = 0; nItem < ((ListGame *) gameList.tailPred)->number; nItem++){
        char * st = GameListLineFull(lg->number, &lg->gameInfo);

        dwLen += strlen(st) + 2; /* Add extra characters for "\r\n" */

        free(st);
        lg = (ListGame *) lg->node.succ;
    }

    /* Allocate memory for the list */
    result = GlobalAlloc(GHND, dwLen+1 );

    if( result != NULL ) {
        lpMem = GlobalLock(result);
    }

    /* Copy the list into the global memory block */
    if( lpMem != NULL ) {
        char * dst = (char *) lpMem;
        size_t len;

        lg = (ListGame *) gameList.head;

        for (nItem = 0; nItem < ((ListGame *) gameList.tailPred)->number; nItem++){
            char * st = GameListLineFull(lg->number, &lg->gameInfo);

            len = sprintf( dst, "%s\r\n", st );
            dst += len;

            free(st);
            lg = (ListGame *) lg->node.succ;
        }

        GlobalUnlock( result );
    }

    return result;
}
