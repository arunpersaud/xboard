/*
 * wedittags.c -- EditTags window for WinBoard
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

#include <windows.h>   /* required for all Windows applications */
#include <richedit.h>
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

#define _(s) T_(s)

/* Module globals */
static char *editTagsText, **resPtr;
static HWND memo;
BOOL editTagsUp = FALSE;
BOOL canEditTags = FALSE;

/* Imports from winboard.c */
extern HINSTANCE hInst;
extern HWND hwndMain;
extern BoardSize boardSize;

LRESULT CALLBACK
EditTagsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hwndText;
  static int sizeX, sizeY;
  int len, newSizeX, newSizeY;
  char *str;
  RECT rect;
  MINMAXINFO *mmi;
  int err;
  
  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Initialize the dialog items */
    Translate(hDlg, DLG_EditTags);
   hwndText = memo = GetDlgItem(hDlg, OPT_TagsText);
    SendMessage(hwndText, WM_SETFONT, 
      (WPARAM)font[boardSize][EDITTAGS_FONT]->hf, MAKELPARAM(FALSE, 0));
    SetDlgItemText(hDlg, OPT_TagsText, editTagsText);
    EnableWindow(GetDlgItem(hDlg, OPT_TagsCancel), canEditTags);
    EnableWindow(GetDlgItem(hDlg, OPT_EditTags), !canEditTags || bookUp);
    SendMessage(hwndText, EM_SETREADONLY, !canEditTags, 0);
    if (bookUp) {
      SetDlgItemText(hDlg, OPT_EditTags, _("&Play Move"));
      SetWindowText(hDlg, _("Edit Book"));
      SetFocus(hwndText);
    } else
    if (canEditTags) {
      SetWindowText(hDlg, _("Edit Tags"));
      SetFocus(hwndText);
    } else {
      SetWindowText(hDlg, _("Tags"));
      SetFocus(GetDlgItem(hDlg, IDOK));
    }
    if (!editTagsDialog) {
      editTagsDialog = hDlg;
      GetClientRect(hDlg, &rect);
      sizeX = rect.right;
      sizeY = rect.bottom;
      SendDlgItemMessage( hDlg, OPT_TagsText, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS );
      if (wpTags.x != CW_USEDEFAULT && wpTags.y != CW_USEDEFAULT &&
	  wpTags.width != CW_USEDEFAULT && wpTags.height != CW_USEDEFAULT) {
	WINDOWPLACEMENT wp;
	EnsureOnScreen(&wpTags.x, &wpTags.y, 0, 0);
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = 0;
	wp.showCmd = SW_SHOW;
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
	wp.rcNormalPosition.left = wpTags.x;
	wp.rcNormalPosition.right = wpTags.x + wpTags.width;
	wp.rcNormalPosition.top = wpTags.y;
	wp.rcNormalPosition.bottom = wpTags.y + wpTags.height;
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
    case OPT_TagsSave:
      if (canEditTags) {
	char *p, *q;
	/* Read changed options from the dialog box */
	len = GetWindowTextLength(hwndText);
	str = (char *) malloc(len + 1);
	GetWindowText(hwndText, str, len + 1);
	p = q = str;
	while (*q) {
	  if (*q == '\r')
	    q++;
	  else
	    *p++ = *q++;
	}
	*p = NULLCHAR; err = 0;
        if(resPtr) *resPtr = strdup(str); else
	if(bookUp) SaveToBook(str), DisplayBook(currentMove); else
	err = ReplaceTags(str, &gameInfo);
	if (err) DisplayError(_("Error replacing tags."), err);

	free(str);
      }
      if(LOWORD(wParam) == IDOK) TagsPopDown();
      return TRUE;
      
    case IDCANCEL:
    case OPT_TagsCancel:
      TagsPopDown();
      return TRUE;
      
    case OPT_EditTags:
      if(bookUp) addToBookFlag = !addToBookFlag; else
      EditTagsEvent();
      return TRUE;

    default:
      break;
    }
    break;

  case WM_NOTIFY: // [HGM] vari: cloned from whistory.c
        if( wParam == OPT_TagsText ) {
            MSGFILTER * lpMF = (MSGFILTER *) lParam;

            if( lpMF->msg == WM_RBUTTONDOWN ) {
                POINTL pt;
                LRESULT index;

                pt.x = LOWORD( lpMF->lParam );
                pt.y = HIWORD( lpMF->lParam );

                index = SendDlgItemMessage( hDlg, OPT_TagsText, EM_CHARFROMPOS, 0, (LPARAM) &pt );

		hwndText = GetDlgItem(hDlg, OPT_TagsText); // cloned from above
		len = GetWindowTextLength(hwndText);
		str = (char *) malloc(len + 1);
		GetWindowText(hwndText, str, len + 1);
		if(bookUp) PlayBookMove(str, index);
		free(str);

                /* Zap the message for good: apparently, returning non-zero is not enough */
                lpMF->msg = WM_USER;

                return TRUE;
            }
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

VOID AddBookMove(char *text)
{
    SendMessage( memo, EM_SETSEL, 999999, 999999 ); // [HGM] multivar: choose insertion point
    SendMessage( memo, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) text );
}

VOID TagsPopDown(void)
{
  if (editTagsDialog) ShowWindow(editTagsDialog, SW_HIDE);
  CheckMenuItem(GetMenu(hwndMain), IDM_Tags, MF_UNCHECKED);
  editTagsUp = bookUp = addToBookFlag = FALSE;
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
  
  CheckMenuItem(GetMenu(hwndMain), IDM_Tags, MF_CHECKED);
  if (editTagsDialog) {
    SendMessage(editTagsDialog, WM_INITDIALOG, 0, 0);
    ShowWindow(editTagsDialog, SW_SHOW);
    if(bookUp) SetFocus(hwndMain);
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

VOID EditTagsPopUp(char *tags, char **dest)
{
  resPtr = dest;
  EitherTagsPopUp(tags, "", TRUE);
}

VOID EditTagsProc()
{
  if (editTagsUp && !bookUp) {
    TagsPopDown();
  } else {
    EditTagsEvent();
  }
}
