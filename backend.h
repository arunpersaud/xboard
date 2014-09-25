/*
 * backend.h -- Interface exported by XBoard back end
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

#ifndef XB_BACKEND
#define XB_BACKEND

#include "lists.h"

typedef int (*FileProc) P((FILE *f, int n, char *title));

extern char *wbOptions;
extern int gotPremove;
extern GameMode gameMode;
extern int matchMode;
extern int matchGame;
extern int pausing, cmailMsgLoaded, flipView, mute;
extern char white_holding[], black_holding[];
extern int currentMove, backwardMostMove, forwardMostMove;
extern int blackPlaysFirst;
extern FILE *debugFP;
extern char* programVersion;
extern Board boards[];
extern char marker[BOARD_RANKS][BOARD_FILES];
extern char lastMsg[MSG_SIZ];
extern Boolean bookUp;
extern Boolean addToBookFlag;
extern int tinyLayout, smallLayout;
extern Boolean mcMode;
extern int dragging;
extern char variantError[];
extern char lastTalker[];

void MarkMenuItem P((char *menuRef, int state));
char *CmailMsg P((void));
/* Tord: Added the useFEN960 parameter in PositionToFEN() below */
char *PositionToFEN P((int move, char* useFEN960, int moveCounts));
void AlphaRank P((char *s, int n)); /* [HGM] Shogi move preprocessor */
void EditPositionPasteFEN P((char *fen));
void TimeDelay P((long ms));
void SendMultiLineToICS P(( char *text ));
void AnalysisPeriodicEvent P((int force));
void SetWhiteToPlayEvent P((void));
void SetBlackToPlayEvent P((void));
void UploadGameEvent P((void));
void InitBackEnd1 P((void));
void InitBackEnd2 P((void));
int HasPromotionChoice P((int fromX, int fromY, int toX, int toY, char *choice, int sweep));
int InPalace P((int row, int column));
int PieceForSquare P((int x, int y));
int OKToStartUserMove P((int x, int y));
void Reset P((int redraw, int init));
void ResetGameEvent P((void));
Boolean HasPattern P(( const char * text, const char * pattern ));
Boolean SearchPattern P(( const char * text, const char * pattern ));
int LoadGame P((FILE *f, int n, char *title, int useList));
int LoadGameFromFile P((char *filename, int n, char *title, int useList));
int CmailLoadGame P((FILE *f, int n, char *title, int useList));
int ReloadGame P((int offset));
int SaveSelected P((FILE *f, int dummy, char *dummy2));
int SaveGame P((FILE *f, int dummy, char *dummy2));
int SaveGameToFile P((char *filename, int append));
int LoadPosition P((FILE *f, int n, char *title));
int ReloadPosition P((int offset));
int SavePosition P((FILE *f, int dummy, char *dummy2));
int DrawSeekGraph P(());
int SeekGraphClick P((ClickType click, int x, int y, int moving));
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
void RevertEvent P((Boolean annotate));
void RetractMoveEvent P((void));
void MoveNowEvent P((void));
void TruncateGameEvent P((void));
void PauseEvent P((void));
void CallFlagEvent P((void));
void ClockClick P((int which));
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
void ShowThinkingEvent P(());
void PeriodicUpdatesEvent P((int newState));
void HintEvent P((void));
void BookEvent P((void));
void AboutGameEvent P((void));
void ExitEvent P((int status));
char *DefaultFileName P((char *));
ChessMove UserMoveTest P((int fromX, int fromY, int toX, int toY, int promoChar, Boolean captureOwn));
void UserMoveEvent P((int fromX, int fromY, int toX, int toY, int promoChar));
void DecrementClocks P((void));
char *TimeString P((long millisec));
void AutoPlayGameLoop P((void));
void AdjustClock P((Boolean which, int dir));
void DisplayBothClocks P((void));
void EditPositionMenuEvent P((ChessSquare selection, int x, int y));
void DropMenuEvent P((ChessSquare selection, int x, int y));
int ParseTimeControl P((char *tc, float ti, int mps));
void EscapeExpand(char *p, char *q);
void ProcessICSInitScript P((FILE * f));
void EditCommentEvent P((void));
void ReplaceComment P((int index, char *text));
int ReplaceTags P((char *tags, GameInfo *gi));/* returns nonzero on error */
void AppendComment P((int index, char *text, Boolean addBraces));
void LoadVariation P((int index, char *text));
void ReloadCmailMsgEvent P((int unregister));
void MailMoveEvent P((void));
void EditTagsEvent P((void));
void GetMoveListEvent P((void));
void ExitAnalyzeMode P((void));
int  AnalyzeModeEvent P((void));
void AnalyzeFileEvent P((void));
void MatchEvent P((int mode));
void RecentEngineEvent P((int nr));
void TypeInEvent P((char first));
void TypeInDoneEvent P((char *move));
void InitPosition P((int redraw));
void NewSettingEvent P((int option, int *feature, char *command, int value));
void SettingsMenuIfReady P((void));
void DoEcho P((void));
void DontEcho P((void));
void TidyProgramName P((char *prog, char *host, char *buf));
void SetGameInfo P((void));
void AskQuestionEvent P((char *title, char *question,
			 char *replyPrefix, char *which));
Boolean ParseOneMove P((char *move, int moveNum,
			ChessMove *moveType, int *fromX, int *fromY,
			int *toX, int *toY, char *promoChar));
char *VariantName P((VariantClass v));
VariantClass StringToVariant P((char *e));
double u64ToDouble P((u64 value));
void OutputChatMessage P((int partner, char *mess));
void EditPositionDone P((Boolean fakeRights));
Boolean GetArgValue P((char *name));
Boolean LoadPV P((int x, int y));
Boolean LoadMultiPV P((int x, int y, char *buf, int index, int *start, int *end, int pane));
void UnLoadPV P(());
void MovePV P((int x, int y, int h));
int PromoScroll P((int x, int y));
void EditBookEvent P((void));
Boolean DisplayBook P((int moveNr));
void SaveToBook P((char *text));
void AddBookMove P((char *text));
void PlayBookMove P((char *text, int index));
void HoverEvent P((int hiX, int hiY, int x, int y));
int PackGame P((Board board));
Boolean ParseFEN P((Board board, int *blackPlaysFirst, char *fen, Boolean autoSize));
void ApplyMove P((int fromX, int fromY, int toX, int toY, int promoChar, Board board));
void PackMove P((int fromX, int fromY, int toX, int toY, ChessSquare promoPiece));
void ics_printf P((char *format, ...));
int GetEngineLine P((char *nick, int engine));
void AddGameToBook P((int always));
void FlushBook P((void));

char *StrStr P((char *string, char *match));
char *StrCaseStr P((char *string, char *match));
char *StrSave P((char *s));
char *StrSavePtr P((char *s, char **savePtr));
char *SavePart P((char *));
char* safeStrCpy P(( char *dst, const char *src, size_t count ));

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
enum ICS_TYPE { ICS_GENERIC, ICS_ICC, ICS_FICS, ICS_CHESSNET /* not really supported */ };
enum ICS_TYPE ics_type;

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
typedef struct XB_ListGame {
    ListNode node;
    int number;
    int position;
    int moves;
    unsigned long offset;   /*  Byte offset of game within file.     */
    GameInfo gameInfo;      /*  Note that some entries may be NULL. */
} ListGame;

extern int border;
extern int doubleClick;
extern int storedGames;
extern int opponentKibitzes;
extern ChessSquare gatingPiece;
extern List gameList;
extern int lastLoadGameNumber;
void ClearGameInfo P((GameInfo *));
int GameListBuild P((FILE *));
void GameListInitGameInfo P((GameInfo *));
char *GameListLine P((int, GameInfo *));
char * GameListLineFull P(( int, GameInfo *));
void InitSearch P((void));
int GameContainsPosition P((FILE *f, ListGame *lg));
void GLT_TagsToList P(( char * tags ));
void GLT_ParseList P((void));
int NamesToList P((char *name, char **engines, char **mnemonics, char *group));
int CreateTourney P((char *name));
char *MakeName P((char *templ));
void SwapEngines P((int n));
void Substitute P((char *participants, int expunge));

extern char* StripHighlight P((char *));  /* returns static data */
extern char* StripHighlightAndTitle P((char *));  /* returns static data */
extern void ics_update_width P((int new_width));
extern Boolean set_cont_sequence P((char *new_seq));
extern int wrap P((char *dest, char *src, int count, int width, int *lp));
int Explode P((Board board, int fromX, int fromY, int toX, int toY));

typedef enum { CheckBox, ComboBox, TextBox, Button, Spin, ResetButton, SaveButton, ListBox, Graph, PopUp,
		 FileName, PathName, Slider, Message, Fractional, Label, Icon,
		 BoxBegin, BoxEnd, BarBegin, BarEnd, DropDown, Break, EndMark, Skip } Control;

typedef struct XB_OPT {   // [HGM] options: descriptor of UCI-style option
    int value;          // current setting, starts as default
    int min;		// Also used for flags
    int max;
    void *handle;       // for use by front end
    void *target;       // for use by front end
    char *textValue;    // points to beginning of text value in name field
    char **choice;      // points to array of combo choices in cps->combo
    Control type;
    char *name;         // holds both option name and text value (in allocated memory)
    char **font;
} Option;

typedef struct XB_CPS {
    char *which;
    int maybeThinking;
    ProcRef pr;
    InputSourceRef isr;
    char *twoMachinesColor; /* "white\n" or "black\n" */
    char *program;
    char *host;
    char *dir;
    struct XB_CPS *other;
    char *initString;
    char *computerString;
    int sendTime; /* 0=don't, 1=do, 2=test */
    int sendDrawOffers;
    int useSigint;
    int useSigterm;
    int offeredDraw; /* countdown */
    int reuse;
    int useSetboard; /* 0=use "edit"; 1=use "setboard" */
    int extendedEdit;/* 1=also set holdings with "edit" */
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
    int excludeMoves;/* 0=don't use "exclude" command; 1=do */
    char *tidy;
    int matchWins;
    char *variants;
    int analysisSupport;
    int analyzing;
    int protocolVersion;
    int initDone;
    int pseudo;

    /* Added by Tord: */
    int useFEN960;   /* 0=use "KQkq" style FENs, 1=use "HAha" style FENs */
    int useOOCastle; /* 0="O-O" notation for castling, 1="king capture rook" notation */
    /* End of additions by Tord */

    int scoreIsAbsolute; /* [AS] 0=don't know (standard), 1=score is always from white side */
    int isUCI;           /* [AS] 0=no (Winboard), 1=UCI (requires Polyglot) */
    int hasOwnBookUCI;   /* [AS] 0=use GUI or Polyglot book, 1=has own book */

    /* [HGM] time odds */
    float timeOdds; /* factor through which we divide time for this engine  */
    int debug;      /* [HGM] ignore engine debug lines starting with '#'    */
    int maxNrOfSessions; /* [HGM] secondary TC: max args in 'level' command */
    int accumulateTC; /* [HGM] secondary TC: how to handle extra sessions   */
    int drawDepth;    /* [HGM] egbb: search depth to play egbb draws        */
    int nps;          /* [HGM] nps: factor for node count to replace time   */
    int supportsNPS;
    int alphaRank;    /* [HGM] shogi: engine uses shogi-type coordinates    */
    int maxCores;     /* [HGM] SMP: engine understands cores command        */
    int memSize;      /* [HGM] memsize: engine understands memory command   */
    char *egtFormats; /* [HGM] EGT: supported tablebase formats             */
    int bookSuspend;  /* [HGM] book: go was deferred because of book hit    */
    int pause;        /* [HGM] pause: 1=supports it, 2=actually paused      */
    int highlight;    /* [HGM] engine wants to get lift and put commands    */
    int nrOptions;    /* [HGM] options: remembered option="..." features    */
#define MAX_OPTIONS 200
    Option option[MAX_OPTIONS];
    int comboCnt;
    char *comboList[20*MAX_OPTIONS];
    char *optionSettings;
    void *programLogo; /* [HGM] logo: bitmap of the logo                    */
    char *fenOverride; /* [HGM} FRC: force FEN casling & ep fields by hand  */
    char userError;    /* [HGM] crash: flag to suppress fatal-error messages*/
    char reload;       /* [HGM] options: flag to resend options with xreuse */
} ChessProgramState;

extern ChessProgramState first, second;

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
extern Boolean shuffleOpenings;
extern ChessProgramStats programStats;
extern int opponentKibitzes; // used by wengineo.c
extern int errorExitStatus;
extern char *recentEngines;
extern char *comboLine;
extern Boolean partnerUp, twoBoards;
extern char engineVariant[];
char *EngineDefinedVariant P((ChessProgramState *cps, int n));
void SettingsPopUp P((ChessProgramState *cps)); // [HGM] really in front-end, but CPS not known in frontend.h
int WaitForEngine P((ChessProgramState *cps, DelayedEventCallback x));
void Load P((ChessProgramState *cps, int n));
int MultiPV P((ChessProgramState *cps));
void MoveHistorySet P(( char movelist[][2*MOVE_LEN], int first, int last, int current, ChessProgramStats_Move * pvInfo ));
void MakeEngineOutputTitle P((void));
void LoadTheme P((void));
void CreateBookEvent P((void));
char *SupportedVariant P((char *list, VariantClass v, int w, int h, int s, int proto, char *engine));
char *CollectPieceDescriptors P((void));


/* A point in time */
typedef struct {
    long sec;  /* Assuming this is >= 32 bits */
    int ms;    /* Assuming this is >= 16 bits */
} TimeMark;

extern TimeMark programStartTime;

void GetTimeMark P((TimeMark *));
long SubtractTimeMarks P((TimeMark *, TimeMark *));

#endif /* XB_BACKEND */
