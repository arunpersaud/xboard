/*
 * Chat window (PV)
 *
 * Author: H.G.Muller (August 2009)
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

#include "common.h"
#include "winboard.h"
#include "frontend.h"
#include "backend.h"

#include "wsnap.h"

int chatCount;
extern char chatPartner[MAX_CHAT][MSG_SIZ];
HANDLE chatHandle[MAX_CHAT];

void SendToICS P((char *s));
void ChatPopUp();
void ChatPopDown();

/* Imports from backend.c */
char * SavePart(char *str);
extern int opponentKibitzes;

/* Imports from winboard.c */
extern HWND ChatDialog;

extern HINSTANCE hInst;
extern HWND hwndMain;

extern WindowPlacement wpChat[MAX_CHAT];

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

// This seems pure front end
LRESULT CALLBACK ChatProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;
    char buf[MSG_SIZ], mess[MSG_SIZ];
    int partner = -1, i;

    for(i=0; i<MAX_CHAT; i++) if(hDlg == chatHandle[i]) { partner = i; break; }

    switch (message) {
    case WM_INITDIALOG:
	if(partner<0) {
		for(i=0; i<MAX_CHAT; i++) if(chatHandle[i] == NULL) { partner = i; break; }
	        chatHandle[partner] = hDlg;
		sprintf(buf, "Chat Window %s", first.tidy);
		SetWindowText(hDlg, buf);
        }
	chatPartner[partner][0] = 0;

        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

	case IDCANCEL:
	    chatHandle[partner] = 0;
	    chatPartner[partner][0] = 0;
            ChatPopDown();
	    EndDialog(hDlg, TRUE);
            break;

	case IDC_Clear:
	    SendMessage( GetDlgItem(hDlg, IDC_ChatMemo), WM_SETTEXT, 0, (LPARAM) "" );
	    break;

	case IDC_Change:
	    GetDlgItemText(hDlg, IDC_ChatPartner, chatPartner[partner], MSG_SIZ);
	    break;

	case IDC_Send:
	    GetDlgItemText(hDlg, OPT_ChatInput, mess, MSG_SIZ);
	    SetDlgItemText(hDlg, OPT_ChatInput, "");
	    // from here on it could be back-end
	    if(!strcmp("WHISPER", chatPartner[partner]))
		sprintf(buf, "whisper %s\n", mess); // WHISPER box uses "whisper" to send
	    else {
		if(!atoi(chatPartner[partner])) {
		    sprintf(buf, "> %s\n", mess); // echo only tells to handle, not channel
		InsertIntoMemo(hDlg, buf);
		}
		sprintf(buf, "xtell %s %s\n", chatPartner[partner], mess);
	    }
	    SendToICS(buf);
	    break;

        default:
          break;
        }

        break;

    case WM_CLOSE:
	chatHandle[partner] = 0;
	chatPartner[partner][0] = 0;
        ChatPopDown();
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
void ChatPopUp()
{
  FARPROC lpProc;
  
  if(chatCount >= MAX_CHAT) return;

  CheckMenuItem(GetMenu(hwndMain), IDM_NewChat, MF_CHECKED);
  chatCount++;

    lpProc = MakeProcInstance( (FARPROC) ChatProc, hInst );

    /* Note to self: dialog must have the WS_VISIBLE style set, otherwise it's not shown! */
    CreateDialog( hInst, MAKEINTRESOURCE(DLG_Chat), hwndMain, (DLGPROC)lpProc );

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
	if(!chatHandle[partner]) return;

	InsertIntoMemo(chatHandle[partner], text);
}
