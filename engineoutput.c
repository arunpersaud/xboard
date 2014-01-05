/*
 * engineoutput.c - split-off backe-end from Engine output (PV) by HGM
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 1995, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

#define SHOW_PONDERING

#include "config.h"

#include <stdio.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* not STDC_HEADERS */
# if HAVE_STRING_H
#  include <string.h>
# else /* not HAVE_STRING_H */
#  include <strings.h>
# endif /* not HAVE_STRING_H */
#endif /* not STDC_HEADERS */

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "moves.h"
#include "engineoutput.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# ifdef WIN32
#  define  _(s) T_(s)
#  undef  ngettext
#  define  ngettext(s,p,n) T_(p)
# else
#  define  _(s) (s)
# endif
# define N_(s)  s
#endif

typedef struct {
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
    int moveKey;
} EngineOutputData;

// called by other front-end
void EngineOutputUpdate( FrontEndProgramStats * stats );
void OutputKibitz(int window, char *text);

// module back-end routines
static void VerifyDisplayMode();
static void UpdateControls( EngineOutputData * ed );

static int  lastDepth[2] = { -1, -1 };
static int  lastForwardMostMove[2] = { -1, -1 };
static int  engineState[2] = { -1, -1 };
static char lastLine[2][MSG_SIZ];
static char header[2][MSG_SIZ];

#define MAX_VAR 400
static int scores[MAX_VAR], textEnd[MAX_VAR], keys[MAX_VAR], curDepth[2], nrVariations[2];

extern int initialRulePlies;

void
MakeEngineOutputTitle ()
{
	static char buf[MSG_SIZ];
	static char oldTitle[MSG_SIZ];
	char title[MSG_SIZ];
	int count, rule = 2*appData.ruleMoves;

	snprintf(title, MSG_SIZ, _("Engine Output") );

	if(!EngineOutputIsUp()) return;
	// figure out value of 50-move counter
	count = currentMove;
	while( (signed char)boards[count][EP_STATUS] <= EP_NONE && count > backwardMostMove ) count--;
	if( count == backwardMostMove ) count -= initialRulePlies;
	count = currentMove - count;
	if(!rule) rule = 100;
	if(count >= rule - 40 && (!appData.icsActive || gameMode == IcsObserving || appData.zippyPlay)) {
		snprintf(buf, MSG_SIZ, ngettext("%s (%d reversible ply)", "%s (%d reversible plies)", count), title, count);
		safeStrCpy(title, buf, MSG_SIZ);
	}
	if(!strcmp(oldTitle, title)) return;
	safeStrCpy(oldTitle, title, MSG_SIZ);
	SetEngineOutputTitle(title);
}

// back end, due to front-end wrapper for SetWindowText, and new SetIcon arguments
void
SetEngineState (int which, enum ENGINE_STATE state, char * state_data)
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
void
SetProgramStats (FrontEndProgramStats * stats) // now directly called by back-end
{
    EngineOutputData ed;
    int clearMemo = FALSE;
    int which, depth, multi;
    ChessMove moveType;
    int ff, ft, rf, rt;
    char pc;

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

    if( !EngineOutputDialogExists() ) {
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

    if( ed.pv != 0 && ed.pv[0] == ' ' ) {
        if( strncmp( ed.pv, " no PV", 6 ) == 0 ) { /* Hack on hack! :-O */
            ed.pv = "";
        }
    }

    /* Clear memo if needed */
    if( lastDepth[which] > depth || (lastDepth[which] == depth && depth <= 1 && ed.pv[0]) ) { // no reason to clear if we won't add line
        clearMemo = TRUE;
    }

    if( lastForwardMostMove[which] != forwardMostMove ) {
        clearMemo = TRUE;
    }

    if( clearMemo ) {
        DoClearMemo(which); nrVariations[which] = 0;
        header[which][0] = NULLCHAR;
        if(gameMode == AnalyzeMode) {
          ChessProgramState *cps = (which ? &second : &first);
          if((multi = MultiPV(cps)) >= 0) {
            snprintf(header[which], MSG_SIZ, "\t%s viewpoint\t\tfewer / Multi-PV setting = %d / more\n",
                                       appData.whitePOV || appData.scoreWhite ? "white" : "mover", cps->option[multi].value);
	  }
          if(!which) snprintf(header[which]+strlen(header[which]), MSG_SIZ-strlen(header[which]), "%s", exclusionHeader);
          InsertIntoMemo( which, header[which], 0);
        } else
        if(appData.ponderNextMove && lastLine[which][0]) {
            InsertIntoMemo( which, lastLine[which], 0 );
            InsertIntoMemo( which, "\n", 0 );
        }
    }

    if(ed.pv && ed.pv[0] && ParseOneMove(ed.pv, currentMove, &moveType, &ff, &rf, &ft, &rt, &pc))
	ed.moveKey = (ff<<24 | rf << 16 | ft << 8 | rt) ^ pc*87161;
    else ed.moveKey = ed.nodes; // kludge to get unique key unlikely to match any move

    /* Update */
    lastDepth[which] = depth == 1 && ed.nodes == 0 ? 0 : depth; // [HGM] info-line kudge
    lastForwardMostMove[which] = forwardMostMove;

    UpdateControls( &ed );
}

#define ENGINE_COLOR_WHITE      'w'
#define ENGINE_COLOR_BLACK      'b'
#define ENGINE_COLOR_UNKNOWN    ' '

// pure back end
static char
GetEngineColor (int which)
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
        default: ; // does not happen, but suppresses pedantic warnings
        }
    }

    return result;
}

// pure back end
static char
GetActiveEngineColor ()
{
    char result = ENGINE_COLOR_UNKNOWN;

    if( gameMode == TwoMachinesPlay ) {
        result = WhiteOnMove(forwardMostMove) ? ENGINE_COLOR_WHITE : ENGINE_COLOR_BLACK;
    }

    return result;
}

// pure back end
static int
IsEnginePondering (int which)
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
    default: ; // does not happen, but suppresses pedantic warnings
    }

    return result;
}

// back end
static void
SetDisplayMode (int mode)
{
    if( windowMode != mode ) {
        windowMode = mode;

        ResizeWindowControls( mode );
    }
}

// pure back end
static void
VerifyDisplayMode ()
{
    int mode;

    /* Get proper mode for current game */
    switch( gameMode ) {
    case IcsObserving:    // [HGM] ICS analyze
	if(!appData.icsEngineAnalyze) return;
    case AnalyzeFile:
    case MachinePlaysWhite:
    case MachinePlaysBlack:
        mode = 0;
        break;
    case AnalyzeMode:
        mode = second.analyzing;
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

// back end. Determine what icon to set in the color-icon field, and print it
void
SetEngineColorIcon (int which)
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

// [HGM] multivar: sort Thinking Output within one depth on score

static int
InsertionPoint (int len, EngineOutputData *ed)
{
	int i, offs = 0, newScore = ed->score, n = ed->which;

	if(ed->nodes == 0 && ed->score == 0 && ed->time == 0)
		newScore = 1e6; // info lines inserted on top
	if(ed->depth != curDepth[n]) { // depth has changed
		curDepth[n] = ed->depth;
		nrVariations[n] = 0; // throw away everything we had
	}
	// loop through all lines. Note even / odd used for different panes
	for(i=nrVariations[n]-2; i>=0; i-=2) {
		// put new item behind those we haven't looked at
		offs = textEnd[i+n];
		textEnd[i+n+2] = offs + len;
		scores[i+n+2] = newScore;
		keys[i+n+2] = ed->moveKey;
		if(ed->moveKey != keys[i+n] && // same move always tops previous one (as a higher score must be a fail low)
		   newScore < scores[i+n]) break;
		// if it had higher score as previous, move previous in stead
		scores[i+n+2] = ed->moveKey == keys[i+n] ? newScore : scores[i+n]; // correct scores of fail-low/high searches
		textEnd[i+n+2] = textEnd[i+n] + len;
		keys[i+n+2] = keys[i+n];
	}
	if(i<0) {
		offs = 0;
		textEnd[n] = offs + len;
		scores[n] = newScore;
	}
	nrVariations[n] += 2;
      return offs + (gameMode == AnalyzeMode)*strlen(header[ed->which]);
}


// pure back end, now SetWindowText is called via wrapper DoSetWindowText
static void
UpdateControls (EngineOutputData *ed)
{
//    int isPondering = FALSE;

    char s_label[MAX_NAME_LENGTH + 32];
    int h;
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
	  || (gameMode == IcsObserving && appData.icsEngineAnalyze)) { // [HGM] ICS-analyze
        char buf[64];
        int time_secs = ed->time / 100;
        int time_mins = time_secs / 60;

        buf[0] = '\0';

        if( ed->an_move_index != 0 && ed->an_move_count != 0 && *ed->hint != '\0' ) {
            char mov[16];

            strncpy( mov, ed->hint, sizeof(mov) );
            mov[ sizeof(mov)-1 ] = '\0';

            snprintf( buf, sizeof(buf)/sizeof(buf[0]), "[%d] %d/%d: %s [%02d:%02d:%02d]", ed->depth, ed->an_move_index,
			ed->an_move_count, mov, time_mins / 60, time_mins % 60, time_secs % 60 );
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
	  snprintf( s_label, sizeof(s_label)/sizeof(s_label[0]), "%s: %lu", _("NPS"), nps_100 * 100 );
        }
        else {
	  snprintf( s_label, sizeof(s_label)/sizeof(s_label[0]), "%s: %.1fk", _("NPS"), nps_100 / 10.0 );
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
            snprintf( s_nodes, sizeof(s_nodes)/sizeof(s_nodes[0]), u64Display, ed->nodes );
        }
        else {
            snprintf( s_nodes, sizeof(s_nodes)/sizeof(s_nodes[0]), "%.1fM", u64ToDouble(ed->nodes) / 1000000.0 );
        }

        /* Score */
        h = ((gameMode == AnalyzeMode && appData.whitePOV || appData.scoreWhite) && !WhiteOnMove(currentMove) ? -1 : 1) * ed->score;
        if( h > 0 ) {
	  snprintf( s_score, sizeof(s_score)/sizeof(s_score[0]), "+%.2f", h / 100.0 );
        }
        else {
	  snprintf( s_score, sizeof(s_score)/sizeof(s_score[0]), "%.2f", h / 100.0 );
        }

        /* Time */
        snprintf( s_time, sizeof(s_time)/sizeof(s_time[0]), "%d:%02d.%02d", time_secs / 60, time_secs % 60, time_cent );

        /* Put all together... */
	if(ed->nodes == 0 && ed->score == 0 && ed->time == 0)
	  snprintf( buf, sizeof(buf)/sizeof(buf[0]), "%3d\t", ed->depth );
	else
	  snprintf( buf, sizeof(buf)/sizeof(buf[0]), "%3d\t%s\t%s\t%s\t", ed->depth, s_score, s_nodes, s_time );

        /* Add PV */
        buflen = strlen(buf);

        strncpy( buf + buflen, ed->pv, sizeof(buf) - buflen );

        buf[ sizeof(buf) - 3 ] = '\0';

        strcat( buf + buflen, "\r\n" );

        /* Update memo */
        InsertIntoMemo( ed->which, buf, InsertionPoint(strlen(buf), ed) );
        strncpy(lastLine[ed->which], buf, MSG_SIZ);
    }

    /* Colors */
    SetEngineColorIcon( ed->which );
}

// [HGM] kibitz: write kibitz line; split window for it if necessary
void
OutputKibitz (int window, char *text)
{
	static int currentLineEnd[2];
	int where = 0;
	if(!EngineOutputIsUp()) return;
	if(!opponentKibitzes) { // on first kibitz of game, clear memos
	    DoClearMemo(1); currentLineEnd[1] = 0;
	    if(gameMode == IcsObserving) { DoClearMemo(0); currentLineEnd[0] = 0; }
	}
	opponentKibitzes = TRUE; // this causes split window DisplayMode in ICS modes.
	VerifyDisplayMode();
	strncpy(text+strlen(text)-1, "\r\n",sizeof(text+strlen(text)-1)); // to not lose line breaks on copying
	if(gameMode == IcsObserving) {
	    DoSetWindowText(0, nLabel, gameInfo.white);
	    SetIcon( 0, nColorIcon,  nColorWhite);
	    SetIcon( 0, nStateIcon,  nClear);
	}
	DoSetWindowText(1, nLabel, gameMode == IcsPlayingBlack ? gameInfo.white : gameInfo.black); // opponent name
	SetIcon( 1, nColorIcon,  gameMode == IcsPlayingBlack ? nColorWhite : nColorBlack);
	SetIcon( 1, nStateIcon,  nClear);
	if(strstr(text, "\\  ") == text) where = currentLineEnd[window-1]; // continuation line
//if(appData.debugMode) fprintf(debugFP, "insert '%s' at %d (end = %d,%d)\n", text, where, currentLineEnd[0], currentLineEnd[1]);
	InsertIntoMemo(window-1, text, where); // [HGM] multivar: always at top
	currentLineEnd[window-1] = where + strlen(text);
}
