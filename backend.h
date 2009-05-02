/*
 * backend.h -- Interface exported by XBoard back end
 * $Id: backend.h,v 2.1 2003/10/27 19:21:00 mann Exp $
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts.
 * Enhancements Copyright 1992-95 Free Software Foundation, Inc.
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
 */
#ifndef _BACKEND
#define _BACKEND

/* unsigned int 64 for engine nodes work and display */
#ifdef WIN32
       /* I don't know the name for this type of other compiler
        * If it not work, just modify here
        * This is for MS Visual Studio
        */
       #ifdef _MSC_VER
               #define u64 unsigned __int64
               #define s64 signed __int64
               #define u64Display "%I64u"
               #define s64Display "%I64d"
               #define u64Const(c) (c ## UI64)
               #define s64Const(c) (c ## I64)
       #else
               /* place holder
                * or dummy types for other compiler
                */
               #define u64 unsigned long long
               #define s64 signed long long
               #define u64Display "%llu"
               #define s64Display "%lld"
               #define u64Const(c) (c ## ULL)
               #define s64Const(c) (c ## LL)
       #endif
#else
       /* GNU gcc */
       #define u64 unsigned long long
       #define s64 signed long long
       #define u64Display "%llu"
       #define s64Display "%lld"
       #define u64Const(c) (c ## ull)
       #define s64Const(c) (c ## ll)
#endif

#include "lists.h"
#include "frontend.h"

extern int gotPremove;
extern GameMode gameMode;
extern int pausing, cmailMsgLoaded, flipView;
extern char white_holding[], black_holding[];
extern int currentMove, backwardMostMove, forwardMostMove;
extern int blackPlaysFirst;
extern FILE *debugFP;
extern char* programVersion;
extern ProcRef firstProgramPR, secondProgramPR;
extern Board boards[];

char *CmailMsg P((void));
/* Tord: Added the useFEN960 parameter in PositionToFEN() below */
char *PositionToFEN P((int move, int useFEN960));
void AlphaRank P((char *s, int n)); /* [HGM] Shogi move preprocessor */
void EditPositionPasteFEN P((char *fen));
void TimeDelay P((long ms));
void SendMultiLineToICS P(( char *text ));
void AnalysisPeriodicEvent P((int force));
void SetWhiteToPlayEvent P((void));
void SetBlackToPlayEvent P((void));
void InitBackEnd1 P((void));
void InitBackEnd2 P((void));
int IsPromotion P((int fromX, int fromY, int toX, int toY));
int InPalace P((int row, int column));
int PieceForSquare P((int x, int y));
int OKToStartUserMove P((int x, int y));
void Reset P((int redraw, int init));
void ResetGameEvent P((void));
int LoadGame P((FILE *f, int n, char *title, int useList));
int LoadGameFromFile P((char *filename, int n, char *title, int useList));
int CmailLoadGame P((FILE *f, int n, char *title, int useList));
int ReloadGame P((int offset));
int SaveGame P((FILE *f, int dummy, char *dummy2));
int SaveGameToFile P((char *filename, int append));
int LoadPosition P((FILE *f, int n, char *title));
int ReloadPosition P((int offset));
int SavePosition P((FILE *f, int dummy, char *dummy2));
void EditPositionEvent P((void));
void FlipViewEvent P((void));
void MachineWhiteEvent P((void));
void MachineBlackEvent P((void));
void TwoMachinesEvent P((void));
void EditGameEvent P((void));
void TrainingEvent P((void));
void IcsClientEvent P((void));
void ForwardEvent P((void));
void BackwardEvent P((void));
void ToEndEvent P((void));
void ToStartEvent P((void));
void ToNrEvent P((int to));
void RevertEvent P((void));
void RetractMoveEvent P((void));
void MoveNowEvent P((void));
void TruncateGameEvent P((void));
void PauseEvent P((void));
void CallFlagEvent P((void));
void AcceptEvent P((void));
void DeclineEvent P((void));
void RematchEvent P((void));
void DrawEvent P((void));
void AbortEvent P((void));
void AdjournEvent P((void));
void ResignEvent P((void));
void UserAdjudicationEvent P((int result));
void StopObservingEvent P((void));
void StopExaminingEvent P((void));
void PonderNextMoveEvent P((int newState));
void NewSettingeEvent P((int option, char *command, int value));
void ShowThinkingEvent P(());
void PeriodicUpdatesEvent P((int newState));
void HintEvent P((void));
void BookEvent P((void));
void AboutGameEvent P((void));
void ExitEvent P((int status));
char *DefaultFileName P((char *));
ChessMove UserMoveTest P((int fromX, int fromY, int toX, int toY, int promoChar));
void UserMoveEvent P((int fromX, int fromY, int toX, int toY, int promoChar));
void DecrementClocks P((void));
char *TimeString P((long millisec));
void AutoPlayGameLoop P((void));
void AdjustClock P((Boolean which, int dir));
void DisplayBothClocks P((void));
void EditPositionMenuEvent P((ChessSquare selection, int x, int y));
void DropMenuEvent P((ChessSquare selection, int x, int y));
int ParseTimeControl P((char *tc, int ti, int mps));
void ProcessICSInitScript P((FILE * f));
void EditCommentEvent P((void));
void ReplaceComment P((int index, char *text));
int ReplaceTags P((char *tags, GameInfo *gi));/* returns nonzero on error */
void AppendComment P((int index, char *text));
void ReloadCmailMsgEvent P((int unregister));
void MailMoveEvent P((void));
void EditTagsEvent P((void));
void GetMoveListEvent P((void));
void ExitAnalyzeMode P((void));
void AnalyzeModeEvent P((void));
void AnalyzeFileEvent P((void));
void DoEcho P((void));
void DontEcho P((void));
void TidyProgramName P((char *prog, char *host, char *buf));
void AskQuestionEvent P((char *title, char *question,
			 char *replyPrefix, char *which));
Boolean ParseOneMove P((char *move, int moveNum,
			ChessMove *moveType, int *fromX, int *fromY,
			int *toX, int *toY, char *promoChar));
char *VariantName P((VariantClass v));
VariantClass StringToVariant P((char *e));
double u64ToDouble P((u64 value));

char *StrStr P((char *string, char *match));
char *StrCaseStr P((char *string, char *match));
char *StrSave P((char *s));
char *StrSavePtr P((char *s, char **savePtr));

#ifndef _amigados
int StrCaseCmp P((char *s1, char *s2));
int ToLower P((int c));
int ToUpper P((int c));
#else
#define StrCaseCmp Stricmp  /*  Use utility.library functions   */
#include <proto/utility.h>
#endif

extern GameInfo gameInfo;

/* ICS vars used with backend.c and zippy.c */
#define ICS_GENERIC 0
#define ICS_ICC 1
#define ICS_FICS 2
#define ICS_CHESSNET 3 /* not really supported */
int ics_type;

 

/* pgntags.c prototypes
 */
char *PGNTags P((GameInfo *));
void PrintPGNTags P((FILE *f, GameInfo *));
int ParsePGNTag P((char *, GameInfo *));
char *PGNResult P((ChessMove result));


/* gamelist.c prototypes
 */
/* A game node in the double linked list of games.
 */
typedef struct _ListGame {
    ListNode node;
    int number;
    unsigned long offset;   /*  Byte offset of game within file.     */
    GameInfo gameInfo;      /*  Note that some entries may be NULL. */
} ListGame;
 
extern List gameList;
void ClearGameInfo P((GameInfo *));
int GameListBuild P((FILE *));
void GameListInitGameInfo P((GameInfo *));
char *GameListLine P((int, GameInfo *));
char * GameListLineFull P(( int, GameInfo *));

extern char* StripHighlight P((char *));  /* returns static data */
extern char* StripHighlightAndTitle P((char *));  /* returns static data */

typedef enum { CheckBox, ComboBox, TextBox, Button, Spin, SaveButton } Control;

typedef struct _OPT {   // [HGM] options: descriptor of UCI-style option
    int value;          // current setting, starts as default
    int min;
    int max;
    void *handle;       // for use by front end
    char *textValue;    // points to beginning of text value in name field
    char **choice;      // points to array of combo choices in cps->combo
    Control type;
    char name[MSG_SIZ]; // holds both option name and text value
} Option;

typedef struct _CPS {
    char *which;
    int maybeThinking;
    ProcRef pr;
    InputSourceRef isr;
    char *twoMachinesColor; /* "white\n" or "black\n" */
    char *program;
    char *host;
    char *dir;
    struct _CPS *other;
    char *initString;
    char *computerString;
    int sendTime; /* 0=don't, 1=do, 2=test */
    int sendDrawOffers;
    int useSigint;
    int useSigterm;
    int offeredDraw; /* countdown */
    int reuse;
    int useSetboard; /* 0=use "edit"; 1=use "setboard" */
    int useSAN;      /* 0=use coordinate notation; 1=use SAN */
    int usePing;     /* 0=not OK to use ping; 1=OK */
    int lastPing;
    int lastPong;
    int usePlayother;/* 0=not OK to use playother; 1=OK */
    int useColors;   /* 0=avoid obsolete white/black commands; 1=use them */
    int useUsermove; /* 0=just send move; 1=send "usermove move" */
    int sendICS;     /* 0=don't use "ics" command; 1=do */
    int sendName;    /* 0=don't use "name" command; 1=do */
    int sdKludge;    /* 0=use "sd DEPTH" command; 1=use "depth\nDEPTH" */
    int stKludge;    /* 0=use "st TIME" command; 1=use "level 1 TIME" */
    char tidy[MSG_SIZ];
    int matchWins;
    char variants[MSG_SIZ];
    int analysisSupport;
    int analyzing;
    int protocolVersion;
    int initDone;

    /* Added by Tord: */
    int useFEN960;   /* 0=use "KQkq" style FENs, 1=use "HAha" style FENs */
    int useOOCastle; /* 0="O-O" notation for castling, 1="king capture rook" notation */
    /* End of additions by Tord */

    int scoreIsAbsolute; /* [AS] 0=don't know (standard), 1=score is always from white side */
    int isUCI;           /* [AS] 0=no (Winboard), 1=UCI (requires Polyglot) */
    int hasOwnBookUCI;   /* [AS] 0=use GUI or Polyglot book, 1=has own book */

    /* [HGM] time odds */
    int timeOdds;   /* factor through which we divide time for this engine  */
    int debug;      /* [HGM] ignore engine debug lines starting with '#'    */
    int maxNrOfSessions; /* [HGM] secondary TC: max args in 'level' command */
    int accumulateTC; /* [HGM] secondary TC: how to handle extra sessions   */
    int nps;          /* [HGM] nps: factor for node count to replace time   */
    int supportsNPS;
    int alphaRank;    /* [HGM] shogi: engine uses shogi-type coordinates    */
    int maxCores;     /* [HGM] SMP: engine understands cores command        */
    int memSize;      /* [HGM] memsize: engine understands memory command   */
    char egtFormats[MSG_SIZ];     /* [HGM] EGT: supported tablebase formats */
    int bookSuspend;  /* [HGM] book: go was deferred because of book hit    */
    int nrOptions;    /* [HGM] options: remembered option="..." features    */
#define MAX_OPTIONS 50
    Option option[MAX_OPTIONS];
    int comboCnt;
    char *comboList[10*MAX_OPTIONS];
    void *programLogo; /* [HGM] logo: bitmap of the logo                    */
} ChessProgramState;

extern ChessProgramState first, second;

/* [AS] Search stats from chessprogram, for the played move */
typedef struct {
    int score;  /* Centipawns */
    int depth;  /* Plies */
    int time;   /* Milliseconds */
} ChessProgramStats_Move;

/* Search stats from chessprogram */
typedef struct {
  char movelist[2*MSG_SIZ]; /* Last PV we were sent */
  int depth;              /* Current search depth */
  int nr_moves;           /* Total nr of root moves */
  int moves_left;         /* Moves remaining to be searched */
  char move_name[MOVE_LEN];  /* Current move being searched, if provided */
  u64 nodes;    /* # of nodes searched */
  int time;               /* Search time (centiseconds) */
  int score;              /* Score (centipawns) */
  int got_only_move;      /* If last msg was "(only move)" */
  int got_fail;           /* 0 - nothing, 1 - got "--", 2 - got "++" */
  int ok_to_send;         /* handshaking between send & recv */
  int line_is_book;       /* 1 if movelist is book moves */
  int seen_stat;          /* 1 if we've seen the stat01: line */
} ChessProgramStats;

extern ChessProgramStats_Move pvInfoList[MAX_MOVES];
extern shuffleOpenings;
extern ChessProgramStats programStats;

#endif /* _BACKEND */
