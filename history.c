/*
 * Move history for WinBoard
 *
 * Author: Alessandro Scotti (Dec 2005)
 * back-end part split off by HGM
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * ------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"

/* templates for low-level front-end tasks (requiring platform-dependent implementation) */
void ClearHistoryMemo P((void));                                   // essential
int AppendToHistoryMemo P(( char * text, int bold, int colorNr )); // essential (coloring / styling optional)
void HighlightMove P(( int from, int to, Boolean highlight ));     // optional (can be dummy)
void ScrollToCurrent P((int caretPos));                            // optional (can be dummy)

/* templates for front-end entry point to allow inquiring about front-end state */
Boolean MoveHistoryDialogExists P((void));
Boolean MoveHistoryIsUp P((void));

/* Module globals */
typedef char MoveHistoryString[ MOVE_LEN*2 ];

static int lastFirst = 0;
static int lastLast = 0;
static int lastCurrent = -1;
static int lastGames;

static char lastLastMove[ MOVE_LEN ];

static MoveHistoryString * currMovelist;
static ChessProgramStats_Move * currPvInfo;
static int currFirst = 0;
static int currLast = 0;
static int currCurrent = -1;

typedef struct {
    int memoOffset;
    int memoLength;
} HistoryMove;

static HistoryMove histMoves[ MAX_MOVES ];

/* Note: in the following code a "Memo" is a Rich Edit control (it's Delphi lingo) */

// back-end after replacing Windows data-types by equivalents
static Boolean
OnlyCurrentPositionChanged ()
{
    Boolean result = FALSE;

    if( lastFirst >= 0 &&
        lastLast >= lastFirst &&
        lastCurrent >= lastFirst &&
        currFirst == lastFirst &&
        currLast == lastLast &&
        currCurrent >= 0 &&
        lastGames == storedGames )
    {
        result = TRUE;

        /* Special case: last move changed */
        if( currCurrent == currLast-1 ) {
            if( strcmp( currMovelist[currCurrent], lastLastMove ) != 0 ) {
                result = FALSE;
            }
        }
    }

    return result;
}

// back-end, after replacing Windows data types
static Boolean
OneMoveAppended ()
{
    Boolean result = FALSE;

    if( lastCurrent >= 0 && lastCurrent >= lastFirst && lastLast >= lastFirst &&
        currCurrent >= 0 && currCurrent >= currFirst && currLast >= currFirst &&
        lastFirst == currFirst &&
        lastLast == (currLast-1) &&
        lastCurrent == (currCurrent-1) &&
        currCurrent == (currLast-1) &&
        lastGames == storedGames )
    {
        result = TRUE;
    }

    return result;
}

// back-end, now that color and font-style are passed as numbers
static void
AppendMoveToMemo (int index)
{
    char buf[64];

    if( index < 0 || index >= MAX_MOVES ) {
        return;
    }

    buf[0] = '\0';

    /* Move number */
    if( (index % 2) == 0 ) {
        sprintf( buf, "%d.%s ", (index / 2)+1, index & 1 ? ".." : "" );
        AppendToHistoryMemo( buf, 1, 0 ); // [HGM] 1 means bold, 0 default color
    }

    /* Move text */
    safeStrCpy( buf, SavePart( currMovelist[index]) , sizeof( buf)/sizeof( buf[0]) );
    strcat( buf, " " );

    histMoves[index].memoOffset = AppendToHistoryMemo( buf, 0, 0 );
    histMoves[index].memoLength = strlen(buf)-1;

    /* PV info (if any) */
    if( appData.showEvalInMoveHistory && currPvInfo[index].depth > 0 ) {
        sprintf( buf, "{%s%.2f/%d} ",
            currPvInfo[index].score >= 0 ? "+" : "",
            currPvInfo[index].score / 100.0,
            currPvInfo[index].depth );

        AppendToHistoryMemo( buf, 0, 1); // [HGM] 1 means gray
    }
}

// back-end
void
RefreshMemoContent ()
{
    int i;

    ClearHistoryMemo();

    for( i=currFirst; i<currLast; i++ ) {
        AppendMoveToMemo( i );
    }
}

// back-end part taken out of HighlightMove to determine character positions
static void
DoHighlight (int index, int onoff)
{
    if( index >= 0 && index < MAX_MOVES ) {
        HighlightMove( histMoves[index].memoOffset,
            histMoves[index].memoOffset + histMoves[index].memoLength, onoff );
    }
}

// back-end, now that a wrapper is provided for the front-end code to do the actual scrolling
void
MemoContentUpdated ()
{
    int caretPos;

    if(lastCurrent <= currLast) DoHighlight( lastCurrent, FALSE );

    lastFirst = currFirst;
    lastLast = currLast;
    lastCurrent = currCurrent;
    lastGames = storedGames;
    lastLastMove[0] = '\0';

    if( lastLast > 0 ) {
      safeStrCpy( lastLastMove, SavePart( currMovelist[lastLast-1] ) , sizeof( lastLastMove)/sizeof( lastLastMove[0]) );
    }

    /* Deselect any text, move caret to end of memo */
    if( currCurrent >= 0 ) {
        caretPos = histMoves[currCurrent].memoOffset + histMoves[currCurrent].memoLength;
    }
    else {
        caretPos = -1;
    }

    ScrollToCurrent(caretPos);
    DoHighlight( currCurrent, TRUE ); // [HGM] moved last, because in X some scrolling methods spoil highlighting
}

// back-end. Must be called as double-click call-back on move-history text edit
void
FindMoveByCharIndex (int char_index)
{
    int index;

    for( index=currFirst; index<currLast; index++ ) {
        if( char_index >= histMoves[index].memoOffset &&
            char_index <  (histMoves[index].memoOffset + histMoves[index].memoLength) )
        {
            ToNrEvent( index + 1 ); // moved here from call-back
        }
    }
}

// back-end. In WinBoard called by call-back, but could be called directly by SetIfExists?
void
UpdateMoveHistory ()
{
        /* Update the GUI */
        if( OnlyCurrentPositionChanged() ) {
            /* Only "cursor" changed, no need to update memo content */
        }
        else if( OneMoveAppended() ) {
            AppendMoveToMemo( currCurrent );
        }
        else {
            RefreshMemoContent();
        }

        MemoContentUpdated();
}

// back-end
void
MoveHistorySet (char movelist[][2*MOVE_LEN], int first, int last, int current, ChessProgramStats_Move * pvInfo)
{
    /* [AS] Danger! For now we rely on the movelist parameter being a static variable! */

    currMovelist = movelist;
    currFirst = first;
    currLast = last;
    currCurrent = current;
    currPvInfo = pvInfo;

    if(MoveHistoryDialogExists())
        UpdateMoveHistory(); // [HGM] call this directly, in stead of through call-back
}
