/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
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

// [HGM] define numbers to indicate icons, for referring to them in platform-independent way
#define nColorBlack   1
#define nColorWhite   2
#define nColorUnknown 3
#define nClear        4
#define nPondering    5
#define nThinking     6
#define nAnalyzing    7

HICON icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle

// [HGM] same for output fields (note that there are two of each type, one per color)
#define nColorIcon 1
#define nStateIcon 2
#define nLabel     3
#define nStateData 4
#define nLabelNPS  5
#define nMemo      6

HWND outputField[2][7]; // [HGM] front-end array to translate output field to window handle

void EngineOutputPopUp();
void EngineOutputPopDown();
int  EngineOutputIsUp();

#define SHOW_PONDERING

/* Imports from backend.c */
char * SavePart(char *str);
extern int opponentKibitzes;

/* Imports from winboard.c */
extern HWND engineOutputDialog;
extern int     engineOutputDialogUp;

extern HINSTANCE hInst;
extern HWND hwndMain;

extern WindowPlacement wpEngineOutput;

/* Module variables */
#define H_MARGIN            2
#define V_MARGIN            2
#define LABEL_V_DISTANCE    1   /* Distance between label and memo */
#define SPLITTER_SIZE       4   /* Distance between first memo and second label */

#define ICON_SIZE           14

#define STATE_UNKNOWN   -1
#define STATE_THINKING   0
#define STATE_IDLE       1
#define STATE_PONDERING  2
#define STATE_ANALYZING  3

static int  windowMode = 1;

static int  needInit = TRUE;

static int  lastDepth[2] = { -1, -1 };
static int  lastForwardMostMove[2] = { -1, -1 };
static int  engineState[2] = { -1, -1 };

typedef struct {
//    HWND hColorIcon; // [HGM] the output-control handles are no loger passed,
//    HWND hLabel;     //       to give better front-end / back-end separation
//    HWND hStateIcon; //       the front-end routines now get them from a (front-end)
//    HWND hStateData; //       table, indexed by output-field indicators.
//    HWND hLabelNPS;
//    HWND hMemo;
    char * name;
    int which;
    int depth;
    u64 nodes;
    int score;
    int time;
    char * pv;
    char * hint;
    int an_move_index;
    int an_move_count;
} EngineOutputData;

static VerifyDisplayMode();
static void UpdateControls( EngineOutputData * ed );
static SetEngineState( int which, int state, char * state_data );

// front end
static HICON LoadIconEx( int id )
{
    return LoadImage( hInst, MAKEINTRESOURCE(id), IMAGE_ICON, ICON_SIZE, ICON_SIZE, 0 );
}

// [HGM] the platform-dependent way of indicating where output should go is now all
// concentrated here, where a table of platform-dependent handles are initialized.
// This cleanses most other routines of front-end stuff, so they can go into the back end.
static void InitializeEngineOutput()
{
 //   if( needInit ) { // needInit was already tested before call
	// [HGM] made this into a table, rather than separate global variables
        icons[nColorBlack]   = LoadIconEx( IDI_BLACK_14 );
        icons[nColorWhite]   = LoadIconEx( IDI_WHITE_14 );
        icons[nColorUnknown] = LoadIconEx( IDI_UNKNOWN_14 );
        icons[nClear]        = LoadIconEx( IDI_TRANS_14 );
        icons[nPondering]    = LoadIconEx( IDI_PONDER_14 );
        icons[nThinking]     = LoadIconEx( IDI_CLOCK_14 );
        icons[nAnalyzing]    = LoadIconEx( IDI_ANALYZE2_14 );

	// [HGM] also make a table of handles to output controls
	// Note that engineOutputDialog must be defined first!
        outputField[0][nColorIcon] = GetDlgItem( engineOutputDialog, IDC_Color1 );
        outputField[0][nLabel]     = GetDlgItem( engineOutputDialog, IDC_EngineLabel1 );
        outputField[0][nStateIcon] = GetDlgItem( engineOutputDialog, IDC_StateIcon1 );
        outputField[0][nStateData] = GetDlgItem( engineOutputDialog, IDC_StateData1 );
        outputField[0][nLabelNPS]  = GetDlgItem( engineOutputDialog, IDC_Engine1_NPS );
        outputField[0][nMemo]      = GetDlgItem( engineOutputDialog, IDC_EngineMemo1 );

        outputField[1][nColorIcon] = GetDlgItem( engineOutputDialog, IDC_Color2 );
        outputField[1][nLabel]     = GetDlgItem( engineOutputDialog, IDC_EngineLabel2 );
        outputField[1][nStateIcon] = GetDlgItem( engineOutputDialog, IDC_StateIcon2 );
        outputField[1][nStateData] = GetDlgItem( engineOutputDialog, IDC_StateData2 );
        outputField[1][nLabelNPS]  = GetDlgItem( engineOutputDialog, IDC_Engine2_NPS );
        outputField[1][nMemo]      = GetDlgItem( engineOutputDialog, IDC_EngineMemo2 );
//        needInit = FALSE;
//    }
}

// front end
static void SetControlPos( HWND hDlg, int id, int x, int y, int width, int height )
{
    HWND hControl = GetDlgItem( hDlg, id );

    SetWindowPos( hControl, HWND_TOP, x, y, width, height, SWP_NOZORDER );
}

#define HIDDEN_X    20000
#define HIDDEN_Y    20000

// front end
static void HideControl( HWND hDlg, int id )
{
    HWND hControl = GetDlgItem( hDlg, id );
    RECT rc;

    GetWindowRect( hControl, &rc );

    /* 
        Avoid hiding an already hidden control, because that causes many
        unnecessary WM_ERASEBKGND messages!
    */
    if( rc.left != HIDDEN_X || rc.top != HIDDEN_Y ) {
        SetControlPos( hDlg, id, 20000, 20000, 100, 100 );
    }
}

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

static int GetHeaderHeight()
{
    int result = GetControlHeight( engineOutputDialog, IDC_EngineLabel1 );

    if( result < ICON_SIZE ) result = ICON_SIZE;

    return result;
}

// The size calculations should be backend? If setControlPos is a platform-dependent way of doing things,
// a platform-independent wrapper for it should be supplied.
static void PositionControlSet( HWND hDlg, int x, int y, int clientWidth, int memoHeight, int idColor, int idEngineLabel, int idNPS, int idMemo, int idStateIcon, int idStateData )
{
    int label_x = x + ICON_SIZE + H_MARGIN;
    int label_h = GetControlHeight( hDlg, IDC_EngineLabel1 );
    int label_y = y + ICON_SIZE - label_h;
    int nps_w = GetControlWidth( hDlg, IDC_Engine1_NPS );
    int nps_x = clientWidth - H_MARGIN - nps_w;
    int state_data_w = GetControlWidth( hDlg, IDC_StateData1 );
    int state_data_x = nps_x - H_MARGIN - state_data_w;
    int state_icon_x = state_data_x - ICON_SIZE - 2;
    int max_w = clientWidth - 2*H_MARGIN;
    int memo_y = y + ICON_SIZE + LABEL_V_DISTANCE;

    SetControlPos( hDlg, idColor, x, y, ICON_SIZE, ICON_SIZE );
    SetControlPos( hDlg, idEngineLabel, label_x, label_y, state_icon_x - label_x, label_h );
    SetControlPos( hDlg, idStateIcon, state_icon_x, y, ICON_SIZE, ICON_SIZE );
    SetControlPos( hDlg, idStateData, state_data_x, label_y, state_data_w, label_h );
    SetControlPos( hDlg, idNPS, nps_x, label_y, nps_w, label_h );
    SetControlPos( hDlg, idMemo, x, memo_y, max_w, memoHeight );
}

// Also here some of the size calculations should go to the back end, and their actual application to a front-end routine
static void ResizeWindowControls( HWND hDlg, int mode )
{
    RECT rc;
    int headerHeight = GetHeaderHeight();
    int labelHeight = GetControlHeight( hDlg, IDC_EngineLabel1 );
    int labelOffset = H_MARGIN + ICON_SIZE + H_MARGIN;
    int labelDeltaY = ICON_SIZE - labelHeight;
    int clientWidth;
    int clientHeight;
    int maxControlWidth;
    int npsWidth;

    /* Initialize variables */
    GetClientRect( hDlg, &rc );

    clientWidth = rc.right - rc.left;
    clientHeight = rc.bottom - rc.top;

    maxControlWidth = clientWidth - 2*H_MARGIN;

    npsWidth = GetControlWidth( hDlg, IDC_Engine1_NPS );

    /* Resize controls */
    if( mode == 0 ) {
        /* One engine */
        PositionControlSet( hDlg, H_MARGIN, V_MARGIN, 
            clientWidth, 
            clientHeight - V_MARGIN - LABEL_V_DISTANCE - headerHeight- V_MARGIN,
            IDC_Color1, IDC_EngineLabel1, IDC_Engine1_NPS, IDC_EngineMemo1, IDC_StateIcon1, IDC_StateData1 );

        /* Hide controls for the second engine */
        HideControl( hDlg, IDC_Color2 );
        HideControl( hDlg, IDC_EngineLabel2 );
        HideControl( hDlg, IDC_StateIcon2 );
        HideControl( hDlg, IDC_StateData2 );
        HideControl( hDlg, IDC_Engine2_NPS );
        HideControl( hDlg, IDC_EngineMemo2 );
        SendDlgItemMessage( hDlg, IDC_EngineMemo2, WM_SETTEXT, 0, (LPARAM) "" );
        /* TODO: we should also hide/disable them!!! what about tab stops?!?! */
    }
    else {
        /* Two engines */
        int memo_h = (clientHeight - headerHeight*2 - V_MARGIN*2 - LABEL_V_DISTANCE*2 - SPLITTER_SIZE) / 2;
        int header1_y = V_MARGIN;
        int header2_y = V_MARGIN + headerHeight + LABEL_V_DISTANCE + memo_h + SPLITTER_SIZE;

        PositionControlSet( hDlg, H_MARGIN, header1_y, clientWidth, memo_h,
            IDC_Color1, IDC_EngineLabel1, IDC_Engine1_NPS, IDC_EngineMemo1, IDC_StateIcon1, IDC_StateData1 );

        PositionControlSet( hDlg, H_MARGIN, header2_y, clientWidth, memo_h,
            IDC_Color2, IDC_EngineLabel2, IDC_Engine2_NPS, IDC_EngineMemo2, IDC_StateIcon2, IDC_StateData2 );
    }

    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo1), NULL, FALSE );
    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo2), NULL, FALSE );
}

// front end. Actual printing of PV lines into the output field
static void InsertIntoMemo( int which, char * text )
{
    SendMessage( outputField[which][nMemo], EM_SETSEL, 0, 0 );

    SendMessage( outputField[which][nMemo], EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) text );
}

// front end. Associates an icon with an output field ("control" in Windows jargon).
// [HGM] let it find out the output field from the 'which' number by itself
static void SetIcon( int which, int field, int nIcon )
{

    if( nIcon != 0 ) {
        SendMessage( outputField[which][field], STM_SETICON, (WPARAM) icons[nIcon], 0 );
    }
}

// front end wrapper for SetWindowText, taking control number in stead of handle
void DoSetWindowText(int which, int field, char *s_label)
{
    SetWindowText( outputField[which][field], s_label );
}

// This seems pure front end
LRESULT CALLBACK EngineOutputProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;

    switch (message) {
    case WM_INITDIALOG:
        if( engineOutputDialog == NULL ) {
            engineOutputDialog = hDlg;

            RestoreWindowPlacement( hDlg, &wpEngineOutput ); /* Restore window placement */

            ResizeWindowControls( hDlg, windowMode );

            SetEngineState( 0, STATE_IDLE, "" );
            SetEngineState( 1, STATE_IDLE, "" );
        }

        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
          EndDialog(hDlg, TRUE);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, FALSE);
          return TRUE;

        default:
          break;
        }

        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO * mmi = (MINMAXINFO *) lParam;
        
            mmi->ptMinTrackSize.x = 100;
            mmi->ptMinTrackSize.y = 160;
        }
        break;

    case WM_CLOSE:
        EngineOutputPopDown();
        break;

    case WM_SIZE:
        ResizeWindowControls( hDlg, windowMode );
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
void EngineOutputPopUp()
{
  FARPROC lpProc;
  
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEngineOutput, MF_CHECKED);

  if( engineOutputDialog ) {
    SendMessage( engineOutputDialog, WM_INITDIALOG, 0, 0 );

    if( ! engineOutputDialogUp ) {
        ShowWindow(engineOutputDialog, SW_SHOW);
    }
  }
  else {
    lpProc = MakeProcInstance( (FARPROC) EngineOutputProc, hInst );

    /* Note to self: dialog must have the WS_VISIBLE style set, otherwise it's not shown! */
    CreateDialog( hInst, MAKEINTRESOURCE(DLG_EngineOutput), hwndMain, (DLGPROC)lpProc );

    FreeProcInstance(lpProc);
  }

  // [HGM] displaced to after creation of dialog, to allow initialization of output fields
  if( needInit ) {
      InitializeEngineOutput();
      needInit = FALSE;
  }

  engineOutputDialogUp = TRUE;
}

// front end
void EngineOutputPopDown()
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEngineOutput, MF_UNCHECKED);

  if( engineOutputDialog ) {
      ShowWindow(engineOutputDialog, SW_HIDE);
  }

  engineOutputDialogUp = FALSE;
}

// front end. [HGM] Takes handle of output control from table, so only number is passed
void DoClearMemo(int which)
{
        SendMessage( outputField[which][nMemo], WM_SETTEXT, 0, (LPARAM) "" );
}

//------------------------ pure back-end routines -------------------------------


// back end, due to front-end wrapper for SetWindowText, and new SetIcon arguments
static SetEngineState( int which, int state, char * state_data )
{
    int x_which = 1 - which;

    if( engineState[ which ] != state ) {
        engineState[ which ] = state;

        switch( state ) {
        case STATE_THINKING:
            SetIcon( which, nStateIcon, nThinking );
            if( engineState[ x_which ] == STATE_THINKING ) {
                SetEngineState( x_which, STATE_IDLE, "" );
            }
            break;
        case STATE_PONDERING:
            SetIcon( which, nStateIcon, nPondering );
            break;
        case STATE_ANALYZING:
            SetIcon( which, nStateIcon, nAnalyzing );
            break;
        default:
            SetIcon( which, nStateIcon, nClear );
            break;
        }
    }

    if( state_data != 0 ) {
        DoSetWindowText( which, nStateData, state_data );
    }
}

// back end, now the front-end wrapper ClearMemo is used, and ed no longer contains handles.
void EngineOutputUpdate( FrontEndProgramStats * stats )
{
    EngineOutputData ed;
    int clearMemo = FALSE;
    int which;
    int depth;

    if( stats == 0 ) {
        SetEngineState( 0, STATE_IDLE, "" );
        SetEngineState( 1, STATE_IDLE, "" );
        return;
    }

    if(gameMode == IcsObserving && !appData.icsEngineAnalyze)
	return; // [HGM] kibitz: shut up engine if we are observing an ICS game

    which = stats->which;
    depth = stats->depth;

    if( which < 0 || which > 1 || depth < 0 || stats->time < 0 || stats->pv == 0 ) {
        return;
    }

    if( engineOutputDialog == NULL ) {
        return;
    }

    VerifyDisplayMode();

    ed.which = which;
    ed.depth = depth;
    ed.nodes = stats->nodes;
    ed.score = stats->score;
    ed.time = stats->time;
    ed.pv = stats->pv;
    ed.hint = stats->hint;
    ed.an_move_index = stats->an_move_index;
    ed.an_move_count = stats->an_move_count;

    /* Get target control. [HGM] this is moved to front end, which get them from a table */
    if( which == 0 ) {
        ed.name = first.tidy;
    }
    else {
        ed.name = second.tidy;
    }

    /* Clear memo if needed */
    if( lastDepth[which] > depth || (lastDepth[which] == depth && depth <= 1) ) {
        clearMemo = TRUE;
    }

    if( lastForwardMostMove[which] != forwardMostMove ) {
        clearMemo = TRUE;
    }

    if( clearMemo ) DoClearMemo(which);

    /* Update */
    lastDepth[which] = depth;
    lastForwardMostMove[which] = forwardMostMove;

    if( ed.pv != 0 && ed.pv[0] == ' ' ) {
        if( strncmp( ed.pv, " no PV", 6 ) == 0 ) { /* Hack on hack! :-O */
            ed.pv = "";
        }
    }

    UpdateControls( &ed );
}

#define ENGINE_COLOR_WHITE      'w'
#define ENGINE_COLOR_BLACK      'b'
#define ENGINE_COLOR_UNKNOWN    ' '

// pure back end
char GetEngineColor( int which )
{
    char result = ENGINE_COLOR_UNKNOWN;

    if( which == 0 || which == 1 ) {
        ChessProgramState * cps;

        switch (gameMode) {
        case MachinePlaysBlack:
        case IcsPlayingBlack:
            result = ENGINE_COLOR_BLACK;
            break;
        case MachinePlaysWhite:
        case IcsPlayingWhite:
            result = ENGINE_COLOR_WHITE;
            break;
        case AnalyzeMode:
        case AnalyzeFile:
            result = WhiteOnMove(forwardMostMove) ? ENGINE_COLOR_WHITE : ENGINE_COLOR_BLACK;
            break;
        case TwoMachinesPlay:
            cps = (which == 0) ? &first : &second;
            result = cps->twoMachinesColor[0];
            result = result == 'w' ? ENGINE_COLOR_WHITE : ENGINE_COLOR_BLACK;
            break;
        }
    }

    return result;
}

// pure back end
char GetActiveEngineColor()
{
    char result = ENGINE_COLOR_UNKNOWN;

    if( gameMode == TwoMachinesPlay ) {
        result = WhiteOnMove(forwardMostMove) ? ENGINE_COLOR_WHITE : ENGINE_COLOR_BLACK;
    }

    return result;
}

// pure back end
static int IsEnginePondering( int which )
{
    int result = FALSE;

    switch (gameMode) {
    case MachinePlaysBlack:
    case IcsPlayingBlack:
        if( WhiteOnMove(forwardMostMove) ) result = TRUE;
        break;
    case MachinePlaysWhite:
    case IcsPlayingWhite:
        if( ! WhiteOnMove(forwardMostMove) ) result = TRUE;
        break;
    case TwoMachinesPlay:
        if( GetActiveEngineColor() != ENGINE_COLOR_UNKNOWN ) {
            if( GetEngineColor( which ) != GetActiveEngineColor() ) result = TRUE;
        }
        break;
    }

    return result;
}

// back end
static void SetDisplayMode( int mode )
{
    if( windowMode != mode ) {
        windowMode = mode;

        ResizeWindowControls( engineOutputDialog, mode );
    }
}

// pure back end
static VerifyDisplayMode()
{
    int mode;

    /* Get proper mode for current game */
    switch( gameMode ) {
    case IcsObserving:    // [HGM] ICS analyze
	if(!appData.icsEngineAnalyze) return;
    case AnalyzeMode:
    case AnalyzeFile:
    case MachinePlaysWhite:
    case MachinePlaysBlack:
        mode = 0;
        break;
    case IcsPlayingWhite:
    case IcsPlayingBlack:
        mode = appData.zippyPlay && opponentKibitzes; // [HGM] kibitz
        break;
    case TwoMachinesPlay:
        mode = 1;
        break;
    default:
        /* Do not change */
        return;
    }

    SetDisplayMode( mode );
}

// back end. Determine what icon to se in the color-icon field, and print it
static void SetEngineColorIcon( int which )
{
    char color = GetEngineColor(which);
    int nicon = 0;

    if( color == ENGINE_COLOR_BLACK )
        nicon = nColorBlack;
    else if( color == ENGINE_COLOR_WHITE )
        nicon = nColorWhite;
    else
        nicon = nColorUnknown;

    SetIcon( which, nColorIcon, nicon );
}

#define MAX_NAME_LENGTH 32

// pure back end, now SetWindowText is called via wrapper DoSetWindowText
static void UpdateControls( EngineOutputData * ed )
{
    int isPondering = FALSE;

    char s_label[MAX_NAME_LENGTH + 32];
    
    char * name = ed->name;

    /* Label */
    if( name == 0 || *name == '\0' ) {
        name = "?";
    }

    strncpy( s_label, name, MAX_NAME_LENGTH );
    s_label[ MAX_NAME_LENGTH-1 ] = '\0';

#ifdef SHOW_PONDERING
    if( IsEnginePondering( ed->which ) ) {
        char buf[8];

        buf[0] = '\0';

        if( ed->hint != 0 && *ed->hint != '\0' ) {
            strncpy( buf, ed->hint, sizeof(buf) );
            buf[sizeof(buf)-1] = '\0';
        }
        else if( ed->pv != 0 && *ed->pv != '\0' ) {
            char * sep = strchr( ed->pv, ' ' );
            int buflen = sizeof(buf);

            if( sep != NULL ) {
                buflen = sep - ed->pv + 1;
                if( buflen > sizeof(buf) ) buflen = sizeof(buf);
            }

            strncpy( buf, ed->pv, buflen );
            buf[ buflen-1 ] = '\0';
        }

        SetEngineState( ed->which, STATE_PONDERING, buf );
    }
    else if( gameMode == TwoMachinesPlay ) {
        SetEngineState( ed->which, STATE_THINKING, "" );
    }
    else if( gameMode == AnalyzeMode || gameMode == AnalyzeFile
	  || gameMode == IcsObserving && appData.icsEngineAnalyze) { // [HGM] ICS-analyze
        char buf[64];
        int time_secs = ed->time / 100;
        int time_mins = time_secs / 60;

        buf[0] = '\0';

        if( ed->an_move_index != 0 && ed->an_move_count != 0 && *ed->hint != '\0' ) {
            char mov[16];

            strncpy( mov, ed->hint, sizeof(mov) );
            mov[ sizeof(mov)-1 ] = '\0';

            sprintf( buf, "%d/%d: %s [%02d:%02d:%02d]", ed->an_move_index, ed->an_move_count, mov, time_mins / 60, time_mins % 60, time_secs % 60 );
        }

        SetEngineState( ed->which, STATE_ANALYZING, buf );
    }
    else {
        SetEngineState( ed->which, STATE_IDLE, "" );
    }
#endif

    DoSetWindowText( ed->which, nLabel, s_label );

    s_label[0] = '\0';

    if( ed->time > 0 && ed->nodes > 0 ) {
        unsigned long nps_100 = ed->nodes / ed->time;

        if( nps_100 < 100000 ) {
            sprintf( s_label, "NPS: %lu", nps_100 * 100 );
        }
        else {
            sprintf( s_label, "NPS: %.1fk", nps_100 / 10.0 );
        }
    }

    DoSetWindowText( ed->which, nLabelNPS, s_label );

    /* Memo */
    if( ed->pv != 0 && *ed->pv != '\0' ) {
        char s_nodes[24];
        char s_score[16];
        char s_time[24];
        char buf[256];
        int buflen;
        int time_secs = ed->time / 100;
        int time_cent = ed->time % 100;

        /* Nodes */
        if( ed->nodes < 1000000 ) {
            sprintf( s_nodes, u64Display, ed->nodes );
        }
        else {
            sprintf( s_nodes, "%.1fM", u64ToDouble(ed->nodes) / 1000000.0 );
        }

        /* Score */
        if( ed->score > 0 ) {
            sprintf( s_score, "+%.2f", ed->score / 100.0 );
        }
        else {
            sprintf( s_score, "%.2f", ed->score / 100.0 );
        }

        /* Time */
        sprintf( s_time, "%d:%02d.%02d", time_secs / 60, time_secs % 60, time_cent );

        /* Put all together... */
        sprintf( buf, "%3d\t%s\t%s\t%s\t", ed->depth, s_score, s_nodes, s_time );

        /* Add PV */
        buflen = strlen(buf);

        strncpy( buf + buflen, ed->pv, sizeof(buf) - buflen );

        buf[ sizeof(buf) - 3 ] = '\0';

        strcat( buf + buflen, "\r\n" );

        /* Update memo */
        InsertIntoMemo( ed->which, buf );
    }

    /* Colors */
    SetEngineColorIcon( ed->which );
}

// back end
int EngineOutputIsUp()
{
    return engineOutputDialogUp;
}

// [HGM] kibitz: write kibitz line; split window for it if necessary
void OutputKibitz(int window, char *text)
{
	if(!EngineOutputIsUp()) return;
	if(!opponentKibitzes) { // on first kibitz of game, clear memos
	    DoClearMemo(1);
	    if(gameMode == IcsObserving) DoClearMemo(0);
	}
	opponentKibitzes = TRUE; // this causes split window DisplayMode in ICS modes.
	VerifyDisplayMode();
	if(gameMode == IcsObserving) {
	    DoSetWindowText(0, nLabel, gameInfo.white);
	    SetIcon( 0, nColorIcon,  nColorWhite);
	    SetIcon( 0, nStateIcon,  nClear);
	}
	DoSetWindowText(1, nLabel, gameMode == IcsPlayingBlack ? gameInfo.white : gameInfo.black); // opponent name
	SetIcon( 1, nColorIcon,  gameMode == IcsPlayingBlack ? nColorWhite : nColorBlack);
	SetIcon( 1, nStateIcon,  nClear);
	InsertIntoMemo(window-1, text);
}
