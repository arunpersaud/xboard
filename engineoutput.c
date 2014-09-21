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
#include <ctype.h>

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
static char columnHeader[MSG_SIZ] = "dep\tscore\tnodes\ttime\t(not shown:  tbhits\tknps\tseldep)\n";
static int  columnMask = 0xF0;

#define MAX_VAR 400
static int scores[MAX_VAR], textEnd[MAX_VAR], keys[MAX_VAR], curDepth[2], nrVariations[2];
static char fail[MAX_VAR];

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
        if(!appData.headers) columnHeader[0] = NULLCHAR;
        DoClearMemo(which); nrVariations[which] = 0;
        header[which][0] = NULLCHAR;
        if(gameMode == AnalyzeMode) {
          ChessProgramState *cps = (which ? &second : &first);
          if((multi = MultiPV(cps)) >= 0) {
            snprintf(header[which], MSG_SIZ, "\t%s viewpoint\t\tfewer / Multi-PV setting = %d / more\n",
                                       appData.whitePOV || appData.scoreWhite ? "white" : "mover", cps->option[multi].value);
	  }
          if(!which) snprintf(header[which]+strlen(header[which]), MSG_SIZ-strlen(header[which]), "%s%s", exclusionHeader, columnHeader);
          InsertIntoMemo( which, header[which], 0);
        } else {
          snprintf(header[which], MSG_SIZ, "%s", columnHeader);
          if(appData.ponderNextMove && lastLine[which][0]) {
            InsertIntoMemo( which, lastLine[which], 0 );
            InsertIntoMemo( which, "\n", 0 );
          }
          InsertIntoMemo( which, header[which], 0);
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
	char failType;

	if(ed->nodes == 0 && ed->score == 0 && ed->time == 0)
		newScore = 1e6; // info lines inserted on top
	if(ed->depth != curDepth[n]) { // depth has changed
		curDepth[n] = ed->depth;
		nrVariations[n] = 0; // throw away everything we had
	}
	i = strlen(ed->pv); if(i > 0) i--;
	failType = ed->pv[i];
	if(failType != '?' && failType != '!') failType = ' ';
	// loop through all lines. Note even / odd used for different panes
	for(i=nrVariations[n]-2; i>=0; i-=2) {
		// put new item behind those we haven't looked at
		offs = textEnd[i+n];
		textEnd[i+n+2] = offs + len;
		scores[i+n+2] = newScore;
		keys[i+n+2] = ed->moveKey;
		fail[i+n+2] = failType;
		if(ed->moveKey != keys[i+n] && // same move always tops previous one (as a higher score must be a fail low)
		   newScore < scores[i+n] && fail[i+n] == ' ') break;
		// if it had higher score as previous, move previous in stead
		scores[i+n+2] = ed->moveKey == keys[i+n] ? newScore : scores[i+n]; // correct scores of fail-low/high searches
		textEnd[i+n+2] = textEnd[i+n] + len;
		keys[i+n+2] = keys[i+n];
		fail[i+n+2] = fail[i+n];
	}
	if(i<0) {
		offs = 0;
		textEnd[n] = offs + len;
		scores[n] = newScore;
		keys[n] = ed->moveKey;
		fail[n] = failType;
	}
	nrVariations[n] += 2;
      return offs + strlen(header[ed->which]);
}

#define MATE_SCORE 100000
static char spaces[] = "            "; // [HGM] align: spaces for padding

static void
Format(char *buf, int val)
{ // [HGM] tbhits: print a positive integer with trailing whitespace to give it fixed width
        if( val < 1000000 ) {
            int h = val, i=0;
            while(h > 0) h /= 10, i++;
            snprintf( buf, 24, "%d%s\t", val, spaces + 2*i);
        }
        else {
            char unit = 'M';
            if(val >= 1e9) val /= 1e3, unit = 'G';
            snprintf( buf, 24, "%.*f%c%s\t", 1 + (val < 1e7), val/1e6, unit, spaces + 10 + 2*(val >= 1e8));
        }
}

// pure back end, now SetWindowText is called via wrapper DoSetWindowText
static void
UpdateControls (EngineOutputData *ed)
{
//    int isPondering = FALSE;

    char s_label[MAX_NAME_LENGTH + 32];
    int h;
    char * name = ed->name;
    char *q, *pvStart = ed->pv;

    /* Label */
    if( name == 0 || *name == '\0' ) {
        name = "?";
    }

    strncpy( s_label, name, MAX_NAME_LENGTH );
    s_label[ MAX_NAME_LENGTH-1 ] = '\0';

    if(pvStart) { // [HGM] tbhits: plit up old PV into extra infos and real PV
        while(strchr(pvStart, '\t')) { // locate last tab before non-int (real PV starts after that)
            for(q=pvStart; isdigit(*q) || *q == ' '; q++);
            if(*q != '\t') break;
            pvStart = q + 1;
        }
    }

#ifdef SHOW_PONDERING
    if( IsEnginePondering( ed->which ) ) {
        char buf[12];

        buf[0] = '\0';

        if( ed->hint != 0 && *ed->hint != '\0' ) {
            strncpy( buf, ed->hint, sizeof(buf) );
            buf[sizeof(buf)-1] = '\0';
        }
        else if( pvStart != 0 && *pvStart != '\0' ) {
            char * sep;
            int buflen = sizeof(buf);

            sep = strchr( pvStart, ' ' );
            if( sep != NULL ) {
                buflen = sep - pvStart + 1;
                if( buflen > sizeof(buf) ) buflen = sizeof(buf);
            }

            strncpy( buf, pvStart, buflen );
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
    if( pvStart != 0 && *pvStart != '\0' ) {
        char s_nodes[24];
        char s_score[16];
        char s_time[24];
        char s_hits[24];
        char s_seld[24];
        char s_knps[24];
        char buf[256], fail;
        int buflen, hits, i, params[5], extra;
        int time_secs = ed->time / 100;
        int time_cent = ed->time % 100;

        /* Nodes */
        if( ed->nodes < 1000000 ) {
            int h = ed->nodes, i=0;
            while(h > 0) h /= 10, i++; // [HGM] align: count digits; pad with 2 spaces for every missing digit
            snprintf( s_nodes, sizeof(s_nodes)/sizeof(s_nodes[0]), u64Display "%s\t", ed->nodes, spaces + 2*i);
        }
        else {
            double x = u64ToDouble(ed->nodes);
            char unit = 'M';
            if(x >= 1e9) x /= 1e3, unit = 'G';
            snprintf( s_nodes, sizeof(s_nodes)/sizeof(s_nodes[0]), "%.*f%c%s\t", 1 + (x < 1e7), x / 1e6,
                      unit, spaces + 10 + 2*(ed->nodes >= 1e8));
        }

        /* TB Hits etc. */
        for(i=hits=0; i<5; i++) params[i] = 0;
//fprintf(stderr, "%s\n%s\n", ed->pv, pvStart);
        if(pvStart != ed->pv) { // check if numbers before PV
            strncpy(buf, ed->pv, 256); buf[pvStart - ed->pv] = NULLCHAR;
            extra = sscanf(buf, "%d %d %d %d %d", params, params+1, params+2, params+3, params+4);
//fprintf(stderr, "extra=%d len=%d\n", extra, pvStart - ed->pv);
            if(extra) hits = params[extra-1], params[extra-1] = 0; // last one is tbhits
        }
        Format(s_seld, params[0]); Format(s_knps, params[1]); Format(s_hits, hits); 

        if(*ed->pv) fail = ed->pv[strlen(ed->pv)-1]; else fail = ' ';
	if(fail != '?' && fail != '!') fail = ' ';

        /* Score */
        h = ((gameMode == AnalyzeMode && appData.whitePOV || appData.scoreWhite) && !WhiteOnMove(currentMove) ? -1 : 1) * ed->score;
        if( h == 0 ) {
	  snprintf( s_score, sizeof(s_score)/sizeof(s_score[0]), "  0.00%c\t", fail );
        } else
	if( h >= MATE_SCORE) snprintf(s_score, 16, "  %s#%d%c\t", ( h > MATE_SCORE+9 ? "" : "  "),  h - MATE_SCORE, fail ); else
	if(-h >= MATE_SCORE) snprintf(s_score, 16, " %s#-%d%c\t", (-h > MATE_SCORE+9 ? "" : "  "), -h - MATE_SCORE, fail ); else
        if( h > 0 ) {
	  snprintf( s_score, sizeof(s_score)/sizeof(s_score[0]), "+%.2f%c\t", h / 100.0, fail );
        }
        else {
	  snprintf( s_score, sizeof(s_score)/sizeof(s_score[0]), " %.2f%c\t", h / 100.0, fail );
        }

        /* Time */
        if(time_secs >= 3600)
            snprintf( s_time, sizeof(s_time)/sizeof(s_time[0]), "%d:%02d:%02d\t", time_secs / 3600, (time_secs / 60) % 60, time_secs % 60 );
        else
        snprintf( s_time, sizeof(s_time)/sizeof(s_time[0]), "%d:%02d.%02d\t", time_secs / 60, time_secs % 60, time_cent );

        if(columnMask & 2) s_score[0] = NULLCHAR; // [HGM] hide: erase columns the user has hidden
        if(columnMask & 4) s_nodes[0] = NULLCHAR;
        if(columnMask & 8) s_time[0]  = NULLCHAR;
        if(columnMask & 16) s_hits[0]  = NULLCHAR;
        if(columnMask & 32) s_knps[0]  = NULLCHAR;
        if(columnMask & 64) s_seld[0]  = NULLCHAR;

        /* Put all together... */
	if(ed->nodes == 0 && ed->score == 0 && ed->time == 0)
	  snprintf( buf, sizeof(buf)/sizeof(buf[0]), "%3d\t", ed->depth );
	else
	  snprintf( buf, sizeof(buf)/sizeof(buf[0]), "%3d\t%s%s%s%s%s%s", ed->depth, s_score, s_nodes, s_time, s_hits, s_knps, s_seld );

        /* Add PV */
        buflen = strlen(buf);

        strncpy( buf + buflen, pvStart, sizeof(buf) - buflen );

        buf[ sizeof(buf) - 3 ] = '\0';

        strcat( buf + buflen, "\r\n" );

        /* Update memo */
        InsertIntoMemo( ed->which, buf, InsertionPoint(strlen(buf), ed) );
        strncpy(lastLine[ed->which], buf, MSG_SIZ);
    }

    /* Colors */
    SetEngineColorIcon( ed->which );
}

static char *titles[] = { "score\t", "nodes\t", "time\t", "tbhits\t", "knps\t", "seldep\t" };

void
Collapse(int n)
{   // handle click on column headers, to hide / show them
    int i, j, nr=0, m=~columnMask, Ncol=7;
    for(i=0; columnHeader[i] && i<n; i++) nr += (columnHeader[i] == '\t');
    if(!nr) return; // depth always shown, so clicks on it ignored
    for(i=j=0; i<Ncol; i++) if(m & 1<<i) j++; // count hidden columns
    if(nr < j) { // shown column clicked: hide it
	for(i=j=0; i<Ncol; i++) if(m & 1<<i && j++ == nr) break;
	columnMask |= 1<<i;
    } else { // hidden column clicked: show it
	m = ~m; nr -= j;
	for(i=j=0; i<Ncol; i++) if(m & 1<<i && j++ == nr) break;
	columnMask &= ~(1<<i);
    }
    // create new header line
    strcpy(columnHeader, "dep\t");
    m = ~columnMask;
    for(i=j=1; i<Ncol; i++) if(m & 1<<i) strcat(columnHeader, titles[i-1]), j++;
    if(j != Ncol) { // list hidden columns, so user ca click them
	m = ~m; strcat(columnHeader, "(not shown:  ");
	for(i=1; i<Ncol; i++) if(m & 1<<i) strcat(columnHeader, titles[i-1]);
	strcat(columnHeader, ")");
    }
    strcat(columnHeader, "\n");
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
