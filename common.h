/*
 * common.h -- Common definitions for X and Windows NT versions of XBoard
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

#ifndef XB_COMMON
#define XB_COMMON


/* Begin compatibility grunge  */

#if defined(__STDC__) || defined(WIN32) || defined(_amigados)
#define	P(args)	args
typedef void *VOIDSTAR;
#else
#define P(args)		()
typedef char *VOIDSTAR;
#endif

#ifdef WIN32
typedef char Boolean;
typedef char *String;
#define popen _popen
#define pclose _pclose

#else
#ifdef _amigados        /*  It is important, that these types have  */
typedef int Boolean;    /*  a length of 4 bytes each, as we are     */
typedef char *String;   /*  using ReadArgs() for argument parsing.  */
#ifdef _DCC
FILE *popen(const char *, const char *);
int pclose(FILE *);
#endif

#else
#ifdef X11
#include <X11/Intrinsic.h>
#else
typedef char Boolean;
typedef char *String;
#define True 1
#define False 0
#endif
#endif
#endif


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define UNKNOWN -1 /* [HGM] nps */

#if !HAVE_RANDOM
# if HAVE_RAND48
#  define srandom srand48
#  define random lrand48
# else /* not HAVE_RAND48 */
#  define srandom srand
#  define random rand
# endif /* not HAVE_RAND48 */
#endif /* !HAVE_RANDOM */

/* End compatibility grunge */

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
                * [HGM] seems that -mno-cygwin comple needs %I64?
                */
               #define u64 unsigned long long
               #define s64 signed long long
               #ifdef USE_I64
                  #define u64Display "%I64u"
                  #define s64Display "%I64d"
               #else
                  #define u64Display "%llu"
                  #define s64Display "%lld"
               #endif
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

#define PROTOVER                2       /* engine protocol version */

// [HGM] license: Messages that engines must print to satisfy their license requirements for patented variants
#define GOTHIC "Gothic Chess (see www.GothicChess.com) is licensed under U.S. Patent #6,481,716 by Ed Trice"
#define FALCON "Falcon Chess (see www.chessvariants.com) is licensed under U.S. Patent #5,690,334 by George W. Duke"

/* [HGM] Some notes about board sizes:
   In games that allow piece drops, the holdings are considered part of the
   board, in the leftmost and rightmost two files. This way they are
   automatically part of the game-history states, and enjoy all display
   functions (including drag-drop and click-click moves to the regular part
   of the board). The drawback of this is that the internal numbering of
   files starts at 2 for the a-file if holdings are displayed. To ensure
   consistency, this shifted numbering system is used _everywhere_ in the
   code, and conversion to the 'normal' system only takes place when the
   file number is converted to or from ASCII (by redefining the character
   constant 'a'). This works because Winboard only communicates with the
   outside world in ASCII. In a similar way, the different rank numbering
   systems (starting at rank 0 or 1) are implemented by redefining '1'.
*/
#define BOARD_RANKS             17            /* [HGM] for in declarations  */
#define BOARD_FILES             16             /* [HGM] for in declarations  */
#define BOARD_HEIGHT (gameInfo.boardHeight)    /* [HGM] made user adjustable */
#define BOARD_WIDTH  (gameInfo.boardWidth + 2*gameInfo.holdingsWidth)
#define BOARD_LEFT   (gameInfo.holdingsWidth)  /* [HGM] play-board edges     */
#define BOARD_RGHT   (gameInfo.boardWidth + gameInfo.holdingsWidth)
#define CASTLING     (BOARD_RANKS-1)           /* [HGM] hide in upper rank   */
#define VIRGIN       (BOARD_RANKS-2)           /* [HGM] pieces not moved     */
#define TOUCHED_W    CASTLING][(BOARD_FILES-6) /* [HGM] in upper rank        */
#define TOUCHED_B    CASTLING][(BOARD_FILES-5) /* [HGM] in upper rank        */
#define EP_RANK      CASTLING][(BOARD_FILES-4) /* [HGM] in upper rank        */
#define EP_FILE      CASTLING][(BOARD_FILES-3) /* [HGM] in upper rank        */
#define EP_STATUS    CASTLING][(BOARD_FILES-2) /* [HGM] in upper rank        */
#define HOLDINGS_SET CASTLING][(BOARD_FILES-1) /* [HGM] in upper-right corner*/
#define ONE          ('1'-(BOARD_HEIGHT==10))  /* [HGM] foremost board rank  */
#define AAA          ('a'-BOARD_LEFT)          /* [HGM] leftmost board file  */
#define VIRGIN_W                 1             /* [HGM] flags in Board[VIRGIN][X] */
#define VIRGIN_B                 2
#define DROP_RANK               -3
#define MAX_MOVES		1000
#define MSG_SIZ			512
#define DIALOG_SIZE		256
#define STAR_MATCH_N            16
#define MOVE_LEN		32
#define TIME_CONTROL		"5"	/* in minutes */
#define TIME_DELAY_QUOTE	"1.0"	/* seconds between moves */
#define TIME_DELAY              ((float) 1.0)
#define MOVES_PER_SESSION	40	/* moves per TIME_CONTROL */
#define TIME_INCREMENT          -1      /* if >= 0, MOVES_PER_SESSION unused */
#define WhiteOnMove(move)	(((move) % 2) == 0)
#define ICS_HOST                "chessclub.com"
#define ICS_PORT	        "5000"
#define ICS_COMM_PORT           ""
#define FIRST_HOST		"localhost"
#define SECOND_HOST		"localhost"
#define TELNET_PROGRAM          "telnet"
#define DEF_BITMAP_DIR          BITMAPDIR
#define MATCH_MODE		"False"
#define INIT_STRING		"new\nrandom\n"
#define WHITE_STRING		"white\ngo\n"
#define BLACK_STRING		"black\ngo\n"
#define COMPUTER_STRING         "computer\n"
#define REUSE_CHESS_PROGRAMS    1
#define WHITE_PIECE_COLOR	"#FFFFCC"
#define BLACK_PIECE_COLOR	"#202020"
#define LIGHT_SQUARE_COLOR	"#C8C365"
#define DARK_SQUARE_COLOR	"#77A26D"
#define JAIL_SQUARE_COLOR       "#808080"
#define HIGHLIGHT_SQUARE_COLOR	"#FFFF00"
#define PREMOVE_HIGHLIGHT_COLOR	"#FF0000"
#define LOWTIMEWARNING_COLOR    "#FF0000"
#define BELLCHAR                '\007'
#define NULLCHAR                '\000'
#define FEATURE_TIMEOUT         10000 /*ms*/

#define CLOCK_FONT 0
#define MESSAGE_FONT 1
#define COORD_FONT 2
#define CONSOLE_FONT 3
#define COMMENT_FONT 4
#define EDITTAGS_FONT 5
#define MOVEHISTORY_FONT 6
#define GAMELIST_FONT 7
#define NUM_FONTS 8

/* Default to no flashing (the "usual" XBoard behavior) */
#define FLASH_COUNT	0		/* Number of times to flash */
#define FLASH_RATE	5		/* Flashes per second */

/* Default delay per character (in msec) while sending login script */
#define MS_LOGIN_DELAY  0

/* [AS] Support for background textures */
#define BACK_TEXTURE_MODE_DISABLED      0
#define BACK_TEXTURE_MODE_PLAIN         1
#define BACK_TEXTURE_MODE_FULL_RANDOM   2

/* Zippy defaults */
#define ZIPPY_TALK FALSE
#define ZIPPY_PLAY FALSE
#define ZIPPY_LINES "yow.lines"
#define ZIPPY_PINHEAD ""
#define ZIPPY_PASSWORD ""
#define ZIPPY_PASSWORD2 ""
#define ZIPPY_WRONG_PASSWORD ""
#define ZIPPY_ACCEPT_ONLY ""
#define ZIPPY_USE_I TRUE
#define ZIPPY_BUGHOUSE 0
#define ZIPPY_NOPLAY_CRAFTY FALSE
#define ZIPPY_GAME_END "gameend\n"
#define ZIPPY_GAME_START ""
#define ZIPPY_ADJOURN FALSE
#define ZIPPY_ABORT FALSE
#define ZIPPY_VARIANTS "normal,fischerandom,crazyhouse,losers,suicide,3checks,twokings,bughouse,shatranj"
#define ZIPPY_MAX_GAMES 0
#define ZIPPY_REPLAY_TIMEOUT 120

typedef VOIDSTAR ProcRef;
#define NoProc ((ProcRef) 0)
typedef VOIDSTAR InputSourceRef;

typedef void (*DelayedEventCallback) P((void));

typedef enum { Press, Release } ClickType;

typedef enum {
    BeginningOfGame, MachinePlaysWhite, MachinePlaysBlack,
    AnalyzeMode, AnalyzeFile, TwoMachinesPlay,
    EditGame, PlayFromGameFile, EndOfGame, EditPosition, Training,
    IcsIdle, IcsPlayingWhite, IcsPlayingBlack, IcsObserving,
    IcsExamining
  } GameMode;

typedef enum {
    /* [HGM] the order here is crucial for Crazyhouse & Shogi: */
    /* only the first N pieces can go into the holdings, and   */
    /* promotions in those variants shift P-W to U-S           */
    WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen,
    WhiteFerz, WhiteAlfil, WhiteAngel, WhiteMarshall, WhiteWazir, WhiteMan,
    WhiteCannon, WhiteNightrider, WhiteCardinal, WhiteDragon, WhiteGrasshopper,
    WhiteSilver, WhiteFalcon, WhiteLance, WhiteCobra, WhiteUnicorn, WhiteLion,
    WhiteTokin, WhiteDagger, WhitePCardinal, WhitePDragon, WhiteCat,
    WhitePSword, WhiteMonarch, WhiteMother, WhiteNothing, WhitePRook, WhitePDagger,
    WhiteDolphin, WhiteStag, WhiteHorned, WhiteEagle, WhiteSword,
    WhiteCrown, WhiteHCrown, WhiteHorse, WhiteDrunk, WhitePBishop, WhiteKing,
    BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen,
    BlackFerz, BlackAlfil, BlackAngel, BlackMarshall, BlackWazir, BlackMan,
    BlackCannon, BlackNightrider, BlackCardinal, BlackDragon, BlackGrasshopper,
    BlackSilver, BlackFalcon, BlackLance, BlackCobra, BlackUnicorn, BlackLion,
    BlackTokin, BlackDagger, BlackPCardinal, BlackPDragon, BlackCat,
    BlackPSword, BlackMonarch, BlackMother, BlackNothing, BlackPRook, BlackPDagger,
    BlackDolphin, BlackStag, BlackHorned, BlackEagle, BlackSword,
    BlackCrown, BlackHCrown, BlackHorse, BlackDrunk, BlackPBishop, BlackKing,
    EmptySquare, DarkSquare,
    NoRights, // [HGM] gamestate: for castling rights hidden in board[CASTLING]
    ClearBoard, WhitePlay, BlackPlay, PromotePiece, DemotePiece /*for use on EditPosition menus*/
  } ChessSquare;

/* [HGM] some macros that can be used as prefixes to convert piece types */
#define WHITE_TO_BLACK (int)BlackPawn - (int)WhitePawn + (int)
#define BLACK_TO_WHITE (int)WhitePawn - (int)BlackPawn + (int)
#define PROMOTED       (int)WhiteDragon - (int)WhiteRook + (int)
#define DEMOTED        (int)WhiteRook - (int)WhiteDragon + (int)
#define SHOGI          (int)EmptySquare + (int)
#define CHUPROMOTED    ((int)WhitePDragon - (int)WhiteDragon)*(gameInfo.variant == VariantChu) + PROMOTED
#define CHUDEMOTED     ((int)WhiteDragon - (int)WhitePDragon)*(gameInfo.variant == VariantChu) + DEMOTED
#define IS_SHOGI(V)    ((V) == VariantShogi || (V) == VariantChu)
#define IS_LION(V)     ((V) == WhiteLion || (V) == BlackLion)


typedef ChessSquare Board[BOARD_RANKS][BOARD_FILES];

typedef enum {
    EndOfFile = 0,
    WhiteKingSideCastle, WhiteQueenSideCastle,
    WhiteKingSideCastleWild, WhiteQueenSideCastleWild,
    WhiteHSideCastleFR, WhiteASideCastleFR,
    BlackKingSideCastle, BlackQueenSideCastle,
    BlackKingSideCastleWild, BlackQueenSideCastleWild,
    BlackHSideCastleFR, BlackASideCastleFR,
    WhitePromotion, WhiteNonPromotion,
    BlackPromotion, BlackNonPromotion,
    WhiteCapturesEnPassant, BlackCapturesEnPassant,
    WhiteDrop, BlackDrop, FirstLeg,
    NormalMove, AmbiguousMove, IllegalMove, ImpossibleMove,
    WhiteWins, BlackWins, GameIsDrawn, GameUnfinished,
    GNUChessGame, XBoardGame, MoveNumberOne, Open, Close, Nothing,
    Comment, PositionDiagram, ElapsedTime, PGNTag, NAG
  } ChessMove;

typedef enum {
    ColorShout, ColorSShout, ColorChannel1, ColorChannel, ColorKibitz,
    ColorTell, ColorChallenge, ColorRequest, ColorSeek, ColorNormal,
    ColorNone, NColorClasses
} ColorClass;

typedef enum {
    SoundMove, SoundBell, SoundRoar, SoundAlarm, SoundIcsWin, SoundIcsLoss,
    SoundIcsDraw, SoundIcsUnfinished, NSoundClasses
} SoundClass;

/* Names for chess variants, not necessarily supported */
typedef enum {
    VariantNormal,       /* Normal chess */
    VariantLoadable,     /* "loadgame" command allowed (not really a variant)*/
    VariantWildCastle,   /* Shuffle chess where king can castle from d file */
    VariantNoCastle,     /* Shuffle chess with no castling at all */
    VariantFischeRandom, /* FischeRandom */
    VariantBughouse,     /* Bughouse, ICC/FICS rules */
    VariantCrazyhouse,   /* Crazyhouse, ICC/FICS rules */
    VariantLosers,       /* Try to lose all pieces or get mated (ICC wild 17)*/
    VariantSuicide,      /* Try to lose all pieces incl. king (FICS) */
    VariantGiveaway,     /* Try to have no legal moves left (ICC wild 26) */
    VariantTwoKings,     /* Weird ICC wild 9 */
    VariantKriegspiel,   /* Kriegspiel; pawns can capture invisible pieces */
    VariantAtomic,       /* Capturing piece explodes (ICC wild 27) */
    Variant3Check,       /* Win by giving check 3 times (ICC wild 25) */
    VariantShatranj,     /* Unsupported (ICC wild 28) */
    Variant29,           /* Temporary name for possible future ICC wild 29 */
    Variant30,           /* Temporary name for possible future ICC wild 30 */
    Variant31,           /* Temporary name for possible future ICC wild 31 */
    Variant32,           /* Temporary name for possible future ICC wild 32 */
    Variant33,           /* Temporary name for possible future ICC wild 33 */
    Variant34,           /* Temporary name for possible future ICC wild 34 */
    Variant35,           /* Temporary name for possible future ICC wild 35 */
    Variant36,           /* Temporary name for possible future ICC wild 36 */
    VariantShogi,        /* [HGM] added variants */
    VariantChu,
    VariantCourier,
    VariantGothic,
    VariantCapablanca,
    VariantKnightmate,
    VariantFairy,
    VariantCylinder,
    VariantFalcon,
    VariantCapaRandom,
    VariantBerolina,
    VariantJanus,
    VariantSuper,
    VariantGreat,
    VariantTwilight,
    VariantMakruk,
    VariantSChess,
    VariantGrand,
    VariantSpartan,
    VariantXiangqi,
    VariantASEAN,
    VariantLion,
    VariantChuChess,
    VariantUnknown       /* Catchall for other unknown variants */
} VariantClass;

#define VARIANT_NAMES { \
  "normal", \
  "normal", \
  "wildcastle", \
  "nocastle", \
  "fischerandom", \
  "bughouse", \
  "crazyhouse", \
  "losers", \
  "suicide", \
  "giveaway", \
  "twokings", \
  "kriegspiel", \
  "atomic", \
  "3check", \
  "shatranj", \
  "wild29", \
  "wild30", \
  "wild31", \
  "wild32", \
  "wild33", \
  "wild34", \
  "wild35", \
  "wild36", \
  "shogi", \
  "chu", \
  "courier", \
  "gothic", \
  "capablanca", \
  "knightmate", \
  "fairy", \
  "cylinder", \
  "falcon",\
  "caparandom",\
  "berolina",\
  "janus",\
  "super",\
  "great",\
  "twilight",\
  "makruk",\
  "seirawan",\
  "grand",\
  "spartan",\
  "xiangqi", \
  "asean",\
  "lion",\
  "elven",\
  "unknown" \
}

#define ENGINES 2

typedef struct {
    char *language;
#if !defined(_amigados)
    char *whitePieceColor;
    char *blackPieceColor;
    char *lightSquareColor;
    char *darkSquareColor;
    char *jailSquareColor;
    char *highlightSquareColor;
    char *premoveHighlightColor;
    char *dialogColor;
    char *buttonColor;
#else
    int whitePieceColor;
    int blackPieceColor;
    int lightSquareColor;
    int darkSquareColor;
    int jailSquareColor;
    int highlightSquareColor;
    int premoveHighlightColor;
#endif
    int movesPerSession;
    float timeIncrement;
    char *engInitString[ENGINES];
    char *computerString[ENGINES];
    char *chessProgram[ENGINES];
    char *directory[ENGINES];
    char *pgnName[ENGINES];
    Boolean firstPlaysBlack;
    Boolean noChessProgram;
    char *host[ENGINES];
    char *themeNames;
    char *pieceDirectory;
    char *border;
    char *soundDirectory;
    char *remoteShell;
    char *remoteUser;
    float timeDelay;
    char *timeControl;
    Boolean trueColors;
    Boolean icsActive;
    Boolean autoBox;
    char *icsHost;
    char *icsPort;
    char *icsCommPort;  /* if set, use serial port instead of tcp host/port */
    char *icsLogon;     /* Hack to permit variable logon scripts. */
    char *icsHelper;
    Boolean icsInputBox;
    Boolean useTelnet;
    Boolean seekGraph;
    Boolean autoRefresh;
    char *telnetProgram;
    char *gateway;
    char *loadGameFile;
    int loadGameIndex;      /* game # within file */
    char *saveGameFile;
    char *autoInstall;
    Boolean autoSaveGames;
    Boolean onlyOwn;        /* [HGM] suppress auto-saving of observed games */
    char *loadPositionFile;
    int loadPositionIndex;  /* position # within file */
    char *savePositionFile;
    Boolean fischerCastling;/* [HGM] fischer: allow Fischr castling in any variant */
    Boolean matchMode;
    int matchGames;
    Boolean monoMode;
    Boolean debugMode;
    Boolean clockMode;
    char *boardSize;
    char *logoDir;
    int logoSize;
    Boolean Iconic;
    char *searchTime;
    int searchDepth;
    Boolean showCoords;
    char *clockFont;
    char *messageFont; /* WinBoard only */
    char *coordFont;
    char *font; /* xboard only */
    char *tagsFont;
    char *commentFont;
    char *historyFont;
    char *gameListFont;
    char *icsFont;
    Boolean ringBellAfterMoves;
    Boolean autoCallFlag;
    Boolean flipView;
    Boolean autoFlipView;
    char *cmailGameName; /* xboard only */
    Boolean headers;
    Boolean alwaysPromoteToQueen;
    Boolean oldSaveStyle;
    Boolean oneClick;
    Boolean quietPlay;
    Boolean showThinking;
    Boolean ponderNextMove;
    Boolean periodicUpdates;
    Boolean autoObserve;
    Boolean autoCreateLogon;
    Boolean autoComment;
    Boolean getMoveList;
    Boolean testLegality;
    Boolean topLevel;      /* xboard, top-level auxiliary windows */
    Boolean titleInWindow; /* xboard only */
    Boolean localLineEditing; /* WinBoard only */
    Boolean zippyTalk;
    Boolean zippyPlay;
    int flashCount; /* Number of times to flash (xboard only) */
    int flashRate; /* Flashes per second (xboard only)  */
    int msLoginDelay;  /* Delay per character (in msec) while sending
			  ICS logon script (xboard only) */
    Boolean colorize;	/* If True, use the following colors to color text */
    /* Strings for colors, as "fg, bg, bold" (strings used in xboard only) */
    char *colorShout;    // [HGM] IMPORTANT: order must conform to ColorClass definition
    char *colorSShout;
    char *colorChannel1;
    char *colorChannel;
    char *colorKibitz;
    char *colorTell;
    char *colorChallenge;
    char *colorRequest;
    char *colorSeek;
    char *colorNormal;
    char *soundProgram; /* sound-playing program */
    char *soundShout;     // [HGM] IMPORTANT: order must be as in ColorClass
    char *soundSShout;
    char *soundChannel1;
    char *soundChannel;
    char *soundKibitz;
    char *soundTell;
    char *soundChallenge;
    char *soundRequest;
    char *soundSeek;
    char *soundMove;     // [HGM] IMPORTANT: order must be as in SoundClass
    char *soundBell;
    char *soundRoar;
    char *soundIcsAlarm;
    char *soundIcsWin;
    char *soundIcsLoss;
    char *soundIcsDraw;
    char *soundIcsUnfinished;
    Boolean disguise;        /* [HGM] Promoted Pawns look like pieces in bughouse */
    Boolean reuse[ENGINES];
    Boolean animateDragging; /* If True, animate mouse dragging of pieces */
    Boolean animate;	/* If True, animate non-mouse moves */
    int animSpeed;	/* Delay in milliseconds between animation frames */
    Boolean popupMoveErrors;
    Boolean popupExitMessage;
    int showJail;
    Boolean highlightLastMove;
    Boolean highlightDragging;
    Boolean blindfold;          /* if true, no pieces are drawn */
    Boolean premove;		/* true if premove feature enabled */
    Boolean premoveWhite;	/* true if premoving White first move  */
    char *premoveWhiteText;	/* text of White premove 1 */
    Boolean premoveBlack;	/* true if premoving Black first move */
    char *premoveBlackText;	/* text of Black premove 1 */
    Boolean icsAlarm;		/* true if sounding alarm at a certain time */
    int icsAlarmTime;		/* time to sound alarm, in milliseconds */
    Boolean autoRaiseBoard;
    int fontSizeTolerance; /* xboard only */
    char *initialMode;
    char *variant;
    char *chatBoxes;
    int protocolVersion[ENGINES];
    Boolean showButtonBar;
    Boolean icsEngineAnalyze;
    Boolean variations;         /* [HGM] enable variation-tree walking */
    Boolean autoExtend;         /* [HGM] enable playing move(s) of right-clicked PV in analysis mode */

    /* [AS] New properties (down to the "ZIPPY" part) */
    Boolean scoreIsAbsolute[ENGINES];  /* If true, engine score is always from white side */
    Boolean saveExtendedInfoInPGN; /* If true, saved PGN games contain extended info */
    Boolean hideThinkingFromHuman; /* If true, program thinking is generated but not displayed in human/computer matches */
    Boolean useBitmaps;
    Boolean useFont;
    Boolean useBorder;
    char * liteBackTextureFile; /* Name of texture bitmap for lite squares */
    char * darkBackTextureFile; /* Name of texture bitmap for dark squares */
    int liteBackTextureMode;
    int darkBackTextureMode;
    char * renderPiecesWithFont; /* Name of font for rendering chess pieces */
    char * fontToPieceTable; /* Map to translate font character to chess pieces */
    char * inscriptions;         /* text (kanji) to write on top of a piece     */
    int fontBackColorWhite;
    int fontForeColorWhite;
    int fontBackColorBlack;
    int fontForeColorBlack;
    int fontPieceSize; /* Size of font relative to square (percentage) */
    int overrideLineGap; /* If >= 0 overrides the lineGap value of the board size properties */
    int adjudicateLossThreshold; /* Adjudicate a two-machine game if both engines agree the score is below this for 6 plies */
    int delayBeforeQuit;
    int delayAfterQuit;
    char * nameOfDebugFile;
    char * pgnEventHeader;
    int defaultFrcPosition;
    char * gameListTags;
    Boolean saveOutOfBookInfo;
    Boolean showEvalInMoveHistory;
    int evalHistColorWhite;
    int evalHistColorBlack;
    Boolean highlightMoveWithArrow;
    Boolean tourney;
    char * tourneyOptions;
    int highlightArrowColor;
    Boolean useStickyWindows;
    Boolean bgObserve;   /* [HGM] bughouse */
    Boolean dualBoard;   /* [HGM] dual     */
    Boolean viewer;
    char * viewerOptions;
    int adjudicateDrawMoves;
    Boolean autoDisplayComment;
    Boolean autoDisplayTags;
    Boolean pseudo[ENGINES]; /* [HGM] pseudo-engines */
    Boolean isUCI[ENGINES];
    Boolean hasOwnBookUCI[ENGINES];
    char * adapterCommand;
    char * ucciAdapter;
    char * polyglotDir;
    Boolean usePolyglotBook;
    Boolean defNoBook;
    char * polyglotBook;
    int bookDepth;
    int bookStrength;
    int defaultHashSize;
    int defaultCacheSizeEGTB;
    char * defaultPathEGTB;
    int defaultMatchGames;

    /* [HGM] Board size */
    int NrFiles;
    int NrRanks;
    int holdingsSize;
    int matchPause;
    char * pieceToCharTable;
    char * pieceNickNames;
    char * colorNickNames;
    Boolean allWhite;
    Boolean upsideDown;
    Boolean alphaRank;
    Boolean testClaims;
    Boolean checkMates;
    Boolean materialDraws;
    Boolean trivialDraws;
    int ruleMoves;
    int drawRepeats;

#if ZIPPY
    char *zippyLines;
    char *zippyPinhead;
    char *zippyPassword;
    char *zippyPassword2;
    char *zippyWrongPassword;
    char *zippyAcceptOnly;
    int zippyUseI;
    int zippyBughouse;
    int zippyNoplayCrafty;
    char *zippyGameEnd;
    char *zippyGameStart;
    int zippyAdjourn;
    int zippyAbort;
    char *zippyVariants;
    int zippyMaxGames;
    int zippyReplayTimeout; /*seconds*/
    int zippyShortGame; /* [HGM] aborter   */
#endif
    Boolean lowTimeWarning; /* [HGM] low time */
    Boolean quitNext;
    char *lowTimeWarningColor;

    char *serverFileName;
    char *serverMovesName;
    char *finger;
    Boolean suppressLoadMoves;
    int serverPause;
    int timeOdds[ENGINES];
    int drawDepth[ENGINES];
    int timeOddsMode;
    int accumulateTC[ENGINES];
    int NPS[ENGINES];
    Boolean autoKibitz;
    int engineComments;
    int eloThreshold1;  /* [HGM] select   */
    int eloThreshold2;
    int dateThreshold;
    int searchMode;
    int stretch;
    int minPieces;
    int maxPieces;
    Boolean ignoreColors;
    Boolean findMirror;
    char *userName;
    int rewindIndex;    /* [HGM] autoinc   */
    int sameColorGames; /* [HGM] alternate */
    int smpCores;       /* [HGM] SMP       */
    char *egtFormats;
    int niceEngines;    /* [HGM] nice      */
    char *logo[ENGINES];/* [HGM] logo      */
    char *pairingEngine;/* [HGM] pairing   */
    Boolean autoLogo;
    Boolean fixedSize;
    Boolean noGUI;      /* [HGM] fast: suppress all display updates */
    char *engOptions[ENGINES]; /* [HGM] options   */
    char *fenOverride[ENGINES];
    char *features[ENGINES];
    char *featureDefaults;
    char *sysOpen;
    Boolean keepAlive;  /* [HGM] alive     */
    Boolean forceIllegal;/*[HGM] illegal   */
    Boolean noJoin;     /* [HGM] join      */
    char *wrapContSeq; /* continuation sequence when xboard wraps text */
    Boolean useInternalWrap; /* use internal wrapping -- noJoin usurps this if set */
    Boolean pasteSelection; /* paste X selection instead of clipboard */
    int nrVariations;   /* [HGM] multivar  */
    int zoom;           /* [HGM] evalGraph */
    int evalThreshold;  /* [HGM] evalGraph */
    Boolean dropMenu;   /* [HGM] pv        */
    Boolean markers;    /* [HGM] markers   */
    Boolean autoCopyPV;
    Boolean pieceMenu;
    Boolean sweepSelect;
    Boolean whitePOV;
    Boolean scoreWhite;
    Boolean pvSAN[ENGINES];

    int recentEngines;
    char *recentEngineList;
    char *tourneyFile;
    char *defName;
    char *processes;
    char *results;
    char *participants;
    char *afterGame;
    char *afterTourney;
    int tourneyType;
    int tourneyCycles;
    int seedBase;
    Boolean roundSync;
    Boolean cycleSync;
    Boolean numberTag;
} AppData, *AppDataPtr;

/*  PGN tags (for showing in the game list) */
#define LPUSERGLT_SIZE      64

#define GLT_EVENT           'e'
#define GLT_SITE            's'
#define GLT_DATE            'd'
#define GLT_ROUND           'o'
#define GLT_PLAYERS         'p'     /* I.e. white "-" black */
#define GLT_RESULT          'r'
#define GLT_WHITE_ELO       'w'
#define GLT_BLACK_ELO       'b'
#define GLT_TIME_CONTROL    't'
#define GLT_VARIANT         'v'
#define GLT_OUT_OF_BOOK     'a'
#define GLT_RESULT_COMMENT  'c'     /* [HGM] rescom */

#define GLT_DEFAULT_TAGS    "eprd"  /* Event, players, result, date */

#define GLT_ALL_TAGS        "esdoprwbtvac"

#define PGN_OUT_OF_BOOK     "Annotator"

extern AppData appData;

typedef struct {
    /* PGN 7-tag info */
    char *event;
    char *site;
    char *date;
    char *round;
    char *white;
    char *black;
    ChessMove result;
    /* Additional info */
    char *fen;          /* NULL or FEN for starting position; input only */
    char *resultDetails;
    char *timeControl;
    char *extraTags;    /* NULL or "[Tag \"Value\"]\n", etc. */
    int whiteRating;    /* -1 if unknown */
    int blackRating;    /* -1 if unknown */
    VariantClass variant;
    char *variantName;
    char *outOfBook;    /* [AS] Move and score when engine went out of book */
    int boardWidth;     /* [HGM] adjustable board size */
    int boardHeight;
/* [HGM] For Shogi and Crazyhouse: */
    int holdingsSize;  /* number of different piece types in holdings       */
    int holdingsWidth; /* number of files left and right of board, 0 or 2   */
} GameInfo;

/* [AS] Search stats from chessprogram, for the played move */
// [HGM] moved here from backend.h because it occurs in declarations of front-end functions
typedef struct {
    int score;  /* Centipawns */
    int depth;  /* Plies */
    int time;   /* Milliseconds */
} ChessProgramStats_Move;

/* [AS] Layout management */
typedef struct {
    Boolean visible;
    int x;
    int y;
    int width;
    int height;
} WindowPlacement;

extern WindowPlacement wpEngineOutput;
extern WindowPlacement wpEvalGraph;
extern WindowPlacement wpMoveHistory;
extern WindowPlacement wpGameList;
extern WindowPlacement wpTags;
extern WindowPlacement wpTextMenu;

#define MAXENGINES 2000

// [HGM] chat
#define MAX_CHAT 5
extern int chatCount;
extern char chatPartner[MAX_CHAT][MSG_SIZ];

// Some prototypes of routines so general they should be available everywhere
/* If status == 0, we are exiting with a benign message, not an error */
void DisplayFatalError P((String message, int error, int status));
void DisplayError P((String message, int error));

// [HGM] generally useful macros; there are way too many memory leaks...
#define FREE(x) if(x) free(x)
#define ASSIGN(x, y) if(x) free(x); x = strdup(y)

// [HGM] for now we use the kludge to redefine all the unstructured options by their array counterpart
//       in due time we would have to make the actual substitutions all through the source

#define firstInitString       engInitString[0]
#define secondInitString      engInitString[1]
#define firstComputerString   computerString[0]
#define secondComputerString  computerString[1]
#define firstChessProgram     chessProgram[0]
#define secondChessProgram    chessProgram[1]
#define firstDirectory        directory[0]
#define secondDirectory       directory[1]
#define firstProtocolVersion  protocolVersion[0]
#define secondProtocolVersion protocolVersion[1]
#define firstScoreIsAbsolute  scoreIsAbsolute[0]
#define secondScoreIsAbsolute scoreIsAbsolute[1]
#define firstHasOwnBookUCI    hasOwnBookUCI[0]
#define secondHasOwnBookUCI   hasOwnBookUCI[1]
#define firstTimeOdds         timeOdds[0]
#define secondTimeOdds        timeOdds[1]
#define firstAccumulateTC     accumulateTC[0]
#define secondAccumulateTC    accumulateTC[1]
#define firstHost    host[0]
#define secondHost   host[1]
#define reuseFirst   reuse[0]
#define reuseSecond  reuse[1]
#define firstIsUCI   isUCI[0]
#define secondIsUCI  isUCI[1]
#define firstNPS     NPS[0]
#define secondNPS    NPS[1]
#define firstLogo    logo[0]
#define secondLogo   logo[1]
#define fenOverride1 fenOverride[0]
#define fenOverride2 fenOverride[1]
#define firstOptions      engOptions[0]
#define secondOptions     engOptions[1]

#endif
