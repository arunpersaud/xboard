/*
 * backend.c -- Common back end for X and Windows NT versions of
 * XBoard $Id$
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts.
 * Enhancements Copyright 1992-2001 Free Software Foundation, Inc.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ------------------------------------------------------------------------
 *
 * See the file ChangeLog for a revision history.  */

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

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

#if HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#else /* not HAVE_SYS_FCNTL_H */
# if HAVE_FCNTL_H
#  include <fcntl.h>
# endif /* HAVE_FCNTL_H */
#endif /* not HAVE_SYS_FCNTL_H */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if defined(_amigados) && !defined(__GNUC__)
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
extern int gettimeofday(struct timeval *, struct timezone *);
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "parser.h"
#include "moves.h"
#if ZIPPY
# include "zippy.h"
#endif
#include "backendz.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


/* A point in time */
typedef struct {
    long sec;  /* Assuming this is >= 32 bits */
    int ms;    /* Assuming this is >= 16 bits */
} TimeMark;

/* Search stats from chessprogram */
typedef struct {
  char movelist[2*MSG_SIZ]; /* Last PV we were sent */
  int depth;              /* Current search depth */
  int nr_moves;           /* Total nr of root moves */
  int moves_left;         /* Moves remaining to be searched */
  char move_name[MOVE_LEN];  /* Current move being searched, if provided */
  u64 nodes;			  /* # of nodes searched */
  int time;               /* Search time (centiseconds) */
  int score;              /* Score (centipawns) */
  int got_only_move;      /* If last msg was "(only move)" */
  int got_fail;           /* 0 - nothing, 1 - got "--", 2 - got "++" */
  int ok_to_send;         /* handshaking between send & recv */
  int line_is_book;       /* 1 if movelist is book moves */
  int seen_stat;          /* 1 if we've seen the stat01: line */
} ChessProgramStats;

int establish P((void));
void read_from_player P((InputSourceRef isr, VOIDSTAR closure,
			 char *buf, int count, int error));
void read_from_ics P((InputSourceRef isr, VOIDSTAR closure,
		      char *buf, int count, int error));
void SendToICS P((char *s));
void SendToICSDelayed P((char *s, long msdelay));
void SendMoveToICS P((ChessMove moveType, int fromX, int fromY,
		      int toX, int toY));
void InitPosition P((int redraw));
void HandleMachineMove P((char *message, ChessProgramState *cps));
int AutoPlayOneMove P((void));
int LoadGameOneMove P((ChessMove readAhead));
int LoadGameFromFile P((char *filename, int n, char *title, int useList));
int LoadPositionFromFile P((char *filename, int n, char *title));
int SavePositionToFile P((char *filename));
void ApplyMove P((int fromX, int fromY, int toX, int toY, int promoChar,
		  Board board));
void MakeMove P((int fromX, int fromY, int toX, int toY, int promoChar));
void ShowMove P((int fromX, int fromY, int toX, int toY));
void FinishMove P((ChessMove moveType, int fromX, int fromY, int toX, int toY,
		   /*char*/int promoChar));
void BackwardInner P((int target));
void ForwardInner P((int target));
void GameEnds P((ChessMove result, char *resultDetails, int whosays));
void EditPositionDone P((void));
void PrintOpponents P((FILE *fp));
void PrintPosition P((FILE *fp, int move));
void StartChessProgram P((ChessProgramState *cps));
void SendToProgram P((char *message, ChessProgramState *cps));
void SendMoveToProgram P((int moveNum, ChessProgramState *cps));
void ReceiveFromProgram P((InputSourceRef isr, VOIDSTAR closure,
			   char *buf, int count, int error));
void SendTimeControl P((ChessProgramState *cps,
			int mps, long tc, int inc, int sd, int st));
char *TimeControlTagValue P((void));
void Attention P((ChessProgramState *cps));
void FeedMovesToProgram P((ChessProgramState *cps, int upto));
void ResurrectChessProgram P((void));
void DisplayComment P((int moveNumber, char *text));
void DisplayMove P((int moveNumber));
void DisplayAnalysis P((void));

void ParseGameHistory P((char *game));
void ParseBoard12 P((char *string));
void StartClocks P((void));
void SwitchClocks P((void));
void StopClocks P((void));
void ResetClocks P((void));
char *PGNDate P((void));
void SetGameInfo P((void));
Boolean ParseFEN P((Board board, int *blackPlaysFirst, char *fen));
int RegisterMove P((void));
void MakeRegisteredMove P((void));
void TruncateGame P((void));
int looking_at P((char *, int *, char *));
void CopyPlayerNameIntoFileName P((char **, char *));
char *SavePart P((char *));
int SaveGameOldStyle P((FILE *));
int SaveGamePGN P((FILE *));
void GetTimeMark P((TimeMark *));
long SubtractTimeMarks P((TimeMark *, TimeMark *));
int CheckFlags P((void));
long NextTickLength P((long));
void CheckTimeControl P((void));
void show_bytes P((FILE *, char *, int));
int string_to_rating P((char *str));
void ParseFeatures P((char* args, ChessProgramState *cps));
void InitBackEnd3 P((void));
void FeatureDone P((ChessProgramState* cps, int val));
void InitChessProgram P((ChessProgramState *cps));
double u64ToDouble P((u64 value));

#ifdef WIN32
	extern void ConsoleCreate();
#endif

extern int tinyLayout, smallLayout;
static ChessProgramStats programStats;

/* States for ics_getting_history */
#define H_FALSE 0
#define H_REQUESTED 1
#define H_GOT_REQ_HEADER 2
#define H_GOT_UNREQ_HEADER 3
#define H_GETTING_MOVES 4
#define H_GOT_UNWANTED_HEADER 5

/* whosays values for GameEnds */
#define GE_ICS 0
#define GE_ENGINE 1
#define GE_PLAYER 2
#define GE_FILE 3
#define GE_XBOARD 4

/* Maximum number of games in a cmail message */
#define CMAIL_MAX_GAMES 20

/* Different types of move when calling RegisterMove */
#define CMAIL_MOVE   0
#define CMAIL_RESIGN 1
#define CMAIL_DRAW   2
#define CMAIL_ACCEPT 3

/* Different types of result to remember for each game */
#define CMAIL_NOT_RESULT 0
#define CMAIL_OLD_RESULT 1
#define CMAIL_NEW_RESULT 2

/* Telnet protocol constants */
#define TN_WILL 0373
#define TN_WONT 0374
#define TN_DO   0375
#define TN_DONT 0376
#define TN_IAC  0377
#define TN_ECHO 0001
#define TN_SGA  0003
#define TN_PORT 23

/* Some compiler can't cast u64 to double
 * This function do the job for us:

 * We use the highest bit for cast, this only
 * works if the highest bit is not
 * in use (This should not happen)
 *
 * We used this for all compiler
 */
double
u64ToDouble(u64 value)
{
  double r;
  u64 tmp = value & 0x7fffffffffffffff;
  r = (double)(s64)tmp;
  if (value & 0x8000000000000000)
	r +=  9.2233720368547758080e18; /* 2^63 */
 return r;
}

/* Fake up flags for now, as we aren't keeping track of castling
   availability yet */
int
PosFlags(index)
{
  int flags = F_ALL_CASTLE_OK;
  if ((index % 2) == 0) flags |= F_WHITE_ON_MOVE;
  switch (gameInfo.variant) {
  case VariantSuicide:
  case VariantGiveaway:
    flags |= F_IGNORE_CHECK;
    flags &= ~F_ALL_CASTLE_OK;
    break;
  case VariantAtomic:
    flags |= F_IGNORE_CHECK | F_ATOMIC_CAPTURE;
    break;
  case VariantKriegspiel:
    flags |= F_KRIEGSPIEL_CAPTURE;
    break;
  case VariantNoCastle:
    flags &= ~F_ALL_CASTLE_OK;
    break;
  default:
    break;
  }
  return flags;
}

FILE *gameFileFP, *debugFP;

char cmailMove[CMAIL_MAX_GAMES][MOVE_LEN], cmailMsg[MSG_SIZ];
char bookOutput[MSG_SIZ*10], thinkOutput[MSG_SIZ*10], lastHint[MSG_SIZ];
char thinkOutput1[MSG_SIZ*10];

ChessProgramState first, second;

/* premove variables */
int premoveToX = 0;
int premoveToY = 0;
int premoveFromX = 0;
int premoveFromY = 0;
int premovePromoChar = 0;
int gotPremove = 0;
Boolean alarmSounded;
/* end premove variables */

char *ics_prefix = "$";
int ics_type = ICS_GENERIC;

int currentMove = 0, forwardMostMove = 0, backwardMostMove = 0;
int pauseExamForwardMostMove = 0;
int nCmailGames = 0, nCmailResults = 0, nCmailMovesRegistered = 0;
int cmailMoveRegistered[CMAIL_MAX_GAMES], cmailResult[CMAIL_MAX_GAMES];
int cmailMsgLoaded = FALSE, cmailMailedMove = FALSE;
int cmailOldMove = -1, firstMove = TRUE, flipView = FALSE;
int blackPlaysFirst = FALSE, startedFromSetupPosition = FALSE;
int searchTime = 0, pausing = FALSE, pauseExamInvalid = FALSE;
int whiteFlag = FALSE, blackFlag = FALSE;
int userOfferedDraw = FALSE;
int ics_user_moved = 0, ics_gamenum = -1, ics_getting_history = H_FALSE;
int matchMode = FALSE, hintRequested = FALSE, bookRequested = FALSE;
int cmailMoveType[CMAIL_MAX_GAMES];
long ics_clock_paused = 0;
ProcRef icsPR = NoProc, cmailPR = NoProc;
InputSourceRef telnetISR = NULL, fromUserISR = NULL, cmailISR = NULL;
GameMode gameMode = BeginningOfGame;
char moveList[MAX_MOVES][MOVE_LEN], parseList[MAX_MOVES][MOVE_LEN * 2];
char *commentList[MAX_MOVES], *cmailCommentList[CMAIL_MAX_GAMES];
char white_holding[64], black_holding[64];
TimeMark lastNodeCountTime;
long lastNodeCount=0;
int have_sent_ICS_logon = 0;
int movesPerSession;
long whiteTimeRemaining, blackTimeRemaining, timeControl, timeIncrement;
long timeRemaining[2][MAX_MOVES];
int matchGame = 0;
TimeMark programStartTime;
char ics_handle[MSG_SIZ];
int have_set_title = 0;

/* animateTraining preserves the state of appData.animate
 * when Training mode is activated. This allows the
 * response to be animated when appData.animate == TRUE and
 * appData.animateDragging == TRUE.
 */
Boolean animateTraining;

GameInfo gameInfo;

AppData appData;

Board boards[MAX_MOVES];
Board initialPosition = {
    { WhiteRook, WhiteKnight, WhiteBishop, WhiteQueen,
	WhiteKing, WhiteBishop, WhiteKnight, WhiteRook },
    { WhitePawn, WhitePawn, WhitePawn, WhitePawn,
	WhitePawn, WhitePawn, WhitePawn, WhitePawn },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { BlackPawn, BlackPawn, BlackPawn, BlackPawn,
	BlackPawn, BlackPawn, BlackPawn, BlackPawn },
    { BlackRook, BlackKnight, BlackBishop, BlackQueen,
	BlackKing, BlackBishop, BlackKnight, BlackRook }
};
Board twoKingsPosition = {
    { WhiteRook, WhiteKnight, WhiteBishop, WhiteQueen,
	WhiteKing, WhiteKing, WhiteKnight, WhiteRook },
    { WhitePawn, WhitePawn, WhitePawn, WhitePawn,
	WhitePawn, WhitePawn, WhitePawn, WhitePawn },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { EmptySquare, EmptySquare, EmptySquare, EmptySquare,
	EmptySquare, EmptySquare, EmptySquare, EmptySquare },
    { BlackPawn, BlackPawn, BlackPawn, BlackPawn,
	BlackPawn, BlackPawn, BlackPawn, BlackPawn },
    { BlackRook, BlackKnight, BlackBishop, BlackQueen,
	BlackKing, BlackKing, BlackKnight, BlackRook }
};


/* Convert str to a rating. Checks for special cases of "----",
   "++++", etc. Also strips ()'s */
int
string_to_rating(str)
  char *str;
{
  while(*str && !isdigit(*str)) ++str;
  if (!*str)
    return 0;	/* One of the special "no rating" cases */
  else
    return atoi(str);
}

void
ClearProgramStats()
{
    /* Init programStats */
    programStats.movelist[0] = 0;
    programStats.depth = 0;
    programStats.nr_moves = 0;
    programStats.moves_left = 0;
    programStats.nodes = 0;
    programStats.time = 100;
    programStats.score = 0;
    programStats.got_only_move = 0;
    programStats.got_fail = 0;
    programStats.line_is_book = 0;
}

void
InitBackEnd1()
{
    int matched, min, sec;

    GetTimeMark(&programStartTime);

    ClearProgramStats();
    programStats.ok_to_send = 1;
    programStats.seen_stat = 0;

    /*
     * Initialize game list
     */
    ListNew(&gameList);


    /*
     * Internet chess server status
     */
    if (appData.icsActive) {
	appData.matchMode = FALSE;
	appData.matchGames = 0;
#if ZIPPY
	appData.noChessProgram = !appData.zippyPlay;
#else
	appData.zippyPlay = FALSE;
	appData.zippyTalk = FALSE;
	appData.noChessProgram = TRUE;
#endif
	if (*appData.icsHelper != NULLCHAR) {
	    appData.useTelnet = TRUE;
	    appData.telnetProgram = appData.icsHelper;
	}
    } else {
	appData.zippyTalk = appData.zippyPlay = FALSE;
    }

    /*
     * Parse timeControl resource
     */
    if (!ParseTimeControl(appData.timeControl, appData.timeIncrement,
			  appData.movesPerSession)) {
	char buf[MSG_SIZ];
	sprintf(buf, _("bad timeControl option %s"), appData.timeControl);
	DisplayFatalError(buf, 0, 2);
    }

    /*
     * Parse searchTime resource
     */
    if (*appData.searchTime != NULLCHAR) {
	matched = sscanf(appData.searchTime, "%d:%d", &min, &sec);
	if (matched == 1) {
	    searchTime = min * 60;
	} else if (matched == 2) {
	    searchTime = min * 60 + sec;
	} else {
	    char buf[MSG_SIZ];
	    sprintf(buf, _("bad searchTime option %s"), appData.searchTime);
	    DisplayFatalError(buf, 0, 2);
	}
    }

    first.which = "first";
    second.which = "second";
    first.maybeThinking = second.maybeThinking = FALSE;
    first.pr = second.pr = NoProc;
    first.isr = second.isr = NULL;
    first.sendTime = second.sendTime = 2;
    first.sendDrawOffers = 1;
    if (appData.firstPlaysBlack) {
	first.twoMachinesColor = "black\n";
	second.twoMachinesColor = "white\n";
    } else {
	first.twoMachinesColor = "white\n";
	second.twoMachinesColor = "black\n";
    }
    first.program = appData.firstChessProgram;
    second.program = appData.secondChessProgram;
    first.host = appData.firstHost;
    second.host = appData.secondHost;
    first.dir = appData.firstDirectory;
    second.dir = appData.secondDirectory;
    first.other = &second;
    second.other = &first;
    first.initString = appData.initString;
    second.initString = appData.secondInitString;
    first.computerString = appData.firstComputerString;
    second.computerString = appData.secondComputerString;
    first.useSigint = second.useSigint = TRUE;
    first.useSigterm = second.useSigterm = TRUE;
    first.reuse = appData.reuseFirst;
    second.reuse = appData.reuseSecond;
    first.useSetboard = second.useSetboard = FALSE;
    first.useSAN = second.useSAN = FALSE;
    first.usePing = second.usePing = FALSE;
    first.lastPing = second.lastPing = 0;
    first.lastPong = second.lastPong = 0;
    first.usePlayother = second.usePlayother = FALSE;
    first.useColors = second.useColors = TRUE;
    first.useUsermove = second.useUsermove = FALSE;
    first.sendICS = second.sendICS = FALSE;
    first.sendName = second.sendName = appData.icsActive;
    first.sdKludge = second.sdKludge = FALSE;
    first.stKludge = second.stKludge = FALSE;
    TidyProgramName(first.program, first.host, first.tidy);
    TidyProgramName(second.program, second.host, second.tidy);
    first.matchWins = second.matchWins = 0;
    strcpy(first.variants, appData.variant);
    strcpy(second.variants, appData.variant);
    first.analysisSupport = second.analysisSupport = 2; /* detect */
    first.analyzing = second.analyzing = FALSE;
    first.initDone = second.initDone = FALSE;

    if (appData.firstProtocolVersion > PROTOVER ||
	appData.firstProtocolVersion < 1) {
      char buf[MSG_SIZ];
      sprintf(buf, _("protocol version %d not supported"),
	      appData.firstProtocolVersion);
      DisplayFatalError(buf, 0, 2);
    } else {
      first.protocolVersion = appData.firstProtocolVersion;
    }

    if (appData.secondProtocolVersion > PROTOVER ||
	appData.secondProtocolVersion < 1) {
      char buf[MSG_SIZ];
      sprintf(buf, _("protocol version %d not supported"),
	      appData.secondProtocolVersion);
      DisplayFatalError(buf, 0, 2);
    } else {
      second.protocolVersion = appData.secondProtocolVersion;
    }

    if (appData.icsActive) {
        appData.clockMode = TRUE;  /* changes dynamically in ICS mode */
    } else if (*appData.searchTime != NULLCHAR || appData.noChessProgram) {
	appData.clockMode = FALSE;
	first.sendTime = second.sendTime = 0;
    }

#if ZIPPY
    /* Override some settings from environment variables, for backward
       compatibility.  Unfortunately it's not feasible to have the env
       vars just set defaults, at least in xboard.  Ugh.
    */
    if (appData.icsActive && (appData.zippyPlay || appData.zippyTalk)) {
      ZippyInit();
    }
#endif

    if (appData.noChessProgram) {
	programVersion = (char*) malloc(5 + strlen(PRODUCT) + strlen(VERSION)
					+ strlen(PATCHLEVEL));
	sprintf(programVersion, "%s %s.%s", PRODUCT, VERSION, PATCHLEVEL);
    } else {
	char *p, *q;
	q = first.program;
	while (*q != ' ' && *q != NULLCHAR) q++;
	p = q;
	while (p > first.program && *(p-1) != '/') p--;
	programVersion = (char*) malloc(8 + strlen(PRODUCT) + strlen(VERSION)
					+ strlen(PATCHLEVEL) + (q - p));
	sprintf(programVersion, "%s %s.%s + ", PRODUCT, VERSION, PATCHLEVEL);
	strncat(programVersion, p, q - p);
    }

    if (!appData.icsActive) {
      char buf[MSG_SIZ];
      /* Check for variants that are supported only in ICS mode,
         or not at all.  Some that are accepted here nevertheless
         have bugs; see comments below.
      */
      VariantClass variant = StringToVariant(appData.variant);
      switch (variant) {
      case VariantBughouse:     /* need four players and two boards */
      case VariantKriegspiel:   /* need to hide pieces and move details */
      case VariantFischeRandom: /* castling doesn't work, shuffle not done */
	sprintf(buf, _("Variant %s supported only in ICS mode"), appData.variant);
	DisplayFatalError(buf, 0, 2);
	return;

      case VariantUnknown:
      case VariantLoadable:
      case Variant29:
      case Variant30:
      case Variant31:
      case Variant32:
      case Variant33:
      case Variant34:
      case Variant35:
      case Variant36:
      default:
	sprintf(buf, _("Unknown variant name %s"), appData.variant);
	DisplayFatalError(buf, 0, 2);
	return;

      case VariantNormal:     /* definitely works! */
      case VariantWildCastle: /* pieces not automatically shuffled */
      case VariantNoCastle:   /* pieces not automatically shuffled */
      case VariantCrazyhouse: /* holdings not shown,
			         offboard interposition not understood */
      case VariantLosers:     /* should work except for win condition,
			         and doesn't know captures are mandatory */
      case VariantSuicide:    /* should work except for win condition,
			         and doesn't know captures are mandatory */
      case VariantGiveaway:   /* should work except for win condition,
			         and doesn't know captures are mandatory */
      case VariantTwoKings:   /* should work */
      case VariantAtomic:     /* should work except for win condition */
      case Variant3Check:     /* should work except for win condition */
      case VariantShatranj:   /* might work if TestLegality is off */
	break;
      }
    }
}

int
ParseTimeControl(tc, ti, mps)
     char *tc;
     int ti;
     int mps;
{
    int matched, min, sec;

    matched = sscanf(tc, "%d:%d", &min, &sec);
    if (matched == 1) {
	timeControl = min * 60 * 1000;
    } else if (matched == 2) {
	timeControl = (min * 60 + sec) * 1000;
    } else {
	return FALSE;
    }

    if (ti >= 0) {
	timeIncrement = ti * 1000;  /* convert to ms */
	movesPerSession = 0;
    } else {
	timeIncrement = 0;
	movesPerSession = mps;
    }
    return TRUE;
}

void
InitBackEnd2()
{
    if (appData.debugMode) {
	fprintf(debugFP, "%s\n", programVersion);
    }

    if (appData.matchGames > 0) {
	appData.matchMode = TRUE;
    } else if (appData.matchMode) {
	appData.matchGames = 1;
    }
    Reset(TRUE, FALSE);
    if (appData.noChessProgram || first.protocolVersion == 1) {
      InitBackEnd3();
    } else {
      /* kludge: allow timeout for initial "feature" commands */
      FreezeUI();
      DisplayMessage("", "Starting chess program");
      ScheduleDelayedEvent(InitBackEnd3, FEATURE_TIMEOUT);
    }
}

void
InitBackEnd3 P((void))
{
    GameMode initialMode;
    char buf[MSG_SIZ];
    int err;

    InitChessProgram(&first);

    #ifdef WIN32
		/* Make a console window if needed */
		if (appData.icsActive) ConsoleCreate();
	#endif

    if (appData.icsActive) {
	err = establish();
	if (err != 0) {
	    if (*appData.icsCommPort != NULLCHAR) {
		sprintf(buf, _("Could not open comm port %s"),
			appData.icsCommPort);
	    } else {
		sprintf(buf, _("Could not connect to host %s, port %s"),
			appData.icsHost, appData.icsPort);
	    }
	    DisplayFatalError(buf, err, 1);
	    return;
	}
	SetICSMode();
	telnetISR =
	  AddInputSource(icsPR, FALSE, read_from_ics, &telnetISR);
	fromUserISR =
	  AddInputSource(NoProc, FALSE, read_from_player, &fromUserISR);
    } else if (appData.noChessProgram) {
	SetNCPMode();
    } else {
	SetGNUMode();
    }

    if (*appData.cmailGameName != NULLCHAR) {
	SetCmailMode();
	OpenLoopback(&cmailPR);
	cmailISR =
	  AddInputSource(cmailPR, FALSE, CmailSigHandlerCallBack, &cmailISR);
    }

    ThawUI();
    DisplayMessage("", "");
    if (StrCaseCmp(appData.initialMode, "") == 0) {
      initialMode = BeginningOfGame;
    } else if (StrCaseCmp(appData.initialMode, "TwoMachines") == 0) {
      initialMode = TwoMachinesPlay;
    } else if (StrCaseCmp(appData.initialMode, "AnalyzeFile") == 0) {
      initialMode = AnalyzeFile;
    } else if (StrCaseCmp(appData.initialMode, "Analysis") == 0) {
      initialMode = AnalyzeMode;
    } else if (StrCaseCmp(appData.initialMode, "MachineWhite") == 0) {
      initialMode = MachinePlaysWhite;
    } else if (StrCaseCmp(appData.initialMode, "MachineBlack") == 0) {
      initialMode = MachinePlaysBlack;
    } else if (StrCaseCmp(appData.initialMode, "EditGame") == 0) {
      initialMode = EditGame;
    } else if (StrCaseCmp(appData.initialMode, "EditPosition") == 0) {
      initialMode = EditPosition;
    } else if (StrCaseCmp(appData.initialMode, "Training") == 0) {
      initialMode = Training;
    } else {
      sprintf(buf, _("Unknown initialMode %s"), appData.initialMode);
      DisplayFatalError(buf, 0, 2);
      return;
    }

    if (appData.matchMode) {
	/* Set up machine vs. machine match */
	if (appData.noChessProgram) {
	    DisplayFatalError(_("Can't have a match with no chess programs"),
			      0, 2);
	    return;
	}
	matchMode = TRUE;
	matchGame = 1;
	if (*appData.loadGameFile != NULLCHAR) {
	    if (!LoadGameFromFile(appData.loadGameFile,
				  appData.loadGameIndex,
				  appData.loadGameFile, FALSE)) {
		DisplayFatalError(_("Bad game file"), 0, 1);
		return;
	    }
	} else if (*appData.loadPositionFile != NULLCHAR) {
	    if (!LoadPositionFromFile(appData.loadPositionFile,
				      appData.loadPositionIndex,
				      appData.loadPositionFile)) {
		DisplayFatalError(_("Bad position file"), 0, 1);
		return;
	    }
	}
	TwoMachinesEvent();
    } else if (*appData.cmailGameName != NULLCHAR) {
	/* Set up cmail mode */
	ReloadCmailMsgEvent(TRUE);
    } else {
	/* Set up other modes */
	if (initialMode == AnalyzeFile) {
	  if (*appData.loadGameFile == NULLCHAR) {
	    DisplayFatalError(_("AnalyzeFile mode requires a game file"), 0, 1);
	    return;
	  }
	}
	if (*appData.loadGameFile != NULLCHAR) {
	    (void) LoadGameFromFile(appData.loadGameFile,
				    appData.loadGameIndex,
				    appData.loadGameFile, TRUE);
	} else if (*appData.loadPositionFile != NULLCHAR) {
	    (void) LoadPositionFromFile(appData.loadPositionFile,
					appData.loadPositionIndex,
					appData.loadPositionFile);
	}
	if (initialMode == AnalyzeMode) {
	  if (appData.noChessProgram) {
	    DisplayFatalError(_("Analysis mode requires a chess engine"), 0, 2);
	    return;
	  }
	  if (appData.icsActive) {
	    DisplayFatalError(_("Analysis mode does not work with ICS mode"),0,2);
	    return;
	  }
	  AnalyzeModeEvent();
	} else if (initialMode == AnalyzeFile) {
	  ShowThinkingEvent(TRUE);
	  AnalyzeFileEvent();
	  AnalysisPeriodicEvent(1);
	} else if (initialMode == MachinePlaysWhite) {
	  if (appData.noChessProgram) {
	    DisplayFatalError(_("MachineWhite mode requires a chess engine"),
			      0, 2);
	    return;
	  }
	  if (appData.icsActive) {
	    DisplayFatalError(_("MachineWhite mode does not work with ICS mode"),
			      0, 2);
	    return;
	  }
	  MachineWhiteEvent();
	} else if (initialMode == MachinePlaysBlack) {
	  if (appData.noChessProgram) {
	    DisplayFatalError(_("MachineBlack mode requires a chess engine"),
			      0, 2);
	    return;
	  }
	  if (appData.icsActive) {
	    DisplayFatalError(_("MachineBlack mode does not work with ICS mode"),
			      0, 2);
	    return;
	  }
	  MachineBlackEvent();
	} else if (initialMode == TwoMachinesPlay) {
	  if (appData.noChessProgram) {
	    DisplayFatalError(_("TwoMachines mode requires a chess engine"),
			      0, 2);
	    return;
	  }
	  if (appData.icsActive) {
	    DisplayFatalError(_("TwoMachines mode does not work with ICS mode"),
			      0, 2);
	    return;
	  }
	  TwoMachinesEvent();
	} else if (initialMode == EditGame) {
	  EditGameEvent();
	} else if (initialMode == EditPosition) {
	  EditPositionEvent();
	} else if (initialMode == Training) {
	  if (*appData.loadGameFile == NULLCHAR) {
	    DisplayFatalError(_("Training mode requires a game file"), 0, 2);
	    return;
	  }
	  TrainingEvent();
	}
    }
}

/*
 * Establish will establish a contact to a remote host.port.
 * Sets icsPR to a ProcRef for a process (or pseudo-process)
 *  used to talk to the host.
 * Returns 0 if okay, error code if not.
 */
int
establish()
{
    char buf[MSG_SIZ];

    if (*appData.icsCommPort != NULLCHAR) {
	/* Talk to the host through a serial comm port */
	return OpenCommPort(appData.icsCommPort, &icsPR);

    } else if (*appData.gateway != NULLCHAR) {
	if (*appData.remoteShell == NULLCHAR) {
	    /* Use the rcmd protocol to run telnet program on a gateway host */
	    sprintf(buf, "%s %s %s",
		    appData.telnetProgram, appData.icsHost, appData.icsPort);
	    return OpenRcmd(appData.gateway, appData.remoteUser, buf, &icsPR);

	} else {
	    /* Use the rsh program to run telnet program on a gateway host */
	    if (*appData.remoteUser == NULLCHAR) {
		sprintf(buf, "%s %s %s %s %s", appData.remoteShell,
			appData.gateway, appData.telnetProgram,
			appData.icsHost, appData.icsPort);
	    } else {
		sprintf(buf, "%s %s -l %s %s %s %s",
			appData.remoteShell, appData.gateway,
			appData.remoteUser, appData.telnetProgram,
			appData.icsHost, appData.icsPort);
	    }
	    return StartChildProcess(buf, "", &icsPR);

	}
    } else if (appData.useTelnet) {
	return OpenTelnet(appData.icsHost, appData.icsPort, &icsPR);

    } else {
	/* TCP socket interface differs somewhat between
	   Unix and NT; handle details in the front end.
	   */
	return OpenTCP(appData.icsHost, appData.icsPort, &icsPR);
    }
}

void
show_bytes(fp, buf, count)
     FILE *fp;
     char *buf;
     int count;
{
    while (count--) {
	if (*buf < 040 || *(unsigned char *) buf > 0177) {
	    fprintf(fp, "\\%03o", *buf & 0xff);
	} else {
	    putc(*buf, fp);
	}
	buf++;
    }
    fflush(fp);
}

/* Returns an errno value */
int
OutputMaybeTelnet(pr, message, count, outError)
     ProcRef pr;
     char *message;
     int count;
     int *outError;
{
    char buf[8192], *p, *q, *buflim;
    int left, newcount, outcount;

    if (*appData.icsCommPort != NULLCHAR || appData.useTelnet ||
	*appData.gateway != NULLCHAR) {
	if (appData.debugMode) {
	    fprintf(debugFP, ">ICS: ");
	    show_bytes(debugFP, message, count);
	    fprintf(debugFP, "\n");
	}
	return OutputToProcess(pr, message, count, outError);
    }

    buflim = &buf[sizeof(buf)-1]; /* allow 1 byte for expanding last char */
    p = message;
    q = buf;
    left = count;
    newcount = 0;
    while (left) {
	if (q >= buflim) {
	    if (appData.debugMode) {
		fprintf(debugFP, ">ICS: ");
		show_bytes(debugFP, buf, newcount);
		fprintf(debugFP, "\n");
	    }
	    outcount = OutputToProcess(pr, buf, newcount, outError);
	    if (outcount < newcount) return -1; /* to be sure */
	    q = buf;
	    newcount = 0;
	}
	if (*p == '\n') {
	    *q++ = '\r';
	    newcount++;
	} else if (((unsigned char) *p) == TN_IAC) {
	    *q++ = (char) TN_IAC;
	    newcount ++;
	}
	*q++ = *p++;
	newcount++;
	left--;
    }
    if (appData.debugMode) {
	fprintf(debugFP, ">ICS: ");
	show_bytes(debugFP, buf, newcount);
	fprintf(debugFP, "\n");
    }
    outcount = OutputToProcess(pr, buf, newcount, outError);
    if (outcount < newcount) return -1; /* to be sure */
    return count;
}

void
read_from_player(isr, closure, message, count, error)
     InputSourceRef isr;
     VOIDSTAR closure;
     char *message;
     int count;
     int error;
{
    int outError, outCount;
    static int gotEof = 0;

    /* Pass data read from player on to ICS */
    if (count > 0) {
	gotEof = 0;
	outCount = OutputMaybeTelnet(icsPR, message, count, &outError);
	if (outCount < count) {
	    DisplayFatalError(_("Error writing to ICS"), outError, 1);
	}
    } else if (count < 0) {
	RemoveInputSource(isr);
	DisplayFatalError(_("Error reading from keyboard"), error, 1);
    } else if (gotEof++ > 0) {
	RemoveInputSource(isr);
	DisplayFatalError(_("Got end of file from keyboard"), 0, 0);
    }
}

void
SendToICS(s)
     char *s;
{
    int count, outCount, outError;

    if (icsPR == NULL) return;

    count = strlen(s);
    outCount = OutputMaybeTelnet(icsPR, s, count, &outError);
    if (outCount < count) {
	DisplayFatalError(_("Error writing to ICS"), outError, 1);
    }
}

/* This is used for sending logon scripts to the ICS. Sending
   without a delay causes problems when using timestamp on ICC
   (at least on my machine). */
void
SendToICSDelayed(s,msdelay)
     char *s;
     long msdelay;
{
    int count, outCount, outError;

    if (icsPR == NULL) return;

    count = strlen(s);
    if (appData.debugMode) {
	fprintf(debugFP, ">ICS: ");
	show_bytes(debugFP, s, count);
	fprintf(debugFP, "\n");
    }
    outCount = OutputToProcessDelayed(icsPR, s, count, &outError,
				      msdelay);
    if (outCount < count) {
	DisplayFatalError(_("Error writing to ICS"), outError, 1);
    }
}


/* Remove all highlighting escape sequences in s
   Also deletes any suffix starting with '('
   */
char *
StripHighlightAndTitle(s)
     char *s;
{
    static char retbuf[MSG_SIZ];
    char *p = retbuf;

    while (*s != NULLCHAR) {
	while (*s == '\033') {
	    while (*s != NULLCHAR && !isalpha(*s)) s++;
	    if (*s != NULLCHAR) s++;
	}
	while (*s != NULLCHAR && *s != '\033') {
	    if (*s == '(' || *s == '[') {
		*p = NULLCHAR;
		return retbuf;
	    }
	    *p++ = *s++;
	}
    }
    *p = NULLCHAR;
    return retbuf;
}

/* Remove all highlighting escape sequences in s */
char *
StripHighlight(s)
     char *s;
{
    static char retbuf[MSG_SIZ];
    char *p = retbuf;

    while (*s != NULLCHAR) {
	while (*s == '\033') {
	    while (*s != NULLCHAR && !isalpha(*s)) s++;
	    if (*s != NULLCHAR) s++;
	}
	while (*s != NULLCHAR && *s != '\033') {
	    *p++ = *s++;
	}
    }
    *p = NULLCHAR;
    return retbuf;
}

char *variantNames[] = VARIANT_NAMES;
char *
VariantName(v)
     VariantClass v;
{
    return variantNames[v];
}


/* Identify a variant from the strings the chess servers use or the
   PGN Variant tag names we use. */
VariantClass
StringToVariant(e)
     char *e;
{
    char *p;
    int wnum = -1;
    VariantClass v = VariantNormal;
    int i, found = FALSE;
    char buf[MSG_SIZ];

    if (!e) return v;

    for (i=0; i<sizeof(variantNames)/sizeof(char*); i++) {
      if (StrCaseStr(e, variantNames[i])) {
	v = (VariantClass) i;
	found = TRUE;
	break;
      }
    }

    if (!found) {
      if ((StrCaseStr(e, "fischer") && StrCaseStr(e, "random"))
	  || StrCaseStr(e, "wild/fr")) {
        v = VariantFischeRandom;
      } else if ((i = 4, p = StrCaseStr(e, "wild")) ||
		 (i = 1, p = StrCaseStr(e, "w"))) {
	p += i;
	while (*p && (isspace(*p) || *p == '(' || *p == '/')) p++;
	if (isdigit(*p)) {
	  wnum = atoi(p);
	} else {
	  wnum = -1;
	}
	switch (wnum) {
	case 0: /* FICS only, actually */
	case 1:
	  /* Castling legal even if K starts on d-file */
	  v = VariantWildCastle;
	  break;
	case 2:
	case 3:
	case 4:
	  /* Castling illegal even if K & R happen to start in
	     normal positions. */
	  v = VariantNoCastle;
	  break;
	case 5:
	case 7:
	case 8:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 18:
	case 19:
	  /* Castling legal iff K & R start in normal positions */
	  v = VariantNormal;
	  break;
	case 6:
	case 20:
	case 21:
	  /* Special wilds for position setup; unclear what to do here */
	  v = VariantLoadable;
	  break;
	case 9:
	  /* Bizarre ICC game */
	  v = VariantTwoKings;
	  break;
	case 16:
	  v = VariantKriegspiel;
	  break;
	case 17:
	  v = VariantLosers;
	  break;
	case 22:
	  v = VariantFischeRandom;
	  break;
	case 23:
	  v = VariantCrazyhouse;
	  break;
	case 24:
	  v = VariantBughouse;
	  break;
	case 25:
	  v = Variant3Check;
	  break;
	case 26:
	  /* Not quite the same as FICS suicide! */
	  v = VariantGiveaway;
	  break;
	case 27:
	  v = VariantAtomic;
	  break;
	case 28:
	  v = VariantShatranj;
	  break;

	/* Temporary names for future ICC types.  The name *will* change in
	   the next xboard/WinBoard release after ICC defines it. */
	case 29:
	  v = Variant29;
	  break;
	case 30:
	  v = Variant30;
	  break;
	case 31:
	  v = Variant31;
	  break;
	case 32:
	  v = Variant32;
	  break;
	case 33:
	  v = Variant33;
	  break;
	case 34:
	  v = Variant34;
	  break;
	case 35:
	  v = Variant35;
	  break;
	case 36:
	  v = Variant36;
	  break;

	case -1:
	  /* Found "wild" or "w" in the string but no number;
	     must assume it's normal chess. */
	  v = VariantNormal;
	  break;
	default:
	  sprintf(buf, "Unknown wild type %d", wnum);
	  DisplayError(buf, 0);
	  v = VariantUnknown;
	  break;
	}
      }
    }
    if (appData.debugMode) {
      fprintf(debugFP, _("recognized '%s' (%d) as variant %s\n"),
	      e, wnum, VariantName(v));
    }
    return v;
}

static int leftover_start = 0, leftover_len = 0;
char star_match[STAR_MATCH_N][MSG_SIZ];

/* Test whether pattern is present at &buf[*index]; if so, return TRUE,
   advance *index beyond it, and set leftover_start to the new value of
   *index; else return FALSE.  If pattern contains the character '*', it
   matches any sequence of characters not containing '\r', '\n', or the
   character following the '*' (if any), and the matched sequence(s) are
   copied into star_match.
   */
int
looking_at(buf, index, pattern)
     char *buf;
     int *index;
     char *pattern;
{
    char *bufp = &buf[*index], *patternp = pattern;
    int star_count = 0;
    char *matchp = star_match[0];

    for (;;) {
	if (*patternp == NULLCHAR) {
	    *index = leftover_start = bufp - buf;
	    *matchp = NULLCHAR;
	    return TRUE;
	}
	if (*bufp == NULLCHAR) return FALSE;
	if (*patternp == '*') {
	    if (*bufp == *(patternp + 1)) {
		*matchp = NULLCHAR;
		matchp = star_match[++star_count];
		patternp += 2;
		bufp++;
		continue;
	    } else if (*bufp == '\n' || *bufp == '\r') {
		patternp++;
		if (*patternp == NULLCHAR)
		  continue;
		else
		  return FALSE;
	    } else {
		*matchp++ = *bufp++;
		continue;
	    }
	}
	if (*patternp != *bufp) return FALSE;
	patternp++;
	bufp++;
    }
}

void
SendToPlayer(data, length)
     char *data;
     int length;
{
    int error, outCount;
    outCount = OutputToProcess(NoProc, data, length, &error);
    if (outCount < length) {
	DisplayFatalError(_("Error writing to display"), error, 1);
    }
}

void
PackHolding(packed, holding)
     char packed[];
     char *holding;
{
    char *p = holding;
    char *q = packed;
    int runlength = 0;
    int curr = 9999;
    do {
	if (*p == curr) {
	    runlength++;
	} else {
	    switch (runlength) {
	      case 0:
		break;
	      case 1:
		*q++ = curr;
		break;
	      case 2:
		*q++ = curr;
		*q++ = curr;
		break;
	      default:
		sprintf(q, "%d", runlength);
		while (*q) q++;
		*q++ = curr;
		break;
	    }
	    runlength = 1;
	    curr = *p;
	}
    } while (*p++);
    *q = NULLCHAR;
}

/* Telnet protocol requests from the front end */
void
TelnetRequest(ddww, option)
     unsigned char ddww, option;
{
    unsigned char msg[3];
    int outCount, outError;

    if (*appData.icsCommPort != NULLCHAR || appData.useTelnet) return;

    if (appData.debugMode) {
	char buf1[8], buf2[8], *ddwwStr, *optionStr;
	switch (ddww) {
	  case TN_DO:
	    ddwwStr = "DO";
	    break;
	  case TN_DONT:
	    ddwwStr = "DONT";
	    break;
	  case TN_WILL:
	    ddwwStr = "WILL";
	    break;
	  case TN_WONT:
	    ddwwStr = "WONT";
	    break;
	  default:
	    ddwwStr = buf1;
	    sprintf(buf1, "%d", ddww);
	    break;
	}
	switch (option) {
	  case TN_ECHO:
	    optionStr = "ECHO";
	    break;
	  default:
	    optionStr = buf2;
	    sprintf(buf2, "%d", option);
	    break;
	}
	fprintf(debugFP, ">%s %s ", ddwwStr, optionStr);
    }
    msg[0] = TN_IAC;
    msg[1] = ddww;
    msg[2] = option;
    outCount = OutputToProcess(icsPR, (char *)msg, 3, &outError);
    if (outCount < 3) {
	DisplayFatalError(_("Error writing to ICS"), outError, 1);
    }
}

void
DoEcho()
{
    if (!appData.icsActive) return;
    TelnetRequest(TN_DO, TN_ECHO);
}

void
DontEcho()
{
    if (!appData.icsActive) return;
    TelnetRequest(TN_DONT, TN_ECHO);
}

static int loggedOn = FALSE;

/*-- Game start info cache: --*/
int gs_gamenum;
char gs_kind[MSG_SIZ];
static char player1Name[128] = "";
static char player2Name[128] = "";
static int player1Rating = -1;
static int player2Rating = -1;
/*----------------------------*/

void
read_from_ics(isr, closure, data, count, error)
     InputSourceRef isr;
     VOIDSTAR closure;
     char *data;
     int count;
     int error;
{
#define BUF_SIZE 8192
#define STARTED_NONE 0
#define STARTED_MOVES 1
#define STARTED_BOARD 2
#define STARTED_OBSERVE 3
#define STARTED_HOLDINGS 4
#define STARTED_CHATTER 5
#define STARTED_COMMENT 6
#define STARTED_MOVES_NOHIDE 7

    static int started = STARTED_NONE;
    static char parse[20000];
    static int parse_pos = 0;
    static char buf[BUF_SIZE + 1];
    static int firstTime = TRUE, intfSet = FALSE;
    static ColorClass curColor = ColorNormal;
    static ColorClass prevColor = ColorNormal;
    static int savingComment = FALSE;
    char str[500];
    int i, oldi;
    int buf_len;
    int next_out;
    int tkind;
#ifdef WIN32
	/* For zippy color lines of winboard
	 * cleanup for gcc compiler */
	int backup;
#endif
    char *p;

#ifdef WIN32
    if (appData.debugMode) {
      if (!error) {
	fprintf(debugFP, "<ICS: ");
	show_bytes(debugFP, data, count);
	fprintf(debugFP, "\n");
      }
    }
#endif

    if (count > 0) {
	/* If last read ended with a partial line that we couldn't parse,
	   prepend it to the new read and try again. */
	if (leftover_len > 0) {
	    for (i=0; i<leftover_len; i++)
	      buf[i] = buf[leftover_start + i];
	}

	/* Copy in new characters, removing nulls and \r's */
	buf_len = leftover_len;
	for (i = 0; i < count; i++) {
	    if (data[i] != NULLCHAR && data[i] != '\r')
	      buf[buf_len++] = data[i];
	}

	buf[buf_len] = NULLCHAR;
	next_out = leftover_len;
	leftover_start = 0;

	i = 0;
	while (i < buf_len) {
	    /* Deal with part of the TELNET option negotiation
	       protocol.  We refuse to do anything beyond the
	       defaults, except that we allow the WILL ECHO option,
	       which ICS uses to turn off password echoing when we are
	       directly connected to it.  We reject this option
	       if localLineEditing mode is on (always on in xboard)
               and we are talking to port 23, which might be a real
	       telnet server that will try to keep WILL ECHO on permanently.
             */
	    if (buf_len - i >= 3 && (unsigned char) buf[i] == TN_IAC) {
		static int remoteEchoOption = FALSE; /* telnet ECHO option */
		unsigned char option;
		oldi = i;
		switch ((unsigned char) buf[++i]) {
		  case TN_WILL:
		    if (appData.debugMode)
		      fprintf(debugFP, "\n<WILL ");
		    switch (option = (unsigned char) buf[++i]) {
		      case TN_ECHO:
			if (appData.debugMode)
			  fprintf(debugFP, "ECHO ");
			/* Reply only if this is a change, according
			   to the protocol rules. */
			if (remoteEchoOption) break;
			if (appData.localLineEditing &&
			    atoi(appData.icsPort) == TN_PORT) {
			    TelnetRequest(TN_DONT, TN_ECHO);
			} else {
			    EchoOff();
			    TelnetRequest(TN_DO, TN_ECHO);
			    remoteEchoOption = TRUE;
			}
			break;
		      default:
			if (appData.debugMode)
			  fprintf(debugFP, "%d ", option);
			/* Whatever this is, we don't want it. */
			TelnetRequest(TN_DONT, option);
			break;
		    }
		    break;
		  case TN_WONT:
		    if (appData.debugMode)
		      fprintf(debugFP, "\n<WONT ");
		    switch (option = (unsigned char) buf[++i]) {
		      case TN_ECHO:
			if (appData.debugMode)
			  fprintf(debugFP, "ECHO ");
			/* Reply only if this is a change, according
			   to the protocol rules. */
			if (!remoteEchoOption) break;
			EchoOn();
			TelnetRequest(TN_DONT, TN_ECHO);
			remoteEchoOption = FALSE;
			break;
		      default:
			if (appData.debugMode)
			  fprintf(debugFP, "%d ", (unsigned char) option);
			/* Whatever this is, it must already be turned
			   off, because we never agree to turn on
			   anything non-default, so according to the
			   protocol rules, we don't reply. */
			break;
		    }
		    break;
		  case TN_DO:
		    if (appData.debugMode)
		      fprintf(debugFP, "\n<DO ");
		    switch (option = (unsigned char) buf[++i]) {
		      default:
			/* Whatever this is, we refuse to do it. */
			if (appData.debugMode)
			  fprintf(debugFP, "%d ", option);
			TelnetRequest(TN_WONT, option);
			break;
		    }
		    break;
		  case TN_DONT:
		    if (appData.debugMode)
		      fprintf(debugFP, "\n<DONT ");
		    switch (option = (unsigned char) buf[++i]) {
		      default:
			if (appData.debugMode)
			  fprintf(debugFP, "%d ", option);
			/* Whatever this is, we are already not doing
			   it, because we never agree to do anything
			   non-default, so according to the protocol
			   rules, we don't reply. */
			break;
		    }
		    break;
		  case TN_IAC:
		    if (appData.debugMode)
		      fprintf(debugFP, "\n<IAC ");
		    /* Doubled IAC; pass it through */
		    i--;
		    break;
		  default:
		    if (appData.debugMode)
		      fprintf(debugFP, "\n<%d ", (unsigned char) buf[i]);
		    /* Drop all other telnet commands on the floor */
		    break;
		}
		if (oldi > next_out)
		  SendToPlayer(&buf[next_out], oldi - next_out);
		if (++i > next_out)
		  next_out = i;
		continue;
	    }

	    /* OK, this at least will *usually* work */
	    if (!loggedOn && looking_at(buf, &i, "ics%")) {
		loggedOn = TRUE;
	    }

	    if (loggedOn && !intfSet) {
		if (ics_type == ICS_ICC) {
		  sprintf(str,
			  "/set-quietly interface %s\n/set-quietly style 12\n",
			  programVersion);

		} else if (ics_type == ICS_CHESSNET) {
		  sprintf(str, "/style 12\n");
		} else {
		  strcpy(str, "alias $ @\n$set interface ");
		  strcat(str, programVersion);
		  strcat(str, "\n$iset startpos 1\n$iset ms 1\n");
#ifdef WIN32
		  strcat(str, "$iset nohighlight 1\n");
#endif
		  strcat(str, "$iset lock 1\n$style 12\n");
		}
		SendToICS(str);
		intfSet = TRUE;
	    }

	    if (started == STARTED_COMMENT) {
		/* Accumulate characters in comment */
		parse[parse_pos++] = buf[i];
		if (buf[i] == '\n') {
		    parse[parse_pos] = NULLCHAR;
		    AppendComment(forwardMostMove, StripHighlight(parse));
		    started = STARTED_NONE;
		} else {
		    /* Don't match patterns against characters in chatter */
		    i++;
		    continue;
		}
	    }
	    if (started == STARTED_CHATTER) {
		if (buf[i] != '\n') {
		    /* Don't match patterns against characters in chatter */
		    i++;
		    continue;
		}
		started = STARTED_NONE;
	    }

            /* Kludge to deal with rcmd protocol */
	    if (firstTime && looking_at(buf, &i, "\001*")) {
		DisplayFatalError(&buf[1], 0, 1);
		continue;
	    } else {
	        firstTime = FALSE;
	    }

	    if (!loggedOn && looking_at(buf, &i, "chessclub.com")) {
		ics_type = ICS_ICC;
		ics_prefix = "/";
		if (appData.debugMode)
		  fprintf(debugFP, "ics_type %d\n", ics_type);
		continue;
	    }
	    if (!loggedOn && looking_at(buf, &i, "freechess.org")) {
		ics_type = ICS_FICS;
		ics_prefix = "$";
		if (appData.debugMode)
		  fprintf(debugFP, "ics_type %d\n", ics_type);
		continue;
	    }
	    if (!loggedOn && looking_at(buf, &i, "chess.net")) {
		ics_type = ICS_CHESSNET;
		ics_prefix = "/";
		if (appData.debugMode)
		  fprintf(debugFP, "ics_type %d\n", ics_type);
		continue;
	    }

	    if (!loggedOn &&
		(looking_at(buf, &i, "\"*\" is *a registered name") ||
		 looking_at(buf, &i, "Logging you in as \"*\"") ||
		 looking_at(buf, &i, "will be \"*\""))) {
	      strcpy(ics_handle, star_match[0]);
	      continue;
	    }

	    if (loggedOn && !have_set_title && ics_handle[0] != NULLCHAR) {
	      char buf[MSG_SIZ];
	      sprintf(buf, "%s@%s", ics_handle, appData.icsHost);
	      DisplayIcsInteractionTitle(buf);
	      have_set_title = TRUE;
	    }

	    /* skip finger notes */
	    if (started == STARTED_NONE &&
		((buf[i] == ' ' && isdigit(buf[i+1])) ||
		 (buf[i] == '1' && buf[i+1] == '0')) &&
		buf[i+2] == ':' && buf[i+3] == ' ') {
	      started = STARTED_CHATTER;
	      i += 3;
	      continue;
	    }

	    /* skip formula vars */
	    if (started == STARTED_NONE &&
		buf[i] == 'f' && isdigit(buf[i+1]) && buf[i+2] == ':') {
	      started = STARTED_CHATTER;
	      i += 3;
	      continue;
	    }

	    oldi = i;
	    if (appData.zippyTalk || appData.zippyPlay) {
#if ZIPPY
	#ifdef WIN32
		/* Backup address for color zippy lines */
		backup = i;
		if (loggedOn == TRUE)
			if (ZippyControl(buf, &backup) || ZippyConverse(buf, &backup) ||
				(appData.zippyPlay && ZippyMatch(buf, &backup)));
		#else
		if (ZippyControl(buf, &i) ||
		    ZippyConverse(buf, &i) ||
		    (appData.zippyPlay && ZippyMatch(buf, &i))) {
		    loggedOn = TRUE;
		    continue;
		}
	#endif
#endif
	    }
		if (/* Don't color "message" or "messages" output */
		    (tkind = 5, looking_at(buf, &i, "*. * (*:*): ")) ||
		    looking_at(buf, &i, "*. * at *:*: ") ||
		    looking_at(buf, &i, "--* (*:*): ") ||
		    /* Regular tells and says */
		    (tkind = 1, looking_at(buf, &i, "* tells you: ")) ||
		    looking_at(buf, &i, "* (your partner) tells you: ") ||
		    looking_at(buf, &i, "* says: ") ||
		    /* Message notifications (same color as tells) */
		    looking_at(buf, &i, "* has left a message ") ||
		    looking_at(buf, &i, "* just sent you a message:\n") ||
		    /* Whispers and kibitzes */
		    (tkind = 2, looking_at(buf, &i, "* whispers: ")) ||
 		    looking_at(buf, &i, "* kibitzes: ") ||
 		    /* Channel tells */
 		    (tkind = 3, looking_at(buf, &i, "*(*: "))) {

		  if (tkind == 1 && strchr(star_match[0], ':')) {
		      /* Avoid "tells you:" spoofs in channels */
		     tkind = 3;
		  }
		  if (star_match[0][0] == NULLCHAR ||
		      strchr(star_match[0], ' ') ||
		      (tkind == 3 && strchr(star_match[1], ' '))) {
		    /* Reject bogus matches */
		    i = oldi;
		  } else {
		    if (appData.colorize) {
		      if (oldi > next_out) {
			SendToPlayer(&buf[next_out], oldi - next_out);
			next_out = oldi;
		      }
		      switch (tkind) {
		      case 1:
			Colorize(ColorTell, FALSE);
			curColor = ColorTell;
			break;
		      case 2:
			Colorize(ColorKibitz, FALSE);
			curColor = ColorKibitz;
			break;
		      case 3:
			p = strrchr(star_match[1], '(');
			if (p == NULL) {
			  p = star_match[1];
			} else {
			  p++;
			}
			if (atoi(p) == 1) {
			  Colorize(ColorChannel1, FALSE);
			  curColor = ColorChannel1;
			} else {
			  Colorize(ColorChannel, FALSE);
			  curColor = ColorChannel;
			}
			break;
		      case 5:
			curColor = ColorNormal;
			break;
		      }
		    }
		    if (started == STARTED_NONE && appData.autoComment &&
			(gameMode == IcsObserving ||
			 gameMode == IcsPlayingWhite ||
			 gameMode == IcsPlayingBlack)) {
		      parse_pos = i - oldi;
		      memcpy(parse, &buf[oldi], parse_pos);
		      parse[parse_pos] = NULLCHAR;
		      started = STARTED_COMMENT;
		      savingComment = TRUE;
		    } else {
		      started = STARTED_CHATTER;
		      savingComment = FALSE;
		    }
		    loggedOn = TRUE;
		    continue;
		  }
		}

		if (looking_at(buf, &i, "* s-shouts: ") ||
		    looking_at(buf, &i, "* c-shouts: ")) {
		    if (appData.colorize) {
			if (oldi > next_out) {
			    SendToPlayer(&buf[next_out], oldi - next_out);
			    next_out = oldi;
			}
			Colorize(ColorSShout, FALSE);
			curColor = ColorSShout;
		    }
		    loggedOn = TRUE;
		    started = STARTED_CHATTER;
		    continue;
		}

		if (looking_at(buf, &i, "--->")) {
		    loggedOn = TRUE;
		    continue;
		}

		if (looking_at(buf, &i, "* shouts: ") ||
		    looking_at(buf, &i, "--> ")) {
		    if (appData.colorize) {
			if (oldi > next_out) {
			    SendToPlayer(&buf[next_out], oldi - next_out);
			    next_out = oldi;
			}
			Colorize(ColorShout, FALSE);
			curColor = ColorShout;
		    }
		    loggedOn = TRUE;
		    started = STARTED_CHATTER;
		    continue;
		}

		if (looking_at( buf, &i, "Challenge:")) {
		    if (appData.colorize) {
			if (oldi > next_out) {
			    SendToPlayer(&buf[next_out], oldi - next_out);
			    next_out = oldi;
			}
			Colorize(ColorChallenge, FALSE);
			curColor = ColorChallenge;
		    }
		    loggedOn = TRUE;
		    continue;
		}

		if (looking_at(buf, &i, "* offers you") ||
		    looking_at(buf, &i, "* offers to be") ||
		    looking_at(buf, &i, "* would like to") ||
		    looking_at(buf, &i, "* requests to") ||
		    looking_at(buf, &i, "Your opponent offers") ||
		    looking_at(buf, &i, "Your opponent requests")) {

		    if (appData.colorize) {
			if (oldi > next_out) {
			    SendToPlayer(&buf[next_out], oldi - next_out);
			    next_out = oldi;
			}
			Colorize(ColorRequest, FALSE);
			curColor = ColorRequest;
		    }
		    continue;
		}

		if (looking_at(buf, &i, "* (*) seeking")) {
		    if (appData.colorize) {
			if (oldi > next_out) {
			    SendToPlayer(&buf[next_out], oldi - next_out);
			    next_out = oldi;
			}
			Colorize(ColorSeek, FALSE);
			curColor = ColorSeek;
		    }
		    continue;
	    }

	    if (looking_at(buf, &i, "\\   ")) {
		if (prevColor != ColorNormal) {
		    if (oldi > next_out) {
			SendToPlayer(&buf[next_out], oldi - next_out);
			next_out = oldi;
		    }
		    Colorize(prevColor, TRUE);
		    curColor = prevColor;
		}
		if (savingComment) {
		    parse_pos = i - oldi;
		    memcpy(parse, &buf[oldi], parse_pos);
		    parse[parse_pos] = NULLCHAR;
		    started = STARTED_COMMENT;
		} else {
		    started = STARTED_CHATTER;
		}
		continue;
	    }

	    if (looking_at(buf, &i, "Black Strength :") ||
		looking_at(buf, &i, "<<< style 10 board >>>") ||
		looking_at(buf, &i, "<10>") ||
		looking_at(buf, &i, "#@#")) {
		/* Wrong board style */
		loggedOn = TRUE;
		SendToICS(ics_prefix);
		SendToICS("set style 12\n");
		SendToICS(ics_prefix);
    	        SendToICS("refresh\n");
		continue;
	    }

	    if (!have_sent_ICS_logon && looking_at(buf, &i, "login:")) {
		ICSInitScript();
		have_sent_ICS_logon = 1;
		continue;
	    }

	    if (ics_getting_history != H_GETTING_MOVES /*smpos kludge*/ &&
		(looking_at(buf, &i, "\n<12> ") ||
		 looking_at(buf, &i, "<12> "))) {
		loggedOn = TRUE;
		if (oldi > next_out) {
		    SendToPlayer(&buf[next_out], oldi - next_out);
		}
		next_out = i;
		started = STARTED_BOARD;
		parse_pos = 0;
		continue;
	    }

	    if ((started == STARTED_NONE && looking_at(buf, &i, "\n<b1> ")) ||
		looking_at(buf, &i, "<b1> ")) {
		if (oldi > next_out) {
		    SendToPlayer(&buf[next_out], oldi - next_out);
		}
		next_out = i;
		started = STARTED_HOLDINGS;
		parse_pos = 0;
		continue;
	    }

	    if (looking_at(buf, &i, "* *vs. * *--- *")) {
		loggedOn = TRUE;
		/* Header for a move list -- first line */

		switch (ics_getting_history) {
		  case H_FALSE:
		    switch (gameMode) {
		      case IcsIdle:
		      case BeginningOfGame:
			/* User typed "moves" or "oldmoves" while we
			   were idle.  Pretend we asked for these
			   moves and soak them up so user can step
			   through them and/or save them.
			   */
			Reset(FALSE, TRUE);
			gameMode = IcsObserving;
			ModeHighlight();
			ics_gamenum = -1;
			ics_getting_history = H_GOT_UNREQ_HEADER;
			break;
		      case EditGame: /*?*/
		      case EditPosition: /*?*/
			/* Should above feature work in these modes too? */
			/* For now it doesn't */
			ics_getting_history = H_GOT_UNWANTED_HEADER;
			break;
		      default:
			ics_getting_history = H_GOT_UNWANTED_HEADER;
			break;
		    }
		    break;
		  case H_REQUESTED:
		    /* Is this the right one? */
		    if (gameInfo.white && gameInfo.black &&
			strcmp(gameInfo.white, star_match[0]) == 0 &&
			strcmp(gameInfo.black, star_match[2]) == 0) {
			/* All is well */
			ics_getting_history = H_GOT_REQ_HEADER;
		    }
		    break;
		  case H_GOT_REQ_HEADER:
		  case H_GOT_UNREQ_HEADER:
		  case H_GOT_UNWANTED_HEADER:
		  case H_GETTING_MOVES:
		    /* Should not happen */
		    DisplayError(_("Error gathering move list: two headers"), 0);
		    ics_getting_history = H_FALSE;
		    break;
		}

		/* Save player ratings into gameInfo if needed */
		if ((ics_getting_history == H_GOT_REQ_HEADER ||
		     ics_getting_history == H_GOT_UNREQ_HEADER) &&
		    (gameInfo.whiteRating == -1 ||
		     gameInfo.blackRating == -1)) {

		    gameInfo.whiteRating = string_to_rating(star_match[1]);
		    gameInfo.blackRating = string_to_rating(star_match[3]);
		    if (appData.debugMode)
		      fprintf(debugFP, _("Ratings from header: W %d, B %d\n"),
			      gameInfo.whiteRating, gameInfo.blackRating);
		}
		continue;
	    }

	    if (looking_at(buf, &i,
	      "* * match, initial time: * minute*, increment: * second")) {
		/* Header for a move list -- second line */
		/* Initial board will follow if this is a wild game */

		if (gameInfo.event != NULL) free(gameInfo.event);
		sprintf(str, "ICS %s %s match", star_match[0], star_match[1]);
		gameInfo.event = StrSave(str);
		gameInfo.variant = StringToVariant(gameInfo.event);
		continue;
	    }

	    if (looking_at(buf, &i, "Move  ")) {
		/* Beginning of a move list */
		switch (ics_getting_history) {
		  case H_FALSE:
		    /* Normally should not happen */
		    /* Maybe user hit reset while we were parsing */
		    break;
		  case H_REQUESTED:
		    /* Happens if we are ignoring a move list that is not
		     * the one we just requested.  Common if the user
		     * tries to observe two games without turning off
		     * getMoveList */
		    break;
		  case H_GETTING_MOVES:
		    /* Should not happen */
		    DisplayError(_("Error gathering move list: nested"), 0);
		    ics_getting_history = H_FALSE;
		    break;
		  case H_GOT_REQ_HEADER:
		    ics_getting_history = H_GETTING_MOVES;
		    started = STARTED_MOVES;
		    parse_pos = 0;
		    if (oldi > next_out) {
			SendToPlayer(&buf[next_out], oldi - next_out);
		    }
		    break;
		  case H_GOT_UNREQ_HEADER:
		    ics_getting_history = H_GETTING_MOVES;
		    started = STARTED_MOVES_NOHIDE;
		    parse_pos = 0;
		    break;
		  case H_GOT_UNWANTED_HEADER:
		    ics_getting_history = H_FALSE;
		    break;
		}
		continue;
	    }

	    if (looking_at(buf, &i, "% ") ||
		((started == STARTED_MOVES || started == STARTED_MOVES_NOHIDE)
		 && looking_at(buf, &i, "}*"))) {
		savingComment = FALSE;
		switch (started) {
		  case STARTED_MOVES:
		  case STARTED_MOVES_NOHIDE:
		    memcpy(&parse[parse_pos], &buf[oldi], i - oldi);
		    parse[parse_pos + i - oldi] = NULLCHAR;
		    ParseGameHistory(parse);
#if ZIPPY
		    if (appData.zippyPlay && first.initDone) {
		        FeedMovesToProgram(&first, forwardMostMove);
			if (gameMode == IcsPlayingWhite) {
			    if (WhiteOnMove(forwardMostMove)) {
				if (first.sendTime) {
				  if (first.useColors) {
				    SendToProgram("black\n", &first);
				  }
				  SendTimeRemaining(&first, TRUE);
				}
				if (first.useColors) {
				  SendToProgram("white\ngo\n", &first);
				} else {
				  SendToProgram("go\n", &first);
				}
				first.maybeThinking = TRUE;
			    } else {
			        if (first.usePlayother) {
				  if (first.sendTime) {
				    SendTimeRemaining(&first, TRUE);
				  }
				  SendToProgram("playother\n", &first);
				  firstMove = FALSE;
			        } else {
				  firstMove = TRUE;
				}
			    }
			} else if (gameMode == IcsPlayingBlack) {
			    if (!WhiteOnMove(forwardMostMove)) {
				if (first.sendTime) {
				  if (first.useColors) {
				    SendToProgram("white\n", &first);
				  }
				  SendTimeRemaining(&first, FALSE);
				}
				if (first.useColors) {
				  SendToProgram("black\ngo\n", &first);
				} else {
				  SendToProgram("go\n", &first);
				}
				first.maybeThinking = TRUE;
			    } else {
			        if (first.usePlayother) {
				  if (first.sendTime) {
				    SendTimeRemaining(&first, FALSE);
				  }
				  SendToProgram("playother\n", &first);
				  firstMove = FALSE;
			        } else {
				  firstMove = TRUE;
				}
			    }
			}
		    }
#endif
		    if (gameMode == IcsObserving && ics_gamenum == -1) {
			/* Moves came from oldmoves or moves command
			   while we weren't doing anything else.
			   */
			currentMove = forwardMostMove;
			ClearHighlights();/*!!could figure this out*/
			flipView = appData.flipView;
			DrawPosition(FALSE, boards[currentMove]);
			DisplayBothClocks();
			sprintf(str, "%s vs. %s",
				gameInfo.white, gameInfo.black);
			DisplayTitle(str);
			gameMode = IcsIdle;
		    } else {
			/* Moves were history of an active game */
			if (gameInfo.resultDetails != NULL) {
			    free(gameInfo.resultDetails);
			    gameInfo.resultDetails = NULL;
			}
		    }
		    HistorySet(parseList, backwardMostMove,
			       forwardMostMove, currentMove-1);
		    DisplayMove(currentMove - 1);
		    if (started == STARTED_MOVES) next_out = i;
		    started = STARTED_NONE;
		    ics_getting_history = H_FALSE;
		    break;

		  case STARTED_OBSERVE:
		    started = STARTED_NONE;
		    SendToICS(ics_prefix);
		    SendToICS("refresh\n");
		    break;

		  default:
		    break;
		}
		continue;
	    }

	    if ((started == STARTED_MOVES || started == STARTED_BOARD ||
		 started == STARTED_HOLDINGS ||
		 started == STARTED_MOVES_NOHIDE) && i >= leftover_len) {
		/* Accumulate characters in move list or board */
		parse[parse_pos++] = buf[i];
	    }

	    /* Start of game messages.  Mostly we detect start of game
	       when the first board image arrives.  On some versions
	       of the ICS, though, we need to do a "refresh" after starting
	       to observe in order to get the current board right away. */
	    if (looking_at(buf, &i, "Adding game * to observation list")) {
		started = STARTED_OBSERVE;
		continue;
	    }

	    /* Handle auto-observe */
	    if (appData.autoObserve &&
		(gameMode == IcsIdle || gameMode == BeginningOfGame) &&
		looking_at(buf, &i, "Game notification: * (*) vs. * (*)")) {
		char *player;
		/* Choose the player that was highlighted, if any. */
		if (star_match[0][0] == '\033' ||
		    star_match[1][0] != '\033') {
		    player = star_match[0];
		} else {
		    player = star_match[2];
		}
		sprintf(str, "%sobserve %s\n",
			ics_prefix, StripHighlightAndTitle(player));
		SendToICS(str);

		/* Save ratings from notify string */
		strcpy(player1Name, star_match[0]);
		player1Rating = string_to_rating(star_match[1]);
		strcpy(player2Name, star_match[2]);
		player2Rating = string_to_rating(star_match[3]);

		if (appData.debugMode)
		  fprintf(debugFP,
			  "Ratings from 'Game notification:' %s %d, %s %d\n",
			  player1Name, player1Rating,
			  player2Name, player2Rating);

		continue;
	    }

	    /* Deal with automatic examine mode after a game,
	       and with IcsObserving -> IcsExamining transition */
	    if (looking_at(buf, &i, "Entering examine mode for game *") ||
		looking_at(buf, &i, "has made you an examiner of game *")) {

		int gamenum = atoi(star_match[0]);
		if ((gameMode == IcsIdle || gameMode == IcsObserving) &&
		    gamenum == ics_gamenum) {
		    /* We were already playing or observing this game;
		       no need to refetch history */
		    gameMode = IcsExamining;
		    if (pausing) {
			pauseExamForwardMostMove = forwardMostMove;
		    } else if (currentMove < forwardMostMove) {
			ForwardInner(forwardMostMove);
		    }
		} else {
		    /* I don't think this case really can happen */
		    SendToICS(ics_prefix);
		    SendToICS("refresh\n");
		}
		continue;
	    }

	    /* Error messages */
	    if (ics_user_moved) {
		if (looking_at(buf, &i, "Illegal move") ||
		    looking_at(buf, &i, "Not a legal move") ||
		    looking_at(buf, &i, "Your king is in check") ||
		    looking_at(buf, &i, "It isn't your turn") ||
		    looking_at(buf, &i, "It is not your move")) {
		    /* Illegal move */
		    ics_user_moved = 0;
		    if (forwardMostMove > backwardMostMove) {
			currentMove = --forwardMostMove;
			DisplayMove(currentMove - 1); /* before DMError */
			DisplayMoveError("Illegal move (rejected by ICS)");
			DrawPosition(FALSE, boards[currentMove]);
			SwitchClocks();
			DisplayBothClocks();
		    }
		    continue;
		}
	    }

	    if (looking_at(buf, &i, "still have time") ||
		looking_at(buf, &i, "not out of time") ||
		looking_at(buf, &i, "either player is out of time") ||
		looking_at(buf, &i, "has timeseal; checking")) {
		/* We must have called his flag a little too soon */
		whiteFlag = blackFlag = FALSE;
		continue;
	    }

	    if (looking_at(buf, &i, "added * seconds to") ||
		looking_at(buf, &i, "seconds were added to")) {
		/* Update the clocks */
		SendToICS(ics_prefix);
		SendToICS("refresh\n");
		continue;
	    }

	    if (!ics_clock_paused && looking_at(buf, &i, "clock paused")) {
		ics_clock_paused = TRUE;
		StopClocks();
		continue;
	    }

	    if (ics_clock_paused && looking_at(buf, &i, "clock resumed")) {
		ics_clock_paused = FALSE;
		StartClocks();
		continue;
	    }

	    /* Grab player ratings from the Creating: message.
	       Note we have to check for the special case when
	       the ICS inserts things like [white] or [black]. */
	    if (looking_at(buf, &i, "Creating: * (*)* * (*)") ||
		looking_at(buf, &i, "Creating: * (*) [*] * (*)")) {
		/* star_matches:
		   0    player 1 name (not necessarily white)
		   1	player 1 rating
		   2	empty, white, or black (IGNORED)
		   3	player 2 name (not necessarily black)
		   4    player 2 rating

		   The names/ratings are sorted out when the game
		   actually starts (below).
		*/
		strcpy(player1Name, StripHighlightAndTitle(star_match[0]));
		player1Rating = string_to_rating(star_match[1]);
		strcpy(player2Name, StripHighlightAndTitle(star_match[3]));
		player2Rating = string_to_rating(star_match[4]);

		if (appData.debugMode)
		  fprintf(debugFP,
			  "Ratings from 'Creating:' %s %d, %s %d\n",
			  player1Name, player1Rating,
			  player2Name, player2Rating);

		continue;
	    }

	    /* Improved generic start/end-of-game messages */
	    if ((tkind=0, looking_at(buf, &i, "{Game * (* vs. *) *}*")) ||
		(tkind=1, looking_at(buf, &i, "{Game * (*(*) vs. *(*)) *}*"))){
	        /* If tkind == 0: */
		/* star_match[0] is the game number */
		/*           [1] is the white player's name */
		/*           [2] is the black player's name */
		/* For end-of-game: */
		/*           [3] is the reason for the game end */
		/*           [4] is a PGN end game-token, preceded by " " */
		/* For start-of-game: */
		/*           [3] begins with "Creating" or "Continuing" */
		/*           [4] is " *" or empty (don't care). */
		int gamenum = atoi(star_match[0]);
		char *whitename, *blackname, *why, *endtoken;
		ChessMove endtype = (ChessMove) 0;

		if (tkind == 0) {
		  whitename = star_match[1];
		  blackname = star_match[2];
		  why = star_match[3];
		  endtoken = star_match[4];
		} else {
		  whitename = star_match[1];
		  blackname = star_match[3];
		  why = star_match[5];
		  endtoken = star_match[6];
		}

                /* Game start messages */
		if (strncmp(why, "Creating ", 9) == 0 ||
		    strncmp(why, "Continuing ", 11) == 0) {
		    gs_gamenum = gamenum;
		    strcpy(gs_kind, strchr(why, ' ') + 1);
#if ZIPPY
		    if (appData.zippyPlay) {
			ZippyGameStart(whitename, blackname);
		    }
#endif /*ZIPPY*/
		    continue;
		}

		/* Game end messages */
		if (gameMode == IcsIdle || gameMode == BeginningOfGame ||
		    ics_gamenum != gamenum) {
		    continue;
		}
		while (endtoken[0] == ' ') endtoken++;
		switch (endtoken[0]) {
		  case '*':
		  default:
		    endtype = GameUnfinished;
		    break;
		  case '0':
		    endtype = BlackWins;
		    break;
		  case '1':
		    if (endtoken[1] == '/')
		      endtype = GameIsDrawn;
		    else
		      endtype = WhiteWins;
		    break;
		}
		GameEnds(endtype, why, GE_ICS);
#if ZIPPY
		if (appData.zippyPlay && first.initDone) {
		    ZippyGameEnd(endtype, why);
		    if (first.pr == NULL) {
		      /* Start the next process early so that we'll
			 be ready for the next challenge */
		      StartChessProgram(&first);
		    }
		    /* Send "new" early, in case this command takes
		       a long time to finish, so that we'll be ready
		       for the next challenge. */
		    Reset(TRUE, TRUE);
		}
#endif /*ZIPPY*/
		continue;
	    }

	    if (looking_at(buf, &i, "Removing game * from observation") ||
		looking_at(buf, &i, "no longer observing game *") ||
		looking_at(buf, &i, "Game * (*) has no examiners")) {
		if (gameMode == IcsObserving &&
		    atoi(star_match[0]) == ics_gamenum)
		  {
			  /* icsEngineAnalyze */
			  if (appData.icsEngineAnalyze) {
			     ExitAnalyzeMode();
				 ModeHighlight();
			  }
		      StopClocks();
		      gameMode = IcsIdle;
		      ics_gamenum = -1;
		      ics_user_moved = FALSE;
		  }
		continue;
	    }

	    if (looking_at(buf, &i, "no longer examining game *")) {
		if (gameMode == IcsExamining &&
		    atoi(star_match[0]) == ics_gamenum)
		  {
		      gameMode = IcsIdle;
		      ics_gamenum = -1;
		      ics_user_moved = FALSE;
		  }
		continue;
	    }

	    /* Advance leftover_start past any newlines we find,
	       so only partial lines can get reparsed */
	    if (looking_at(buf, &i, "\n")) {
		prevColor = curColor;
		if (curColor != ColorNormal) {
		    if (oldi > next_out) {
			SendToPlayer(&buf[next_out], oldi - next_out);
			next_out = oldi;
		    }
		    Colorize(ColorNormal, FALSE);
		    curColor = ColorNormal;
		}
		if (started == STARTED_BOARD) {
		    started = STARTED_NONE;
		    parse[parse_pos] = NULLCHAR;
		    ParseBoard12(parse);
		    ics_user_moved = 0;

		    /* Send premove here */
		    if (appData.premove) {
		      char str[MSG_SIZ];
		      if (currentMove == 0 &&
			  gameMode == IcsPlayingWhite &&
			  appData.premoveWhite) {
			sprintf(str, "%s%s\n", ics_prefix,
				appData.premoveWhiteText);
			if (appData.debugMode)
			  fprintf(debugFP, "Sending premove:\n");
			SendToICS(str);
		      } else if (currentMove == 1 &&
				 gameMode == IcsPlayingBlack &&
				 appData.premoveBlack) {
			sprintf(str, "%s%s\n", ics_prefix,
				appData.premoveBlackText);
			if (appData.debugMode)
			  fprintf(debugFP, "Sending premove:\n");
			SendToICS(str);
		      } else if (gotPremove) {
			gotPremove = 0;
			ClearPremoveHighlights();
			if (appData.debugMode)
			  fprintf(debugFP, "Sending premove:\n");
			  UserMoveEvent(premoveFromX, premoveFromY,
				        premoveToX, premoveToY,
				        premovePromoChar);
		      }
		    }

		    /* Usually suppress following prompt */
		    if (!(forwardMostMove == 0 && gameMode == IcsExamining)) {
			if (looking_at(buf, &i, "*% ")) {
			    savingComment = FALSE;
			}
		    }
		    next_out = i;
		} else if (started == STARTED_HOLDINGS) {
		    int gamenum;
		    char new_piece[MSG_SIZ];
		    started = STARTED_NONE;
		    parse[parse_pos] = NULLCHAR;
		    if (appData.debugMode)
		      fprintf(debugFP, "Parsing holdings: %s\n", parse);
		    if (sscanf(parse, " game %d", &gamenum) == 1 &&
			gamenum == ics_gamenum) {
		        if (gameInfo.variant == VariantNormal) {
			  gameInfo.variant = VariantCrazyhouse; /*temp guess*/
			  /* Get a move list just to see the header, which
			     will tell us whether this is really bug or zh */
			  if (ics_getting_history == H_FALSE) {
			    ics_getting_history = H_REQUESTED;
			    sprintf(str, "%smoves %d\n", ics_prefix, gamenum);
			    SendToICS(str);
			  }
			}
			new_piece[0] = NULLCHAR;
			sscanf(parse, "game %d white [%s black [%s <- %s",
			       &gamenum, white_holding, black_holding,
			       new_piece);
                        white_holding[strlen(white_holding)-1] = NULLCHAR;
                        black_holding[strlen(black_holding)-1] = NULLCHAR;
#if ZIPPY
			if (appData.zippyPlay && first.initDone) {
			    ZippyHoldings(white_holding, black_holding,
					  new_piece);
			}
#endif /*ZIPPY*/
			if (tinyLayout || smallLayout) {
			    char wh[16], bh[16];
			    PackHolding(wh, white_holding);
			    PackHolding(bh, black_holding);
			    sprintf(str, "[%s-%s] %s-%s", wh, bh,
				    gameInfo.white, gameInfo.black);
			} else {
			    sprintf(str, "%s [%s] vs. %s [%s]",
				    gameInfo.white, white_holding,
				    gameInfo.black, black_holding);
			}
			DrawPosition(FALSE, NULL);
			DisplayTitle(str);
		    }
		    /* Suppress following prompt */
		    if (looking_at(buf, &i, "*% ")) {
			savingComment = FALSE;
		    }
		    next_out = i;
		}
		continue;
	    }

	    i++;		/* skip unparsed character and loop back */
	}

	if (started != STARTED_MOVES && started != STARTED_BOARD &&
	    started != STARTED_HOLDINGS && i > next_out) {
	    SendToPlayer(&buf[next_out], i - next_out);
	    next_out = i;
	}

	leftover_len = buf_len - leftover_start;
	/* if buffer ends with something we couldn't parse,
	   reparse it after appending the next read */

    } else if (count == 0) {
	RemoveInputSource(isr);
        DisplayFatalError(_("Connection closed by ICS"), 0, 0);
    } else {
	DisplayFatalError(_("Error reading from ICS"), error, 1);
    }
}


/* Board style 12 looks like this:

   <12> r-b---k- pp----pp ---bP--- ---p---- q------- ------P- P--Q--BP -----R-K W -1 0 0 0 0 0 0 paf MaxII 0 2 12 21 25 234 174 24 Q/d7-a4 (0:06) Qxa4 0 0

 * The "<12> " is stripped before it gets to this routine.  The two
 * trailing 0's (flip state and clock ticking) are later addition, and
 * some chess servers may not have them, or may have only the first.
 * Additional trailing fields may be added in the future.
 */

#define PATTERN "%72c%c%d%d%d%d%d%d%d%s%s%d%d%d%d%d%d%d%d%s%s%s%d%d"

#define RELATION_OBSERVING_PLAYED    0
#define RELATION_OBSERVING_STATIC   -2   /* examined, oldmoves, or smoves */
#define RELATION_PLAYING_MYMOVE      1
#define RELATION_PLAYING_NOTMYMOVE  -1
#define RELATION_EXAMINING           2
#define RELATION_ISOLATED_BOARD     -3
#define RELATION_STARTING_POSITION  -4   /* FICS only */

void
ParseBoard12(string)
     char *string;
{
    GameMode newGameMode;
    int gamenum, newGame, newMove, relation, basetime, increment, ics_flip = 0, i;
    int j, k, n, moveNum, white_stren, black_stren, white_time, black_time, takeback;
    int double_push, castle_ws, castle_wl, castle_bs, castle_bl, irrev_count;
    char to_play, board_chars[72];
    char move_str[500], str[500], elapsed_time[500];
    char black[32], white[32];
    Board board;
    int prevMove = currentMove;
    int ticking = 2;
    ChessMove moveType;
    int fromX, fromY, toX, toY;
    char promoChar;

    fromX = fromY = toX = toY = -1;

    newGame = FALSE;

    if (appData.debugMode)
      fprintf(debugFP, _("Parsing board: %s\n"), string);

    move_str[0] = NULLCHAR;
    elapsed_time[0] = NULLCHAR;
    n = sscanf(string, PATTERN, board_chars, &to_play, &double_push,
	       &castle_ws, &castle_wl, &castle_bs, &castle_bl, &irrev_count,
	       &gamenum, white, black, &relation, &basetime, &increment,
	       &white_stren, &black_stren, &white_time, &black_time,
	       &moveNum, str, elapsed_time, move_str, &ics_flip,
	       &ticking);

    if (n < 22) {
	sprintf(str, _("Failed to parse board string:\n\"%s\""), string);
	DisplayError(str, 0);
	return;
    }

    /* Convert the move number to internal form */
    moveNum = (moveNum - 1) * 2;
    if (to_play == 'B') moveNum++;
    if (moveNum >= MAX_MOVES) {
      DisplayFatalError(_("Game too long; increase MAX_MOVES and recompile"),
			0, 1);
      return;
    }

    switch (relation) {
      case RELATION_OBSERVING_PLAYED:
      case RELATION_OBSERVING_STATIC:
	if (gamenum == -1) {
	    /* Old ICC buglet */
	    relation = RELATION_OBSERVING_STATIC;
	}
	newGameMode = IcsObserving;
	break;
      case RELATION_PLAYING_MYMOVE:
      case RELATION_PLAYING_NOTMYMOVE:
	newGameMode =
	  ((relation == RELATION_PLAYING_MYMOVE) == (to_play == 'W')) ?
	    IcsPlayingWhite : IcsPlayingBlack;
	break;
      case RELATION_EXAMINING:
	newGameMode = IcsExamining;
	break;
      case RELATION_ISOLATED_BOARD:
      default:
	/* Just display this board.  If user was doing something else,
	   we will forget about it until the next board comes. */
	newGameMode = IcsIdle;
	break;
      case RELATION_STARTING_POSITION:
	newGameMode = gameMode;
	break;
    }

    /* Modify behavior for initial board display on move listing
       of wild games.
       */
    switch (ics_getting_history) {
      case H_FALSE:
      case H_REQUESTED:
	break;
      case H_GOT_REQ_HEADER:
      case H_GOT_UNREQ_HEADER:
	/* This is the initial position of the current game */
	gamenum = ics_gamenum;
	moveNum = 0;		/* old ICS bug workaround */
 	if (to_play == 'B') {
	  startedFromSetupPosition = TRUE;
	  blackPlaysFirst = TRUE;
	  moveNum = 1;
	  if (forwardMostMove == 0) forwardMostMove = 1;
	  if (backwardMostMove == 0) backwardMostMove = 1;
	  if (currentMove == 0) currentMove = 1;
	}
	newGameMode = gameMode;
	relation = RELATION_STARTING_POSITION; /* ICC needs this */
	break;
      case H_GOT_UNWANTED_HEADER:
	/* This is an initial board that we don't want */
	return;
      case H_GETTING_MOVES:
	/* Should not happen */
	DisplayError(_("Error gathering move list: extra board"), 0);
	ics_getting_history = H_FALSE;
	return;
    }

    /* Take action if this is the first board of a new game, or of a
       different game than is currently being displayed.  */
    if (gamenum != ics_gamenum || newGameMode != gameMode ||
	relation == RELATION_ISOLATED_BOARD) {

	/* Forget the old game and get the history (if any) of the new one */
	if (gameMode != BeginningOfGame) {
       Reset(FALSE, TRUE);
	}
	newGame = TRUE;
	if (appData.autoRaiseBoard) BoardToTop();
	prevMove = -3;
	if (gamenum == -1) {
	    newGameMode = IcsIdle;
	} else if (moveNum > 0 && newGameMode != IcsIdle &&
		   appData.getMoveList) {
	    /* Need to get game history */
	    ics_getting_history = H_REQUESTED;
	    sprintf(str, "%smoves %d\n", ics_prefix, gamenum);
	    SendToICS(str);
	}

	/* Initially flip the board to have black on the bottom if playing
	   black or if the ICS flip flag is set, but let the user change
	   it with the Flip View button. */
	flipView = appData.autoFlipView ?
	  (newGameMode == IcsPlayingBlack) || ics_flip :
	  appData.flipView;

	/* Done with values from previous mode; copy in new ones */
	gameMode = newGameMode;
	ModeHighlight();
	ics_gamenum = gamenum;
	if (gamenum == gs_gamenum) {
	    int klen = strlen(gs_kind);
	    if (gs_kind[klen - 1] == '.') gs_kind[klen - 1] = NULLCHAR;
	    sprintf(str, "ICS %s", gs_kind);
	    gameInfo.event = StrSave(str);
	} else {
	    gameInfo.event = StrSave("ICS game");
	}
	gameInfo.site = StrSave(appData.icsHost);
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave(white);
	gameInfo.black = StrSave(black);
	timeControl = basetime * 60 * 1000;
	timeIncrement = increment * 1000;
	movesPerSession = 0;
	gameInfo.timeControl = TimeControlTagValue();
	gameInfo.variant = StringToVariant(gameInfo.event);

	/* Do we have the ratings? */
	if (strcmp(player1Name, white) == 0 &&
	    strcmp(player2Name, black) == 0) {
	    if (appData.debugMode)
	      fprintf(debugFP, "Remembered ratings: W %d, B %d\n",
		      player1Rating, player2Rating);
	    gameInfo.whiteRating = player1Rating;
	    gameInfo.blackRating = player2Rating;
	} else if (strcmp(player2Name, white) == 0 &&
		   strcmp(player1Name, black) == 0) {
	    if (appData.debugMode)
	      fprintf(debugFP, "Remembered ratings: W %d, B %d\n",
		      player2Rating, player1Rating);
	    gameInfo.whiteRating = player2Rating;
	    gameInfo.blackRating = player1Rating;
	}
	player1Name[0] = player2Name[0] = NULLCHAR;

	/* Silence shouts if requested */
	if (appData.quietPlay &&
	    (gameMode == IcsPlayingWhite || gameMode == IcsPlayingBlack)) {
	    SendToICS(ics_prefix);
	    SendToICS("set shout 0\n");
	}
    }

    /* Deal with midgame name changes */
    if (!newGame) {
	if (!gameInfo.white || strcmp(gameInfo.white, white) != 0) {
	    if (gameInfo.white) free(gameInfo.white);
	    gameInfo.white = StrSave(white);
	}
	if (!gameInfo.black || strcmp(gameInfo.black, black) != 0) {
	    if (gameInfo.black) free(gameInfo.black);
	    gameInfo.black = StrSave(black);
	}
    }

    /* Throw away game result if anything actually changes in examine mode */
    if (gameMode == IcsExamining && !newGame) {
	gameInfo.result = GameUnfinished;
	if (gameInfo.resultDetails != NULL) {
	    free(gameInfo.resultDetails);
	    gameInfo.resultDetails = NULL;
	}
    }

    /* In pausing && IcsExamining mode, we ignore boards coming
       in if they are in a different variation than we are. */
    if (pauseExamInvalid) return;
    if (pausing && gameMode == IcsExamining) {
	if (moveNum <= pauseExamForwardMostMove) {
	    pauseExamInvalid = TRUE;
	    forwardMostMove = pauseExamForwardMostMove;
	    return;
	}
    }

    /* Parse the board */
    for (k = 0; k < 8; k++)
      for (j = 0; j < 8; j++)
	board[k][j] = CharToPiece(board_chars[(7-k)*9 + j]);
    CopyBoard(boards[moveNum], board);
    if (moveNum == 0) {
	startedFromSetupPosition =
	  !CompareBoards(board, initialPosition);
    }

    if (ics_getting_history == H_GOT_REQ_HEADER ||
	ics_getting_history == H_GOT_UNREQ_HEADER) {
	/* This was an initial position from a move list, not
	   the current position */
	return;
    }

    /* Update currentMove and known move number limits */
    newMove = newGame || moveNum > forwardMostMove;

	/* If we found takebacks during icsEngineAnalyze 
	   try send to engine */
	if (!newGame && appData.icsEngineAnalyze && moveNum < forwardMostMove) {
		takeback = forwardMostMove - moveNum;
		for (i = 0; i < takeback; i++) {
		    if (appData.debugMode) fprintf(debugFP, "take back move\n");
		    SendToProgram("undo\n", &first);
		 }
	}
    if (newGame) {
	forwardMostMove = backwardMostMove = currentMove = moveNum;
	if (gameMode == IcsExamining && moveNum == 0) {
	  /* Workaround for ICS limitation: we are not told the wild
	     type when starting to examine a game.  But if we ask for
	     the move list, the move list header will tell us */
	    ics_getting_history = H_REQUESTED;
	    sprintf(str, "%smoves %d\n", ics_prefix, gamenum);
	    SendToICS(str);
	}
    } else if (moveNum == forwardMostMove + 1 || moveNum == forwardMostMove
	       || (moveNum < forwardMostMove && moveNum >= backwardMostMove)) {
	forwardMostMove = moveNum;
	if (!pausing || currentMove > forwardMostMove)
	  currentMove = forwardMostMove;
    } else {
	/* New part of history that is not contiguous with old part */
	if (pausing && gameMode == IcsExamining) {
	    pauseExamInvalid = TRUE;
	    forwardMostMove = pauseExamForwardMostMove;
	    return;
	}
	forwardMostMove = backwardMostMove = currentMove = moveNum;
	if (gameMode == IcsExamining && moveNum > 0 && appData.getMoveList) {
	    ics_getting_history = H_REQUESTED;
	    sprintf(str, "%smoves %d\n", ics_prefix, gamenum);
	    SendToICS(str);
	}
    }

    /* Update the clocks */
    if (strchr(elapsed_time, '.')) {
      /* Time is in ms */
      timeRemaining[0][moveNum] = whiteTimeRemaining = white_time;
      timeRemaining[1][moveNum] = blackTimeRemaining = black_time;
    } else {
      /* Time is in seconds */
      timeRemaining[0][moveNum] = whiteTimeRemaining = white_time * 1000;
      timeRemaining[1][moveNum] = blackTimeRemaining = black_time * 1000;
    }


#if ZIPPY
    if (appData.zippyPlay && newGame &&
	gameMode != IcsObserving && gameMode != IcsIdle &&
	gameMode != IcsExamining)
      ZippyFirstBoard(moveNum, basetime, increment);
#endif

    /* Put the move on the move list, first converting
       to canonical algebraic form. */
    if (moveNum > 0) {
	if (moveNum <= backwardMostMove) {
	    /* We don't know what the board looked like before
	       this move.  Punt. */
	    strcpy(parseList[moveNum - 1], move_str);
	    strcat(parseList[moveNum - 1], " ");
	    strcat(parseList[moveNum - 1], elapsed_time);
	    moveList[moveNum - 1][0] = NULLCHAR;
	} else if (ParseOneMove(move_str, moveNum - 1, &moveType,
				&fromX, &fromY, &toX, &toY, &promoChar)) {
	    (void) CoordsToAlgebraic(boards[moveNum - 1],
				     PosFlags(moveNum - 1), EP_UNKNOWN,
				     fromY, fromX, toY, toX, promoChar,
				     parseList[moveNum-1]);
	    switch (MateTest(boards[moveNum], PosFlags(moveNum), EP_UNKNOWN)){
	      case MT_NONE:
	      case MT_STALEMATE:
	      default:
		break;
	      case MT_CHECK:
		strcat(parseList[moveNum - 1], "+");
		break;
	      case MT_CHECKMATE:
		strcat(parseList[moveNum - 1], "#");
		break;
	    }
	    strcat(parseList[moveNum - 1], " ");
	    strcat(parseList[moveNum - 1], elapsed_time);
	    /* currentMoveString is set as a side-effect of ParseOneMove */
	    strcpy(moveList[moveNum - 1], currentMoveString);
	    strcat(moveList[moveNum - 1], "\n");
	} else if (strcmp(move_str, "none") == 0) {
	    /* Again, we don't know what the board looked like;
	       this is really the start of the game. */
	    parseList[moveNum - 1][0] = NULLCHAR;
	    moveList[moveNum - 1][0] = NULLCHAR;
	    backwardMostMove = moveNum;
	    startedFromSetupPosition = TRUE;
 	    fromX = fromY = toX = toY = -1;
	} else {
	    /* Move from ICS was illegal!?  Punt. */
#if 0
	    if (appData.testLegality && appData.debugMode) {
		sprintf(str, "Illegal move \"%s\" from ICS", move_str);
		DisplayError(str, 0);
	    }
#endif
	    strcpy(parseList[moveNum - 1], move_str);
	    strcat(parseList[moveNum - 1], " ");
	    strcat(parseList[moveNum - 1], elapsed_time);
	    moveList[moveNum - 1][0] = NULLCHAR;
 	    fromX = fromY = toX = toY = -1;
	}

#if ZIPPY
	/* Send move to chess program (BEFORE animating it). */
	if (appData.zippyPlay && !newGame && newMove &&
	   (!appData.getMoveList || backwardMostMove == 0) && first.initDone) {

	    if ((gameMode == IcsPlayingWhite && WhiteOnMove(moveNum)) ||
		(gameMode == IcsPlayingBlack && !WhiteOnMove(moveNum))) {
		if (moveList[moveNum - 1][0] == NULLCHAR) {
		    sprintf(str, _("Couldn't parse move \"%s\" from ICS"),
			    move_str);
		    DisplayError(str, 0);
		} else {
		    if (first.sendTime) {
			SendTimeRemaining(&first, gameMode == IcsPlayingWhite);
		    }
		    SendMoveToProgram(moveNum - 1, &first);
		    if (firstMove) {
			firstMove = FALSE;
			if (first.useColors) {
			  SendToProgram(gameMode == IcsPlayingWhite ?
					"white\ngo\n" :
					"black\ngo\n", &first);
			} else {
			  SendToProgram("go\n", &first);
			}
			first.maybeThinking = TRUE;
		    }
		}
	    } else if (gameMode == IcsObserving || gameMode == IcsExamining) {
	      if (moveList[moveNum - 1][0] == NULLCHAR) {
		sprintf(str, _("Couldn't parse move \"%s\" from ICS"), move_str);
		DisplayError(str, 0);
	      } else {
		SendMoveToProgram(moveNum - 1, &first);
	      }
	    }
	}
#endif
    }

    if (moveNum > 0 && !gotPremove) {
	/* If move comes from a remote source, animate it.  If it
	   isn't remote, it will have already been animated. */
	if (!pausing && !ics_user_moved && prevMove == moveNum - 1) {
	    AnimateMove(boards[moveNum - 1], fromX, fromY, toX, toY);
	}
	if (!pausing && appData.highlightLastMove) {
	    SetHighlights(fromX, fromY, toX, toY);
	}
    }

    /* Start the clocks */
    whiteFlag = blackFlag = FALSE;
    appData.clockMode = !(basetime == 0 && increment == 0);
    if (ticking == 0) {
      ics_clock_paused = TRUE;
      StopClocks();
    } else if (ticking == 1) {
      ics_clock_paused = FALSE;
    }
    if (gameMode == IcsIdle ||
	relation == RELATION_OBSERVING_STATIC ||
	relation == RELATION_EXAMINING ||
	ics_clock_paused)
      DisplayBothClocks();
    else
      StartClocks();

    /* Display opponents and material strengths */
    if (gameInfo.variant != VariantBughouse &&
	gameInfo.variant != VariantCrazyhouse) {
	if (tinyLayout || smallLayout) {
	    sprintf(str, "%s(%d) %s(%d) {%d %d}",
		    gameInfo.white, white_stren, gameInfo.black, black_stren,
		    basetime, increment);
	} else {
	    sprintf(str, "%s (%d) vs. %s (%d) {%d %d}",
		    gameInfo.white, white_stren, gameInfo.black, black_stren,
		    basetime, increment);
	}
	DisplayTitle(str);
    }


    /* Display the board */
    if (!pausing) {

      if (appData.premove)
	  if (!gotPremove ||
	     ((gameMode == IcsPlayingWhite) && (WhiteOnMove(currentMove))) ||
	     ((gameMode == IcsPlayingBlack) && (!WhiteOnMove(currentMove))))
	      ClearPremoveHighlights();

      DrawPosition(FALSE, boards[currentMove]);
      DisplayMove(moveNum - 1);
      if (appData.ringBellAfterMoves && !ics_user_moved)
	RingBell();
    }

    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);
}

void
GetMoveListEvent()
{
    char buf[MSG_SIZ];
    if (appData.icsActive && gameMode != IcsIdle && ics_gamenum > 0) {
	ics_getting_history = H_REQUESTED;
	sprintf(buf, "%smoves %d\n", ics_prefix, ics_gamenum);
	SendToICS(buf);
    }
}

void
AnalysisPeriodicEvent(force)
     int force;
{
    if (((programStats.ok_to_send == 0 || programStats.line_is_book)
	 && !force) || !appData.periodicUpdates)
      return;

    /* Send . command to Crafty to collect stats */
    SendToProgram(".\n", &first);

    /* Don't send another until we get a response (this makes
       us stop sending to old Crafty's which don't understand
       the "." command (sending illegal cmds resets node count & time,
       which looks bad)) */
    programStats.ok_to_send = 0;
}

void
SendMoveToProgram(moveNum, cps)
     int moveNum;
     ChessProgramState *cps;
{
    char buf[MSG_SIZ];
    if (cps->useUsermove) {
      SendToProgram("usermove ", cps);
    }
    if (cps->useSAN) {
      char *space;
      if ((space = strchr(parseList[moveNum], ' ')) != NULL) {
	int len = space - parseList[moveNum];
	memcpy(buf, parseList[moveNum], len);
	buf[len++] = '\n';
	buf[len] = NULLCHAR;
      } else {
	sprintf(buf, "%s\n", parseList[moveNum]);
      }
      SendToProgram(buf, cps);
    } else {
      SendToProgram(moveList[moveNum], cps);
    }
}

void
SendMoveToICS(moveType, fromX, fromY, toX, toY)
     ChessMove moveType;
     int fromX, fromY, toX, toY;
{
    char user_move[MSG_SIZ];

    switch (moveType) {
      default:
	sprintf(user_move, "say Internal error; bad moveType %d (%d,%d-%d,%d)",
		(int)moveType, fromX, fromY, toX, toY);
	DisplayError(user_move + strlen("say "), 0);
	break;
      case WhiteKingSideCastle:
      case BlackKingSideCastle:
      case WhiteQueenSideCastleWild:
      case BlackQueenSideCastleWild:
	sprintf(user_move, "o-o\n");
	break;
      case WhiteQueenSideCastle:
      case BlackQueenSideCastle:
      case WhiteKingSideCastleWild:
      case BlackKingSideCastleWild:
	sprintf(user_move, "o-o-o\n");
	break;
      case WhitePromotionQueen:
      case BlackPromotionQueen:
      case WhitePromotionRook:
      case BlackPromotionRook:
      case WhitePromotionBishop:
      case BlackPromotionBishop:
      case WhitePromotionKnight:
      case BlackPromotionKnight:
      case WhitePromotionKing:
      case BlackPromotionKing:
	sprintf(user_move, "%c%c%c%c=%c\n",
		'a' + fromX, '1' + fromY, 'a' + toX, '1' + toY,
		PieceToChar(PromoPiece(moveType)));
	break;
      case WhiteDrop:
      case BlackDrop:
	sprintf(user_move, "%c@%c%c\n",
		ToUpper(PieceToChar((ChessSquare) fromX)),
		'a' + toX, '1' + toY);
	break;
      case NormalMove:
      case WhiteCapturesEnPassant:
      case BlackCapturesEnPassant:
      case IllegalMove:  /* could be a variant we don't quite understand */
	sprintf(user_move, "%c%c%c%c\n",
		'a' + fromX, '1' + fromY, 'a' + toX, '1' + toY);
	break;
    }
    SendToICS(user_move);
}

void
CoordsToComputerAlgebraic(rf, ff, rt, ft, promoChar, move)
     int rf, ff, rt, ft;
     char promoChar;
     char move[7];
{
    if (rf == DROP_RANK) {
	sprintf(move, "%c@%c%c\n",
		ToUpper(PieceToChar((ChessSquare) ff)), 'a' + ft, '1' + rt);
    } else {
	if (promoChar == 'x' || promoChar == NULLCHAR) {
	    sprintf(move, "%c%c%c%c\n",
		    'a' + ff, '1' + rf, 'a' + ft, '1' + rt);
	} else {
	    sprintf(move, "%c%c%c%c%c\n",
		    'a' + ff, '1' + rf, 'a' + ft, '1' + rt, promoChar);
	}
    }
}

void
ProcessICSInitScript(f)
     FILE *f;
{
    char buf[MSG_SIZ];

    while (fgets(buf, MSG_SIZ, f)) {
	SendToICSDelayed(buf,(long)appData.msLoginDelay);
    }

    fclose(f);
}


/* Parser for moves from gnuchess, ICS, or user typein box */
Boolean
ParseOneMove(move, moveNum, moveType, fromX, fromY, toX, toY, promoChar)
     char *move;
     int moveNum;
     ChessMove *moveType;
     int *fromX, *fromY, *toX, *toY;
     char *promoChar;
{
    *moveType = yylexstr(moveNum, move);
    switch (*moveType) {
      case WhitePromotionQueen:
      case BlackPromotionQueen:
      case WhitePromotionRook:
      case BlackPromotionRook:
      case WhitePromotionBishop:
      case BlackPromotionBishop:
      case WhitePromotionKnight:
      case BlackPromotionKnight:
      case WhitePromotionKing:
      case BlackPromotionKing:
      case NormalMove:
      case WhiteCapturesEnPassant:
      case BlackCapturesEnPassant:
      case WhiteKingSideCastle:
      case WhiteQueenSideCastle:
      case BlackKingSideCastle:
      case BlackQueenSideCastle:
      case WhiteKingSideCastleWild:
      case WhiteQueenSideCastleWild:
      case BlackKingSideCastleWild:
      case BlackQueenSideCastleWild:
      case IllegalMove:		/* bug or odd chess variant */
	*fromX = currentMoveString[0] - 'a';
	*fromY = currentMoveString[1] - '1';
	*toX = currentMoveString[2] - 'a';
	*toY = currentMoveString[3] - '1';
	*promoChar = currentMoveString[4];
	if (*fromX < 0 || *fromX > 7 || *fromY < 0 || *fromY > 7 ||
	    *toX < 0 || *toX > 7 || *toY < 0 || *toY > 7) {
	    *fromX = *fromY = *toX = *toY = 0;
	    return FALSE;
	}
	if (appData.testLegality) {
	  return (*moveType != IllegalMove);
	} else {
	  return !(fromX == fromY && toX == toY);
	}

      case WhiteDrop:
      case BlackDrop:
	*fromX = *moveType == WhiteDrop ?
	  (int) CharToPiece(ToUpper(currentMoveString[0])) :
	(int) CharToPiece(ToLower(currentMoveString[0]));
	*fromY = DROP_RANK;
	*toX = currentMoveString[2] - 'a';
	*toY = currentMoveString[3] - '1';
	*promoChar = NULLCHAR;
	return TRUE;

      case AmbiguousMove:
      case ImpossibleMove:
      case (ChessMove) 0:	/* end of file */
      case ElapsedTime:
      case Comment:
      case PGNTag:
      case NAG:
      case WhiteWins:
      case BlackWins:
      case GameIsDrawn:
      default:
	/* bug? */
	*fromX = *fromY = *toX = *toY = 0;
	*promoChar = NULLCHAR;
	return FALSE;
    }
}


void
InitPosition(redraw)
     int redraw;
{
    currentMove = forwardMostMove = backwardMostMove = 0;
    switch (gameInfo.variant) {
    default:
      CopyBoard(boards[0], initialPosition);
      break;
    case VariantTwoKings:
      CopyBoard(boards[0], twoKingsPosition);
      startedFromSetupPosition = TRUE;
      break;
    case VariantWildCastle:
      CopyBoard(boards[0], initialPosition);
      /* !!?shuffle with kings guaranteed to be on d or e file */
      break;
    case VariantNoCastle:
      CopyBoard(boards[0], initialPosition);
      /* !!?unconstrained back-rank shuffle */
      break;
    case VariantFischeRandom:
      CopyBoard(boards[0], initialPosition);
      /* !!shuffle according to FR rules */
      break;
    }
    if (redraw)
      DrawPosition(FALSE, boards[currentMove]);
}

void
SendBoard(cps, moveNum)
     ChessProgramState *cps;
     int moveNum;
{
    char message[MSG_SIZ];

    if (cps->useSetboard) {
      char* fen = PositionToFEN(moveNum);
      sprintf(message, "setboard %s\n", fen);
      SendToProgram(message, cps);
      free(fen);

    } else {
      ChessSquare *bp;
      int i, j;
      /* Kludge to set black to move, avoiding the troublesome and now
       * deprecated "black" command.
       */
      if (!WhiteOnMove(moveNum)) SendToProgram("a2a3\n", cps);

      SendToProgram("edit\n", cps);
      SendToProgram("#\n", cps);
      for (i = BOARD_SIZE - 1; i >= 0; i--) {
	bp = &boards[moveNum][i][0];
	for (j = 0; j < BOARD_SIZE; j++, bp++) {
	  if ((int) *bp < (int) BlackPawn) {
	    sprintf(message, "%c%c%c\n", PieceToChar(*bp),
		    'a' + j, '1' + i);
	    SendToProgram(message, cps);
	  }
	}
      }

      SendToProgram("c\n", cps);
      for (i = BOARD_SIZE - 1; i >= 0; i--) {
	bp = &boards[moveNum][i][0];
	for (j = 0; j < BOARD_SIZE; j++, bp++) {
	  if (((int) *bp != (int) EmptySquare)
	      && ((int) *bp >= (int) BlackPawn)) {
	    sprintf(message, "%c%c%c\n", ToUpper(PieceToChar(*bp)),
		    'a' + j, '1' + i);
	    SendToProgram(message, cps);
	  }
	}
      }

      SendToProgram(".\n", cps);
    }
}

int
IsPromotion(fromX, fromY, toX, toY)
     int fromX, fromY, toX, toY;
{
    return gameMode != EditPosition &&
      fromX >=0 && fromY >= 0 && toX >= 0 && toY >= 0 &&
	((boards[currentMove][fromY][fromX] == WhitePawn && toY == 7) ||
	 (boards[currentMove][fromY][fromX] == BlackPawn && toY == 0));
}


int
PieceForSquare (x, y)
     int x;
     int y;
{
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
     return -1;
  else
     return boards[currentMove][y][x];
}

int
OKToStartUserMove(x, y)
     int x, y;
{
    ChessSquare from_piece;
    int white_piece;

    if (matchMode) return FALSE;
    if (gameMode == EditPosition) return TRUE;

    if (x >= 0 && y >= 0)
      from_piece = boards[currentMove][y][x];
    else
      from_piece = EmptySquare;

    if (from_piece == EmptySquare) return FALSE;

    white_piece = (int)from_piece >= (int)WhitePawn &&
      (int)from_piece <= (int)WhiteKing;

    switch (gameMode) {
      case PlayFromGameFile:
      case AnalyzeFile:
      case TwoMachinesPlay:
      case EndOfGame:
	return FALSE;

      case IcsObserving:
      case IcsIdle:
	return FALSE;

      case MachinePlaysWhite:
      case IcsPlayingBlack:
	if (appData.zippyPlay) return FALSE;
	if (white_piece) {
	    DisplayMoveError(_("You are playing Black"));
	    return FALSE;
	}
	break;

      case MachinePlaysBlack:
      case IcsPlayingWhite:
	if (appData.zippyPlay) return FALSE;
	if (!white_piece) {
	    DisplayMoveError(_("You are playing White"));
	    return FALSE;
	}
	break;

      case EditGame:
	if (!white_piece && WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is White's turn"));
	    return FALSE;
	}
	if (white_piece && !WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is Black's turn"));
	    return FALSE;
	}
	if (cmailMsgLoaded && (currentMove < cmailOldMove)) {
	    /* Editing correspondence game history */
	    /* Could disallow this or prompt for confirmation */
	    cmailOldMove = -1;
	}
	if (currentMove < forwardMostMove) {
	    /* Discarding moves */
	    /* Could prompt for confirmation here,
	       but I don't think that's such a good idea */
	    forwardMostMove = currentMove;
	}
	break;

      case BeginningOfGame:
	if (appData.icsActive) return FALSE;
	if (!appData.noChessProgram) {
	    if (!white_piece) {
		DisplayMoveError(_("You are playing White"));
		return FALSE;
	    }
	}
	break;

      case Training:
	if (!white_piece && WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is White's turn"));
	    return FALSE;
	}
	if (white_piece && !WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is Black's turn"));
	    return FALSE;
	}
	break;

      default:
      case IcsExamining:
	break;
    }
    if (currentMove != forwardMostMove && gameMode != AnalyzeMode
	&& gameMode != AnalyzeFile && gameMode != Training) {
	DisplayMoveError(_("Displayed position is not current"));
	return FALSE;
    }
    return TRUE;
}

FILE *lastLoadGameFP = NULL, *lastLoadPositionFP = NULL;
int lastLoadGameNumber = 0, lastLoadPositionNumber = 0;
int lastLoadGameUseList = FALSE;
char lastLoadGameTitle[MSG_SIZ], lastLoadPositionTitle[MSG_SIZ];
ChessMove lastLoadGameStart = (ChessMove) 0;


void
UserMoveEvent(fromX, fromY, toX, toY, promoChar)
     int fromX, fromY, toX, toY;
     int promoChar;
{
    ChessMove moveType;

    if (fromX < 0 || fromY < 0) return;
    if ((fromX == toX) && (fromY == toY)) {
	return;
    }

    /* Check if the user is playing in turn.  This is complicated because we
       let the user "pick up" a piece before it is his turn.  So the piece he
       tried to pick up may have been captured by the time he puts it down!
       Therefore we use the color the user is supposed to be playing in this
       test, not the color of the piece that is currently on the starting
       square---except in EditGame mode, where the user is playing both
       sides; fortunately there the capture race can't happen.  (It can
       now happen in IcsExamining mode, but that's just too bad.  The user
       will get a somewhat confusing message in that case.)
       */

    switch (gameMode) {
      case PlayFromGameFile:
      case AnalyzeFile:
      case TwoMachinesPlay:
      case EndOfGame:
      case IcsObserving:
      case IcsIdle:
	/* We switched into a game mode where moves are not accepted,
           perhaps while the mouse button was down. */
	return;

      case MachinePlaysWhite:
	/* User is moving for Black */
	if (WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is White's turn"));
	    return;
	}
	break;

      case MachinePlaysBlack:
	/* User is moving for White */
	if (!WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is Black's turn"));
	    return;
	}
	break;

      case EditGame:
      case IcsExamining:
      case BeginningOfGame:
      case AnalyzeMode:
      case Training:
	if ((int) boards[currentMove][fromY][fromX] >= (int) BlackPawn &&
	    (int) boards[currentMove][fromY][fromX] <= (int) BlackKing) {
	    /* User is moving for Black */
	    if (WhiteOnMove(currentMove)) {
		DisplayMoveError(_("It is White's turn"));
		return;
	    }
	} else {
	    /* User is moving for White */
	    if (!WhiteOnMove(currentMove)) {
		DisplayMoveError(_("It is Black's turn"));
		return;
	    }
	}
	break;

      case IcsPlayingBlack:
	/* User is moving for Black */
	if (WhiteOnMove(currentMove)) {
	    if (!appData.premove) {
		DisplayMoveError(_("It is White's turn"));
	    } else if (toX >= 0 && toY >= 0) {
		premoveToX = toX;
		premoveToY = toY;
		premoveFromX = fromX;
		premoveFromY = fromY;
		premovePromoChar = promoChar;
		gotPremove = 1;
		if (appData.debugMode)
		    fprintf(debugFP, "Got premove: fromX %d,"
			    "fromY %d, toX %d, toY %d\n",
			    fromX, fromY, toX, toY);
	    }
	    return;
	}
	break;

      case IcsPlayingWhite:
	/* User is moving for White */
	if (!WhiteOnMove(currentMove)) {
	    if (!appData.premove) {
		DisplayMoveError(_("It is Black's turn"));
	    } else if (toX >= 0 && toY >= 0) {
		premoveToX = toX;
		premoveToY = toY;
		premoveFromX = fromX;
		premoveFromY = fromY;
		premovePromoChar = promoChar;
		gotPremove = 1;
		if (appData.debugMode)
		    fprintf(debugFP, "Got premove: fromX %d,"
			    "fromY %d, toX %d, toY %d\n",
			    fromX, fromY, toX, toY);
	    }
	    return;
	}
	break;

      default:
	break;

      case EditPosition:
	if (toX == -2 || toY == -2) {
	    boards[0][fromY][fromX] = EmptySquare;
	    DrawPosition(FALSE, boards[currentMove]);
	} else if (toX >= 0 && toY >= 0) {
	    boards[0][toY][toX] = boards[0][fromY][fromX];
	    boards[0][fromY][fromX] = EmptySquare;
	    DrawPosition(FALSE, boards[currentMove]);
	}
	return;
    }

    if (toX < 0 || toY < 0) return;
    userOfferedDraw = FALSE;

    if (appData.testLegality) {
	moveType = LegalityTest(boards[currentMove], PosFlags(currentMove),
				EP_UNKNOWN, fromY, fromX, toY, toX, promoChar);
	if (moveType == IllegalMove || moveType == ImpossibleMove) {
	    DisplayMoveError(_("Illegal move"));
	    return;
	}
    } else {
	moveType = PromoCharToMoveType(WhiteOnMove(currentMove), promoChar);
    }

    if (gameMode == Training) {
      /* compare the move played on the board to the next move in the
       * game. If they match, display the move and the opponent's response.
       * If they don't match, display an error message.
       */
      int saveAnimate;
      Board testBoard;
      CopyBoard(testBoard, boards[currentMove]);
      ApplyMove(fromX, fromY, toX, toY, promoChar, testBoard);

      if (CompareBoards(testBoard, boards[currentMove+1])) {
	ForwardInner(currentMove+1);

	/* Autoplay the opponent's response.
	 * if appData.animate was TRUE when Training mode was entered,
	 * the response will be animated.
	 */
	saveAnimate = appData.animate;
	appData.animate = animateTraining;
	ForwardInner(currentMove+1);
	appData.animate = saveAnimate;

	/* check for the end of the game */
	if (currentMove >= forwardMostMove) {
	  gameMode = PlayFromGameFile;
	  ModeHighlight();
	  SetTrainingModeOff();
	  DisplayInformation(_("End of game"));
	}
      } else {
	DisplayError(_("Incorrect move"), 0);
      }
      return;
    }

    FinishMove(moveType, fromX, fromY, toX, toY, promoChar);
}

/* Common tail of UserMoveEvent and DropMenuEvent */
void
FinishMove(moveType, fromX, fromY, toX, toY, promoChar)
     ChessMove moveType;
     int fromX, fromY, toX, toY;
     /*char*/int promoChar;
{
  /* Ok, now we know that the move is good, so we can kill
     the previous line in Analysis Mode */
  if (gameMode == AnalyzeMode && currentMove < forwardMostMove) {
    forwardMostMove = currentMove;
  }

  /* If we need the chess program but it's dead, restart it */
  ResurrectChessProgram();

  /* A user move restarts a paused game*/
  if (pausing)
    PauseEvent();

  thinkOutput[0] = NULLCHAR;

  MakeMove(fromX, fromY, toX, toY, promoChar); /*updates forwardMostMove*/

  if (gameMode == BeginningOfGame) {
    if (appData.noChessProgram) {
      gameMode = EditGame;
      SetGameInfo();
    } else {
      char buf[MSG_SIZ];
      gameMode = MachinePlaysBlack;
      SetGameInfo();
      sprintf(buf, "%s vs. %s", gameInfo.white, gameInfo.black);
      DisplayTitle(buf);
      if (first.sendName) {
	sprintf(buf, "name %s\n", gameInfo.white);
	SendToProgram(buf, &first);
      }
    }
    ModeHighlight();
  }

  /* Relay move to ICS or chess engine */
  if (appData.icsActive) {
    if (gameMode == IcsPlayingWhite || gameMode == IcsPlayingBlack ||
	gameMode == IcsExamining) {
      SendMoveToICS(moveType, fromX, fromY, toX, toY);
      ics_user_moved = 1;
    }
  } else {
    if (first.sendTime && (gameMode == BeginningOfGame ||
			   gameMode == MachinePlaysWhite ||
			   gameMode == MachinePlaysBlack)) {
      SendTimeRemaining(&first, gameMode != MachinePlaysBlack);
    }
    SendMoveToProgram(forwardMostMove-1, &first);
    if (gameMode != EditGame && gameMode != PlayFromGameFile) {
      first.maybeThinking = TRUE;
    }
    if (currentMove == cmailOldMove + 1) {
      cmailMoveType[lastLoadGameNumber - 1] = CMAIL_MOVE;
    }
  }

  ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/

  switch (gameMode) {
  case EditGame:
    switch (MateTest(boards[currentMove], PosFlags(currentMove),
		     EP_UNKNOWN)) {
    case MT_NONE:
    case MT_CHECK:
      break;
    case MT_CHECKMATE:
      if (WhiteOnMove(currentMove)) {
	GameEnds(BlackWins, "Black mates", GE_PLAYER);
      } else {
	GameEnds(WhiteWins, "White mates", GE_PLAYER);
      }
      break;
    case MT_STALEMATE:
      GameEnds(GameIsDrawn, "Stalemate", GE_PLAYER);
      break;
    }
    break;

  case MachinePlaysBlack:
  case MachinePlaysWhite:
    /* disable certain menu options while machine is thinking */
    SetMachineThinkingEnables();
    break;

  default:
    break;
  }
}

void
HandleMachineMove(message, cps)
     char *message;
     ChessProgramState *cps;
{
    char machineMove[MSG_SIZ], buf1[MSG_SIZ*10], buf2[MSG_SIZ];
    char realname[MSG_SIZ];
    int fromX, fromY, toX, toY;
    ChessMove moveType;
    char promoChar;
    char *p;
    int machineWhite;

    /*
     * Kludge to ignore BEL characters
     */
    while (*message == '\007') message++;

    /*
     * Look for book output
     */
    if (cps == &first && bookRequested) {
	if (message[0] == '\t' || message[0] == ' ') {
	    /* Part of the book output is here; append it */
	    strcat(bookOutput, message);
	    strcat(bookOutput, "  \n");
	    return;
	} else if (bookOutput[0] != NULLCHAR) {
	    /* All of book output has arrived; display it */
	    char *p = bookOutput;
	    while (*p != NULLCHAR) {
		if (*p == '\t') *p = ' ';
		p++;
	    }
	    DisplayInformation(bookOutput);
	    bookRequested = FALSE;
	    /* Fall through to parse the current output */
	}
    }

    /*
     * Look for machine move.
     */
    if ((sscanf(message, "%s %s %s", buf1, buf2, machineMove) == 3 &&
	 strcmp(buf2, "...") == 0) ||
	(sscanf(message, "%s %s", buf1, machineMove) == 2 &&
	 strcmp(buf1, "move") == 0)) {

        /* This method is only useful on engines that support ping */
        if (cps->lastPing != cps->lastPong) {
	  if (gameMode == BeginningOfGame) {
	    /* Extra move from before last new; ignore */
	    if (appData.debugMode) {
		fprintf(debugFP, "Ignoring extra move from %s\n", cps->which);
	    }
	  } else {
	    if (appData.debugMode) {
		fprintf(debugFP, "Undoing extra move from %s, gameMode %d\n",
			cps->which, gameMode);
	    }
	    SendToProgram("undo\n", cps);
	  }
	  return;
	}

	switch (gameMode) {
	  case BeginningOfGame:
	    /* Extra move from before last reset; ignore */
	    if (appData.debugMode) {
		fprintf(debugFP, "Ignoring extra move from %s\n", cps->which);
	    }
	    return;

	  case EndOfGame:
	  case IcsIdle:
	  default:
	    /* Extra move after we tried to stop.  The mode test is
	       not a reliable way of detecting this problem, but it's
	       the best we can do on engines that don't support ping.
	    */
	    if (appData.debugMode) {
		fprintf(debugFP, "Undoing extra move from %s, gameMode %d\n",
			cps->which, gameMode);
	    }
	    SendToProgram("undo\n", cps);
	    return;

	  case MachinePlaysWhite:
	  case IcsPlayingWhite:
	    machineWhite = TRUE;
	    break;

	  case MachinePlaysBlack:
	  case IcsPlayingBlack:
	    machineWhite = FALSE;
	    break;

	  case TwoMachinesPlay:
	    machineWhite = (cps->twoMachinesColor[0] == 'w');
	    break;
	}
	if (WhiteOnMove(forwardMostMove) != machineWhite) {
	    if (appData.debugMode) {
		fprintf(debugFP,
			"Ignoring move out of turn by %s, gameMode %d"
			", forwardMost %d\n",
			cps->which, gameMode, forwardMostMove);
	    }
	    return;
	}

	if (!ParseOneMove(machineMove, forwardMostMove, &moveType,
			      &fromX, &fromY, &toX, &toY, &promoChar)) {
	    /* Machine move could not be parsed; ignore it. */
	    sprintf(buf1, _("Illegal move \"%s\" from %s machine"),
		    machineMove, cps->which);
	    DisplayError(buf1, 0);
	    if (gameMode == TwoMachinesPlay) {
	      GameEnds(machineWhite ? BlackWins : WhiteWins,
		       "Forfeit due to illegal move", GE_XBOARD);
	    }
	    return;
	}

	hintRequested = FALSE;
	lastHint[0] = NULLCHAR;
	bookRequested = FALSE;
	/* Program may be pondering now */
	cps->maybeThinking = TRUE;
	if (cps->sendTime == 2) cps->sendTime = 1;
	if (cps->offeredDraw) cps->offeredDraw--;

#if ZIPPY
	if ((gameMode == IcsPlayingWhite || gameMode == IcsPlayingBlack) &&
	    first.initDone) {
	  SendMoveToICS(moveType, fromX, fromY, toX, toY);
	  ics_user_moved = 1;
	}
#endif
	/* currentMoveString is set as a side-effect of ParseOneMove */
	strcpy(machineMove, currentMoveString);
	strcat(machineMove, "\n");
	strcpy(moveList[forwardMostMove], machineMove);

	MakeMove(fromX, fromY, toX, toY, promoChar);/*updates forwardMostMove*/

	if (gameMode == TwoMachinesPlay) {
	    if (cps->other->sendTime) {
		SendTimeRemaining(cps->other,
				  cps->other->twoMachinesColor[0] == 'w');
	    }
	    SendMoveToProgram(forwardMostMove-1, cps->other);
	    if (firstMove) {
		firstMove = FALSE;
		if (cps->other->useColors) {
		  SendToProgram(cps->other->twoMachinesColor, cps->other);
		}
		SendToProgram("go\n", cps->other);
	    }
	    cps->other->maybeThinking = TRUE;
	}

	ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
	if (!pausing && appData.ringBellAfterMoves) {
	    RingBell();
	}
	/*
	 * Reenable menu items that were disabled while
	 * machine was thinking
	 */
	if (gameMode != TwoMachinesPlay)
	    SetUserThinkingEnables();
	return;
    }

    /* Set special modes for chess engines.  Later something general
     *  could be added here; for now there is just one kludge feature,
     *  needed because Crafty 15.10 and earlier don't ignore SIGINT
     *  when "xboard" is given as an interactive command.
     */
    if (strncmp(message, "kibitz Hello from Crafty", 24) == 0) {
	cps->useSigint = FALSE;
	cps->useSigterm = FALSE;
    }

    /*
     * Look for communication commands
     */
    if (!strncmp(message, "telluser ", 9)) {
	DisplayNote(message + 9);
	return;
    }
    if (!strncmp(message, "tellusererror ", 14)) {
	DisplayError(message + 14, 0);
	return;
    }
    if (!strncmp(message, "tellopponent ", 13)) {
      if (appData.icsActive) {
	if (loggedOn) {
	  sprintf(buf1, "%ssay %s\n", ics_prefix, message + 13);
	  SendToICS(buf1);
	}
      } else {
	DisplayNote(message + 13);
      }
      return;
    }
    if (!strncmp(message, "tellothers ", 11)) {
      if (appData.icsActive) {
	if (loggedOn) {
	  sprintf(buf1, "%swhisper %s\n", ics_prefix, message + 11);
	  SendToICS(buf1);
	}
      }
      return;
    }
    if (!strncmp(message, "tellall ", 8)) {
      if (appData.icsActive) {
	if (loggedOn) {
	  sprintf(buf1, "%skibitz %s\n", ics_prefix, message + 8);
	  SendToICS(buf1);
	}
      } else {
	DisplayNote(message + 8);
      }
      return;
    }
    if (strncmp(message, "warning", 7) == 0) {
	/* Undocumented feature, use tellusererror in new code */
	DisplayError(message, 0);
	return;
    }
    if (sscanf(message, "askuser %s %[^\n]", buf1, buf2) == 2) {
	strcpy(realname, cps->tidy);
	strcat(realname, " query");
	AskQuestion(realname, buf2, buf1, cps->pr);
	return;
    }
    /* Commands from the engine directly to ICS.  We don't allow these to be
     *  sent until we are logged on. Crafty kibitzes have been known to
     *  interfere with the login process.
     */
    if (loggedOn) {
	if (!strncmp(message, "tellics ", 8)) {
	    SendToICS(message + 8);
	    SendToICS("\n");
	    return;
	}
	if (!strncmp(message, "tellicsnoalias ", 15)) {
	    SendToICS(ics_prefix);
	    SendToICS(message + 15);
	    SendToICS("\n");
	    return;
	}
	/* The following are for backward compatibility only */
	if (!strncmp(message,"whisper",7) || !strncmp(message,"kibitz",6) ||
	    !strncmp(message,"draw",4) || !strncmp(message,"tell",3)) {
	    SendToICS(ics_prefix);
	    SendToICS(message);
	    SendToICS("\n");
	    return;
	}
    }
    if (strncmp(message, "feature ", 8) == 0) {
      ParseFeatures(message+8, cps);
    }
    if (sscanf(message, "pong %d", &cps->lastPong) == 1) {
      return;
    }
    /*
     * If the move is illegal, cancel it and redraw the board.
     * Also deal with other error cases.  Matching is rather loose
     * here to accommodate engines written before the spec.
     */
    if (strncmp(message + 1, "llegal move", 11) == 0 ||
	strncmp(message, "Error", 5) == 0) {
	if (StrStr(message, "name") ||
	    StrStr(message, "rating") || StrStr(message, "?") ||
	    StrStr(message, "result") || StrStr(message, "board") ||
	    StrStr(message, "bk") || StrStr(message, "computer") ||
	    StrStr(message, "variant") || StrStr(message, "hint") ||
	    StrStr(message, "random") || StrStr(message, "depth") ||
	    StrStr(message, "accepted")) {
	    return;
	}
	if (StrStr(message, "protover")) {
	  /* Program is responding to input, so it's apparently done
             initializing, and this error message indicates it is
             protocol version 1.  So we don't need to wait any longer
             for it to initialize and send feature commands. */
	  FeatureDone(cps, 1);
	  cps->protocolVersion = 1;
	  return;
	}
	cps->maybeThinking = FALSE;

	if (StrStr(message, "draw")) {
	    /* Program doesn't have "draw" command */
	    cps->sendDrawOffers = 0;
	    return;
	}
	if (cps->sendTime != 1 &&
	    (StrStr(message, "time") || StrStr(message, "otim"))) {
	  /* Program apparently doesn't have "time" or "otim" command */
	  cps->sendTime = 0;
	  return;
	}
	if (StrStr(message, "analyze")) {
	    cps->analysisSupport = FALSE;
	    cps->analyzing = FALSE;
	    Reset(FALSE, TRUE);
	    sprintf(buf2, "%s does not support analysis", cps->tidy);
	    DisplayError(buf2, 0);
	    return;
	}
	if (StrStr(message, "(no matching move)st")) {
	  /* Special kludge for GNU Chess 4 only */
	  cps->stKludge = TRUE;
	  SendTimeControl(cps, movesPerSession, timeControl,
			  timeIncrement, appData.searchDepth,
			  searchTime);
	  return;
	}
	if (StrStr(message, "(no matching move)sd")) {
	  /* Special kludge for GNU Chess 4 only */
	  cps->sdKludge = TRUE;
	  SendTimeControl(cps, movesPerSession, timeControl,
			  timeIncrement, appData.searchDepth,
			  searchTime);
	  return;
	}
	if (!StrStr(message, "llegal")) return;
	if (gameMode == BeginningOfGame || gameMode == EndOfGame ||
	    gameMode == IcsIdle) return;
	if (forwardMostMove <= backwardMostMove) return;
#if 0
	/* Following removed: it caused a bug where a real illegal move
	   message in analyze mored would be ignored. */
	if (cps == &first && programStats.ok_to_send == 0) {
	    /* Bogus message from Crafty responding to "."  This filtering
	       can miss some of the bad messages, but fortunately the bug
	       is fixed in current Crafty versions, so it doesn't matter. */
	    return;
	}
#endif
	if (pausing) PauseEvent();
	if (gameMode == PlayFromGameFile) {
	    /* Stop reading this game file */
	    gameMode = EditGame;
	    ModeHighlight();
	}
	currentMove = --forwardMostMove;
	DisplayMove(currentMove-1); /* before DisplayMoveError */
	SwitchClocks();
	DisplayBothClocks();
	sprintf(buf1, _("Illegal move \"%s\" (rejected by %s chess program)"),
		parseList[currentMove], cps->which);
	DisplayMoveError(buf1);
	DrawPosition(FALSE, boards[currentMove]);
	return;
    }
    if (strncmp(message, "time", 4) == 0 && StrStr(message, "Illegal")) {
	/* Program has a broken "time" command that
	   outputs a string not ending in newline.
	   Don't use it. */
	cps->sendTime = 0;
    }

    /*
     * If chess program startup fails, exit with an error message.
     * Attempts to recover here are futile.
     */
    if ((StrStr(message, "unknown host") != NULL)
	|| (StrStr(message, "No remote directory") != NULL)
	|| (StrStr(message, "not found") != NULL)
	|| (StrStr(message, "No such file") != NULL)
	|| (StrStr(message, "can't alloc") != NULL)
	|| (StrStr(message, "Permission denied") != NULL)) {

	cps->maybeThinking = FALSE;
	sprintf(buf1, _("Failed to start %s chess program %s on %s: %s\n"),
		cps->which, cps->program, cps->host, message);
	RemoveInputSource(cps->isr);
	DisplayFatalError(buf1, 0, 1);
	return;
    }

    /*
     * Look for hint output
     */
    if (sscanf(message, "Hint: %s", buf1) == 1) {
	if (cps == &first && hintRequested) {
	    hintRequested = FALSE;
	    if (ParseOneMove(buf1, forwardMostMove, &moveType,
				 &fromX, &fromY, &toX, &toY, &promoChar)) {
		(void) CoordsToAlgebraic(boards[forwardMostMove],
				    PosFlags(forwardMostMove), EP_UNKNOWN,
				    fromY, fromX, toY, toX, promoChar, buf1);
		sprintf(buf2, "Hint: %s", buf1);
		DisplayInformation(buf2);
	    } else {
		/* Hint move could not be parsed!? */
		sprintf(buf2,
			_("Illegal hint move \"%s\"\nfrom %s chess program"),
			buf1, cps->which);
		DisplayError(buf2, 0);
	    }
	} else {
	    strcpy(lastHint, buf1);
	}
	return;
    }

    /*
     * Ignore other messages if game is not in progress
     */
    if (gameMode == BeginningOfGame || gameMode == EndOfGame ||
	gameMode == IcsIdle || cps->lastPing != cps->lastPong) return;

    /*
     * look for win, lose, draw, or draw offer
     */
    if (strncmp(message, "1-0", 3) == 0) {
	char *p, *q, *r = "";
        p = strchr(message, '{');
	if (p) {
	    q = strchr(p, '}');
	    if (q) {
		*q = NULLCHAR;
		r = p + 1;
	    }
	}
	GameEnds(WhiteWins, r, GE_ENGINE);
	return;
    } else if (strncmp(message, "0-1", 3) == 0) {
	char *p, *q, *r = "";
        p = strchr(message, '{');
	if (p) {
	    q = strchr(p, '}');
	    if (q) {
		*q = NULLCHAR;
		r = p + 1;
	    }
	}
	/* Kludge for Arasan 4.1 bug */
	if (strcmp(r, "Black resigns") == 0) {
	    GameEnds(WhiteWins, r, GE_ENGINE);
	    return;
	}
	GameEnds(BlackWins, r, GE_ENGINE);
	return;
    } else if (strncmp(message, "1/2", 3) == 0) {
	char *p, *q, *r = "";
        p = strchr(message, '{');
	if (p) {
	    q = strchr(p, '}');
	    if (q) {
		*q = NULLCHAR;
		r = p + 1;
	    }
	}
	GameEnds(GameIsDrawn, r, GE_ENGINE);
	return;

    } else if (strncmp(message, "White resign", 12) == 0) {
	GameEnds(BlackWins, "White resigns", GE_ENGINE);
	return;
    } else if (strncmp(message, "Black resign", 12) == 0) {
	GameEnds(WhiteWins, "Black resigns", GE_ENGINE);
	return;
    } else if (strncmp(message, "White", 5) == 0 &&
	       message[5] != '(' &&
	       StrStr(message, "Black") == NULL) {
	GameEnds(WhiteWins, "White mates", GE_ENGINE);
	return;
    } else if (strncmp(message, "Black", 5) == 0 &&
	       message[5] != '(') {
	GameEnds(BlackWins, "Black mates", GE_ENGINE);
	return;
    } else if (strcmp(message, "resign") == 0 ||
	       strcmp(message, "computer resigns") == 0) {
	switch (gameMode) {
	  case MachinePlaysBlack:
	  case IcsPlayingBlack:
	    GameEnds(WhiteWins, "Black resigns", GE_ENGINE);
	    break;
	  case MachinePlaysWhite:
	  case IcsPlayingWhite:
	    GameEnds(BlackWins, "White resigns", GE_ENGINE);
	    break;
	  case TwoMachinesPlay:
	    if (cps->twoMachinesColor[0] == 'w')
	      GameEnds(BlackWins, "White resigns", GE_ENGINE);
	    else
	      GameEnds(WhiteWins, "Black resigns", GE_ENGINE);
	    break;
	  default:
	    /* can't happen */
	    break;
	}
	return;
    } else if (strncmp(message, "opponent mates", 14) == 0) {
	switch (gameMode) {
	  case MachinePlaysBlack:
	  case IcsPlayingBlack:
	    GameEnds(WhiteWins, "White mates", GE_ENGINE);
	    break;
	  case MachinePlaysWhite:
	  case IcsPlayingWhite:
	    GameEnds(BlackWins, "Black mates", GE_ENGINE);
	    break;
	  case TwoMachinesPlay:
	    if (cps->twoMachinesColor[0] == 'w')
	      GameEnds(BlackWins, "Black mates", GE_ENGINE);
	    else
	      GameEnds(WhiteWins, "White mates", GE_ENGINE);
	    break;
	  default:
	    /* can't happen */
	    break;
	}
	return;
    } else if (strncmp(message, "computer mates", 14) == 0) {
	switch (gameMode) {
	  case MachinePlaysBlack:
	  case IcsPlayingBlack:
	    GameEnds(BlackWins, "Black mates", GE_ENGINE);
	    break;
	  case MachinePlaysWhite:
	  case IcsPlayingWhite:
	    GameEnds(WhiteWins, "White mates", GE_ENGINE);
	    break;
	  case TwoMachinesPlay:
	    if (cps->twoMachinesColor[0] == 'w')
	      GameEnds(WhiteWins, "White mates", GE_ENGINE);
	    else
	      GameEnds(BlackWins, "Black mates", GE_ENGINE);
	    break;
	  default:
	    /* can't happen */
	    break;
	}
	return;
    } else if (strncmp(message, "checkmate", 9) == 0) {
	if (WhiteOnMove(forwardMostMove)) {
	    GameEnds(BlackWins, "Black mates", GE_ENGINE);
	} else {
	    GameEnds(WhiteWins, "White mates", GE_ENGINE);
	}
	return;
    } else if (strstr(message, "Draw") != NULL ||
	       strstr(message, "game is a draw") != NULL) {
	GameEnds(GameIsDrawn, "Draw", GE_ENGINE);
	return;
    } else if (strstr(message, "offer") != NULL &&
	       strstr(message, "draw") != NULL) {
#if ZIPPY
	if (appData.zippyPlay && first.initDone) {
	    /* Relay offer to ICS */
	    SendToICS(ics_prefix);
	    SendToICS("draw\n");
	}
#endif
	cps->offeredDraw = 2; /* valid until this engine moves twice */
	if (gameMode == TwoMachinesPlay) {
	    if (cps->other->offeredDraw) {
		GameEnds(GameIsDrawn, "Draw agreed", GE_XBOARD);
	    } else {
	        if (cps->other->sendDrawOffers) {
		    SendToProgram("draw\n", cps->other);
		}
	    }
	} else if (gameMode == MachinePlaysWhite ||
		   gameMode == MachinePlaysBlack) {
	  if (userOfferedDraw) {
	    DisplayInformation(_("Machine accepts your draw offer"));
	    GameEnds(GameIsDrawn, "Draw agreed", GE_XBOARD);
	  } else {
            DisplayInformation(_("Machine offers a draw\nSelect Action / Draw to agree"));
	  }
	}
    }


    /*
     * Look for thinking output
     */
    if (appData.showThinking) {
	int plylev, mvleft, mvtot, curscore, time;
	char mvname[MOVE_LEN];
	u64 nodes;
	char plyext;
	int ignore = FALSE;
	int prefixHint = FALSE;
	mvname[0] = NULLCHAR;

	switch (gameMode) {
	  case MachinePlaysBlack:
	  case IcsPlayingBlack:
	    if (WhiteOnMove(forwardMostMove)) prefixHint = TRUE;
	    break;
	  case MachinePlaysWhite:
	  case IcsPlayingWhite:
	    if (!WhiteOnMove(forwardMostMove)) prefixHint = TRUE;
	    break;
	  case AnalyzeMode:
	  case AnalyzeFile:
	  /* icsEngineAnalyze */
	  case IcsObserving:
		if (!appData.icsEngineAnalyze) ignore = TRUE;
	    break;
	  case TwoMachinesPlay:
	    if ((cps->twoMachinesColor[0] == 'w') !=
		WhiteOnMove(forwardMostMove)) {
		ignore = TRUE;
	    }
	    break;
	  default:
	    ignore = TRUE;
	    break;
	}

	if (!ignore) {
	    buf1[0] = NULLCHAR;
	    if (sscanf(message, "%d%c %d %d" u64Display "%[^\n]\n",
		       &plylev, &plyext, &curscore, &time, &nodes, buf1) >= 5) {

		if (plyext != ' ' && plyext != '\t') {
		    time *= 100;
		}
		programStats.depth = plylev;
		programStats.nodes = nodes;
		programStats.time = time;
		programStats.score = curscore;
		programStats.got_only_move = 0;

		/* Buffer overflow protection */
		if (buf1[0] != NULLCHAR) {
		    if (strlen(buf1) >= sizeof(programStats.movelist)
			&& appData.debugMode) {
			fprintf(debugFP,
				"PV is too long; using the first %d bytes.\n",
				sizeof(programStats.movelist) - 1);
		    }
		    strncpy(programStats.movelist, buf1,
			    sizeof(programStats.movelist));
		    programStats.movelist[sizeof(programStats.movelist) - 1]
		      = NULLCHAR;
		} else {
		    sprintf(programStats.movelist, " no PV\n");
		}

		if (programStats.seen_stat) {
		    programStats.ok_to_send = 1;
		}

		if (strchr(programStats.movelist, '(') != NULL) {
		    programStats.line_is_book = 1;
		    programStats.nr_moves = 0;
		    programStats.moves_left = 0;
		} else {
		    programStats.line_is_book = 0;
		}

		sprintf(thinkOutput, "[%d]%c%+.2f %s%s%s",
			plylev,
			(gameMode == TwoMachinesPlay ?
			 ToUpper(cps->twoMachinesColor[0]) : ' '),
			((double) curscore) / 100.0,
			prefixHint ? lastHint : "",
			prefixHint ? " " : "", buf1);

		if (currentMove == forwardMostMove || gameMode == AnalyzeMode
			|| gameMode == AnalyzeFile || appData.icsEngineAnalyze) {
		    DisplayMove(currentMove - 1);
		    DisplayAnalysis();
		}
		return;

	    } else if ((p=StrStr(message, "(only move)")) != NULL) {
		/* crafty (9.25+) says "(only move) <move>"
		 * if there is only 1 legal move
                 */
		sscanf(p, "(only move) %s", buf1);
		sprintf(thinkOutput, "%s (only move)", buf1);
		sprintf(programStats.movelist, "%s (only move)", buf1);
		programStats.depth = 1;
		programStats.nr_moves = 1;
		programStats.moves_left = 1;
		programStats.nodes = 1;
		programStats.time = 1;
		programStats.got_only_move = 1;

		/* Not really, but we also use this member to
		   mean "line isn't going to change" (Crafty
		   isn't searching, so stats won't change) */
		programStats.line_is_book = 1;

		if (currentMove == forwardMostMove || gameMode==AnalyzeMode ||
		    gameMode == AnalyzeFile || appData.icsEngineAnalyze) {
		    DisplayMove(currentMove - 1);
		    DisplayAnalysis();
		}
		return;
	    } else if (sscanf(message,"stat01: %d" u64Display "%d %d %d %s",
			      &time, &nodes, &plylev, &mvleft,
			      &mvtot, mvname) >= 5) {
		/* The stat01: line is from Crafty (9.29+) in response
		   to the "." command */
		programStats.seen_stat = 1;
		cps->maybeThinking = TRUE;

		if (programStats.got_only_move || !appData.periodicUpdates)
		  return;

		programStats.depth = plylev;
		programStats.time = time;
		programStats.nodes = nodes;
		programStats.moves_left = mvleft;
		programStats.nr_moves = mvtot;
		strcpy(programStats.move_name, mvname);
		programStats.ok_to_send = 1;
		DisplayAnalysis();
		return;

	    } else if (strncmp(message,"++",2) == 0) {
		/* Crafty 9.29+ outputs this */
		programStats.got_fail = 2;
		return;

	    } else if (strncmp(message,"--",2) == 0) {
		/* Crafty 9.29+ outputs this */
		programStats.got_fail = 1;
		return;

	    } else if (thinkOutput[0] != NULLCHAR &&
		       strncmp(message, "    ", 4) == 0) {
	        p = message;
		while (*p && *p == ' ') p++;
		strcat(thinkOutput, " ");
		strcat(thinkOutput, p);
		strcat(programStats.movelist, " ");
		strcat(programStats.movelist, p);
		if (currentMove == forwardMostMove || gameMode==AnalyzeMode ||
		    gameMode == AnalyzeFile || appData.icsEngineAnalyze) {
		    DisplayMove(currentMove - 1);
		    DisplayAnalysis();
		}
		return;
	    }
	}
    }
}


/* Parse a game score from the character string "game", and
   record it as the history of the current game.  The game
   score is NOT assumed to start from the standard position.
   The display is not updated in any way.
   */
void
ParseGameHistory(game)
     char *game;
{
    ChessMove moveType;
    int fromX, fromY, toX, toY, boardIndex;
    char promoChar;
    char *p, *q;
    char buf[MSG_SIZ];

    if (appData.debugMode)
      fprintf(debugFP, "Parsing game history: %s\n", game);

    if (gameInfo.event == NULL) gameInfo.event = StrSave("ICS game");
    gameInfo.site = StrSave(appData.icsHost);
    gameInfo.date = PGNDate();
    gameInfo.round = StrSave("-");

    /* Parse out names of players */
    while (*game == ' ') game++;
    p = buf;
    while (*game != ' ') *p++ = *game++;
    *p = NULLCHAR;
    gameInfo.white = StrSave(buf);
    while (*game == ' ') game++;
    p = buf;
    while (*game != ' ' && *game != '\n') *p++ = *game++;
    *p = NULLCHAR;
    gameInfo.black = StrSave(buf);

    /* Parse moves */
    boardIndex = blackPlaysFirst ? 1 : 0;
    yynewstr(game);
    for (;;) {
	yyboardindex = boardIndex;
	moveType = (ChessMove) yylex();
	switch (moveType) {
	  case WhitePromotionQueen:
	  case BlackPromotionQueen:
	  case WhitePromotionRook:
	  case BlackPromotionRook:
	  case WhitePromotionBishop:
	  case BlackPromotionBishop:
	  case WhitePromotionKnight:
	  case BlackPromotionKnight:
	  case WhitePromotionKing:
	  case BlackPromotionKing:
	  case NormalMove:
	  case WhiteCapturesEnPassant:
	  case BlackCapturesEnPassant:
	  case WhiteKingSideCastle:
	  case WhiteQueenSideCastle:
	  case BlackKingSideCastle:
	  case BlackQueenSideCastle:
	  case WhiteKingSideCastleWild:
	  case WhiteQueenSideCastleWild:
	  case BlackKingSideCastleWild:
	  case BlackQueenSideCastleWild:
	  case IllegalMove:		/* maybe suicide chess, etc. */
	    fromX = currentMoveString[0] - 'a';
	    fromY = currentMoveString[1] - '1';
	    toX = currentMoveString[2] - 'a';
	    toY = currentMoveString[3] - '1';
	    promoChar = currentMoveString[4];
	    break;
	  case WhiteDrop:
	  case BlackDrop:
	    fromX = moveType == WhiteDrop ?
	      (int) CharToPiece(ToUpper(currentMoveString[0])) :
	    (int) CharToPiece(ToLower(currentMoveString[0]));
	    fromY = DROP_RANK;
	    toX = currentMoveString[2] - 'a';
	    toY = currentMoveString[3] - '1';
	    promoChar = NULLCHAR;
	    break;
	  case AmbiguousMove:
	    /* bug? */
	    sprintf(buf, _("Ambiguous move in ICS output: \"%s\""), yy_text);
	    DisplayError(buf, 0);
	    return;
	  case ImpossibleMove:
	    /* bug? */
	    sprintf(buf, _("Illegal move in ICS output: \"%s\""), yy_text);
	    DisplayError(buf, 0);
	    return;
	  case (ChessMove) 0:	/* end of file */
	    if (boardIndex < backwardMostMove) {
		/* Oops, gap.  How did that happen? */
		DisplayError(_("Gap in move list"), 0);
		return;
	    }
	    backwardMostMove =  blackPlaysFirst ? 1 : 0;
	    if (boardIndex > forwardMostMove) {
		forwardMostMove = boardIndex;
	    }
	    return;
	  case ElapsedTime:
	    if (boardIndex > (blackPlaysFirst ? 1 : 0)) {
		strcat(parseList[boardIndex-1], " ");
		strcat(parseList[boardIndex-1], yy_text);
	    }
	    continue;
	  case Comment:
	  case PGNTag:
	  case NAG:
	  default:
	    /* ignore */
	    continue;
	  case WhiteWins:
	  case BlackWins:
	  case GameIsDrawn:
	  case GameUnfinished:
	    if (gameMode == IcsExamining) {
		if (boardIndex < backwardMostMove) {
		    /* Oops, gap.  How did that happen? */
		    return;
		}
		backwardMostMove = blackPlaysFirst ? 1 : 0;
		return;
	    }
	    gameInfo.result = moveType;
	    p = strchr(yy_text, '{');
	    if (p == NULL) p = strchr(yy_text, '(');
	    if (p == NULL) {
		p = yy_text;
		if (p[0] == '0' || p[0] == '1' || p[0] == '*') p = "";
	    } else {
		q = strchr(p, *p == '{' ? '}' : ')');
		if (q != NULL) *q = NULLCHAR;
		p++;
	    }
	    gameInfo.resultDetails = StrSave(p);
	    continue;
	}
	if (boardIndex >= forwardMostMove &&
	    !(gameMode == IcsObserving && ics_gamenum == -1)) {
	    backwardMostMove = blackPlaysFirst ? 1 : 0;
	    return;
	}
	(void) CoordsToAlgebraic(boards[boardIndex], PosFlags(boardIndex),
				 EP_UNKNOWN, fromY, fromX, toY, toX, promoChar,
				 parseList[boardIndex]);
	CopyBoard(boards[boardIndex + 1], boards[boardIndex]);
	/* currentMoveString is set as a side-effect of yylex */
	strcpy(moveList[boardIndex], currentMoveString);
	strcat(moveList[boardIndex], "\n");
	boardIndex++;
	ApplyMove(fromX, fromY, toX, toY, promoChar, boards[boardIndex]);
	switch (MateTest(boards[boardIndex],
			 PosFlags(boardIndex), EP_UNKNOWN)) {
	  case MT_NONE:
	  case MT_STALEMATE:
	  default:
	    break;
	  case MT_CHECK:
	    strcat(parseList[boardIndex - 1], "+");
	    break;
	  case MT_CHECKMATE:
	    strcat(parseList[boardIndex - 1], "#");
	    break;
	}
    }
}


/* Apply a move to the given board  */
void
ApplyMove(fromX, fromY, toX, toY, promoChar, board)
     int fromX, fromY, toX, toY;
     int promoChar;
     Board board;
{
    ChessSquare captured = board[toY][toX];
    if (fromY == DROP_RANK) {
	/* must be first */
	board[toY][toX] = (ChessSquare) fromX;
    } else if (fromX == toX && fromY == toY) {
	return;
    } else if (fromY == 0 && fromX == 4
	&& board[fromY][fromX] == WhiteKing
	&& toY == 0 && toX == 6) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = WhiteKing;
	board[fromY][7] = EmptySquare;
	board[toY][5] = WhiteRook;
    } else if (fromY == 0 && fromX == 4
	       && board[fromY][fromX] == WhiteKing
	       && toY == 0 && toX == 2) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = WhiteKing;
	board[fromY][0] = EmptySquare;
	board[toY][3] = WhiteRook;
    } else if (fromY == 0 && fromX == 3
	       && board[fromY][fromX] == WhiteKing
	       && toY == 0 && toX == 5) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = WhiteKing;
	board[fromY][7] = EmptySquare;
	board[toY][4] = WhiteRook;
    } else if (fromY == 0 && fromX == 3
	       && board[fromY][fromX] == WhiteKing
	       && toY == 0 && toX == 1) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = WhiteKing;
	board[fromY][0] = EmptySquare;
	board[toY][2] = WhiteRook;
    } else if (board[fromY][fromX] == WhitePawn
	       && toY == 7) {
	/* white pawn promotion */
	board[7][toX] = CharToPiece(ToUpper(promoChar));
	if (board[7][toX] == EmptySquare) {
	    board[7][toX] = WhiteQueen;
	}
	board[fromY][fromX] = EmptySquare;
    } else if ((fromY == 4)
	       && (toX != fromX)
	       && (board[fromY][fromX] == WhitePawn)
	       && (board[toY][toX] == EmptySquare)) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = WhitePawn;
	captured = board[toY - 1][toX];
	board[toY - 1][toX] = EmptySquare;
    } else if (fromY == 7 && fromX == 4
	       && board[fromY][fromX] == BlackKing
	       && toY == 7 && toX == 6) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = BlackKing;
	board[fromY][7] = EmptySquare;
	board[toY][5] = BlackRook;
    } else if (fromY == 7 && fromX == 4
	       && board[fromY][fromX] == BlackKing
	       && toY == 7 && toX == 2) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = BlackKing;
	board[fromY][0] = EmptySquare;
	board[toY][3] = BlackRook;
    } else if (fromY == 7 && fromX == 3
	       && board[fromY][fromX] == BlackKing
	       && toY == 7 && toX == 5) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = BlackKing;
	board[fromY][7] = EmptySquare;
	board[toY][4] = BlackRook;
    } else if (fromY == 7 && fromX == 3
	       && board[fromY][fromX] == BlackKing
	       && toY == 7 && toX == 1) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = BlackKing;
	board[fromY][0] = EmptySquare;
	board[toY][2] = BlackRook;
    } else if (board[fromY][fromX] == BlackPawn
	       && toY == 0) {
	/* black pawn promotion */
	board[0][toX] = CharToPiece(ToLower(promoChar));
	if (board[0][toX] == EmptySquare) {
	    board[0][toX] = BlackQueen;
	}
	board[fromY][fromX] = EmptySquare;
    } else if ((fromY == 3)
	       && (toX != fromX)
	       && (board[fromY][fromX] == BlackPawn)
	       && (board[toY][toX] == EmptySquare)) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = BlackPawn;
	captured = board[toY + 1][toX];
	board[toY + 1][toX] = EmptySquare;
    } else {
	board[toY][toX] = board[fromY][fromX];
	board[fromY][fromX] = EmptySquare;
    }
    if (gameInfo.variant == VariantCrazyhouse) {
#if 0
      /* !!A lot more code needs to be written to support holdings */
      if (fromY == DROP_RANK) {
	/* Delete from holdings */
	if (holdings[(int) fromX] > 0) holdings[(int) fromX]--;
      }
      if (captured != EmptySquare) {
	/* Add to holdings */
	if (captured < BlackPawn) {
	  holdings[(int)captured - (int)BlackPawn + (int)WhitePawn]++;
	} else {
	  holdings[(int)captured - (int)WhitePawn + (int)BlackPawn]++;
	}
      }
#endif
    } else if (gameInfo.variant == VariantAtomic) {
      if (captured != EmptySquare) {
	int y, x;
	for (y = toY-1; y <= toY+1; y++) {
	  for (x = toX-1; x <= toX+1; x++) {
	    if (y >= 0 && y <= 7 && x >= 0 && x <= 7 &&
		board[y][x] != WhitePawn && board[y][x] != BlackPawn) {
	      board[y][x] = EmptySquare;
	    }
	  }
	}
	board[toY][toX] = EmptySquare;
      }
    }
}

/* Updates forwardMostMove */
void
MakeMove(fromX, fromY, toX, toY, promoChar)
     int fromX, fromY, toX, toY;
     int promoChar;
{
    forwardMostMove++;
    if (forwardMostMove >= MAX_MOVES) {
      DisplayFatalError(_("Game too long; increase MAX_MOVES and recompile"),
			0, 1);
      return;
    }
    SwitchClocks();
    timeRemaining[0][forwardMostMove] = whiteTimeRemaining;
    timeRemaining[1][forwardMostMove] = blackTimeRemaining;
    if (commentList[forwardMostMove] != NULL) {
	free(commentList[forwardMostMove]);
	commentList[forwardMostMove] = NULL;
    }
    CopyBoard(boards[forwardMostMove], boards[forwardMostMove - 1]);
    ApplyMove(fromX, fromY, toX, toY, promoChar, boards[forwardMostMove]);
    gameInfo.result = GameUnfinished;
    if (gameInfo.resultDetails != NULL) {
	free(gameInfo.resultDetails);
	gameInfo.resultDetails = NULL;
    }
    CoordsToComputerAlgebraic(fromY, fromX, toY, toX, promoChar,
			      moveList[forwardMostMove - 1]);
    (void) CoordsToAlgebraic(boards[forwardMostMove - 1],
			     PosFlags(forwardMostMove - 1), EP_UNKNOWN,
			     fromY, fromX, toY, toX, promoChar,
			     parseList[forwardMostMove - 1]);
    switch (MateTest(boards[forwardMostMove],
		     PosFlags(forwardMostMove), EP_UNKNOWN)){
      case MT_NONE:
      case MT_STALEMATE:
      default:
	break;
      case MT_CHECK:
	strcat(parseList[forwardMostMove - 1], "+");
	break;
      case MT_CHECKMATE:
	strcat(parseList[forwardMostMove - 1], "#");
	break;
    }
}

/* Updates currentMove if not pausing */
void
ShowMove(fromX, fromY, toX, toY)
{
    int instant = (gameMode == PlayFromGameFile) ?
	(matchMode || (appData.timeDelay == 0 && !pausing)) : pausing;
    if (!pausing || gameMode == PlayFromGameFile || gameMode == AnalyzeFile) {
	if (!instant) {
	    if (forwardMostMove == currentMove + 1) {
		AnimateMove(boards[forwardMostMove - 1],
			    fromX, fromY, toX, toY);
	    }
	    if (appData.highlightLastMove) {
		SetHighlights(fromX, fromY, toX, toY);
	    }
	}
	currentMove = forwardMostMove;
    }

    if (instant) return;
    DisplayMove(currentMove - 1);
    DrawPosition(FALSE, boards[currentMove]);
    DisplayBothClocks();
    HistorySet(parseList,backwardMostMove,forwardMostMove,currentMove-1);
}


void
InitChessProgram(cps)
     ChessProgramState *cps;
{
    char buf[MSG_SIZ];
    if (appData.noChessProgram) return;
    hintRequested = FALSE;
    bookRequested = FALSE;
    SendToProgram(cps->initString, cps);
    if (gameInfo.variant != VariantNormal &&
	gameInfo.variant != VariantLoadable) {
      char *v = VariantName(gameInfo.variant);
      if (StrStr(cps->variants, v) == NULL) {
	sprintf(buf, _("Variant %s not supported by %s"), v, cps->tidy);
	DisplayFatalError(buf, 0, 1);
	return;
      }
      sprintf(buf, "variant %s\n", VariantName(gameInfo.variant));
      SendToProgram(buf, cps);
    }
    if (cps->sendICS) {
      sprintf(buf, "ics %s\n", appData.icsActive ? appData.icsHost : "-");
      SendToProgram(buf, cps);
    }
    cps->maybeThinking = FALSE;
    cps->offeredDraw = 0;
    if (!appData.icsActive) {
	SendTimeControl(cps, movesPerSession, timeControl,
			timeIncrement, appData.searchDepth,
			searchTime);
    }
    if (appData.showThinking) {
	SendToProgram("post\n", cps);
    }
    SendToProgram("hard\n", cps);
    if (!appData.ponderNextMove) {
	/* Warning: "easy" is a toggle in GNU Chess, so don't send
	   it without being sure what state we are in first.  "hard"
	   is not a toggle, so that one is OK.
	 */
	SendToProgram("easy\n", cps);
    }
    if (cps->usePing) {
      sprintf(buf, "ping %d\n", ++cps->lastPing);
      SendToProgram(buf, cps);
    }
    cps->initDone = TRUE;
}


void
StartChessProgram(cps)
     ChessProgramState *cps;
{
    char buf[MSG_SIZ];
    int err;

    if (appData.noChessProgram) return;
    cps->initDone = FALSE;

    if (strcmp(cps->host, "localhost") == 0) {
	err = StartChildProcess(cps->program, cps->dir, &cps->pr);
    } else if (*appData.remoteShell == NULLCHAR) {
	err = OpenRcmd(cps->host, appData.remoteUser, cps->program, &cps->pr);
    } else {
	if (*appData.remoteUser == NULLCHAR) {
	    sprintf(buf, "%s %s %s", appData.remoteShell, cps->host,
		    cps->program);
	} else {
	    sprintf(buf, "%s %s -l %s %s", appData.remoteShell,
		    cps->host, appData.remoteUser, cps->program);
	}
	err = StartChildProcess(buf, "", &cps->pr);
    }

    if (err != 0) {
	sprintf(buf, "Startup failure on '%s'", cps->program);
	DisplayFatalError(buf, err, 1);
	cps->pr = NoProc;
	cps->isr = NULL;
	return;
    }

    cps->isr = AddInputSource(cps->pr, TRUE, ReceiveFromProgram, cps);
    if (cps->protocolVersion > 1) {
      sprintf(buf, "xboard\nprotover %d\n", cps->protocolVersion);
      SendToProgram(buf, cps);
    } else {
      SendToProgram("xboard\n", cps);
    }
}


void
TwoMachinesEventIfReady P((void))
{
  if (first.lastPing != first.lastPong) {
    DisplayMessage("", "Waiting for first chess program");
    ScheduleDelayedEvent(TwoMachinesEventIfReady, 1000);
    return;
  }
  if (second.lastPing != second.lastPong) {
    DisplayMessage("", "Waiting for second chess program");
    ScheduleDelayedEvent(TwoMachinesEventIfReady, 1000);
    return;
  }
  ThawUI();
  TwoMachinesEvent();
}

void
NextMatchGame P((void))
{
    Reset(FALSE, TRUE);
    if (*appData.loadGameFile != NULLCHAR) {
	LoadGameFromFile(appData.loadGameFile,
			 appData.loadGameIndex,
			 appData.loadGameFile, FALSE);
    } else if (*appData.loadPositionFile != NULLCHAR) {
	LoadPositionFromFile(appData.loadPositionFile,
			     appData.loadPositionIndex,
			     appData.loadPositionFile);
    }
    TwoMachinesEventIfReady();
}

void
GameEnds(result, resultDetails, whosays)
     ChessMove result;
     char *resultDetails;
     int whosays;
{
    GameMode nextGameMode;
    int isIcsGame;

    if (appData.debugMode) {
      fprintf(debugFP, "GameEnds(%d, %s, %d)\n",
	      result, resultDetails ? resultDetails : "(null)", whosays);
    }

    if (appData.icsActive && whosays == GE_ENGINE) {
	/* If we are playing on ICS, the server decides when the
	   game is over, but the engine can offer to draw, claim
	   a draw, or resign.
	 */
#if ZIPPY
	if (appData.zippyPlay && first.initDone) {
	    if (result == GameIsDrawn) {
		/* In case draw still needs to be claimed */
		SendToICS(ics_prefix);
		SendToICS("draw\n");
	    } else if (StrCaseStr(resultDetails, "resign")) {
		SendToICS(ics_prefix);
		SendToICS("resign\n");
	    }
        }
#endif
	return;
    }

    /* If we're loading the game from a file, stop */
    if (whosays == GE_FILE) {
      (void) StopLoadGameTimer();
      gameFileFP = NULL;
    }

    /* Cancel draw offers */
   first.offeredDraw = second.offeredDraw = 0;

    /* If this is an ICS game, only ICS can really say it's done;
       if not, anyone can. */
    isIcsGame = (gameMode == IcsPlayingWhite ||
	         gameMode == IcsPlayingBlack ||
		 gameMode == IcsObserving    ||
		 gameMode == IcsExamining);

    if (!isIcsGame || whosays == GE_ICS) {
	/* OK -- not an ICS game, or ICS said it was done */
	StopClocks();
	if (!isIcsGame && !appData.noChessProgram)
	  SetUserThinkingEnables();

	if (resultDetails != NULL) {
	    gameInfo.result = result;
	    gameInfo.resultDetails = StrSave(resultDetails);

	    /* Tell program how game ended in case it is learning */
	    if (gameMode == MachinePlaysWhite ||
		gameMode == MachinePlaysBlack ||
		gameMode == TwoMachinesPlay ||
		gameMode == IcsPlayingWhite ||
		gameMode == IcsPlayingBlack ||
		gameMode == BeginningOfGame) {
		char buf[MSG_SIZ];
		sprintf(buf, "result %s {%s}\n", PGNResult(result),
			resultDetails);
		if (first.pr != NoProc) {
		    SendToProgram(buf, &first);
		}
		if (second.pr != NoProc &&
		    gameMode == TwoMachinesPlay) {
		    SendToProgram(buf, &second);
		}
	    }

	    /* display last move only if game was not loaded from file */
	    if ((whosays != GE_FILE) && (currentMove == forwardMostMove))
		DisplayMove(currentMove - 1);

	    if (forwardMostMove != 0) {
		if (gameMode != PlayFromGameFile && gameMode != EditGame) {
		    if (*appData.saveGameFile != NULLCHAR) {
			SaveGameToFile(appData.saveGameFile, TRUE);
		    } else if (appData.autoSaveGames) {
			AutoSaveGame();
		    }
		    if (*appData.savePositionFile != NULLCHAR) {
			SavePositionToFile(appData.savePositionFile);
		    }
		}
	    }
	}

	if (appData.icsActive) {
	    if (appData.quietPlay &&
		(gameMode == IcsPlayingWhite ||
		 gameMode == IcsPlayingBlack)) {
		SendToICS(ics_prefix);
		SendToICS("set shout 1\n");
	    }
	    nextGameMode = IcsIdle;
	    ics_user_moved = FALSE;
	    /* clean up premove.  It's ugly when the game has ended and the
	     * premove highlights are still on the board.
	     */
	    if (gotPremove) {
	      gotPremove = FALSE;
	      ClearPremoveHighlights();
	      DrawPosition(FALSE, boards[currentMove]);
	    }
	    if (whosays == GE_ICS) {
		switch (result) {
		case WhiteWins:
		    if (gameMode == IcsPlayingWhite)
			PlayIcsWinSound();
		    else if(gameMode == IcsPlayingBlack)
			PlayIcsLossSound();
		    break;
		case BlackWins:
		    if (gameMode == IcsPlayingBlack)
			PlayIcsWinSound();
		    else if(gameMode == IcsPlayingWhite)
			PlayIcsLossSound();
		    break;
		case GameIsDrawn:
		    PlayIcsDrawSound();
		    break;
		default:
		    PlayIcsUnfinishedSound();
		}
	    }
	} else if (gameMode == EditGame ||
	           gameMode == PlayFromGameFile ||
	           gameMode == AnalyzeMode ||
		   gameMode == AnalyzeFile) {
	    nextGameMode = gameMode;
	} else {
	    nextGameMode = EndOfGame;
	}
	pausing = FALSE;
	ModeHighlight();
    } else {
	nextGameMode = gameMode;
    }

    if (appData.noChessProgram) {
	gameMode = nextGameMode;
	ModeHighlight();
	return;
    }

    if (first.reuse) {
	/* Put first chess program into idle state */
	if (first.pr != NoProc &&
	    (gameMode == MachinePlaysWhite ||
	     gameMode == MachinePlaysBlack ||
	     gameMode == TwoMachinesPlay ||
	     gameMode == IcsPlayingWhite ||
	     gameMode == IcsPlayingBlack ||
	     gameMode == BeginningOfGame)) {
	    SendToProgram("force\n", &first);
	    if (first.usePing) {
	      char buf[MSG_SIZ];
	      sprintf(buf, "ping %d\n", ++first.lastPing);
	      SendToProgram(buf, &first);
	    }
	}
    } else if (result != GameUnfinished || nextGameMode == IcsIdle) {
	/* Kill off first chess program */
	if (first.isr != NULL)
	  RemoveInputSource(first.isr);
	first.isr = NULL;

	if (first.pr != NoProc) {
	    ExitAnalyzeMode();
	    SendToProgram("quit\n", &first);
	    DestroyChildProcess(first.pr, first.useSigterm);
	}
	first.pr = NoProc;
    }
    if (second.reuse) {
	/* Put second chess program into idle state */
	if (second.pr != NoProc &&
	    gameMode == TwoMachinesPlay) {
	    SendToProgram("force\n", &second);
	    if (second.usePing) {
	      char buf[MSG_SIZ];
	      sprintf(buf, "ping %d\n", ++second.lastPing);
	      SendToProgram(buf, &second);
	    }
	}
    } else if (result != GameUnfinished || nextGameMode == IcsIdle) {
	/* Kill off second chess program */
	if (second.isr != NULL)
	  RemoveInputSource(second.isr);
	second.isr = NULL;

	if (second.pr != NoProc) {
	    SendToProgram("quit\n", &second);
	    DestroyChildProcess(second.pr, second.useSigterm);
	}
	second.pr = NoProc;
    }

    if (matchMode && gameMode == TwoMachinesPlay) {
        switch (result) {
	case WhiteWins:
	  if (first.twoMachinesColor[0] == 'w') {
	    first.matchWins++;
	  } else {
	    second.matchWins++;
	  }
	  break;
	case BlackWins:
	  if (first.twoMachinesColor[0] == 'b') {
	    first.matchWins++;
	  } else {
	    second.matchWins++;
	  }
	  break;
	default:
	  break;
	}
	if (matchGame < appData.matchGames) {
	    char *tmp;
	    tmp = first.twoMachinesColor;
	    first.twoMachinesColor = second.twoMachinesColor;
	    second.twoMachinesColor = tmp;
	    gameMode = nextGameMode;
	    matchGame++;
	    ScheduleDelayedEvent(NextMatchGame, 10000);
	    return;
	} else {
	    char buf[MSG_SIZ];
	    gameMode = nextGameMode;
	    sprintf(buf, _("Match %s vs. %s: final score %d-%d-%d"),
		    first.tidy, second.tidy,
		    first.matchWins, second.matchWins,
		    appData.matchGames - (first.matchWins + second.matchWins));
	    DisplayFatalError(buf, 0, 0);
	}
    }
    if ((gameMode == AnalyzeMode || gameMode == AnalyzeFile) &&
	!(nextGameMode == AnalyzeMode || nextGameMode == AnalyzeFile))
      ExitAnalyzeMode();
    gameMode = nextGameMode;
    ModeHighlight();
}

/* Assumes program was just initialized (initString sent).
   Leaves program in force mode. */
void
FeedMovesToProgram(cps, upto)
     ChessProgramState *cps;
     int upto;
{
    int i;

    if (appData.debugMode)
      fprintf(debugFP, "Feeding %smoves %d through %d to %s chess program\n",
	      startedFromSetupPosition ? "position and " : "",
	      backwardMostMove, upto, cps->which);
    SendToProgram("force\n", cps);
    if (startedFromSetupPosition) {
	SendBoard(cps, backwardMostMove);
    }
    for (i = backwardMostMove; i < upto; i++) {
	SendMoveToProgram(i, cps);
    }
}


void
ResurrectChessProgram()
{
     /* The chess program may have exited.
	If so, restart it and feed it all the moves made so far. */

    if (appData.noChessProgram || first.pr != NoProc) return;

    StartChessProgram(&first);
    InitChessProgram(&first);
    FeedMovesToProgram(&first, currentMove);

    if (!first.sendTime) {
	/* can't tell gnuchess what its clock should read,
	   so we bow to its notion. */
	ResetClocks();
	timeRemaining[0][currentMove] = whiteTimeRemaining;
	timeRemaining[1][currentMove] = blackTimeRemaining;
    }

    if ((gameMode == AnalyzeMode || gameMode == AnalyzeFile ||
		 appData.icsEngineAnalyze) && first.analysisSupport) {
      SendToProgram("analyze\n", &first);
      first.analyzing = TRUE;
    }
}

/*
 * Button procedures
 */
void
Reset(redraw, init)
     int redraw, init;
{
    int i;

    if (appData.debugMode) {
	fprintf(debugFP, "Reset(%d, %d) from gameMode %d\n",
		redraw, init, gameMode);
    }

    pausing = pauseExamInvalid = FALSE;
    startedFromSetupPosition = blackPlaysFirst = FALSE;
    firstMove = TRUE;
    whiteFlag = blackFlag = FALSE;
    userOfferedDraw = FALSE;
    hintRequested = bookRequested = FALSE;
    first.maybeThinking = FALSE;
    second.maybeThinking = FALSE;
    thinkOutput[0] = NULLCHAR;
    lastHint[0] = NULLCHAR;
    ClearGameInfo(&gameInfo);
    gameInfo.variant = StringToVariant(appData.variant);
    ics_user_moved = ics_clock_paused = FALSE;
    ics_getting_history = H_FALSE;
    ics_gamenum = -1;
    white_holding[0] = black_holding[0] = NULLCHAR;
    ClearProgramStats();

    ResetFrontEnd();
    ClearHighlights();
    flipView = appData.flipView;
    ClearPremoveHighlights();
    gotPremove = FALSE;
    alarmSounded = FALSE;

    GameEnds((ChessMove) 0, NULL, GE_PLAYER);
    ExitAnalyzeMode();
    gameMode = BeginningOfGame;
    ModeHighlight();
    InitPosition(redraw);
    for (i = 0; i < MAX_MOVES; i++) {
	if (commentList[i] != NULL) {
	    free(commentList[i]);
	    commentList[i] = NULL;
	}
    }
    ResetClocks();
    timeRemaining[0][0] = whiteTimeRemaining;
    timeRemaining[1][0] = blackTimeRemaining;
    if (first.pr == NULL) {
	StartChessProgram(&first);
    }
    if (init) InitChessProgram(&first);
    DisplayTitle("");
    DisplayMessage("", "");
    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);
}

void
AutoPlayGameLoop()
{
    for (;;) {
	if (!AutoPlayOneMove())
	  return;
	if (matchMode || appData.timeDelay == 0)
	  continue;
	if (appData.timeDelay < 0 || gameMode == AnalyzeFile)
	  return;
	StartLoadGameTimer((long)(1000.0 * appData.timeDelay));
	break;
    }
}


int
AutoPlayOneMove()
{
    int fromX, fromY, toX, toY;

    if (appData.debugMode) {
      fprintf(debugFP, "AutoPlayOneMove(): current %d\n", currentMove);
    }

    if (gameMode != PlayFromGameFile)
      return FALSE;

    if (currentMove >= forwardMostMove) {
      gameMode = EditGame;
      ModeHighlight();
      return FALSE;
    }

    toX = moveList[currentMove][2] - 'a';
    toY = moveList[currentMove][3] - '1';

    if (moveList[currentMove][1] == '@') {
	if (appData.highlightLastMove) {
	    SetHighlights(-1, -1, toX, toY);
	}
    } else {
	fromX = moveList[currentMove][0] - 'a';
	fromY = moveList[currentMove][1] - '1';
	AnimateMove(boards[currentMove], fromX, fromY, toX, toY);

	if (appData.highlightLastMove) {
	    SetHighlights(fromX, fromY, toX, toY);
	}
    }
    DisplayMove(currentMove);
    SendMoveToProgram(currentMove++, &first);
    DisplayBothClocks();
    DrawPosition(FALSE, boards[currentMove]);
    if (commentList[currentMove] != NULL) {
	DisplayComment(currentMove - 1, commentList[currentMove]);
    }
    return TRUE;
}


int
LoadGameOneMove(readAhead)
     ChessMove readAhead;
{
    int fromX = 0, fromY = 0, toX = 0, toY = 0, done;
    char promoChar = NULLCHAR;
    ChessMove moveType;
    char move[MSG_SIZ];
    char *p, *q;

    if (gameMode != PlayFromGameFile && gameMode != AnalyzeFile &&
	gameMode != AnalyzeMode && gameMode != Training) {
	gameFileFP = NULL;
	return FALSE;
    }

    yyboardindex = forwardMostMove;
    if (readAhead != (ChessMove)0) {
      moveType = readAhead;
    } else {
      if (gameFileFP == NULL)
	  return FALSE;
      moveType = (ChessMove) yylex();
    }

    done = FALSE;
    switch (moveType) {
      case Comment:
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed Comment: %s\n", yy_text);
	p = yy_text;
	if (*p == '{' || *p == '[' || *p == '(') {
	    p[strlen(p) - 1] = NULLCHAR;
	    p++;
	}

	/* append the comment but don't display it */
	while (*p == '\n') p++;
	AppendComment(currentMove, p);
	return TRUE;

      case WhiteCapturesEnPassant:
      case BlackCapturesEnPassant:
      case WhitePromotionQueen:
      case BlackPromotionQueen:
      case WhitePromotionRook:
      case BlackPromotionRook:
      case WhitePromotionBishop:
      case BlackPromotionBishop:
      case WhitePromotionKnight:
      case BlackPromotionKnight:
      case WhitePromotionKing:
      case BlackPromotionKing:
      case NormalMove:
      case WhiteKingSideCastle:
      case WhiteQueenSideCastle:
      case BlackKingSideCastle:
      case BlackQueenSideCastle:
      case WhiteKingSideCastleWild:
      case WhiteQueenSideCastleWild:
      case BlackKingSideCastleWild:
      case BlackQueenSideCastleWild:
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed %s into %s\n", yy_text, currentMoveString);
	fromX = currentMoveString[0] - 'a';
	fromY = currentMoveString[1] - '1';
	toX = currentMoveString[2] - 'a';
	toY = currentMoveString[3] - '1';
	promoChar = currentMoveString[4];
	break;

      case WhiteDrop:
      case BlackDrop:
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed %s into %s\n", yy_text, currentMoveString);
	fromX = moveType == WhiteDrop ?
	  (int) CharToPiece(ToUpper(currentMoveString[0])) :
	(int) CharToPiece(ToLower(currentMoveString[0]));
	fromY = DROP_RANK;
	toX = currentMoveString[2] - 'a';
	toY = currentMoveString[3] - '1';
	break;

      case WhiteWins:
      case BlackWins:
      case GameIsDrawn:
      case GameUnfinished:
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed game end: %s\n", yy_text);
	p = strchr(yy_text, '{');
	if (p == NULL) p = strchr(yy_text, '(');
	if (p == NULL) {
	    p = yy_text;
	    if (p[0] == '0' || p[0] == '1' || p[0] == '*') p = "";
	} else {
	    q = strchr(p, *p == '{' ? '}' : ')');
	    if (q != NULL) *q = NULLCHAR;
	    p++;
	}
	GameEnds(moveType, p, GE_FILE);
	done = TRUE;
	if (cmailMsgLoaded) {
 	    ClearHighlights();
	    flipView = WhiteOnMove(currentMove);
	    if (moveType == GameUnfinished) flipView = !flipView;
	    if (appData.debugMode)
	      fprintf(debugFP, "Setting flipView to %d\n", flipView) ;
	}
	break;

      case (ChessMove) 0:	/* end of file */
	if (appData.debugMode)
	  fprintf(debugFP, "Parser hit end of file\n");
	switch (MateTest(boards[currentMove], PosFlags(currentMove),
			 EP_UNKNOWN)) {
	  case MT_NONE:
	  case MT_CHECK:
	    break;
	  case MT_CHECKMATE:
	    if (WhiteOnMove(currentMove)) {
		GameEnds(BlackWins, "Black mates", GE_FILE);
	    } else {
		GameEnds(WhiteWins, "White mates", GE_FILE);
	    }
	    break;
	  case MT_STALEMATE:
	    GameEnds(GameIsDrawn, "Stalemate", GE_FILE);
	    break;
	}
	done = TRUE;
	break;

      case MoveNumberOne:
	if (lastLoadGameStart == GNUChessGame) {
	    /* GNUChessGames have numbers, but they aren't move numbers */
	    if (appData.debugMode)
	      fprintf(debugFP, "Parser ignoring: '%s' (%d)\n",
		      yy_text, (int) moveType);
	    return LoadGameOneMove((ChessMove)0); /* tail recursion */
	}
	/* else fall thru */

      case XBoardGame:
      case GNUChessGame:
      case PGNTag:
	/* Reached start of next game in file */
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed start of next game: %s\n", yy_text);
	switch (MateTest(boards[currentMove], PosFlags(currentMove),
			 EP_UNKNOWN)) {
	  case MT_NONE:
	  case MT_CHECK:
	    break;
	  case MT_CHECKMATE:
	    if (WhiteOnMove(currentMove)) {
		GameEnds(BlackWins, "Black mates", GE_FILE);
	    } else {
		GameEnds(WhiteWins, "White mates", GE_FILE);
	    }
	    break;
	  case MT_STALEMATE:
	    GameEnds(GameIsDrawn, "Stalemate", GE_FILE);
	    break;
	}
	done = TRUE;
	break;

      case PositionDiagram:	/* should not happen; ignore */
      case ElapsedTime:		/* ignore */
      case NAG:                 /* ignore */
	if (appData.debugMode)
	  fprintf(debugFP, "Parser ignoring: '%s' (%d)\n",
		  yy_text, (int) moveType);
	return LoadGameOneMove((ChessMove)0); /* tail recursion */

      case IllegalMove:
	if (appData.testLegality) {
	    if (appData.debugMode)
	      fprintf(debugFP, "Parsed IllegalMove: %s\n", yy_text);
	    sprintf(move, _("Illegal move: %d.%s%s"),
		    (forwardMostMove / 2) + 1,
		    WhiteOnMove(forwardMostMove) ? " " : ".. ", yy_text);
	    DisplayError(move, 0);
	    done = TRUE;
	} else {
	    if (appData.debugMode)
	      fprintf(debugFP, "Parsed %s into IllegalMove %s\n",
		      yy_text, currentMoveString);
	    fromX = currentMoveString[0] - 'a';
	    fromY = currentMoveString[1] - '1';
	    toX = currentMoveString[2] - 'a';
	    toY = currentMoveString[3] - '1';
	    promoChar = currentMoveString[4];
	}
	break;

      case AmbiguousMove:
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed AmbiguousMove: %s\n", yy_text);
	sprintf(move, _("Ambiguous move: %d.%s%s"),
		(forwardMostMove / 2) + 1,
		WhiteOnMove(forwardMostMove) ? " " : ".. ", yy_text);
	DisplayError(move, 0);
	done = TRUE;
	break;

      default:
      case ImpossibleMove:
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed ImpossibleMove: %s\n", yy_text);
	sprintf(move, _("Illegal move: %d.%s%s"),
		(forwardMostMove / 2) + 1,
		WhiteOnMove(forwardMostMove) ? " " : ".. ", yy_text);
	DisplayError(move, 0);
	done = TRUE;
	break;
    }

    if (done) {
	if (appData.matchMode || (appData.timeDelay == 0 && !pausing)) {
	    DrawPosition(FALSE, boards[currentMove]);
	    DisplayBothClocks();
	    if (!appData.matchMode && commentList[currentMove] != NULL)
	      DisplayComment(currentMove - 1, commentList[currentMove]);
	}
	(void) StopLoadGameTimer();
	gameFileFP = NULL;
	cmailOldMove = forwardMostMove;
	return FALSE;
    } else {
	/* currentMoveString is set as a side-effect of yylex */
	strcat(currentMoveString, "\n");
	strcpy(moveList[forwardMostMove], currentMoveString);

	thinkOutput[0] = NULLCHAR;
	MakeMove(fromX, fromY, toX, toY, promoChar);
	currentMove = forwardMostMove;
	return TRUE;
    }
}

/* Load the nth game from the given file */
int
LoadGameFromFile(filename, n, title, useList)
     char *filename;
     int n;
     char *title;
     /*Boolean*/ int useList;
{
    FILE *f;
    char buf[MSG_SIZ];

    if (strcmp(filename, "-") == 0) {
	f = stdin;
	title = "stdin";
    } else {
	f = fopen(filename, "rb");
	if (f == NULL) {
	    sprintf(buf, _("Can't open \"%s\""), filename);
	    DisplayError(buf, errno);
	    return FALSE;
	}
    }
    if (fseek(f, 0, 0) == -1) {
	/* f is not seekable; probably a pipe */
	useList = FALSE;
    }
    if (useList && n == 0) {
	int error = GameListBuild(f);
	if (error) {
	    DisplayError(_("Cannot build game list"), error);
	} else if (!ListEmpty(&gameList) &&
		   ((ListGame *) gameList.tailPred)->number > 1) {
	    GameListPopUp(f, title);
	    return TRUE;
	}
	GameListDestroy();
	n = 1;
    }
    if (n == 0) n = 1;
    return LoadGame(f, n, title, FALSE);
}


void
MakeRegisteredMove()
{
    int fromX, fromY, toX, toY;
    char promoChar;
    if (cmailMoveRegistered[lastLoadGameNumber - 1]) {
	switch (cmailMoveType[lastLoadGameNumber - 1]) {
	  case CMAIL_MOVE:
	  case CMAIL_DRAW:
	    if (appData.debugMode)
	      fprintf(debugFP, "Restoring %s for game %d\n",
		      cmailMove[lastLoadGameNumber - 1], lastLoadGameNumber);

	    thinkOutput[0] = NULLCHAR;
	    strcpy(moveList[currentMove], cmailMove[lastLoadGameNumber - 1]);
	    fromX = cmailMove[lastLoadGameNumber - 1][0] - 'a';
	    fromY = cmailMove[lastLoadGameNumber - 1][1] - '1';
	    toX = cmailMove[lastLoadGameNumber - 1][2] - 'a';
	    toY = cmailMove[lastLoadGameNumber - 1][3] - '1';
	    promoChar = cmailMove[lastLoadGameNumber - 1][4];
	    MakeMove(fromX, fromY, toX, toY, promoChar);
	    ShowMove(fromX, fromY, toX, toY);

	    switch (MateTest(boards[currentMove], PosFlags(currentMove),
			     EP_UNKNOWN)) {
	      case MT_NONE:
	      case MT_CHECK:
		break;

	      case MT_CHECKMATE:
		if (WhiteOnMove(currentMove)) {
		    GameEnds(BlackWins, "Black mates", GE_PLAYER);
		} else {
		    GameEnds(WhiteWins, "White mates", GE_PLAYER);
		}
		break;

	      case MT_STALEMATE:
		GameEnds(GameIsDrawn, "Stalemate", GE_PLAYER);
		break;
	    }

	    break;

	  case CMAIL_RESIGN:
	    if (WhiteOnMove(currentMove)) {
		GameEnds(BlackWins, "White resigns", GE_PLAYER);
	    } else {
		GameEnds(WhiteWins, "Black resigns", GE_PLAYER);
	    }
	    break;

	  case CMAIL_ACCEPT:
	    GameEnds(GameIsDrawn, "Draw agreed", GE_PLAYER);
	    break;

	  default:
	    break;
	}
    }

    return;
}

/* Wrapper around LoadGame for use when a Cmail message is loaded */
int
CmailLoadGame(f, gameNumber, title, useList)
     FILE *f;
     int gameNumber;
     char *title;
     int useList;
{
    int retVal;

    if (gameNumber > nCmailGames) {
	DisplayError(_("No more games in this message"), 0);
	return FALSE;
    }
    if (f == lastLoadGameFP) {
	int offset = gameNumber - lastLoadGameNumber;
	if (offset == 0) {
	    cmailMsg[0] = NULLCHAR;
	    if (cmailMoveRegistered[lastLoadGameNumber - 1]) {
		cmailMoveRegistered[lastLoadGameNumber - 1] = FALSE;
		nCmailMovesRegistered--;
	    }
	    cmailMoveType[lastLoadGameNumber - 1] = CMAIL_MOVE;
	    if (cmailResult[lastLoadGameNumber - 1] == CMAIL_NEW_RESULT) {
		cmailResult[lastLoadGameNumber - 1] = CMAIL_NOT_RESULT;
	    }
	} else {
	    if (! RegisterMove()) return FALSE;
	}
    }

    retVal = LoadGame(f, gameNumber, title, useList);

    /* Make move registered during previous look at this game, if any */
    MakeRegisteredMove();

    if (cmailCommentList[lastLoadGameNumber - 1] != NULL) {
	commentList[currentMove]
	  = StrSave(cmailCommentList[lastLoadGameNumber - 1]);
	DisplayComment(currentMove - 1, commentList[currentMove]);
    }

    return retVal;
}

/* Support for LoadNextGame, LoadPreviousGame, ReloadSameGame */
int
ReloadGame(offset)
     int offset;
{
    int gameNumber = lastLoadGameNumber + offset;
    if (lastLoadGameFP == NULL) {
	DisplayError(_("No game has been loaded yet"), 0);
	return FALSE;
    }
    if (gameNumber <= 0) {
	DisplayError(_("Can't back up any further"), 0);
	return FALSE;
    }
    if (cmailMsgLoaded) {
	return CmailLoadGame(lastLoadGameFP, gameNumber,
			     lastLoadGameTitle, lastLoadGameUseList);
    } else {
	return LoadGame(lastLoadGameFP, gameNumber,
			lastLoadGameTitle, lastLoadGameUseList);
    }
}



/* Load the nth game from open file f */
int
LoadGame(f, gameNumber, title, useList)
     FILE *f;
     int gameNumber;
     char *title;
     int useList;
{
    ChessMove cm;
    char buf[MSG_SIZ];
    int gn = gameNumber;
    ListGame *lg = NULL;
    int numPGNTags = 0;
    int err;
    GameMode oldGameMode;

    if (appData.debugMode)
	fprintf(debugFP, "LoadGame(): on entry, gameMode %d\n", gameMode);

    if (gameMode == Training )
	SetTrainingModeOff();

    oldGameMode = gameMode;
    if (gameMode != BeginningOfGame) {
      Reset(FALSE, TRUE);
    }

    gameFileFP = f;
    if (lastLoadGameFP != NULL && lastLoadGameFP != f) {
	fclose(lastLoadGameFP);
    }

    if (useList) {
	lg = (ListGame *) ListElem(&gameList, gameNumber-1);

	if (lg) {
	    fseek(f, lg->offset, 0);
	    GameListHighlight(gameNumber);
	    gn = 1;
	}
	else {
	    DisplayError(_("Game number out of range"), 0);
	    return FALSE;
	}
    } else {
	GameListDestroy();
	if (fseek(f, 0, 0) == -1) {
	    if (f == lastLoadGameFP ?
	 	gameNumber == lastLoadGameNumber + 1 :
		gameNumber == 1) {
		gn = 1;
	    } else {
		DisplayError(_("Can't seek on game file"), 0);
		return FALSE;
	    }
	}
    }
    lastLoadGameFP = f;
    lastLoadGameNumber = gameNumber;
    strcpy(lastLoadGameTitle, title);
    lastLoadGameUseList = useList;

    yynewfile(f);


    if (lg && lg->gameInfo.white && lg->gameInfo.black) {
	sprintf(buf, "%s vs. %s", lg->gameInfo.white,
		lg->gameInfo.black);
	    DisplayTitle(buf);
    } else if (*title != NULLCHAR) {
	if (gameNumber > 1) {
	    sprintf(buf, "%s %d", title, gameNumber);
	    DisplayTitle(buf);
	} else {
	    DisplayTitle(title);
	}
    }

    if (gameMode != AnalyzeFile && gameMode != AnalyzeMode) {
	gameMode = PlayFromGameFile;
	ModeHighlight();
    }

    currentMove = forwardMostMove = backwardMostMove = 0;
    CopyBoard(boards[0], initialPosition);
    StopClocks();

    /*
     * Skip the first gn-1 games in the file.
     * Also skip over anything that precedes an identifiable
     * start of game marker, to avoid being confused by
     * garbage at the start of the file.  Currently
     * recognized start of game markers are the move number "1",
     * the pattern "gnuchess .* game", the pattern
     * "^[#;%] [^ ]* game file", and a PGN tag block.
     * A game that starts with one of the latter two patterns
     * will also have a move number 1, possibly
     * following a position diagram.
     * 5-4-02: Let's try being more lenient and allowing a game to
     * start with an unnumbered move.  Does that break anything?
     */
    cm = lastLoadGameStart = (ChessMove) 0;
    while (gn > 0) {
	yyboardindex = forwardMostMove;
	cm = (ChessMove) yylex();
	switch (cm) {
	  case (ChessMove) 0:
	    if (cmailMsgLoaded) {
		nCmailGames = CMAIL_MAX_GAMES - gn;
	    } else {
		Reset(TRUE, TRUE);
		DisplayError(_("Game not found in file"), 0);
	    }
	    return FALSE;

	  case GNUChessGame:
	  case XBoardGame:
	    gn--;
	    lastLoadGameStart = cm;
	    break;

	  case MoveNumberOne:
	    switch (lastLoadGameStart) {
	      case GNUChessGame:
	      case XBoardGame:
	      case PGNTag:
		break;
	      case MoveNumberOne:
	      case (ChessMove) 0:
		gn--;		/* count this game */
		lastLoadGameStart = cm;
		break;
	      default:
		/* impossible */
		break;
	    }
	    break;

	  case PGNTag:
	    switch (lastLoadGameStart) {
	      case GNUChessGame:
	      case PGNTag:
	      case MoveNumberOne:
	      case (ChessMove) 0:
		gn--;		/* count this game */
		lastLoadGameStart = cm;
		break;
	      case XBoardGame:
		lastLoadGameStart = cm; /* game counted already */
		break;
	      default:
		/* impossible */
		break;
	    }
	    if (gn > 0) {
		do {
		    yyboardindex = forwardMostMove;
		    cm = (ChessMove) yylex();
		} while (cm == PGNTag || cm == Comment);
	    }
	    break;

	  case WhiteWins:
	  case BlackWins:
	  case GameIsDrawn:
	    if (cmailMsgLoaded && (CMAIL_MAX_GAMES == lastLoadGameNumber)) {
		if (   cmailResult[CMAIL_MAX_GAMES - gn - 1]
		    != CMAIL_OLD_RESULT) {
		    nCmailResults ++ ;
		    cmailResult[  CMAIL_MAX_GAMES
				- gn - 1] = CMAIL_OLD_RESULT;
		}
	    }
	    break;

	  case NormalMove:
	    /* Only a NormalMove can be at the start of a game
	     * without a position diagram. */
	    if (lastLoadGameStart == (ChessMove) 0) {
	      gn--;
	      lastLoadGameStart = MoveNumberOne;
	    }
	    break;

	  default:
	    break;
	}
    }

    if (appData.debugMode)
      fprintf(debugFP, "Parsed game start '%s' (%d)\n", yy_text, (int) cm);

    if (cm == XBoardGame) {
	/* Skip any header junk before position diagram and/or move 1 */
	for (;;) {
	    yyboardindex = forwardMostMove;
	    cm = (ChessMove) yylex();

	    if (cm == (ChessMove) 0 ||
		cm == GNUChessGame || cm == XBoardGame) {
		/* Empty game; pretend end-of-file and handle later */
		cm = (ChessMove) 0;
		break;
	    }

	    if (cm == MoveNumberOne || cm == PositionDiagram ||
		cm == PGNTag || cm == Comment)
	      break;
	}
    } else if (cm == GNUChessGame) {
	if (gameInfo.event != NULL) {
	    free(gameInfo.event);
	}
	gameInfo.event = StrSave(yy_text);
    }

    startedFromSetupPosition = FALSE;
    while (cm == PGNTag) {
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed PGNTag: %s\n", yy_text);
	err = ParsePGNTag(yy_text, &gameInfo);
	if (!err) numPGNTags++;

	if (gameInfo.fen != NULL) {
	  Board initial_position;
	  startedFromSetupPosition = TRUE;
	  if (!ParseFEN(initial_position, &blackPlaysFirst, gameInfo.fen)) {
	    Reset(TRUE, TRUE);
	    DisplayError(_("Bad FEN position in file"), 0);
	    return FALSE;
	  }
	  CopyBoard(boards[0], initial_position);
	  if (blackPlaysFirst) {
	    currentMove = forwardMostMove = backwardMostMove = 1;
	    CopyBoard(boards[1], initial_position);
	    strcpy(moveList[0], "");
	    strcpy(parseList[0], "");
	    timeRemaining[0][1] = whiteTimeRemaining;
	    timeRemaining[1][1] = blackTimeRemaining;
	    if (commentList[0] != NULL) {
	      commentList[1] = commentList[0];
	      commentList[0] = NULL;
	    }
	  } else {
	    currentMove = forwardMostMove = backwardMostMove = 0;
	  }
	  yyboardindex = forwardMostMove;
	  free(gameInfo.fen);
	  gameInfo.fen = NULL;
	}

	yyboardindex = forwardMostMove;
	cm = (ChessMove) yylex();

	/* Handle comments interspersed among the tags */
	while (cm == Comment) {
	    char *p;
	    if (appData.debugMode)
	      fprintf(debugFP, "Parsed Comment: %s\n", yy_text);
	    p = yy_text;
	    if (*p == '{' || *p == '[' || *p == '(') {
		p[strlen(p) - 1] = NULLCHAR;
		p++;
	    }
	    while (*p == '\n') p++;
	    AppendComment(currentMove, p);
	    yyboardindex = forwardMostMove;
	    cm = (ChessMove) yylex();
	}
    }

    /* don't rely on existence of Event tag since if game was
     * pasted from clipboard the Event tag may not exist
     */
    if (numPGNTags > 0){
        char *tags;
	if (gameInfo.variant == VariantNormal) {
	  gameInfo.variant = StringToVariant(gameInfo.event);
	}
	if (!matchMode) {
	  tags = PGNTags(&gameInfo);
	  TagsPopUp(tags, CmailMsg());
	  free(tags);
	}
    } else {
	/* Make something up, but don't display it now */
	SetGameInfo();
	TagsPopDown();
    }

    if (cm == PositionDiagram) {
	int i, j;
	char *p;
	Board initial_position;

	if (appData.debugMode)
	  fprintf(debugFP, "Parsed PositionDiagram: %s\n", yy_text);

	if (!startedFromSetupPosition) {
	    p = yy_text;
	    for (i = BOARD_SIZE - 1; i >= 0; i--)
	      for (j = 0; j < BOARD_SIZE; p++)
		switch (*p) {
		  case '[':
		  case '-':
		  case ' ':
		  case '\t':
		  case '\n':
		  case '\r':
		    break;
		  default:
		    initial_position[i][j++] = CharToPiece(*p);
		    break;
		}
	    while (*p == ' ' || *p == '\t' ||
		   *p == '\n' || *p == '\r') p++;

	    if (strncmp(p, "black", strlen("black"))==0)
	      blackPlaysFirst = TRUE;
	    else
	      blackPlaysFirst = FALSE;
	    startedFromSetupPosition = TRUE;

	    CopyBoard(boards[0], initial_position);
	    if (blackPlaysFirst) {
		currentMove = forwardMostMove = backwardMostMove = 1;
		CopyBoard(boards[1], initial_position);
		strcpy(moveList[0], "");
		strcpy(parseList[0], "");
		timeRemaining[0][1] = whiteTimeRemaining;
		timeRemaining[1][1] = blackTimeRemaining;
		if (commentList[0] != NULL) {
		    commentList[1] = commentList[0];
		    commentList[0] = NULL;
		}
	    } else {
		currentMove = forwardMostMove = backwardMostMove = 0;
	    }
	}
	yyboardindex = forwardMostMove;
	cm = (ChessMove) yylex();
    }

    if (first.pr == NoProc) {
	StartChessProgram(&first);
    }
    InitChessProgram(&first);
    SendToProgram("force\n", &first);
    if (startedFromSetupPosition) {
	SendBoard(&first, forwardMostMove);
	DisplayBothClocks();
    }

    while (cm == Comment) {
	char *p;
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed Comment: %s\n", yy_text);
	p = yy_text;
	if (*p == '{' || *p == '[' || *p == '(') {
	    p[strlen(p) - 1] = NULLCHAR;
	    p++;
	}
	while (*p == '\n') p++;
	AppendComment(currentMove, p);
	yyboardindex = forwardMostMove;
	cm = (ChessMove) yylex();
    }

    if ((cm == (ChessMove) 0 && lastLoadGameStart != (ChessMove) 0) ||
	cm == WhiteWins || cm == BlackWins ||
	cm == GameIsDrawn || cm == GameUnfinished) {
	DisplayMessage("", _("No moves in game"));
	if (cmailMsgLoaded) {
	    if (appData.debugMode)
	      fprintf(debugFP, "Setting flipView to %d.\n", FALSE);
 	    ClearHighlights();
	    flipView = FALSE;
	}
	DrawPosition(FALSE, boards[currentMove]);
	DisplayBothClocks();
	gameMode = EditGame;
	ModeHighlight();
	gameFileFP = NULL;
	cmailOldMove = 0;
	return TRUE;
    }

    if (commentList[currentMove] != NULL) {
      if (!matchMode && (pausing || appData.timeDelay != 0)) {
	DisplayComment(currentMove - 1, commentList[currentMove]);
      }
    }
    if (!matchMode && appData.timeDelay != 0)
      DrawPosition(FALSE, boards[currentMove]);

    if (gameMode == AnalyzeFile || gameMode == AnalyzeMode) {
      programStats.ok_to_send = 1;
    }

    /* if the first token after the PGN tags is a move
     * and not move number 1, retrieve it from the parser
     */
    if (cm != MoveNumberOne)
	LoadGameOneMove(cm);

    /* load the remaining moves from the file */
    while (LoadGameOneMove((ChessMove)0)) {
      timeRemaining[0][forwardMostMove] = whiteTimeRemaining;
      timeRemaining[1][forwardMostMove] = blackTimeRemaining;
    }

    /* rewind to the start of the game */
    currentMove = backwardMostMove;

    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);

    if (oldGameMode == AnalyzeFile ||
	oldGameMode == AnalyzeMode) {
      AnalyzeFileEvent();
    }

    if (matchMode || appData.timeDelay == 0) {
      ToEndEvent();
      gameMode = EditGame;
      ModeHighlight();
    } else if (appData.timeDelay > 0) {
      AutoPlayGameLoop();
    }

    if (appData.debugMode)
	fprintf(debugFP, "LoadGame(): on exit, gameMode %d\n", gameMode);
    return TRUE;
}

/* Support for LoadNextPosition, LoadPreviousPosition, ReloadSamePosition */
int
ReloadPosition(offset)
     int offset;
{
    int positionNumber = lastLoadPositionNumber + offset;
    if (lastLoadPositionFP == NULL) {
	DisplayError(_("No position has been loaded yet"), 0);
	return FALSE;
    }
    if (positionNumber <= 0) {
	DisplayError(_("Can't back up any further"), 0);
	return FALSE;
    }
    return LoadPosition(lastLoadPositionFP, positionNumber,
			lastLoadPositionTitle);
}

/* Load the nth position from the given file */
int
LoadPositionFromFile(filename, n, title)
     char *filename;
     int n;
     char *title;
{
    FILE *f;
    char buf[MSG_SIZ];

    if (strcmp(filename, "-") == 0) {
	return LoadPosition(stdin, n, "stdin");
    } else {
	f = fopen(filename, "rb");
	if (f == NULL) {
	    sprintf(buf, _("Can't open \"%s\""), filename);
	    DisplayError(buf, errno);
	    return FALSE;
	} else {
	    return LoadPosition(f, n, title);
	}
    }
}

/* Load the nth position from the given open file, and close it */
int
LoadPosition(f, positionNumber, title)
     FILE *f;
     int positionNumber;
     char *title;
{
    char *p, line[MSG_SIZ];
    Board initial_position;
    int i, j, fenMode, pn;

    if (gameMode == Training )
	SetTrainingModeOff();

    if (gameMode != BeginningOfGame) {
	Reset(FALSE, TRUE);
    }
    if (lastLoadPositionFP != NULL && lastLoadPositionFP != f) {
	fclose(lastLoadPositionFP);
    }
    if (positionNumber == 0) positionNumber = 1;
    lastLoadPositionFP = f;
    lastLoadPositionNumber = positionNumber;
    strcpy(lastLoadPositionTitle, title);
    if (first.pr == NoProc) {
      StartChessProgram(&first);
      InitChessProgram(&first);
    }
    pn = positionNumber;
    if (positionNumber < 0) {
	/* Negative position number means to seek to that byte offset */
	if (fseek(f, -positionNumber, 0) == -1) {
	    DisplayError(_("Can't seek on position file"), 0);
	    return FALSE;
	};
	pn = 1;
    } else {
	if (fseek(f, 0, 0) == -1) {
	    if (f == lastLoadPositionFP ?
		positionNumber == lastLoadPositionNumber + 1 :
		positionNumber == 1) {
		pn = 1;
	    } else {
		DisplayError(_("Can't seek on position file"), 0);
		return FALSE;
	    }
	}
    }
    /* See if this file is FEN or old-style xboard */
    if (fgets(line, MSG_SIZ, f) == NULL) {
	DisplayError(_("Position not found in file"), 0);
	return FALSE;
    }
    switch (line[0]) {
      case '#':  case 'x':
      default:
	fenMode = FALSE;
	break;
      case 'p':  case 'n':  case 'b':  case 'r':  case 'q':  case 'k':
      case 'P':  case 'N':  case 'B':  case 'R':  case 'Q':  case 'K':
      case '1':  case '2':  case '3':  case '4':  case '5':  case '6':
      case '7':  case '8':
	fenMode = TRUE;
	break;
    }

    if (pn >= 2) {
	if (fenMode || line[0] == '#') pn--;
	while (pn > 0) {
	    /* skip postions before number pn */
	    if (fgets(line, MSG_SIZ, f) == NULL) {
	        Reset(TRUE, TRUE);
		DisplayError(_("Position not found in file"), 0);
		return FALSE;
	    }
	    if (fenMode || line[0] == '#') pn--;
	}
    }

    if (fenMode) {
	if (!ParseFEN(initial_position, &blackPlaysFirst, line)) {
	    DisplayError(_("Bad FEN position in file"), 0);
	    return FALSE;
	}
    } else {
	(void) fgets(line, MSG_SIZ, f);
	(void) fgets(line, MSG_SIZ, f);

	for (i = BOARD_SIZE - 1; i >= 0; i--) {
	    (void) fgets(line, MSG_SIZ, f);
	    for (p = line, j = 0; j < BOARD_SIZE; p++) {
		if (*p == ' ')
		  continue;
		initial_position[i][j++] = CharToPiece(*p);
	    }
	}

	blackPlaysFirst = FALSE;
	if (!feof(f)) {
	    (void) fgets(line, MSG_SIZ, f);
	    if (strncmp(line, "black", strlen("black"))==0)
	      blackPlaysFirst = TRUE;
	}
    }
    startedFromSetupPosition = TRUE;

    SendToProgram("force\n", &first);
    CopyBoard(boards[0], initial_position);
    if (blackPlaysFirst) {
	currentMove = forwardMostMove = backwardMostMove = 1;
	strcpy(moveList[0], "");
	strcpy(parseList[0], "");
	CopyBoard(boards[1], initial_position);
	DisplayMessage("", _("Black to play"));
    } else {
	currentMove = forwardMostMove = backwardMostMove = 0;
	DisplayMessage("", _("White to play"));
    }
    SendBoard(&first, forwardMostMove);

    if (positionNumber > 1) {
	sprintf(line, "%s %d", title, positionNumber);
	DisplayTitle(line);
    } else {
	DisplayTitle(title);
    }
    gameMode = EditGame;
    ModeHighlight();
    ResetClocks();
    timeRemaining[0][1] = whiteTimeRemaining;
    timeRemaining[1][1] = blackTimeRemaining;
    DrawPosition(FALSE, boards[currentMove]);

    return TRUE;
}


void
CopyPlayerNameIntoFileName(dest, src)
     char **dest, *src;
{
    while (*src != NULLCHAR && *src != ',') {
	if (*src == ' ') {
	    *(*dest)++ = '_';
	    src++;
	} else {
	    *(*dest)++ = *src++;
	}
    }
}

char *DefaultFileName(ext)
     char *ext;
{
    static char def[MSG_SIZ];
    char *p;

    if (gameInfo.white != NULL && gameInfo.white[0] != '-') {
	p = def;
	CopyPlayerNameIntoFileName(&p, gameInfo.white);
	*p++ = '-';
	CopyPlayerNameIntoFileName(&p, gameInfo.black);
	*p++ = '.';
	strcpy(p, ext);
    } else {
	def[0] = NULLCHAR;
    }
    return def;
}

/* Save the current game to the given file */
int
SaveGameToFile(filename, append)
     char *filename;
     int append;
{
    FILE *f;
    char buf[MSG_SIZ];

    if (strcmp(filename, "-") == 0) {
	return SaveGame(stdout, 0, NULL);
    } else {
	f = fopen(filename, append ? "a" : "w");
	if (f == NULL) {
	    sprintf(buf, _("Can't open \"%s\""), filename);
	    DisplayError(buf, errno);
	    return FALSE;
	} else {
	    return SaveGame(f, 0, NULL);
	}
    }
}

char *
SavePart(str)
     char *str;
{
    static char buf[MSG_SIZ];
    char *p;

    p = strchr(str, ' ');
    if (p == NULL) return str;
    strncpy(buf, str, p - str);
    buf[p - str] = NULLCHAR;
    return buf;
}

#define PGN_MAX_LINE 75

/* Save game in PGN style and close the file */
int
SaveGamePGN(f)
     FILE *f;
{
    int i, offset, linelen, newblock;
    time_t tm;
    char *movetext;
    char numtext[32];
    int movelen, numlen, blank;

    tm = time((time_t *) NULL);

    PrintPGNTags(f, &gameInfo);

    if (backwardMostMove > 0 || startedFromSetupPosition) {
        char *fen = PositionToFEN(backwardMostMove);
        fprintf(f, "[FEN \"%s\"]\n[SetUp \"1\"]\n", fen);
	fprintf(f, "\n{--------------\n");
	PrintPosition(f, backwardMostMove);
	fprintf(f, "--------------}\n");
        free(fen);
    } else {
	fprintf(f, "\n");
    }

    i = backwardMostMove;
    offset = backwardMostMove & (~1L); /* output move numbers start at 1 */
    linelen = 0;
    newblock = TRUE;

    while (i < forwardMostMove) {
	/* Print comments preceding this move */
	if (commentList[i] != NULL) {
	    if (linelen > 0) fprintf(f, "\n");
	    fprintf(f, "{\n%s}\n", commentList[i]);
	    linelen = 0;
	    newblock = TRUE;
	}

	/* Format move number */
	if ((i % 2) == 0) {
	    sprintf(numtext, "%d.", (i - offset)/2 + 1);
	} else {
	    if (newblock) {
		sprintf(numtext, "%d...", (i - offset)/2 + 1);
	    } else {
		numtext[0] = NULLCHAR;
	    }
	}
	numlen = strlen(numtext);
	newblock = FALSE;

	/* Print move number */
	blank = linelen > 0 && numlen > 0;
	if (linelen + (blank ? 1 : 0) + numlen > PGN_MAX_LINE) {
	    fprintf(f, "\n");
	    linelen = 0;
	    blank = 0;
	}
	if (blank) {
	    fprintf(f, " ");
	    linelen++;
	}
	fprintf(f, numtext);
	linelen += numlen;

	/* Get move */
	movetext = SavePart(parseList[i]);
	movelen = strlen(movetext);

	/* Print move */
	blank = linelen > 0 && movelen > 0;
	if (linelen + (blank ? 1 : 0) + movelen > PGN_MAX_LINE) {
	    fprintf(f, "\n");
	    linelen = 0;
	    blank = 0;
	}
	if (blank) {
	    fprintf(f, " ");
	    linelen++;
	}
	fprintf(f, movetext);
	linelen += movelen;

	i++;
    }

    /* Start a new line */
    if (linelen > 0) fprintf(f, "\n");

    /* Print comments after last move */
    if (commentList[i] != NULL) {
	fprintf(f, "{\n%s}\n", commentList[i]);
    }

    /* Print result */
    if (gameInfo.resultDetails != NULL &&
	gameInfo.resultDetails[0] != NULLCHAR) {
	fprintf(f, "{%s} %s\n\n", gameInfo.resultDetails,
		PGNResult(gameInfo.result));
    } else {
	fprintf(f, "%s\n\n", PGNResult(gameInfo.result));
    }

    fclose(f);
    return TRUE;
}

/* Save game in old style and close the file */
int
SaveGameOldStyle(f)
     FILE *f;
{
    int i, offset;
    time_t tm;

    tm = time((time_t *) NULL);

    fprintf(f, "# %s game file -- %s", programName, ctime(&tm));
    PrintOpponents(f);

    if (backwardMostMove > 0 || startedFromSetupPosition) {
	fprintf(f, "\n[--------------\n");
	PrintPosition(f, backwardMostMove);
	fprintf(f, "--------------]\n");
    } else {
	fprintf(f, "\n");
    }

    i = backwardMostMove;
    offset = backwardMostMove & (~1L); /* output move numbers start at 1 */

    while (i < forwardMostMove) {
	if (commentList[i] != NULL) {
	    fprintf(f, "[%s]\n", commentList[i]);
	}

	if ((i % 2) == 1) {
	    fprintf(f, "%d. ...  %s\n", (i - offset)/2 + 1, parseList[i]);
	    i++;
	} else {
	    fprintf(f, "%d. %s  ", (i - offset)/2 + 1, parseList[i]);
	    i++;
	    if (commentList[i] != NULL) {
		fprintf(f, "\n");
		continue;
	    }
	    if (i >= forwardMostMove) {
		fprintf(f, "\n");
		break;
	    }
	    fprintf(f, "%s\n", parseList[i]);
	    i++;
	}
    }

    if (commentList[i] != NULL) {
	fprintf(f, "[%s]\n", commentList[i]);
    }

    /* This isn't really the old style, but it's close enough */
    if (gameInfo.resultDetails != NULL &&
	gameInfo.resultDetails[0] != NULLCHAR) {
	fprintf(f, "%s (%s)\n\n", PGNResult(gameInfo.result),
		gameInfo.resultDetails);
    } else {
	fprintf(f, "%s\n\n", PGNResult(gameInfo.result));
    }

    fclose(f);
    return TRUE;
}

/* Save the current game to open file f and close the file */
int
SaveGame(f, dummy, dummy2)
     FILE *f;
     int dummy;
     char *dummy2;
{
    if (gameMode == EditPosition) EditPositionDone();
    if (appData.oldSaveStyle)
      return SaveGameOldStyle(f);
    else
      return SaveGamePGN(f);
}

/* Save the current position to the given file */
int
SavePositionToFile(filename)
     char *filename;
{
    FILE *f;
    char buf[MSG_SIZ];

    if (strcmp(filename, "-") == 0) {
	return SavePosition(stdout, 0, NULL);
    } else {
	f = fopen(filename, "a");
	if (f == NULL) {
	    sprintf(buf, _("Can't open \"%s\""), filename);
	    DisplayError(buf, errno);
	    return FALSE;
	} else {
	    SavePosition(f, 0, NULL);
	    return TRUE;
	}
    }
}

/* Save the current position to the given open file and close the file */
int
SavePosition(f, dummy, dummy2)
     FILE *f;
     int dummy;
     char *dummy2;
{
    time_t tm;
    char *fen;

    if (appData.oldSaveStyle) {
	tm = time((time_t *) NULL);

	fprintf(f, "# %s position file -- %s", programName, ctime(&tm));
	PrintOpponents(f);
	fprintf(f, "[--------------\n");
	PrintPosition(f, currentMove);
	fprintf(f, "--------------]\n");
    } else {
	fen = PositionToFEN(currentMove);
	fprintf(f, "%s\n", fen);
	free(fen);
    }
    fclose(f);
    return TRUE;
}

void
ReloadCmailMsgEvent(unregister)
     int unregister;
{
#if !WIN32
    static char *inFilename = NULL;
    static char *outFilename;
    int i;
    struct stat inbuf, outbuf;
    int status;

    /* Any registered moves are unregistered if unregister is set, */
    /* i.e. invoked by the signal handler */
    if (unregister) {
	for (i = 0; i < CMAIL_MAX_GAMES; i ++) {
	    cmailMoveRegistered[i] = FALSE;
	    if (cmailCommentList[i] != NULL) {
		free(cmailCommentList[i]);
		cmailCommentList[i] = NULL;
	    }
	}
	nCmailMovesRegistered = 0;
    }

    for (i = 0; i < CMAIL_MAX_GAMES; i ++) {
	cmailResult[i] = CMAIL_NOT_RESULT;
    }
    nCmailResults = 0;

    if (inFilename == NULL) {
	/* Because the filenames are static they only get malloced once  */
	/* and they never get freed                                      */
	inFilename = (char *) malloc(strlen(appData.cmailGameName) + 9);
	sprintf(inFilename, "%s.game.in", appData.cmailGameName);

	outFilename = (char *) malloc(strlen(appData.cmailGameName) + 5);
	sprintf(outFilename, "%s.out", appData.cmailGameName);
    }

    status = stat(outFilename, &outbuf);
    if (status < 0) {
	cmailMailedMove = FALSE;
    } else {
	status = stat(inFilename, &inbuf);
	cmailMailedMove = (inbuf.st_mtime < outbuf.st_mtime);
    }

    /* LoadGameFromFile(CMAIL_MAX_GAMES) with cmailMsgLoaded == TRUE
       counts the games, notes how each one terminated, etc.

       It would be nice to remove this kludge and instead gather all
       the information while building the game list.  (And to keep it
       in the game list nodes instead of having a bunch of fixed-size
       parallel arrays.)  Note this will require getting each game's
       termination from the PGN tags, as the game list builder does
       not process the game moves.  --mann
       */
    cmailMsgLoaded = TRUE;
    LoadGameFromFile(inFilename, CMAIL_MAX_GAMES, "", FALSE);

    /* Load first game in the file or popup game menu */
    LoadGameFromFile(inFilename, 0, appData.cmailGameName, TRUE);

#endif /* !WIN32 */
    return;
}

int
RegisterMove()
{
    FILE *f;
    char string[MSG_SIZ];

    if (   cmailMailedMove
	|| (cmailResult[lastLoadGameNumber - 1] == CMAIL_OLD_RESULT)) {
	return TRUE;		/* Allow free viewing  */
    }

    /* Unregister move to ensure that we don't leave RegisterMove        */
    /* with the move registered when the conditions for registering no   */
    /* longer hold                                                       */
    if (cmailMoveRegistered[lastLoadGameNumber - 1]) {
	cmailMoveRegistered[lastLoadGameNumber - 1] = FALSE;
	nCmailMovesRegistered --;

	if (cmailCommentList[lastLoadGameNumber - 1] != NULL)
	  {
	      free(cmailCommentList[lastLoadGameNumber - 1]);
	      cmailCommentList[lastLoadGameNumber - 1] = NULL;
	  }
    }

    if (cmailOldMove == -1) {
	DisplayError(_("You have edited the game history.\nUse Reload Same Game and make your move again."), 0);
	return FALSE;
    }

    if (currentMove > cmailOldMove + 1) {
	DisplayError(_("You have entered too many moves.\nBack up to the correct position and try again."), 0);
	return FALSE;
    }

    if (currentMove < cmailOldMove) {
	DisplayError(_("Displayed position is not current.\nStep forward to the correct position and try again."), 0);
	return FALSE;
    }

    if (forwardMostMove > currentMove) {
	/* Silently truncate extra moves */
	TruncateGame();
    }

    if (   (currentMove == cmailOldMove + 1)
	|| (   (currentMove == cmailOldMove)
	    && (   (cmailMoveType[lastLoadGameNumber - 1] == CMAIL_ACCEPT)
		|| (cmailMoveType[lastLoadGameNumber - 1] == CMAIL_RESIGN)))) {
	if (gameInfo.result != GameUnfinished) {
	    cmailResult[lastLoadGameNumber - 1] = CMAIL_NEW_RESULT;
	}

	if (commentList[currentMove] != NULL) {
	    cmailCommentList[lastLoadGameNumber - 1]
	      = StrSave(commentList[currentMove]);
	}
	strcpy(cmailMove[lastLoadGameNumber - 1], moveList[currentMove - 1]);

	if (appData.debugMode)
	  fprintf(debugFP, "Saving %s for game %d\n",
		  cmailMove[lastLoadGameNumber - 1], lastLoadGameNumber);

	sprintf(string,
		"%s.game.out.%d", appData.cmailGameName, lastLoadGameNumber);

	f = fopen(string, "w");
	if (appData.oldSaveStyle) {
	    SaveGameOldStyle(f); /* also closes the file */

	    sprintf(string, "%s.pos.out", appData.cmailGameName);
	    f = fopen(string, "w");
	    SavePosition(f, 0, NULL); /* also closes the file */
	} else {
	    fprintf(f, "{--------------\n");
	    PrintPosition(f, currentMove);
	    fprintf(f, "--------------}\n\n");

	    SaveGame(f, 0, NULL); /* also closes the file*/
	}

	cmailMoveRegistered[lastLoadGameNumber - 1] = TRUE;
	nCmailMovesRegistered ++;
    } else if (nCmailGames == 1) {
	DisplayError(_("You have not made a move yet"), 0);
	return FALSE;
    }

    return TRUE;
}

void
MailMoveEvent()
{
#if !WIN32
    static char *partCommandString = "cmail -xv%s -remail -game %s 2>&1";
    FILE *commandOutput;
    char buffer[MSG_SIZ], msg[MSG_SIZ], string[MSG_SIZ];
    int nBytes = 0;		/*  Suppress warnings on uninitialized variables    */
    int nBuffers;
    int i;
    int archived;
    char *arcDir;

    if (! cmailMsgLoaded) {
	DisplayError(_("The cmail message is not loaded.\nUse Reload CMail Message and make your move again."), 0);
	return;
    }

    if (nCmailGames == nCmailResults) {
	DisplayError(_("No unfinished games"), 0);
	return;
    }

#if CMAIL_PROHIBIT_REMAIL
    if (cmailMailedMove) {
	sprintf(msg, _("You have already mailed a move.\nWait until a move arrives from your opponent.\nTo resend the same move, type\n\"cmail -remail -game %s\"\non the command line."), appData.cmailGameName);
	DisplayError(msg, 0);
	return;
    }
#endif

    if (! (cmailMailedMove || RegisterMove())) return;

    if (   cmailMailedMove
	|| (nCmailMovesRegistered + nCmailResults == nCmailGames)) {
	sprintf(string, partCommandString,
		appData.debugMode ? " -v" : "", appData.cmailGameName);
	commandOutput = popen(string, "r");

	if (commandOutput == NULL) {
	    DisplayError(_("Failed to invoke cmail"), 0);
	} else {
	    for (nBuffers = 0; (! feof(commandOutput)); nBuffers ++) {
		nBytes = fread(buffer, 1, MSG_SIZ - 1, commandOutput);
	    }
	    if (nBuffers > 1) {
		(void) memcpy(msg, buffer + nBytes, MSG_SIZ - nBytes - 1);
		(void) memcpy(msg + MSG_SIZ - nBytes - 1, buffer, nBytes);
		nBytes = MSG_SIZ - 1;
	    } else {
		(void) memcpy(msg, buffer, nBytes);
	    }
	    *(msg + nBytes) = '\0'; /* \0 for end-of-string*/

	    if(StrStr(msg, "Mailed cmail message to ") != NULL) {
		cmailMailedMove = TRUE; /* Prevent >1 moves    */

		archived = TRUE;
		for (i = 0; i < nCmailGames; i ++) {
		    if (cmailResult[i] == CMAIL_NOT_RESULT) {
			archived = FALSE;
		    }
		}
		if (   archived
		    && (   (arcDir = (char *) getenv("CMAIL_ARCDIR"))
			!= NULL)) {
		    sprintf(buffer, "%s/%s.%s.archive",
			    arcDir,
			    appData.cmailGameName,
			    gameInfo.date);
		    LoadGameFromFile(buffer, 1, buffer, FALSE);
		    cmailMsgLoaded = FALSE;
		}
	    }

	    DisplayInformation(msg);
	    pclose(commandOutput);
	}
    } else {
	if ((*cmailMsg) != '\0') {
	    DisplayInformation(cmailMsg);
	}
    }

    return;
#endif /* !WIN32 */
}

char *
CmailMsg()
{
#if WIN32
    return NULL;
#else
    int  prependComma = 0;
    char number[5];
    char string[MSG_SIZ];	/* Space for game-list */
    int  i;

    if (!cmailMsgLoaded) return "";

    if (cmailMailedMove) {
	sprintf(cmailMsg, _("Waiting for reply from opponent\n"));
    } else {
	/* Create a list of games left */
	sprintf(string, "[");
	for (i = 0; i < nCmailGames; i ++) {
	    if (! (   cmailMoveRegistered[i]
		   || (cmailResult[i] == CMAIL_OLD_RESULT))) {
		if (prependComma) {
		    sprintf(number, ",%d", i + 1);
		} else {
		    sprintf(number, "%d", i + 1);
		    prependComma = 1;
		}

		strcat(string, number);
	    }
	}
	strcat(string, "]");

	if (nCmailMovesRegistered + nCmailResults == 0) {
	    switch (nCmailGames) {
	      case 1:
		sprintf(cmailMsg,
			_("Still need to make move for game\n"));
		break;

	      case 2:
		sprintf(cmailMsg,
			_("Still need to make moves for both games\n"));
		break;

	      default:
		sprintf(cmailMsg,
			_("Still need to make moves for all %d games\n"),
			nCmailGames);
		break;
	    }
	} else {
	    switch (nCmailGames - nCmailMovesRegistered - nCmailResults) {
	      case 1:
		sprintf(cmailMsg,
			_("Still need to make a move for game %s\n"),
			string);
		break;

	      case 0:
		if (nCmailResults == nCmailGames) {
		    sprintf(cmailMsg, _("No unfinished games\n"));
		} else {
		    sprintf(cmailMsg, _("Ready to send mail\n"));
		}
		break;

	      default:
		sprintf(cmailMsg,
			_("Still need to make moves for games %s\n"),
			string);
	    }
	}
    }
    return cmailMsg;
#endif /* WIN32 */
}

void
ResetGameEvent()
{
    if (gameMode == Training)
      SetTrainingModeOff();

    Reset(TRUE, TRUE);
    cmailMsgLoaded = FALSE;
    if (appData.icsActive) {
      SendToICS(ics_prefix);
      SendToICS("refresh\n");
    }
}

static int exiting = 0;

void
ExitEvent(status)
     int status;
{
    exiting++;
    if (exiting > 2) {
      /* Give up on clean exit */
      exit(status);
    }
    if (exiting > 1) {
      /* Keep trying for clean exit */
      return;
    }

    if (appData.icsActive && appData.colorize) Colorize(ColorNone, FALSE);

    if (telnetISR != NULL) {
      RemoveInputSource(telnetISR);
    }
    if (icsPR != NoProc) {
      DestroyChildProcess(icsPR, TRUE);
    }
    /* Save game if resource set and not already saved by GameEnds() */
    if (gameInfo.resultDetails == NULL && forwardMostMove > 0) {
      if (*appData.saveGameFile != NULLCHAR) {
	SaveGameToFile(appData.saveGameFile, TRUE);
      } else if (appData.autoSaveGames) {
	AutoSaveGame();
      }
      if (*appData.savePositionFile != NULLCHAR) {
	SavePositionToFile(appData.savePositionFile);
      }
    }
    GameEnds((ChessMove) 0, NULL, GE_PLAYER);

    /* Kill off chess programs */
    if (first.pr != NoProc) {
	ExitAnalyzeMode();
	SendToProgram("quit\n", &first);
	DestroyChildProcess(first.pr, first.useSigterm);
    }
    if (second.pr != NoProc) {
	SendToProgram("quit\n", &second);
	DestroyChildProcess(second.pr, second.useSigterm);
    }
    if (first.isr != NULL) {
	RemoveInputSource(first.isr);
    }
    if (second.isr != NULL) {
	RemoveInputSource(second.isr);
    }

    ShutDownFrontEnd();
    exit(status);
}

void
PauseEvent()
{
    if (appData.debugMode)
	fprintf(debugFP, "PauseEvent(): pausing %d\n", pausing);
    if (pausing) {
	pausing = FALSE;
	ModeHighlight();
	if (gameMode == MachinePlaysWhite ||
	    gameMode == MachinePlaysBlack) {
	    StartClocks();
	} else {
	    DisplayBothClocks();
	}
	if (gameMode == PlayFromGameFile) {
	    if (appData.timeDelay >= 0)
		AutoPlayGameLoop();
	} else if (gameMode == IcsExamining && pauseExamInvalid) {
	    Reset(FALSE, TRUE);
	    SendToICS(ics_prefix);
	    SendToICS("refresh\n");
	} else if (currentMove < forwardMostMove) {
	    ForwardInner(forwardMostMove);
	}
	pauseExamInvalid = FALSE;
    } else {
	switch (gameMode) {
	  default:
	    return;
	  case IcsExamining:
	    pauseExamForwardMostMove = forwardMostMove;
	    pauseExamInvalid = FALSE;
	    /* fall through */
	  case IcsObserving:
	  case IcsPlayingWhite:
	  case IcsPlayingBlack:
	    pausing = TRUE;
	    ModeHighlight();
	    return;
	  case PlayFromGameFile:
	    (void) StopLoadGameTimer();
	    pausing = TRUE;
	    ModeHighlight();
	    break;
	  case BeginningOfGame:
	    if (appData.icsActive) return;
	    /* else fall through */
	  case MachinePlaysWhite:
	  case MachinePlaysBlack:
	  case TwoMachinesPlay:
	    if (forwardMostMove == 0)
	      return;		/* don't pause if no one has moved */
	    if ((gameMode == MachinePlaysWhite &&
		 !WhiteOnMove(forwardMostMove)) ||
		(gameMode == MachinePlaysBlack &&
		 WhiteOnMove(forwardMostMove))) {
		StopClocks();
	    }
	    pausing = TRUE;
	    ModeHighlight();
	    break;
	}
    }
}

void
EditCommentEvent()
{
    char title[MSG_SIZ];

    if (currentMove < 1 || parseList[currentMove - 1][0] == NULLCHAR) {
	strcpy(title, _("Edit comment"));
    } else {
	sprintf(title, _("Edit comment on %d.%s%s"), (currentMove - 1) / 2 + 1,
		WhiteOnMove(currentMove - 1) ? " " : ".. ",
		parseList[currentMove - 1]);
    }

    EditCommentPopUp(currentMove, title, commentList[currentMove]);
}


void
EditTagsEvent()
{
    char *tags = PGNTags(&gameInfo);
    EditTagsPopUp(tags);
    free(tags);
}

void
AnalyzeModeEvent()
{
    if (appData.noChessProgram || gameMode == AnalyzeMode)
      return;

    if (gameMode != AnalyzeFile) {
		if (!appData.icsEngineAnalyze) {
			EditGameEvent();
	        if (gameMode != EditGame) return;
		}
	ResurrectChessProgram();
	SendToProgram("analyze\n", &first);
	first.analyzing = TRUE;
	/*first.maybeThinking = TRUE;*/
	first.maybeThinking = FALSE; /* avoid killing GNU Chess */
	AnalysisPopUp(_("Analysis"),
		      _("Starting analysis mode...\nIf this message stays up, your chess program does not support analysis."));
    }
    if (!appData.icsEngineAnalyze) gameMode = AnalyzeMode;
    pausing = FALSE;
    ModeHighlight();
    SetGameInfo();

    StartAnalysisClock();
    GetTimeMark(&lastNodeCountTime);
    lastNodeCount = 0;
}

void
AnalyzeFileEvent()
{
    if (appData.noChessProgram || gameMode == AnalyzeFile)
      return;

    if (gameMode != AnalyzeMode) {
	EditGameEvent();
	if (gameMode != EditGame) return;
	ResurrectChessProgram();
	SendToProgram("analyze\n", &first);
	first.analyzing = TRUE;
	/*first.maybeThinking = TRUE;*/
	first.maybeThinking = FALSE; /* avoid killing GNU Chess */
	AnalysisPopUp(_("Analysis"),
		      _("Starting analysis mode...\nIf this message stays up, your chess program does not support analysis."));
    }
    gameMode = AnalyzeFile;
    pausing = FALSE;
    ModeHighlight();
    SetGameInfo();

    StartAnalysisClock();
    GetTimeMark(&lastNodeCountTime);
    lastNodeCount = 0;
}

void
MachineWhiteEvent()
{
    char buf[MSG_SIZ];

    if (appData.noChessProgram || (gameMode == MachinePlaysWhite))
      return;


    if (gameMode == PlayFromGameFile ||
	gameMode == TwoMachinesPlay  ||
	gameMode == Training         ||
	gameMode == AnalyzeMode      ||
	gameMode == EndOfGame)
	EditGameEvent();

    if (gameMode == EditPosition)
        EditPositionDone();

    if (!WhiteOnMove(currentMove)) {
	DisplayError(_("It is not White's turn"), 0);
	return;
    }

    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile)
      ExitAnalyzeMode();

    if (gameMode == EditGame || gameMode == AnalyzeMode ||
	gameMode == AnalyzeFile)
	TruncateGame();

    ResurrectChessProgram();	/* in case it isn't running */
    gameMode = MachinePlaysWhite;
    pausing = FALSE;
    ModeHighlight();
    SetGameInfo();
    sprintf(buf, "%s vs. %s", gameInfo.white, gameInfo.black);
    DisplayTitle(buf);
    if (first.sendName) {
      sprintf(buf, "name %s\n", gameInfo.black);
      SendToProgram(buf, &first);
    }
    if (first.sendTime) {
      if (first.useColors) {
	SendToProgram("black\n", &first); /*gnu kludge*/
      }
      SendTimeRemaining(&first, TRUE);
    }
    if (first.useColors) {
      SendToProgram("white\ngo\n", &first);
    } else {
      SendToProgram("go\n", &first);
    }
    SetMachineThinkingEnables();
    first.maybeThinking = TRUE;
    StartClocks();

    if (appData.autoFlipView && !flipView) {
      flipView = !flipView;
      DrawPosition(FALSE, NULL);
    }
}

void
MachineBlackEvent()
{
    char buf[MSG_SIZ];

    if (appData.noChessProgram || (gameMode == MachinePlaysBlack))
	return;


    if (gameMode == PlayFromGameFile ||
	gameMode == TwoMachinesPlay  ||
	gameMode == Training         ||
	gameMode == AnalyzeMode      ||
	gameMode == EndOfGame)
        EditGameEvent();

    if (gameMode == EditPosition)
        EditPositionDone();

    if (WhiteOnMove(currentMove)) {
	DisplayError(_("It is not Black's turn"), 0);
	return;
    }

    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile)
      ExitAnalyzeMode();

    if (gameMode == EditGame || gameMode == AnalyzeMode ||
	gameMode == AnalyzeFile)
	TruncateGame();

    ResurrectChessProgram();	/* in case it isn't running */
    gameMode = MachinePlaysBlack;
    pausing = FALSE;
    ModeHighlight();
    SetGameInfo();
    sprintf(buf, "%s vs. %s", gameInfo.white, gameInfo.black);
    DisplayTitle(buf);
    if (first.sendName) {
      sprintf(buf, "name %s\n", gameInfo.white);
      SendToProgram(buf, &first);
    }
    if (first.sendTime) {
      if (first.useColors) {
	SendToProgram("white\n", &first); /*gnu kludge*/
      }
      SendTimeRemaining(&first, FALSE);
    }
    if (first.useColors) {
      SendToProgram("black\ngo\n", &first);
    } else {
      SendToProgram("go\n", &first);
    }
    SetMachineThinkingEnables();
    first.maybeThinking = TRUE;
    StartClocks();

    if (appData.autoFlipView && flipView) {
      flipView = !flipView;
      DrawPosition(FALSE, NULL);
    }
}


void
DisplayTwoMachinesTitle()
{
    char buf[MSG_SIZ];
    if (appData.matchGames > 0) {
        if (first.twoMachinesColor[0] == 'w') {
	    sprintf(buf, "%s vs. %s (%d-%d-%d)",
		    gameInfo.white, gameInfo.black,
		    first.matchWins, second.matchWins,
		    matchGame - 1 - (first.matchWins + second.matchWins));
	} else {
	    sprintf(buf, "%s vs. %s (%d-%d-%d)",
		    gameInfo.white, gameInfo.black,
		    second.matchWins, first.matchWins,
		    matchGame - 1 - (first.matchWins + second.matchWins));
	}
    } else {
	sprintf(buf, "%s vs. %s", gameInfo.white, gameInfo.black);
    }
    DisplayTitle(buf);
}

void
TwoMachinesEvent P((void))
{
    int i;
    char buf[MSG_SIZ];
    ChessProgramState *onmove;

    if (appData.noChessProgram) return;

    switch (gameMode) {
      case TwoMachinesPlay:
	return;
      case MachinePlaysWhite:
      case MachinePlaysBlack:
	if (WhiteOnMove(forwardMostMove) == (gameMode == MachinePlaysWhite)) {
	    DisplayError(_("Wait until your turn,\nor select Move Now"), 0);
	    return;
	}
	/* fall through */
      case BeginningOfGame:
      case PlayFromGameFile:
      case EndOfGame:
	EditGameEvent();
	if (gameMode != EditGame) return;
	break;
      case EditPosition:
	EditPositionDone();
	break;
      case AnalyzeMode:
      case AnalyzeFile:
	ExitAnalyzeMode();
	break;
      case EditGame:
      default:
	break;
    }

    forwardMostMove = currentMove;
    ResurrectChessProgram();	/* in case first program isn't running */

    if (second.pr == NULL) {
	StartChessProgram(&second);
	if (second.protocolVersion == 1) {
	  TwoMachinesEventIfReady();
	} else {
	  /* kludge: allow timeout for initial "feature" command */
	  FreezeUI();
	  DisplayMessage("", _("Starting second chess program"));
	  ScheduleDelayedEvent(TwoMachinesEventIfReady, FEATURE_TIMEOUT);
	}
	return;
    }
    DisplayMessage("", "");
    InitChessProgram(&second);
    SendToProgram("force\n", &second);
    if (startedFromSetupPosition) {
	SendBoard(&second, backwardMostMove);
    }
    for (i = backwardMostMove; i < forwardMostMove; i++) {
	SendMoveToProgram(i, &second);
    }

    gameMode = TwoMachinesPlay;
    pausing = FALSE;
    ModeHighlight();
    SetGameInfo();
    DisplayTwoMachinesTitle();
    firstMove = TRUE;
    if ((first.twoMachinesColor[0] == 'w') == WhiteOnMove(forwardMostMove)) {
	onmove = &first;
    } else {
	onmove = &second;
    }

    SendToProgram(first.computerString, &first);
    if (first.sendName) {
      sprintf(buf, "name %s\n", second.tidy);
      SendToProgram(buf, &first);
    }
    SendToProgram(second.computerString, &second);
    if (second.sendName) {
      sprintf(buf, "name %s\n", first.tidy);
      SendToProgram(buf, &second);
    }

    if (!first.sendTime || !second.sendTime) {
	ResetClocks();
	timeRemaining[0][forwardMostMove] = whiteTimeRemaining;
	timeRemaining[1][forwardMostMove] = blackTimeRemaining;
    }
    if (onmove->sendTime) {
      if (onmove->useColors) {
	SendToProgram(onmove->other->twoMachinesColor, onmove); /*gnu kludge*/
      }
      SendTimeRemaining(onmove, WhiteOnMove(forwardMostMove));
    }
    if (onmove->useColors) {
      SendToProgram(onmove->twoMachinesColor, onmove);
    }
    SendToProgram("go\n", onmove);
    onmove->maybeThinking = TRUE;
    SetMachineThinkingEnables();

    StartClocks();
}

void
TrainingEvent()
{
    if (gameMode == Training) {
      SetTrainingModeOff();
      gameMode = PlayFromGameFile;
      DisplayMessage("", _("Training mode off"));
    } else {
      gameMode = Training;
      animateTraining = appData.animate;

      /* make sure we are not already at the end of the game */
      if (currentMove < forwardMostMove) {
	SetTrainingModeOn();
	DisplayMessage("", _("Training mode on"));
      } else {
	gameMode = PlayFromGameFile;
	DisplayError(_("Already at end of game"), 0);
      }
    }
    ModeHighlight();
}

void
IcsClientEvent()
{
    if (!appData.icsActive) return;
    switch (gameMode) {
      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case IcsObserving:
      case IcsIdle:
      case BeginningOfGame:
      case IcsExamining:
	return;

      case EditGame:
	break;

      case EditPosition:
	EditPositionDone();
	break;

      case AnalyzeMode:
      case AnalyzeFile:
	ExitAnalyzeMode();
	break;

      default:
	EditGameEvent();
	break;
    }

    gameMode = IcsIdle;
    ModeHighlight();
    return;
}


void
EditGameEvent()
{
    int i;

    switch (gameMode) {
      case Training:
	SetTrainingModeOff();
	break;
      case MachinePlaysWhite:
      case MachinePlaysBlack:
      case BeginningOfGame:
	SendToProgram("force\n", &first);
	SetUserThinkingEnables();
	break;
      case PlayFromGameFile:
	(void) StopLoadGameTimer();
	if (gameFileFP != NULL) {
	    gameFileFP = NULL;
	}
	break;
      case EditPosition:
	EditPositionDone();
	break;
      case AnalyzeMode:
      case AnalyzeFile:
	ExitAnalyzeMode();
	SendToProgram("force\n", &first);
	break;
      case TwoMachinesPlay:
	GameEnds((ChessMove) 0, NULL, GE_PLAYER);
	ResurrectChessProgram();
	SetUserThinkingEnables();
	break;
      case EndOfGame:
	ResurrectChessProgram();
	break;
      case IcsPlayingBlack:
      case IcsPlayingWhite:
	DisplayError(_("Warning: You are still playing a game"), 0);
	break;
      case IcsObserving:
	DisplayError(_("Warning: You are still observing a game"), 0);
	break;
      case IcsExamining:
	DisplayError(_("Warning: You are still examining a game"), 0);
	break;
      case IcsIdle:
	break;
      case EditGame:
      default:
	return;
    }

    pausing = FALSE;
    StopClocks();
    first.offeredDraw = second.offeredDraw = 0;

    if (gameMode == PlayFromGameFile) {
	whiteTimeRemaining = timeRemaining[0][currentMove];
	blackTimeRemaining = timeRemaining[1][currentMove];
	DisplayTitle("");
    }

    if (gameMode == MachinePlaysWhite ||
	gameMode == MachinePlaysBlack ||
	gameMode == TwoMachinesPlay ||
	gameMode == EndOfGame) {
	i = forwardMostMove;
	while (i > currentMove) {
	    SendToProgram("undo\n", &first);
	    i--;
	}
	whiteTimeRemaining = timeRemaining[0][currentMove];
	blackTimeRemaining = timeRemaining[1][currentMove];
	DisplayBothClocks();
	if (whiteFlag || blackFlag) {
	    whiteFlag = blackFlag = 0;
	}
	DisplayTitle("");
    }

    gameMode = EditGame;
    ModeHighlight();
    SetGameInfo();
}


void
EditPositionEvent()
{
    if (gameMode == EditPosition) {
	EditGameEvent();
	return;
    }

    EditGameEvent();
    if (gameMode != EditGame) return;

    gameMode = EditPosition;
    ModeHighlight();
    SetGameInfo();
    if (currentMove > 0)
      CopyBoard(boards[0], boards[currentMove]);

    blackPlaysFirst = !WhiteOnMove(currentMove);
    ResetClocks();
    currentMove = forwardMostMove = backwardMostMove = 0;
    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);
    DisplayMove(-1);
}

void
ExitAnalyzeMode()
{
	/* icsEngineAnalyze - possible call from other functions */
	if (appData.icsEngineAnalyze) {
	    appData.icsEngineAnalyze = FALSE;
		DisplayMessage("","Close ICS engine analyze...");
	}
    if (first.analysisSupport && first.analyzing) {
      SendToProgram("exit\n", &first);
      first.analyzing = FALSE;
    }
    AnalysisPopDown();
    thinkOutput[0] = NULLCHAR;
}

void
EditPositionDone()
{
    startedFromSetupPosition = TRUE;
    InitChessProgram(&first);
    SendToProgram("force\n", &first);
    if (blackPlaysFirst) {
	strcpy(moveList[0], "");
	strcpy(parseList[0], "");
	currentMove = forwardMostMove = backwardMostMove = 1;
	CopyBoard(boards[1], boards[0]);
    } else {
	currentMove = forwardMostMove = backwardMostMove = 0;
    }
    SendBoard(&first, forwardMostMove);
    DisplayTitle("");
    timeRemaining[0][forwardMostMove] = whiteTimeRemaining;
    timeRemaining[1][forwardMostMove] = blackTimeRemaining;
    gameMode = EditGame;
    ModeHighlight();
    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);
}

/* Pause for `ms' milliseconds */
/* !! Ugh, this is a kludge. Fix it sometime. --tpm */
void
TimeDelay(ms)
     long ms;
{
    TimeMark m1, m2;

    GetTimeMark(&m1);
    do {
	GetTimeMark(&m2);
    } while (SubtractTimeMarks(&m2, &m1) < ms);
}

/* !! Ugh, this is a kludge. Fix it sometime. --tpm */
void
SendMultiLineToICS(buf)
     char *buf;
{
    char temp[MSG_SIZ+1], *p;
    int len;

    len = strlen(buf);
    if (len > MSG_SIZ)
      len = MSG_SIZ;

    strncpy(temp, buf, len);
    temp[len] = 0;

    p = temp;
    while (*p) {
	if (*p == '\n' || *p == '\r')
	  *p = ' ';
	++p;
    }

    strcat(temp, "\n");
    SendToICS(temp);
    SendToPlayer(temp, strlen(temp));
}

void
SetWhiteToPlayEvent()
{
    if (gameMode == EditPosition) {
	blackPlaysFirst = FALSE;
	DisplayBothClocks();	/* works because currentMove is 0 */
    } else if (gameMode == IcsExamining) {
        SendToICS(ics_prefix);
	SendToICS("tomove white\n");
    }
}

void
SetBlackToPlayEvent()
{
    if (gameMode == EditPosition) {
	blackPlaysFirst = TRUE;
	currentMove = 1;	/* kludge */
	DisplayBothClocks();
	currentMove = 0;
    } else if (gameMode == IcsExamining) {
        SendToICS(ics_prefix);
	SendToICS("tomove black\n");
    }
}

void
EditPositionMenuEvent(selection, x, y)
     ChessSquare selection;
     int x, y;
{
    char buf[MSG_SIZ];

    if (gameMode != EditPosition && gameMode != IcsExamining) return;

    switch (selection) {
      case ClearBoard:
	if (gameMode == IcsExamining && ics_type == ICS_FICS) {
	    SendToICS(ics_prefix);
	    SendToICS("bsetup clear\n");
	} else if (gameMode == IcsExamining && ics_type == ICS_ICC) {
	    SendToICS(ics_prefix);
	    SendToICS("clearboard\n");
	} else {
	    for (x = 0; x < BOARD_SIZE; x++) {
		for (y = 0; y < BOARD_SIZE; y++) {
		    if (gameMode == IcsExamining) {
			if (boards[currentMove][y][x] != EmptySquare) {
			    sprintf(buf, "%sx@%c%c\n", ics_prefix,
				    'a' + x, '1' + y);
			    SendToICS(buf);
			}
		    } else {
			boards[0][y][x] = EmptySquare;
		    }
		}
	    }
	}
	if (gameMode == EditPosition) {
	    DrawPosition(FALSE, boards[0]);
	}
	break;

      case WhitePlay:
	SetWhiteToPlayEvent();
	break;

      case BlackPlay:
	SetBlackToPlayEvent();
	break;

      case EmptySquare:
	if (gameMode == IcsExamining) {
	    sprintf(buf, "%sx@%c%c\n", ics_prefix, 'a' + x, '1' + y);
	    SendToICS(buf);
	} else {
	    boards[0][y][x] = EmptySquare;
	    DrawPosition(FALSE, boards[0]);
	}
	break;

      default:
	if (gameMode == IcsExamining) {
  	    sprintf(buf, "%s%c@%c%c\n", ics_prefix,
		    PieceToChar(selection), 'a' + x, '1' + y);
	    SendToICS(buf);
	} else {
	    boards[0][y][x] = selection;
	    DrawPosition(FALSE, boards[0]);
	}
	break;
    }
}


void
DropMenuEvent(selection, x, y)
     ChessSquare selection;
     int x, y;
{
    ChessMove moveType;

    switch (gameMode) {
      case IcsPlayingWhite:
      case MachinePlaysBlack:
	if (!WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is Black's turn"));
	    return;
	}
	moveType = WhiteDrop;
	break;
      case IcsPlayingBlack:
      case MachinePlaysWhite:
	if (WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is White's turn"));
	    return;
	}
	moveType = BlackDrop;
	break;
      case EditGame:
	moveType = WhiteOnMove(currentMove) ? WhiteDrop : BlackDrop;
	break;
      default:
	return;
    }

    if (moveType == BlackDrop && selection < BlackPawn) {
      selection = (ChessSquare) ((int) selection
				 + (int) BlackPawn - (int) WhitePawn);
    }
    if (boards[currentMove][y][x] != EmptySquare) {
	DisplayMoveError(_("That square is occupied"));
	return;
    }

    FinishMove(moveType, (int) selection, DROP_RANK, x, y, NULLCHAR);
}

void
AcceptEvent()
{
    /* Accept a pending offer of any kind from opponent */

    if (appData.icsActive) {
        SendToICS(ics_prefix);
	SendToICS("accept\n");
    } else if (cmailMsgLoaded) {
	if (currentMove == cmailOldMove &&
	    commentList[cmailOldMove] != NULL &&
	    StrStr(commentList[cmailOldMove], WhiteOnMove(cmailOldMove) ?
		   "Black offers a draw" : "White offers a draw")) {
	    TruncateGame();
	    GameEnds(GameIsDrawn, "Draw agreed", GE_PLAYER);
	    cmailMoveType[lastLoadGameNumber - 1] = CMAIL_ACCEPT;
	} else {
	    DisplayError(_("There is no pending offer on this move"), 0);
	    cmailMoveType[lastLoadGameNumber - 1] = CMAIL_MOVE;
	}
    } else {
	/* Not used for offers from chess program */
    }
}

void
DeclineEvent()
{
    /* Decline a pending offer of any kind from opponent */

    if (appData.icsActive) {
        SendToICS(ics_prefix);
	SendToICS("decline\n");
    } else if (cmailMsgLoaded) {
	if (currentMove == cmailOldMove &&
	    commentList[cmailOldMove] != NULL &&
	    StrStr(commentList[cmailOldMove], WhiteOnMove(cmailOldMove) ?
		   "Black offers a draw" : "White offers a draw")) {
#ifdef NOTDEF
	    AppendComment(cmailOldMove, "Draw declined");
	    DisplayComment(cmailOldMove - 1, "Draw declined");
#endif /*NOTDEF*/
	} else {
	    DisplayError(_("There is no pending offer on this move"), 0);
	}
    } else {
	/* Not used for offers from chess program */
    }
}

void
RematchEvent()
{
    /* Issue ICS rematch command */
    if (appData.icsActive) {
        SendToICS(ics_prefix);
	SendToICS("rematch\n");
    }
}

void
CallFlagEvent()
{
    /* Call your opponent's flag (claim a win on time) */
    if (appData.icsActive) {
        SendToICS(ics_prefix);
	SendToICS("flag\n");
    } else {
	switch (gameMode) {
	  default:
	    return;
	  case MachinePlaysWhite:
	    if (whiteFlag) {
		if (blackFlag)
		  GameEnds(GameIsDrawn, "Both players ran out of time",
			   GE_PLAYER);
		else
		  GameEnds(BlackWins, "Black wins on time", GE_PLAYER);
	    } else {
		DisplayError(_("Your opponent is not out of time"), 0);
	    }
	    break;
	  case MachinePlaysBlack:
	    if (blackFlag) {
		if (whiteFlag)
		  GameEnds(GameIsDrawn, "Both players ran out of time",
			   GE_PLAYER);
		else
		  GameEnds(WhiteWins, "White wins on time", GE_PLAYER);
	    } else {
		DisplayError(_("Your opponent is not out of time"), 0);
	    }
	    break;
	}
    }
}

void
DrawEvent()
{
    /* Offer draw or accept pending draw offer from opponent */

    if (appData.icsActive) {
	/* Note: tournament rules require draw offers to be
	   made after you make your move but before you punch
	   your clock.  Currently ICS doesn't let you do that;
	   instead, you immediately punch your clock after making
	   a move, but you can offer a draw at any time. */

        SendToICS(ics_prefix);
	SendToICS("draw\n");
    } else if (cmailMsgLoaded) {
	if (currentMove == cmailOldMove &&
	    commentList[cmailOldMove] != NULL &&
	    StrStr(commentList[cmailOldMove], WhiteOnMove(cmailOldMove) ?
		   "Black offers a draw" : "White offers a draw")) {
	    GameEnds(GameIsDrawn, "Draw agreed", GE_PLAYER);
	    cmailMoveType[lastLoadGameNumber - 1] = CMAIL_ACCEPT;
	} else if (currentMove == cmailOldMove + 1) {
	    char *offer = WhiteOnMove(cmailOldMove) ?
	      "White offers a draw" : "Black offers a draw";
	    AppendComment(currentMove, offer);
	    DisplayComment(currentMove - 1, offer);
	    cmailMoveType[lastLoadGameNumber - 1] = CMAIL_DRAW;
	} else {
	    DisplayError(_("You must make your move before offering a draw"), 0);
	    cmailMoveType[lastLoadGameNumber - 1] = CMAIL_MOVE;
	}
    } else if (first.offeredDraw) {
	GameEnds(GameIsDrawn, "Draw agreed", GE_XBOARD);
    } else {
        if (first.sendDrawOffers) {
	    SendToProgram("draw\n", &first);
            userOfferedDraw = TRUE;
	}
    }
}

void
AdjournEvent()
{
    /* Offer Adjourn or accept pending Adjourn offer from opponent */

    if (appData.icsActive) {
        SendToICS(ics_prefix);
	SendToICS("adjourn\n");
    } else {
	/* Currently GNU Chess doesn't offer or accept Adjourns */
    }
}


void
AbortEvent()
{
    /* Offer Abort or accept pending Abort offer from opponent */

    if (appData.icsActive) {
        SendToICS(ics_prefix);
	SendToICS("abort\n");
    } else {
	GameEnds(GameUnfinished, "Game aborted", GE_PLAYER);
    }
}

void
ResignEvent()
{
    /* Resign.  You can do this even if it's not your turn. */

    if (appData.icsActive) {
        SendToICS(ics_prefix);
	SendToICS("resign\n");
    } else {
	switch (gameMode) {
	  case MachinePlaysWhite:
	    GameEnds(WhiteWins, "Black resigns", GE_PLAYER);
	    break;
	  case MachinePlaysBlack:
	    GameEnds(BlackWins, "White resigns", GE_PLAYER);
	    break;
	  case EditGame:
	    if (cmailMsgLoaded) {
		TruncateGame();
		if (WhiteOnMove(cmailOldMove)) {
		    GameEnds(BlackWins, "White resigns", GE_PLAYER);
		} else {
		    GameEnds(WhiteWins, "Black resigns", GE_PLAYER);
		}
		cmailMoveType[lastLoadGameNumber - 1] = CMAIL_RESIGN;
	    }
	    break;
	  default:
	    break;
	}
    }
}


void
StopObservingEvent()
{
    /* Stop observing current games */
    SendToICS(ics_prefix);
    SendToICS("unobserve\n");
}

void
StopExaminingEvent()
{
    /* Stop observing current game */
    SendToICS(ics_prefix);
    SendToICS("unexamine\n");
}

void
ForwardInner(target)
     int target;
{
    int limit;

    if (appData.debugMode)
	fprintf(debugFP, "ForwardInner(%d), current %d, forward %d\n",
		target, currentMove, forwardMostMove);

    if (gameMode == EditPosition)
      return;

    if (gameMode == PlayFromGameFile && !pausing)
      PauseEvent();

    if (gameMode == IcsExamining && pausing)
      limit = pauseExamForwardMostMove;
    else
      limit = forwardMostMove;

    if (target > limit) target = limit;

    if (target > 0 && moveList[target - 1][0]) {
	int fromX, fromY, toX, toY;
	toX = moveList[target - 1][2] - 'a';
	toY = moveList[target - 1][3] - '1';
	if (moveList[target - 1][1] == '@') {
	    if (appData.highlightLastMove) {
		SetHighlights(-1, -1, toX, toY);
	    }
	} else {
	    fromX = moveList[target - 1][0] - 'a';
	    fromY = moveList[target - 1][1] - '1';
	    if (target == currentMove + 1) {
		AnimateMove(boards[currentMove], fromX, fromY, toX, toY);
	    }
	    if (appData.highlightLastMove) {
		SetHighlights(fromX, fromY, toX, toY);
	    }
	}
    }
    if (gameMode == EditGame || gameMode == AnalyzeMode ||
	gameMode == Training || gameMode == PlayFromGameFile ||
	gameMode == AnalyzeFile) {
	while (currentMove < target) {
	    SendMoveToProgram(currentMove++, &first);
	}
    } else {
	currentMove = target;
    }

    if (gameMode == EditGame || gameMode == EndOfGame) {
	whiteTimeRemaining = timeRemaining[0][currentMove];
	blackTimeRemaining = timeRemaining[1][currentMove];
    }
    DisplayBothClocks();
    DisplayMove(currentMove - 1);
    DrawPosition(FALSE, boards[currentMove]);
    HistorySet(parseList,backwardMostMove,forwardMostMove,currentMove-1);
    if (commentList[currentMove] && !matchMode && gameMode != Training) {
	DisplayComment(currentMove - 1, commentList[currentMove]);
    }
}


void
ForwardEvent()
{
    if (gameMode == IcsExamining && !pausing) {
        SendToICS(ics_prefix);
	SendToICS("forward\n");
    } else {
	ForwardInner(currentMove + 1);
    }
}

void
ToEndEvent()
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	/* to optimze, we temporarily turn off analysis mode while we feed
	 * the remaining moves to the engine. Otherwise we get analysis output
	 * after each move.
	 */
        if (first.analysisSupport) {
	  SendToProgram("exit\nforce\n", &first);
	  first.analyzing = FALSE;
	}
    }

    if (gameMode == IcsExamining && !pausing) {
        SendToICS(ics_prefix);
	SendToICS("forward 999999\n");
    } else {
	ForwardInner(forwardMostMove);
    }

    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	/* we have fed all the moves, so reactivate analysis mode */
	SendToProgram("analyze\n", &first);
	first.analyzing = TRUE;
	/*first.maybeThinking = TRUE;*/
	first.maybeThinking = FALSE; /* avoid killing GNU Chess */
    }
}

void
BackwardInner(target)
     int target;
{
    if (appData.debugMode)
	fprintf(debugFP, "BackwardInner(%d), current %d, forward %d\n",
		target, currentMove, forwardMostMove);

    if (gameMode == EditPosition) return;
    if (currentMove <= backwardMostMove) {
	ClearHighlights();
	DrawPosition(FALSE, boards[currentMove]);
	return;
    }
    if (gameMode == PlayFromGameFile && !pausing)
      PauseEvent();

    if (moveList[target][0]) {
	int fromX, fromY, toX, toY;
	toX = moveList[target][2] - 'a';
	toY = moveList[target][3] - '1';
	if (moveList[target][1] == '@') {
	    if (appData.highlightLastMove) {
		SetHighlights(-1, -1, toX, toY);
	    }
	} else {
	    fromX = moveList[target][0] - 'a';
	    fromY = moveList[target][1] - '1';
	    if (target == currentMove - 1) {
		AnimateMove(boards[currentMove], toX, toY, fromX, fromY);
	    }
	    if (appData.highlightLastMove) {
		SetHighlights(fromX, fromY, toX, toY);
	    }
	}
    }
    if (gameMode == EditGame || gameMode==AnalyzeMode ||
	gameMode == PlayFromGameFile || gameMode == AnalyzeFile) {
	while (currentMove > target) {
	    SendToProgram("undo\n", &first);
	    currentMove--;
	}
    } else {
	currentMove = target;
    }

    if (gameMode == EditGame || gameMode == EndOfGame) {
	whiteTimeRemaining = timeRemaining[0][currentMove];
	blackTimeRemaining = timeRemaining[1][currentMove];
    }
    DisplayBothClocks();
    DisplayMove(currentMove - 1);
    DrawPosition(FALSE, boards[currentMove]);
    HistorySet(parseList,backwardMostMove,forwardMostMove,currentMove-1);
    if (commentList[currentMove] != NULL) {
	DisplayComment(currentMove - 1, commentList[currentMove]);
    }
}

void
BackwardEvent()
{
    if (gameMode == IcsExamining && !pausing) {
        SendToICS(ics_prefix);
	SendToICS("backward\n");
    } else {
	BackwardInner(currentMove - 1);
    }
}

void
ToStartEvent()
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	/* to optimze, we temporarily turn off analysis mode while we undo
	 * all the moves. Otherwise we get analysis output after each undo.
	 */
        if (first.analysisSupport) {
	  SendToProgram("exit\nforce\n", &first);
	  first.analyzing = FALSE;
	}
    }

    if (gameMode == IcsExamining && !pausing) {
        SendToICS(ics_prefix);
	SendToICS("backward 999999\n");
    } else {
	BackwardInner(backwardMostMove);
    }

    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	/* we have fed all the moves, so reactivate analysis mode */
	SendToProgram("analyze\n", &first);
	first.analyzing = TRUE;
	/*first.maybeThinking = TRUE;*/
	first.maybeThinking = FALSE; /* avoid killing GNU Chess */
    }
}

void
ToNrEvent(int to)
{
  if (gameMode == PlayFromGameFile && !pausing) PauseEvent();
  if (to >= forwardMostMove) to = forwardMostMove;
  if (to <= backwardMostMove) to = backwardMostMove;
  if (to < currentMove) {
    BackwardInner(to);
  } else {
    ForwardInner(to);
  }
}

void
RevertEvent()
{
    if (gameMode != IcsExamining) {
	DisplayError(_("You are not examining a game"), 0);
	return;
    }
    if (pausing) {
	DisplayError(_("You can't revert while pausing"), 0);
	return;
    }
    SendToICS(ics_prefix);
    SendToICS("revert\n");
}

void
RetractMoveEvent()
{
    switch (gameMode) {
      case MachinePlaysWhite:
      case MachinePlaysBlack:
	if (WhiteOnMove(forwardMostMove) == (gameMode == MachinePlaysWhite)) {
	    DisplayError(_("Wait until your turn,\nor select Move Now"), 0);
	    return;
	}
	if (forwardMostMove < 2) return;
	currentMove = forwardMostMove = forwardMostMove - 2;
	whiteTimeRemaining = timeRemaining[0][currentMove];
	blackTimeRemaining = timeRemaining[1][currentMove];
	DisplayBothClocks();
	DisplayMove(currentMove - 1);
	ClearHighlights();/*!! could figure this out*/
	DrawPosition(FALSE, boards[currentMove]);
	SendToProgram("remove\n", &first);
	/*first.maybeThinking = TRUE;*/ /* GNU Chess does not ponder here */
	break;

      case BeginningOfGame:
      default:
	break;

      case IcsPlayingWhite:
      case IcsPlayingBlack:
	if (WhiteOnMove(forwardMostMove) == (gameMode == IcsPlayingWhite)) {
	    SendToICS(ics_prefix);
	    SendToICS("takeback 2\n");
	} else {
	    SendToICS(ics_prefix);
	    SendToICS("takeback 1\n");
	}
	break;
    }
}

void
MoveNowEvent()
{
    ChessProgramState *cps;

    switch (gameMode) {
      case MachinePlaysWhite:
	if (!WhiteOnMove(forwardMostMove)) {
	    DisplayError(_("It is your turn"), 0);
	    return;
	}
	cps = &first;
	break;
      case MachinePlaysBlack:
	if (WhiteOnMove(forwardMostMove)) {
	    DisplayError(_("It is your turn"), 0);
	    return;
	}
	cps = &first;
	break;
      case TwoMachinesPlay:
	if (WhiteOnMove(forwardMostMove) ==
	    (first.twoMachinesColor[0] == 'w')) {
	    cps = &first;
	} else {
	    cps = &second;
	}
	break;
      case BeginningOfGame:
      default:
	return;
    }
    SendToProgram("?\n", cps);
}

void
TruncateGameEvent()
{
    EditGameEvent();
    if (gameMode != EditGame) return;
    TruncateGame();
}

void
TruncateGame()
{
    if (forwardMostMove > currentMove) {
	if (gameInfo.resultDetails != NULL) {
	    free(gameInfo.resultDetails);
	    gameInfo.resultDetails = NULL;
	    gameInfo.result = GameUnfinished;
	}
	forwardMostMove = currentMove;
	HistorySet(parseList, backwardMostMove, forwardMostMove,
		   currentMove-1);
    }
}

void
HintEvent()
{
    if (appData.noChessProgram) return;
    switch (gameMode) {
      case MachinePlaysWhite:
	if (WhiteOnMove(forwardMostMove)) {
	    DisplayError(_("Wait until your turn"), 0);
	    return;
	}
	break;
      case BeginningOfGame:
      case MachinePlaysBlack:
	if (!WhiteOnMove(forwardMostMove)) {
	    DisplayError(_("Wait until your turn"), 0);
	    return;
	}
	break;
      default:
	DisplayError(_("No hint available"), 0);
	return;
    }
    SendToProgram("hint\n", &first);
    hintRequested = TRUE;
}

void
BookEvent()
{
    if (appData.noChessProgram) return;
    switch (gameMode) {
      case MachinePlaysWhite:
	if (WhiteOnMove(forwardMostMove)) {
	    DisplayError(_("Wait until your turn"), 0);
	    return;
	}
	break;
      case BeginningOfGame:
      case MachinePlaysBlack:
	if (!WhiteOnMove(forwardMostMove)) {
	    DisplayError(_("Wait until your turn"), 0);
	    return;
	}
	break;
      case EditPosition:
	EditPositionDone();
	break;
      case TwoMachinesPlay:
	return;
      default:
	break;
    }
    SendToProgram("bk\n", &first);
    bookOutput[0] = NULLCHAR;
    bookRequested = TRUE;
}

void
AboutGameEvent()
{
    char *tags = PGNTags(&gameInfo);
    TagsPopUp(tags, CmailMsg());
    free(tags);
}

/* end button procedures */

void
PrintPosition(fp, move)
     FILE *fp;
     int move;
{
    int i, j;

    for (i = BOARD_SIZE - 1; i >= 0; i--) {
	for (j = 0; j < BOARD_SIZE; j++) {
	    char c = PieceToChar(boards[move][i][j]);
	    fputc(c == 'x' ? '.' : c, fp);
	    fputc(j == BOARD_SIZE - 1 ? '\n' : ' ', fp);
	}
    }
    if ((gameMode == EditPosition) ? !blackPlaysFirst : (move % 2 == 0))
      fprintf(fp, "white to play\n");
    else
      fprintf(fp, "black to play\n");
}

void
PrintOpponents(fp)
     FILE *fp;
{
    if (gameInfo.white != NULL) {
	fprintf(fp, "\t%s vs. %s\n", gameInfo.white, gameInfo.black);
    } else {
	fprintf(fp, "\n");
    }
}

/* Find last component of program's own name, using some heuristics */
void
TidyProgramName(prog, host, buf)
     char *prog, *host, buf[MSG_SIZ];
{
    char *p, *q;
    int local = (strcmp(host, "localhost") == 0);
    while (!local && (p = strchr(prog, ';')) != NULL) {
	p++;
	while (*p == ' ') p++;
	prog = p;
    }
    if (*prog == '"' || *prog == '\'') {
	q = strchr(prog + 1, *prog);
    } else {
	q = strchr(prog, ' ');
    }
    if (q == NULL) q = prog + strlen(prog);
    p = q;
    while (p >= prog && *p != '/' && *p != '\\') p--;
    p++;
    if (q - p >= 4 && StrCaseCmp(q - 4, ".exe") == 0) q -= 4;
    memcpy(buf, p, q - p);
    buf[q - p] = NULLCHAR;
    if (!local) {
	strcat(buf, "@");
	strcat(buf, host);
    }
}

char *
TimeControlTagValue()
{
    char buf[MSG_SIZ];
    if (!appData.clockMode) {
	strcpy(buf, "-");
    } else if (movesPerSession > 0) {
	sprintf(buf, "%d/%ld", movesPerSession, timeControl/1000);
    } else if (timeIncrement == 0) {
	sprintf(buf, "%ld", timeControl/1000);
    } else {
	sprintf(buf, "%ld+%ld", timeControl/1000, timeIncrement/1000);
    }
    return StrSave(buf);
}

void
SetGameInfo()
{
    /* This routine is used only for certain modes */
    VariantClass v = gameInfo.variant;
    ClearGameInfo(&gameInfo);
    gameInfo.variant = v;

    switch (gameMode) {
      case MachinePlaysWhite:
	gameInfo.event = StrSave("Computer chess game");
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave(first.tidy);
	gameInfo.black = StrSave(UserName());
	gameInfo.timeControl = TimeControlTagValue();
	break;

      case MachinePlaysBlack:
	gameInfo.event = StrSave("Computer chess game");
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave(UserName());
	gameInfo.black = StrSave(first.tidy);
	gameInfo.timeControl = TimeControlTagValue();
	break;

      case TwoMachinesPlay:
	gameInfo.event = StrSave("Computer chess game");
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	if (matchGame > 0) {
	    char buf[MSG_SIZ];
	    sprintf(buf, "%d", matchGame);
	    gameInfo.round = StrSave(buf);
	} else {
	    gameInfo.round = StrSave("-");
	}
	if (first.twoMachinesColor[0] == 'w') {
	    gameInfo.white = StrSave(first.tidy);
	    gameInfo.black = StrSave(second.tidy);
	} else {
	    gameInfo.white = StrSave(second.tidy);
	    gameInfo.black = StrSave(first.tidy);
	}
	gameInfo.timeControl = TimeControlTagValue();
	break;

      case EditGame:
	gameInfo.event = StrSave("Edited game");
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave("-");
	gameInfo.black = StrSave("-");
	break;

      case EditPosition:
	gameInfo.event = StrSave("Edited position");
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave("-");
	gameInfo.black = StrSave("-");
	break;

      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case IcsObserving:
      case IcsExamining:
	break;

      case PlayFromGameFile:
	gameInfo.event = StrSave("Game from non-PGN file");
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave("?");
	gameInfo.black = StrSave("?");
	break;

      default:
	break;
    }
}

void
ReplaceComment(index, text)
     int index;
     char *text;
{
    int len;

    while (*text == '\n') text++;
    len = strlen(text);
    while (len > 0 && text[len - 1] == '\n') len--;

    if (commentList[index] != NULL)
      free(commentList[index]);

    if (len == 0) {
	commentList[index] = NULL;
	return;
    }
    commentList[index] = (char *) malloc(len + 2);
    strncpy(commentList[index], text, len);
    commentList[index][len] = '\n';
    commentList[index][len + 1] = NULLCHAR;
}

void
CrushCRs(text)
     char *text;
{
  char *p = text;
  char *q = text;
  char ch;

  do {
    ch = *p++;
    if (ch == '\r') continue;
    *q++ = ch;
  } while (ch != '\0');
}

void
AppendComment(index, text)
     int index;
     char *text;
{
    int oldlen, len;
    char *old;

    CrushCRs(text);
    while (*text == '\n') text++;
    len = strlen(text);
    while (len > 0 && text[len - 1] == '\n') len--;

    if (len == 0) return;

    if (commentList[index] != NULL) {
	old = commentList[index];
	oldlen = strlen(old);
	commentList[index] = (char *) malloc(oldlen + len + 2);
	strcpy(commentList[index], old);
	free(old);
	strncpy(&commentList[index][oldlen], text, len);
	commentList[index][oldlen + len] = '\n';
	commentList[index][oldlen + len + 1] = NULLCHAR;
    } else {
	commentList[index] = (char *) malloc(len + 2);
	strncpy(commentList[index], text, len);
	commentList[index][len] = '\n';
	commentList[index][len + 1] = NULLCHAR;
    }
}

void
SendToProgram(message, cps)
     char *message;
     ChessProgramState *cps;
{
    int count, outCount, error;
    char buf[MSG_SIZ];

    if (cps->pr == NULL) return;
    Attention(cps);

    if (appData.debugMode) {
	TimeMark now;
	GetTimeMark(&now);
	fprintf(debugFP, "%ld >%-6s: %s",
		SubtractTimeMarks(&now, &programStartTime),
		cps->which, message);
    }

    count = strlen(message);
    outCount = OutputToProcess(cps->pr, message, count, &error);
    if (outCount < count && !exiting) {
	sprintf(buf, _("Error writing to %s chess program"), cps->which);
	DisplayFatalError(buf, error, 1);
    }
}

void
ReceiveFromProgram(isr, closure, message, count, error)
     InputSourceRef isr;
     VOIDSTAR closure;
     char *message;
     int count;
     int error;
{
    char *end_str;
    char buf[MSG_SIZ];
    ChessProgramState *cps = (ChessProgramState *)closure;

    if (isr != cps->isr) return; /* Killed intentionally */
    if (count <= 0) {
	if (count == 0) {
	    sprintf(buf,
		    _("Error: %s chess program (%s) exited unexpectedly"),
		    cps->which, cps->program);
	    RemoveInputSource(cps->isr);
	    DisplayFatalError(buf, 0, 1);
	} else {
	    sprintf(buf,
		    _("Error reading from %s chess program (%s)"),
		    cps->which, cps->program);
	    RemoveInputSource(cps->isr);
	    DisplayFatalError(buf, error, 1);
	}
	GameEnds((ChessMove) 0, NULL, GE_PLAYER);
	return;
    }

    if ((end_str = strchr(message, '\r')) != NULL)
      *end_str = NULLCHAR;
    if ((end_str = strchr(message, '\n')) != NULL)
      *end_str = NULLCHAR;

    if (appData.debugMode) {
	TimeMark now;
	GetTimeMark(&now);
	fprintf(debugFP, "%ld <%-6s: %s\n",
		SubtractTimeMarks(&now, &programStartTime),
		cps->which, message);
    }
	/* if icsEngineAnalyze is active we block all
	   whisper and kibitz output, because nobody want 
	   see this 
	 */
	if (appData.icsEngineAnalyze) {
		if (strstr(message, "whisper") != NULL ||
		    strstr(message, "kibitz") != NULL || 
			strstr(message, "tellics") != NULL) return;
		HandleMachineMove(message, cps);
	} else {
		HandleMachineMove(message, cps);
	}
}


void
SendTimeControl(cps, mps, tc, inc, sd, st)
     ChessProgramState *cps;
     int mps, inc, sd, st;
     long tc;
{
    char buf[MSG_SIZ];
    int seconds = (tc / 1000) % 60;

    if (st > 0) {
      /* Set exact time per move, normally using st command */
      if (cps->stKludge) {
	/* GNU Chess 4 has no st command; uses level in a nonstandard way */
	seconds = st % 60;
	if (seconds == 0) {
	  sprintf(buf, "level 1 %d\n", st/60);
	} else {
	  sprintf(buf, "level 1 %d:%02d\n", st/60, seconds);
	}
      } else {
	sprintf(buf, "st %d\n", st);
      }
    } else {
      /* Set conventional or incremental time control, using level command */
      if (seconds == 0) {
	/* Note old gnuchess bug -- minutes:seconds used to not work.
	   Fixed in later versions, but still avoid :seconds
	   when seconds is 0. */
	sprintf(buf, "level %d %ld %d\n", mps, tc/60000, inc/1000);
      } else {
	sprintf(buf, "level %d %ld:%02d %d\n", mps, tc/60000,
		seconds, inc/1000);
      }
    }
    SendToProgram(buf, cps);

    /* Orthoganally (except for GNU Chess 4), limit time to st seconds */
    /* Orthogonally, limit search to given depth */
    if (sd > 0) {
      if (cps->sdKludge) {
	sprintf(buf, "depth\n%d\n", sd);
      } else {
	sprintf(buf, "sd %d\n", sd);
      }
      SendToProgram(buf, cps);
    }
}

void
SendTimeRemaining(cps, machineWhite)
     ChessProgramState *cps;
     int /*boolean*/ machineWhite;
{
    char message[MSG_SIZ];
    long time, otime;

    /* Note: this routine must be called when the clocks are stopped
       or when they have *just* been set or switched; otherwise
       it will be off by the time since the current tick started.
    */
    if (machineWhite) {
	time = whiteTimeRemaining / 10;
	otime = blackTimeRemaining / 10;
    } else {
	time = blackTimeRemaining / 10;
	otime = whiteTimeRemaining / 10;
    }
    if (time <= 0) time = 1;
    if (otime <= 0) otime = 1;

    sprintf(message, "time %ld\notim %ld\n", time, otime);
    SendToProgram(message, cps);
}

int
BoolFeature(p, name, loc, cps)
     char **p;
     char *name;
     int *loc;
     ChessProgramState *cps;
{
  char buf[MSG_SIZ];
  int len = strlen(name);
  int val;
  if (strncmp((*p), name, len) == 0 && (*p)[len] == '=') {
    (*p) += len + 1;
    sscanf(*p, "%d", &val);
    *loc = (val != 0);
    while (**p && **p != ' ') (*p)++;
    sprintf(buf, "accepted %s\n", name);
    SendToProgram(buf, cps);
    return TRUE;
  }
  return FALSE;
}

int
IntFeature(p, name, loc, cps)
     char **p;
     char *name;
     int *loc;
     ChessProgramState *cps;
{
  char buf[MSG_SIZ];
  int len = strlen(name);
  if (strncmp((*p), name, len) == 0 && (*p)[len] == '=') {
    (*p) += len + 1;
    sscanf(*p, "%d", loc);
    while (**p && **p != ' ') (*p)++;
    sprintf(buf, "accepted %s\n", name);
    SendToProgram(buf, cps);
    return TRUE;
  }
  return FALSE;
}

int
StringFeature(p, name, loc, cps)
     char **p;
     char *name;
     char loc[];
     ChessProgramState *cps;
{
  char buf[MSG_SIZ];
  int len = strlen(name);
  if (strncmp((*p), name, len) == 0
      && (*p)[len] == '=' && (*p)[len+1] == '\"') {
    (*p) += len + 2;
    sscanf(*p, "%[^\"]", loc);
    while (**p && **p != '\"') (*p)++;
    if (**p == '\"') (*p)++;
    sprintf(buf, "accepted %s\n", name);
    SendToProgram(buf, cps);
    return TRUE;
  }
  return FALSE;
}

void
FeatureDone(cps, val)
     ChessProgramState* cps;
     int val;
{
  DelayedEventCallback cb = GetDelayedEvent();
  if ((cb == InitBackEnd3 && cps == &first) ||
      (cb == TwoMachinesEventIfReady && cps == &second)) {
    CancelDelayedEvent();
    ScheduleDelayedEvent(cb, val ? 1 : 3600000);
  }
  cps->initDone = val;
}

/* Parse feature command from engine */
void
ParseFeatures(args, cps)
     char* args;
     ChessProgramState *cps;
{
  char *p = args;
  char *q;
  int val;
  char buf[MSG_SIZ];

  for (;;) {
    while (*p == ' ') p++;
    if (*p == NULLCHAR) return;

    if (BoolFeature(&p, "setboard", &cps->useSetboard, cps)) continue;
    if (BoolFeature(&p, "time", &cps->sendTime, cps)) continue;
    if (BoolFeature(&p, "draw", &cps->sendDrawOffers, cps)) continue;
    if (BoolFeature(&p, "sigint", &cps->useSigint, cps)) continue;
    if (BoolFeature(&p, "sigterm", &cps->useSigterm, cps)) continue;
    if (BoolFeature(&p, "reuse", &val, cps)) {
      /* Engine can disable reuse, but can't enable it if user said no */
      if (!val) cps->reuse = FALSE;
      continue;
    }
    if (BoolFeature(&p, "analyze", &cps->analysisSupport, cps)) continue;
    if (StringFeature(&p, "myname", &cps->tidy, cps)) {
      if (gameMode == TwoMachinesPlay) {
	DisplayTwoMachinesTitle();
      } else {
	DisplayTitle("");
      }
      continue;
    }
    if (StringFeature(&p, "variants", &cps->variants, cps)) continue;
    if (BoolFeature(&p, "san", &cps->useSAN, cps)) continue;
    if (BoolFeature(&p, "ping", &cps->usePing, cps)) continue;
    if (BoolFeature(&p, "playother", &cps->usePlayother, cps)) continue;
    if (BoolFeature(&p, "colors", &cps->useColors, cps)) continue;
    if (BoolFeature(&p, "usermove", &cps->useUsermove, cps)) continue;
    if (BoolFeature(&p, "ics", &cps->sendICS, cps)) continue;
    if (BoolFeature(&p, "name", &cps->sendName, cps)) continue;
    if (BoolFeature(&p, "pause", &val, cps)) continue; /* unused at present */
    if (IntFeature(&p, "done", &val, cps)) {
      FeatureDone(cps, val);
      continue;
    }

    /* unknown feature: complain and skip */
    q = p;
    while (*q && *q != '=') q++;
    sprintf(buf, "rejected %.*s\n", q-p, p);
    SendToProgram(buf, cps);
    p = q;
    if (*p == '=') {
      p++;
      if (*p == '\"') {
	p++;
	while (*p && *p != '\"') p++;
	if (*p == '\"') p++;
      } else {
	while (*p && *p != ' ') p++;
      }
    }
  }

}

void
PeriodicUpdatesEvent(newState)
     int newState;
{
    if (newState == appData.periodicUpdates)
      return;

    appData.periodicUpdates=newState;

    /* Display type changes, so update it now */
    DisplayAnalysis();

    /* Get the ball rolling again... */
    if (newState) {
	AnalysisPeriodicEvent(1);
	StartAnalysisClock();
    }
}

void
PonderNextMoveEvent(newState)
     int newState;
{
    if (newState == appData.ponderNextMove) return;
    if (gameMode == EditPosition) EditPositionDone();
    if (newState) {
	SendToProgram("hard\n", &first);
	if (gameMode == TwoMachinesPlay) {
	    SendToProgram("hard\n", &second);
	}
    } else {
	SendToProgram("easy\n", &first);
	thinkOutput[0] = NULLCHAR;
	if (gameMode == TwoMachinesPlay) {
	    SendToProgram("easy\n", &second);
	}
    }
    appData.ponderNextMove = newState;
}

void
ShowThinkingEvent(newState)
     int newState;
{
    if (newState == appData.showThinking) return;
    if (gameMode == EditPosition) EditPositionDone();
    if (newState) {
	SendToProgram("post\n", &first);
	if (gameMode == TwoMachinesPlay) {
	    SendToProgram("post\n", &second);
	}
    } else {
	SendToProgram("nopost\n", &first);
	thinkOutput[0] = NULLCHAR;
	if (gameMode == TwoMachinesPlay) {
	    SendToProgram("nopost\n", &second);
	}
    }
    appData.showThinking = newState;
}

void
AskQuestionEvent(title, question, replyPrefix, which)
     char *title; char *question; char *replyPrefix; char *which;
{
  ProcRef pr = (which[0] == '1') ? first.pr : second.pr;
  if (pr == NoProc) return;
  AskQuestion(title, question, replyPrefix, pr);
}

void
DisplayMove(moveNumber)
     int moveNumber;
{
    char message[MSG_SIZ];
    char res[MSG_SIZ];
    char cpThinkOutput[MSG_SIZ];

    if (moveNumber == forwardMostMove - 1 ||
	gameMode == AnalyzeMode || gameMode == AnalyzeFile) {

	strcpy(cpThinkOutput, thinkOutput);
	if (strchr(cpThinkOutput, '\n'))
	  *strchr(cpThinkOutput, '\n') = NULLCHAR;
    } else {
	*cpThinkOutput = NULLCHAR;
    }

    if (moveNumber == forwardMostMove - 1 &&
	gameInfo.resultDetails != NULL) {
	if (gameInfo.resultDetails[0] == NULLCHAR) {
	    sprintf(res, " %s", PGNResult(gameInfo.result));
	} else {
	    sprintf(res, " {%s} %s",
		    gameInfo.resultDetails, PGNResult(gameInfo.result));
	}
    } else {
	res[0] = NULLCHAR;
    }

    if (moveNumber < 0 || parseList[moveNumber][0] == NULLCHAR) {
	DisplayMessage(res, cpThinkOutput);
    } else {
	sprintf(message, "%d.%s%s%s", moveNumber / 2 + 1,
		WhiteOnMove(moveNumber) ? " " : ".. ",
		parseList[moveNumber], res);
	DisplayMessage(message, cpThinkOutput);
    }
}

void
DisplayAnalysisText(text)
     char *text;
{
    char buf[MSG_SIZ];

    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile 
		|| appData.icsEngineAnalyze) {
	sprintf(buf, "Analysis (%s)", first.tidy);
	AnalysisPopUp(buf, text);
    }
}

static int
only_one_move(str)
     char *str;
{
    while (*str && isspace(*str)) ++str;
    while (*str && !isspace(*str)) ++str;
    if (!*str) return 1;
    while (*str && isspace(*str)) ++str;
    if (!*str) return 1;
    return 0;
}

void
DisplayAnalysis()
{
    char buf[MSG_SIZ];
    double nps;
    static char *xtra[] = { "", " (--)", " (++)" };
    int h, m, s, cs;

    if (programStats.time == 0) {
	programStats.time = 1;
    }

    if (programStats.got_only_move) {
	strcpy(buf, programStats.movelist);
    } else {
	nps = (u64ToDouble(programStats.nodes) /
	      ((double)programStats.time /100.0));

	cs = programStats.time % 100;
	s = programStats.time / 100;
	h = (s / (60*60));
	s = s - h*60*60;
	m = (s/60);
	s = s - m*60;

	if (programStats.moves_left > 0 && appData.periodicUpdates) {
	  if (programStats.move_name[0] != NULLCHAR) {
	    sprintf(buf, "depth=%d %d/%d(%s) %+.2f %s%s\nNodes: "u64Display" NPS: %d\nTime: %02d:%02d:%02d.%02d",
		    programStats.depth,
		    programStats.nr_moves-programStats.moves_left,
		    programStats.nr_moves, programStats.move_name,
		    ((float)programStats.score)/100.0, programStats.movelist,
		    only_one_move(programStats.movelist)?
		    xtra[programStats.got_fail] : "",
		    (u64)programStats.nodes, (int)nps, h, m, s, cs);
	  } else {
	    sprintf(buf, "depth=%d %d/%d %+.2f %s%s\nNodes: "u64Display" NPS: %d\nTime: %02d:%02d:%02d.%02d",
		    programStats.depth,
		    programStats.nr_moves-programStats.moves_left,
		    programStats.nr_moves, ((float)programStats.score)/100.0,
		    programStats.movelist,
		    only_one_move(programStats.movelist)?
		    xtra[programStats.got_fail] : "",
		    (u64)programStats.nodes, (int)nps, h, m, s, cs);
	  }
	} else {
	    sprintf(buf, "depth=%d %+.2f %s%s\nNodes: "u64Display" NPS: %d\nTime: %02d:%02d:%02d.%02d",
		    programStats.depth,
		    ((float)programStats.score)/100.0,
		    programStats.movelist,
		    only_one_move(programStats.movelist)?
		    xtra[programStats.got_fail] : "",
		    (u64)programStats.nodes, (int)nps, h, m, s, cs);
	}
    }
    DisplayAnalysisText(buf);
}

void
DisplayComment(moveNumber, text)
     int moveNumber;
     char *text;
{
    char title[MSG_SIZ];

    if (moveNumber < 0 || parseList[moveNumber][0] == NULLCHAR) {
	strcpy(title, "Comment");
    } else {
	sprintf(title, "Comment on %d.%s%s", moveNumber / 2 + 1,
		WhiteOnMove(moveNumber) ? " " : ".. ",
		parseList[moveNumber]);
    }

    CommentPopUp(title, text);
}

/* This routine sends a ^C interrupt to gnuchess, to awaken it if it
 * might be busy thinking or pondering.  It can be omitted if your
 * gnuchess is configured to stop thinking immediately on any user
 * input.  However, that gnuchess feature depends on the FIONREAD
 * ioctl, which does not work properly on some flavors of Unix.
 */
void
Attention(cps)
     ChessProgramState *cps;
{
#if ATTENTION
    if (!cps->useSigint) return;
    if (appData.noChessProgram || (cps->pr == NoProc)) return;
    switch (gameMode) {
      case MachinePlaysWhite:
      case MachinePlaysBlack:
      case TwoMachinesPlay:
      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case AnalyzeMode:
      case AnalyzeFile:
	/* Skip if we know it isn't thinking */
	if (!cps->maybeThinking) return;
	if (appData.debugMode)
	  fprintf(debugFP, "Interrupting %s\n", cps->which);
	InterruptChildProcess(cps->pr);
	cps->maybeThinking = FALSE;
	break;
      default:
	break;
    }
#endif /*ATTENTION*/
}

int
CheckFlags()
{
    if (whiteTimeRemaining <= 0) {
	if (!whiteFlag) {
	    whiteFlag = TRUE;
	    if (appData.icsActive) {
		if (appData.autoCallFlag &&
		    gameMode == IcsPlayingBlack && !blackFlag) {
		  SendToICS(ics_prefix);
		  SendToICS("flag\n");
		}
	    } else {
		if (blackFlag) {
		    DisplayTitle(_("Both flags fell"));
		} else {
	 	    DisplayTitle(_("White's flag fell"));
 		    if (appData.autoCallFlag) {
			GameEnds(BlackWins, "Black wins on time", GE_XBOARD);
			return TRUE;
		    }
		}
	    }
	}
    }
    if (blackTimeRemaining <= 0) {
	if (!blackFlag) {
	    blackFlag = TRUE;
	    if (appData.icsActive) {
		if (appData.autoCallFlag &&
		    gameMode == IcsPlayingWhite && !whiteFlag) {
		  SendToICS(ics_prefix);
		  SendToICS("flag\n");
		}
	    } else {
		if (whiteFlag) {
		    DisplayTitle(_("Both flags fell"));
		} else {
		    DisplayTitle(_("Black's flag fell"));
		    if (appData.autoCallFlag) {
			GameEnds(WhiteWins, "White wins on time", GE_XBOARD);
			return TRUE;
		    }
		}
	    }
	}
    }
    return FALSE;
}

void
CheckTimeControl()
{
    if (!appData.clockMode || appData.icsActive ||
	gameMode == PlayFromGameFile || forwardMostMove == 0) return;

    if (timeIncrement >= 0) {
	if (WhiteOnMove(forwardMostMove)) {
	    blackTimeRemaining += timeIncrement;
	} else {
	    whiteTimeRemaining += timeIncrement;
	}
    }
    /*
     * add time to clocks when time control is achieved
     */
    if (movesPerSession) {
      switch ((forwardMostMove + 1) % (movesPerSession * 2)) {
      case 0:
	/* White made time control */
	whiteTimeRemaining += timeControl;
	break;
      case 1:
	/* Black made time control */
	blackTimeRemaining += timeControl;
	break;
      default:
	break;
      }
    }
}

void
DisplayBothClocks()
{
    int wom = gameMode == EditPosition ?
      !blackPlaysFirst : WhiteOnMove(currentMove);
    DisplayWhiteClock(whiteTimeRemaining, wom);
    DisplayBlackClock(blackTimeRemaining, !wom);
}


/* Timekeeping seems to be a portability nightmare.  I think everyone
   has ftime(), but I'm really not sure, so I'm including some ifdefs
   to use other calls if you don't.  Clocks will be less accurate if
   you have neither ftime nor gettimeofday.
*/

/* Get the current time as a TimeMark */
void
GetTimeMark(tm)
     TimeMark *tm;
{
#if HAVE_GETTIMEOFDAY

    struct timeval timeVal;
    struct timezone timeZone;

    gettimeofday(&timeVal, &timeZone);
    tm->sec = (long) timeVal.tv_sec;
    tm->ms = (int) (timeVal.tv_usec / 1000L);

#else /*!HAVE_GETTIMEOFDAY*/
#if HAVE_FTIME

#include <sys/timeb.h>
    struct timeb timeB;

    ftime(&timeB);
    tm->sec = (long) timeB.time;
    tm->ms = (int) timeB.millitm;

#else /*!HAVE_FTIME && !HAVE_GETTIMEOFDAY*/
    tm->sec = (long) time(NULL);
    tm->ms = 0;
#endif
#endif
}

/* Return the difference in milliseconds between two
   time marks.  We assume the difference will fit in a long!
*/
long
SubtractTimeMarks(tm2, tm1)
     TimeMark *tm2, *tm1;
{
    return 1000L*(tm2->sec - tm1->sec) +
           (long) (tm2->ms - tm1->ms);
}


/*
 * Code to manage the game clocks.
 *
 * In tournament play, black starts the clock and then white makes a move.
 * We give the human user a slight advantage if he is playing white---the
 * clocks don't run until he makes his first move, so it takes zero time.
 * Also, we don't account for network lag, so we could get out of sync
 * with GNU Chess's clock -- but then, referees are always right.
 */

static TimeMark tickStartTM;
static long intendedTickLength;

long
NextTickLength(timeRemaining)
     long timeRemaining;
{
    long nominalTickLength, nextTickLength;

    if (timeRemaining > 0L && timeRemaining <= 10000L)
      nominalTickLength = 100L;
    else
      nominalTickLength = 1000L;
    nextTickLength = timeRemaining % nominalTickLength;
    if (nextTickLength <= 0) nextTickLength += nominalTickLength;

    return nextTickLength;
}

/* Stop clocks and reset to a fresh time control */
void
ResetClocks()
{
    (void) StopClockTimer();
    if (appData.icsActive) {
	whiteTimeRemaining = blackTimeRemaining = 0;
    } else {
	whiteTimeRemaining = blackTimeRemaining = timeControl;
    }
    if (whiteFlag || blackFlag) {
	DisplayTitle("");
	whiteFlag = blackFlag = FALSE;
    }
    DisplayBothClocks();
}

#define FUDGE 25 /* 25ms = 1/40 sec; should be plenty even for 50 Hz clocks */

/* Decrement running clock by amount of time that has passed */
void
DecrementClocks()
{
    long timeRemaining;
    long lastTickLength, fudge;
    TimeMark now;

    if (!appData.clockMode) return;
    if (gameMode==AnalyzeMode || gameMode == AnalyzeFile) return;

    GetTimeMark(&now);

    lastTickLength = SubtractTimeMarks(&now, &tickStartTM);

    /* Fudge if we woke up a little too soon */
    fudge = intendedTickLength - lastTickLength;
    if (fudge < 0 || fudge > FUDGE) fudge = 0;

    if (WhiteOnMove(forwardMostMove)) {
	timeRemaining = whiteTimeRemaining -= lastTickLength;
	DisplayWhiteClock(whiteTimeRemaining - fudge,
			  WhiteOnMove(currentMove));
    } else {
	timeRemaining = blackTimeRemaining -= lastTickLength;
	DisplayBlackClock(blackTimeRemaining - fudge,
			  !WhiteOnMove(currentMove));
    }

    if (CheckFlags()) return;

    tickStartTM = now;
    intendedTickLength = NextTickLength(timeRemaining - fudge) + fudge;
    StartClockTimer(intendedTickLength);

    /* if the time remaining has fallen below the alarm threshold, sound the
     * alarm. if the alarm has sounded and (due to a takeback or time control
     * with increment) the time remaining has increased to a level above the
     * threshold, reset the alarm so it can sound again.
     */

    if (appData.icsActive && appData.icsAlarm) {

	/* make sure we are dealing with the user's clock */
	if (!( ((gameMode == IcsPlayingWhite) && WhiteOnMove(currentMove)) ||
	       ((gameMode == IcsPlayingBlack) && !WhiteOnMove(currentMove))
	   )) return;

	if (alarmSounded && (timeRemaining > appData.icsAlarmTime)) {
	    alarmSounded = FALSE;
	} else if (!alarmSounded && (timeRemaining <= appData.icsAlarmTime)) {
	    PlayAlarmSound();
	    alarmSounded = TRUE;
	}
    }
}


/* A player has just moved, so stop the previously running
   clock and (if in clock mode) start the other one.
   We redisplay both clocks in case we're in ICS mode, because
   ICS gives us an update to both clocks after every move.
   Note that this routine is called *after* forwardMostMove
   is updated, so the last fractional tick must be subtracted
   from the color that is *not* on move now.
*/
void
SwitchClocks()
{
    long lastTickLength;
    TimeMark now;
    int flagged = FALSE;

    GetTimeMark(&now);

    if (StopClockTimer() && appData.clockMode) {
	lastTickLength = SubtractTimeMarks(&now, &tickStartTM);
	if (WhiteOnMove(forwardMostMove)) {
	    blackTimeRemaining -= lastTickLength;
	} else {
	    whiteTimeRemaining -= lastTickLength;
	}
	flagged = CheckFlags();
    }
    CheckTimeControl();

    if (flagged || !appData.clockMode) return;

    switch (gameMode) {
      case MachinePlaysBlack:
      case MachinePlaysWhite:
      case BeginningOfGame:
	if (pausing) return;
	break;

      case EditGame:
      case PlayFromGameFile:
      case IcsExamining:
	return;

      default:
	break;
    }

    tickStartTM = now;
    intendedTickLength = NextTickLength(WhiteOnMove(forwardMostMove) ?
      whiteTimeRemaining : blackTimeRemaining);
    StartClockTimer(intendedTickLength);
}


/* Stop both clocks */
void
StopClocks()
{
    long lastTickLength;
    TimeMark now;

    if (!StopClockTimer()) return;
    if (!appData.clockMode) return;

    GetTimeMark(&now);

    lastTickLength = SubtractTimeMarks(&now, &tickStartTM);
    if (WhiteOnMove(forwardMostMove)) {
	whiteTimeRemaining -= lastTickLength;
	DisplayWhiteClock(whiteTimeRemaining, WhiteOnMove(currentMove));
    } else {
	blackTimeRemaining -= lastTickLength;
	DisplayBlackClock(blackTimeRemaining, !WhiteOnMove(currentMove));
    }
    CheckFlags();
}

/* Start clock of player on move.  Time may have been reset, so
   if clock is already running, stop and restart it. */
void
StartClocks()
{
    (void) StopClockTimer(); /* in case it was running already */
    DisplayBothClocks();
    if (CheckFlags()) return;

    if (!appData.clockMode) return;
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) return;

    GetTimeMark(&tickStartTM);
    intendedTickLength = NextTickLength(WhiteOnMove(forwardMostMove) ?
      whiteTimeRemaining : blackTimeRemaining);
    StartClockTimer(intendedTickLength);
}

char *
TimeString(ms)
     long ms;
{
    long second, minute, hour, day;
    char *sign = "";
    static char buf[32];

    if (ms > 0 && ms <= 9900) {
      /* convert milliseconds to tenths, rounding up */
      double tenths = floor( ((double)(ms + 99L)) / 100.00 );

      sprintf(buf, " %03.1f ", tenths/10.0);
      return buf;
    }

    /* convert milliseconds to seconds, rounding up */
    /* use floating point to avoid strangeness of integer division
       with negative dividends on many machines */
    second = (long) floor(((double) (ms + 999L)) / 1000.0);

    if (second < 0) {
	sign = "-";
	second = -second;
    }

    day = second / (60 * 60 * 24);
    second = second % (60 * 60 * 24);
    hour = second / (60 * 60);
    second = second % (60 * 60);
    minute = second / 60;
    second = second % 60;

    if (day > 0)
      sprintf(buf, " %s%ld:%02ld:%02ld:%02ld ",
	      sign, day, hour, minute, second);
    else if (hour > 0)
      sprintf(buf, " %s%ld:%02ld:%02ld ", sign, hour, minute, second);
    else
      sprintf(buf, " %s%2ld:%02ld ", sign, minute, second);

    return buf;
}


/*
 * This is necessary because some C libraries aren't ANSI C compliant yet.
 */
char *
StrStr(string, match)
     char *string, *match;
{
    int i, length;

    length = strlen(match);

    for (i = strlen(string) - length; i >= 0; i--, string++)
      if (!strncmp(match, string, length))
	return string;

    return NULL;
}

char *
StrCaseStr(string, match)
     char *string, *match;
{
    int i, j, length;

    length = strlen(match);

    for (i = strlen(string) - length; i >= 0; i--, string++) {
	for (j = 0; j < length; j++) {
	    if (ToLower(match[j]) != ToLower(string[j]))
	      break;
	}
	if (j == length) return string;
    }

    return NULL;
}

#ifndef _amigados
int
StrCaseCmp(s1, s2)
     char *s1, *s2;
{
    char c1, c2;

    for (;;) {
	c1 = ToLower(*s1++);
	c2 = ToLower(*s2++);
	if (c1 > c2) return 1;
	if (c1 < c2) return -1;
	if (c1 == NULLCHAR) return 0;
    }
}


int
ToLower(c)
     int c;
{
    return isupper(c) ? tolower(c) : c;
}


int
ToUpper(c)
     int c;
{
    return islower(c) ? toupper(c) : c;
}
#endif /* !_amigados	*/

char *
StrSave(s)
     char *s;
{
    char *ret;

    if ((ret = (char *) malloc(strlen(s) + 1))) {
	strcpy(ret, s);
    }
    return ret;
}

char *
StrSavePtr(s, savePtr)
     char *s, **savePtr;
{
    if (*savePtr) {
	free(*savePtr);
    }
    if ((*savePtr = (char *) malloc(strlen(s) + 1))) {
	strcpy(*savePtr, s);
    }
    return(*savePtr);
}

char *
PGNDate()
{
    time_t clock;
    struct tm *tm;
    char buf[MSG_SIZ];

    clock = time((time_t *)NULL);
    tm = localtime(&clock);
    sprintf(buf, "%04d.%02d.%02d",
	    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    return StrSave(buf);
}


char *
PositionToFEN(move)
     int move;
{
    int i, j, fromX, fromY, toX, toY;
    int whiteToPlay;
    char buf[128];
    char *p, *q;
    int emptycount;

    whiteToPlay = (gameMode == EditPosition) ?
      !blackPlaysFirst : (move % 2 == 0);
    p = buf;

    /* Piece placement data */
    for (i = BOARD_SIZE - 1; i >= 0; i--) {
	emptycount = 0;
	for (j = 0; j < BOARD_SIZE; j++) {
	    if (boards[move][i][j] == EmptySquare) {
		emptycount++;
	    } else {
		if (emptycount > 0) {
		    *p++ = '0' + emptycount;
		    emptycount = 0;
		}
		*p++ = PieceToChar(boards[move][i][j]);
	    }
	}
	if (emptycount > 0) {
	    *p++ = '0' + emptycount;
	    emptycount = 0;
	}
	*p++ = '/';
    }
    *(p - 1) = ' ';

    /* Active color */
    *p++ = whiteToPlay ? 'w' : 'b';
    *p++ = ' ';

    /* !!We don't keep track of castling availability, so fake it */
    q = p;
    if (boards[move][0][4] == WhiteKing) {
	if (boards[move][0][7] == WhiteRook) *p++ = 'K';
	if (boards[move][0][0] == WhiteRook) *p++ = 'Q';
    }
    if (boards[move][7][4] == BlackKing) {
	if (boards[move][7][7] == BlackRook) *p++ = 'k';
	if (boards[move][7][0] == BlackRook) *p++ = 'q';
    }
    if (q == p) *p++ = '-';
    *p++ = ' ';

    /* En passant target square */
    if (move > backwardMostMove) {
	fromX = moveList[move - 1][0] - 'a';
	fromY = moveList[move - 1][1] - '1';
	toX = moveList[move - 1][2] - 'a';
	toY = moveList[move - 1][3] - '1';
	if (fromY == (whiteToPlay ? 6 : 1) &&
	    toY == (whiteToPlay ? 4 : 3) &&
	    boards[move][toY][toX] == (whiteToPlay ? BlackPawn : WhitePawn) &&
	    fromX == toX) {
	    /* 2-square pawn move just happened */
	    *p++ = toX + 'a';
	    *p++ = whiteToPlay ? '6' : '3';
	} else {
	    *p++ = '-';
	}
    } else {
	*p++ = '-';
    }

    /* !!We don't keep track of halfmove clock for 50-move rule */
    strcpy(p, " 0 ");
    p += 3;

    /* Fullmove number */
    sprintf(p, "%d", (move / 2) + 1);

    return StrSave(buf);
}

Boolean
ParseFEN(board, blackPlaysFirst, fen)
     Board board;
     int *blackPlaysFirst;
     char *fen;
{
    int i, j;
    char *p;
    int emptycount;

    p = fen;

    /* Piece placement data */
    for (i = BOARD_SIZE - 1; i >= 0; i--) {
	j = 0;
	for (;;) {
	    if (*p == '/' || *p == ' ') {
		if (*p == '/') p++;
		emptycount = BOARD_SIZE - j;
		while (emptycount--) board[i][j++] = EmptySquare;
		break;
	    } else if (isdigit(*p)) {
		emptycount = *p++ - '0';
		if (j + emptycount > BOARD_SIZE) return FALSE;
		while (emptycount--) board[i][j++] = EmptySquare;
	    } else if (isalpha(*p)) {
		if (j >= BOARD_SIZE) return FALSE;
		board[i][j++] = CharToPiece(*p++);
	    } else {
		return FALSE;
	    }
	}
    }
    while (*p == '/' || *p == ' ') p++;

    /* Active color */
    switch (*p) {
      case 'w':
	*blackPlaysFirst = FALSE;
	break;
      case 'b':
	*blackPlaysFirst = TRUE;
	break;
      default:
	return FALSE;
    }
    /* !!We ignore the rest of the FEN notation */
    return TRUE;
}

void
EditPositionPasteFEN(char *fen)
{
  if (fen != NULL) {
    Board initial_position;

    if (!ParseFEN(initial_position, &blackPlaysFirst, fen)) {
      DisplayError(_("Bad FEN position in clipboard"), 0);
      return ;
    } else {
      int savedBlackPlaysFirst = blackPlaysFirst;
      EditPositionEvent();
      blackPlaysFirst = savedBlackPlaysFirst;
      CopyBoard(boards[0], initial_position);
      EditPositionDone();
      DisplayBothClocks();
      DrawPosition(FALSE, boards[currentMove]);
    }
  }
}
