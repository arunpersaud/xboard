/*
 * wgamelist.c -- Game list window for WinBoard
 * $Id$
 *
 * Copyright 1995 Free Software Foundation, Inc.
 *
 * ------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ------------------------------------------------------------------------
 */

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
#include "winboard.h"
#include "frontend.h"
#include "backend.h"

/* Module globals */
HWND gameListDialog = NULL;
BOOLEAN gameListUp = FALSE;
FILE* gameFile;
char* gameFileName = NULL;
int gameListX, gameListY, gameListW, gameListH;

/* Imports from winboard.c */
extern HINSTANCE hInst;
extern HWND hwndMain;


LRESULT CALLBACK
GameListDialog(HWND hDlg, UINT message,	WPARAM wParam, LPARAM lParam)
{
  static HANDLE hwndText;
  int nItem;
  ListGame *lg;
  RECT rect;
  static int sizeX, sizeY;
  int newSizeX, newSizeY, flags;
  MINMAXINFO *mmi;

  switch (message) {
  case WM_INITDIALOG: 
    if (gameListDialog) {
      SendDlgItemMessage(hDlg, OPT_GameListText, LB_RESETCONTENT, 0, 0);
    }
    /* Initialize the dialog items */
    hwndText = GetDlgItem(hDlg, OPT_TagsText);
    lg = (ListGame *) gameList.head;
    for (nItem = 0; nItem < ((ListGame *) gameList.tailPred)->number; nItem++){
      char *st = GameListLine(lg->number, &lg->gameInfo);
      SendDlgItemMessage(hDlg, OPT_GameListText, LB_ADDSTRING, 0, (LPARAM) st);
      free(st);
      lg = (ListGame *) lg->node.succ;
    }
    SendDlgItemMessage(hDlg, OPT_GameListText, LB_SETCURSEL, 0, 0);
    /* Size and position the dialog */
    if (!gameListDialog) {
      gameListDialog = hDlg;
      flags = SWP_NOZORDER;
      GetClientRect(hDlg, &rect);
      sizeX = rect.right;
      sizeY = rect.bottom;
      if (gameListX != CW_USEDEFAULT && gameListY != CW_USEDEFAULT &&
	  gameListW != CW_USEDEFAULT && gameListH != CW_USEDEFAULT) {
	WINDOWPLACEMENT wp;
	EnsureOnScreen(&gameListX, &gameListY);
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = 0;
	wp.showCmd = SW_SHOW;
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
	wp.rcNormalPosition.left = gameListX;
	wp.rcNormalPosition.right = gameListX + gameListW;
	wp.rcNormalPosition.top = gameListY;
	wp.rcNormalPosition.bottom = gameListY + gameListH;
	SetWindowPlacement(hDlg, &wp);

	GetClientRect(hDlg, &rect);
	newSizeX = rect.right;
	newSizeY = rect.bottom;
        ResizeEditPlusButtons(hDlg, hwndText, sizeX, sizeY,
			      newSizeX, newSizeY);
	sizeX = newSizeX;
	sizeY = newSizeY;
      }
    }
    return FALSE;
    
  case WM_SIZE:
    newSizeX = LOWORD(lParam);
    newSizeY = HIWORD(lParam);
    ResizeEditPlusButtons(hDlg, GetDlgItem(hDlg, OPT_GameListText),
      sizeX, sizeY, newSizeX, newSizeY);
    sizeX = newSizeX;
    sizeY = newSizeY;
    break;

  case WM_GETMINMAXINFO:
    /* Prevent resizing window too small */
    mmi = (MINMAXINFO *) lParam;
    mmi->ptMinTrackSize.x = 100;
    mmi->ptMinTrackSize.y = 100;
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
    case OPT_GameListLoad:
      nItem = SendDlgItemMessage(hDlg, OPT_GameListText, LB_GETCURSEL, 0, 0);
      if (nItem < 0) {
	/* is this possible? */
	DisplayError("No game selected", 0);
	return TRUE;
      }
      break; /* load the game*/
      
    case OPT_GameListNext:
      nItem = SendDlgItemMessage(hDlg, OPT_GameListText, LB_GETCURSEL, 0, 0);
      nItem++;
      if (nItem >= ((ListGame *) gameList.tailPred)->number) {
	DisplayError("Can't go forward any further", 0);
	return TRUE;
      }
      SendDlgItemMessage(hDlg, OPT_GameListText, LB_SETCURSEL, nItem, 0);
      break; /* load the game*/
      
    case OPT_GameListPrev:
      nItem = SendDlgItemMessage(hDlg, OPT_GameListText, LB_GETCURSEL, 0, 0);
      nItem--;
      if (nItem < 0) {
	DisplayError("Can't back up any further", 0);
      }
      SendDlgItemMessage(hDlg, OPT_GameListText, LB_SETCURSEL, nItem, 0);
      break; /* load the game*/
      
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
    if (cmailMsgLoaded) {
      CmailLoadGame(gameFile, nItem + 1, gameFileName, TRUE);
    } else {
      LoadGame(gameFile, nItem + 1, gameFileName, TRUE);
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
  } else {
    lpProc = MakeProcInstance((FARPROC)GameListDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_GameList),
      hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
  gameListUp = TRUE;
}

VOID GameListPopDown(void)
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowGameList, MF_UNCHECKED);
  if (gameListDialog) ShowWindow(gameListDialog, SW_HIDE);
  gameListUp = FALSE;
}


VOID GameListHighlight(int index)
{
  if (gameListDialog == NULL) return;
  SendDlgItemMessage(gameListDialog, OPT_GameListText, 
    LB_SETCURSEL, index - 1, 0);
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
    GameListPopDown();
  } else {
    if (gameFileName) {
      GameListPopUp(gameFile, gameFileName);
    } else {
      DisplayError("No game list", 0);
    }
  }
}
