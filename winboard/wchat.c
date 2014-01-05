/*
 * Chat window (PV)
 *
 * Author: H.G.Muller (August 2009)
 *
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <commdlg.h>
#include <dlgs.h>
#include <Windowsx.h>

#include "common.h"
#include "frontend.h"
#include "winboard.h"
#include "backend.h"

#include "wsnap.h"

int chatCount;
static int onTop;
extern char chatPartner[MAX_CHAT][MSG_SIZ];
HANDLE chatHandle[MAX_CHAT];
static WNDPROC chatInputWindowProc;

void SendToICS P((char *s));
void ChatPopUp P((char *s));
void ChatPopDown();

/* Imports from backend.c */
extern int opponentKibitzes;

/* Imports from winboard.c */
VOID SaveInHistory(char *cmd);
char *PrevInHistory(char *cmd);
char *NextInHistory();
extern HWND ChatDialog;

extern HINSTANCE hInst;
extern HWND hwndConsole;
extern char ics_handle[];

extern WindowPlacement wpChat[MAX_CHAT];
extern WindowPlacement wpConsole;

extern BoardSize boardSize;

/* Module variables */
#define H_MARGIN            5
#define V_MARGIN            5

// front end, although we might make GetWindowRect front end instead
static int GetControlWidth( HWND hDlg, int id )
{
    RECT rc;

    GetWindowRect( GetDlgItem( hDlg, id ), &rc );

    return rc.right - rc.left;
}

// front end?
static int GetControlHeight( HWND hDlg, int id )
{
    RECT rc;

    GetWindowRect( GetDlgItem( hDlg, id ), &rc );

    return rc.bottom - rc.top;
}

static void SetControlPos( HWND hDlg, int id, int x, int y, int width, int height )
{
    HWND hControl = GetDlgItem( hDlg, id );

    SetWindowPos( hControl, HWND_TOP, x, y, width, height, SWP_NOZORDER );
}

// Also here some of the size calculations should go to the back end, and their actual application to a front-end routine
static void ResizeWindowControls( HWND hDlg )
{
    RECT rc;
    int clientWidth;
    int clientHeight;
    int maxControlWidth;
    int buttonWidth, buttonHeight;

    /* Initialize variables */
    GetClientRect( hDlg, &rc );

    clientWidth = rc.right - rc.left;
    clientHeight = rc.bottom - rc.top;

    maxControlWidth = clientWidth - 2*H_MARGIN;
    buttonWidth  = GetControlWidth(hDlg, IDC_Send);
    buttonHeight = GetControlHeight(hDlg, IDC_Send);

    /* Resize controls */
    SetControlPos( hDlg, IDC_Clear, maxControlWidth+H_MARGIN-2*buttonWidth-5, V_MARGIN, buttonWidth, buttonHeight );
    SetControlPos( hDlg, IDC_Send, maxControlWidth+H_MARGIN-buttonWidth, V_MARGIN, buttonWidth, buttonHeight );
    SetControlPos( hDlg, IDC_ChatMemo, H_MARGIN, 2*V_MARGIN+buttonHeight, maxControlWidth, clientHeight-3*V_MARGIN-2*buttonHeight );
    SetControlPos( hDlg, OPT_ChatInput, H_MARGIN, clientHeight-V_MARGIN-buttonHeight, maxControlWidth, buttonHeight );

//    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo1), NULL, FALSE );
//    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo2), NULL, FALSE );
}

// front end. Actual printing of PV lines into the output field
static void InsertIntoMemo( HANDLE hDlg, char * text )
{
    HANDLE hMemo = GetDlgItem(hDlg, IDC_ChatMemo);

    SendMessage( hMemo, EM_SETSEL, 1000000, 1000000 );

    SendMessage( hMemo, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) text );
    SendMessage( hMemo, EM_SCROLLCARET, 0, 0);
}

LRESULT CALLBACK
InterceptArrowKeys(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MSG_SIZ];
  char *p;
  CHARRANGE sel;

  switch (message) {
  case WM_KEYDOWN: // cloned from ConsoleInputSubClass()
    switch (wParam) {
    case VK_UP:
      GetWindowText(hwnd, buf, MSG_SIZ);
      p = PrevInHistory(buf);
      if (p != NULL) {
	SetWindowText(hwnd, p);
	sel.cpMin = 999999;
	sel.cpMax = 999999;
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
        return 0;
      }
      break;
    case VK_DOWN:
      p = NextInHistory();
      if (p != NULL) {
	SetWindowText(hwnd, p);
	sel.cpMin = 999999;
	sel.cpMax = 999999;
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
        return 0;
      }
      break;
    }
  }
  return (*chatInputWindowProc)(hwnd, message, wParam, lParam);
}

// This seems pure front end
LRESULT CALLBACK ChatProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;
    char buf[MSG_SIZ], mess[MSG_SIZ];
    int partner = -1, i, x, y;
    static BOOL filterHasFocus[MAX_CHAT];
    WORD wMask;
    HWND hMemo;

    for(i=0; i<MAX_CHAT; i++) if(hDlg == chatHandle[i]) { partner = i; break; }

    switch (message) {
    case WM_INITDIALOG:
        Translate(hDlg, DLG_Chat);
	if(partner<0) {
		for(i=0; i<MAX_CHAT; i++) if(chatHandle[i] == NULL) { partner = i; break; }
	        chatHandle[partner] = hDlg;
		snprintf(buf, MSG_SIZ, T_("Chat Window %s"), ics_handle[0] ? ics_handle : first.tidy);
		SetWindowText(hDlg, buf);
        }
	for(i=0; i<MAX_CHAT; i++) if(chatHandle[i]) {
	    if(i == partner) continue;
	    // set our button in other open chats
	    SetDlgItemText(chatHandle[i], IDC_Focus1+partner-(i<partner), chatPartner[partner]);
	    EnableWindow( GetDlgItem(chatHandle[i], IDC_Focus1+partner-(i<partner)), 1 );
	    // and buttons for other chats in ours
	    SetDlgItemText(hDlg, IDC_Focus1+i-(i>partner), chatPartner[i]);
	} else EnableWindow( GetDlgItem(hDlg, IDC_Focus1+i-(i>partner)), 0 );
	for(i=0; i<MAX_CHAT-1; i++) { Button_SetStyle(GetDlgItem(hDlg, IDC_Focus1+i), BS_PUSHBUTTON|BS_LEFT, TRUE); }
        x = wpConsole.x; y = wpConsole.y; EnsureOnScreen(&x, &y, 0, 0);
        SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	SendMessage( GetDlgItem(hDlg, IDC_ChatPartner), // [HGM] clickbox: initialize with requested handle
			WM_SETTEXT, 0, (LPARAM) chatPartner[partner] );
	filterHasFocus[partner] = TRUE;
	onTop = partner; // a newly opened box becomes top one
	if(chatPartner[partner][0]) {
	    filterHasFocus[partner] = FALSE;
	    SetFocus( GetDlgItem(hDlg, OPT_ChatInput) );
	}
	hMemo = GetDlgItem(hDlg, IDC_ChatMemo);
	wMask = (WORD) SendMessage(hMemo, EM_GETEVENTMASK, 0, 0L);
	SendMessage(hMemo, EM_SETEVENTMASK, 0, wMask | ENM_LINK);
	SendMessage(hMemo, EM_AUTOURLDETECT, TRUE, 0L);
	chatInputWindowProc = (WNDPROC) // cloned from ConsoleWndProc(). Assume they all share same proc.
	      SetWindowLongPtr(GetDlgItem(hDlg, OPT_ChatInput), GWLP_WNDPROC, (LONG_PTR) InterceptArrowKeys);
        return FALSE;

    case WM_NOTIFY:
      if (((NMHDR*)lParam)->code == EN_LINK)
      {
	ENLINK *pLink = (ENLINK*)lParam;
	if (pLink->msg == WM_LBUTTONUP)
	{
	  TEXTRANGE tr;

	  tr.chrg = pLink->chrg;
	  tr.lpstrText = malloc(1+tr.chrg.cpMax-tr.chrg.cpMin);
	  SendMessage( GetDlgItem(hDlg, IDC_ChatMemo), EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	  ShellExecute(NULL, "open", tr.lpstrText, NULL, NULL, SW_SHOW);
	  free(tr.lpstrText);
	}
      }
    break;

    case WM_COMMAND:
      /*
        [AS]
        If <Enter> is pressed while editing the filter, it's better to apply
        the filter rather than selecting the current game.
      */
      if( LOWORD(wParam) == IDC_ChatPartner ) {
          switch( HIWORD(wParam) ) {
          case EN_SETFOCUS:
              filterHasFocus[partner] = TRUE;
              break;
          case EN_KILLFOCUS:
              filterHasFocus[partner] = FALSE;
              break;
          }
      }

      if( filterHasFocus[partner] && (LOWORD(wParam) == IDC_Send) ) {
	  SetFocus(GetDlgItem(hDlg, OPT_ChatInput));
          wParam = IDC_Change;
      }
      /* [AS] End command replacement */

        switch (LOWORD(wParam)) {

	case IDCANCEL: /* let Esc key switch focus back to console */
	    SetFocus(GetDlgItem(hwndConsole, OPT_ConsoleInput));
	    break;

	case IDC_Clear:
	    SendMessage( GetDlgItem(hDlg, IDC_ChatMemo), WM_SETTEXT, 0, (LPARAM) "" );
	    break;

	case IDC_Change:
	    GetDlgItemText(hDlg, IDC_ChatPartner, chatPartner[partner], MSG_SIZ);
	    for(i=0; i<MAX_CHAT; i++) if(chatHandle[i] && i != partner) {
	      // set our button in other open chats
	      SetDlgItemText(chatHandle[i], IDC_Focus1+partner-(i<partner), chatPartner[partner]);
	    }
	    break;

	case IDC_Send:
	    GetDlgItemText(hDlg, OPT_ChatInput, mess, MSG_SIZ);
	    SetDlgItemText(hDlg, OPT_ChatInput, "");
	    // from here on it could be back-end
	    SaveInHistory(mess);
	    if(!strcmp("whispers", chatPartner[partner]))
	      snprintf(buf, MSG_SIZ, "whisper %s\n", mess); // WHISPER box uses "whisper" to send
	    else if(!strcmp("shouts", chatPartner[partner]))
	      snprintf(buf, MSG_SIZ, "shout %s\n", mess); // SHOUT box uses "shout" to send
	    else {
		if(!atoi(chatPartner[partner])) {
		  snprintf(buf, MSG_SIZ, "> %s\r\n", mess); // echo only tells to handle, not channel
		InsertIntoMemo(hDlg, buf);
		snprintf(buf, MSG_SIZ, "xtell %s %s\n", chatPartner[partner], mess);
		} else
		  snprintf(buf, MSG_SIZ, "tell %s %s\n", chatPartner[partner], mess);
	    }
	    SendToICS(buf);
	    break;

	case IDC_Focus1:
	case IDC_Focus2:
	case IDC_Focus3:
	case IDC_Focus4:
	    i = LOWORD(wParam) - IDC_Focus1;
	    if(i >= partner) i++;
	    onTop = i;
	    SetFocus(GetDlgItem(hDlg, IDC_Send));
	    if(chatHandle[i]) {
		int j;
		for(j=0; j<MAX_CHAT; j++) if(i != j && chatHandle[j])
		    Button_SetState(GetDlgItem(chatHandle[j], IDC_Focus1+i-(j<i)), FALSE);
		SetFocus(GetDlgItem(chatHandle[i], OPT_ChatInput));
	    }
	    break;

        default:
          break;
        }

        break;

    case WM_CLOSE:
	chatHandle[partner] = 0;
	chatPartner[partner][0] = 0;
        ChatPopDown();
	for(i=0; i<MAX_CHAT; i++) if(chatHandle[i] && i != partner) {
	    // set our button in other open chats
	    SetDlgItemText(chatHandle[i], IDC_Focus1+partner-(i<partner), "");
	    EnableWindow( GetDlgItem(chatHandle[i], IDC_Focus1+partner-(i<partner)), 0 );
	}
	EndDialog(hDlg, TRUE);
        break;

    case WM_SIZE:
        ResizeWindowControls( hDlg );
        break;

    case WM_ENTERSIZEMOVE:
        return OnEnterSizeMove( &sd, hDlg, wParam, lParam );

    case WM_SIZING:
        return OnSizing( &sd, hDlg, wParam, lParam );

    case WM_MOVING:
        return OnMoving( &sd, hDlg, wParam, lParam );

    case WM_EXITSIZEMOVE:
        return OnExitSizeMove( &sd, hDlg, wParam, lParam );
    }

    return FALSE;
}

// front end
void ChatPopUp(char *icsHandle)
{
  FARPROC lpProc;
  int i, partner = -1;
  char buf[MSG_SIZ];
  static int first = 1;

  CheckMenuItem(GetMenu(hwndMain), IDM_NewChat, MF_CHECKED);
  for(i=0; i<MAX_CHAT; i++) if(chatHandle[i] == NULL) { partner = i; break; }
  if(partner == -1) { DisplayError("You first have to close a Chat Box\nbefore you can open a new one", 0); return; }
  if(icsHandle) { // [HGM] clickbox set handle in advance
    safeStrCpy(chatPartner[partner], icsHandle,
	       sizeof(chatPartner[partner])/sizeof(chatPartner[partner][0]) );
    if(sscanf(icsHandle, "%d", &i) == 1) { // make sure channel is on
	snprintf(buf, MSG_SIZ, "addlist ch %d\n", i);
	SendToICS(buf);
	if(first) first=0, SendToICS(buf); // work-around for weirdness: On public FICS code first attempt on login is completely ignored
    }
  } else chatPartner[partner][0] = NULLCHAR;
  chatCount++;

    lpProc = MakeProcInstance( (FARPROC) ChatProc, hInst );

    /* Note to self: dialog must have the WS_VISIBLE style set, otherwise it's not shown! */
    CreateDialog( hInst, MAKEINTRESOURCE(DLG_Chat), hwndConsole, (DLGPROC)lpProc );

    FreeProcInstance(lpProc);

}

// front end
void ChatPopDown()
{
  if(--chatCount <= 0)
	CheckMenuItem(GetMenu(hwndMain), IDM_NewChat, MF_UNCHECKED);
}


//------------------------ pure back-end routines -------------------------------

void OutputChatMessage(int partner, char *text)
{
	int j, n = strlen(text);

	if(!chatHandle[partner]) return;
	text[n+1] = 0; text[n] = '\n'; text[n-1] = '\r'; // Needs CR to not lose line breaks on copy-paste
	InsertIntoMemo(chatHandle[partner], text);
	if(partner != onTop) for(j=0; j<MAX_CHAT; j++) if(j != partner && chatHandle[j])
	    Button_SetState(GetDlgItem(chatHandle[j], IDC_Focus1+partner-(j<partner)), TRUE);
}
