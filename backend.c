/*
 * backend.c -- Common back end for X and Windows NT versions of
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
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
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
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

/* [AS] Also useful here for debugging */
#ifdef WIN32
#include <windows.h>

#define DoSleep( n ) if( (n) != 0 ) Sleep( (n) );

#else

#define DoSleep( n ) if( (n) >= 0) sleep(n)

#endif

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
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
# define _(s) gettext (s) 
# define N_(s) gettext_noop (s) 
#else 
# define _(s) (s) 
# define N_(s) s 
#endif 


/* A point in time */
typedef struct {
    long sec;  /* Assuming this is >= 32 bits */
    int ms;    /* Assuming this is >= 16 bits */
} TimeMark;

int establish P((void));
void read_from_player P((InputSourceRef isr, VOIDSTAR closure,
			 char *buf, int count, int error));
void read_from_ics P((InputSourceRef isr, VOIDSTAR closure,
		      char *buf, int count, int error));
void SendToICS P((char *s));
void SendToICSDelayed P((char *s, long msdelay));
void SendMoveToICS P((ChessMove moveType, int fromX, int fromY,
		      int toX, int toY));
void HandleMachineMove P((char *message, ChessProgramState *cps));
int AutoPlayOneMove P((void));
int LoadGameOneMove P((ChessMove readAhead));
int LoadGameFromFile P((char *filename, int n, char *title, int useList));
int LoadPositionFromFile P((char *filename, int n, char *title));
int SavePositionToFile P((char *filename));
void ApplyMove P((int fromX, int fromY, int toX, int toY, int promoChar,
		  Board board, char *castle, char *ep));
void MakeMove P((int fromX, int fromY, int toX, int toY, int promoChar));
void ShowMove P((int fromX, int fromY, int toX, int toY));
int FinishMove P((ChessMove moveType, int fromX, int fromY, int toX, int toY,
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
void InitChessProgram P((ChessProgramState *cps, int setup));
void OutputKibitz(int window, char *text);
int PerpetualChase(int first, int last);
int EngineOutputIsUp();
void InitDrawingSizes(int x, int y);

#ifdef WIN32
       extern void ConsoleCreate();
#endif

ChessProgramState *WhitePlayer();
void InsertIntoMemo P((int which, char *text)); // [HGM] kibitz: in engineo.c
int VerifyDisplayMode P(());

char *GetInfoFromComment( int, char * ); // [HGM] PV time: returns stripped comment
void InitEngineUCI( const char * iniDir, ChessProgramState * cps ); // [HGM] moved here from winboard.c
char *ProbeBook P((int moveNr, char *book)); // [HGM] book: returns a book move
char *SendMoveToBookUser P((int nr, ChessProgramState *cps, int initial)); // [HGM] book
extern char installDir[MSG_SIZ];

extern int tinyLayout, smallLayout;
ChessProgramStats programStats;
static int exiting = 0; /* [HGM] moved to top */
static int setboardSpoiledMachineBlack = 0 /*, errorExitFlag = 0*/;
int startedFromPositionFile = FALSE; Board filePosition;       /* [HGM] loadPos */
char endingGame = 0;    /* [HGM] crash: flag to prevent recursion of GameEnds() */
int whiteNPS, blackNPS; /* [HGM] nps: for easily making clocks aware of NPS     */
VariantClass currentlyInitializedVariant; /* [HGM] variantswitch */
int lastIndex = 0;      /* [HGM] autoinc: last game/position used in match mode */
int opponentKibitzes;
int lastSavedGame; /* [HGM] save: ID of game */

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
#define GE_ENGINE1 5
#define GE_ENGINE2 6

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

/* [AS] */
static char * safeStrCpy( char * dst, const char * src, size_t count )
{
    assert( dst != NULL );
    assert( src != NULL );
    assert( count > 0 );

    strncpy( dst, src, count );
    dst[ count-1 ] = '\0';
    return dst;
}

#if 0
//[HGM] for future use? Conditioned out for now to suppress warning.
static char * safeStrCat( char * dst, const char * src, size_t count )
{
    size_t  dst_len;

    assert( dst != NULL );
    assert( src != NULL );
    assert( count > 0 );

    dst_len = strlen(dst);

    assert( count > dst_len ); /* Buffer size must be greater than current length */

    safeStrCpy( dst + dst_len, src, count - dst_len );

    return dst;
}
#endif

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
  u64 tmp = value & u64Const(0x7fffffffffffffff);
  r = (double)(s64)tmp;
  if (value & u64Const(0x8000000000000000))
       r +=  9.2233720368547758080e18; /* 2^63 */
 return r;
}

/* Fake up flags for now, as we aren't keeping track of castling
   availability yet. [HGM] Change of logic: the flag now only
   indicates the type of castlings allowed by the rule of the game.
   The actual rights themselves are maintained in the array
   castlingRights, as part of the game history, and are not probed
   by this function.
 */
int
PosFlags(index)
{
  int flags = F_ALL_CASTLE_OK;
  if ((index % 2) == 0) flags |= F_WHITE_ON_MOVE;
  switch (gameInfo.variant) {
  case VariantSuicide:
    flags &= ~F_ALL_CASTLE_OK;
  case VariantGiveaway:		// [HGM] moved this case label one down: seems Giveaway does have castling on ICC!
    flags |= F_IGNORE_CHECK;
  case VariantLosers:
    flags |= F_MANDATORY_CAPTURE; //[HGM] losers: sets flag so TestLegality rejects non-capts if capts exist
    break;
  case VariantAtomic:
    flags |= F_IGNORE_CHECK | F_ATOMIC_CAPTURE;
    break;
  case VariantKriegspiel:
    flags |= F_KRIEGSPIEL_CAPTURE;
    break;
  case VariantCapaRandom: 
  case VariantFischeRandom:
    flags |= F_FRC_TYPE_CASTLING; /* [HGM] enable this through flag */
  case VariantNoCastle:
  case VariantShatranj:
  case VariantCourier:
    flags &= ~F_ALL_CASTLE_OK;
    break;
  default:
    break;
  }
  return flags;
}

FILE *gameFileFP, *debugFP;

/* 
    [AS] Note: sometimes, the sscanf() function is used to parse the input
    into a fixed-size buffer. Because of this, we must be prepared to
    receive strings as long as the size of the input buffer, which is currently
    set to 4K for Windows and 8K for the rest.
    So, we must either allocate sufficiently large buffers here, or
    reduce the size of the input buffer in the input reading part.
*/

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
ChessProgramStats_Move pvInfoList[MAX_MOVES]; /* [AS] Info about engine thinking */
int hiddenThinkOutputState = 0; /* [AS] */
int adjudicateLossThreshold = 0; /* [AS] Automatic adjudication */
int adjudicateLossPlies = 6;
char white_holding[64], black_holding[64];
TimeMark lastNodeCountTime;
long lastNodeCount=0;
int have_sent_ICS_logon = 0;
int movesPerSession;
long whiteTimeRemaining, blackTimeRemaining, timeControl, timeIncrement;
long timeControl_2; /* [AS] Allow separate time controls */
char *fullTimeControlString = NULL; /* [HGM] secondary TC: merge of MPS, TC and inc */
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
/* [HGM] Following 7 needed for accurate legality tests: */
char  epStatus[MAX_MOVES];
char  castlingRights[MAX_MOVES][BOARD_SIZE]; // stores files for pieces with castling rights or -1
char  castlingRank[BOARD_SIZE]; // and corresponding ranks
char  initialRights[BOARD_SIZE], FENcastlingRights[BOARD_SIZE], fileRights[BOARD_SIZE];
int   nrCastlingRights; // For TwoKings, or to implement castling-unknown status
int   initialRulePlies, FENrulePlies;
char  FENepStatus;
FILE  *serverMoves = NULL; // next two for broadcasting (/serverMoves option)
int loadFlag = 0; 
int shuffleOpenings;

ChessSquare  FIDEArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteKnight, WhiteBishop, WhiteQueen,
	WhiteKing, WhiteBishop, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackBishop, BlackQueen,
	BlackKing, BlackBishop, BlackKnight, BlackRook }
};

ChessSquare twoKingsArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteKnight, WhiteBishop, WhiteQueen,
	WhiteKing, WhiteKing, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackBishop, BlackQueen,
        BlackKing, BlackKing, BlackKnight, BlackRook }
};

ChessSquare  KnightmateArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteMan, WhiteBishop, WhiteQueen,
        WhiteUnicorn, WhiteBishop, WhiteMan, WhiteRook },
    { BlackRook, BlackMan, BlackBishop, BlackQueen,
        BlackUnicorn, BlackBishop, BlackMan, BlackRook }
};

ChessSquare fairyArray[2][BOARD_SIZE] = { /* [HGM] Queen side differs from King side */
    { WhiteCannon, WhiteNightrider, WhiteAlfil, WhiteQueen,
        WhiteKing, WhiteBishop, WhiteKnight, WhiteRook },
    { BlackCannon, BlackNightrider, BlackAlfil, BlackQueen,
	BlackKing, BlackBishop, BlackKnight, BlackRook }
};

ChessSquare ShatranjArray[2][BOARD_SIZE] = { /* [HGM] (movGen knows about Shatranj Q and P) */
    { WhiteRook, WhiteKnight, WhiteAlfil, WhiteKing,
        WhiteFerz, WhiteAlfil, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackAlfil, BlackKing,
        BlackFerz, BlackAlfil, BlackKnight, BlackRook }
};


#if (BOARD_SIZE>=10)
ChessSquare ShogiArray[2][BOARD_SIZE] = {
    { WhiteQueen, WhiteKnight, WhiteFerz, WhiteWazir,
        WhiteKing, WhiteWazir, WhiteFerz, WhiteKnight, WhiteQueen },
    { BlackQueen, BlackKnight, BlackFerz, BlackWazir,
        BlackKing, BlackWazir, BlackFerz, BlackKnight, BlackQueen }
};

ChessSquare XiangqiArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteKnight, WhiteAlfil, WhiteFerz,
        WhiteWazir, WhiteFerz, WhiteAlfil, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackAlfil, BlackFerz,
        BlackWazir, BlackFerz, BlackAlfil, BlackKnight, BlackRook }
};

ChessSquare CapablancaArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteKnight, WhiteAngel, WhiteBishop, WhiteQueen, 
        WhiteKing, WhiteBishop, WhiteMarshall, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackAngel, BlackBishop, BlackQueen, 
        BlackKing, BlackBishop, BlackMarshall, BlackKnight, BlackRook }
};

ChessSquare GreatArray[2][BOARD_SIZE] = {
    { WhiteDragon, WhiteKnight, WhiteAlfil, WhiteGrasshopper, WhiteKing, 
        WhiteSilver, WhiteCardinal, WhiteAlfil, WhiteKnight, WhiteDragon },
    { BlackDragon, BlackKnight, BlackAlfil, BlackGrasshopper, BlackKing, 
        BlackSilver, BlackCardinal, BlackAlfil, BlackKnight, BlackDragon },
};

ChessSquare JanusArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteAngel, WhiteKnight, WhiteBishop, WhiteKing, 
        WhiteQueen, WhiteBishop, WhiteKnight, WhiteAngel, WhiteRook },
    { BlackRook, BlackAngel, BlackKnight, BlackBishop, BlackKing, 
        BlackQueen, BlackBishop, BlackKnight, BlackAngel, BlackRook }
};

#ifdef GOTHIC
ChessSquare GothicArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteKnight, WhiteBishop, WhiteQueen, WhiteMarshall, 
        WhiteKing, WhiteAngel, WhiteBishop, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackBishop, BlackQueen, BlackMarshall, 
        BlackKing, BlackAngel, BlackBishop, BlackKnight, BlackRook }
};
#else // !GOTHIC
#define GothicArray CapablancaArray
#endif // !GOTHIC

#ifdef FALCON
ChessSquare FalconArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteKnight, WhiteBishop, WhiteLance, WhiteQueen, 
        WhiteKing, WhiteLance, WhiteBishop, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackBishop, BlackLance, BlackQueen, 
        BlackKing, BlackLance, BlackBishop, BlackKnight, BlackRook }
};
#else // !FALCON
#define FalconArray CapablancaArray
#endif // !FALCON

#else // !(BOARD_SIZE>=10)
#define XiangqiPosition FIDEArray
#define CapablancaArray FIDEArray
#define GothicArray FIDEArray
#define GreatArray FIDEArray
#endif // !(BOARD_SIZE>=10)

#if (BOARD_SIZE>=12)
ChessSquare CourierArray[2][BOARD_SIZE] = {
    { WhiteRook, WhiteKnight, WhiteAlfil, WhiteBishop, WhiteMan, WhiteKing,
        WhiteFerz, WhiteWazir, WhiteBishop, WhiteAlfil, WhiteKnight, WhiteRook },
    { BlackRook, BlackKnight, BlackAlfil, BlackBishop, BlackMan, BlackKing,
        BlackFerz, BlackWazir, BlackBishop, BlackAlfil, BlackKnight, BlackRook }
};
#else // !(BOARD_SIZE>=12)
#define CourierArray CapablancaArray
#endif // !(BOARD_SIZE>=12)


Board initialPosition;


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
    programStats.time = -1;        // [HGM] PGNtime: make invalid to recognize engine output
    programStats.score = 0;
    programStats.got_only_move = 0;
    programStats.got_fail = 0;
    programStats.line_is_book = 0;
}

void
InitBackEnd1()
{
    int matched, min, sec;

    ShowThinkingEvent(); // [HGM] thinking: make sure post/nopost state is set according to options

    GetTimeMark(&programStartTime);
    srand(programStartTime.ms); // [HGM] book: makes sure random is unpredictabe to msec level

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

    /* [AS] Initialize pv info list [HGM] and game state */
    {
        int i, j;

        for( i=0; i<MAX_MOVES; i++ ) {
            pvInfoList[i].depth = -1;
            epStatus[i]=EP_NONE;
            for( j=0; j<BOARD_SIZE; j++ ) castlingRights[i][j] = -1;
        }
    }

    /*
     * Parse timeControl resource
     */
    if (!ParseTimeControl(appData.timeControl, appData.timeIncrement,
			  appData.movesPerSession)) {
	char buf[MSG_SIZ];
	snprintf(buf, sizeof(buf), _("bad timeControl option %s"), appData.timeControl);
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
	    snprintf(buf, sizeof(buf), _("bad searchTime option %s"), appData.searchTime);
	    DisplayFatalError(buf, 0, 2);
	}
    }

    /* [AS] Adjudication threshold */
    adjudicateLossThreshold = appData.adjudicateLossThreshold;
    
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
    first.nps = appData.firstNPS;   // [HGM] nps: copy nodes per second
    second.nps = appData.secondNPS;
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

    /* New features added by Tord: */
    first.useFEN960 = FALSE; second.useFEN960 = FALSE;
    first.useOOCastle = TRUE; second.useOOCastle = TRUE;
    /* End of new features added by Tord. */
    first.fenOverride  = appData.fenOverride1;
    second.fenOverride = appData.fenOverride2;

    /* [HGM] time odds: set factor for each machine */
    first.timeOdds  = appData.firstTimeOdds;
    second.timeOdds = appData.secondTimeOdds;
    { int norm = 1;
        if(appData.timeOddsMode) {
            norm = first.timeOdds;
            if(norm > second.timeOdds) norm = second.timeOdds;
        }
        first.timeOdds /= norm;
        second.timeOdds /= norm;
    }

    /* [HGM] secondary TC: how to handle sessions that do not fit in 'level'*/
    first.accumulateTC = appData.firstAccumulateTC;
    second.accumulateTC = appData.secondAccumulateTC;
    first.maxNrOfSessions = second.maxNrOfSessions = 1;

    /* [HGM] debug */
    first.debug = second.debug = FALSE;
    first.supportsNPS = second.supportsNPS = UNKNOWN;

    /* [HGM] options */
    first.optionSettings  = appData.firstOptions;
    second.optionSettings = appData.secondOptions;

    first.scoreIsAbsolute = appData.firstScoreIsAbsolute; /* [AS] */
    second.scoreIsAbsolute = appData.secondScoreIsAbsolute; /* [AS] */
    first.isUCI = appData.firstIsUCI; /* [AS] */
    second.isUCI = appData.secondIsUCI; /* [AS] */
    first.hasOwnBookUCI = appData.firstHasOwnBookUCI; /* [AS] */
    second.hasOwnBookUCI = appData.secondHasOwnBookUCI; /* [AS] */

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
	programVersion = (char*) malloc(5 + strlen(PACKAGE_STRING));
	sprintf(programVersion, "%s", PACKAGE_STRING);
    } else {
#if 0
	char *p, *q;
	q = first.program;
	while (*q != ' ' && *q != NULLCHAR) q++;
	p = q;
	while (p > first.program && *(p-1) != '/' && *(p-1) != '\\') p--; /* [HGM] backslash added */
	programVersion = (char*) malloc(8 + strlen(PACKAGE_STRING + (q - p));
	sprintf(programVersion, "%s + ", PACKAGE_STRING);
	strncat(programVersion, p, q - p);
#else
	/* [HGM] tidy: use tidy name, in stead of full pathname (which was probably a bug due to / vs \ ) */
	programVersion = (char*) malloc(8 + strlen(PACKAGE_STRING) + strlen(first.tidy));
	sprintf(programVersion, "%s + %s", PACKAGE_STRING, first.tidy);
#endif
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
      /* case VariantFischeRandom: (Fabien: moved below) */
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

      case VariantXiangqi:    /* [HGM] repetition rules not implemented */
      case VariantFairy:      /* [HGM] TestLegality definitely off! */
      case VariantGothic:     /* [HGM] should work */
      case VariantCapablanca: /* [HGM] should work */
      case VariantCourier:    /* [HGM] initial forced moves not implemented */
      case VariantShogi:      /* [HGM] drops not tested for legality */
      case VariantKnightmate: /* [HGM] should work */
      case VariantCylinder:   /* [HGM] untested */
      case VariantFalcon:     /* [HGM] untested */
      case VariantCrazyhouse: /* holdings not shown, ([HGM] fixed that!)
			         offboard interposition not understood */
      case VariantNormal:     /* definitely works! */
      case VariantWildCastle: /* pieces not automatically shuffled */
      case VariantNoCastle:   /* pieces not automatically shuffled */
      case VariantFischeRandom: /* [HGM] works and shuffles pieces */
      case VariantLosers:     /* should work except for win condition,
			         and doesn't know captures are mandatory */
      case VariantSuicide:    /* should work except for win condition,
			         and doesn't know captures are mandatory */
      case VariantGiveaway:   /* should work except for win condition,
			         and doesn't know captures are mandatory */
      case VariantTwoKings:   /* should work */
      case VariantAtomic:     /* should work except for win condition */
      case Variant3Check:     /* should work except for win condition */
      case VariantShatranj:   /* should work except for all win conditions */
      case VariantBerolina:   /* might work if TestLegality is off */
      case VariantCapaRandom: /* should work */
      case VariantJanus:      /* should work */
      case VariantSuper:      /* experimental */
      case VariantGreat:      /* experimental, requires legality testing to be off */
	break;
      }
    }

    InitEngineUCI( installDir, &first );  // [HGM] moved here from winboard.c, to make available in xboard
    InitEngineUCI( installDir, &second );
}

int NextIntegerFromString( char ** str, long * value )
{
    int result = -1;
    char * s = *str;

    while( *s == ' ' || *s == '\t' ) {
        s++;
    }

    *value = 0;

    if( *s >= '0' && *s <= '9' ) {
        while( *s >= '0' && *s <= '9' ) {
            *value = *value * 10 + (*s - '0');
            s++;
        }

        result = 0;
    }

    *str = s;

    return result;
}

int NextTimeControlFromString( char ** str, long * value )
{
    long temp;
    int result = NextIntegerFromString( str, &temp );

    if( result == 0 ) {
        *value = temp * 60; /* Minutes */
        if( **str == ':' ) {
            (*str)++;
            result = NextIntegerFromString( str, &temp );
            *value += temp; /* Seconds */
        }
    }

    return result;
}

int NextSessionFromString( char ** str, int *moves, long * tc, long *inc)
{   /* [HGM] routine added to read '+moves/time' for secondary time control */
    int result = -1; long temp, temp2;

    if(**str != '+') return -1; // old params remain in force!
    (*str)++;
    if( NextTimeControlFromString( str, &temp ) ) return -1;

    if(**str != '/') {
        /* time only: incremental or sudden-death time control */
        if(**str == '+') { /* increment follows; read it */
            (*str)++;
            if(result = NextIntegerFromString( str, &temp2)) return -1;
            *inc = temp2 * 1000;
        } else *inc = 0;
        *moves = 0; *tc = temp * 1000; 
        return 0;
    } else if(temp % 60 != 0) return -1;     /* moves was given as min:sec */

    (*str)++; /* classical time control */
    result = NextTimeControlFromString( str, &temp2);
    if(result == 0) {
        *moves = temp/60;
        *tc    = temp2 * 1000;
        *inc   = 0;
    }
    return result;
}

int GetTimeQuota(int movenr)
{   /* [HGM] get time to add from the multi-session time-control string */
    int moves=1; /* kludge to force reading of first session */
    long time, increment;
    char *s = fullTimeControlString;

    if(appData.debugMode) fprintf(debugFP, "TC string = '%s'\n", fullTimeControlString);
    do {
        if(moves) NextSessionFromString(&s, &moves, &time, &increment);
        if(appData.debugMode) fprintf(debugFP, "mps=%d tc=%d inc=%d\n", moves, (int) time, (int) increment);
        if(movenr == -1) return time;    /* last move before new session     */
        if(!moves) return increment;     /* current session is incremental   */
        if(movenr >= 0) movenr -= moves; /* we already finished this session */
    } while(movenr >= -1);               /* try again for next session       */

    return 0; // no new time quota on this move
}

int
ParseTimeControl(tc, ti, mps)
     char *tc;
     int ti;
     int mps;
{
#if 0
    int matched, min, sec;

    matched = sscanf(tc, "%d:%d", &min, &sec);
    if (matched == 1) {
	timeControl = min * 60 * 1000;
    } else if (matched == 2) {
	timeControl = (min * 60 + sec) * 1000;
    } else {
	return FALSE;
    }
#else
    long tc1;
    long tc2;
    char buf[MSG_SIZ];

    if(ti >= 0 && !strchr(tc, '+') && !strchr(tc, '/') ) mps = 0;
    if(ti > 0) {
        if(mps)
             sprintf(buf, "+%d/%s+%d", mps, tc, ti);
        else sprintf(buf, "+%s+%d", tc, ti);
    } else {
        if(mps)
             sprintf(buf, "+%d/%s", mps, tc);
        else sprintf(buf, "+%s", tc);
    }
    fullTimeControlString = StrSave(buf);

    if( NextTimeControlFromString( &tc, &tc1 ) != 0 ) {
        return FALSE;
    }

    if( *tc == '/' ) {
        /* Parse second time control */
        tc++;

        if( NextTimeControlFromString( &tc, &tc2 ) != 0 ) {
            return FALSE;
        }

        if( tc2 == 0 ) {
            return FALSE;
        }

        timeControl_2 = tc2 * 1000;
    }
    else {
        timeControl_2 = 0;
    }

    if( tc1 == 0 ) {
        return FALSE;
    }

    timeControl = tc1 * 1000;
#endif

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
    if(appData.matchMode && appData.sameColorGames > 0) /* [HGM] alternate: overrule matchGames */
	appData.matchGames = appData.sameColorGames;
    if(appData.rewindIndex > 1) { /* [HGM] autoinc: rewind implies auto-increment and overrules given index */
	if(appData.loadPositionIndex >= 0) appData.loadPositionIndex = -1;
	if(appData.loadGameIndex >= 0) appData.loadGameIndex = -1;
    }
    Reset(TRUE, FALSE);
    if (appData.noChessProgram || first.protocolVersion == 1) {
      InitBackEnd3();
    } else {
      /* kludge: allow timeout for initial "feature" commands */
      FreezeUI();
      DisplayMessage("", _("Starting chess program"));
      ScheduleDelayedEvent(InitBackEnd3, FEATURE_TIMEOUT);
    }
}

void
InitBackEnd3 P((void))
{
    GameMode initialMode;
    char buf[MSG_SIZ];
    int err;

    InitChessProgram(&first, startedFromSetupPosition);


    if (appData.icsActive) {
#ifdef WIN32
        /* [DM] Make a console window if needed [HGM] merged ifs */
        ConsoleCreate(); 
#endif
	err = establish();
	if (err != 0) {
	    if (*appData.icsCommPort != NULLCHAR) {
		sprintf(buf, _("Could not open comm port %s"),  
			appData.icsCommPort);
	    } else {
		snprintf(buf, sizeof(buf), _("Could not connect to host %s, port %s"),  
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
	    int index = appData.loadGameIndex; // [HGM] autoinc
	    if(index<0) lastIndex = index = 1;
	    if (!LoadGameFromFile(appData.loadGameFile,
				  index,
				  appData.loadGameFile, FALSE)) {
		DisplayFatalError(_("Bad game file"), 0, 1);
		return;
	    }
	} else if (*appData.loadPositionFile != NULLCHAR) {
	    int index = appData.loadPositionIndex; // [HGM] autoinc
	    if(index<0) lastIndex = index = 1;
	    if (!LoadPositionFromFile(appData.loadPositionFile,
				      index,
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
            /* [HGM] try to make self-starting even after FEN load */
            /* to allow automatic setup of fairy variants with wtm */
            if(initialMode == BeginningOfGame && !blackPlaysFirst) {
                gameMode = BeginningOfGame;
                setboardSpoiledMachineBlack = 1;
            }
            /* [HGM] loadPos: make that every new game uses the setup */
            /* from file as long as we do not switch variant          */
            if(!blackPlaysFirst) { int i;
                startedFromPositionFile = TRUE;
                CopyBoard(filePosition, boards[0]);
                for(i=0; i<BOARD_SIZE; i++) fileRights[i] = castlingRights[0][i];
            }
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
	  appData.showThinking = TRUE; // [HGM] thinking: moved out of ShowThinkingEvent
	  ShowThinkingEvent();
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
	    snprintf(buf, sizeof(buf), "%s %s %s",
		    appData.telnetProgram, appData.icsHost, appData.icsPort);
	    return OpenRcmd(appData.gateway, appData.remoteUser, buf, &icsPR);

	} else {
	    /* Use the rsh program to run telnet program on a gateway host */
	    if (*appData.remoteUser == NULLCHAR) {
		snprintf(buf, sizeof(buf), "%s %s %s %s %s", appData.remoteShell,
			appData.gateway, appData.telnetProgram,
			appData.icsHost, appData.icsPort);
	    } else {
		snprintf(buf, sizeof(buf), "%s %s -l %s %s %s %s",
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

    /* [HGM] skip over optional board-size prefixes */
    if( sscanf(e, "%dx%d_", &i, &i) == 2 ||
        sscanf(e, "%dx%d+%d_", &i, &i, &i) == 3 ) {
        while( *e++ != '_');
    }

    for (i=0; i<sizeof(variantNames)/sizeof(char*); i++) {
      if (StrCaseStr(e, variantNames[i])) {
	v = (VariantClass) i;
	found = TRUE;
	break;
      }
    }

    if (!found) {
      if ((StrCaseStr(e, "fischer") && StrCaseStr(e, "random"))
	  || StrCaseStr(e, "wild/fr") 
	  || StrCaseStr(e, "frc") || StrCaseStr(e, "960")) {
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
        case 37:
          v = VariantShogi;
	  break;
        case 38:
          v = VariantXiangqi;
	  break;
        case 39:
          v = VariantCourier;
	  break;
        case 40:
          v = VariantGothic;
	  break;
        case 41:
          v = VariantCapablanca;
	  break;
        case 42:
          v = VariantKnightmate;
	  break;
        case 43:
          v = VariantFairy;
          break;
        case 44:
          v = VariantCylinder;
	  break;
        case 45:
          v = VariantFalcon;
	  break;
        case 46:
          v = VariantCapaRandom;
	  break;
        case 47:
          v = VariantBerolina;
	  break;
        case 48:
          v = VariantJanus;
	  break;
        case 49:
          v = VariantSuper;
	  break;
        case 50:
          v = VariantGreat;
	  break;
	case -1:
	  /* Found "wild" or "w" in the string but no number;
	     must assume it's normal chess. */
	  v = VariantNormal;
	  break;
	default:
	  sprintf(buf, _("Unknown wild type %d"), wnum);
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

void
CopyHoldings(Board board, char *holdings, ChessSquare lowestPiece)
{
    /* put the holdings sent to us by the server on the board holdings area */
    int i, j, holdingsColumn, holdingsStartRow, direction, countsColumn;
    char p;
    ChessSquare piece;

    if(gameInfo.holdingsWidth < 2)  return;

    if( (int)lowestPiece >= BlackPawn ) {
        holdingsColumn = 0;
        countsColumn = 1;
        holdingsStartRow = BOARD_HEIGHT-1;
        direction = -1;
    } else {
        holdingsColumn = BOARD_WIDTH-1;
        countsColumn = BOARD_WIDTH-2;
        holdingsStartRow = 0;
        direction = 1;
    }

    for(i=0; i<BOARD_HEIGHT; i++) { /* clear holdings */
        board[i][holdingsColumn] = EmptySquare;
        board[i][countsColumn]   = (ChessSquare) 0;
    }
    while( (p=*holdings++) != NULLCHAR ) {
        piece = CharToPiece( ToUpper(p) );
        if(piece == EmptySquare) continue;
        /*j = (int) piece - (int) WhitePawn;*/
        j = PieceToNumber(piece);
        if(j >= gameInfo.holdingsSize) continue; /* ignore pieces that do not fit */
        if(j < 0) continue;               /* should not happen */
        piece = (ChessSquare) ( (int)piece + (int)lowestPiece );
        board[holdingsStartRow+j*direction][holdingsColumn] = piece;
        board[holdingsStartRow+j*direction][countsColumn]++;
    }

}


void
VariantSwitch(Board board, VariantClass newVariant)
{
   int newHoldingsWidth, newWidth = 8, newHeight = 8, i, j;
   int oldCurrentMove = currentMove, oldForwardMostMove = forwardMostMove, oldBackwardMostMove = backwardMostMove;
//   Board tempBoard; int saveCastling[BOARD_SIZE], saveEP;

   startedFromPositionFile = FALSE;
   if(gameInfo.variant == newVariant) return;

   /* [HGM] This routine is called each time an assignment is made to
    * gameInfo.variant during a game, to make sure the board sizes
    * are set to match the new variant. If that means adding or deleting
    * holdings, we shift the playing board accordingly
    * This kludge is needed because in ICS observe mode, we get boards
    * of an ongoing game without knowing the variant, and learn about the
    * latter only later. This can be because of the move list we requested,
    * in which case the game history is refilled from the beginning anyway,
    * but also when receiving holdings of a crazyhouse game. In the latter
    * case we want to add those holdings to the already received position.
    */


  if (appData.debugMode) {
    fprintf(debugFP, "Switch board from %s to %s\n",
               VariantName(gameInfo.variant), VariantName(newVariant));
    setbuf(debugFP, NULL);
  }
    shuffleOpenings = 0;       /* [HGM] shuffle */
    gameInfo.holdingsSize = 5; /* [HGM] prepare holdings */
    switch(newVariant) {
            case VariantShogi:
              newWidth = 9;  newHeight = 9;
              gameInfo.holdingsSize = 7;
            case VariantBughouse:
            case VariantCrazyhouse:
              newHoldingsWidth = 2; break;
            default:
              newHoldingsWidth = gameInfo.holdingsSize = 0;
    }

    if(newWidth  != gameInfo.boardWidth  ||
       newHeight != gameInfo.boardHeight ||
       newHoldingsWidth != gameInfo.holdingsWidth ) {

        /* shift position to new playing area, if needed */
        if(newHoldingsWidth > gameInfo.holdingsWidth) {
           for(i=0; i<BOARD_HEIGHT; i++) 
               for(j=BOARD_RGHT-1; j>=BOARD_LEFT; j--)
                   board[i][j+newHoldingsWidth-gameInfo.holdingsWidth] =
                                                     board[i][j];
           for(i=0; i<newHeight; i++) {
               board[i][0] = board[i][newWidth+2*newHoldingsWidth-1] = EmptySquare;
               board[i][1] = board[i][newWidth+2*newHoldingsWidth-2] = (ChessSquare) 0;
           }
        } else if(newHoldingsWidth < gameInfo.holdingsWidth) {
           for(i=0; i<BOARD_HEIGHT; i++)
               for(j=BOARD_LEFT; j<BOARD_RGHT; j++)
                   board[i][j+newHoldingsWidth-gameInfo.holdingsWidth] =
                                                 board[i][j];
        }

        gameInfo.boardWidth  = newWidth;
        gameInfo.boardHeight = newHeight;
        gameInfo.holdingsWidth = newHoldingsWidth;
        gameInfo.variant = newVariant;
        InitDrawingSizes(-2, 0);

        /* [HGM] The following should definitely be solved in a better way */
#if 0
        CopyBoard(board, tempBoard); /* save position in case it is board[0] */
        for(i=0; i<BOARD_SIZE; i++) saveCastling[i] = castlingRights[0][i];
        saveEP = epStatus[0];
#endif
        InitPosition(FALSE);          /* this sets up board[0], but also other stuff        */
#if 0
        epStatus[0] = saveEP;
        for(i=0; i<BOARD_SIZE; i++) castlingRights[0][i] = saveCastling[i];
        CopyBoard(tempBoard, board); /* restore position received from ICS   */
#endif
    } else { gameInfo.variant = newVariant; InitPosition(FALSE); }

    forwardMostMove = oldForwardMostMove;
    backwardMostMove = oldBackwardMostMove;
    currentMove = oldCurrentMove; /* InitPos reset these, but we need still to redraw the position */
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

ColorClass curColor = ColorNormal;
int suppressKibitz = 0;

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
    static ColorClass prevColor = ColorNormal;
    static int savingComment = FALSE;
    char str[500];
    int i, oldi;
    int buf_len;
    int next_out;
    int tkind;
    int backup;    /* [DM] For zippy color lines */
    char *p;

    if (appData.debugMode) {
      if (!error) {
	fprintf(debugFP, "<ICS: ");
	show_bytes(debugFP, data, count);
	fprintf(debugFP, "\n");
      }
    }

    if (appData.debugMode) { int f = forwardMostMove;
        fprintf(debugFP, "ics input %d, castling = %d %d %d %d %d %d\n", f,
                castlingRights[f][0],castlingRights[f][1],castlingRights[f][2],castlingRights[f][3],castlingRights[f][4],castlingRights[f][5]);
    }
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
	    if(buf_len >= 5 && buf[buf_len-5]=='\n' && buf[buf_len-4]=='\\' && 
                               buf[buf_len-3]==' '  && buf[buf_len-2]==' '  && buf[buf_len-1]==' ') {
		buf_len -= 5; // [HGM] ICS: join continuation line of Lasker 2.2.3 server with previous
		buf[buf_len++] = ' '; // replace by space (assumes ICS does not break lines within word)
	    }
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
		    if(!suppressKibitz) // [HGM] kibitz
			AppendComment(forwardMostMove, StripHighlight(parse));
		    else { // [HGM kibitz: divert memorized engine kibitz to engine-output window
			int nrDigit = 0, nrAlph = 0, i;
			if(parse_pos > MSG_SIZ - 30) // defuse unreasonably long input
			{ parse_pos = MSG_SIZ-30; parse[parse_pos - 1] = '\n'; }
			parse[parse_pos] = NULLCHAR;
			// try to be smart: if it does not look like search info, it should go to
			// ICS interaction window after all, not to engine-output window.
			for(i=0; i<parse_pos; i++) { // count letters and digits
			    nrDigit += (parse[i] >= '0' && parse[i] <= '9');
			    nrAlph  += (parse[i] >= 'a' && parse[i] <= 'z');
			    nrAlph  += (parse[i] >= 'A' && parse[i] <= 'Z');
			}
			if(nrAlph < 9*nrDigit) { // if more than 10% digit we assume search info
			    int depth=0; float score;
			    if(sscanf(parse, "!!! %f/%d", &score, &depth) == 2 && depth>0) {
				// [HGM] kibitz: save kibitzed opponent info for PGN and eval graph
				pvInfoList[forwardMostMove-1].depth = depth;
				pvInfoList[forwardMostMove-1].score = 100*score;
			    }
			    OutputKibitz(suppressKibitz, parse);
			} else {
			    char tmp[MSG_SIZ];
			    sprintf(tmp, _("your opponent kibitzes: %s"), parse);
			    SendToPlayer(tmp, strlen(tmp));
			}
		    }
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
	      snprintf(buf, sizeof(buf), "%s@%s", ics_handle, appData.icsHost);
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
	    // [HGM] kibitz: try to recognize opponent engine-score kibitzes, to divert them to engine-output window
	    if (appData.autoKibitz && started == STARTED_NONE && 
                !appData.icsEngineAnalyze &&                     // [HGM] [DM] ICS analyze
		(gameMode == IcsPlayingWhite || gameMode == IcsPlayingBlack || gameMode == IcsObserving)) {
		if(looking_at(buf, &i, "* kibitzes: ") &&
		   (StrStr(star_match[0], gameInfo.white) == star_match[0] || 
		    StrStr(star_match[0], gameInfo.black) == star_match[0]   )) { // kibitz of self or opponent
		    	suppressKibitz = TRUE;
			if((StrStr(star_match[0], gameInfo.white) == star_match[0]
				&& (gameMode == IcsPlayingWhite)) ||
			   (StrStr(star_match[0], gameInfo.black) == star_match[0]
				&& (gameMode == IcsPlayingBlack))   ) // opponent kibitz
			    started = STARTED_CHATTER; // own kibitz we simply discard
			else {
			    started = STARTED_COMMENT; // make sure it will be collected in parse[]
			    parse_pos = 0; parse[0] = NULLCHAR;
			    savingComment = TRUE;
			    suppressKibitz = gameMode != IcsObserving ? 2 :
				(StrStr(star_match[0], gameInfo.white) == NULL) + 1;
			} 
			continue;
		} else
		if(looking_at(buf, &i, "kibitzed to")) { // suppress the acknowledgements of our own autoKibitz
		    started = STARTED_CHATTER;
		    suppressKibitz = TRUE;
		}
	    } // [HGM] kibitz: end of patch

	    if (appData.zippyTalk || appData.zippyPlay) {
                /* [DM] Backup address for color zippy lines */
                backup = i;
#if ZIPPY
       #ifdef WIN32
               if (loggedOn == TRUE)
                       if (ZippyControl(buf, &backup) || ZippyConverse(buf, &backup) ||
                          (appData.zippyPlay && ZippyMatch(buf, &backup)));
       #else
                if (ZippyControl(buf, &i) ||
                    ZippyConverse(buf, &i) ||
                    (appData.zippyPlay && ZippyMatch(buf, &i))) {
		      loggedOn = TRUE;
                      if (!appData.colorize) continue;
		}
       #endif
#endif
	    } // [DM] 'else { ' deleted
	 	if (
		    /* Regular tells and says */
		    (tkind = 1, looking_at(buf, &i, "* tells you: ")) ||
		    looking_at(buf, &i, "* (your partner) tells you: ") ||
		    looking_at(buf, &i, "* says: ") ||
		    /* Don't color "message" or "messages" output */
		    (tkind = 5, looking_at(buf, &i, "*. * (*:*): ")) ||
		    looking_at(buf, &i, "*. * at *:*: ") ||
		    looking_at(buf, &i, "--* (*:*): ") ||
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
                /* [HGM] we switched variant. Translate boards if needed. */
                VariantSwitch(boards[currentMove], StringToVariant(gameInfo.event));
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
		 && looking_at(buf, &i, "}*"))) { char *bookHit = NULL; // [HGM] book
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
#if 0
				if (first.useColors) {
				  SendToProgram("white\ngo\n", &first);
				} else {
				  SendToProgram("go\n", &first);
				}
#else
				if (first.useColors) {
				  SendToProgram("white\n", &first); // [HGM] book: made sending of "go\n" book dependent
				}
				bookHit = SendMoveToBookUser(forwardMostMove-1, &first, TRUE); // [HGM] book: probe book for initial pos
#endif
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
#if 0
				if (first.useColors) {
				  SendToProgram("black\ngo\n", &first);
				} else {
				  SendToProgram("go\n", &first);
				}
#else
				if (first.useColors) {
				  SendToProgram("black\n", &first);
				}
				bookHit = SendMoveToBookUser(forwardMostMove-1, &first, TRUE);
#endif
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
		if(bookHit) { // [HGM] book: simulate book reply
		    static char bookMove[MSG_SIZ]; // a bit generous?

		    programStats.nodes = programStats.depth = programStats.time = 
		    programStats.score = programStats.got_only_move = 0;
		    sprintf(programStats.movelist, "%s (xbook)", bookHit);

		    strcpy(bookMove, "move ");
		    strcat(bookMove, bookHit);
		    HandleMachineMove(bookMove, &first);
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
//	    if (ics_user_moved) {
	    if (1) { // [HGM] old way ignored error after move type in; ics_user_moved is not set then!
		if (looking_at(buf, &i, "Illegal move") ||
		    looking_at(buf, &i, "Not a legal move") ||
		    looking_at(buf, &i, "Your king is in check") ||
		    looking_at(buf, &i, "It isn't your turn") ||
		    looking_at(buf, &i, "It is not your move")) {
		    /* Illegal move */
		    if (ics_user_moved && forwardMostMove > backwardMostMove) { // only backup if we already moved
			currentMove = --forwardMostMove;
			DisplayMove(currentMove - 1); /* before DMError */
			DrawPosition(FALSE, boards[currentMove]);
			SwitchClocks();
			DisplayBothClocks();
		    }
		    DisplayMoveError(_("Illegal move (rejected by ICS)")); // [HGM] but always relay error msg
		    ics_user_moved = 0;
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
		    gameInfo.variant = VariantNormal; // [HGM] variantswitch: suppress sending of 'variant'
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
                      fprintf(debugFP, "Parsing holdings: %s, currentMove = %d\n",
                                                        parse, currentMove);
		    if (sscanf(parse, " game %d", &gamenum) == 1 &&
			gamenum == ics_gamenum) {
		        if (gameInfo.variant == VariantNormal) {
                          /* [HGM] We seem to switch variant during a game!
                           * Presumably no holdings were displayed, so we have
                           * to move the position two files to the right to
                           * create room for them!
                           */
                          VariantSwitch(boards[currentMove], VariantCrazyhouse); /* temp guess */
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
                        /* [HGM] copy holdings to board holdings area */
                        CopyHoldings(boards[currentMove], white_holding, WhitePawn);
                        CopyHoldings(boards[currentMove], black_holding, BlackPawn);
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

                        DrawPosition(FALSE, boards[currentMove]);
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
	
	if (started != STARTED_MOVES && started != STARTED_BOARD && !suppressKibitz && // [HGM] kibitz suppress printing in ICS interaction window
	    started != STARTED_HOLDINGS && i > next_out) {
	    SendToPlayer(&buf[next_out], i - next_out);
	    next_out = i;
	}
	suppressKibitz = FALSE; // [HGM] kibitz: has done its duty in if-statement above
	
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

#define PATTERN "%c%d%d%d%d%d%d%d%s%s%d%d%d%d%d%d%d%d%s%s%s%d%d"

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
    char to_play, board_chars[200];
    char move_str[500], str[500], elapsed_time[500];
    char black[32], white[32];
    Board board;
    int prevMove = currentMove;
    int ticking = 2;
    ChessMove moveType;
    int fromX, fromY, toX, toY;
    char promoChar;
    int ranks=1, files=0; /* [HGM] ICS80: allow variable board size */
    char *bookHit = NULL; // [HGM] book

    fromX = fromY = toX = toY = -1;
    
    newGame = FALSE;

    if (appData.debugMode)
      fprintf(debugFP, _("Parsing board: %s\n"), string);

    move_str[0] = NULLCHAR;
    elapsed_time[0] = NULLCHAR;
    {   /* [HGM] figure out how many ranks and files the board has, for ICS extension used by Capablanca server */
        int  i = 0, j;
        while(i < 199 && (string[i] != ' ' || string[i+2] != ' ')) {
	    if(string[i] == ' ') { ranks++; files = 0; }
            else files++;
	    i++;
	}
	for(j = 0; j <i; j++) board_chars[j] = string[j];
        board_chars[i] = '\0';
	string += i + 1;
    }
    n = sscanf(string, PATTERN, &to_play, &double_push,
	       &castle_ws, &castle_wl, &castle_bs, &castle_bl, &irrev_count,
	       &gamenum, white, black, &relation, &basetime, &increment,
	       &white_stren, &black_stren, &white_time, &black_time,
	       &moveNum, str, elapsed_time, move_str, &ics_flip,
	       &ticking);

    if (n < 21) {
        snprintf(str, sizeof(str), _("Failed to parse board string:\n\"%s\""), string);
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
        timeControl_2 = 0;
	timeIncrement = increment * 1000;
	movesPerSession = 0;
	gameInfo.timeControl = TimeControlTagValue();
        VariantSwitch(board, StringToVariant(gameInfo.event) );
  if (appData.debugMode) {
    fprintf(debugFP, "ParseBoard says variant = '%s'\n", gameInfo.event);
    fprintf(debugFP, "recognized as %s\n", VariantName(gameInfo.variant));
    setbuf(debugFP, NULL);
  }

        gameInfo.outOfBook = NULL;
	
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
    
  if (appData.debugMode) {
    fprintf(debugFP, "load %dx%d board\n", files, ranks);
  }
    /* Parse the board */
    for (k = 0; k < ranks; k++) {
      for (j = 0; j < files; j++)
        board[k][j+gameInfo.holdingsWidth] = CharToPiece(board_chars[(ranks-1-k)*(files+1) + j]);
      if(gameInfo.holdingsWidth > 1) {
           board[k][0] = board[k][BOARD_WIDTH-1] = EmptySquare;
           board[k][1] = board[k][BOARD_WIDTH-2] = (ChessSquare) 0;;
      }
    }
    CopyBoard(boards[moveNum], board);
    if (moveNum == 0) {
	startedFromSetupPosition =
	  !CompareBoards(board, initialPosition);
        if(startedFromSetupPosition)
            initialRulePlies = irrev_count; /* [HGM] 50-move counter offset */
    }

    /* [HGM] Set castling rights. Take the outermost Rooks,
       to make it also work for FRC opening positions. Note that board12
       is really defective for later FRC positions, as it has no way to
       indicate which Rook can castle if they are on the same side of King.
       For the initial position we grant rights to the outermost Rooks,
       and remember thos rights, and we then copy them on positions
       later in an FRC game. This means WB might not recognize castlings with
       Rooks that have moved back to their original position as illegal,
       but in ICS mode that is not its job anyway.
    */
    if(moveNum == 0 || gameInfo.variant != VariantFischeRandom)
    { int i, j; ChessSquare wKing = WhiteKing, bKing = BlackKing;

        for(i=BOARD_LEFT, j= -1; i<BOARD_RGHT; i++)
            if(board[0][i] == WhiteRook) j = i;
        initialRights[0] = castlingRights[moveNum][0] = (castle_ws == 0 && gameInfo.variant != VariantFischeRandom ? -1 : j);
        for(i=BOARD_RGHT-1, j= -1; i>=BOARD_LEFT; i--)
            if(board[0][i] == WhiteRook) j = i;
        initialRights[1] = castlingRights[moveNum][1] = (castle_wl == 0 && gameInfo.variant != VariantFischeRandom ? -1 : j);
        for(i=BOARD_LEFT, j= -1; i<BOARD_RGHT; i++)
            if(board[BOARD_HEIGHT-1][i] == BlackRook) j = i;
        initialRights[3] = castlingRights[moveNum][3] = (castle_bs == 0 && gameInfo.variant != VariantFischeRandom ? -1 : j);
        for(i=BOARD_RGHT-1, j= -1; i>=BOARD_LEFT; i--)
            if(board[BOARD_HEIGHT-1][i] == BlackRook) j = i;
        initialRights[4] = castlingRights[moveNum][4] = (castle_bl == 0 && gameInfo.variant != VariantFischeRandom ? -1 : j);

	if(gameInfo.variant == VariantKnightmate) { wKing = WhiteUnicorn; bKing = BlackUnicorn; }
        for(k=BOARD_LEFT; k<BOARD_RGHT; k++)
            if(board[0][k] == wKing) initialRights[2] = castlingRights[moveNum][2] = k;
        for(k=BOARD_LEFT; k<BOARD_RGHT; k++)
            if(board[BOARD_HEIGHT-1][k] == bKing)
                initialRights[5] = castlingRights[moveNum][5] = k;
    } else { int r;
        r = castlingRights[moveNum][0] = initialRights[0];
        if(board[0][r] != WhiteRook) castlingRights[moveNum][0] = -1;
        r = castlingRights[moveNum][1] = initialRights[1];
        if(board[0][r] != WhiteRook) castlingRights[moveNum][1] = -1;
        r = castlingRights[moveNum][3] = initialRights[3];
        if(board[BOARD_HEIGHT-1][r] != BlackRook) castlingRights[moveNum][3] = -1;
        r = castlingRights[moveNum][4] = initialRights[4];
        if(board[BOARD_HEIGHT-1][r] != BlackRook) castlingRights[moveNum][4] = -1;
        /* wildcastle kludge: always assume King has rights */
        r = castlingRights[moveNum][2] = initialRights[2];
        r = castlingRights[moveNum][5] = initialRights[5];
    }
    /* [HGM] e.p. rights. Assume that ICS sends file number here? */
    epStatus[moveNum] = double_push == -1 ? EP_NONE : double_push + BOARD_LEFT;

    
    if (ics_getting_history == H_GOT_REQ_HEADER ||
	ics_getting_history == H_GOT_UNREQ_HEADER) {
	/* This was an initial position from a move list, not
	   the current position */
	return;
    }
    
    /* Update currentMove and known move number limits */
    newMove = newGame || moveNum > forwardMostMove;

    /* [DM] If we found takebacks during icsEngineAnalyze try send to engine */
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
  if (appData.debugMode) {
    if (appData.debugMode) { int f = forwardMostMove;
        fprintf(debugFP, "parseboard %d, castling = %d %d %d %d %d %d\n", f,
                castlingRights[f][0],castlingRights[f][1],castlingRights[f][2],castlingRights[f][3],castlingRights[f][4],castlingRights[f][5]);
    }
    fprintf(debugFP, "accepted move %s from ICS, parse it.\n", move_str);
    fprintf(debugFP, "moveNum = %d\n", moveNum);
    fprintf(debugFP, "board = %d-%d x %d\n", BOARD_LEFT, BOARD_RGHT, BOARD_HEIGHT);
    setbuf(debugFP, NULL);
  }
	if (moveNum <= backwardMostMove) {
	    /* We don't know what the board looked like before
	       this move.  Punt. */
	    strcpy(parseList[moveNum - 1], move_str);
	    strcat(parseList[moveNum - 1], " ");
	    strcat(parseList[moveNum - 1], elapsed_time);
	    moveList[moveNum - 1][0] = NULLCHAR;
	} else if (strcmp(move_str, "none") == 0) {
	    // [HGM] long SAN: swapped order; test for 'none' before parsing move
	    /* Again, we don't know what the board looked like;
	       this is really the start of the game. */
	    parseList[moveNum - 1][0] = NULLCHAR;
	    moveList[moveNum - 1][0] = NULLCHAR;
	    backwardMostMove = moveNum;
	    startedFromSetupPosition = TRUE;
 	    fromX = fromY = toX = toY = -1;
	} else {
	  // [HGM] long SAN: if legality-testing is off, disambiguation might not work or give wrong move. 
	  //                 So we parse the long-algebraic move string in stead of the SAN move
	  int valid; char buf[MSG_SIZ], *prom;

	  // str looks something like "Q/a1-a2"; kill the slash
	  if(str[1] == '/') 
		sprintf(buf, "%c%s", str[0], str+2);
	  else  strcpy(buf, str); // might be castling
	  if((prom = strstr(move_str, "=")) && !strstr(buf, "=")) 
		strcat(buf, prom); // long move lacks promo specification!
	  if(!appData.testLegality && move_str[1] != '@') { // drops never ambiguous (parser chokes on long form!)
		if(appData.debugMode) 
			fprintf(debugFP, "replaced ICS move '%s' by '%s'\n", move_str, buf);
		strcpy(move_str, buf);
          }
	  valid = ParseOneMove(move_str, moveNum - 1, &moveType,
				&fromX, &fromY, &toX, &toY, &promoChar)
	       || ParseOneMove(buf, moveNum - 1, &moveType,
				&fromX, &fromY, &toX, &toY, &promoChar);
	  // end of long SAN patch
	  if (valid) {
	    (void) CoordsToAlgebraic(boards[moveNum - 1],
				     PosFlags(moveNum - 1), EP_UNKNOWN,
				     fromY, fromX, toY, toX, promoChar,
				     parseList[moveNum-1]);
            switch (MateTest(boards[moveNum], PosFlags(moveNum), EP_UNKNOWN,
                             castlingRights[moveNum]) ) {
	      case MT_NONE:
	      case MT_STALEMATE:
	      default:
		break;
	      case MT_CHECK:
                if(gameInfo.variant != VariantShogi)
                    strcat(parseList[moveNum - 1], "+");
		break;
	      case MT_CHECKMATE:
	      case MT_STAINMATE: // [HGM] xq: for notation stalemate that wins counts as checkmate
		strcat(parseList[moveNum - 1], "#");
		break;
	    }
	    strcat(parseList[moveNum - 1], " ");
	    strcat(parseList[moveNum - 1], elapsed_time);
	    /* currentMoveString is set as a side-effect of ParseOneMove */
	    strcpy(moveList[moveNum - 1], currentMoveString);
	    strcat(moveList[moveNum - 1], "\n");
	  } else {
	    /* Move from ICS was illegal!?  Punt. */
  if (appData.debugMode) {
    fprintf(debugFP, "Illegal move from ICS '%s'\n", move_str);
    fprintf(debugFP, "board L=%d, R=%d, H=%d, holdings=%d\n", BOARD_LEFT, BOARD_RGHT, BOARD_HEIGHT, gameInfo.holdingsWidth);
  }
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
	}
  if (appData.debugMode) {
    fprintf(debugFP, "Move parsed to '%s'\n", parseList[moveNum - 1]);
    setbuf(debugFP, NULL);
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
		    bookHit = SendMoveToBookUser(moveNum - 1, &first, FALSE); // [HGM] book
		    if (firstMove && !bookHit) {
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
		if(gameInfo.variant == currentlyInitializedVariant) // [HGM] refrain sending moves engine can't understand!
		SendMoveToProgram(moveNum - 1, &first);
	      }
	    }
	}
#endif
    }

    if (moveNum > 0 && !gotPremove && !appData.noGUI) {
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
	gameInfo.variant != VariantCrazyhouse && !appData.noGUI) {
	if (tinyLayout || smallLayout) {
	    if(gameInfo.variant == VariantNormal)
		sprintf(str, "%s(%d) %s(%d) {%d %d}", 
		    gameInfo.white, white_stren, gameInfo.black, black_stren,
		    basetime, increment);
	    else
		sprintf(str, "%s(%d) %s(%d) {%d %d w%d}", 
		    gameInfo.white, white_stren, gameInfo.black, black_stren,
		    basetime, increment, (int) gameInfo.variant);
	} else {
	    if(gameInfo.variant == VariantNormal)
		sprintf(str, "%s (%d) vs. %s (%d) {%d %d}", 
		    gameInfo.white, white_stren, gameInfo.black, black_stren,
		    basetime, increment);
	    else
		sprintf(str, "%s (%d) vs. %s (%d) {%d %d %s}", 
		    gameInfo.white, white_stren, gameInfo.black, black_stren,
		    basetime, increment, VariantName(gameInfo.variant));
	}
	DisplayTitle(str);
  if (appData.debugMode) {
    fprintf(debugFP, "Display title '%s, gameInfo.variant = %d'\n", str, gameInfo.variant);
  }
    }

   
    /* Display the board */
    if (!pausing && !appData.noGUI) {
      
      if (appData.premove)
	  if (!gotPremove || 
	     ((gameMode == IcsPlayingWhite) && (WhiteOnMove(currentMove))) ||
	     ((gameMode == IcsPlayingBlack) && (!WhiteOnMove(currentMove))))
	      ClearPremoveHighlights();

      DrawPosition(FALSE, boards[currentMove]);
      DisplayMove(moveNum - 1);
      if (appData.ringBellAfterMoves && /*!ics_user_moved*/ // [HGM] use absolute method to recognize own move
	    !((gameMode == IcsPlayingWhite) && (!WhiteOnMove(moveNum)) ||
	      (gameMode == IcsPlayingBlack) &&  (WhiteOnMove(moveNum))   ) ) {
	if(newMove) RingBell(); else PlayIcsUnfinishedSound();
      }
    }

    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);
#if ZIPPY
    if(bookHit) { // [HGM] book: simulate book reply
	static char bookMove[MSG_SIZ]; // a bit generous?

	programStats.nodes = programStats.depth = programStats.time = 
	programStats.score = programStats.got_only_move = 0;
	sprintf(programStats.movelist, "%s (xbook)", bookHit);

	strcpy(bookMove, "move ");
	strcat(bookMove, bookHit);
	HandleMachineMove(bookMove, &first);
    }
#endif
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
      if(cps->alphaRank) { /* [HGM] shogi: temporarily convert to shogi coordinates before sending */
	AlphaRank(moveList[moveNum], 4);
	SendToProgram(moveList[moveNum], cps);
	AlphaRank(moveList[moveNum], 4); // and back
      } else
      /* Added by Tord: Send castle moves in "O-O" in FRC games if required by
       * the engine. It would be nice to have a better way to identify castle 
       * moves here. */
      if((gameInfo.variant == VariantFischeRandom || gameInfo.variant == VariantCapaRandom)
									 && cps->useOOCastle) {
        int fromX = moveList[moveNum][0] - AAA; 
        int fromY = moveList[moveNum][1] - ONE;
        int toX = moveList[moveNum][2] - AAA; 
        int toY = moveList[moveNum][3] - ONE;
        if((boards[moveNum][fromY][fromX] == WhiteKing 
            && boards[moveNum][toY][toX] == WhiteRook)
           || (boards[moveNum][fromY][fromX] == BlackKing 
               && boards[moveNum][toY][toX] == BlackRook)) {
	  if(toX > fromX) SendToProgram("O-O\n", cps);
	  else SendToProgram("O-O-O\n", cps);
	}
	else SendToProgram(moveList[moveNum], cps);
      }
      else SendToProgram(moveList[moveNum], cps);
      /* End of additions by Tord */
    }

    /* [HGM] setting up the opening has brought engine in force mode! */
    /*       Send 'go' if we are in a mode where machine should play. */
    if( (moveNum == 0 && setboardSpoiledMachineBlack && cps == &first) &&
        (gameMode == TwoMachinesPlay   ||
#ifdef ZIPPY
         gameMode == IcsPlayingBlack     || gameMode == IcsPlayingWhite ||
#endif
         gameMode == MachinePlaysBlack || gameMode == MachinePlaysWhite) ) {
        SendToProgram("go\n", cps);
  if (appData.debugMode) {
    fprintf(debugFP, "(extra)\n");
  }
    }
    setboardSpoiledMachineBlack = 0;
}

void
SendMoveToICS(moveType, fromX, fromY, toX, toY)
     ChessMove moveType;
     int fromX, fromY, toX, toY;
{
    char user_move[MSG_SIZ];

    switch (moveType) {
      default:
	sprintf(user_move, _("say Internal error; bad moveType %d (%d,%d-%d,%d)"),
		(int)moveType, fromX, fromY, toX, toY);
	DisplayError(user_move + strlen("say "), 0);
	break;
      case WhiteKingSideCastle:
      case BlackKingSideCastle:
      case WhiteQueenSideCastleWild:
      case BlackQueenSideCastleWild:
      /* PUSH Fabien */
      case WhiteHSideCastleFR:
      case BlackHSideCastleFR:
      /* POP Fabien */
	sprintf(user_move, "o-o\n");
	break;
      case WhiteQueenSideCastle:
      case BlackQueenSideCastle:
      case WhiteKingSideCastleWild:
      case BlackKingSideCastleWild:
      /* PUSH Fabien */
      case WhiteASideCastleFR:
      case BlackASideCastleFR:
      /* POP Fabien */
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
      case WhitePromotionChancellor:
      case BlackPromotionChancellor:
      case WhitePromotionArchbishop:
      case BlackPromotionArchbishop:
        if(gameInfo.variant == VariantShatranj || gameInfo.variant == VariantCourier)
            sprintf(user_move, "%c%c%c%c=%c\n",
                AAA + fromX, ONE + fromY, AAA + toX, ONE + toY,
		PieceToChar(WhiteFerz));
        else if(gameInfo.variant == VariantGreat)
            sprintf(user_move, "%c%c%c%c=%c\n",
                AAA + fromX, ONE + fromY, AAA + toX, ONE + toY,
		PieceToChar(WhiteMan));
        else
            sprintf(user_move, "%c%c%c%c=%c\n",
                AAA + fromX, ONE + fromY, AAA + toX, ONE + toY,
		PieceToChar(PromoPiece(moveType)));
	break;
      case WhiteDrop:
      case BlackDrop:
	sprintf(user_move, "%c@%c%c\n",
		ToUpper(PieceToChar((ChessSquare) fromX)),
                AAA + toX, ONE + toY);
	break;
      case NormalMove:
      case WhiteCapturesEnPassant:
      case BlackCapturesEnPassant:
      case IllegalMove:  /* could be a variant we don't quite understand */
	sprintf(user_move, "%c%c%c%c\n",
                AAA + fromX, ONE + fromY, AAA + toX, ONE + toY);
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
                ToUpper(PieceToChar((ChessSquare) ff)), AAA + ft, ONE + rt);
    } else {
	if (promoChar == 'x' || promoChar == NULLCHAR) {
	    sprintf(move, "%c%c%c%c\n",
                    AAA + ff, ONE + rf, AAA + ft, ONE + rt);
	} else {
	    sprintf(move, "%c%c%c%c%c\n",
                    AAA + ff, ONE + rf, AAA + ft, ONE + rt, promoChar);
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


/* [HGM] Shogi move preprocessor: swap digits for letters, vice versa */
void
AlphaRank(char *move, int n)
{
//    char *p = move, c; int x, y;

    if (appData.debugMode) {
        fprintf(debugFP, "alphaRank(%s,%d)\n", move, n);
    }

    if(move[1]=='*' && 
       move[2]>='0' && move[2]<='9' &&
       move[3]>='a' && move[3]<='x'    ) {
        move[1] = '@';
        move[2] = BOARD_RGHT  -1 - (move[2]-'1') + AAA;
        move[3] = BOARD_HEIGHT-1 - (move[3]-'a') + ONE;
    } else
    if(move[0]>='0' && move[0]<='9' &&
       move[1]>='a' && move[1]<='x' &&
       move[2]>='0' && move[2]<='9' &&
       move[3]>='a' && move[3]<='x'    ) {
        /* input move, Shogi -> normal */
        move[0] = BOARD_RGHT  -1 - (move[0]-'1') + AAA;
        move[1] = BOARD_HEIGHT-1 - (move[1]-'a') + ONE;
        move[2] = BOARD_RGHT  -1 - (move[2]-'1') + AAA;
        move[3] = BOARD_HEIGHT-1 - (move[3]-'a') + ONE;
    } else
    if(move[1]=='@' &&
       move[3]>='0' && move[3]<='9' &&
       move[2]>='a' && move[2]<='x'    ) {
        move[1] = '*';
        move[2] = BOARD_RGHT - 1 - (move[2]-AAA) + '1';
        move[3] = BOARD_HEIGHT-1 - (move[3]-ONE) + 'a';
    } else
    if(
       move[0]>='a' && move[0]<='x' &&
       move[3]>='0' && move[3]<='9' &&
       move[2]>='a' && move[2]<='x'    ) {
         /* output move, normal -> Shogi */
        move[0] = BOARD_RGHT - 1 - (move[0]-AAA) + '1';
        move[1] = BOARD_HEIGHT-1 - (move[1]-ONE) + 'a';
        move[2] = BOARD_RGHT - 1 - (move[2]-AAA) + '1';
        move[3] = BOARD_HEIGHT-1 - (move[3]-ONE) + 'a';
        if(move[4] == PieceToChar(BlackQueen)) move[4] = '+';
    }
    if (appData.debugMode) {
        fprintf(debugFP, "   out = '%s'\n", move);
    }
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
    if (appData.debugMode) {
        fprintf(debugFP, "move to parse: %s\n", move);
    }
    *moveType = yylexstr(moveNum, move);

    switch (*moveType) {
      case WhitePromotionChancellor:
      case BlackPromotionChancellor:
      case WhitePromotionArchbishop:
      case BlackPromotionArchbishop:
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
      /* Code added by Tord: */
      case WhiteHSideCastleFR:
      case WhiteASideCastleFR:
      case BlackHSideCastleFR:
      case BlackASideCastleFR:
      /* End of code added by Tord */
      case IllegalMove:		/* bug or odd chess variant */
        *fromX = currentMoveString[0] - AAA;
        *fromY = currentMoveString[1] - ONE;
        *toX = currentMoveString[2] - AAA;
        *toY = currentMoveString[3] - ONE;
	*promoChar = currentMoveString[4];
        if (*fromX < BOARD_LEFT || *fromX >= BOARD_RGHT || *fromY < 0 || *fromY >= BOARD_HEIGHT ||
            *toX < BOARD_LEFT || *toX >= BOARD_RGHT || *toY < 0 || *toY >= BOARD_HEIGHT) {
    if (appData.debugMode) {
        fprintf(debugFP, "Off-board move (%d,%d)-(%d,%d)%c, type = %d\n", *fromX, *fromY, *toX, *toY, *promoChar, *moveType);
    }
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
        *toX = currentMoveString[2] - AAA;
        *toY = currentMoveString[3] - ONE;
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
    if (appData.debugMode) {
        fprintf(debugFP, "Impossible move %s, type = %d\n", currentMoveString, *moveType);
    }
	/* bug? */
	*fromX = *fromY = *toX = *toY = 0;
	*promoChar = NULLCHAR;
	return FALSE;
    }
}

// [HGM] shuffle: a general way to suffle opening setups, applicable to arbitrary variants.
// All positions will have equal probability, but the current method will not provide a unique
// numbering scheme for arrays that contain 3 or more pieces of the same kind.
#define DARK 1
#define LITE 2
#define ANY 3

int squaresLeft[4];
int piecesLeft[(int)BlackPawn];
int seed, nrOfShuffles;

void GetPositionNumber()
{	// sets global variable seed
	int i;

	seed = appData.defaultFrcPosition;
	if(seed < 0) { // randomize based on time for negative FRC position numbers
		for(i=0; i<50; i++) seed += random();
		seed = random() ^ random() >> 8 ^ random() << 8;
		if(seed<0) seed = -seed;
	}
}

int put(Board board, int pieceType, int rank, int n, int shade)
// put the piece on the (n-1)-th empty squares of the given shade
{
	int i;

	for(i=BOARD_LEFT; i<BOARD_RGHT; i++) {
		if( (((i-BOARD_LEFT)&1)+1) & shade && board[rank][i] == EmptySquare && n-- == 0) {
			board[rank][i] = (ChessSquare) pieceType;
			squaresLeft[((i-BOARD_LEFT)&1) + 1]--;
			squaresLeft[ANY]--;
			piecesLeft[pieceType]--; 
			return i;
		}
	}
        return -1;
}


void AddOnePiece(Board board, int pieceType, int rank, int shade)
// calculate where the next piece goes, (any empty square), and put it there
{
	int i;

        i = seed % squaresLeft[shade];
	nrOfShuffles *= squaresLeft[shade];
	seed /= squaresLeft[shade];
        put(board, pieceType, rank, i, shade);
}

void AddTwoPieces(Board board, int pieceType, int rank)
// calculate where the next 2 identical pieces go, (any empty square), and put it there
{
	int i, n=squaresLeft[ANY], j=n-1, k;

	k = n*(n-1)/2; // nr of possibilities, not counting permutations
        i = seed % k;  // pick one
	nrOfShuffles *= k;
	seed /= k;
	while(i >= j) i -= j--;
        j = n - 1 - j; i += j;
        put(board, pieceType, rank, j, ANY);
        put(board, pieceType, rank, i, ANY);
}

void SetUpShuffle(Board board, int number)
{
	int i, p, first=1;

	GetPositionNumber(); nrOfShuffles = 1;

	squaresLeft[DARK] = (BOARD_RGHT - BOARD_LEFT + 1)/2;
	squaresLeft[ANY]  = BOARD_RGHT - BOARD_LEFT;
	squaresLeft[LITE] = squaresLeft[ANY] - squaresLeft[DARK];

	for(p = 0; p<=(int)WhiteKing; p++) piecesLeft[p] = 0;

	for(i=BOARD_LEFT; i<BOARD_RGHT; i++) { // count pieces and clear board
	    p = (int) board[0][i];
	    if(p < (int) BlackPawn) piecesLeft[p] ++;
	    board[0][i] = EmptySquare;
	}

	if(PosFlags(0) & F_ALL_CASTLE_OK) {
	    // shuffles restricted to allow normal castling put KRR first
	    if(piecesLeft[(int)WhiteKing]) // King goes rightish of middle
		put(board, WhiteKing, 0, (gameInfo.boardWidth+1)/2, ANY);
	    else if(piecesLeft[(int)WhiteUnicorn]) // in Knightmate Unicorn castles
		put(board, WhiteUnicorn, 0, (gameInfo.boardWidth+1)/2, ANY);
	    if(piecesLeft[(int)WhiteRook]) // First supply a Rook for K-side castling
		put(board, WhiteRook, 0, gameInfo.boardWidth-2, ANY);
	    if(piecesLeft[(int)WhiteRook]) // Then supply a Rook for Q-side castling
		put(board, WhiteRook, 0, 0, ANY);
	    // in variants with super-numerary Kings and Rooks, we leave these for the shuffle
	}

	if(((BOARD_RGHT-BOARD_LEFT) & 1) == 0)
	    // only for even boards make effort to put pairs of colorbound pieces on opposite colors
	    for(p = (int) WhiteKing; p > (int) WhitePawn; p--) {
		if(p != (int) WhiteBishop && p != (int) WhiteFerz && p != (int) WhiteAlfil) continue;
		while(piecesLeft[p] >= 2) {
		    AddOnePiece(board, p, 0, LITE);
		    AddOnePiece(board, p, 0, DARK);
		}
		// Odd color-bound pieces are shuffled with the rest (to not run out of paired squares)
	    }

	for(p = (int) WhiteKing - 2; p > (int) WhitePawn; p--) {
	    // Remaining pieces (non-colorbound, or odd color bound) can be put anywhere
	    // but we leave King and Rooks for last, to possibly obey FRC restriction
	    if(p == (int)WhiteRook) continue;
	    while(piecesLeft[p] >= 2) AddTwoPieces(board, p, 0); // add in pairs, for not counting permutations
	    if(piecesLeft[p]) AddOnePiece(board, p, 0, ANY);     // add the odd piece
	}

	// now everything is placed, except perhaps King (Unicorn) and Rooks

	if(PosFlags(0) & F_FRC_TYPE_CASTLING) {
	    // Last King gets castling rights
	    while(piecesLeft[(int)WhiteUnicorn]) {
		i = put(board, WhiteUnicorn, 0, piecesLeft[(int)WhiteRook]/2, ANY);
		initialRights[2]  = initialRights[5]  = castlingRights[0][2] = castlingRights[0][5] = i;
	    }

	    while(piecesLeft[(int)WhiteKing]) {
		i = put(board, WhiteKing, 0, piecesLeft[(int)WhiteRook]/2, ANY);
		initialRights[2]  = initialRights[5]  = castlingRights[0][2] = castlingRights[0][5] = i;
	    }


	} else {
	    while(piecesLeft[(int)WhiteKing])    AddOnePiece(board, WhiteKing, 0, ANY);
	    while(piecesLeft[(int)WhiteUnicorn]) AddOnePiece(board, WhiteUnicorn, 0, ANY);
	}

	// Only Rooks can be left; simply place them all
	while(piecesLeft[(int)WhiteRook]) {
		i = put(board, WhiteRook, 0, 0, ANY);
		if(PosFlags(0) & F_FRC_TYPE_CASTLING) { // first and last Rook get FRC castling rights
			if(first) {
				first=0;
				initialRights[1]  = initialRights[4]  = castlingRights[0][1] = castlingRights[0][4] = i;
			}
			initialRights[0]  = initialRights[3]  = castlingRights[0][0] = castlingRights[0][3] = i;
		}
	}
	for(i=BOARD_LEFT; i<BOARD_RGHT; i++) { // copy black from white
	    board[BOARD_HEIGHT-1][i] =  (int) board[0][i] < BlackPawn ? WHITE_TO_BLACK board[0][i] : EmptySquare;
	}

	if(number >= 0) appData.defaultFrcPosition %= nrOfShuffles; // normalize
}

int SetCharTable( char *table, const char * map )
/* [HGM] moved here from winboard.c because of its general usefulness */
/*       Basically a safe strcpy that uses the last character as King */
{
    int result = FALSE; int NrPieces;

    if( map != NULL && (NrPieces=strlen(map)) <= (int) EmptySquare 
                    && NrPieces >= 12 && !(NrPieces&1)) {
        int i; /* [HGM] Accept even length from 12 to 34 */

        for( i=0; i<(int) EmptySquare; i++ ) table[i] = '.';
        for( i=0; i<NrPieces/2-1; i++ ) {
            table[i] = map[i];
            table[i + (int)BlackPawn - (int) WhitePawn] = map[i+NrPieces/2];
        }
        table[(int) WhiteKing]  = map[NrPieces/2-1];
        table[(int) BlackKing]  = map[NrPieces-1];

        result = TRUE;
    }

    return result;
}

void Prelude(Board board)
{	// [HGM] superchess: random selection of exo-pieces
	int i, j, k; ChessSquare p; 
	static ChessSquare exoPieces[4] = { WhiteAngel, WhiteMarshall, WhiteSilver, WhiteLance };

	GetPositionNumber(); // use FRC position number

	if(appData.pieceToCharTable != NULL) { // select pieces to participate from given char table
	    SetCharTable(pieceToChar, appData.pieceToCharTable);
	    for(i=(int)WhiteQueen+1, j=0; i<(int)WhiteKing && j<4; i++) 
		if(PieceToChar((ChessSquare)i) != '.') exoPieces[j++] = (ChessSquare) i;
	}

	j = seed%4;		    seed /= 4; 
	p = board[0][BOARD_LEFT+j];   board[0][BOARD_LEFT+j] = EmptySquare; k = PieceToNumber(p);
	board[k][BOARD_WIDTH-1] = p;  board[k][BOARD_WIDTH-2]++;
	board[BOARD_HEIGHT-1-k][0] = WHITE_TO_BLACK p;  board[BOARD_HEIGHT-1-k][1]++;
	j = seed%3 + (seed%3 >= j); seed /= 3; 
	p = board[0][BOARD_LEFT+j];   board[0][BOARD_LEFT+j] = EmptySquare; k = PieceToNumber(p);
	board[k][BOARD_WIDTH-1] = p;  board[k][BOARD_WIDTH-2]++;
	board[BOARD_HEIGHT-1-k][0] = WHITE_TO_BLACK p;  board[BOARD_HEIGHT-1-k][1]++;
	j = seed%3;		    seed /= 3; 
	p = board[0][BOARD_LEFT+j+5]; board[0][BOARD_LEFT+j+5] = EmptySquare; k = PieceToNumber(p);
	board[k][BOARD_WIDTH-1] = p;  board[k][BOARD_WIDTH-2]++;
	board[BOARD_HEIGHT-1-k][0] = WHITE_TO_BLACK p;  board[BOARD_HEIGHT-1-k][1]++;
	j = seed%2 + (seed%2 >= j); seed /= 2; 
	p = board[0][BOARD_LEFT+j+5]; board[0][BOARD_LEFT+j+5] = EmptySquare; k = PieceToNumber(p);
	board[k][BOARD_WIDTH-1] = p;  board[k][BOARD_WIDTH-2]++;
	board[BOARD_HEIGHT-1-k][0] = WHITE_TO_BLACK p;  board[BOARD_HEIGHT-1-k][1]++;
	j = seed%4; seed /= 4; put(board, exoPieces[3],    0, j, ANY);
	j = seed%3; seed /= 3; put(board, exoPieces[2],   0, j, ANY);
	j = seed%2; seed /= 2; put(board, exoPieces[1], 0, j, ANY);
	put(board, exoPieces[0],    0, 0, ANY);
	for(i=BOARD_LEFT; i<BOARD_RGHT; i++) board[BOARD_HEIGHT-1][i] = WHITE_TO_BLACK board[0][i];
}

void
InitPosition(redraw)
     int redraw;
{
    ChessSquare (* pieces)[BOARD_SIZE];
    int i, j, pawnRow, overrule,
    oldx = gameInfo.boardWidth,
    oldy = gameInfo.boardHeight,
    oldh = gameInfo.holdingsWidth,
    oldv = gameInfo.variant;

    currentMove = forwardMostMove = backwardMostMove = 0;
    if(appData.icsActive) shuffleOpenings = FALSE; // [HGM] shuffle: in ICS mode, only shuffle on ICS request

    /* [AS] Initialize pv info list [HGM] and game status */
    {
        for( i=0; i<MAX_MOVES; i++ ) {
            pvInfoList[i].depth = 0;
            epStatus[i]=EP_NONE;
            for( j=0; j<BOARD_SIZE; j++ ) castlingRights[i][j] = -1;
        }

        initialRulePlies = 0; /* 50-move counter start */

        castlingRank[0] = castlingRank[1] = castlingRank[2] = 0;
        castlingRank[3] = castlingRank[4] = castlingRank[5] = BOARD_HEIGHT-1;
    }

    
    /* [HGM] logic here is completely changed. In stead of full positions */
    /* the initialized data only consist of the two backranks. The switch */
    /* selects which one we will use, which is than copied to the Board   */
    /* initialPosition, which for the rest is initialized by Pawns and    */
    /* empty squares. This initial position is then copied to boards[0],  */
    /* possibly after shuffling, so that it remains available.            */

    gameInfo.holdingsWidth = 0; /* default board sizes */
    gameInfo.boardWidth    = 8;
    gameInfo.boardHeight   = 8;
    gameInfo.holdingsSize  = 0;
    nrCastlingRights = -1; /* [HGM] Kludge to indicate default should be used */
    for(i=0; i<BOARD_SIZE; i++) initialRights[i] = -1; /* but no rights yet */
    SetCharTable(pieceToChar, "PNBRQ...........Kpnbrq...........k"); 

    switch (gameInfo.variant) {
    case VariantFischeRandom:
      shuffleOpenings = TRUE;
    default:
      pieces = FIDEArray;
      break;
    case VariantShatranj:
      pieces = ShatranjArray;
      nrCastlingRights = 0;
      SetCharTable(pieceToChar, "PN.R.QB...Kpn.r.qb...k"); 
      break;
    case VariantTwoKings:
      pieces = twoKingsArray;
      break;
    case VariantCapaRandom:
      shuffleOpenings = TRUE;
    case VariantCapablanca:
      pieces = CapablancaArray;
      gameInfo.boardWidth = 10;
      SetCharTable(pieceToChar, "PNBRQ..ACKpnbrq..ack"); 
      break;
    case VariantGothic:
      pieces = GothicArray;
      gameInfo.boardWidth = 10;
      SetCharTable(pieceToChar, "PNBRQ..ACKpnbrq..ack"); 
      break;
    case VariantJanus:
      pieces = JanusArray;
      gameInfo.boardWidth = 10;
      SetCharTable(pieceToChar, "PNBRQ..JKpnbrq..jk"); 
      nrCastlingRights = 6;
        castlingRights[0][0] = initialRights[0] = BOARD_RGHT-1;
        castlingRights[0][1] = initialRights[1] = BOARD_LEFT;
        castlingRights[0][2] = initialRights[2] =(BOARD_WIDTH-1)>>1;
        castlingRights[0][3] = initialRights[3] = BOARD_RGHT-1;
        castlingRights[0][4] = initialRights[4] = BOARD_LEFT;
        castlingRights[0][5] = initialRights[5] =(BOARD_WIDTH-1)>>1;
      break;
    case VariantFalcon:
      pieces = FalconArray;
      gameInfo.boardWidth = 10;
      SetCharTable(pieceToChar, "PNBRQ.............FKpnbrq.............fk"); 
      break;
    case VariantXiangqi:
      pieces = XiangqiArray;
      gameInfo.boardWidth  = 9;
      gameInfo.boardHeight = 10;
      nrCastlingRights = 0;
      SetCharTable(pieceToChar, "PH.R.AE..K.C.ph.r.ae..k.c."); 
      break;
    case VariantShogi:
      pieces = ShogiArray;
      gameInfo.boardWidth  = 9;
      gameInfo.boardHeight = 9;
      gameInfo.holdingsSize = 7;
      nrCastlingRights = 0;
      SetCharTable(pieceToChar, "PNBRLS...G.++++++Kpnbrls...g.++++++k"); 
      break;
    case VariantCourier:
      pieces = CourierArray;
      gameInfo.boardWidth  = 12;
      nrCastlingRights = 0;
      SetCharTable(pieceToChar, "PNBR.FE..WMKpnbr.fe..wmk"); 
      for(i=0; i<BOARD_SIZE; i++) initialRights[i] = -1;
      break;
    case VariantKnightmate:
      pieces = KnightmateArray;
      SetCharTable(pieceToChar, "P.BRQ.....M.........K.p.brq.....m.........k."); 
      break;
    case VariantFairy:
      pieces = fairyArray;
      SetCharTable(pieceToChar, "PNBRQFEACWMOHIJGDVSLUKpnbrqfeacwmohijgdvsluk"); 
      break;
    case VariantGreat:
      pieces = GreatArray;
      gameInfo.boardWidth = 10;
      SetCharTable(pieceToChar, "PN....E...S..HWGMKpn....e...s..hwgmk");
      gameInfo.holdingsSize = 8;
      break;
    case VariantSuper:
      pieces = FIDEArray;
      SetCharTable(pieceToChar, "PNBRQ..SE.......V.AKpnbrq..se.......v.ak");
      gameInfo.holdingsSize = 8;
      startedFromSetupPosition = TRUE;
      break;
    case VariantCrazyhouse:
    case VariantBughouse:
      pieces = FIDEArray;
      SetCharTable(pieceToChar, "PNBRQ.......~~~~Kpnbrq.......~~~~k"); 
      gameInfo.holdingsSize = 5;
      break;
    case VariantWildCastle:
      pieces = FIDEArray;
      /* !!?shuffle with kings guaranteed to be on d or e file */
      shuffleOpenings = 1;
      break;
    case VariantNoCastle:
      pieces = FIDEArray;
      nrCastlingRights = 0;
      for(i=0; i<BOARD_SIZE; i++) initialRights[i] = -1;
      /* !!?unconstrained back-rank shuffle */
      shuffleOpenings = 1;
      break;
    }

    overrule = 0;
    if(appData.NrFiles >= 0) {
        if(gameInfo.boardWidth != appData.NrFiles) overrule++;
        gameInfo.boardWidth = appData.NrFiles;
    }
    if(appData.NrRanks >= 0) {
        gameInfo.boardHeight = appData.NrRanks;
    }
    if(appData.holdingsSize >= 0) {
        i = appData.holdingsSize;
        if(i > gameInfo.boardHeight) i = gameInfo.boardHeight;
        gameInfo.holdingsSize = i;
    }
    if(gameInfo.holdingsSize) gameInfo.holdingsWidth = 2;
    if(BOARD_HEIGHT > BOARD_SIZE || BOARD_WIDTH > BOARD_SIZE)
        DisplayFatalError(_("Recompile to support this BOARD_SIZE!"), 0, 2);

    pawnRow = gameInfo.boardHeight - 7; /* seems to work in all common variants */
    if(pawnRow < 1) pawnRow = 1;

    /* User pieceToChar list overrules defaults */
    if(appData.pieceToCharTable != NULL)
        SetCharTable(pieceToChar, appData.pieceToCharTable);

    for( j=0; j<BOARD_WIDTH; j++ ) { ChessSquare s = EmptySquare;

        if(j==BOARD_LEFT-1 || j==BOARD_RGHT)
            s = (ChessSquare) 0; /* account holding counts in guard band */
        for( i=0; i<BOARD_HEIGHT; i++ )
            initialPosition[i][j] = s;

        if(j < BOARD_LEFT || j >= BOARD_RGHT || overrule) continue;
        initialPosition[0][j] = pieces[0][j-gameInfo.holdingsWidth];
        initialPosition[pawnRow][j] = WhitePawn;
        initialPosition[BOARD_HEIGHT-pawnRow-1][j] = BlackPawn;
        if(gameInfo.variant == VariantXiangqi) {
            if(j&1) {
                initialPosition[pawnRow][j] = 
                initialPosition[BOARD_HEIGHT-pawnRow-1][j] = EmptySquare;
                if(j==BOARD_LEFT+1 || j>=BOARD_RGHT-2) {
                   initialPosition[2][j] = WhiteCannon;
                   initialPosition[BOARD_HEIGHT-3][j] = BlackCannon;
                }
            }
        }
        initialPosition[BOARD_HEIGHT-1][j] =  pieces[1][j-gameInfo.holdingsWidth];
    }
    if( (gameInfo.variant == VariantShogi) && !overrule ) {

            j=BOARD_LEFT+1;
            initialPosition[1][j] = WhiteBishop;
            initialPosition[BOARD_HEIGHT-2][j] = BlackRook;
            j=BOARD_RGHT-2;
            initialPosition[1][j] = WhiteRook;
            initialPosition[BOARD_HEIGHT-2][j] = BlackBishop;
    }

    if( nrCastlingRights == -1) {
        /* [HGM] Build normal castling rights (must be done after board sizing!) */
        /*       This sets default castling rights from none to normal corners   */
        /* Variants with other castling rights must set them themselves above    */
        nrCastlingRights = 6;
       
        castlingRights[0][0] = initialRights[0] = BOARD_RGHT-1;
        castlingRights[0][1] = initialRights[1] = BOARD_LEFT;
        castlingRights[0][2] = initialRights[2] = BOARD_WIDTH>>1;
        castlingRights[0][3] = initialRights[3] = BOARD_RGHT-1;
        castlingRights[0][4] = initialRights[4] = BOARD_LEFT;
        castlingRights[0][5] = initialRights[5] = BOARD_WIDTH>>1;
     }

     if(gameInfo.variant == VariantSuper) Prelude(initialPosition);
     if(gameInfo.variant == VariantGreat) { // promotion commoners
	initialPosition[PieceToNumber(WhiteMan)][BOARD_WIDTH-1] = WhiteMan;
	initialPosition[PieceToNumber(WhiteMan)][BOARD_WIDTH-2] = 9;
	initialPosition[BOARD_HEIGHT-1-PieceToNumber(WhiteMan)][0] = BlackMan;
	initialPosition[BOARD_HEIGHT-1-PieceToNumber(WhiteMan)][1] = 9;
     }
#if 0
    if(gameInfo.variant == VariantFischeRandom) {
      if( appData.defaultFrcPosition < 0 ) {
        ShuffleFRC( initialPosition );
      }
      else {
        SetupFRC( initialPosition, appData.defaultFrcPosition );
      }
      startedFromSetupPosition = TRUE;
    } else 
#else
  if (appData.debugMode) {
    fprintf(debugFP, "shuffleOpenings = %d\n", shuffleOpenings);
  }
    if(shuffleOpenings) {
	SetUpShuffle(initialPosition, appData.defaultFrcPosition);
	startedFromSetupPosition = TRUE;
    }
#endif
    if(startedFromPositionFile) {
      /* [HGM] loadPos: use PositionFile for every new game */
      CopyBoard(initialPosition, filePosition);
      for(i=0; i<nrCastlingRights; i++)
          castlingRights[0][i] = initialRights[i] = fileRights[i];
      startedFromSetupPosition = TRUE;
    }

    CopyBoard(boards[0], initialPosition);

    if(oldx != gameInfo.boardWidth ||
       oldy != gameInfo.boardHeight ||
       oldh != gameInfo.holdingsWidth
#ifdef GOTHIC
       || oldv == VariantGothic ||        // For licensing popups
       gameInfo.variant == VariantGothic
#endif
#ifdef FALCON
       || oldv == VariantFalcon ||
       gameInfo.variant == VariantFalcon
#endif
                                         )
            InitDrawingSizes(-2 ,0);

    if (redraw)
      DrawPosition(TRUE, boards[currentMove]);
}

void
SendBoard(cps, moveNum)
     ChessProgramState *cps;
     int moveNum;
{
    char message[MSG_SIZ];
    
    if (cps->useSetboard) {
      char* fen = PositionToFEN(moveNum, cps->fenOverride);
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
      for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
	bp = &boards[moveNum][i][BOARD_LEFT];
        for (j = BOARD_LEFT; j < BOARD_RGHT; j++, bp++) {
	  if ((int) *bp < (int) BlackPawn) {
	    sprintf(message, "%c%c%c\n", PieceToChar(*bp), 
                    AAA + j, ONE + i);
            if(message[0] == '+' || message[0] == '~') {
                sprintf(message, "%c%c%c+\n",
                        PieceToChar((ChessSquare)(DEMOTED *bp)),
                        AAA + j, ONE + i);
            }
            if(cps->alphaRank) { /* [HGM] shogi: translate coords */
                message[1] = BOARD_RGHT   - 1 - j + '1';
                message[2] = BOARD_HEIGHT - 1 - i + 'a';
            }
	    SendToProgram(message, cps);
	  }
	}
      }
    
      SendToProgram("c\n", cps);
      for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
	bp = &boards[moveNum][i][BOARD_LEFT];
        for (j = BOARD_LEFT; j < BOARD_RGHT; j++, bp++) {
	  if (((int) *bp != (int) EmptySquare)
	      && ((int) *bp >= (int) BlackPawn)) {
	    sprintf(message, "%c%c%c\n", ToUpper(PieceToChar(*bp)),
                    AAA + j, ONE + i);
            if(message[0] == '+' || message[0] == '~') {
                sprintf(message, "%c%c%c+\n",
                        PieceToChar((ChessSquare)(DEMOTED *bp)),
                        AAA + j, ONE + i);
            }
            if(cps->alphaRank) { /* [HGM] shogi: translate coords */
                message[1] = BOARD_RGHT   - 1 - j + '1';
                message[2] = BOARD_HEIGHT - 1 - i + 'a';
            }
	    SendToProgram(message, cps);
	  }
	}
      }
    
      SendToProgram(".\n", cps);
    }
    setboardSpoiledMachineBlack = 0; /* [HGM] assume WB 4.2.7 already solves this after sending setboard */
}

int
IsPromotion(fromX, fromY, toX, toY)
     int fromX, fromY, toX, toY;
{
    /* [HGM] add Shogi promotions */
    int promotionZoneSize=1, highestPromotingPiece = (int)WhitePawn;
    ChessSquare piece;

    if(gameMode == EditPosition || gameInfo.variant == VariantXiangqi ||
      !(fromX >=0 && fromY >= 0 && toX >= 0 && toY >= 0) ) return FALSE;
   /* [HGM] Note to self: line above also weeds out drops */
    piece = boards[currentMove][fromY][fromX];
    if(gameInfo.variant == VariantShogi) {
        promotionZoneSize = 3;
        highestPromotingPiece = (int)WhiteKing;
        /* [HGM] Should be Silver = Ferz, really, but legality testing is off,
           and if in normal chess we then allow promotion to King, why not
           allow promotion of other piece in Shogi?                         */
    }
    if((int)piece >= BlackPawn) {
        if(toY >= promotionZoneSize && fromY >= promotionZoneSize)
             return FALSE;
        highestPromotingPiece = WHITE_TO_BLACK highestPromotingPiece;
    } else {
        if(  toY < BOARD_HEIGHT - promotionZoneSize &&
           fromY < BOARD_HEIGHT - promotionZoneSize) return FALSE;
    }
    return ( (int)piece <= highestPromotingPiece );
}

int
InPalace(row, column)
     int row, column;
{   /* [HGM] for Xiangqi */
    if( (row < 3 || row > BOARD_HEIGHT-4) &&
         column < (BOARD_WIDTH + 4)/2 &&
         column > (BOARD_WIDTH - 5)/2 ) return TRUE;
    return FALSE;
}

int
PieceForSquare (x, y)
     int x;
     int y;
{
  if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT)
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
      (int)from_piece < (int)BlackPawn; /* [HGM] can be > King! */

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


ChessMove
UserMoveTest(fromX, fromY, toX, toY, promoChar)
     int fromX, fromY, toX, toY;
     int promoChar;
{
    ChessMove moveType;
    ChessSquare pdown, pup;

    if (fromX < 0 || fromY < 0) return ImpossibleMove;
    if ((fromX == toX) && (fromY == toY)) {
        return ImpossibleMove;
    }

    /* [HGM] suppress all moves into holdings area and guard band */
    if( toX < BOARD_LEFT || toX >= BOARD_RGHT || toY < 0 )
            return ImpossibleMove;

    /* [HGM] <sameColor> moved to here from winboard.c */
    /* note: this code seems to exist for filtering out some obviously illegal premoves */
    pdown = boards[currentMove][fromY][fromX];
    pup = boards[currentMove][toY][toX];
    if (    gameMode != EditPosition &&
            (WhitePawn <= pdown && pdown < BlackPawn &&
             WhitePawn <= pup && pup < BlackPawn  ||
             BlackPawn <= pdown && pdown < EmptySquare &&
             BlackPawn <= pup && pup < EmptySquare 
            ) && !((gameInfo.variant == VariantFischeRandom || gameInfo.variant == VariantCapaRandom) &&
                    (pup == WhiteRook && pdown == WhiteKing && fromY == 0 && toY == 0||
                     pup == BlackRook && pdown == BlackKing && fromY == BOARD_HEIGHT-1 && toY == BOARD_HEIGHT-1  ) 
        )           )
         return ImpossibleMove;

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
        return ImpossibleMove;

      case MachinePlaysWhite:
	/* User is moving for Black */
	if (WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is White's turn"));
            return ImpossibleMove;
	}
	break;

      case MachinePlaysBlack:
	/* User is moving for White */
	if (!WhiteOnMove(currentMove)) {
	    DisplayMoveError(_("It is Black's turn"));
            return ImpossibleMove;
	}
	break;

      case EditGame:
      case IcsExamining:
      case BeginningOfGame:
      case AnalyzeMode:
      case Training:
	if ((int) boards[currentMove][fromY][fromX] >= (int) BlackPawn &&
            (int) boards[currentMove][fromY][fromX] < (int) EmptySquare) {
	    /* User is moving for Black */
	    if (WhiteOnMove(currentMove)) {
		DisplayMoveError(_("It is White's turn"));
                return ImpossibleMove;
	    }
	} else {
	    /* User is moving for White */
	    if (!WhiteOnMove(currentMove)) {
		DisplayMoveError(_("It is Black's turn"));
                return ImpossibleMove;
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
            return ImpossibleMove;
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
            return ImpossibleMove;
	}
	break;

      default:
	break;

      case EditPosition:
	/* EditPosition, empty square, or different color piece;
	   click-click move is possible */
	if (toX == -2 || toY == -2) {
	    boards[0][fromY][fromX] = EmptySquare;
	    return AmbiguousMove;
	} else if (toX >= 0 && toY >= 0) {
	    boards[0][toY][toX] = boards[0][fromY][fromX];
	    boards[0][fromY][fromX] = EmptySquare;
	    return AmbiguousMove;
	}
        return ImpossibleMove;
    }

    /* [HGM] If move started in holdings, it means a drop */
    if( fromX == BOARD_LEFT-2 || fromX == BOARD_RGHT+1) { 
         if( pup != EmptySquare ) return ImpossibleMove;
         if(appData.testLegality) {
             /* it would be more logical if LegalityTest() also figured out
              * which drops are legal. For now we forbid pawns on back rank.
              * Shogi is on its own here...
              */
             if( (pdown == WhitePawn || pdown == BlackPawn) &&
                 (toY == 0 || toY == BOARD_HEIGHT -1 ) )
                 return(ImpossibleMove); /* no pawn drops on 1st/8th */
         }
         return WhiteDrop; /* Not needed to specify white or black yet */
    }

    userOfferedDraw = FALSE;
	
    /* [HGM] always test for legality, to get promotion info */
    moveType = LegalityTest(boards[currentMove], PosFlags(currentMove),
                          epStatus[currentMove], castlingRights[currentMove],
                                         fromY, fromX, toY, toX, promoChar);

    /* [HGM] but possibly ignore an IllegalMove result */
    if (appData.testLegality) {
	if (moveType == IllegalMove || moveType == ImpossibleMove) {
	    DisplayMoveError(_("Illegal move"));
            return ImpossibleMove;
	}
    }
if(appData.debugMode) fprintf(debugFP, "moveType 3 = %d, promochar = %x\n", moveType, promoChar);
    return moveType;
    /* [HGM] <popupFix> in stead of calling FinishMove directly, this
       function is made into one that returns an OK move type if FinishMove
       should be called. This to give the calling driver routine the
       opportunity to finish the userMove input with a promotion popup,
       without bothering the user with this for invalid or illegal moves */

/*    FinishMove(moveType, fromX, fromY, toX, toY, promoChar); */
}

/* Common tail of UserMoveEvent and DropMenuEvent */
int
FinishMove(moveType, fromX, fromY, toX, toY, promoChar)
     ChessMove moveType;
     int fromX, fromY, toX, toY;
     /*char*/int promoChar;
{
    char *bookHit = 0;
if(appData.debugMode) fprintf(debugFP, "moveType 5 = %d, promochar = %x\n", moveType, promoChar);
    if((gameInfo.variant == VariantSuper || gameInfo.variant == VariantGreat) && promoChar != NULLCHAR) { 
	// [HGM] superchess: suppress promotions to non-available piece
	int k = PieceToNumber(CharToPiece(ToUpper(promoChar)));
	if(WhiteOnMove(currentMove)) {
	    if(!boards[currentMove][k][BOARD_WIDTH-2]) return 0;
	} else {
	    if(!boards[currentMove][BOARD_HEIGHT-1-k][1]) return 0;
	}
    }

    /* [HGM] <popupFix> kludge to avoid having to know the exact promotion
       move type in caller when we know the move is a legal promotion */
    if(moveType == NormalMove && promoChar)
        moveType = PromoCharToMoveType(WhiteOnMove(currentMove), promoChar);
if(appData.debugMode) fprintf(debugFP, "moveType 1 = %d, promochar = %x\n", moveType, promoChar);
    /* [HGM] convert drag-and-drop piece drops to standard form */
    if( fromX == BOARD_LEFT-2 || fromX == BOARD_RGHT+1) {
         moveType = WhiteOnMove(currentMove) ? WhiteDrop : BlackDrop;
	   if(appData.debugMode) fprintf(debugFP, "Drop move %d, curr=%d, x=%d,y=%d, p=%d\n", 
		moveType, currentMove, fromX, fromY, boards[currentMove][fromY][fromX]);
//         fromX = boards[currentMove][fromY][fromX];
	   // holdings might not be sent yet in ICS play; we have to figure out which piece belongs here
	   if(fromX == 0) fromY = BOARD_HEIGHT-1 - fromY; // black holdings upside-down
	   fromX = fromX ? WhitePawn : BlackPawn; // first piece type in selected holdings
	   while(PieceToChar(fromX) == '.' || PieceToNumber(fromX) != fromY && fromX != (int) EmptySquare) fromX++; 
         fromY = DROP_RANK;
    }

    /* [HGM] <popupFix> The following if has been moved here from
       UserMoveEvent(). Because it seemed to belon here (why not allow
       piece drops in training games?), and because it can only be
       performed after it is known to what we promote. */
    if (gameMode == Training) {
      /* compare the move played on the board to the next move in the
       * game. If they match, display the move and the opponent's response. 
       * If they don't match, display an error message.
       */
      int saveAnimate;
      Board testBoard; char testRights[BOARD_SIZE]; char testStatus;
      CopyBoard(testBoard, boards[currentMove]);
      ApplyMove(fromX, fromY, toX, toY, promoChar, testBoard, testRights, &testStatus);

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
      return 1;
    }

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
      StartClocks();
      SetGameInfo();
      sprintf(buf, "%s vs. %s", gameInfo.white, gameInfo.black);
      DisplayTitle(buf);
      if (first.sendName) {
	sprintf(buf, "name %s\n", gameInfo.white);
	SendToProgram(buf, &first);
      }
      StartClocks();
    }
    ModeHighlight();
  }
if(appData.debugMode) fprintf(debugFP, "moveType 2 = %d, promochar = %x\n", moveType, promoChar);
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
    if (gameMode != EditGame && gameMode != PlayFromGameFile) {
	 // [HGM] book: if program might be playing, let it use book
	bookHit = SendMoveToBookUser(forwardMostMove-1, &first, FALSE);
	first.maybeThinking = TRUE;
    } else SendMoveToProgram(forwardMostMove-1, &first);
    if (currentMove == cmailOldMove + 1) {
      cmailMoveType[lastLoadGameNumber - 1] = CMAIL_MOVE;
    }
  }

  ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/

  switch (gameMode) {
  case EditGame:
    switch (MateTest(boards[currentMove], PosFlags(currentMove),
                     EP_UNKNOWN, castlingRights[currentMove]) ) {
    case MT_NONE:
    case MT_CHECK:
      break;
    case MT_CHECKMATE:
    case MT_STAINMATE:
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

  if(bookHit) { // [HGM] book: simulate book reply
	static char bookMove[MSG_SIZ]; // a bit generous?

	programStats.nodes = programStats.depth = programStats.time = 
	programStats.score = programStats.got_only_move = 0;
	sprintf(programStats.movelist, "%s (xbook)", bookHit);

	strcpy(bookMove, "move ");
	strcat(bookMove, bookHit);
	HandleMachineMove(bookMove, &first);
  }
  return 1;
}

void
UserMoveEvent(fromX, fromY, toX, toY, promoChar)
     int fromX, fromY, toX, toY;
     int promoChar;
{
    /* [HGM] This routine was added to allow calling of its two logical
       parts from other modules in the old way. Before, UserMoveEvent()
       automatically called FinishMove() if the move was OK, and returned
       otherwise. I separated the two, in order to make it possible to
       slip a promotion popup in between. But that it always needs two
       calls, to the first part, (now called UserMoveTest() ), and to
       FinishMove if the first part succeeded. Calls that do not need
       to do anything in between, can call this routine the old way. 
    */
    ChessMove moveType = UserMoveTest(fromX, fromY, toX, toY, promoChar);
if(appData.debugMode) fprintf(debugFP, "moveType 4 = %d, promochar = %x\n", moveType, promoChar);
    if(moveType != ImpossibleMove)
        FinishMove(moveType, fromX, fromY, toX, toY, promoChar);
}

void SendProgramStatsToFrontend( ChessProgramState * cps, ChessProgramStats * cpstats )
{
//    char * hint = lastHint;
    FrontEndProgramStats stats;

    stats.which = cps == &first ? 0 : 1;
    stats.depth = cpstats->depth;
    stats.nodes = cpstats->nodes;
    stats.score = cpstats->score;
    stats.time = cpstats->time;
    stats.pv = cpstats->movelist;
    stats.hint = lastHint;
    stats.an_move_index = 0;
    stats.an_move_count = 0;

    if( gameMode == AnalyzeMode || gameMode == AnalyzeFile ) {
        stats.hint = cpstats->move_name;
        stats.an_move_index = cpstats->nr_moves - cpstats->moves_left;
        stats.an_move_count = cpstats->nr_moves;
    }

    SetProgramStats( &stats );
}

char *SendMoveToBookUser(int moveNr, ChessProgramState *cps, int initial)
{   // [HGM] book: this routine intercepts moves to simulate book replies
    char *bookHit = NULL;

    //first determine if the incoming move brings opponent into his book
    if(appData.usePolyglotBook && (cps == &first ? !appData.firstHasOwnBookUCI : !appData.secondHasOwnBookUCI))
	bookHit = ProbeBook(moveNr+1, appData.polyglotBook); // returns move
    if(appData.debugMode) fprintf(debugFP, "book hit = %s\n", bookHit ? bookHit : "(NULL)");
    if(bookHit != NULL && !cps->bookSuspend) {
	// make sure opponent is not going to reply after receiving move to book position
	SendToProgram("force\n", cps);
	cps->bookSuspend = TRUE; // flag indicating it has to be restarted
    }
    if(!initial) SendMoveToProgram(moveNr, cps); // with hit on initial position there is no move
    // now arrange restart after book miss
    if(bookHit) {
	// after a book hit we never send 'go', and the code after the call to this routine
	// has '&& !bookHit' added to suppress potential sending there (based on 'firstMove').
	char buf[MSG_SIZ];
	if (cps->useUsermove) sprintf(buf, "usermove "); // sorry, no SAN yet :(
	sprintf(buf, "%s\n", bookHit); // force book move into program supposed to play it
	SendToProgram(buf, cps);
	if(!initial) firstMove = FALSE; // normally we would clear the firstMove condition after return & sending 'go'
    } else if(initial) { // 'go' was needed irrespective of firstMove, and it has to be done in this routine
	SendToProgram("go\n", cps);
	cps->bookSuspend = FALSE; // after a 'go' we are never suspended
    } else { // 'go' might be sent based on 'firstMove' after this routine returns
	if(cps->bookSuspend && !firstMove) // 'go' needed, and it will not be done after we return
	    SendToProgram("go\n", cps); 
	cps->bookSuspend = FALSE; // anyhow, we will not be suspended after a miss
    }
    return bookHit; // notify caller of hit, so it can take action to send move to opponent
}

char *savedMessage;
ChessProgramState *savedState;
void DeferredBookMove(void)
{
	if(savedState->lastPing != savedState->lastPong)
		    ScheduleDelayedEvent(DeferredBookMove, 10);
	else
	HandleMachineMove(savedMessage, savedState);
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
    char *bookHit;

FakeBookMove: // [HGM] book: we jump here to simulate machine moves after book hit
    /*
     * Kludge to ignore BEL characters
     */
    while (*message == '\007') message++;

    /*
     * [HGM] engine debug message: ignore lines starting with '#' character
     */
    if(cps->debug && *message == '#') return;

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
    if ((sscanf(message, "%s %s %s", buf1, buf2, machineMove) == 3 && strcmp(buf2, "...") == 0) ||
	(sscanf(message, "%s %s", buf1, machineMove) == 2 && strcmp(buf1, "move") == 0)) 
    {
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

    if (appData.debugMode) { int f = forwardMostMove;
        fprintf(debugFP, "machine move %d, castling = %d %d %d %d %d %d\n", f,
                castlingRights[f][0],castlingRights[f][1],castlingRights[f][2],castlingRights[f][3],castlingRights[f][4],castlingRights[f][5]);
    }
        if(cps->alphaRank) AlphaRank(machineMove, 4);
        if (!ParseOneMove(machineMove, forwardMostMove, &moveType,
                              &fromX, &fromY, &toX, &toY, &promoChar)) {
	    /* Machine move could not be parsed; ignore it. */
            sprintf(buf1, _("Illegal move \"%s\" from %s machine"),
		    machineMove, cps->which);
	    DisplayError(buf1, 0);
            sprintf(buf1, "Xboard: Forfeit due to invalid move: %s (%c%c%c%c) res=%d",
                    machineMove, fromX+AAA, fromY+ONE, toX+AAA, toY+ONE, moveType);
	    if (gameMode == TwoMachinesPlay) {
	      GameEnds(machineWhite ? BlackWins : WhiteWins,
                       buf1, GE_XBOARD);
	    }
	    return;
	}

        /* [HGM] Apparently legal, but so far only tested with EP_UNKOWN */
        /* So we have to redo legality test with true e.p. status here,  */
        /* to make sure an illegal e.p. capture does not slip through,   */
        /* to cause a forfeit on a justified illegal-move complaint      */
        /* of the opponent.                                              */
        if( gameMode==TwoMachinesPlay && appData.testLegality
            && fromY != DROP_RANK /* [HGM] temporary; should still add legality test for drops */
                                                              ) {
           ChessMove moveType;
           moveType = LegalityTest(boards[forwardMostMove], PosFlags(forwardMostMove),
                        epStatus[forwardMostMove], castlingRights[forwardMostMove],
                             fromY, fromX, toY, toX, promoChar);
	    if (appData.debugMode) {
                int i;
                for(i=0; i< nrCastlingRights; i++) fprintf(debugFP, "(%d,%d) ",
                    castlingRights[forwardMostMove][i], castlingRank[i]);
                fprintf(debugFP, "castling rights\n");
	    }
            if(moveType == IllegalMove) {
                sprintf(buf1, "Xboard: Forfeit due to illegal move: %s (%c%c%c%c)%c",
                        machineMove, fromX+AAA, fromY+ONE, toX+AAA, toY+ONE, 0);
                GameEnds(machineWhite ? BlackWins : WhiteWins,
                           buf1, GE_XBOARD);
		return;
           } else if(gameInfo.variant != VariantFischeRandom && gameInfo.variant != VariantCapaRandom)
           /* [HGM] Kludge to handle engines that send FRC-style castling
              when they shouldn't (like TSCP-Gothic) */
           switch(moveType) {
             case WhiteASideCastleFR:
             case BlackASideCastleFR:
               toX+=2;
               currentMoveString[2]++;
               break;
             case WhiteHSideCastleFR:
             case BlackHSideCastleFR:
               toX--;
               currentMoveString[2]--;
               break;
	     default: ; // nothing to do, but suppresses warning of pedantic compilers
           }
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
	  if(appData.autoKibitz && !appData.icsEngineAnalyze ) { /* [HGM] kibitz: send most-recent PV info to ICS */
		char buf[3*MSG_SIZ];

		sprintf(buf, "kibitz !!! %+.2f/%d (%.2f sec, %u nodes, %1.0f knps) PV=%s\n",
			programStats.score / 100.,
			programStats.depth,
			programStats.time / 100.,
			(unsigned int)programStats.nodes,
			(unsigned int)programStats.nodes / (10*abs(programStats.time) + 1.),
			programStats.movelist);
		SendToICS(buf);
	  }
	}
#endif
	/* currentMoveString is set as a side-effect of ParseOneMove */
	strcpy(machineMove, currentMoveString);
	strcat(machineMove, "\n");
	strcpy(moveList[forwardMostMove], machineMove);

        /* [AS] Save move info and clear stats for next move */
        pvInfoList[ forwardMostMove ].score = programStats.score;
        pvInfoList[ forwardMostMove ].depth = programStats.depth;
        pvInfoList[ forwardMostMove ].time =  programStats.time; // [HGM] PGNtime: take time from engine stats
        ClearProgramStats();
        thinkOutput[0] = NULLCHAR;
        hiddenThinkOutputState = 0;

	MakeMove(fromX, fromY, toX, toY, promoChar);/*updates forwardMostMove*/

        /* [AS] Adjudicate game if needed (note: remember that forwardMostMove now points past the last move) */
        if( gameMode == TwoMachinesPlay && adjudicateLossThreshold != 0 && forwardMostMove >= adjudicateLossPlies ) {
            int count = 0;

            while( count < adjudicateLossPlies ) {
                int score = pvInfoList[ forwardMostMove - count - 1 ].score;

                if( count & 1 ) {
                    score = -score; /* Flip score for winning side */
                }

                if( score > adjudicateLossThreshold ) {
                    break;
                }

                count++;
            }

            if( count >= adjudicateLossPlies ) {
	        ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/

                GameEnds( WhiteOnMove(forwardMostMove) ? WhiteWins : BlackWins, 
                    "Xboard adjudication", 
                    GE_XBOARD );

                return;
            }
        }

	if( gameMode == TwoMachinesPlay ) {
	  // [HGM] some adjudications useful with buggy engines
            int k, count = 0, epFile = epStatus[forwardMostMove]; static int bare = 1;
	  if(gameInfo.holdingsSize == 0 || gameInfo.variant == VariantSuper || gameInfo.variant == VariantGreat) {


	    if( appData.testLegality )
	    {   /* [HGM] Some more adjudications for obstinate engines */
		int NrWN=0, NrBN=0, NrWB=0, NrBB=0, NrWR=0, NrBR=0,
                    NrWQ=0, NrBQ=0, NrW=0, NrK=0, bishopsColor = 0,
                    NrPieces=0, NrPawns=0, PawnAdvance=0, i, j;
		static int moveCount = 6;
		ChessMove result;
		char *reason = NULL;

                /* Count what is on board. */
		for(i=0; i<BOARD_HEIGHT; i++) for(j=BOARD_LEFT; j<BOARD_RGHT; j++)
		{   ChessSquare p = boards[forwardMostMove][i][j];
		    int m=i;

		    switch((int) p)
		    {   /* count B,N,R and other of each side */
                        case WhiteKing:
                        case BlackKing:
			     NrK++; break; // [HGM] atomic: count Kings
                        case WhiteKnight:
                             NrWN++; break;
                        case WhiteBishop:
                        case WhiteFerz:    // [HGM] shatranj: kludge to mke it work in shatranj
                             bishopsColor |= 1 << ((i^j)&1);
                             NrWB++; break;
                        case BlackKnight:
                             NrBN++; break;
                        case BlackBishop:
                        case BlackFerz:    // [HGM] shatranj: kludge to mke it work in shatranj
                             bishopsColor |= 1 << ((i^j)&1);
                             NrBB++; break;
                        case WhiteRook:
                             NrWR++; break;
                        case BlackRook:
                             NrBR++; break;
                        case WhiteQueen:
                             NrWQ++; break;
                        case BlackQueen:
                             NrBQ++; break;
                        case EmptySquare: 
                             break;
                        case BlackPawn:
                             m = 7-i;
                        case WhitePawn:
                             PawnAdvance += m; NrPawns++;
                    }
                    NrPieces += (p != EmptySquare);
                    NrW += ((int)p < (int)BlackPawn);
		    if(gameInfo.variant == VariantXiangqi && 
		      (p == WhiteFerz || p == WhiteAlfil || p == BlackFerz || p == BlackAlfil)) {
			NrPieces--; // [HGM] XQ: do not count purely defensive pieces
                        NrW -= ((int)p < (int)BlackPawn);
		    }
                }

		/* Some material-based adjudications that have to be made before stalemate test */
		if(gameInfo.variant == VariantAtomic && NrK < 2) {
		    // [HGM] atomic: stm must have lost his King on previous move, as destroying own K is illegal
		     epStatus[forwardMostMove] = EP_CHECKMATE; // make claimable as if stm is checkmated
		     if(appData.checkMates) {
			 SendMoveToProgram(forwardMostMove-1, cps->other); // make sure opponent gets move
                         ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
                         GameEnds( WhiteOnMove(forwardMostMove) ? BlackWins : WhiteWins, 
							"Xboard adjudication: King destroyed", GE_XBOARD );
                         return;
		     }
		}

		/* Bare King in Shatranj (loses) or Losers (wins) */
                if( NrW == 1 || NrPieces - NrW == 1) {
                  if( gameInfo.variant == VariantLosers) { // [HGM] losers: bare King wins (stm must have it first)
		     epStatus[forwardMostMove] = EP_WINS;  // mark as win, so it becomes claimable
		     if(appData.checkMates) {
			 SendMoveToProgram(forwardMostMove-1, cps->other); // make sure opponent gets to see move
                         ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
                         GameEnds( WhiteOnMove(forwardMostMove) ? WhiteWins : BlackWins, 
							"Xboard adjudication: Bare king", GE_XBOARD );
                         return;
		     }
		  } else
                  if( gameInfo.variant == VariantShatranj && --bare < 0)
                  {    /* bare King */
			epStatus[forwardMostMove] = EP_WINS; // make claimable as win for stm
			if(appData.checkMates) {
			    /* but only adjudicate if adjudication enabled */
			    SendMoveToProgram(forwardMostMove-1, cps->other); // make sure opponent gets move
			    ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
			    GameEnds( NrW > 1 ? WhiteWins : NrPieces - NrW > 1 ? BlackWins : GameIsDrawn, 
							"Xboard adjudication: Bare king", GE_XBOARD );
			    return;
			}
		  }
                } else bare = 1;


            // don't wait for engine to announce game end if we can judge ourselves
            switch (MateTest(boards[forwardMostMove], PosFlags(forwardMostMove), epFile,
                                       castlingRights[forwardMostMove]) ) {
	      case MT_CHECK:
		if(gameInfo.variant == Variant3Check) { // [HGM] 3check: when in check, test if 3rd time
		    int i, checkCnt = 0;    // (should really be done by making nr of checks part of game state)
		    for(i=forwardMostMove-2; i>=backwardMostMove; i-=2) {
			if(MateTest(boards[i], PosFlags(i), epStatus[i], castlingRights[i]) == MT_CHECK)
			    checkCnt++;
			if(checkCnt >= 2) {
			    reason = "Xboard adjudication: 3rd check";
			    epStatus[forwardMostMove] = EP_CHECKMATE;
			    break;
			}
		    }
		}
	      case MT_NONE:
	      default:
		break;
	      case MT_STALEMATE:
	      case MT_STAINMATE:
		reason = "Xboard adjudication: Stalemate";
		if(epStatus[forwardMostMove] != EP_CHECKMATE) { // [HGM] don't touch win through baring or K-capt
		    epStatus[forwardMostMove] = EP_STALEMATE;   // default result for stalemate is draw
		    if(gameInfo.variant == VariantLosers  || gameInfo.variant == VariantGiveaway) // [HGM] losers:
			epStatus[forwardMostMove] = EP_WINS;    // in these variants stalemated is always a win
		    else if(gameInfo.variant == VariantSuicide) // in suicide it depends
			epStatus[forwardMostMove] = NrW == NrPieces-NrW ? EP_STALEMATE :
						   ((NrW < NrPieces-NrW) != WhiteOnMove(forwardMostMove) ?
									EP_CHECKMATE : EP_WINS);
		    else if(gameInfo.variant == VariantShatranj || gameInfo.variant == VariantXiangqi)
		        epStatus[forwardMostMove] = EP_CHECKMATE; // and in these variants being stalemated loses
		}
		break;
	      case MT_CHECKMATE:
		reason = "Xboard adjudication: Checkmate";
		epStatus[forwardMostMove] = (gameInfo.variant == VariantLosers ? EP_WINS : EP_CHECKMATE);
		break;
	    }

		switch(i = epStatus[forwardMostMove]) {
		    case EP_STALEMATE:
			result = GameIsDrawn; break;
		    case EP_CHECKMATE:
			result = WhiteOnMove(forwardMostMove) ? BlackWins : WhiteWins; break;
		    case EP_WINS:
			result = WhiteOnMove(forwardMostMove) ? WhiteWins : BlackWins; break;
		    default:
			result = (ChessMove) 0;
		}
                if(appData.checkMates && result) { // [HGM] mates: adjudicate finished games if requested
		    SendMoveToProgram(forwardMostMove-1, cps->other); /* make sure opponent gets to see move */
		    ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
		    GameEnds( result, reason, GE_XBOARD );
		    return;
		}

                /* Next absolutely insufficient mating material. */
                if( NrPieces == 2 || gameInfo.variant != VariantXiangqi && 
				     gameInfo.variant != VariantShatranj && // [HGM] baring will remain possible
			(NrPieces == 3 && NrWN+NrBN+NrWB+NrBB == 1 ||
			 NrPieces == NrBB+NrWB+2 && bishopsColor != 3)) // [HGM] all Bishops (Ferz!) same color
                {    /* KBK, KNK, KK of KBKB with like Bishops */

                     /* always flag draws, for judging claims */
                     epStatus[forwardMostMove] = EP_INSUF_DRAW;

                     if(appData.materialDraws) {
                         /* but only adjudicate them if adjudication enabled */
			 SendToProgram("force\n", cps->other); // suppress reply
			 SendMoveToProgram(forwardMostMove-1, cps->other); /* make sure opponent gets to see last move */
                         ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
                         GameEnds( GameIsDrawn, "Xboard adjudication: Insufficient mating material", GE_XBOARD );
                         return;
                     }
                }

                /* Then some trivial draws (only adjudicate, cannot be claimed) */
                if(NrPieces == 4 && 
                   (   NrWR == 1 && NrBR == 1 /* KRKR */
                   || NrWQ==1 && NrBQ==1     /* KQKQ */
                   || NrWN==2 || NrBN==2     /* KNNK */
                   || NrWN+NrWB == 1 && NrBN+NrBB == 1 /* KBKN, KBKB, KNKN */
                  ) ) {
                     if(--moveCount < 0 && appData.trivialDraws)
                     {    /* if the first 3 moves do not show a tactical win, declare draw */
			  SendToProgram("force\n", cps->other); // suppress reply
			  SendMoveToProgram(forwardMostMove-1, cps->other); /* make sure opponent gets to see move */
                          ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
                          GameEnds( GameIsDrawn, "Xboard adjudication: Trivial draw", GE_XBOARD );
                          return;
                     }
                } else moveCount = 6;
	    }
	  }
#if 1
    if (appData.debugMode) { int i;
      fprintf(debugFP, "repeat test fmm=%d bmm=%d ep=%d, reps=%d\n",
              forwardMostMove, backwardMostMove, epStatus[backwardMostMove],
              appData.drawRepeats);
      for( i=forwardMostMove; i>=backwardMostMove; i-- )
           fprintf(debugFP, "%d ep=%d\n", i, epStatus[i]);

    }
#endif
                /* Check for rep-draws */
                count = 0;
                for(k = forwardMostMove-2;
                    k>=backwardMostMove && k>=forwardMostMove-100 &&
                        epStatus[k] < EP_UNKNOWN &&
                        epStatus[k+2] <= EP_NONE && epStatus[k+1] <= EP_NONE;
                    k-=2)
                {   int rights=0;
#if 0
    if (appData.debugMode) {
      fprintf(debugFP, " loop\n");
    }
#endif
                    if(CompareBoards(boards[k], boards[forwardMostMove])) {
#if 0
    if (appData.debugMode) {
      fprintf(debugFP, "match\n");
    }
#endif
                        /* compare castling rights */
                        if( castlingRights[forwardMostMove][2] != castlingRights[k][2] &&
                             (castlingRights[k][0] >= 0 || castlingRights[k][1] >= 0) )
                                rights++; /* King lost rights, while rook still had them */
                        if( castlingRights[forwardMostMove][2] >= 0 ) { /* king has rights */
                            if( castlingRights[forwardMostMove][0] != castlingRights[k][0] ||
                                castlingRights[forwardMostMove][1] != castlingRights[k][1] )
                                   rights++; /* but at least one rook lost them */
                        }
                        if( castlingRights[forwardMostMove][5] != castlingRights[k][5] &&
                             (castlingRights[k][3] >= 0 || castlingRights[k][4] >= 0) )
                                rights++; 
                        if( castlingRights[forwardMostMove][5] >= 0 ) {
                            if( castlingRights[forwardMostMove][3] != castlingRights[k][3] ||
                                castlingRights[forwardMostMove][4] != castlingRights[k][4] )
                                   rights++;
                        }
#if 0
    if (appData.debugMode) {
      for(i=0; i<nrCastlingRights; i++)
      fprintf(debugFP, " (%d,%d)", castlingRights[forwardMostMove][i], castlingRights[k][i]);
    }

    if (appData.debugMode) {
      fprintf(debugFP, " %d %d\n", rights, k);
    }
#endif
                        if( rights == 0 && ++count > appData.drawRepeats-2
                            && appData.drawRepeats > 1) {
                             /* adjudicate after user-specified nr of repeats */
			     SendToProgram("force\n", cps->other); // suppress reply
			     SendMoveToProgram(forwardMostMove-1, cps->other); /* make sure opponent gets to see move */
                             ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
			     if(gameInfo.variant == VariantXiangqi && appData.testLegality) { 
				// [HGM] xiangqi: check for forbidden perpetuals
				int m, ourPerpetual = 1, hisPerpetual = 1;
				for(m=forwardMostMove; m>k; m-=2) {
				    if(MateTest(boards[m], PosFlags(m), 
							EP_NONE, castlingRights[m]) != MT_CHECK)
					ourPerpetual = 0; // the current mover did not always check
				    if(MateTest(boards[m-1], PosFlags(m-1), 
							EP_NONE, castlingRights[m-1]) != MT_CHECK)
					hisPerpetual = 0; // the opponent did not always check
				}
				if(appData.debugMode) fprintf(debugFP, "XQ perpetual test, our=%d, his=%d\n",
									ourPerpetual, hisPerpetual);
				if(ourPerpetual && !hisPerpetual) { // we are actively checking him: forfeit
				    GameEnds( WhiteOnMove(forwardMostMove) ? WhiteWins : BlackWins, 
		 			   "Xboard adjudication: perpetual checking", GE_XBOARD );
				    return;
				}
				if(hisPerpetual && !ourPerpetual)   // he is checking us, but did not repeat yet
				    break; // (or we would have caught him before). Abort repetition-checking loop.
				// Now check for perpetual chases
				if(!ourPerpetual && !hisPerpetual) { // no perpetual check, test for chase
				    hisPerpetual = PerpetualChase(k, forwardMostMove);
				    ourPerpetual = PerpetualChase(k+1, forwardMostMove);
				    if(ourPerpetual && !hisPerpetual) { // we are actively chasing him: forfeit
					GameEnds( WhiteOnMove(forwardMostMove) ? WhiteWins : BlackWins, 
		 				      "Xboard adjudication: perpetual chasing", GE_XBOARD );
					return;
				    }
				    if(hisPerpetual && !ourPerpetual)   // he is chasing us, but did not repeat yet
					break; // Abort repetition-checking loop.
				}
				// if neither of us is checking or chasing all the time, or both are, it is draw
			     }
                             GameEnds( GameIsDrawn, "Xboard adjudication: repetition draw", GE_XBOARD );
                             return;
                        }
                        if( rights == 0 && count > 1 ) /* occurred 2 or more times before */
                             epStatus[forwardMostMove] = EP_REP_DRAW;
                    }
                }

                /* Now we test for 50-move draws. Determine ply count */
                count = forwardMostMove;
                /* look for last irreversble move */
                while( epStatus[count] <= EP_NONE && count > backwardMostMove )
                    count--;
                /* if we hit starting position, add initial plies */
                if( count == backwardMostMove )
                    count -= initialRulePlies;
                count = forwardMostMove - count; 
                if( count >= 100)
                         epStatus[forwardMostMove] = EP_RULE_DRAW;
                         /* this is used to judge if draw claims are legal */
                if(appData.ruleMoves > 0 && count >= 2*appData.ruleMoves) {
			 SendToProgram("force\n", cps->other); // suppress reply
			 SendMoveToProgram(forwardMostMove-1, cps->other); /* make sure opponent gets to see move */
                         ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
                         GameEnds( GameIsDrawn, "Xboard adjudication: 50-move rule", GE_XBOARD );
                         return;
                }

                /* if draw offer is pending, treat it as a draw claim
                 * when draw condition present, to allow engines a way to
                 * claim draws before making their move to avoid a race
                 * condition occurring after their move
                 */
                if( cps->other->offeredDraw || cps->offeredDraw ) {
                         char *p = NULL;
                         if(epStatus[forwardMostMove] == EP_RULE_DRAW)
                             p = "Draw claim: 50-move rule";
                         if(epStatus[forwardMostMove] == EP_REP_DRAW)
                             p = "Draw claim: 3-fold repetition";
                         if(epStatus[forwardMostMove] == EP_INSUF_DRAW)
                             p = "Draw claim: insufficient mating material";
                         if( p != NULL ) {
			     SendToProgram("force\n", cps->other); // suppress reply
			     SendMoveToProgram(forwardMostMove-1, cps->other); /* make sure opponent gets to see move */
                             GameEnds( GameIsDrawn, p, GE_XBOARD );
                             ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/
                             return;
                         }
                }


	        if( appData.adjudicateDrawMoves > 0 && forwardMostMove > (2*appData.adjudicateDrawMoves) ) {
		    SendToProgram("force\n", cps->other); // suppress reply
		    SendMoveToProgram(forwardMostMove-1, cps->other); /* make sure opponent gets to see move */
		    ShowMove(fromX, fromY, toX, toY); /*updates currentMove*/

	            GameEnds( GameIsDrawn, "Xboard adjudication: long game", GE_XBOARD );

	            return;
        	}
        }

	bookHit = NULL;
	if (gameMode == TwoMachinesPlay) {
            /* [HGM] relaying draw offers moved to after reception of move */
            /* and interpreting offer as claim if it brings draw condition */
            if (cps->offeredDraw == 1 && cps->other->sendDrawOffers) {
                SendToProgram("draw\n", cps->other);
            }
	    if (cps->other->sendTime) {
		SendTimeRemaining(cps->other,
				  cps->other->twoMachinesColor[0] == 'w');
	    }
	    bookHit = SendMoveToBookUser(forwardMostMove-1, cps->other, FALSE);
	    if (firstMove && !bookHit) {
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

	// [HGM] book: after book hit opponent has received move and is now in force mode
	// force the book reply into it, and then fake that it outputted this move by jumping
	// back to the beginning of HandleMachineMove, with cps toggled and message set to this move
	if(bookHit) {
		static char bookMove[MSG_SIZ]; // a bit generous?

		strcpy(bookMove, "move ");
		strcat(bookMove, bookHit);
		message = bookMove;
		cps = cps->other;
		programStats.nodes = programStats.depth = programStats.time = 
		programStats.score = programStats.got_only_move = 0;
		sprintf(programStats.movelist, "%s (xbook)", bookHit);

		if(cps->lastPing != cps->lastPong) {
		    savedMessage = message; // args for deferred call
		    savedState = cps;
		    ScheduleDelayedEvent(DeferredBookMove, 10);
		    return;
		}
		goto FakeBookMove;
	}

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
    if (strncmp(message, "feature ", 8) == 0) { // [HGM] moved forward to pre-empt non-compliant commands
      ParseFeatures(message+8, cps);
      return; // [HGM] This return was missing, causing option features to be recognized as non-compliant commands!
    }

    /* [HGM] Allow engine to set up a position. Don't ask me why one would
     * want this, I was asked to put it in, and obliged.
     */
    if (!strncmp(message, "setboard ", 9)) {
        Board initial_position; int i;

        GameEnds(GameUnfinished, "Engine aborts game", GE_XBOARD);

        if (!ParseFEN(initial_position, &blackPlaysFirst, message + 9)) {
            DisplayError(_("Bad FEN received from engine"), 0);
            return ;
        } else {
           Reset(FALSE, FALSE);
           CopyBoard(boards[0], initial_position);
           initialRulePlies = FENrulePlies;
           epStatus[0] = FENepStatus;
           for( i=0; i<nrCastlingRights; i++ )
                castlingRights[0][i] = FENcastlingRights[i];
           if(blackPlaysFirst) gameMode = MachinePlaysWhite;
           else gameMode = MachinePlaysBlack;                 
           DrawPosition(FALSE, boards[currentMove]);
        }
	return;
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
	  snprintf(buf1, sizeof(buf1), "%ssay %s\n", ics_prefix, message + 13);
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
	  snprintf(buf1, sizeof(buf1), "%swhisper %s\n", ics_prefix, message + 11);
	  SendToICS(buf1);
	}
      }
      return;
    }
    if (!strncmp(message, "tellall ", 8)) {
      if (appData.icsActive) {
	if (loggedOn) {
	  snprintf(buf1, sizeof(buf1), "%skibitz %s\n", ics_prefix, message + 8);
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
	    sprintf(buf2, _("%s does not support analysis"), cps->tidy);
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
        if (!StrStr(message, "llegal")) {
            return;
        }
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

        /* [HGM] illegal-move claim should forfeit game when Xboard */
        /* only passes fully legal moves                            */
        if( appData.testLegality && gameMode == TwoMachinesPlay ) {
            GameEnds( cps->twoMachinesColor[0] == 'w' ? BlackWins : WhiteWins,
                                "False illegal-move claim", GE_XBOARD );
        }
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
	snprintf(buf1, sizeof(buf1), _("Failed to start %s chess program %s on %s: %s\n"),
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
		snprintf(buf2, sizeof(buf2), _("Hint: %s"), buf1);
		DisplayInformation(buf2);
	    } else {
		/* Hint move could not be parsed!? */
	      snprintf(buf2, sizeof(buf2),
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
        GameEnds(WhiteWins, r, GE_ENGINE1 + (cps != &first)); /* [HGM] pass claimer indication for claim test */
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
            GameEnds(WhiteWins, r, GE_ENGINE1 + (cps != &first));
	    return;
	}
        GameEnds(BlackWins, r, GE_ENGINE1 + (cps != &first));
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
            
        GameEnds(GameIsDrawn, r, GE_ENGINE1 + (cps != &first));
	return;

    } else if (strncmp(message, "White resign", 12) == 0) {
        GameEnds(BlackWins, "White resigns", GE_ENGINE1 + (cps != &first));
	return;
    } else if (strncmp(message, "Black resign", 12) == 0) {
        GameEnds(WhiteWins, "Black resigns", GE_ENGINE1 + (cps != &first));
	return;
    } else if (strncmp(message, "White matches", 13) == 0 ||
               strncmp(message, "Black matches", 13) == 0   ) {
        /* [HGM] ignore GNUShogi noises */
        return;
    } else if (strncmp(message, "White", 5) == 0 &&
	       message[5] != '(' &&
	       StrStr(message, "Black") == NULL) {
        GameEnds(WhiteWins, "White mates", GE_ENGINE1 + (cps != &first));
	return;
    } else if (strncmp(message, "Black", 5) == 0 &&
	       message[5] != '(') {
        GameEnds(BlackWins, "Black mates", GE_ENGINE1 + (cps != &first));
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
              GameEnds(BlackWins, "White resigns", GE_ENGINE1 + (cps != &first));
	    else
              GameEnds(WhiteWins, "Black resigns", GE_ENGINE1 + (cps != &first));
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
              GameEnds(BlackWins, "Black mates", GE_ENGINE1 + (cps != &first));
	    else
              GameEnds(WhiteWins, "White mates", GE_ENGINE1 + (cps != &first));
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
            GameEnds(BlackWins, "Black mates", GE_ENGINE1);
	    break;
	  case MachinePlaysWhite:
	  case IcsPlayingWhite:
            GameEnds(WhiteWins, "White mates", GE_ENGINE);
	    break;
	  case TwoMachinesPlay:
	    if (cps->twoMachinesColor[0] == 'w')
              GameEnds(WhiteWins, "White mates", GE_ENGINE1 + (cps != &first));
	    else
              GameEnds(BlackWins, "Black mates", GE_ENGINE1 + (cps != &first));
	    break;
	  default:
	    /* can't happen */
	    break;
	}
	return;
    } else if (strncmp(message, "checkmate", 9) == 0) {
	if (WhiteOnMove(forwardMostMove)) {
            GameEnds(BlackWins, "Black mates", GE_ENGINE1 + (cps != &first));
	} else {
            GameEnds(WhiteWins, "White mates", GE_ENGINE1 + (cps != &first));
	}
	return;
    } else if (strstr(message, "Draw") != NULL ||
	       strstr(message, "game is a draw") != NULL) {
        GameEnds(GameIsDrawn, "Draw", GE_ENGINE1 + (cps != &first));
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
            /* [HGM] in two-machine mode we delay relaying draw offer      */
            /* until after we also have move, to see if it is really claim */
	    }
#if 0
              else {
	        if (cps->other->sendDrawOffers) {
		    SendToProgram("draw\n", cps->other);
		}
	    }
#endif
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
    if ( appData.showThinking // [HGM] thinking: test all options that cause this output
	  || !appData.hideThinkingFromHuman || appData.adjudicateLossThreshold != 0 || EngineOutputIsUp()
				) {
	int plylev, mvleft, mvtot, curscore, time;
	char mvname[MOVE_LEN];
	u64 nodes; // [DM]
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
            break;
          case IcsObserving: /* [DM] icsEngineAnalyze */
            if (!appData.icsEngineAnalyze) ignore = TRUE;
	    break;
	  case TwoMachinesPlay:
	    if ((cps->twoMachinesColor[0] == 'w') != WhiteOnMove(forwardMostMove)) {
		ignore = TRUE;
	    }
	    break;
	  default:
	    ignore = TRUE;
	    break;
	}

	if (!ignore) {
	    buf1[0] = NULLCHAR;
	    if (sscanf(message, "%d%c %d %d " u64Display " %[^\n]\n",
		       &plylev, &plyext, &curscore, &time, &nodes, buf1) >= 5) {

		if (plyext != ' ' && plyext != '\t') {
		    time *= 100;
		}

                /* [AS] Negate score if machine is playing black and reporting absolute scores */
                if( cps->scoreIsAbsolute && 
                    ((gameMode == MachinePlaysBlack) || (gameMode == TwoMachinesPlay && cps->twoMachinesColor[0] == 'b')) )
                {
                    curscore = -curscore;
                }


		programStats.depth = plylev;
		programStats.nodes = nodes;
		programStats.time = time;
		programStats.score = curscore;
		programStats.got_only_move = 0;

		if(cps->nps >= 0) { /* [HGM] nps: use engine nodes or time to decrement clock */
			int ticklen;

			if(cps->nps == 0) ticklen = 10*time;                    // use engine reported time
			else ticklen = (1000. * u64ToDouble(nodes)) / cps->nps; // convert node count to time
			if(WhiteOnMove(forwardMostMove)) 
			     whiteTimeRemaining = timeRemaining[0][forwardMostMove] - ticklen;
			else blackTimeRemaining = timeRemaining[1][forwardMostMove] - ticklen;
		}

		/* Buffer overflow protection */
		if (buf1[0] != NULLCHAR) {
		    if (strlen(buf1) >= sizeof(programStats.movelist)
			&& appData.debugMode) {
			fprintf(debugFP,
				"PV is too long; using the first %d bytes.\n",
				sizeof(programStats.movelist) - 1);
		    }

                    safeStrCpy( programStats.movelist, buf1, sizeof(programStats.movelist) );
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

                SendProgramStatsToFrontend( cps, &programStats );

                /* 
                    [AS] Protect the thinkOutput buffer from overflow... this
                    is only useful if buf1 hasn't overflowed first!
                */
		sprintf(thinkOutput, "[%d]%c%+.2f %s%s",
			plylev, 
			(gameMode == TwoMachinesPlay ?
			 ToUpper(cps->twoMachinesColor[0]) : ' '),
			((double) curscore) / 100.0,
			prefixHint ? lastHint : "",
			prefixHint ? " " : "" );

                if( buf1[0] != NULLCHAR ) {
                    unsigned max_len = sizeof(thinkOutput) - strlen(thinkOutput) - 1;

                    if( strlen(buf1) > max_len ) {
			if( appData.debugMode) {
			    fprintf(debugFP,"PV is too long for thinkOutput, truncating.\n");
                        }
                        buf1[max_len+1] = '\0';
                    }

                    strcat( thinkOutput, buf1 );
                }

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

                SendProgramStatsToFrontend( cps, &programStats );
                
		if (currentMove == forwardMostMove || gameMode==AnalyzeMode || 
                           gameMode == AnalyzeFile || appData.icsEngineAnalyze) {
		    DisplayMove(currentMove - 1);
		    DisplayAnalysis();
		}
		return;
	    } else if (sscanf(message,"stat01: %d " u64Display " %d %d %d %s",
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
                programStats.movelist[0] = '\0';

                SendProgramStatsToFrontend( cps, &programStats );

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
                unsigned message_len;

	        p = message;
		while (*p && *p == ' ') p++;

                message_len = strlen( p );

                /* [AS] Avoid buffer overflow */
                if( sizeof(thinkOutput) - strlen(thinkOutput) - 1 > message_len ) {
		    strcat(thinkOutput, " ");
		    strcat(thinkOutput, p);
                }

                if( sizeof(programStats.movelist) - strlen(programStats.movelist) - 1 > message_len ) {
		    strcat(programStats.movelist, " ");
		    strcat(programStats.movelist, p);
                }

		if (currentMove == forwardMostMove || gameMode==AnalyzeMode ||
                           gameMode == AnalyzeFile || appData.icsEngineAnalyze) {
		    DisplayMove(currentMove - 1);
		    DisplayAnalysis();
		}
		return;
	    }
	}
        else {
	    buf1[0] = NULLCHAR;

	    if (sscanf(message, "%d%c %d %d " u64Display " %[^\n]\n",
		       &plylev, &plyext, &curscore, &time, &nodes, buf1) >= 5) 
            {
                ChessProgramStats cpstats;

		if (plyext != ' ' && plyext != '\t') {
		    time *= 100;
		}

                /* [AS] Negate score if machine is playing black and reporting absolute scores */
                if( cps->scoreIsAbsolute && ((gameMode == MachinePlaysBlack) || (gameMode == TwoMachinesPlay && cps->twoMachinesColor[0] == 'b')) ) {
                    curscore = -curscore;
                }

		cpstats.depth = plylev;
		cpstats.nodes = nodes;
		cpstats.time = time;
		cpstats.score = curscore;
		cpstats.got_only_move = 0;
                cpstats.movelist[0] = '\0';

		if (buf1[0] != NULLCHAR) {
                    safeStrCpy( cpstats.movelist, buf1, sizeof(cpstats.movelist) );
		}

		cpstats.ok_to_send = 0;
		cpstats.line_is_book = 0;
		cpstats.nr_moves = 0;
		cpstats.moves_left = 0;

                SendProgramStatsToFrontend( cps, &cpstats );
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
	  case IllegalMove:		/* maybe suicide chess, etc. */
  if (appData.debugMode) {
    fprintf(debugFP, "Illegal move from ICS: '%s'\n", yy_text);
    fprintf(debugFP, "board L=%d, R=%d, H=%d, holdings=%d\n", BOARD_LEFT, BOARD_RGHT, BOARD_HEIGHT, gameInfo.holdingsWidth);
    setbuf(debugFP, NULL);
  }
          case WhitePromotionChancellor:
          case BlackPromotionChancellor:
          case WhitePromotionArchbishop:
          case BlackPromotionArchbishop:
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
          /* PUSH Fabien */
          case WhiteHSideCastleFR:
          case WhiteASideCastleFR:
          case BlackHSideCastleFR:
          case BlackASideCastleFR:
          /* POP Fabien */
            fromX = currentMoveString[0] - AAA;
            fromY = currentMoveString[1] - ONE;
            toX = currentMoveString[2] - AAA;
            toY = currentMoveString[3] - ONE;
	    promoChar = currentMoveString[4];
	    break;
	  case WhiteDrop:
	  case BlackDrop:
	    fromX = moveType == WhiteDrop ?
	      (int) CharToPiece(ToUpper(currentMoveString[0])) :
	    (int) CharToPiece(ToLower(currentMoveString[0]));
	    fromY = DROP_RANK;
            toX = currentMoveString[2] - AAA;
            toY = currentMoveString[3] - ONE;
	    promoChar = NULLCHAR;
	    break;
	  case AmbiguousMove:
	    /* bug? */
	    sprintf(buf, _("Ambiguous move in ICS output: \"%s\""), yy_text);
  if (appData.debugMode) {
    fprintf(debugFP, "Ambiguous move from ICS: '%s'\n", yy_text);
    fprintf(debugFP, "board L=%d, R=%d, H=%d, holdings=%d\n", BOARD_LEFT, BOARD_RGHT, BOARD_HEIGHT, gameInfo.holdingsWidth);
    setbuf(debugFP, NULL);
  }
	    DisplayError(buf, 0);
	    return;
	  case ImpossibleMove:
	    /* bug? */
	    sprintf(buf, _("Illegal move in ICS output: \"%s\""), yy_text);
  if (appData.debugMode) {
    fprintf(debugFP, "Impossible move from ICS: '%s'\n", yy_text);
    fprintf(debugFP, "board L=%d, R=%d, H=%d, holdings=%d\n", BOARD_LEFT, BOARD_RGHT, BOARD_HEIGHT, gameInfo.holdingsWidth);
    setbuf(debugFP, NULL);
  }
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
        {int i; for(i=0; i<BOARD_SIZE; i++) castlingRights[boardIndex+1][i] = castlingRights[boardIndex][i];}
	/* currentMoveString is set as a side-effect of yylex */
	strcpy(moveList[boardIndex], currentMoveString);
	strcat(moveList[boardIndex], "\n");
	boardIndex++;
	ApplyMove(fromX, fromY, toX, toY, promoChar, boards[boardIndex], 
					castlingRights[boardIndex], &epStatus[boardIndex]);
        switch (MateTest(boards[boardIndex], PosFlags(boardIndex),
                                 EP_UNKNOWN, castlingRights[boardIndex]) ) {
	  case MT_NONE:
	  case MT_STALEMATE:
	  default:
	    break;
	  case MT_CHECK:
            if(gameInfo.variant != VariantShogi)
                strcat(parseList[boardIndex - 1], "+");
	    break;
	  case MT_CHECKMATE:
	  case MT_STAINMATE:
	    strcat(parseList[boardIndex - 1], "#");
	    break;
	}
    }
}


/* Apply a move to the given board  */
void
ApplyMove(fromX, fromY, toX, toY, promoChar, board, castling, ep)
     int fromX, fromY, toX, toY;
     int promoChar;
     Board board;
     char *castling;
     char *ep;
{
  ChessSquare captured = board[toY][toX], piece, king; int p, oldEP = EP_NONE, berolina = 0;

    /* [HGM] compute & store e.p. status and castling rights for new position */
    /* we can always do that 'in place', now pointers to these rights are passed to ApplyMove */
    { int i;

      if(gameInfo.variant == VariantBerolina) berolina = EP_BEROLIN_A;
      oldEP = *ep;
      *ep = EP_NONE;

      if( board[toY][toX] != EmptySquare ) 
           *ep = EP_CAPTURE;  

      if( board[fromY][fromX] == WhitePawn ) {
           if(fromY != toY) // [HGM] Xiangqi sideway Pawn moves should not count as 50-move breakers
	       *ep = EP_PAWN_MOVE;
           if( toY-fromY==2) {
               if(toX>BOARD_LEFT   && board[toY][toX-1] == BlackPawn &&
			gameInfo.variant != VariantBerolina || toX < fromX)
	              *ep = toX | berolina;
               if(toX<BOARD_RGHT-1 && board[toY][toX+1] == BlackPawn &&
			gameInfo.variant != VariantBerolina || toX > fromX) 
	              *ep = toX;
	   }
      } else 
      if( board[fromY][fromX] == BlackPawn ) {
           if(fromY != toY) // [HGM] Xiangqi sideway Pawn moves should not count as 50-move breakers
	       *ep = EP_PAWN_MOVE; 
           if( toY-fromY== -2) {
               if(toX>BOARD_LEFT   && board[toY][toX-1] == WhitePawn &&
			gameInfo.variant != VariantBerolina || toX < fromX)
	              *ep = toX | berolina;
               if(toX<BOARD_RGHT-1 && board[toY][toX+1] == WhitePawn &&
			gameInfo.variant != VariantBerolina || toX > fromX) 
	              *ep = toX;
	   }
       }

       for(i=0; i<nrCastlingRights; i++) {
           if(castling[i] == fromX && castlingRank[i] == fromY ||
              castling[i] == toX   && castlingRank[i] == toY   
             ) castling[i] = -1; // revoke for moved or captured piece
       }

    }

  /* [HGM] In Shatranj and Courier all promotions are to Ferz */
  if((gameInfo.variant==VariantShatranj || gameInfo.variant==VariantCourier)
       && promoChar != 0) promoChar = PieceToChar(WhiteFerz);
         
  if (fromX == toX && fromY == toY) return;

  if (fromY == DROP_RANK) {
	/* must be first */
        piece = board[toY][toX] = (ChessSquare) fromX;
  } else {
     piece = board[fromY][fromX]; /* [HGM] remember, for Shogi promotion */
     king = piece < (int) BlackPawn ? WhiteKing : BlackKing; /* [HGM] Knightmate simplify testing for castling */
     if(gameInfo.variant == VariantKnightmate)
         king += (int) WhiteUnicorn - (int) WhiteKing;

    /* Code added by Tord: */
    /* FRC castling assumed when king captures friendly rook. */
    if (board[fromY][fromX] == WhiteKing &&
	     board[toY][toX] == WhiteRook) {
      board[fromY][fromX] = EmptySquare;
      board[toY][toX] = EmptySquare;
      if(toX > fromX) {
        board[0][BOARD_RGHT-2] = WhiteKing; board[0][BOARD_RGHT-3] = WhiteRook;
      } else {
        board[0][BOARD_LEFT+2] = WhiteKing; board[0][BOARD_LEFT+3] = WhiteRook;
      }
    } else if (board[fromY][fromX] == BlackKing &&
	       board[toY][toX] == BlackRook) {
      board[fromY][fromX] = EmptySquare;
      board[toY][toX] = EmptySquare;
      if(toX > fromX) {
        board[BOARD_HEIGHT-1][BOARD_RGHT-2] = BlackKing; board[BOARD_HEIGHT-1][BOARD_RGHT-3] = BlackRook;
      } else {
        board[BOARD_HEIGHT-1][BOARD_LEFT+2] = BlackKing; board[BOARD_HEIGHT-1][BOARD_LEFT+3] = BlackRook;
      }
    /* End of code added by Tord */

    } else if (board[fromY][fromX] == king
        && fromX != BOARD_LEFT && fromX != BOARD_RGHT-1 // [HGM] cylinder */
        && toY == fromY && toX > fromX+1) {
	board[fromY][fromX] = EmptySquare;
        board[toY][toX] = king;
        board[toY][toX-1] = board[fromY][BOARD_RGHT-1];
        board[fromY][BOARD_RGHT-1] = EmptySquare;
    } else if (board[fromY][fromX] == king
        && fromX != BOARD_LEFT && fromX != BOARD_RGHT-1 // [HGM] cylinder */
               && toY == fromY && toX < fromX-1) {
	board[fromY][fromX] = EmptySquare;
        board[toY][toX] = king;
        board[toY][toX+1] = board[fromY][BOARD_LEFT];
        board[fromY][BOARD_LEFT] = EmptySquare;
    } else if (board[fromY][fromX] == WhitePawn
               && toY == BOARD_HEIGHT-1
               && gameInfo.variant != VariantXiangqi
               ) {
	/* white pawn promotion */
        board[toY][toX] = CharToPiece(ToUpper(promoChar));
        if (board[toY][toX] == EmptySquare) {
            board[toY][toX] = WhiteQueen;
	}
        if(gameInfo.variant==VariantBughouse ||
           gameInfo.variant==VariantCrazyhouse) /* [HGM] use shadow piece */
            board[toY][toX] = (ChessSquare) (PROMOTED board[toY][toX]);
	board[fromY][fromX] = EmptySquare;
    } else if ((fromY == BOARD_HEIGHT-4)
	       && (toX != fromX)
               && gameInfo.variant != VariantXiangqi
               && gameInfo.variant != VariantBerolina
	       && (board[fromY][fromX] == WhitePawn)
	       && (board[toY][toX] == EmptySquare)) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = WhitePawn;
	captured = board[toY - 1][toX];
	board[toY - 1][toX] = EmptySquare;
    } else if ((fromY == BOARD_HEIGHT-4)
	       && (toX == fromX)
               && gameInfo.variant == VariantBerolina
	       && (board[fromY][fromX] == WhitePawn)
	       && (board[toY][toX] == EmptySquare)) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = WhitePawn;
	if(oldEP & EP_BEROLIN_A) {
		captured = board[fromY][fromX-1];
		board[fromY][fromX-1] = EmptySquare;
	}else{	captured = board[fromY][fromX+1];
		board[fromY][fromX+1] = EmptySquare;
	}
    } else if (board[fromY][fromX] == king
        && fromX != BOARD_LEFT && fromX != BOARD_RGHT-1 // [HGM] cylinder */
               && toY == fromY && toX > fromX+1) {
	board[fromY][fromX] = EmptySquare;
        board[toY][toX] = king;
        board[toY][toX-1] = board[fromY][BOARD_RGHT-1];
        board[fromY][BOARD_RGHT-1] = EmptySquare;
    } else if (board[fromY][fromX] == king
        && fromX != BOARD_LEFT && fromX != BOARD_RGHT-1 // [HGM] cylinder */
               && toY == fromY && toX < fromX-1) {
	board[fromY][fromX] = EmptySquare;
        board[toY][toX] = king;
        board[toY][toX+1] = board[fromY][BOARD_LEFT];
        board[fromY][BOARD_LEFT] = EmptySquare;
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
	       && toY == 0
               && gameInfo.variant != VariantXiangqi
               ) {
	/* black pawn promotion */
	board[0][toX] = CharToPiece(ToLower(promoChar));
	if (board[0][toX] == EmptySquare) {
	    board[0][toX] = BlackQueen;
	}
        if(gameInfo.variant==VariantBughouse ||
           gameInfo.variant==VariantCrazyhouse) /* [HGM] use shadow piece */
            board[toY][toX] = (ChessSquare) (PROMOTED board[toY][toX]);
	board[fromY][fromX] = EmptySquare;
    } else if ((fromY == 3)
	       && (toX != fromX)
               && gameInfo.variant != VariantXiangqi
               && gameInfo.variant != VariantBerolina
	       && (board[fromY][fromX] == BlackPawn)
	       && (board[toY][toX] == EmptySquare)) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = BlackPawn;
	captured = board[toY + 1][toX];
	board[toY + 1][toX] = EmptySquare;
    } else if ((fromY == 3)
	       && (toX == fromX)
               && gameInfo.variant == VariantBerolina
	       && (board[fromY][fromX] == BlackPawn)
	       && (board[toY][toX] == EmptySquare)) {
	board[fromY][fromX] = EmptySquare;
	board[toY][toX] = BlackPawn;
	if(oldEP & EP_BEROLIN_A) {
		captured = board[fromY][fromX-1];
		board[fromY][fromX-1] = EmptySquare;
	}else{	captured = board[fromY][fromX+1];
		board[fromY][fromX+1] = EmptySquare;
	}
    } else {
	board[toY][toX] = board[fromY][fromX];
	board[fromY][fromX] = EmptySquare;
    }

    /* [HGM] now we promote for Shogi, if needed */
    if(gameInfo.variant == VariantShogi && promoChar == 'q')
        board[toY][toX] = (ChessSquare) (PROMOTED piece);
  }

    if (gameInfo.holdingsWidth != 0) {

      /* !!A lot more code needs to be written to support holdings  */
      /* [HGM] OK, so I have written it. Holdings are stored in the */
      /* penultimate board files, so they are automaticlly stored   */
      /* in the game history.                                       */
      if (fromY == DROP_RANK) {
        /* Delete from holdings, by decreasing count */
        /* and erasing image if necessary            */
        p = (int) fromX;
        if(p < (int) BlackPawn) { /* white drop */
             p -= (int)WhitePawn;
             if(p >= gameInfo.holdingsSize) p = 0;
             if(--board[p][BOARD_WIDTH-2] == 0)
                  board[p][BOARD_WIDTH-1] = EmptySquare;
        } else {                  /* black drop */
             p -= (int)BlackPawn;
             if(p >= gameInfo.holdingsSize) p = 0;
             if(--board[BOARD_HEIGHT-1-p][1] == 0)
                  board[BOARD_HEIGHT-1-p][0] = EmptySquare;
        }
      }
      if (captured != EmptySquare && gameInfo.holdingsSize > 0
          && gameInfo.variant != VariantBughouse        ) {
        /* [HGM] holdings: Add to holdings, if holdings exist */
	if(gameInfo.variant == VariantSuper || gameInfo.variant == VariantGreat) { 
		// [HGM] superchess: suppress flipping color of captured pieces by reverse pre-flip
		captured = (int) captured >= (int) BlackPawn ? BLACK_TO_WHITE captured : WHITE_TO_BLACK captured;
	}
        p = (int) captured;
        if (p >= (int) BlackPawn) {
          p -= (int)BlackPawn;
          if(gameInfo.variant == VariantShogi && DEMOTED p >= 0) {
                  /* in Shogi restore piece to its original  first */
                  captured = (ChessSquare) (DEMOTED captured);
                  p = DEMOTED p;
          }
          p = PieceToNumber((ChessSquare)p);
          if(p >= gameInfo.holdingsSize) { p = 0; captured = BlackPawn; }
          board[p][BOARD_WIDTH-2]++;
          board[p][BOARD_WIDTH-1] = BLACK_TO_WHITE captured;
	} else {
          p -= (int)WhitePawn;
          if(gameInfo.variant == VariantShogi && DEMOTED p >= 0) {
                  captured = (ChessSquare) (DEMOTED captured);
                  p = DEMOTED p;
          }
          p = PieceToNumber((ChessSquare)p);
          if(p >= gameInfo.holdingsSize) { p = 0; captured = WhitePawn; }
          board[BOARD_HEIGHT-1-p][1]++;
          board[BOARD_HEIGHT-1-p][0] = WHITE_TO_BLACK captured;
	}
      }

    } else if (gameInfo.variant == VariantAtomic) {
      if (captured != EmptySquare) {
	int y, x;
	for (y = toY-1; y <= toY+1; y++) {
	  for (x = toX-1; x <= toX+1; x++) {
            if (y >= 0 && y < BOARD_HEIGHT && x >= BOARD_LEFT && x < BOARD_RGHT &&
		board[y][x] != WhitePawn && board[y][x] != BlackPawn) {
	      board[y][x] = EmptySquare;
	    }
	  }
	}
	board[toY][toX] = EmptySquare;
      }
    }
    if(gameInfo.variant == VariantShogi && promoChar != NULLCHAR && promoChar != '=') {
        /* [HGM] Shogi promotions */
        board[toY][toX] = (ChessSquare) (PROMOTED piece);
    }

    if((gameInfo.variant == VariantSuper || gameInfo.variant == VariantGreat) 
		&& promoChar != NULLCHAR && gameInfo.holdingsSize) { 
	// [HGM] superchess: take promotion piece out of holdings
	int k = PieceToNumber(CharToPiece(ToUpper(promoChar)));
	if((int)piece < (int)BlackPawn) { // determine stm from piece color
	    if(!--board[k][BOARD_WIDTH-2])
		board[k][BOARD_WIDTH-1] = EmptySquare;
	} else {
	    if(!--board[BOARD_HEIGHT-1-k][1])
		board[BOARD_HEIGHT-1-k][0] = EmptySquare;
	}
    }

}

/* Updates forwardMostMove */
void
MakeMove(fromX, fromY, toX, toY, promoChar)
     int fromX, fromY, toX, toY;
     int promoChar;
{
//    forwardMostMove++; // [HGM] bare: moved downstream

    if(serverMoves != NULL) { /* [HGM] write moves on file for broadcasting (should be separate routine, really) */
        int timeLeft; static int lastLoadFlag=0; int king, piece;
        piece = boards[forwardMostMove][fromY][fromX];
        king = piece < (int) BlackPawn ? WhiteKing : BlackKing;
        if(gameInfo.variant == VariantKnightmate)
            king += (int) WhiteUnicorn - (int) WhiteKing;
        if(forwardMostMove == 0) {
            if(blackPlaysFirst) 
                fprintf(serverMoves, "%s;", second.tidy);
            fprintf(serverMoves, "%s;", first.tidy);
            if(!blackPlaysFirst) 
                fprintf(serverMoves, "%s;", second.tidy);
        } else fprintf(serverMoves, loadFlag|lastLoadFlag ? ":" : ";");
        lastLoadFlag = loadFlag;
        // print base move
        fprintf(serverMoves, "%c%c:%c%c", AAA+fromX, ONE+fromY, AAA+toX, ONE+toY);
        // print castling suffix
        if( toY == fromY && piece == king ) {
            if(toX-fromX > 1)
                fprintf(serverMoves, ":%c%c:%c%c", AAA+BOARD_RGHT-1, ONE+fromY, AAA+toX-1,ONE+toY);
            if(fromX-toX >1)
                fprintf(serverMoves, ":%c%c:%c%c", AAA+BOARD_LEFT, ONE+fromY, AAA+toX+1,ONE+toY);
        }
        // e.p. suffix
        if( (boards[forwardMostMove][fromY][fromX] == WhitePawn ||
             boards[forwardMostMove][fromY][fromX] == BlackPawn   ) &&
             boards[forwardMostMove][toY][toX] == EmptySquare
             && fromX != toX )
                fprintf(serverMoves, ":%c%c:%c%c", AAA+fromX, ONE+fromY, AAA+toX, ONE+fromY);
        // promotion suffix
        if(promoChar != NULLCHAR)
                fprintf(serverMoves, ":%c:%c%c", promoChar, AAA+toX, ONE+toY);
        if(!loadFlag) {
            fprintf(serverMoves, "/%d/%d",
               pvInfoList[forwardMostMove].depth, pvInfoList[forwardMostMove].score);
            if(forwardMostMove+1 & 1) timeLeft = whiteTimeRemaining/1000;
            else                      timeLeft = blackTimeRemaining/1000;
            fprintf(serverMoves, "/%d", timeLeft);
        }
        fflush(serverMoves);
    }

    if (forwardMostMove+1 >= MAX_MOVES) {
      DisplayFatalError(_("Game too long; increase MAX_MOVES and recompile"),
			0, 1);
      return;
    }
    SwitchClocks();
    timeRemaining[0][forwardMostMove+1] = whiteTimeRemaining;
    timeRemaining[1][forwardMostMove+1] = blackTimeRemaining;
    if (commentList[forwardMostMove+1] != NULL) {
	free(commentList[forwardMostMove+1]);
	commentList[forwardMostMove+1] = NULL;
    }
    CopyBoard(boards[forwardMostMove+1], boards[forwardMostMove]);
    {int i; for(i=0; i<BOARD_SIZE; i++) castlingRights[forwardMostMove+1][i] = castlingRights[forwardMostMove][i];}
    ApplyMove(fromX, fromY, toX, toY, promoChar, boards[forwardMostMove+1], 
				castlingRights[forwardMostMove+1], &epStatus[forwardMostMove+1]);
    forwardMostMove++; // [HGM] bare: moved to after ApplyMove, to make sure clock interrupt finds complete board
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
    switch (MateTest(boards[forwardMostMove], PosFlags(forwardMostMove),
                       epStatus[forwardMostMove], /* [HGM] use true e.p. */
                            castlingRights[forwardMostMove]) ) {
      case MT_NONE:
      case MT_STALEMATE:
      default:
	break;
      case MT_CHECK:
        if(gameInfo.variant != VariantShogi)
            strcat(parseList[forwardMostMove - 1], "+");
	break;
      case MT_CHECKMATE:
      case MT_STAINMATE:
	strcat(parseList[forwardMostMove - 1], "#");
	break;
    }
    if (appData.debugMode) {
        fprintf(debugFP, "move: %s, parse: %s (%c)\n", moveList[forwardMostMove-1], parseList[forwardMostMove-1], moveList[forwardMostMove-1][4]);
    }

}

/* Updates currentMove if not pausing */
void
ShowMove(fromX, fromY, toX, toY)
{
    int instant = (gameMode == PlayFromGameFile) ?
	(matchMode || (appData.timeDelay == 0 && !pausing)) : pausing;
    if(appData.noGUI) return;
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

void SendEgtPath(ChessProgramState *cps)
{       /* [HGM] EGT: match formats given in feature with those given by user, and send info for each match */
	char buf[MSG_SIZ], name[MSG_SIZ], *p;

	if((p = cps->egtFormats) == NULL || appData.egtFormats == NULL) return;

	while(*p) {
	    char c, *q = name+1, *r, *s;

	    name[0] = ','; // extract next format name from feature and copy with prefixed ','
	    while(*p && *p != ',') *q++ = *p++;
	    *q++ = ':'; *q = 0;
	    if( appData.defaultPathEGTB && appData.defaultPathEGTB[0] && 
		strcmp(name, ",nalimov:") == 0 ) {
		// take nalimov path from the menu-changeable option first, if it is defined
		sprintf(buf, "egtpath nalimov %s\n", appData.defaultPathEGTB);
		SendToProgram(buf,cps);     // send egtbpath command for nalimov
	    } else
	    if( (s = StrStr(appData.egtFormats, name+1)) == appData.egtFormats ||
		(s = StrStr(appData.egtFormats, name)) != NULL) {
		// format name occurs amongst user-supplied formats, at beginning or immediately after comma
		s = r = StrStr(s, ":") + 1; // beginning of path info
		while(*r && *r != ',') r++; // path info is everything upto next ';' or end of string
		c = *r; *r = 0;             // temporarily null-terminate path info
		    *--q = 0;               // strip of trailig ':' from name
		    sprintf(buf, "egtbpath %s %s\n", name+1, s);
		*r = c;
		SendToProgram(buf,cps);     // send egtbpath command for this format
	    }
	    if(*p == ',') p++; // read away comma to position for next format name
	}
}

void
InitChessProgram(cps, setup)
     ChessProgramState *cps;
     int setup; /* [HGM] needed to setup FRC opening position */
{
    char buf[MSG_SIZ], b[MSG_SIZ]; int overruled;
    if (appData.noChessProgram) return;
    hintRequested = FALSE;
    bookRequested = FALSE;

    /* [HGM] some new WB protocol commands to configure engine are sent now, if engine supports them */
    /*       moved to before sending initstring in 4.3.15, so Polyglot can delay UCI 'isready' to recepton of 'new' */
    if(cps->memSize) { /* [HGM] memory */
	sprintf(buf, "memory %d\n", appData.defaultHashSize + appData.defaultCacheSizeEGTB);
	SendToProgram(buf, cps);
    }
    SendEgtPath(cps); /* [HGM] EGT */
    if(cps->maxCores) { /* [HGM] SMP: (protocol specified must be last settings command before new!) */
	sprintf(buf, "cores %d\n", appData.smpCores);
	SendToProgram(buf, cps);
    }

    SendToProgram(cps->initString, cps);
    if (gameInfo.variant != VariantNormal &&
	gameInfo.variant != VariantLoadable
        /* [HGM] also send variant if board size non-standard */
        || gameInfo.boardWidth != 8 || gameInfo.boardHeight != 8 || gameInfo.holdingsSize != 0
                                            ) {
      char *v = VariantName(gameInfo.variant);
      if (cps->protocolVersion != 1 && StrStr(cps->variants, v) == NULL) {
        /* [HGM] in protocol 1 we have to assume all variants valid */
	sprintf(buf, _("Variant %s not supported by %s"), v, cps->tidy);
	DisplayFatalError(buf, 0, 1);
	return;
      }

      /* [HGM] make prefix for non-standard board size. Awkward testing... */
      overruled = gameInfo.boardWidth != 8 || gameInfo.boardHeight != 8 || gameInfo.holdingsSize != 0;
      if( gameInfo.variant == VariantXiangqi )
           overruled = gameInfo.boardWidth != 9 || gameInfo.boardHeight != 10 || gameInfo.holdingsSize != 0;
      if( gameInfo.variant == VariantShogi )
           overruled = gameInfo.boardWidth != 9 || gameInfo.boardHeight != 9 || gameInfo.holdingsSize != 7;
      if( gameInfo.variant == VariantBughouse || gameInfo.variant == VariantCrazyhouse )
           overruled = gameInfo.boardWidth != 8 || gameInfo.boardHeight != 8 || gameInfo.holdingsSize != 5;
      if( gameInfo.variant == VariantCapablanca || gameInfo.variant == VariantCapaRandom || 
                               gameInfo.variant == VariantGothic  || gameInfo.variant == VariantFalcon )
           overruled = gameInfo.boardWidth != 10 || gameInfo.boardHeight != 8 || gameInfo.holdingsSize != 0;
      if( gameInfo.variant == VariantCourier )
           overruled = gameInfo.boardWidth != 12 || gameInfo.boardHeight != 8 || gameInfo.holdingsSize != 0;
      if( gameInfo.variant == VariantSuper )
           overruled = gameInfo.boardWidth != 8 || gameInfo.boardHeight != 8 || gameInfo.holdingsSize != 8;
      if( gameInfo.variant == VariantGreat )
           overruled = gameInfo.boardWidth != 10 || gameInfo.boardHeight != 8 || gameInfo.holdingsSize != 8;

      if(overruled) {
           sprintf(b, "%dx%d+%d_%s", gameInfo.boardWidth, gameInfo.boardHeight, 
                               gameInfo.holdingsSize, VariantName(gameInfo.variant)); // cook up sized variant name
           /* [HGM] varsize: try first if this defiant size variant is specifically known */
           if(StrStr(cps->variants, b) == NULL) { 
               // specific sized variant not known, check if general sizing allowed
               if (cps->protocolVersion != 1) { // for protocol 1 we cannot check and hope for the best
                   if(StrStr(cps->variants, "boardsize") == NULL) {
                       sprintf(buf, "Board size %dx%d+%d not supported by %s",
                            gameInfo.boardWidth, gameInfo.boardHeight, gameInfo.holdingsSize, cps->tidy);
                       DisplayFatalError(buf, 0, 1);
                       return;
                   }
                   /* [HGM] here we really should compare with the maximum supported board size */
               }
           }
      } else sprintf(b, "%s", VariantName(gameInfo.variant));
      sprintf(buf, "variant %s\n", b);
      SendToProgram(buf, cps);
    }
    currentlyInitializedVariant = gameInfo.variant;

    /* [HGM] send opening position in FRC to first engine */
    if(setup) {
          SendToProgram("force\n", cps);
          SendBoard(cps, 0);
          /* engine is now in force mode! Set flag to wake it up after first move. */
          setboardSpoiledMachineBlack = 1;
    }

    if (cps->sendICS) {
      snprintf(buf, sizeof(buf), "ics %s\n", appData.icsActive ? appData.icsHost : "-");
      SendToProgram(buf, cps);
    }
    cps->maybeThinking = FALSE;
    cps->offeredDraw = 0;
    if (!appData.icsActive) {
	SendTimeControl(cps, movesPerSession, timeControl,
			timeIncrement, appData.searchDepth,
			searchTime);
    }
    if (appData.showThinking 
	// [HGM] thinking: four options require thinking output to be sent
	|| !appData.hideThinkingFromHuman || appData.adjudicateLossThreshold != 0 || EngineOutputIsUp()
				) {
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
	  snprintf(buf, sizeof(buf), "%s %s %s", appData.remoteShell, cps->host,
		    cps->program);
	} else {
	  snprintf(buf, sizeof(buf), "%s %s -l %s %s", appData.remoteShell,
		    cps->host, appData.remoteUser, cps->program);
	}
	err = StartChildProcess(buf, "", &cps->pr);
    }
    
    if (err != 0) {
	sprintf(buf, _("Startup failure on '%s'"), cps->program);
	DisplayFatalError(buf, err, 1);
	cps->pr = NoProc;
	cps->isr = NULL;
	return;
    }
    
    cps->isr = AddInputSource(cps->pr, TRUE, ReceiveFromProgram, cps);
    if (cps->protocolVersion > 1) {
      sprintf(buf, "xboard\nprotover %d\n", cps->protocolVersion);
      cps->nrOptions = 0; // [HGM] options: clear all engine-specific options
      cps->comboCnt = 0;  //                and values of combo boxes
      SendToProgram(buf, cps);
    } else {
      SendToProgram("xboard\n", cps);
    }
}


void
TwoMachinesEventIfReady P((void))
{
  if (first.lastPing != first.lastPong) {
    DisplayMessage("", _("Waiting for first chess program"));
    ScheduleDelayedEvent(TwoMachinesEventIfReady, 10); // [HGM] fast: lowered from 1000
    return;
  }
  if (second.lastPing != second.lastPong) {
    DisplayMessage("", _("Waiting for second chess program"));
    ScheduleDelayedEvent(TwoMachinesEventIfReady, 10); // [HGM] fast: lowered from 1000
    return;
  }
  ThawUI();
  TwoMachinesEvent();
}

void
NextMatchGame P((void))
{
    int index; /* [HGM] autoinc: step lod index during match */
    Reset(FALSE, TRUE);
    if (*appData.loadGameFile != NULLCHAR) {
	index = appData.loadGameIndex;
	if(index < 0) { // [HGM] autoinc
	    lastIndex = index = (index == -2 && first.twoMachinesColor[0] == 'b') ? lastIndex : lastIndex+1;
	    if(appData.rewindIndex > 0 && index > appData.rewindIndex) lastIndex = index = 1;
	} 
	LoadGameFromFile(appData.loadGameFile,
			 index,
			 appData.loadGameFile, FALSE);
    } else if (*appData.loadPositionFile != NULLCHAR) {
	index = appData.loadPositionIndex;
	if(index < 0) { // [HGM] autoinc
	    lastIndex = index = (index == -2 && first.twoMachinesColor[0] == 'b') ? lastIndex : lastIndex+1;
	    if(appData.rewindIndex > 0 && index > appData.rewindIndex) lastIndex = index = 1;
	} 
	LoadPositionFromFile(appData.loadPositionFile,
			     index,
			     appData.loadPositionFile);
    }
    TwoMachinesEventIfReady();
}

void UserAdjudicationEvent( int result )
{
    ChessMove gameResult = GameIsDrawn;

    if( result > 0 ) {
        gameResult = WhiteWins;
    }
    else if( result < 0 ) {
        gameResult = BlackWins;
    }

    if( gameMode == TwoMachinesPlay ) {
        GameEnds( gameResult, "User adjudication", GE_XBOARD );
    }
}


// [HGM] save: calculate checksum of game to make games easily identifiable
int StringCheckSum(char *s)
{
	int i = 0;
	if(s==NULL) return 0;
	while(*s) i = i*259 + *s++;
	return i;
}

int GameCheckSum()
{
	int i, sum=0;
	for(i=backwardMostMove; i<forwardMostMove; i++) {
		sum += pvInfoList[i].depth;
		sum += StringCheckSum(parseList[i]);
		sum += StringCheckSum(commentList[i]);
		sum *= 261;
	}
	if(i>1 && sum==0) sum++; // make sure never zero for non-empty game
	return sum + StringCheckSum(commentList[i]);
} // end of save patch

void
GameEnds(result, resultDetails, whosays)
     ChessMove result;
     char *resultDetails;
     int whosays;
{
    GameMode nextGameMode;
    int isIcsGame;
    char buf[MSG_SIZ];

    if(endingGame) return; /* [HGM] crash: forbid recursion */
    endingGame = 1;

    if (appData.debugMode) {
      fprintf(debugFP, "GameEnds(%d, %s, %d)\n",
	      result, resultDetails ? resultDetails : "(null)", whosays);
    }

    if (appData.icsActive && (whosays == GE_ENGINE || whosays >= GE_ENGINE1)) {
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
	endingGame = 0; /* [HGM] crash */
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
    
        /* [HGM] if a machine claims the game end we verify this claim */
        if(gameMode == TwoMachinesPlay && appData.testClaims) {
	    if(appData.testLegality && whosays >= GE_ENGINE1 ) {
                char claimer;
		ChessMove trueResult = (ChessMove) -1;

                claimer = whosays == GE_ENGINE1 ?      /* color of claimer */
                                            first.twoMachinesColor[0] :
                                            second.twoMachinesColor[0] ;

		// [HGM] losers: because the logic is becoming a bit hairy, determine true result first
		if(epStatus[forwardMostMove] == EP_CHECKMATE) {
		    /* [HGM] verify: engine mate claims accepted if they were flagged */
		    trueResult = WhiteOnMove(forwardMostMove) ? BlackWins : WhiteWins;
		} else
		if(epStatus[forwardMostMove] == EP_WINS) { // added code for games where being mated is a win
		    /* [HGM] verify: engine mate claims accepted if they were flagged */
		    trueResult = WhiteOnMove(forwardMostMove) ? WhiteWins : BlackWins;
		} else
		if(epStatus[forwardMostMove] == EP_STALEMATE) { // only used to indicate draws now
		    trueResult = GameIsDrawn; // default; in variants where stalemate loses, Status is CHECKMATE
		}

		// now verify win claims, but not in drop games, as we don't understand those yet
                if( (gameInfo.holdingsWidth == 0 || gameInfo.variant == VariantSuper
						 || gameInfo.variant == VariantGreat) &&
                    (result == WhiteWins && claimer == 'w' ||
                     result == BlackWins && claimer == 'b'   ) ) { // case to verify: engine claims own win
		      if (appData.debugMode) {
	 		fprintf(debugFP, "result=%d sp=%d move=%d\n",
		 		result, epStatus[forwardMostMove], forwardMostMove);
		      }
		      if(result != trueResult) {
	                      sprintf(buf, "False win claim: '%s'", resultDetails);
	                      result = claimer == 'w' ? BlackWins : WhiteWins;
	                      resultDetails = buf;
		      }
                } else
                if( result == GameIsDrawn && epStatus[forwardMostMove] > EP_DRAWS
                    && (forwardMostMove <= backwardMostMove ||
                        epStatus[forwardMostMove-1] > EP_DRAWS ||
                        (claimer=='b')==(forwardMostMove&1))
                                                                                  ) {
                      /* [HGM] verify: draws that were not flagged are false claims */
                      sprintf(buf, "False draw claim: '%s'", resultDetails);
                      result = claimer == 'w' ? BlackWins : WhiteWins;
                      resultDetails = buf;
                }
                /* (Claiming a loss is accepted no questions asked!) */
	    }
	    /* [HGM] bare: don't allow bare King to win */
	    if((gameInfo.holdingsWidth == 0 || gameInfo.variant == VariantSuper || gameInfo.variant == VariantGreat)
	       && gameInfo.variant != VariantLosers && gameInfo.variant != VariantGiveaway 
	       && gameInfo.variant != VariantSuicide // [HGM] losers: except in losers, of course...
	       && result != GameIsDrawn)
	    {   int i, j, k=0, color = (result==WhiteWins ? (int)WhitePawn : (int)BlackPawn);
		for(j=BOARD_LEFT; j<BOARD_RGHT; j++) for(i=0; i<BOARD_HEIGHT; i++) {
			int p = (int)boards[forwardMostMove][i][j] - color;
			if(p >= 0 && p <= (int)WhiteKing) k++;
		}
		if (appData.debugMode) {
	 	     fprintf(debugFP, "GE(%d, %s, %d) bare king k=%d color=%d\n",
		 	result, resultDetails ? resultDetails : "(null)", whosays, k, color);
		}
		if(k <= 1) {
			result = GameIsDrawn;
			sprintf(buf, "%s but bare king", resultDetails);
			resultDetails = buf;
		}
	    }
        }


        if(serverMoves != NULL && !loadFlag) { char c = '=';
            if(result==WhiteWins) c = '+';
            if(result==BlackWins) c = '-';
            if(resultDetails != NULL)
                fprintf(serverMoves, ";%c;%s\n", c, resultDetails);
        }
 	if (resultDetails != NULL) {
	    gameInfo.result = result;
	    gameInfo.resultDetails = StrSave(resultDetails);

	    /* display last move only if game was not loaded from file */
	    if ((whosays != GE_FILE) && (currentMove == forwardMostMove))
		DisplayMove(currentMove - 1);
    
	    if (forwardMostMove != 0) {
		if (gameMode != PlayFromGameFile && gameMode != EditGame
		    && lastSavedGame != GameCheckSum() // [HGM] save: suppress duplicates
								) {
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

	    /* Tell program how game ended in case it is learning */
            /* [HGM] Moved this to after saving the PGN, just in case */
            /* engine died and we got here through time loss. In that */
            /* case we will get a fatal error writing the pipe, which */
            /* would otherwise lose us the PGN.                       */
            /* [HGM] crash: not needed anymore, but doesn't hurt;     */
            /* output during GameEnds should never be fatal anymore   */
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
	endingGame = 0; /* [HGM] crash */
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
            DoSleep( appData.delayBeforeQuit );
	    SendToProgram("quit\n", &first);
            DoSleep( appData.delayAfterQuit );
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
            DoSleep( appData.delayBeforeQuit );
	    SendToProgram("quit\n", &second);
            DoSleep( appData.delayAfterQuit );
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
	    if(appData.sameColorGames <= 1) { /* [HGM] alternate: suppress color swap */
		tmp = first.twoMachinesColor;
		first.twoMachinesColor = second.twoMachinesColor;
		second.twoMachinesColor = tmp;
	    }
	    gameMode = nextGameMode;
	    matchGame++;
            if(appData.matchPause>10000 || appData.matchPause<10)
                appData.matchPause = 10000; /* [HGM] make pause adjustable */
            ScheduleDelayedEvent(NextMatchGame, appData.matchPause);
	    endingGame = 0; /* [HGM] crash */
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
    endingGame = 0;  /* [HGM] crash */
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
    if(currentlyInitializedVariant != gameInfo.variant) { char buf[MSG_SIZ];
        // [HGM] variantswitch: make engine aware of new variant
	if(cps->protocolVersion > 1 && StrStr(cps->variants, VariantName(gameInfo.variant)) == NULL)
		return; // [HGM] refrain from feeding moves altogether if variant is unsupported!
	sprintf(buf, "variant %s\n", VariantName(gameInfo.variant));
	SendToProgram(buf, cps);
        currentlyInitializedVariant = gameInfo.variant;
    }
    SendToProgram("force\n", cps);
    if (startedFromSetupPosition) {
	SendBoard(cps, backwardMostMove);
    if (appData.debugMode) {
        fprintf(debugFP, "feedMoves\n");
    }
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
    InitChessProgram(&first, FALSE);
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
    first.bookSuspend = FALSE; // [HGM] book
    second.bookSuspend = FALSE;
    thinkOutput[0] = NULLCHAR;
    lastHint[0] = NULLCHAR;
    ClearGameInfo(&gameInfo);
    gameInfo.variant = StringToVariant(appData.variant);
    ics_user_moved = ics_clock_paused = FALSE;
    ics_getting_history = H_FALSE;
    ics_gamenum = -1;
    white_holding[0] = black_holding[0] = NULLCHAR;
    ClearProgramStats();
    opponentKibitzes = FALSE; // [HGM] kibitz: do not reserve space in engine-output window in zippy mode
    
    ResetFrontEnd();
    ClearHighlights();
    flipView = appData.flipView;
    ClearPremoveHighlights();
    gotPremove = FALSE;
    alarmSounded = FALSE;

    GameEnds((ChessMove) 0, NULL, GE_PLAYER);
    if(appData.serverMovesName != NULL) {
        /* [HGM] prepare to make moves file for broadcasting */
        clock_t t = clock();
        if(serverMoves != NULL) fclose(serverMoves);
        serverMoves = fopen(appData.serverMovesName, "r");
        if(serverMoves != NULL) {
            fclose(serverMoves);
            /* delay 15 sec before overwriting, so all clients can see end */
            while(clock()-t < appData.serverPause*CLOCKS_PER_SEC);
        }
        serverMoves = fopen(appData.serverMovesName, "w");
    }

    ExitAnalyzeMode();
    gameMode = BeginningOfGame;
    ModeHighlight();
    if(appData.icsActive) gameInfo.variant = VariantNormal;
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
    if (init) {
	    InitChessProgram(&first, startedFromSetupPosition);
    }
    DisplayTitle("");
    DisplayMessage("", "");
    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);
    lastSavedGame = 0; // [HGM] save: make sure next game counts as unsaved
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

      /* [AS] Clear current move marker at the end of a game */
      /* HistorySet(parseList, backwardMostMove, forwardMostMove, -1); */

      return FALSE;
    }
    
    toX = moveList[currentMove][2] - AAA;
    toY = moveList[currentMove][3] - ONE;

    if (moveList[currentMove][1] == '@') {
	if (appData.highlightLastMove) {
	    SetHighlights(-1, -1, toX, toY);
	}
    } else {
        fromX = moveList[currentMove][0] - AAA;
        fromY = moveList[currentMove][1] - ONE;

        HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove); /* [AS] */

	AnimateMove(boards[currentMove], fromX, fromY, toX, toY);

	if (appData.highlightLastMove) {
	    SetHighlights(fromX, fromY, toX, toY);
	}
    }
    DisplayMove(currentMove);
    SendMoveToProgram(currentMove++, &first);
    DisplayBothClocks();
    DrawPosition(FALSE, boards[currentMove]);
    // [HGM] PV info: always display, routine tests if empty
    DisplayComment(currentMove - 1, commentList[currentMove]);
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
      case WhitePromotionChancellor:
      case BlackPromotionChancellor:
      case WhitePromotionArchbishop:
      case BlackPromotionArchbishop:
      case WhitePromotionCentaur:
      case BlackPromotionCentaur:
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
      /* PUSH Fabien */
      case WhiteHSideCastleFR:
      case WhiteASideCastleFR:
      case BlackHSideCastleFR:
      case BlackASideCastleFR:
      /* POP Fabien */
	if (appData.debugMode)
	  fprintf(debugFP, "Parsed %s into %s\n", yy_text, currentMoveString);
        fromX = currentMoveString[0] - AAA;
        fromY = currentMoveString[1] - ONE;
        toX = currentMoveString[2] - AAA;
        toY = currentMoveString[3] - ONE;
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
        toX = currentMoveString[2] - AAA;
        toY = currentMoveString[3] - ONE;
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
                         EP_UNKNOWN, castlingRights[currentMove]) ) {
	  case MT_NONE:
	  case MT_CHECK:
	    break;
	  case MT_CHECKMATE:
	  case MT_STAINMATE:
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
                         EP_UNKNOWN, castlingRights[currentMove]) ) {
	  case MT_NONE:
	  case MT_CHECK:
	    break;
	  case MT_CHECKMATE:
	  case MT_STAINMATE:
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
            fromX = currentMoveString[0] - AAA;
            fromY = currentMoveString[1] - ONE;
            toX = currentMoveString[2] - AAA;
            toY = currentMoveString[3] - ONE;
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
	  fprintf(debugFP, "Parsed ImpossibleMove (type = %d): %s\n", moveType, yy_text);
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
            if (!appData.matchMode) // [HGM] PV info: routine tests if empty
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
	  snprintf(buf, sizeof(buf),  _("Can't open \"%s\""), filename);
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
            fromX = cmailMove[lastLoadGameNumber - 1][0] - AAA;
            fromY = cmailMove[lastLoadGameNumber - 1][1] - ONE;
            toX = cmailMove[lastLoadGameNumber - 1][2] - AAA;
            toY = cmailMove[lastLoadGameNumber - 1][3] - ONE;
	    promoChar = cmailMove[lastLoadGameNumber - 1][4];
	    MakeMove(fromX, fromY, toX, toY, promoChar);
	    ShowMove(fromX, fromY, toX, toY);
	      
	    switch (MateTest(boards[currentMove], PosFlags(currentMove),
                             EP_UNKNOWN, castlingRights[currentMove]) ) {
	      case MT_NONE:
	      case MT_CHECK:
		break;
    		
	      case MT_CHECKMATE:
	      case MT_STAINMATE:
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
    VariantClass oldVariant = gameInfo.variant; /* [HGM] PGNvariant */

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
      snprintf(buf, sizeof(buf), "%s vs. %s", lg->gameInfo.white,
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

        /* [HGM] PGNvariant: automatically switch to variant given in PGN tag */
        if(gameInfo.variant != oldVariant) {
            startedFromPositionFile = FALSE; /* [HGM] loadPos: variant switch likely makes position invalid */
	    InitPosition(TRUE);
            oldVariant = gameInfo.variant;
	    if (appData.debugMode) 
	      fprintf(debugFP, "New variant %d\n", (int) oldVariant);
        }


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
          /* [HGM] copy FEN attributes as well. Bugfix 4.3.14m and 4.3.15e: moved to after 'blackPlaysFirst' */
          {   int i;
              initialRulePlies = FENrulePlies;
              epStatus[forwardMostMove] = FENepStatus;
              for( i=0; i< nrCastlingRights; i++ )
                  initialRights[i] = castlingRights[forwardMostMove][i] = FENcastlingRights[i];
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
          if( appData.autoDisplayTags ) {
	    tags = PGNTags(&gameInfo);
	    TagsPopUp(tags, CmailMsg());
	    free(tags);
          }
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
            for (i = BOARD_HEIGHT - 1; i >= 0; i--)
              for (j = BOARD_LEFT; j < BOARD_RGHT; p++)
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
    InitChessProgram(&first, FALSE);
    SendToProgram("force\n", &first);
    if (startedFromSetupPosition) {
	SendBoard(&first, forwardMostMove);
    if (appData.debugMode) {
        fprintf(debugFP, "Load Game\n");
    }
	DisplayBothClocks();
    }      

    /* [HGM] server: flag to write setup moves in broadcast file as one */
    loadFlag = appData.suppressLoadMoves;

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

    // [HGM] PV info: routine tests if comment empty
    if (!matchMode && (pausing || appData.timeDelay != 0)) {
	DisplayComment(currentMove - 1, commentList[currentMove]);
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

    loadFlag = 0; /* [HGM] true game starts */
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
            snprintf(buf, sizeof(buf), _("Can't open \"%s\""), filename);
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
      InitChessProgram(&first, FALSE);
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
#if 0
    switch (line[0]) {
      case '#':  case 'x':
      default:
	fenMode = FALSE;
	break;
      case 'p':  case 'n':  case 'b':  case 'r':  case 'q':  case 'k':
      case 'P':  case 'N':  case 'B':  case 'R':  case 'Q':  case 'K':
      case '1':  case '2':  case '3':  case '4':  case '5':  case '6':
      case '7':  case '8':  case '9':
      case 'H':  case 'A':  case 'M':  case 'h':  case 'a':  case 'm':
      case 'E':  case 'F':  case 'G':  case 'e':  case 'f':  case 'g':
      case 'C':  case 'W':             case 'c':  case 'w': 
	fenMode = TRUE;
	break;
    }
#else
    // [HGM] FEN can begin with digit, any piece letter valid in this variant, or a + for Shogi promoted pieces
    fenMode = line[0] >= '0' && line[0] <= '9' || line[0] == '+' || CharToPiece(line[0]) != EmptySquare;
#endif

    if (pn >= 2) {
	if (fenMode || line[0] == '#') pn--;
	while (pn > 0) {
	    /* skip positions before number pn */
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
    
        for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
	    (void) fgets(line, MSG_SIZ, f);
            for (p = line, j = BOARD_LEFT; j < BOARD_RGHT; p++) {
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
          /* [HGM] copy FEN attributes as well */
          {   int i;
              initialRulePlies = FENrulePlies;
              epStatus[forwardMostMove] = FENepStatus;
              for( i=0; i< nrCastlingRights; i++ )
                  castlingRights[forwardMostMove][i] = FENcastlingRights[i];
          }
    SendBoard(&first, forwardMostMove);
    if (appData.debugMode) {
int i, j;
  for(i=0;i<2;i++){for(j=0;j<6;j++)fprintf(debugFP, " %d", castlingRights[i][j]);fprintf(debugFP,"\n");}
  for(j=0;j<6;j++)fprintf(debugFP, " %d", initialRights[j]);fprintf(debugFP,"\n");
        fprintf(debugFP, "Load Position\n");
    }

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
	    snprintf(buf, sizeof(buf), _("Can't open \"%s\""), filename);
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

#define PGN_SIDE_WHITE  0
#define PGN_SIDE_BLACK  1

/* [AS] */
static int FindFirstMoveOutOfBook( int side )
{
    int result = -1;

    if( backwardMostMove == 0 && ! startedFromSetupPosition) {
        int index = backwardMostMove;
        int has_book_hit = 0;

        if( (index % 2) != side ) {
            index++;
        }

        while( index < forwardMostMove ) {
            /* Check to see if engine is in book */
            int depth = pvInfoList[index].depth;
            int score = pvInfoList[index].score;
            int in_book = 0;

            if( depth <= 2 ) {
                in_book = 1;
            }
            else if( score == 0 && depth == 63 ) {
                in_book = 1; /* Zappa */
            }
            else if( score == 2 && depth == 99 ) {
                in_book = 1; /* Abrok */
            }

            has_book_hit += in_book;

            if( ! in_book ) {
                result = index;

                break;
            }

            index += 2;
        }
    }

    return result;
}

/* [AS] */
void GetOutOfBookInfo( char * buf )
{
    int oob[2];
    int i;
    int offset = backwardMostMove & (~1L); /* output move numbers start at 1 */

    oob[0] = FindFirstMoveOutOfBook( PGN_SIDE_WHITE );
    oob[1] = FindFirstMoveOutOfBook( PGN_SIDE_BLACK );

    *buf = '\0';

    if( oob[0] >= 0 || oob[1] >= 0 ) {
        for( i=0; i<2; i++ ) {
            int idx = oob[i];

            if( idx >= 0 ) {
                if( i > 0 && oob[0] >= 0 ) {
                    strcat( buf, "   " );
                }

                sprintf( buf+strlen(buf), "%d%s. ", (idx - offset)/2 + 1, idx & 1 ? ".." : "" );
                sprintf( buf+strlen(buf), "%s%.2f", 
                    pvInfoList[idx].score >= 0 ? "+" : "",
                    pvInfoList[idx].score / 100.0 );
            }
        }
    }
}

/* Save game in PGN style and close the file */
int
SaveGamePGN(f)
     FILE *f;
{
    int i, offset, linelen, newblock;
    time_t tm;
//    char *movetext;
    char numtext[32];
    int movelen, numlen, blank;
    char move_buffer[100]; /* [AS] Buffer for move+PV info */

    offset = backwardMostMove & (~1L); /* output move numbers start at 1 */
    
    tm = time((time_t *) NULL);
    
    PrintPGNTags(f, &gameInfo);
    
    if (backwardMostMove > 0 || startedFromSetupPosition) {
        char *fen = PositionToFEN(backwardMostMove, NULL);
        fprintf(f, "[FEN \"%s\"]\n[SetUp \"1\"]\n", fen);
	fprintf(f, "\n{--------------\n");
	PrintPosition(f, backwardMostMove);
	fprintf(f, "--------------}\n");
        free(fen);
    }
    else {
        /* [AS] Out of book annotation */
        if( appData.saveOutOfBookInfo ) {
            char buf[64];

            GetOutOfBookInfo( buf );

            if( buf[0] != '\0' ) {
                fprintf( f, "[%s \"%s\"]\n", PGN_OUT_OF_BOOK, buf ); 
            }
        }

	fprintf(f, "\n");
    }

    i = backwardMostMove;
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
	strcpy(move_buffer, SavePart(parseList[i])); // [HGM] pgn: print move via buffer, so it can be edited
	movelen = strlen(move_buffer); /* [HGM] pgn: line-break point before move */
#if 0
	// SavePart already does this!
        if( i >= 0 && appData.saveExtendedInfoInPGN && pvInfoList[i].depth > 0 ) {
		int p = movelen - 1;
		if(move_buffer[p] == ' ') p--;
		if(move_buffer[p] == ')') { // [HGM] pgn: strip off ICS time if we have extended info
		    while(p && move_buffer[--p] != '(');
		    if(p && move_buffer[p-1] == ' ') move_buffer[movelen=p-1] = 0;
		}
        }
#endif
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
	fprintf(f, move_buffer);
	linelen += movelen;

        /* [AS] Add PV info if present */
        if( i >= 0 && appData.saveExtendedInfoInPGN && pvInfoList[i].depth > 0 ) {
            /* [HGM] add time */
            char buf[MSG_SIZ]; int seconds = 0;

#if 1
            if(i >= backwardMostMove) {
		if(WhiteOnMove(i))
			seconds = timeRemaining[0][i] - timeRemaining[0][i+1]
				  + GetTimeQuota(i/2) / (1000*WhitePlayer()->timeOdds);
		else
			seconds = timeRemaining[1][i] - timeRemaining[1][i+1]
                                  + GetTimeQuota(i/2) / (1000*WhitePlayer()->other->timeOdds);
            }
            seconds = (seconds+50)/100; // deci-seconds, rounded to nearest
#else
            seconds = (pvInfoList[i].time + 5)/10; // [HGM] PVtime: use engine time
#endif

            if( seconds <= 0) buf[0] = 0; else
            if( seconds < 30 ) sprintf(buf, " %3.1f%c", seconds/10., 0); else {
		seconds = (seconds + 4)/10; // round to full seconds
	        if( seconds < 60 ) sprintf(buf, " %d%c", seconds, 0); else
				   sprintf(buf, " %d:%02d%c", seconds/60, seconds%60, 0);
	    }

            sprintf( move_buffer, "{%s%.2f/%d%s}", 
                pvInfoList[i].score >= 0 ? "+" : "",
                pvInfoList[i].score / 100.0,
                pvInfoList[i].depth,
		buf );

	    movelen = strlen(move_buffer); /* [HGM] pgn: line-break point after move */

	    /* Print score/depth */
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
	    fprintf(f, move_buffer);
	    linelen += movelen;
        }

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
    lastSavedGame = GameCheckSum(); // [HGM] save: remember ID of last saved game to prevent double saving
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
    lastSavedGame = GameCheckSum(); // [HGM] save: remember ID of last saved game to prevent double saving
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
	    snprintf(buf, sizeof(buf), _("Can't open \"%s\""), filename);
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
	fen = PositionToFEN(currentMove, NULL);
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
#if 0
    /* Save game if resource set and not already saved by GameEnds() */
    if ((gameInfo.resultDetails == NULL || errorExitFlag )
                             && forwardMostMove > 0) {
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
#else
    /* [HGM] crash: leave writing PGN and position entirely to GameEnds() */
    GameEnds(gameInfo.result, gameInfo.resultDetails==NULL ? "xboard exit" : gameInfo.resultDetails, GE_PLAYER);
#endif
    /* [HGM] crash: the above GameEnds() is a dud if another one was running */
    /* make sure this other one finishes before killing it!                  */
    if(endingGame) { int count = 0;
        if(appData.debugMode) fprintf(debugFP, "ExitEvent() during GameEnds(), wait\n");
        while(endingGame && count++ < 10) DoSleep(1);
        if(appData.debugMode && endingGame) fprintf(debugFP, "GameEnds() seems stuck, proceed exiting\n");
    }

    /* Kill off chess programs */
    if (first.pr != NoProc) {
	ExitAnalyzeMode();
        
        DoSleep( appData.delayBeforeQuit );
	SendToProgram("quit\n", &first);
        DoSleep( appData.delayAfterQuit );
	DestroyChildProcess(first.pr, 10 /* [AS] first.useSigterm */ );
    }
    if (second.pr != NoProc) {
        DoSleep( appData.delayBeforeQuit );
	SendToProgram("quit\n", &second);
        DoSleep( appData.delayAfterQuit );
	DestroyChildProcess(second.pr, 10 /* [AS] second.useSigterm */ );
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
    char *bookHit = NULL;

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
    if(gameMode == BeginningOfGame) { /* [HGM] time odds: to get right odds in human mode */
	gameMode = MachinePlaysWhite;
	ResetClocks();
    } else
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
      SendToProgram("white\n", &first); // [HGM] book: send 'go' separately
    }
    bookHit = SendMoveToBookUser(forwardMostMove-1, &first, TRUE); // [HGM] book: send go or retrieve book move
    SetMachineThinkingEnables();
    first.maybeThinking = TRUE;
    StartClocks();

    if (appData.autoFlipView && !flipView) {
      flipView = !flipView;
      DrawPosition(FALSE, NULL);
      DisplayBothClocks();       // [HGM] logo: clocks might have to be exchanged;
    }

    if(bookHit) { // [HGM] book: simulate book reply
	static char bookMove[MSG_SIZ]; // a bit generous?

	programStats.nodes = programStats.depth = programStats.time = 
	programStats.score = programStats.got_only_move = 0;
	sprintf(programStats.movelist, "%s (xbook)", bookHit);

	strcpy(bookMove, "move ");
	strcat(bookMove, bookHit);
	HandleMachineMove(bookMove, &first);
    }
}

void
MachineBlackEvent()
{
    char buf[MSG_SIZ];
   char *bookHit = NULL;

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
      SendToProgram("black\n", &first); // [HGM] book: 'go' sent separately
    }
    bookHit = SendMoveToBookUser(forwardMostMove-1, &first, TRUE); // [HGM] book: send go or retrieve book move
    SetMachineThinkingEnables();
    first.maybeThinking = TRUE;
    StartClocks();

    if (appData.autoFlipView && flipView) {
      flipView = !flipView;
      DrawPosition(FALSE, NULL);
      DisplayBothClocks();       // [HGM] logo: clocks might have to be exchanged;
    }
    if(bookHit) { // [HGM] book: simulate book reply
	static char bookMove[MSG_SIZ]; // a bit generous?

	programStats.nodes = programStats.depth = programStats.time = 
	programStats.score = programStats.got_only_move = 0;
	sprintf(programStats.movelist, "%s (xbook)", bookHit);

	strcpy(bookMove, "move ");
	strcat(bookMove, bookHit);
	HandleMachineMove(bookMove, &first);
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
    char *bookHit = NULL;
    
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
    InitChessProgram(&second, FALSE);
    SendToProgram("force\n", &second);
    if (startedFromSetupPosition) {
	SendBoard(&second, backwardMostMove);
    if (appData.debugMode) {
        fprintf(debugFP, "Two Machines\n");
    }
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

    ResetClocks();
    if (!first.sendTime || !second.sendTime) {
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
    bookHit = SendMoveToBookUser(forwardMostMove-1, onmove, TRUE); // [HGM] book: send go or retrieve book move
//    SendToProgram("go\n", onmove);
    onmove->maybeThinking = TRUE;
    SetMachineThinkingEnables();

    StartClocks();

    if(bookHit) { // [HGM] book: simulate book reply
	static char bookMove[MSG_SIZ]; // a bit generous?

	programStats.nodes = programStats.depth = programStats.time = 
	programStats.score = programStats.got_only_move = 0;
	sprintf(programStats.movelist, "%s (xbook)", bookHit);

	strcpy(bookMove, "move ");
	strcat(bookMove, bookHit);
	HandleMachineMove(bookMove, &first);
    }
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
    /* [DM] icsEngineAnalyze - possible call from other functions */
    if (appData.icsEngineAnalyze) {
        appData.icsEngineAnalyze = FALSE;

        DisplayMessage("",_("Close ICS engine analyze..."));
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
    int king = gameInfo.variant == VariantKnightmate ? WhiteUnicorn : WhiteKing;

    startedFromSetupPosition = TRUE;
    InitChessProgram(&first, FALSE);
    castlingRights[0][2] = castlingRights[0][5] = BOARD_WIDTH>>1;
    if(boards[0][0][BOARD_WIDTH>>1] == king) {
	castlingRights[0][1] = boards[0][0][BOARD_LEFT] == WhiteRook ? 0 : -1;
	castlingRights[0][0] = boards[0][0][BOARD_RGHT-1] == WhiteRook ? BOARD_RGHT-1 : -1;
    } else castlingRights[0][2] = -1;
    if(boards[0][BOARD_HEIGHT-1][BOARD_WIDTH>>1] == WHITE_TO_BLACK king) {
	castlingRights[0][4] = boards[0][BOARD_HEIGHT-1][BOARD_LEFT] == BlackRook ? 0 : -1;
	castlingRights[0][3] = boards[0][BOARD_HEIGHT-1][BOARD_RGHT-1] == BlackRook ? BOARD_RGHT-1 : -1;
    } else castlingRights[0][5] = -1;
    SendToProgram("force\n", &first);
    if (blackPlaysFirst) {
	strcpy(moveList[0], "");
	strcpy(parseList[0], "");
	currentMove = forwardMostMove = backwardMostMove = 1;
	CopyBoard(boards[1], boards[0]);
	/* [HGM] copy rights as well, as this code is also used after pasting a FEN */
	{ int i;
	  epStatus[1] = epStatus[0];
	  for(i=0; i<nrCastlingRights; i++) castlingRights[1][i] = castlingRights[0][i];
	}
    } else {
	currentMove = forwardMostMove = backwardMostMove = 0;
    }
    SendBoard(&first, forwardMostMove);
    if (appData.debugMode) {
        fprintf(debugFP, "EditPosDone\n");
    }
    DisplayTitle("");
    timeRemaining[0][forwardMostMove] = whiteTimeRemaining;
    timeRemaining[1][forwardMostMove] = blackTimeRemaining;
    gameMode = EditGame;
    ModeHighlight();
    HistorySet(parseList, backwardMostMove, forwardMostMove, currentMove-1);
    ClearHighlights(); /* [AS] */
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
    ChessSquare piece = boards[0][y][x];

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
            for (x = 0; x < BOARD_WIDTH; x++) { ChessSquare p = EmptySquare;
		if(x == BOARD_LEFT-1 || x == BOARD_RGHT) p = (ChessSquare) 0; /* [HGM] holdings */
                for (y = 0; y < BOARD_HEIGHT; y++) {
		    if (gameMode == IcsExamining) {
			if (boards[currentMove][y][x] != EmptySquare) {
			    sprintf(buf, "%sx@%c%c\n", ics_prefix,
                                    AAA + x, ONE + y);
			    SendToICS(buf);
			}
		    } else {
			boards[0][y][x] = p;
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
            sprintf(buf, "%sx@%c%c\n", ics_prefix, AAA + x, ONE + y);
	    SendToICS(buf);
	} else {
	    boards[0][y][x] = EmptySquare;
	    DrawPosition(FALSE, boards[0]);
	}
	break;

      case PromotePiece:
        if(piece >= (int)WhitePawn && piece < (int)WhiteMan ||
           piece >= (int)BlackPawn && piece < (int)BlackMan   ) {
            selection = (ChessSquare) (PROMOTED piece);
        } else if(piece == EmptySquare) selection = WhiteSilver;
        else selection = (ChessSquare)((int)piece - 1);
        goto defaultlabel;

      case DemotePiece:
        if(piece > (int)WhiteMan && piece <= (int)WhiteKing ||
           piece > (int)BlackMan && piece <= (int)BlackKing   ) {
            selection = (ChessSquare) (DEMOTED piece);
        } else if(piece == EmptySquare) selection = BlackSilver;
        else selection = (ChessSquare)((int)piece + 1);       
        goto defaultlabel;

      case WhiteQueen:
      case BlackQueen:
        if(gameInfo.variant == VariantShatranj ||
           gameInfo.variant == VariantXiangqi  ||
           gameInfo.variant == VariantCourier    )
            selection = (ChessSquare)((int)selection - (int)WhiteQueen + (int)WhiteFerz);
        goto defaultlabel;

      case WhiteKing:
      case BlackKing:
        if(gameInfo.variant == VariantXiangqi)
            selection = (ChessSquare)((int)selection - (int)WhiteKing + (int)WhiteWazir);
        if(gameInfo.variant == VariantKnightmate)
            selection = (ChessSquare)((int)selection - (int)WhiteKing + (int)WhiteUnicorn);
      default:
        defaultlabel:
	if (gameMode == IcsExamining) {
  	    sprintf(buf, "%s%c@%c%c\n", ics_prefix,
                    PieceToChar(selection), AAA + x, ONE + y);
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
        toX = moveList[target - 1][2] - AAA;
        toY = moveList[target - 1][3] - ONE;
	if (moveList[target - 1][1] == '@') {
	    if (appData.highlightLastMove) {
		SetHighlights(-1, -1, toX, toY);
	    }
	} else {
            fromX = moveList[target - 1][0] - AAA;
            fromY = moveList[target - 1][1] - ONE;
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
    if ( !matchMode && gameMode != Training) { // [HGM] PV info: routine tests if empty
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
    int full_redraw = TRUE; /* [AS] Was FALSE, had to change it! */

    if (appData.debugMode)
	fprintf(debugFP, "BackwardInner(%d), current %d, forward %d\n",
		target, currentMove, forwardMostMove);

    if (gameMode == EditPosition) return;
    if (currentMove <= backwardMostMove) {
	ClearHighlights();
	DrawPosition(full_redraw, boards[currentMove]);
	return;
    }
    if (gameMode == PlayFromGameFile && !pausing)
      PauseEvent();
    
    if (moveList[target][0]) {
	int fromX, fromY, toX, toY;
        toX = moveList[target][2] - AAA;
        toY = moveList[target][3] - ONE;
	if (moveList[target][1] == '@') {
	    if (appData.highlightLastMove) {
		SetHighlights(-1, -1, toX, toY);
	    }
	} else {
            fromX = moveList[target][0] - AAA;
            fromY = moveList[target][1] - ONE;
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
    DrawPosition(full_redraw, boards[currentMove]);
    HistorySet(parseList,backwardMostMove,forwardMostMove,currentMove-1);
    // [HGM] PV info: routine tests if comment empty
    DisplayComment(currentMove - 1, commentList[currentMove]);
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
	DrawPosition(TRUE, boards[currentMove]); /* [AS] Changed to full redraw! */
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
    
    for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
        for (j = BOARD_LEFT; j < BOARD_RGHT; j++) {
	    char c = PieceToChar(boards[move][i][j]);
	    fputc(c == 'x' ? '.' : c, fp);
            fputc(j == BOARD_RGHT - 1 ? '\n' : ' ', fp);
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
    if(p == prog && *p == '"') p++;
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
	gameInfo.event = StrSave( appData.pgnEventHeader );
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave(first.tidy);
	gameInfo.black = StrSave(UserName());
	gameInfo.timeControl = TimeControlTagValue();
	break;

      case MachinePlaysBlack:
	gameInfo.event = StrSave( appData.pgnEventHeader );
	gameInfo.site = StrSave(HostName());
	gameInfo.date = PGNDate();
	gameInfo.round = StrSave("-");
	gameInfo.white = StrSave(UserName());
	gameInfo.black = StrSave(first.tidy);
	gameInfo.timeControl = TimeControlTagValue();
	break;

      case TwoMachinesPlay:
	gameInfo.event = StrSave( appData.pgnEventHeader );
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

    text = GetInfoFromComment( index, text ); /* [HGM] PV time: strip PV info from comment */

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

static char * FindStr( char * text, char * sub_text )
{
    char * result = strstr( text, sub_text );

    if( result != NULL ) {
        result += strlen( sub_text );
    }

    return result;
}

/* [AS] Try to extract PV info from PGN comment */
/* [HGM] PV time: and then remove it, to prevent it appearing twice */
char *GetInfoFromComment( int index, char * text )
{
    char * sep = text;

    if( text != NULL && index > 0 ) {
        int score = 0;
        int depth = 0;
        int time = -1, sec = 0, deci;
        char * s_eval = FindStr( text, "[%eval " );
        char * s_emt = FindStr( text, "[%emt " );

        if( s_eval != NULL || s_emt != NULL ) {
            /* New style */
            char delim;

            if( s_eval != NULL ) {
                if( sscanf( s_eval, "%d,%d%c", &score, &depth, &delim ) != 3 ) {
                    return text;
                }

                if( delim != ']' ) {
                    return text;
                }
            }

            if( s_emt != NULL ) {
            }
        }
        else {
            /* We expect something like: [+|-]nnn.nn/dd */
            int score_lo = 0;

            sep = strchr( text, '/' );
            if( sep == NULL || sep < (text+4) ) {
                return text;
            }

            time = -1; sec = -1; deci = -1;
            if( sscanf( text, "%d.%d/%d %d:%d", &score, &score_lo, &depth, &time, &sec ) != 5 &&
		sscanf( text, "%d.%d/%d %d.%d", &score, &score_lo, &depth, &time, &deci ) != 5 &&
                sscanf( text, "%d.%d/%d %d", &score, &score_lo, &depth, &time ) != 4 &&
                sscanf( text, "%d.%d/%d", &score, &score_lo, &depth ) != 3   ) {
                return text;
            }

            if( score_lo < 0 || score_lo >= 100 ) {
                return text;
            }

            if(sec >= 0) time = 600*time + 10*sec; else
            if(deci >= 0) time = 10*time + deci; else time *= 10; // deci-sec

            score = score >= 0 ? score*100 + score_lo : score*100 - score_lo;

            /* [HGM] PV time: now locate end of PV info */
            while( *++sep >= '0' && *sep <= '9'); // strip depth
            if(time >= 0)
            while( *++sep >= '0' && *sep <= '9'); // strip time
            if(sec >= 0)
            while( *++sep >= '0' && *sep <= '9'); // strip seconds
            if(deci >= 0)
            while( *++sep >= '0' && *sep <= '9'); // strip fractional seconds
            while(*sep == ' ') sep++;
        }

        if( depth <= 0 ) {
            return text;
        }

        if( time < 0 ) {
            time = -1;
        }

        pvInfoList[index-1].depth = depth;
        pvInfoList[index-1].score = score;
        pvInfoList[index-1].time  = 10*time; // centi-sec
    }
    return sep;
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
    if (outCount < count && !exiting 
                         && !endingGame) { /* [HGM] crash: to not hang GameEnds() writing to deceased engines */
	sprintf(buf, _("Error writing to %s chess program"), cps->which);
        if(gameInfo.resultDetails==NULL) { /* [HGM] crash: if game in progress, give reason for abort */
            if(epStatus[forwardMostMove] <= EP_DRAWS) {
                gameInfo.result = GameIsDrawn; /* [HGM] accept exit as draw claim */
                sprintf(buf, "%s program exits in draw position (%s)", cps->which, cps->program);
            } else {
                gameInfo.result = cps->twoMachinesColor[0]=='w' ? BlackWins : WhiteWins;
            }
            gameInfo.resultDetails = buf;
        }
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
        if(gameInfo.resultDetails==NULL) { /* [HGM] crash: if game in progress, give reason for abort */
                if(epStatus[forwardMostMove] <= EP_DRAWS) {
                    gameInfo.result = GameIsDrawn; /* [HGM] accept exit as draw claim */
                    sprintf(buf, _("%s program exits in draw position (%s)"), cps->which, cps->program);
                } else {
                    gameInfo.result = cps->twoMachinesColor[0]=='w' ? BlackWins : WhiteWins;
                }
                gameInfo.resultDetails = buf;
            }
	    RemoveInputSource(cps->isr);
	    DisplayFatalError(buf, 0, 1);
	} else {
	    sprintf(buf,
		    _("Error reading from %s chess program (%s)"),
		    cps->which, cps->program);
	    RemoveInputSource(cps->isr);

            /* [AS] Program is misbehaving badly... kill it */
            if( count == -2 ) {
                DestroyChildProcess( cps->pr, 9 );
                cps->pr = NoProc;
            }

            DisplayFatalError(buf, error, 1);
	}
	return;
    }
    
    if ((end_str = strchr(message, '\r')) != NULL)
      *end_str = NULLCHAR;
    if ((end_str = strchr(message, '\n')) != NULL)
      *end_str = NULLCHAR;
    
    if (appData.debugMode) {
	TimeMark now; int print = 1;
	char *quote = ""; char c; int i;

	if(appData.engineComments != 1) { /* [HGM] debug: decide if protocol-violating output is written */
		char start = message[0];
		if(start >='A' && start <= 'Z') start += 'a' - 'A'; // be tolerant to capitalizing
		if(sscanf(message, "%d%c%d%d%d", &i, &c, &i, &i, &i) != 5 && 
		   sscanf(message, "move %c", &c)!=1  && sscanf(message, "offer%c", &c)!=1 &&
		   sscanf(message, "resign%c", &c)!=1 && sscanf(message, "feature %c", &c)!=1 &&
		   sscanf(message, "error %c", &c)!=1 && sscanf(message, "illegal %c", &c)!=1 &&
		   sscanf(message, "tell%c", &c)!=1   && sscanf(message, "0-1 %c", &c)!=1 &&
		   sscanf(message, "1-0 %c", &c)!=1   && sscanf(message, "1/2-1/2 %c", &c)!=1 && start != '#')
			{ quote = "# "; print = (appData.engineComments == 2); }
		message[0] = start; // restore original message
	}
	if(print) {
		GetTimeMark(&now);
		fprintf(debugFP, "%ld <%-6s: %s%s\n", 
			SubtractTimeMarks(&now, &programStartTime), cps->which, 
			quote,
			message);
	}
    }

    /* [DM] if icsEngineAnalyze is active we block all whisper and kibitz output, because nobody want to see this */
    if (appData.icsEngineAnalyze) {
        if (strstr(message, "whisper") != NULL ||
             strstr(message, "kibitz") != NULL || 
            strstr(message, "tellics") != NULL) return;
    }

    HandleMachineMove(message, cps);
}


void
SendTimeControl(cps, mps, tc, inc, sd, st)
     ChessProgramState *cps;
     int mps, inc, sd, st;
     long tc;
{
    char buf[MSG_SIZ];
    int seconds;

    if( timeControl_2 > 0 ) {
        if( (gameMode == MachinePlaysBlack) || (gameMode == TwoMachinesPlay && cps->twoMachinesColor[0] == 'b') ) {
            tc = timeControl_2;
        }
    }
    tc  /= cps->timeOdds; /* [HGM] time odds: apply before telling engine */
    inc /= cps->timeOdds;
    st  /= cps->timeOdds;

    seconds = (tc / 1000) % 60; /* [HGM] displaced to after applying odds */

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

    if(cps->nps > 0) { /* [HGM] nps */
	if(cps->supportsNPS == FALSE) cps->nps = -1; // don't use if engine explicitly says not supported!
	else {
		sprintf(buf, "nps %d\n", cps->nps);
	      SendToProgram(buf, cps);
	}
    }
}

ChessProgramState *WhitePlayer()
/* [HGM] return pointer to 'first' or 'second', depending on who plays white */
{
    if(gameMode == TwoMachinesPlay && first.twoMachinesColor[0] == 'b' || 
       gameMode == BeginningOfGame || gameMode == MachinePlaysBlack)
        return &second;
    return &first;
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
    /* [HGM] translate opponent's time by time-odds factor */
    otime = (otime * cps->other->timeOdds) / cps->timeOdds;
    if (appData.debugMode) {
        fprintf(debugFP, "time odds: %d %d \n", cps->timeOdds, cps->other->timeOdds);
    }

    if (time <= 0) time = 1;
    if (otime <= 0) otime = 1;
    
    sprintf(message, "time %ld\n", time);
    SendToProgram(message, cps);

    sprintf(message, "otim %ld\n", otime);
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

int 
ParseOption(Option *opt, ChessProgramState *cps)
// [HGM] options: process the string that defines an engine option, and determine
// name, type, default value, and allowed value range
{
	char *p, *q, buf[MSG_SIZ];
	int n, min = (-1)<<31, max = 1<<31, def;

	if(p = strstr(opt->name, " -spin ")) {
	    if((n = sscanf(p, " -spin %d %d %d", &def, &min, &max)) < 3 ) return FALSE;
	    if(max < min) max = min; // enforce consistency
	    if(def < min) def = min;
	    if(def > max) def = max;
	    opt->value = def;
	    opt->min = min;
	    opt->max = max;
	    opt->type = Spin;
	} else if((p = strstr(opt->name, " -slider "))) {
	    // for now -slider is a synonym for -spin, to already provide compatibility with future polyglots
	    if((n = sscanf(p, " -slider %d %d %d", &def, &min, &max)) < 3 ) return FALSE;
	    if(max < min) max = min; // enforce consistency
	    if(def < min) def = min;
	    if(def > max) def = max;
	    opt->value = def;
	    opt->min = min;
	    opt->max = max;
	    opt->type = Spin; // Slider;
	} else if((p = strstr(opt->name, " -string "))) {
	    opt->textValue = p+9;
	    opt->type = TextBox;
	} else if((p = strstr(opt->name, " -file "))) {
	    // for now -file is a synonym for -string, to already provide compatibility with future polyglots
	    opt->textValue = p+7;
	    opt->type = TextBox; // FileName;
	} else if((p = strstr(opt->name, " -path "))) {
	    // for now -file is a synonym for -string, to already provide compatibility with future polyglots
	    opt->textValue = p+7;
	    opt->type = TextBox; // PathName;
	} else if(p = strstr(opt->name, " -check ")) {
	    if(sscanf(p, " -check %d", &def) < 1) return FALSE;
	    opt->value = (def != 0);
	    opt->type = CheckBox;
	} else if(p = strstr(opt->name, " -combo ")) {
	    opt->textValue = (char*) (&cps->comboList[cps->comboCnt]); // cheat with pointer type
	    cps->comboList[cps->comboCnt++] = q = p+8; // holds possible choices
	    if(*q == '*') cps->comboList[cps->comboCnt-1]++;
	    opt->value = n = 0;
	    while(q = StrStr(q, " /// ")) {
		n++; *q = 0;    // count choices, and null-terminate each of them
		q += 5;
		if(*q == '*') { // remember default, which is marked with * prefix
		    q++;
		    opt->value = n;
		}
		cps->comboList[cps->comboCnt++] = q;
	    }
	    cps->comboList[cps->comboCnt++] = NULL;
	    opt->max = n + 1;
	    opt->type = ComboBox;
	} else if(p = strstr(opt->name, " -button")) {
	    opt->type = Button;
	} else if(p = strstr(opt->name, " -save")) {
	    opt->type = SaveButton;
	} else return FALSE;
	*p = 0; // terminate option name
	// now look if the command-line options define a setting for this engine option.
	if(cps->optionSettings && cps->optionSettings[0])
	    p = strstr(cps->optionSettings, opt->name); else p = NULL;
	if(p && (p == cps->optionSettings || p[-1] == ',')) {
		sprintf(buf, "option %s", p);
		if(p = strstr(buf, ",")) *p = 0;
		strcat(buf, "\n");
		SendToProgram(buf, cps);
	}
	return TRUE;
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
    /* Added by Tord: */
    if (BoolFeature(&p, "fen960", &cps->useFEN960, cps)) continue;
    if (BoolFeature(&p, "oocastle", &cps->useOOCastle, cps)) continue;
    /* End of additions by Tord */

    /* [HGM] added features: */
    if (BoolFeature(&p, "debug", &cps->debug, cps)) continue;
    if (BoolFeature(&p, "nps", &cps->supportsNPS, cps)) continue;
    if (IntFeature(&p, "level", &cps->maxNrOfSessions, cps)) continue;
    if (BoolFeature(&p, "memory", &cps->memSize, cps)) continue;
    if (BoolFeature(&p, "smp", &cps->maxCores, cps)) continue;
    if (StringFeature(&p, "egt", &cps->egtFormats, cps)) continue;
    if (StringFeature(&p, "option", &(cps->option[cps->nrOptions].name), cps)) {
	if(!ParseOption(&(cps->option[cps->nrOptions++]), cps)) { // [HGM] options: add option feature
	    sprintf(buf, "rejected option %s\n", cps->option[--cps->nrOptions].name);
	    SendToProgram(buf, cps);
	    continue;
	}
	if(cps->nrOptions >= MAX_OPTIONS) {
	    cps->nrOptions--;
	    sprintf(buf, "%s engine has too many options\n", cps->which);
	    DisplayError(buf, 0);
	}
	continue;
    }
    if (BoolFeature(&p, "smp", &cps->maxCores, cps)) continue;
    /* End of additions by HGM */

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
NewSettingEvent(option, command, value)
     char *command;
     int option, value;
{
    char buf[MSG_SIZ];

    if (gameMode == EditPosition) EditPositionDone();
    sprintf(buf, "%s%s %d\n", (option ? "option ": ""), command, value);
    SendToProgram(buf, &first);
    if (gameMode == TwoMachinesPlay) {
	SendToProgram(buf, &second);
    }
}

void
ShowThinkingEvent()
// [HGM] thinking: this routine is now also called from "Options -> Engine..." popup
{
    static int oldState = 2; // kludge alert! Neither true nor fals, so first time oldState is always updated
    int newState = appData.showThinking
	// [HGM] thinking: other features now need thinking output as well
	|| !appData.hideThinkingFromHuman || appData.adjudicateLossThreshold != 0 || EngineOutputIsUp();
    
    if (oldState == newState) return;
    oldState = newState;
    if (gameMode == EditPosition) EditPositionDone();
    if (oldState) {
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
//    appData.showThinking = newState; // [HGM] thinking: responsible option should already have be changed when calling this routine!
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

    if(appData.noGUI) return; // [HGM] fast: suppress display of moves
    
    if (moveNumber == forwardMostMove - 1 || 
	gameMode == AnalyzeMode || gameMode == AnalyzeFile) {

	safeStrCpy(cpThinkOutput, thinkOutput, sizeof(cpThinkOutput));

        if (strchr(cpThinkOutput, '\n')) {
	    *strchr(cpThinkOutput, '\n') = NULLCHAR;
        }
    } else {
	*cpThinkOutput = NULLCHAR;
    }

    /* [AS] Hide thinking from human user */
    if( appData.hideThinkingFromHuman && gameMode != TwoMachinesPlay ) {
        *cpThinkOutput = NULLCHAR;
        if( thinkOutput[0] != NULLCHAR ) {
            int i;

            for( i=0; i<=hiddenThinkOutputState; i++ ) {
                cpThinkOutput[i] = '.';
            }
            cpThinkOutput[i] = NULLCHAR;
            hiddenThinkOutputState = (hiddenThinkOutputState + 1) % 3;
        }
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
    char lst[MSG_SIZ / 2];
    double nps;
    static char *xtra[] = { "", " (--)", " (++)" };
    int h, m, s, cs;
  
    if (programStats.time == 0) {
	programStats.time = 1;
    }
  
    if (programStats.got_only_move) {
	safeStrCpy(buf, programStats.movelist, sizeof(buf));
    } else {
        safeStrCpy( lst, programStats.movelist, sizeof(lst));

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
	    sprintf(buf, "depth=%d %d/%d(%s) %+.2f %s%s\nNodes: " u64Display " NPS: %d\nTime: %02d:%02d:%02d.%02d",
		    programStats.depth,
		    programStats.nr_moves-programStats.moves_left,
		    programStats.nr_moves, programStats.move_name,
		    ((float)programStats.score)/100.0, lst,
		    only_one_move(lst)?
		    xtra[programStats.got_fail] : "",
		    (u64)programStats.nodes, (int)nps, h, m, s, cs);
	  } else {
	    sprintf(buf, "depth=%d %d/%d %+.2f %s%s\nNodes: " u64Display " NPS: %d\nTime: %02d:%02d:%02d.%02d",
		    programStats.depth,
		    programStats.nr_moves-programStats.moves_left,
		    programStats.nr_moves, ((float)programStats.score)/100.0,
		    lst,
		    only_one_move(lst)?
		    xtra[programStats.got_fail] : "",
		    (u64)programStats.nodes, (int)nps, h, m, s, cs);
	  }
	} else {
	    sprintf(buf, "depth=%d %+.2f %s%s\nNodes: " u64Display " NPS: %d\nTime: %02d:%02d:%02d.%02d",
		    programStats.depth,
		    ((float)programStats.score)/100.0,
		    lst,
		    only_one_move(lst)?
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
    char buf[8000]; // comment can be long!
    int score, depth;

    if( appData.autoDisplayComment ) {
        if (moveNumber < 0 || parseList[moveNumber][0] == NULLCHAR) {
	    strcpy(title, "Comment");
        } else {
	    sprintf(title, "Comment on %d.%s%s", moveNumber / 2 + 1,
		    WhiteOnMove(moveNumber) ? " " : ".. ",
		    parseList[moveNumber]);
        }
	// [HGM] PV info: display PV info together with (or as) comment
	if(moveNumber >= 0 && (depth = pvInfoList[moveNumber].depth) > 0) {
	    if(text == NULL) text = "";                                           
	    score = pvInfoList[moveNumber].score;
	    sprintf(buf, "%s%.2f/%d %d\n%s", score>0 ? "+" : "", score/100.,
                              depth, (pvInfoList[moveNumber].time+50)/100, text);
	    text = buf;
	}
    } else title[0] = 0;

    if (text != NULL)
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
                    if(gameMode != TwoMachinesPlay) DisplayTitle(_("Both flags fell"));
		} else {
                    if(gameMode != TwoMachinesPlay) DisplayTitle(_("White's flag fell"));
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
                    if(gameMode != TwoMachinesPlay) DisplayTitle(_("Both flags fell"));
		} else {
                    if(gameMode != TwoMachinesPlay) DisplayTitle(_("Black's flag fell"));
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

    /*
     * add time to clocks when time control is achieved ([HGM] now also used for increment)
     */
    if ( !WhiteOnMove(forwardMostMove) )
	/* White made time control */
        whiteTimeRemaining += GetTimeQuota((forwardMostMove-1)/2)
        /* [HGM] time odds: correct new time quota for time odds! */
                                            / WhitePlayer()->timeOdds;
      else
	/* Black made time control */
        blackTimeRemaining += GetTimeQuota((forwardMostMove-1)/2)
                                            / WhitePlayer()->other->timeOdds;
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

/* VS 2008 requires the #include outside of the function */
#if !HAVE_GETTIMEOFDAY && HAVE_FTIME
#include <sys/timeb.h>
#endif

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

// include <sys/timeb.h> / moved to just above start of function
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

/* Adjust clock one minute up or down */
void
AdjustClock(Boolean which, int dir)
{
    if(which) blackTimeRemaining += 60000*dir;
    else      whiteTimeRemaining += 60000*dir;
    DisplayBothClocks();
}

/* Stop clocks and reset to a fresh time control */
void
ResetClocks() 
{
    (void) StopClockTimer();
    if (appData.icsActive) {
	whiteTimeRemaining = blackTimeRemaining = 0;
    } else { /* [HGM] correct new time quote for time odds */
        whiteTimeRemaining = GetTimeQuota(-1) / WhitePlayer()->timeOdds;
        blackTimeRemaining = GetTimeQuota(-1) / WhitePlayer()->other->timeOdds;
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
	if(whiteNPS >= 0) lastTickLength = 0;
	timeRemaining = whiteTimeRemaining -= lastTickLength;
	DisplayWhiteClock(whiteTimeRemaining - fudge,
			  WhiteOnMove(currentMove));
    } else {
	if(blackNPS >= 0) lastTickLength = 0;
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
	    if(blackNPS >= 0) lastTickLength = 0;
	    blackTimeRemaining -= lastTickLength;
           /* [HGM] PGNtime: save time for PGN file if engine did not give it */
//         if(pvInfoList[forwardMostMove-1].time == -1)
                 pvInfoList[forwardMostMove-1].time =               // use GUI time
                      (timeRemaining[1][forwardMostMove-1] - blackTimeRemaining)/10;
	} else {
	   if(whiteNPS >= 0) lastTickLength = 0;
	   whiteTimeRemaining -= lastTickLength;
           /* [HGM] PGNtime: save time for PGN file if engine did not give it */
//         if(pvInfoList[forwardMostMove-1].time == -1)
                 pvInfoList[forwardMostMove-1].time = 
                      (timeRemaining[0][forwardMostMove-1] - whiteTimeRemaining)/10;
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
	if(whiteNPS >= 0) lastTickLength = 0;
	whiteTimeRemaining -= lastTickLength;
	DisplayWhiteClock(whiteTimeRemaining, WhiteOnMove(currentMove));
    } else {
	if(blackNPS >= 0) lastTickLength = 0;
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

   /* [HGM] nps: figure out nps factors, by determining which engine plays white and/or black once and for all */
    whiteNPS = blackNPS = -1; 
    if(gameMode == MachinePlaysWhite || gameMode == TwoMachinesPlay && first.twoMachinesColor[0] == 'w'
       || appData.zippyPlay && gameMode == IcsPlayingBlack) // first (perhaps only) engine has white
	whiteNPS = first.nps;
    if(gameMode == MachinePlaysBlack || gameMode == TwoMachinesPlay && first.twoMachinesColor[0] == 'b'
       || appData.zippyPlay && gameMode == IcsPlayingWhite) // first (perhaps only) engine has black
	blackNPS = first.nps;
    if(gameMode == TwoMachinesPlay && first.twoMachinesColor[0] == 'b') // second only used in Two-Machines mode
	whiteNPS = second.nps;
    if(gameMode == TwoMachinesPlay && first.twoMachinesColor[0] == 'w')
	blackNPS = second.nps;
    if(appData.debugMode) fprintf(debugFP, "nps: w=%d, b=%d\n", whiteNPS, blackNPS);

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
PositionToFEN(move, overrideCastling)
     int move;
     char *overrideCastling;
{
    int i, j, fromX, fromY, toX, toY;
    int whiteToPlay;
    char buf[128];
    char *p, *q;
    int emptycount;
    ChessSquare piece;

    whiteToPlay = (gameMode == EditPosition) ?
      !blackPlaysFirst : (move % 2 == 0);
    p = buf;

    /* Piece placement data */
    for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
	emptycount = 0;
        for (j = BOARD_LEFT; j < BOARD_RGHT; j++) {
	    if (boards[move][i][j] == EmptySquare) {
		emptycount++;
            } else { ChessSquare piece = boards[move][i][j];
		if (emptycount > 0) {
                    if(emptycount<10) /* [HGM] can be >= 10 */
                        *p++ = '0' + emptycount;
                    else { *p++ = '0' + emptycount/10; *p++ = '0' + emptycount%10; }
		    emptycount = 0;
		}
                if(PieceToChar(piece) == '+') {
                    /* [HGM] write promoted pieces as '+<unpromoted>' (Shogi) */
                    *p++ = '+';
                    piece = (ChessSquare)(DEMOTED piece);
                } 
                *p++ = PieceToChar(piece);
                if(p[-1] == '~') {
                    /* [HGM] flag promoted pieces as '<promoted>~' (Crazyhouse) */
                    p[-1] = PieceToChar((ChessSquare)(DEMOTED piece));
                    *p++ = '~';
                }
	    }
	}
	if (emptycount > 0) {
            if(emptycount<10) /* [HGM] can be >= 10 */
                *p++ = '0' + emptycount;
            else { *p++ = '0' + emptycount/10; *p++ = '0' + emptycount%10; }
	    emptycount = 0;
	}
	*p++ = '/';
    }
    *(p - 1) = ' ';

    /* [HGM] print Crazyhouse or Shogi holdings */
    if( gameInfo.holdingsWidth ) {
        *(p-1) = '['; /* if we wanted to support BFEN, this could be '/' */
        q = p;
        for(i=0; i<gameInfo.holdingsSize; i++) { /* white holdings */
            piece = boards[move][i][BOARD_WIDTH-1];
            if( piece != EmptySquare )
              for(j=0; j<(int) boards[move][i][BOARD_WIDTH-2]; j++)
                  *p++ = PieceToChar(piece);
        }
        for(i=0; i<gameInfo.holdingsSize; i++) { /* black holdings */
            piece = boards[move][BOARD_HEIGHT-i-1][0];
            if( piece != EmptySquare )
              for(j=0; j<(int) boards[move][BOARD_HEIGHT-i-1][1]; j++)
                  *p++ = PieceToChar(piece);
        }

        if( q == p ) *p++ = '-';
        *p++ = ']';
        *p++ = ' ';
    }

    /* Active color */
    *p++ = whiteToPlay ? 'w' : 'b';
    *p++ = ' ';

  if(q = overrideCastling) { // [HGM] FRC: override castling & e.p fields for non-compliant engines
    while(*p++ = *q++); if(q != overrideCastling+1) p[-1] = ' ';
  } else {
  if(nrCastlingRights) {
     q = p;
     if(gameInfo.variant == VariantFischeRandom || gameInfo.variant == VariantCapaRandom) {
       /* [HGM] write directly from rights */
           if(castlingRights[move][2] >= 0 &&
              castlingRights[move][0] >= 0   )
                *p++ = castlingRights[move][0] + AAA + 'A' - 'a';
           if(castlingRights[move][2] >= 0 &&
              castlingRights[move][1] >= 0   )
                *p++ = castlingRights[move][1] + AAA + 'A' - 'a';
           if(castlingRights[move][5] >= 0 &&
              castlingRights[move][3] >= 0   )
                *p++ = castlingRights[move][3] + AAA;
           if(castlingRights[move][5] >= 0 &&
              castlingRights[move][4] >= 0   )
                *p++ = castlingRights[move][4] + AAA;
     } else {

        /* [HGM] write true castling rights */
        if( nrCastlingRights == 6 ) {
            if(castlingRights[move][0] == BOARD_RGHT-1 &&
               castlingRights[move][2] >= 0  ) *p++ = 'K';
            if(castlingRights[move][1] == BOARD_LEFT &&
               castlingRights[move][2] >= 0  ) *p++ = 'Q';
            if(castlingRights[move][3] == BOARD_RGHT-1 &&
               castlingRights[move][5] >= 0  ) *p++ = 'k';
            if(castlingRights[move][4] == BOARD_LEFT &&
               castlingRights[move][5] >= 0  ) *p++ = 'q';
        }
     }
     if (q == p) *p++ = '-'; /* No castling rights */
     *p++ = ' ';
  }

  if(gameInfo.variant != VariantShogi    && gameInfo.variant != VariantXiangqi &&
     gameInfo.variant != VariantShatranj && gameInfo.variant != VariantCourier ) { 
    /* En passant target square */
    if (move > backwardMostMove) {
        fromX = moveList[move - 1][0] - AAA;
        fromY = moveList[move - 1][1] - ONE;
        toX = moveList[move - 1][2] - AAA;
        toY = moveList[move - 1][3] - ONE;
	if (fromY == (whiteToPlay ? BOARD_HEIGHT-2 : 1) &&
	    toY == (whiteToPlay ? BOARD_HEIGHT-4 : 3) &&
	    boards[move][toY][toX] == (whiteToPlay ? BlackPawn : WhitePawn) &&
	    fromX == toX) {
	    /* 2-square pawn move just happened */
            *p++ = toX + AAA;
	    *p++ = whiteToPlay ? '6'+BOARD_HEIGHT-8 : '3';
	} else {
	    *p++ = '-';
	}
    } else {
	*p++ = '-';
    }
    *p++ = ' ';
  }
  }

    /* [HGM] find reversible plies */
    {   int i = 0, j=move;

        if (appData.debugMode) { int k;
            fprintf(debugFP, "write FEN 50-move: %d %d %d\n", initialRulePlies, forwardMostMove, backwardMostMove);
            for(k=backwardMostMove; k<=forwardMostMove; k++)
                fprintf(debugFP, "e%d. p=%d\n", k, epStatus[k]);

        }

        while(j > backwardMostMove && epStatus[j] <= EP_NONE) j--,i++;
        if( j == backwardMostMove ) i += initialRulePlies;
        sprintf(p, "%d ", i);
        p += i>=100 ? 4 : i >= 10 ? 3 : 2;
    }
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
    ChessSquare piece;

    p = fen;

    /* [HGM] by default clear Crazyhouse holdings, if present */
    if(gameInfo.holdingsWidth) {
       for(i=0; i<BOARD_HEIGHT; i++) {
           board[i][0]             = EmptySquare; /* black holdings */
           board[i][BOARD_WIDTH-1] = EmptySquare; /* white holdings */
           board[i][1]             = (ChessSquare) 0; /* black counts */
           board[i][BOARD_WIDTH-2] = (ChessSquare) 0; /* white counts */
       }
    }

    /* Piece placement data */
    for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
	j = 0;
	for (;;) {
            if (*p == '/' || *p == ' ' || (*p == '[' && i == 0) ) {
                if (*p == '/') p++;
                emptycount = gameInfo.boardWidth - j;
                while (emptycount--)
                        board[i][(j++)+gameInfo.holdingsWidth] = EmptySquare;
		break;
#if(BOARD_SIZE >= 10)
            } else if(*p=='x' || *p=='X') { /* [HGM] X means 10 */
                p++; emptycount=10;
                if (j + emptycount > gameInfo.boardWidth) return FALSE;
                while (emptycount--)
                        board[i][(j++)+gameInfo.holdingsWidth] = EmptySquare;
#endif
            } else if (isdigit(*p)) {
		emptycount = *p++ - '0';
                while(isdigit(*p)) emptycount = 10*emptycount + *p++ - '0'; /* [HGM] allow > 9 */
                if (j + emptycount > gameInfo.boardWidth) return FALSE;
                while (emptycount--)
                        board[i][(j++)+gameInfo.holdingsWidth] = EmptySquare;
            } else if (*p == '+' || isalpha(*p)) {
                if (j >= gameInfo.boardWidth) return FALSE;
                if(*p=='+') {
                    piece = CharToPiece(*++p);
                    if(piece == EmptySquare) return FALSE; /* unknown piece */
                    piece = (ChessSquare) (PROMOTED piece ); p++;
                    if(PieceToChar(piece) != '+') return FALSE; /* unpromotable piece */
                } else piece = CharToPiece(*p++);

                if(piece==EmptySquare) return FALSE; /* unknown piece */
                if(*p == '~') { /* [HGM] make it a promoted piece for Crazyhouse */
                    piece = (ChessSquare) (PROMOTED piece);
                    if(PieceToChar(piece) != '~') return FALSE; /* cannot be a promoted piece */
                    p++;
                }
                board[i][(j++)+gameInfo.holdingsWidth] = piece;
	    } else {
		return FALSE;
	    }
	}
    }
    while (*p == '/' || *p == ' ') p++;

    /* [HGM] look for Crazyhouse holdings here */
    while(*p==' ') p++;
    if( gameInfo.holdingsWidth && p[-1] == '/' || *p == '[') {
        if(*p == '[') p++;
        if(*p == '-' ) *p++; /* empty holdings */ else {
            if( !gameInfo.holdingsWidth ) return FALSE; /* no room to put holdings! */
            /* if we would allow FEN reading to set board size, we would   */
            /* have to add holdings and shift the board read so far here   */
            while( (piece = CharToPiece(*p) ) != EmptySquare ) {
                *p++;
                if((int) piece >= (int) BlackPawn ) {
                    i = (int)piece - (int)BlackPawn;
		    i = PieceToNumber((ChessSquare)i);
                    if( i >= gameInfo.holdingsSize ) return FALSE;
                    board[BOARD_HEIGHT-1-i][0] = piece; /* black holdings */
                    board[BOARD_HEIGHT-1-i][1]++;       /* black counts   */
                } else {
                    i = (int)piece - (int)WhitePawn;
		    i = PieceToNumber((ChessSquare)i);
                    if( i >= gameInfo.holdingsSize ) return FALSE;
                    board[i][BOARD_WIDTH-1] = piece;    /* white holdings */
                    board[i][BOARD_WIDTH-2]++;          /* black holdings */
                }
            }
        }
        if(*p == ']') *p++;
    }

    while(*p == ' ') p++;

    /* Active color */
    switch (*p++) {
      case 'w':
        *blackPlaysFirst = FALSE;
	break;
      case 'b': 
	*blackPlaysFirst = TRUE;
	break;
      default:
	return FALSE;
    }

    /* [HGM] We NO LONGER ignore the rest of the FEN notation */
    /* return the extra info in global variiables             */

    /* set defaults in case FEN is incomplete */
    FENepStatus = EP_UNKNOWN;
    for(i=0; i<nrCastlingRights; i++ ) {
        FENcastlingRights[i] =
            gameInfo.variant == VariantFischeRandom || gameInfo.variant == VariantCapaRandom ? -1 : initialRights[i];
    }   /* assume possible unless obviously impossible */
    if(initialRights[0]>=0 && board[castlingRank[0]][initialRights[0]] != WhiteRook) FENcastlingRights[0] = -1;
    if(initialRights[1]>=0 && board[castlingRank[1]][initialRights[1]] != WhiteRook) FENcastlingRights[1] = -1;
    if(initialRights[2]>=0 && board[castlingRank[2]][initialRights[2]] != WhiteKing) FENcastlingRights[2] = -1;
    if(initialRights[3]>=0 && board[castlingRank[3]][initialRights[3]] != BlackRook) FENcastlingRights[3] = -1;
    if(initialRights[4]>=0 && board[castlingRank[4]][initialRights[4]] != BlackRook) FENcastlingRights[4] = -1;
    if(initialRights[5]>=0 && board[castlingRank[5]][initialRights[5]] != BlackKing) FENcastlingRights[5] = -1;
    FENrulePlies = 0;

    while(*p==' ') p++;
    if(nrCastlingRights) {
      if(*p=='K' || *p=='Q' || *p=='k' || *p=='q' || *p=='-') {
          /* castling indicator present, so default becomes no castlings */
          for(i=0; i<nrCastlingRights; i++ ) {
                 FENcastlingRights[i] = -1;
          }
      }
      while(*p=='K' || *p=='Q' || *p=='k' || *p=='q' || *p=='-' ||
             (gameInfo.variant == VariantFischeRandom || gameInfo.variant == VariantCapaRandom) &&
             ( *p >= 'a' && *p < 'a' + gameInfo.boardWidth) ||
             ( *p >= 'A' && *p < 'A' + gameInfo.boardWidth)   ) {
        char c = *p++; int whiteKingFile=-1, blackKingFile=-1;

        for(i=BOARD_LEFT; i<BOARD_RGHT; i++) {
            if(board[BOARD_HEIGHT-1][i] == BlackKing) blackKingFile = i;
            if(board[0             ][i] == WhiteKing) whiteKingFile = i;
        }
        switch(c) {
          case'K':
              for(i=BOARD_RGHT-1; board[0][i]!=WhiteRook && i>whiteKingFile; i--);
              FENcastlingRights[0] = i != whiteKingFile ? i : -1;
              FENcastlingRights[2] = whiteKingFile;
              break;
          case'Q':
              for(i=BOARD_LEFT; board[0][i]!=WhiteRook && i<whiteKingFile; i++);
              FENcastlingRights[1] = i != whiteKingFile ? i : -1;
              FENcastlingRights[2] = whiteKingFile;
              break;
          case'k':
              for(i=BOARD_RGHT-1; board[BOARD_HEIGHT-1][i]!=BlackRook && i>blackKingFile; i--);
              FENcastlingRights[3] = i != blackKingFile ? i : -1;
              FENcastlingRights[5] = blackKingFile;
              break;
          case'q':
              for(i=BOARD_LEFT; board[BOARD_HEIGHT-1][i]!=BlackRook && i<blackKingFile; i++);
              FENcastlingRights[4] = i != blackKingFile ? i : -1;
              FENcastlingRights[5] = blackKingFile;
          case '-':
              break;
          default: /* FRC castlings */
              if(c >= 'a') { /* black rights */
                  for(i=BOARD_LEFT; i<BOARD_RGHT; i++)
                    if(board[BOARD_HEIGHT-1][i] == BlackKing) break;
                  if(i == BOARD_RGHT) break;
                  FENcastlingRights[5] = i;
                  c -= AAA;
                  if(board[BOARD_HEIGHT-1][c] <  BlackPawn ||
                     board[BOARD_HEIGHT-1][c] >= BlackKing   ) break;
                  if(c > i)
                      FENcastlingRights[3] = c;
                  else
                      FENcastlingRights[4] = c;
              } else { /* white rights */
                  for(i=BOARD_LEFT; i<BOARD_RGHT; i++)
                    if(board[0][i] == WhiteKing) break;
                  if(i == BOARD_RGHT) break;
                  FENcastlingRights[2] = i;
                  c -= AAA - 'a' + 'A';
                  if(board[0][c] >= WhiteKing) break;
                  if(c > i)
                      FENcastlingRights[0] = c;
                  else
                      FENcastlingRights[1] = c;
              }
        }
      }
    if (appData.debugMode) {
        fprintf(debugFP, "FEN castling rights:");
        for(i=0; i<nrCastlingRights; i++)
        fprintf(debugFP, " %d", FENcastlingRights[i]);
        fprintf(debugFP, "\n");
    }

      while(*p==' ') p++;
    }

    /* read e.p. field in games that know e.p. capture */
    if(gameInfo.variant != VariantShogi    && gameInfo.variant != VariantXiangqi &&
       gameInfo.variant != VariantShatranj && gameInfo.variant != VariantCourier ) { 
      if(*p=='-') {
        p++; FENepStatus = EP_NONE;
      } else {
         char c = *p++ - AAA;

         if(c < BOARD_LEFT || c >= BOARD_RGHT) return TRUE;
         if(*p >= '0' && *p <='9') *p++;
         FENepStatus = c;
      }
    }


    if(sscanf(p, "%d", &i) == 1) {
        FENrulePlies = i; /* 50-move ply counter */
        /* (The move number is still ignored)    */
    }

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
          /* [HGM] copy FEN attributes as well */
          {   int i;
              initialRulePlies = FENrulePlies;
              epStatus[0] = FENepStatus;
              for( i=0; i<nrCastlingRights; i++ )
                  castlingRights[0][i] = FENcastlingRights[i];
          }
      EditPositionDone();
      DisplayBothClocks();
      DrawPosition(FALSE, boards[currentMove]);
    }
  }
}
