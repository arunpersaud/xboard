/*
 * wedittags.c -- EditTags window for WinBoard
 *
 * Copyright 1995,2009 Free Software Foundation, Inc.
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

#include <windows.h>   /* required for all Windows applications */
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
#include "wedittags.h"

/* Module globals */
static char *editTagsText;
HWND editTagsDialog = NULL;
BOOL editTagsUp = FALSE;
BOOL canEditTags = FALSE;
int editTagsX, editTagsY, editTagsW, editTagsH;

/* Imports from winboard.c */
extern HINSTANCE hInst;
extern HWND hwndMain;
extern BoardSize boardSize;

LRESULT CALLBACK
EditTagsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hwndText;
  static int sizeX, sizeY;
  int len, newSizeX, newSizeY, flags;
  char *str;
  RECT rect;
  MINMAXINFO *mmi;
  int err;
  
  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Initialize the dialog items */
    hwndText = GetDlgItem(hDlg, OPT_TagsText);
    SendMessage(hwndText, WM_SETFONT, 
      (WPARAM)font[boardSize][EDITTAGS_FONT]->hf, MAKELPARAM(FALSE, 0));
    SetDlgItemText(hDlg, OPT_TagsText, editTagsText);
    EnableWindow(GetDlgItem(hDlg, OPT_TagsCancel), canEditTags);
    EnableWindow(GetDlgItem(hDlg, OPT_EditTags), !canEditTags);
    SendMessage(hwndText, EM_SETREADONLY, !canEditTags, 0);
    if (canEditTags) {
      SetWindowText(hDlg, "Edit Tags");
      SetFocus(hwndText);
    } else {
      SetWindowText(hDlg, "Tags");
      SetFocus(GetDlgItem(hDlg, IDOK));
    }
    if (!editTagsDialog) {
      editTagsDialog = hDlg;
      flags = SWP_NOZORDER;
      GetClientRect(hDlg, &rect);
      sizeX = rect.right;
      sizeY = rect.bottom;
      if (editTagsX != CW_USEDEFAULT && editTagsY != CW_USEDEFAULT &&
	  editTagsW != CW_USEDEFAULT && editTagsH != CW_USEDEFAULT) {
	WINDOWPLACEMENT wp;
	EnsureOnScreen(&editTagsX, &editTagsY, 0, 0);
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = 0;
	wp.showCmd = SW_SHOW;
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
	wp.rcNormalPosition.left = editTagsX;
	wp.rcNormalPosition.right = editTagsX + editTagsW;
	wp.rcNormalPosition.top = editTagsY;
	wp.rcNormalPosition.bottom = editTagsY + editTagsH;
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
    
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (canEditTags) {
	char *p, *q;
	/* Read changed options from the dialog box */
	len = GetWindowTextLength(hwndText);
	str = (char *) malloc(len + 1);
	GetWindowText(hwndText, str, len + 1);
	p = q = str;
	while (*q) {
	  if (*q == '\r'|| *q == '\n')
	    q++;
	  else
	    *p++ = *q++;
	}
	*p = NULLCHAR;
	err = ReplaceTags(str, &gameInfo);
	if (err) DisplayError("Error replacing tags.", err);

	free(str);
      }
      TagsPopDown();
      return TRUE;
      
    case IDCANCEL:
    case OPT_TagsCancel:
      TagsPopDown();
      return TRUE;
      
    case OPT_EditTags:
      EditTagsEvent();
      return TRUE;

    default:
      break;
    }
    break;

  case WM_SIZE:
    newSizeX = LOWORD(lParam);
    newSizeY = HIWORD(lParam);
    ResizeEditPlusButtons(hDlg, hwndText, sizeX, sizeY, newSizeX, newSizeY);
    sizeX = newSizeX;
    sizeY = newSizeY;
    break;

  case WM_GETMINMAXINFO:
    /* Prevent resizing window too small */
    mmi = (MINMAXINFO *) lParam;
    mmi->ptMinTrackSize.x = 100;
    mmi->ptMinTrackSize.y = 100;
    break;
  }
  return FALSE;
}

VOID TagsPopDown(void)
{
  if (editTagsDialog) ShowWindow(editTagsDialog, SW_HIDE);
  CheckMenuItem(GetMenu(hwndMain), IDM_EditTags, MF_UNCHECKED);
  editTagsUp = FALSE;
}


VOID EitherTagsPopUp(char *tags, char *msg, BOOLEAN edit)
{
  FARPROC lpProc;
  char *p, *q;
  
  if (msg == NULL) msg = "";
  p = (char *) malloc(2 * (strlen(tags) + strlen(msg)) + 2);
  q = p;
  while (*tags) {
    if (*tags == '\n') *q++ = '\r';
    *q++ = *tags++;
  }
  if (*msg != NULLCHAR) {
    *q++ = '\r';
    *q++ = '\n';
    while (*msg) {
      if (*msg == '\n') *q++ = '\r';
      *q++ = *msg++;
    }
  }
  *q = NULLCHAR;
  if (editTagsText != NULL) free(editTagsText);
  editTagsText = p;
  canEditTags = edit;
  
  CheckMenuItem(GetMenu(hwndMain), IDM_EditTags, MF_CHECKED);
  if (editTagsDialog) {
    SendMessage(editTagsDialog, WM_INITDIALOG, 0, 0);
    ShowWindow(editTagsDialog, SW_SHOW);
  } else {
    lpProc = MakeProcInstance((FARPROC)EditTagsDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_EditTags),
      hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
  editTagsUp = TRUE;
}

VOID TagsPopUp(char *tags, char *msg)
{
  HWND hwnd = GetActiveWindow();
  EitherTagsPopUp(tags, msg, FALSE);
  SetActiveWindow(hwnd);
}

VOID EditTagsPopUp(char *tags)
{
  EitherTagsPopUp(tags, "", TRUE);
}

VOID EditTagsProc()
{
  if (editTagsUp) {
    TagsPopDown();
  } else {
    EditTagsEvent();
  }
}
