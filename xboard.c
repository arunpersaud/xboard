/*
 * xboard.c -- X front end for XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#define HIGHDRAG 1

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <math.h>

#if !OMIT_SOCKETS
# if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
# else /* not HAVE_SYS_SOCKET_H */
#  if HAVE_LAN_SOCKET_H
#   include <lan/socket.h>
#   include <lan/in.h>
#   include <lan/netdb.h>
#  else /* not HAVE_LAN_SOCKET_H */
#   define OMIT_SOCKETS 1
#  endif /* not HAVE_LAN_SOCKET_H */
# endif /* not HAVE_SYS_SOCKET_H */
#endif /* !OMIT_SOCKETS */

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* not STDC_HEADERS */
extern char *getenv();
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

#if HAVE_SYS_SYSTEMINFO_H
# include <sys/systeminfo.h>
#endif /* HAVE_SYS_SYSTEMINFO_H */

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

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
# define HAVE_DIR_STRUCT
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
#  define HAVE_DIR_STRUCT
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
#  define HAVE_DIR_STRUCT
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
#  define HAVE_DIR_STRUCT
# endif
#endif

#if ENABLE_NLS
#include <locale.h>
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#if USE_XAW3D
#include <X11/Xaw3d/Dialog.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/List.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/SmeLine.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/MenuButton.h>
#include <X11/Xaw3d/Text.h>
#include <X11/Xaw3d/AsciiText.h>
#else
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#endif

// [HGM] bitmaps: put before incuding the bitmaps / pixmaps, to know how many piece types there are.
#include "common.h"

#if HAVE_LIBXPM
#include <X11/xpm.h>
#include "pixmaps/pixmaps.h"
#define IMAGE_EXT "xpm"
#else
#define IMAGE_EXT "xim"
#include "bitmaps/bitmaps.h"
#endif

#include "bitmaps/icon_white.bm"
#include "bitmaps/icon_black.bm"
#include "bitmaps/checkmark.bm"

#include "frontend.h"
#include "backend.h"
#include "backendz.h"
#include "moves.h"
#include "xboard.h"
#include "childio.h"
#include "xgamelist.h"
#include "xhistory.h"
#include "xedittags.h"
#include "menus.h"
#include "board.h"
#include "dialogs.h"
#include "gettext.h"


#ifdef __EMX__
#ifndef HAVE_USLEEP
#define HAVE_USLEEP
#endif
#define usleep(t)   _sleep2(((t)+500)/1000)
#endif

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

int main P((int argc, char **argv));
FILE * XsraSelFile P((Widget w, char *prompt, char *ok, char *cancel, char *failed,
		char *init_path, char *filter, char *mode, int (*show_entry)(), char **name_return));
RETSIGTYPE CmailSigHandler P((int sig));
RETSIGTYPE IntSigHandler P((int sig));
RETSIGTYPE TermSizeSigHandler P((int sig));
static void CreateGCs P((int redo));
static void CreateAnyPieces P((void));
void CreateXIMPieces P((void));
void CreateXPMPieces P((void));
void CreateXPMBoard P((char *s, int n));
void CreatePieces P((void));
void CreatePieceMenus P((void));
Widget CreateMenuBar P((Menu *mb, int boardWidth));
Widget CreateButtonBar P ((MenuItem *mi));
#if ENABLE_NLS
char *InsertPxlSize P((char *pattern, int targetPxlSize));
XFontSet CreateFontSet P((char *base_fnt_lst));
#else
char *FindFont P((char *pattern, int targetPxlSize));
#endif
void PieceMenuPopup P((Widget w, XEvent *event,
		       String *params, Cardinal *num_params));
static void PieceMenuSelect P((Widget w, ChessSquare piece, caddr_t junk));
static void DropMenuSelect P((Widget w, ChessSquare piece, caddr_t junk));
void ReadBitmap P((Pixmap *pm, String name, unsigned char bits[],
		   u_int wreq, u_int hreq));
void CreateGrid P((void));
int EventToSquare P((int x, int limit));
void EventProc P((Widget widget, caddr_t unused, XEvent *event));
void DelayedDrag P((void));
static void MoveTypeInProc P((Widget widget, caddr_t unused, XEvent *event));
void HandleUserMove P((Widget w, XEvent *event,
		     String *prms, Cardinal *nprms));
void AnimateUserMove P((Widget w, XEvent * event,
		     String * params, Cardinal * nParams));
void HandlePV P((Widget w, XEvent * event,
		     String * params, Cardinal * nParams));
void SelectPV P((Widget w, XEvent * event,
		     String * params, Cardinal * nParams));
void StopPV P((Widget w, XEvent * event,
		     String * params, Cardinal * nParams));
void WhiteClock P((Widget w, XEvent *event,
		   String *prms, Cardinal *nprms));
void BlackClock P((Widget w, XEvent *event,
		   String *prms, Cardinal *nprms));
void DrawPositionProc P((Widget w, XEvent *event,
		     String *prms, Cardinal *nprms));
void XDrawPosition P((Widget w, /*Boolean*/int repaint,
		     Board board));
void CommentClick P((Widget w, XEvent * event,
		   String * params, Cardinal * nParams));
void CommentPopUp P((char *title, char *label));
void CommentPopDown P((void));
void ICSInputBoxPopUp P((void));
void ICSInputBoxPopDown P((void));
void FileNamePopUp P((char *label, char *def, char *filter,
		      FileProc proc, char *openMode));
void FileNamePopDown P((void));
void FileNameCallback P((Widget w, XtPointer client_data,
			 XtPointer call_data));
void FileNameAction P((Widget w, XEvent *event,
		       String *prms, Cardinal *nprms));
void AskQuestionReplyAction P((Widget w, XEvent *event,
			  String *prms, Cardinal *nprms));
void AskQuestionProc P((Widget w, XEvent *event,
			  String *prms, Cardinal *nprms));
void AskQuestionPopDown P((void));
void PromotionPopDown P((void));
void PromotionCallback P((Widget w, XtPointer client_data,
			  XtPointer call_data));
void SelectCommand P((Widget w, XtPointer client_data, XtPointer call_data));
void KeyBindingProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void QuitWrapper P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TypeInProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EnterKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void UpKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DownKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TempBackwardProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TempForwardProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
Boolean TempBackwardActive = False;
void ManInner P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DisplayMove P((int moveNumber));
void DisplayTitle P((char *title));
void ICSInitScript P((void));
void ErrorPopUp P((char *title, char *text, int modal));
void ErrorPopDown P((void));
static char *ExpandPathName P((char *path));
void SelectMove P((Widget w, XEvent * event, String * params, Cardinal * nParams));
void GameListOptionsPopDown P(());
void GenericPopDown P(());
void update_ics_width P(());
int get_term_width P(());
int CopyMemoProc P(());
void DrawArrowHighlight P((int fromX, int fromY, int toX,int toY));
Boolean IsDrawArrowEnabled P(());

/*
* XBoard depends on Xt R4 or higher
*/
int xtVersion = XtSpecificationRelease;

int xScreen;
Display *xDisplay;
Window xBoardWindow;
Pixel lightSquareColor, darkSquareColor, whitePieceColor, blackPieceColor,
  jailSquareColor, highlightSquareColor, premoveHighlightColor;
Pixel lowTimeWarningColor;
GC lightSquareGC, darkSquareGC, jailSquareGC, lineGC, wdPieceGC, wlPieceGC,
  bdPieceGC, blPieceGC, wbPieceGC, bwPieceGC, coordGC, highlineGC,
  wjPieceGC, bjPieceGC, prelineGC, countGC;
Pixmap iconPixmap, wIconPixmap, bIconPixmap, xMarkPixmap;
Widget shellWidget, layoutWidget, formWidget, boardWidget, messageWidget,
  whiteTimerWidget, blackTimerWidget, titleWidget, widgetList[16],
  commentShell, promotionShell, whitePieceMenu, blackPieceMenu, dropMenu,
  menuBarWidget, buttonBarWidget, editShell, errorShell, analysisShell,
  ICSInputShell, fileNameShell, askQuestionShell;
Widget historyShell, evalGraphShell, gameListShell;
XSegment secondSegments[BOARD_RANKS + BOARD_FILES + 2];
XSegment gridSegments[BOARD_RANKS + BOARD_FILES + 2];
XSegment jailGridSegments[BOARD_RANKS + BOARD_FILES + 6];
#if ENABLE_NLS
XFontSet fontSet, clockFontSet;
#else
Font clockFontID;
XFontStruct *clockFontStruct;
#endif
Font coordFontID, countFontID;
XFontStruct *coordFontStruct, *countFontStruct;
XtAppContext appContext;
char *layoutName;
char *oldICSInteractionTitle;

FileProc fileProc;
char *fileOpenMode;
char installDir[] = "."; // [HGM] UCI: needed for UCI; probably needs run-time initializtion

Position commentX = -1, commentY = -1;
Dimension commentW, commentH;
typedef unsigned int BoardSize;
BoardSize boardSize;
Boolean chessProgram;

int  minX, minY; // [HGM] placement: volatile limits on upper-left corner
int smallLayout = 0, tinyLayout = 0,
  marginW, marginH, // [HGM] for run-time resizing
  fromX = -1, fromY = -1, toX, toY, commentUp = False, analysisUp = False,
  ICSInputBoxUp = False, askQuestionUp = False,
  filenameUp = False, promotionUp = False, pmFromX = -1, pmFromY = -1,
  errorUp = False, errorExitStatus = -1, defaultLineGap;
Dimension textHeight;
Pixel timerForegroundPixel, timerBackgroundPixel;
Pixel buttonForegroundPixel, buttonBackgroundPixel;
char *chessDir, *programName, *programVersion;
Boolean alwaysOnTop = False;
char *icsTextMenuString;
char *icsNames;
char *firstChessProgramNames;
char *secondChessProgramNames;

WindowPlacement wpMain;
WindowPlacement wpConsole;
WindowPlacement wpComment;
WindowPlacement wpMoveHistory;
WindowPlacement wpEvalGraph;
WindowPlacement wpEngineOutput;
WindowPlacement wpGameList;
WindowPlacement wpTags;


#define SOLID 0
#define OUTLINE 1
Pixmap pieceBitmap[2][(int)BlackPawn];
Pixmap pieceBitmap2[2][(int)BlackPawn+4];       /* [HGM] pieces */
Pixmap xpmPieceBitmap[4][(int)BlackPawn];	/* LL, LD, DL, DD actually used*/
Pixmap xpmPieceBitmap2[4][(int)BlackPawn+4];	/* LL, LD, DL, DD set to select from */
Pixmap xpmLightSquare, xpmDarkSquare, xpmJailSquare;
Pixmap xpmBoardBitmap[2];
int useImages, useImageSqs, useTexture, textureW[2], textureH[2];
XImage *ximPieceBitmap[4][(int)BlackPawn+4];	/* LL, LD, DL, DD */
Pixmap ximMaskPm[(int)BlackPawn];               /* clipmasks, used for XIM pieces */
Pixmap ximMaskPm2[(int)BlackPawn+4];            /* clipmasks, used for XIM pieces */
XImage *ximLightSquare, *ximDarkSquare;
XImage *xim_Cross;

#define pieceToSolid(piece) &pieceBitmap[SOLID][(piece) % (int)BlackPawn]
#define pieceToOutline(piece) &pieceBitmap[OUTLINE][(piece) % (int)BlackPawn]

#define White(piece) ((int)(piece) < (int)BlackPawn)

/* Bitmaps for use as masks when drawing XPM pieces.
   Need one for each black and white piece.		*/
static Pixmap xpmMask[BlackKing + 1];

/* This magic number is the number of intermediate frames used
   in each half of the animation. For short moves it's reduced
   by 1. The total number of frames will be factor * 2 + 1.  */
#define kFactor	   4

SizeDefaults sizeDefaults[] = SIZE_DEFAULTS;

#define PAUSE_BUTTON "P"
MenuItem buttonBar[] = {
    {"<<", "<<", ToStartEvent},
    {"<", "<", BackwardEvent},
    {N_(PAUSE_BUTTON), PAUSE_BUTTON, PauseEvent},
    {">", ">", ForwardEvent},
    {">>", ">>", ToEndEvent},
    {NULL, NULL, NULL}
};

#define PIECE_MENU_SIZE 18
String pieceMenuStrings[2][PIECE_MENU_SIZE] = {
    { N_("White"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"),
      N_("Queen"), N_("King"), "----", N_("Elephant"), N_("Cannon"),
      N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"),
      N_("Empty square"), N_("Clear board") },
    { N_("Black"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"),
      N_("Queen"), N_("King"), "----", N_("Elephant"), N_("Cannon"),
      N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"),
      N_("Empty square"), N_("Clear board") }
};
/* must be in same order as pieceMenuStrings! */
ChessSquare pieceMenuTranslation[2][PIECE_MENU_SIZE] = {
    { WhitePlay, (ChessSquare) 0, WhitePawn, WhiteKnight, WhiteBishop,
	WhiteRook, WhiteQueen, WhiteKing, (ChessSquare) 0, WhiteAlfil,
	WhiteCannon, WhiteAngel, WhiteMarshall, (ChessSquare) 0,
	PromotePiece, DemotePiece, EmptySquare, ClearBoard },
    { BlackPlay, (ChessSquare) 0, BlackPawn, BlackKnight, BlackBishop,
	BlackRook, BlackQueen, BlackKing, (ChessSquare) 0, BlackAlfil,
	BlackCannon, BlackAngel, BlackMarshall, (ChessSquare) 0,
	PromotePiece, DemotePiece, EmptySquare, ClearBoard },
};

#define DROP_MENU_SIZE 6
String dropMenuStrings[DROP_MENU_SIZE] = {
    "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"), N_("Queen")
  };
/* must be in same order as dropMenuStrings! */
ChessSquare dropMenuTranslation[DROP_MENU_SIZE] = {
    (ChessSquare) 0, WhitePawn, WhiteKnight, WhiteBishop,
    WhiteRook, WhiteQueen
};

typedef struct {
    char piece;
    char* widget;
} DropMenuEnables;

DropMenuEnables dmEnables[] = {
    { 'P', "Pawn" },
    { 'N', "Knight" },
    { 'B', "Bishop" },
    { 'R', "Rook" },
    { 'Q', "Queen" }
};

Arg shellArgs[] = {
    { XtNwidth, 0 },
    { XtNheight, 0 },
    { XtNminWidth, 0 },
    { XtNminHeight, 0 },
    { XtNmaxWidth, 0 },
    { XtNmaxHeight, 0 }
};

Arg layoutArgs[] = {
    { XtNborderWidth, 0 },
    { XtNdefaultDistance, 0 },
};

Arg formArgs[] = {
    { XtNborderWidth, 0 },
    { XtNresizable, (XtArgVal) True },
};

Arg boardArgs[] = {
    { XtNborderWidth, 0 },
    { XtNwidth, 0 },
    { XtNheight, 0 }
};

Arg titleArgs[] = {
    { XtNjustify, (XtArgVal) XtJustifyRight },
    { XtNlabel, (XtArgVal) "..." },
    { XtNresizable, (XtArgVal) True },
    { XtNresize, (XtArgVal) False }
};

Arg messageArgs[] = {
    { XtNjustify, (XtArgVal) XtJustifyLeft },
    { XtNlabel, (XtArgVal) "..." },
    { XtNresizable, (XtArgVal) True },
    { XtNresize, (XtArgVal) False }
};

Arg timerArgs[] = {
    { XtNborderWidth, 0 },
    { XtNjustify, (XtArgVal) XtJustifyLeft }
};

XtResource clientResources[] = {
    { "flashCount", "flashCount", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, flashCount), XtRImmediate,
	(XtPointer) FLASH_COUNT  },
};

XrmOptionDescRec shellOptions[] = {
    { "-flashCount", "flashCount", XrmoptionSepArg, NULL },
    { "-flash", "flashCount", XrmoptionNoArg, "3" },
    { "-xflash", "flashCount", XrmoptionNoArg, "0" },
};

XtActionsRec boardActions[] = {
    { "DrawPosition", DrawPositionProc },
    { "HandleUserMove", HandleUserMove },
    { "AnimateUserMove", AnimateUserMove },
    { "HandlePV", HandlePV },
    { "SelectPV", SelectPV },
    { "StopPV", StopPV },
    { "FileNameAction", FileNameAction },
    { "AskQuestionProc", AskQuestionProc },
    { "AskQuestionReplyAction", AskQuestionReplyAction },
    { "PieceMenuPopup", PieceMenuPopup },
    { "WhiteClock", WhiteClock },
    { "BlackClock", BlackClock },
    { "MenuItem", KeyBindingProc }, // [HGM] generic handler for key bindings
    { "QuitProc", QuitWrapper },
    { "ManProc", ManInner },
    { "TempBackwardProc", TempBackwardProc },
    { "TempForwardProc", TempForwardProc },
    { "CommentClick", (XtActionProc) CommentClick },
    { "CommentPopDown", (XtActionProc) CommentPopDown },
    { "TagsPopDown", (XtActionProc) TagsPopDown },
    { "ErrorPopDown", (XtActionProc) ErrorPopDown },
    { "ICSInputBoxPopDown", (XtActionProc) ICSInputBoxPopDown },
    { "FileNamePopDown", (XtActionProc) FileNamePopDown },
    { "AskQuestionPopDown", (XtActionProc) AskQuestionPopDown },
    { "GameListPopDown", (XtActionProc) GameListPopDown },
    { "GameListOptionsPopDown", (XtActionProc) GameListOptionsPopDown },
    { "PromotionPopDown", (XtActionProc) PromotionPopDown },
    { "EngineOutputPopDown", (XtActionProc) EngineOutputPopDown },
    { "EvalGraphPopDown", (XtActionProc) EvalGraphPopDown },
    { "GenericPopDown", (XtActionProc) GenericPopDown },
    { "CopyMemoProc", (XtActionProc) CopyMemoProc },
    { "SelectMove", (XtActionProc) SelectMove },
    { "LoadSelectedProc", LoadSelectedProc },
    { "SetFilterProc", SetFilterProc },
    { "TypeInProc", TypeInProc },
    { "EnterKeyProc", EnterKeyProc },
    { "UpKeyProc", UpKeyProc },
    { "DownKeyProc", DownKeyProc },
};

char globalTranslations[] =
  ":<Key>F9: MenuItem(ResignProc) \n \
   :Ctrl<Key>n: MenuItem(NewGame) \n \
   :Meta<Key>V: MenuItem(NewVariant) \n \
   :Ctrl<Key>o: MenuItem(LoadGame) \n \
   :Meta<Key>Next: MenuItem(LoadNextGameProc) \n \
   :Meta<Key>Prior: MenuItem(LoadPrevGameProc) \n \
   :Ctrl<Key>Down: LoadSelectedProc(3) \n \
   :Ctrl<Key>Up: LoadSelectedProc(-3) \n \
   :Ctrl<Key>s: MenuItem(SaveGame) \n \
   :Ctrl<Key>c: MenuItem(CopyGame) \n \
   :Ctrl<Key>v: MenuItem(PasteGame) \n \
   :Ctrl<Key>O: MenuItem(LoadPosition) \n \
   :Shift<Key>Next: MenuItem(LoadNextPositionProc) \n \
   :Shift<Key>Prior: MenuItem(LoadPrevPositionProc) \n \
   :Ctrl<Key>S: MenuItem(SavePosition) \n \
   :Ctrl<Key>C: MenuItem(CopyPosition) \n \
   :Ctrl<Key>V: MenuItem(PastePosition) \n \
   :Ctrl<Key>q: MenuItem(Exit) \n \
   :Ctrl<Key>w: MenuItem(MachineWhite) \n \
   :Ctrl<Key>b: MenuItem(MachineBlack) \n \
   :Ctrl<Key>t: MenuItem(TwoMachines) \n \
   :Ctrl<Key>a: MenuItem(AnalysisMode) \n \
   :Ctrl<Key>g: MenuItem(AnalyzeFile) \n \
   :Ctrl<Key>e: MenuItem(EditGame) \n \
   :Ctrl<Key>E: MenuItem(EditPosition) \n \
   :Meta<Key>O: MenuItem(ShowEngineOutput) \n \
   :Meta<Key>E: MenuItem(ShowEvaluationGraph) \n \
   :Meta<Key>G: MenuItem(ShowGameList) \n \
   :Meta<Key>H: MenuItem(ShowMoveHistory) \n \
   :<Key>Pause: MenuItem(Pause) \n \
   :<Key>F3: MenuItem(Accept) \n \
   :<Key>F4: MenuItem(Decline) \n \
   :<Key>F12: MenuItem(Rematch) \n \
   :<Key>F5: MenuItem(CallFlag) \n \
   :<Key>F6: MenuItem(Draw) \n \
   :<Key>F7: MenuItem(Adjourn) \n \
   :<Key>F8: MenuItem(Abort) \n \
   :<Key>F10: MenuItem(StopObserving) \n \
   :<Key>F11: MenuItem(StopExamining) \n \
   :Ctrl<Key>d: MenuItem(DebugProc) \n \
   :Meta Ctrl<Key>F12: MenuItem(DebugProc) \n \
   :Meta<Key>End: MenuItem(ToEnd) \n \
   :Meta<Key>Right: MenuItem(Forward) \n \
   :Meta<Key>Home: MenuItem(ToStart) \n \
   :Meta<Key>Left: MenuItem(Backward) \n \
   :<Key>Left: MenuItem(Backward) \n \
   :<Key>Right: MenuItem(Forward) \n \
   :<Key>Home: MenuItem(Revert) \n \
   :<Key>End: MenuItem(TruncateGame) \n \
   :Ctrl<Key>m: MenuItem(MoveNow) \n \
   :Ctrl<Key>x: MenuItem(RetractMove) \n \
   :Meta<Key>J: MenuItem(Adjudications) \n \
   :Meta<Key>U: MenuItem(CommonEngine) \n \
   :Meta<Key>T: MenuItem(TimeControl) \n \
   :Ctrl<Key>P: MenuItem(PonderNextMove) \n "
#ifndef OPTIONSDIALOG
    "\
   :Ctrl<Key>Q: MenuItem(AlwaysQueenProc) \n \
   :Ctrl<Key>F: MenuItem(AutoflagProc) \n \
   :Ctrl<Key>A: MenuItem(AnimateMovingProc) \n \
   :Ctrl<Key>L: MenuItem(TestLegalityProc) \n \
   :Ctrl<Key>H: MenuItem(HideThinkingProc) \n "
#endif
   "\
   :<Key>F1: MenuItem(Manual) \n \
   :<Key>F2: MenuItem(FlipView) \n \
   :<KeyDown>Return: TempBackwardProc() \n \
   :<KeyUp>Return: TempForwardProc() \n";

char boardTranslations[] =
   "<Btn1Down>: HandleUserMove(0) \n \
   Shift<Btn1Up>: HandleUserMove(1) \n \
   <Btn1Up>: HandleUserMove(0) \n \
   <Btn1Motion>: AnimateUserMove() \n \
   <Btn3Motion>: HandlePV() \n \
   <Btn2Motion>: HandlePV() \n \
   <Btn3Up>: PieceMenuPopup(menuB) \n \
   <Btn2Up>: PieceMenuPopup(menuB) \n \
   Shift<Btn2Down>: XawPositionSimpleMenu(menuB) XawPositionSimpleMenu(menuD)\
                 PieceMenuPopup(menuB) \n \
   Any<Btn2Down>: XawPositionSimpleMenu(menuW) XawPositionSimpleMenu(menuD) \
                 PieceMenuPopup(menuW) \n \
   Shift<Btn3Down>: XawPositionSimpleMenu(menuW) XawPositionSimpleMenu(menuD)\
                 PieceMenuPopup(menuW) \n \
   Any<Btn3Down>: XawPositionSimpleMenu(menuB) XawPositionSimpleMenu(menuD) \
                 PieceMenuPopup(menuB) \n";

char whiteTranslations[] =
   "Shift<BtnDown>: WhiteClock(1)\n \
   <BtnDown>: WhiteClock(0)\n";
char blackTranslations[] =
   "Shift<BtnDown>: BlackClock(1)\n \
   <BtnDown>: BlackClock(0)\n";

char ICSInputTranslations[] =
    "<Key>Up: UpKeyProc() \n "
    "<Key>Down: DownKeyProc() \n "
    "<Key>Return: EnterKeyProc() \n";

// [HGM] vari: another hideous kludge: call extend-end first so we can be sure select-start works,
//             as the widget is destroyed before the up-click can call extend-end
char commentTranslations[] = "<Btn3Down>: extend-end() select-start() CommentClick() \n";

String xboardResources[] = {
    "*fileName*value.translations: #override\\n <Key>Return: FileNameAction()",
    "*question*value.translations: #override\\n <Key>Return: AskQuestionReplyAction()",
    "*errorpopup*translations: #override\\n <Key>Return: ErrorPopDown()",
    NULL
  };


/* Max possible square size */
#define MAXSQSIZE 256

static int xpm_avail[MAXSQSIZE];

#ifdef HAVE_DIR_STRUCT

/* Extract piece size from filename */
static int
xpm_getsize (char *name, int len, char *ext)
{
    char *p, *d;
    char buf[10];

    if (len < 4)
      return 0;

    if ((p=strchr(name, '.')) == NULL ||
	StrCaseCmp(p+1, ext) != 0)
      return 0;

    p = name + 3;
    d = buf;

    while (*p && isdigit(*p))
      *(d++) = *(p++);

    *d = 0;
    return atoi(buf);
}

/* Setup xpm_avail */
static int
xpm_getavail (char *dirname, char *ext)
{
    DIR *dir;
    struct dirent *ent;
    int  i;

    for (i=0; i<MAXSQSIZE; ++i)
      xpm_avail[i] = 0;

    if (appData.debugMode)
      fprintf(stderr, "XPM dir:%s:ext:%s:\n", dirname, ext);

    dir = opendir(dirname);
    if (!dir)
      {
	  fprintf(stderr, _("%s: Can't access XPM directory %s\n"),
		  programName, dirname);
	  exit(1);
      }

    while ((ent=readdir(dir)) != NULL) {
	i = xpm_getsize(ent->d_name, NAMLEN(ent), ext);
	if (i > 0 && i < MAXSQSIZE)
	  xpm_avail[i] = 1;
    }

    closedir(dir);

    return 0;
}

void
xpm_print_avail (FILE *fp, char *ext)
{
    int i;

    fprintf(fp, _("Available `%s' sizes:\n"), ext);
    for (i=1; i<MAXSQSIZE; ++i) {
	if (xpm_avail[i])
	  printf("%d\n", i);
    }
}

/* Return XPM piecesize closest to size */
int
xpm_closest_to (char *dirname, int size, char *ext)
{
    int i;
    int sm_diff = MAXSQSIZE;
    int sm_index = 0;
    int diff;

    xpm_getavail(dirname, ext);

    if (appData.debugMode)
      xpm_print_avail(stderr, ext);

    for (i=1; i<MAXSQSIZE; ++i) {
	if (xpm_avail[i]) {
	    diff = size - i;
	    diff = (diff<0) ? -diff : diff;
	    if (diff < sm_diff) {
		sm_diff = diff;
		sm_index = i;
	    }
	}
    }

    if (!sm_index) {
	fprintf(stderr, _("Error: No `%s' files!\n"), ext);
	exit(1);
    }

    return sm_index;
}
#else	/* !HAVE_DIR_STRUCT */
/* If we are on a system without a DIR struct, we can't
   read the directory, so we can't collect a list of
   filenames, etc., so we can't do any size-fitting. */
int
xpm_closest_to (char *dirname, int size, char *ext)
{
    fprintf(stderr, _("\
Warning: No DIR structure found on this system --\n\
         Unable to autosize for XPM/XIM pieces.\n\
   Please report this error to %s.\n\
   Include system type & operating system in message.\n"), PACKAGE_BUGREPORT););
    return size;
}
#endif /* HAVE_DIR_STRUCT */

static char *cnames[9] = { "black", "red", "green", "yellow", "blue",
			     "magenta", "cyan", "white" };
typedef struct {
    int attr, bg, fg;
} TextColors;
TextColors textColors[(int)NColorClasses];

/* String is: "fg, bg, attr". Which is 0, 1, 2 */
static int
parse_color (char *str, int which)
{
    char *p, buf[100], *d;
    int i;

    if (strlen(str) > 99)	/* watch bounds on buf */
      return -1;

    p = str;
    d = buf;
    for (i=0; i<which; ++i) {
	p = strchr(p, ',');
	if (!p)
	  return -1;
	++p;
    }

    /* Could be looking at something like:
       black, , 1
       .. in which case we want to stop on a comma also */
    while (*p && *p != ',' && !isalpha(*p) && !isdigit(*p))
      ++p;

    if (*p == ',') {
	return -1;		/* Use default for empty field */
    }

    if (which == 2 || isdigit(*p))
      return atoi(p);

    while (*p && isalpha(*p))
      *(d++) = *(p++);

    *d = 0;

    for (i=0; i<8; ++i) {
	if (!StrCaseCmp(buf, cnames[i]))
	  return which? (i+40) : (i+30);
    }
    if (!StrCaseCmp(buf, "default")) return -1;

    fprintf(stderr, _("%s: unrecognized color %s\n"), programName, buf);
    return -2;
}

static int
parse_cpair (ColorClass cc, char *str)
{
    if ((textColors[(int)cc].fg=parse_color(str, 0)) == -2) {
	fprintf(stderr, _("%s: can't parse foreground color in `%s'\n"),
		programName, str);
	return -1;
    }

    /* bg and attr are optional */
    textColors[(int)cc].bg = parse_color(str, 1);
    if ((textColors[(int)cc].attr = parse_color(str, 2)) < 0) {
	textColors[(int)cc].attr = 0;
    }
    return 0;
}


/* Arrange to catch delete-window events */
Atom wm_delete_window;
void
CatchDeleteWindow (Widget w, String procname)
{
  char buf[MSG_SIZ];
  XSetWMProtocols(xDisplay, XtWindow(w), &wm_delete_window, 1);
  snprintf(buf, sizeof(buf), "<Message>WM_PROTOCOLS: %s() \n", procname);
  XtAugmentTranslations(w, XtParseTranslationTable(buf));
}

void
BoardToTop ()
{
  Arg args[16];
  XtSetArg(args[0], XtNiconic, False);
  XtSetValues(shellWidget, args, 1);

  XtPopup(shellWidget, XtGrabNone); /* Raise if lowered  */
}

//---------------------------------------------------------------------------------------------------------
// some symbol definitions to provide the proper (= XBoard) context for the code in args.h
#define XBOARD True
#define JAWS_ARGS
#define CW_USEDEFAULT (1<<31)
#define ICS_TEXT_MENU_SIZE 90
#define DEBUG_FILE "xboard.debug"
#define SetCurrentDirectory chdir
#define GetCurrentDirectory(SIZE, NAME) getcwd(NAME, SIZE)
#define OPTCHAR "-"
#define SEPCHAR " "

// these two must some day move to frontend.h, when they are implemented
Boolean GameListIsUp();

// The option definition and parsing code common to XBoard and WinBoard is collected in this file
#include "args.h"

// front-end part of option handling

// [HGM] This platform-dependent table provides the location for storing the color info
extern char *crWhite, * crBlack;

void *
colorVariable[] = {
  &appData.whitePieceColor,
  &appData.blackPieceColor,
  &appData.lightSquareColor,
  &appData.darkSquareColor,
  &appData.highlightSquareColor,
  &appData.premoveHighlightColor,
  &appData.lowTimeWarningColor,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &crWhite,
  &crBlack,
  NULL
};

// [HGM] font: keep a font for each square size, even non-stndard ones
#define NUM_SIZES 18
#define MAX_SIZE 130
Boolean fontIsSet[NUM_FONTS], fontValid[NUM_FONTS][MAX_SIZE];
char *fontTable[NUM_FONTS][MAX_SIZE];

void
ParseFont (char *name, int number)
{ // in XBoard, only 2 of the fonts are currently implemented, and we just copy their name
  int size;
  if(sscanf(name, "size%d:", &size)) {
    // [HGM] font: font is meant for specific boardSize (likely from settings file);
    //       defer processing it until we know if it matches our board size
    if(size >= 0 && size<MAX_SIZE) { // for now, fixed limit
	fontTable[number][size] = strdup(strchr(name, ':')+1);
	fontValid[number][size] = True;
    }
    return;
  }
  switch(number) {
    case 0: // CLOCK_FONT
	appData.clockFont = strdup(name);
      break;
    case 1: // MESSAGE_FONT
	appData.font = strdup(name);
      break;
    case 2: // COORD_FONT
	appData.coordFont = strdup(name);
      break;
    default:
      return;
  }
  fontIsSet[number] = True; // [HGM] font: indicate a font was specified (not from settings file)
}

void
SetFontDefaults ()
{ // only 2 fonts currently
  appData.clockFont = CLOCK_FONT_NAME;
  appData.coordFont = COORD_FONT_NAME;
  appData.font  =   DEFAULT_FONT_NAME;
}

void
CreateFonts ()
{ // no-op, until we identify the code for this already in XBoard and move it here
}

void
ParseColor (int n, char *name)
{ // in XBoard, just copy the color-name string
  if(colorVariable[n]) *(char**)colorVariable[n] = strdup(name);
}

void
ParseTextAttribs (ColorClass cc, char *s)
{
    (&appData.colorShout)[cc] = strdup(s);
}

void
ParseBoardSize (void *addr, char *name)
{
    appData.boardSize = strdup(name);
}

void
LoadAllSounds ()
{ // In XBoard the sound-playing program takes care of obtaining the actual sound
}

void
SetCommPortDefaults ()
{ // for now, this is a no-op, as the corresponding option does not exist in XBoard
}

// [HGM] args: these three cases taken out to stay in front-end
void
SaveFontArg (FILE *f, ArgDescriptor *ad)
{
  char *name;
  int i, n = (int)(intptr_t)ad->argLoc;
  switch(n) {
    case 0: // CLOCK_FONT
	name = appData.clockFont;
      break;
    case 1: // MESSAGE_FONT
	name = appData.font;
      break;
    case 2: // COORD_FONT
	name = appData.coordFont;
      break;
    default:
      return;
  }
  for(i=0; i<NUM_SIZES; i++) // [HGM] font: current font becomes standard for current size
    if(sizeDefaults[i].squareSize == squareSize) { // only for standard sizes!
	fontTable[n][squareSize] = strdup(name);
	fontValid[n][squareSize] = True;
	break;
  }
  for(i=0; i<MAX_SIZE; i++) if(fontValid[n][i]) // [HGM] font: store all standard fonts
    fprintf(f, OPTCHAR "%s" SEPCHAR "\"size%d:%s\"\n", ad->argName, i, fontTable[n][i]);
}

void
ExportSounds ()
{ // nothing to do, as the sounds are at all times represented by their text-string names already
}

void
SaveAttribsArg (FILE *f, ArgDescriptor *ad)
{	// here the "argLoc" defines a table index. It could have contained the 'ta' pointer itself, though
	fprintf(f, OPTCHAR "%s" SEPCHAR "%s\n", ad->argName, (&appData.colorShout)[(int)(intptr_t)ad->argLoc]);
}

void
SaveColor (FILE *f, ArgDescriptor *ad)
{	// in WinBoard the color is an int and has to be converted to text. In X it would be a string already?
	if(colorVariable[(int)(intptr_t)ad->argLoc])
	fprintf(f, OPTCHAR "%s" SEPCHAR "%s\n", ad->argName, *(char**)colorVariable[(int)(intptr_t)ad->argLoc]);
}

void
SaveBoardSize (FILE *f, char *name, void *addr)
{ // wrapper to shield back-end from BoardSize & sizeInfo
  fprintf(f, OPTCHAR "%s" SEPCHAR "%s\n", name, appData.boardSize);
}

void
ParseCommPortSettings (char *s)
{ // no such option in XBoard (yet)
}

extern Widget engineOutputShell;
int frameX, frameY;

void
GetActualPlacement (Widget wg, WindowPlacement *wp)
{
  Arg args[16];
  Dimension w, h;
  Position x, y;
  XWindowAttributes winAt;
  Window win, dummy;
  int i, rx, ry;

  if(!wg) return;

    win = XtWindow(wg);
    XGetWindowAttributes(xDisplay, win, &winAt); // this works, where XtGetValues on XtNx, XtNy does not!
    XTranslateCoordinates (xDisplay, win, winAt.root, -winAt.border_width, -winAt.border_width, &rx, &ry, &dummy);
    wp->x = rx - winAt.x;
    wp->y = ry - winAt.y;
    wp->height = winAt.height;
    wp->width = winAt.width;
    frameX = winAt.x; frameY = winAt.y; // remember to decide if windows touch
}

void
GetWindowCoords ()
{ // wrapper to shield use of window handles from back-end (make addressible by number?)
  // In XBoard this will have to wait until awareness of window parameters is implemented
  GetActualPlacement(shellWidget, &wpMain);
  if(EngineOutputIsUp()) GetActualPlacement(engineOutputShell, &wpEngineOutput);
  if(MoveHistoryIsUp()) GetActualPlacement(shells[HistoryDlg], &wpMoveHistory);
  if(EvalGraphIsUp()) GetActualPlacement(evalGraphShell, &wpEvalGraph);
  if(GameListIsUp()) GetActualPlacement(gameListShell, &wpGameList);
  if(shellUp[CommentDlg]) GetActualPlacement(shells[CommentDlg], &wpComment);
  if(shellUp[TagsDlg]) GetActualPlacement(shells[TagsDlg], &wpTags);
}

void
PrintCommPortSettings (FILE *f, char *name)
{ // This option does not exist in XBoard
}

int
MySearchPath (char *installDir, char *name, char *fullname)
{ // just append installDir and name. Perhaps ExpandPath should be used here?
  name = ExpandPathName(name);
  if(name && name[0] == '/')
    safeStrCpy(fullname, name, MSG_SIZ );
  else {
    sprintf(fullname, "%s%c%s", installDir, '/', name);
  }
  return 1;
}

int
MyGetFullPathName (char *name, char *fullname)
{ // should use ExpandPath?
  name = ExpandPathName(name);
  safeStrCpy(fullname, name, MSG_SIZ );
  return 1;
}

void
EnsureOnScreen (int *x, int *y, int minX, int minY)
{
  return;
}

int
MainWindowUp ()
{ // [HGM] args: allows testing if main window is realized from back-end
  return xBoardWindow != 0;
}

void
PopUpStartupDialog ()
{  // start menu not implemented in XBoard
}

char *
ConvertToLine (int argc, char **argv)
{
  static char line[128*1024], buf[1024];
  int i;

  line[0] = NULLCHAR;
  for(i=1; i<argc; i++)
    {
      if( (strchr(argv[i], ' ') || strchr(argv[i], '\n') ||strchr(argv[i], '\t') || argv[i][0] == NULLCHAR)
	  && argv[i][0] != '{' )
	snprintf(buf, sizeof(buf)/sizeof(buf[0]), "{%s} ", argv[i]);
      else
	snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%s ", argv[i]);
      strncat(line, buf, 128*1024 - strlen(line) - 1 );
    }

  line[strlen(line)-1] = NULLCHAR;
  return line;
}

//--------------------------------------------------------------------------------------------

#ifdef IDSIZES
  // eventually, all layout determining code should go into a subroutine, but until then IDSIZE remains undefined
#else
#define BoardSize int
void
InitDrawingSizes (BoardSize boardSize, int flags)
{   // [HGM] resize is functional now, but for board format changes only (nr of ranks, files)
    Dimension timerWidth, boardWidth, boardHeight, w, h, sep, bor, wr, hr;
    Arg args[16];
    XtGeometryResult gres;
    int i;
    static Dimension oldWidth, oldHeight;
    static VariantClass oldVariant;
    static int oldDual = -1, oldMono = -1;

    if(!formWidget) return;

    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap;
    boardWidth = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);

  if(boardWidth != oldWidth || boardHeight != oldHeight || oldDual != twoBoards) { // do resizing stuff only if size actually changed
    /*
     * Enable shell resizing.
     */
    shellArgs[0].value = (XtArgVal) &w;
    shellArgs[1].value = (XtArgVal) &h;
    XtGetValues(shellWidget, shellArgs, 2);

    shellArgs[4].value = 3*w; shellArgs[2].value = 10;
    shellArgs[5].value = 2*h; shellArgs[3].value = 10;
    XtSetValues(shellWidget, &shellArgs[2], 4);

    XtSetArg(args[0], XtNdefaultDistance, &sep);
    XtGetValues(formWidget, args, 1);

    oldWidth = boardWidth; oldHeight = boardHeight; oldDual = twoBoards;
    CreateGrid();
    hOffset = boardWidth + 10;
    for(i=0; i<BOARD_WIDTH+BOARD_HEIGHT+2; i++) { // [HGM] dual: grid for second board
	secondSegments[i] = gridSegments[i];
	secondSegments[i].x1 += hOffset;
	secondSegments[i].x2 += hOffset;
    }

    XtSetArg(args[0], XtNwidth, boardWidth);
    XtSetArg(args[1], XtNheight, boardHeight);
    XtSetValues(boardWidget, args, 2);

    timerWidth = (boardWidth - sep) / 2;
    XtSetArg(args[0], XtNwidth, timerWidth);
    XtSetValues(whiteTimerWidget, args, 1);
    XtSetValues(blackTimerWidget, args, 1);

    XawFormDoLayout(formWidget, False);

    if (appData.titleInWindow) {
	i = 0;
	XtSetArg(args[i], XtNborderWidth, &bor); i++;
	XtSetArg(args[i], XtNheight, &h);  i++;
	XtGetValues(titleWidget, args, i);
	if (smallLayout) {
	    w = boardWidth - 2*bor;
	} else {
	    XtSetArg(args[0], XtNwidth, &w);
	    XtGetValues(menuBarWidget, args, 1);
	    w = boardWidth - w - sep - 2*bor - 2; // WIDTH_FUDGE
	}

	gres = XtMakeResizeRequest(titleWidget, w, h, &wr, &hr);
	if (gres != XtGeometryYes && appData.debugMode) {
	    fprintf(stderr,
		    _("%s: titleWidget geometry error %d %d %d %d %d\n"),
		    programName, gres, w, h, wr, hr);
	}
    }

    XawFormDoLayout(formWidget, True);

    /*
     * Inhibit shell resizing.
     */
    shellArgs[0].value = w = (XtArgVal) boardWidth + marginW + twoBoards*hOffset; // [HGM] dual
    shellArgs[1].value = h = (XtArgVal) boardHeight + marginH;
    shellArgs[4].value = shellArgs[2].value = w;
    shellArgs[5].value = shellArgs[3].value = h;
    XtSetValues(shellWidget, &shellArgs[0], 6);

    XSync(xDisplay, False);
    DelayedDrag();
  }

    // [HGM] pieces: tailor piece bitmaps to needs of specific variant
    // (only for xpm)

  if(gameInfo.variant != oldVariant) { // and only if variant changed

    if(useImages) {
      for(i=0; i<4; i++) {
	int p;
	for(p=0; p<=(int)WhiteKing; p++)
	   xpmPieceBitmap[i][p] = xpmPieceBitmap2[i][p]; // defaults
	if(gameInfo.variant == VariantShogi) {
	   xpmPieceBitmap[i][(int)WhiteCannon] = xpmPieceBitmap2[i][(int)WhiteKing+1];
	   xpmPieceBitmap[i][(int)WhiteNightrider] = xpmPieceBitmap2[i][(int)WhiteKing+2];
	   xpmPieceBitmap[i][(int)WhiteSilver] = xpmPieceBitmap2[i][(int)WhiteKing+3];
	   xpmPieceBitmap[i][(int)WhiteGrasshopper] = xpmPieceBitmap2[i][(int)WhiteKing+4];
	   xpmPieceBitmap[i][(int)WhiteQueen] = xpmPieceBitmap2[i][(int)WhiteLance];
	}
#ifdef GOTHIC
	if(gameInfo.variant == VariantGothic) {
	   xpmPieceBitmap[i][(int)WhiteMarshall] = xpmPieceBitmap2[i][(int)WhiteSilver];
	}
#endif
	if(gameInfo.variant == VariantSChess && (squareSize == 49 || squareSize == 72)) {
	   xpmPieceBitmap[i][(int)WhiteAngel]    = xpmPieceBitmap2[i][(int)WhiteFalcon];
	   xpmPieceBitmap[i][(int)WhiteMarshall] = xpmPieceBitmap2[i][(int)WhiteAlfil];
	}
#if !HAVE_LIBXPM
	// [HGM] why are thee ximMasks used at all? the ximPieceBitmaps seem to be never used!
	for(p=0; p<=(int)WhiteKing; p++)
	   ximMaskPm[p] = ximMaskPm2[p]; // defaults
	if(gameInfo.variant == VariantShogi) {
	   ximMaskPm[(int)WhiteCannon] = ximMaskPm2[(int)WhiteKing+1];
	   ximMaskPm[(int)WhiteNightrider] = ximMaskPm2[(int)WhiteKing+2];
	   ximMaskPm[(int)WhiteSilver] = ximMaskPm2[(int)WhiteKing+3];
	   ximMaskPm[(int)WhiteGrasshopper] = ximMaskPm2[(int)WhiteKing+4];
	   ximMaskPm[(int)WhiteQueen] = ximMaskPm2[(int)WhiteLance];
	}
#ifdef GOTHIC
	if(gameInfo.variant == VariantGothic) {
           ximMaskPm[(int)WhiteMarshall] = ximMaskPm2[(int)WhiteSilver];
	}
#endif
	if(gameInfo.variant == VariantSChess && (squareSize == 49 || squareSize == 72)) {
           ximMaskPm[(int)WhiteAngel]    = ximMaskPm2[(int)WhiteFalcon];
           ximMaskPm[(int)WhiteMarshall] = ximMaskPm2[(int)WhiteAlfil];
	}
#endif
      }
    } else {
      for(i=0; i<2; i++) {
	int p;
	for(p=0; p<=(int)WhiteKing; p++)
	   pieceBitmap[i][p] = pieceBitmap2[i][p]; // defaults
	if(gameInfo.variant == VariantShogi) {
	   pieceBitmap[i][(int)WhiteCannon] = pieceBitmap2[i][(int)WhiteKing+1];
	   pieceBitmap[i][(int)WhiteNightrider] = pieceBitmap2[i][(int)WhiteKing+2];
	   pieceBitmap[i][(int)WhiteSilver] = pieceBitmap2[i][(int)WhiteKing+3];
	   pieceBitmap[i][(int)WhiteGrasshopper] = pieceBitmap2[i][(int)WhiteKing+4];
	   pieceBitmap[i][(int)WhiteQueen] = pieceBitmap2[i][(int)WhiteLance];
	}
#ifdef GOTHIC
	if(gameInfo.variant == VariantGothic) {
	   pieceBitmap[i][(int)WhiteMarshall] = pieceBitmap2[i][(int)WhiteSilver];
	}
#endif
	if(gameInfo.variant == VariantSChess && (squareSize == 49 || squareSize == 72)) {
	   pieceBitmap[i][(int)WhiteAngel]    = pieceBitmap2[i][(int)WhiteFalcon];
	   pieceBitmap[i][(int)WhiteMarshall] = pieceBitmap2[i][(int)WhiteAlfil];
	}
      }
    }
    oldMono = -10; // kludge to force recreation of animation masks
    oldVariant = gameInfo.variant;
  }
#if HAVE_LIBXPM
  if(appData.monoMode != oldMono)
    CreateAnimVars();
#endif
  oldMono = appData.monoMode;
}
#endif

void
ParseIcsTextColors ()
{   // [HGM] tken out of main(), so it can be called from ICS-Options dialog
    if (parse_cpair(ColorShout, appData.colorShout) < 0 ||
	parse_cpair(ColorSShout, appData.colorSShout) < 0 ||
	parse_cpair(ColorChannel1, appData.colorChannel1) < 0  ||
	parse_cpair(ColorChannel, appData.colorChannel) < 0  ||
	parse_cpair(ColorKibitz, appData.colorKibitz) < 0 ||
	parse_cpair(ColorTell, appData.colorTell) < 0 ||
	parse_cpair(ColorChallenge, appData.colorChallenge) < 0  ||
	parse_cpair(ColorRequest, appData.colorRequest) < 0  ||
	parse_cpair(ColorSeek, appData.colorSeek) < 0  ||
	parse_cpair(ColorNormal, appData.colorNormal) < 0)
      {
	  if (appData.colorize) {
	      fprintf(stderr,
		      _("%s: can't parse color names; disabling colorization\n"),
		      programName);
	  }
	  appData.colorize = FALSE;
      }
}

static int
MakeOneColor (char *name, Pixel *color)
{
    XrmValue vFrom, vTo;
    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) name;
	vFrom.size = strlen(name);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  return True;
	} else {
	  *color = *(Pixel *) vTo.addr;
	}
    }
    return False;
}

static int
MakeColors ()
{   // [HGM] taken out of main(), so it can be called from BoardOptions dialog
    int forceMono = False;

    forceMono |= MakeOneColor(appData.lightSquareColor, &lightSquareColor);
    forceMono |= MakeOneColor(appData.darkSquareColor, &darkSquareColor);
    forceMono |= MakeOneColor(appData.whitePieceColor, &whitePieceColor);
    forceMono |= MakeOneColor(appData.blackPieceColor, &blackPieceColor);
    forceMono |= MakeOneColor(appData.highlightSquareColor, &highlightSquareColor);
    forceMono |= MakeOneColor(appData.premoveHighlightColor, &premoveHighlightColor);

    return forceMono;
}

static void
CreateAnyPieces ()
{   // [HGM] taken out of main
#if HAVE_LIBXPM
    if (appData.monoMode && // [HGM] no sense to go on to certain doom
       (appData.bitmapDirectory == NULL || appData.bitmapDirectory[0] == NULLCHAR))
	    appData.bitmapDirectory = strdup(DEF_BITMAP_DIR);

    if (appData.bitmapDirectory[0] != NULLCHAR) {
      CreatePieces();
    } else {
      CreateXPMPieces();
      CreateXPMBoard(appData.liteBackTextureFile, 1);
      CreateXPMBoard(appData.darkBackTextureFile, 0);
    }
#else
    CreateXIMPieces();
    /* Create regular pieces */
    if (!useImages) CreatePieces();
#endif
}

void
InitDrawingParams ()
{
    MakeColors(); CreateGCs(True);
    CreateAnyPieces();
}

int
main (int argc, char **argv)
{
    int i, j, clockFontPxlSize, coordFontPxlSize, fontPxlSize;
    XSetWindowAttributes window_attributes;
    Arg args[16];
    Dimension timerWidth, boardWidth, boardHeight, w, h, sep, bor, wr, hr;
    XrmValue vFrom, vTo;
    XtGeometryResult gres;
    char *p;
    XrmDatabase xdb;
    int forceMono = False;

    srandom(time(0)); // [HGM] book: make random truly random

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    debugFP = stderr;

    if(argc > 1 && (!strcmp(argv[1], "-v" ) || !strcmp(argv[1], "--version" ))) {
	printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
	exit(0);
    }

    programName = strrchr(argv[0], '/');
    if (programName == NULL)
      programName = argv[0];
    else
      programName++;

#ifdef ENABLE_NLS
    XtSetLanguageProc(NULL, NULL, NULL);
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    shellWidget =
      XtAppInitialize(&appContext, "XBoard", shellOptions,
		      XtNumber(shellOptions),
		      &argc, argv, xboardResources, NULL, 0);
    appData.boardSize = "";
    InitAppData(ConvertToLine(argc, argv));
    p = getenv("HOME");
    if (p == NULL) p = "/tmp";
    i = strlen(p) + strlen("/.xboardXXXXXx.pgn") + 1;
    gameCopyFilename = (char*) malloc(i);
    gamePasteFilename = (char*) malloc(i);
    snprintf(gameCopyFilename,i, "%s/.xboard%05uc.pgn", p, getpid());
    snprintf(gamePasteFilename,i, "%s/.xboard%05up.pgn", p, getpid());

    XtGetApplicationResources(shellWidget, (XtPointer) &appData,
			      clientResources, XtNumber(clientResources),
			      NULL, 0);

    { // [HGM] initstring: kludge to fix bad bug. expand '\n' characters in init string and computer string.
	static char buf[MSG_SIZ];
	EscapeExpand(buf, appData.firstInitString);
	appData.firstInitString = strdup(buf);
	EscapeExpand(buf, appData.secondInitString);
	appData.secondInitString = strdup(buf);
	EscapeExpand(buf, appData.firstComputerString);
	appData.firstComputerString = strdup(buf);
	EscapeExpand(buf, appData.secondComputerString);
	appData.secondComputerString = strdup(buf);
    }

    if ((chessDir = (char *) getenv("CHESSDIR")) == NULL) {
	chessDir = ".";
    } else {
	if (chdir(chessDir) != 0) {
	    fprintf(stderr, _("%s: can't cd to CHESSDIR: "), programName);
	    perror(chessDir);
	    exit(1);
	}
    }

    if (appData.debugMode && appData.nameOfDebugFile && strcmp(appData.nameOfDebugFile, "stderr")) {
	/* [DM] debug info to file [HGM] make the filename a command-line option, and allow it to remain stderr */
        if ((debugFP = fopen(appData.nameOfDebugFile, "w")) == NULL)  {
           printf(_("Failed to open file '%s'\n"), appData.nameOfDebugFile);
           exit(errno);
        }
        setbuf(debugFP, NULL);
    }

#if ENABLE_NLS
    if (appData.debugMode) {
      fprintf(debugFP, "locale = %s\n", setlocale(LC_ALL, NULL));
    }
#endif

    /* [HGM,HR] make sure board size is acceptable */
    if(appData.NrFiles > BOARD_FILES ||
       appData.NrRanks > BOARD_RANKS   )
	 DisplayFatalError(_("Recompile with larger BOARD_RANKS or BOARD_FILES to support this size"), 0, 2);

#if !HIGHDRAG
    /* This feature does not work; animation needs a rewrite */
    appData.highlightDragging = FALSE;
#endif
    InitBackEnd1();

    xDisplay = XtDisplay(shellWidget);
    xScreen = DefaultScreen(xDisplay);
    wm_delete_window = XInternAtom(xDisplay, "WM_DELETE_WINDOW", True);

	gameInfo.variant = StringToVariant(appData.variant);
	InitPosition(FALSE);

#ifdef IDSIZE
    InitDrawingSizes(-1, 0); // [HGM] initsize: make this into a subroutine
#else
    if (isdigit(appData.boardSize[0])) {
        i = sscanf(appData.boardSize, "%d,%d,%d,%d,%d,%d,%d", &squareSize,
		   &lineGap, &clockFontPxlSize, &coordFontPxlSize,
		   &fontPxlSize, &smallLayout, &tinyLayout);
        if (i == 0) {
	    fprintf(stderr, _("%s: bad boardSize syntax %s\n"),
		    programName, appData.boardSize);
	    exit(2);
	}
	if (i < 7) {
	    /* Find some defaults; use the nearest known size */
	    SizeDefaults *szd, *nearest;
	    int distance = 99999;
	    nearest = szd = sizeDefaults;
	    while (szd->name != NULL) {
		if (abs(szd->squareSize - squareSize) < distance) {
		    nearest = szd;
		    distance = abs(szd->squareSize - squareSize);
		    if (distance == 0) break;
		}
		szd++;
	    }
	    if (i < 2) lineGap = nearest->lineGap;
	    if (i < 3) clockFontPxlSize = nearest->clockFontPxlSize;
	    if (i < 4) coordFontPxlSize = nearest->coordFontPxlSize;
	    if (i < 5) fontPxlSize = nearest->fontPxlSize;
	    if (i < 6) smallLayout = nearest->smallLayout;
	    if (i < 7) tinyLayout = nearest->tinyLayout;
	}
    } else {
        SizeDefaults *szd = sizeDefaults;
        if (*appData.boardSize == NULLCHAR) {
	    while (DisplayWidth(xDisplay, xScreen) < szd->minScreenSize ||
		   DisplayHeight(xDisplay, xScreen) < szd->minScreenSize) {
	      szd++;
	    }
	    if (szd->name == NULL) szd--;
	    appData.boardSize = strdup(szd->name); // [HGM] settings: remember name for saving settings
	} else {
	    while (szd->name != NULL &&
		   StrCaseCmp(szd->name, appData.boardSize) != 0) szd++;
	    if (szd->name == NULL) {
		fprintf(stderr, _("%s: unrecognized boardSize name %s\n"),
			programName, appData.boardSize);
		exit(2);
	    }
	}
	squareSize = szd->squareSize;
	lineGap = szd->lineGap;
	clockFontPxlSize = szd->clockFontPxlSize;
	coordFontPxlSize = szd->coordFontPxlSize;
	fontPxlSize = szd->fontPxlSize;
	smallLayout = szd->smallLayout;
	tinyLayout = szd->tinyLayout;
	// [HGM] font: use defaults from settings file if available and not overruled
    }
    if(!fontIsSet[CLOCK_FONT] && fontValid[CLOCK_FONT][squareSize])
	appData.clockFont = fontTable[CLOCK_FONT][squareSize];
    if(!fontIsSet[MESSAGE_FONT] && fontValid[MESSAGE_FONT][squareSize])
	appData.font = fontTable[MESSAGE_FONT][squareSize];
    if(!fontIsSet[COORD_FONT] && fontValid[COORD_FONT][squareSize])
	appData.coordFont = fontTable[COORD_FONT][squareSize];

    /* Now, using squareSize as a hint, find a good XPM/XIM set size */
    if (strlen(appData.pixmapDirectory) > 0) {
	p = ExpandPathName(appData.pixmapDirectory);
	if (!p) {
	    fprintf(stderr, _("Error expanding path name \"%s\"\n"),
		   appData.pixmapDirectory);
	    exit(1);
	}
	if (appData.debugMode) {
          fprintf(stderr, _("\
XBoard square size (hint): %d\n\
%s fulldir:%s:\n"), squareSize, IMAGE_EXT, p);
	}
	squareSize = xpm_closest_to(p, squareSize, IMAGE_EXT);
	if (appData.debugMode) {
	    fprintf(stderr, _("Closest %s size: %d\n"), IMAGE_EXT, squareSize);
	}
    }
    defaultLineGap = lineGap;
    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap;

    /* [HR] height treated separately (hacked) */
    boardWidth = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);
    if (appData.showJail == 1) {
	/* Jail on top and bottom */
	XtSetArg(boardArgs[1], XtNwidth, boardWidth);
	XtSetArg(boardArgs[2], XtNheight,
		 boardHeight + 2*(lineGap + squareSize));
    } else if (appData.showJail == 2) {
	/* Jail on sides */
	XtSetArg(boardArgs[1], XtNwidth,
		 boardWidth + 2*(lineGap + squareSize));
	XtSetArg(boardArgs[2], XtNheight, boardHeight);
    } else {
	/* No jail */
	XtSetArg(boardArgs[1], XtNwidth, boardWidth);
	XtSetArg(boardArgs[2], XtNheight, boardHeight);
    }

    /*
     * Determine what fonts to use.
     */
#if ENABLE_NLS
    appData.font = InsertPxlSize(appData.font, fontPxlSize);
    appData.clockFont = InsertPxlSize(appData.clockFont, clockFontPxlSize);
    appData.coordFont = InsertPxlSize(appData.coordFont, coordFontPxlSize);
    fontSet = CreateFontSet(appData.font);
    clockFontSet = CreateFontSet(appData.clockFont);
    {
      /* For the coordFont, use the 0th font of the fontset. */
      XFontSet coordFontSet = CreateFontSet(appData.coordFont);
      XFontStruct **font_struct_list;
      XFontSetExtents *fontSize;
      char **font_name_list;
      XFontsOfFontSet(coordFontSet, &font_struct_list, &font_name_list);
      coordFontID = XLoadFont(xDisplay, font_name_list[0]);
      coordFontStruct = XQueryFont(xDisplay, coordFontID);
      fontSize = XExtentsOfFontSet(fontSet); // [HGM] figure out how much vertical space font takes
      textHeight = fontSize->max_logical_extent.height + 5; // add borderWidth
    }
#else
    appData.font = FindFont(appData.font, fontPxlSize);
    appData.clockFont = FindFont(appData.clockFont, clockFontPxlSize);
    appData.coordFont = FindFont(appData.coordFont, coordFontPxlSize);
    clockFontID = XLoadFont(xDisplay, appData.clockFont);
    clockFontStruct = XQueryFont(xDisplay, clockFontID);
    coordFontID = XLoadFont(xDisplay, appData.coordFont);
    coordFontStruct = XQueryFont(xDisplay, coordFontID);
#endif
    countFontID = coordFontID;  // [HGM] holdings
    countFontStruct = coordFontStruct;

    xdb = XtDatabase(xDisplay);
#if ENABLE_NLS
    XrmPutLineResource(&xdb, "*international: True");
    vTo.size = sizeof(XFontSet);
    vTo.addr = (XtPointer) &fontSet;
    XrmPutResource(&xdb, "*fontSet", XtRFontSet, &vTo);
#else
    XrmPutStringResource(&xdb, "*font", appData.font);
#endif

    /*
     * Detect if there are not enough colors available and adapt.
     */
    if (DefaultDepth(xDisplay, xScreen) <= 2) {
      appData.monoMode = True;
    }

    forceMono = MakeColors();

    if (forceMono) {
      fprintf(stderr, _("%s: too few colors available; trying monochrome mode\n"),
	      programName);
	appData.monoMode = True;
    }

    if (appData.lowTimeWarning && !appData.monoMode) {
      vFrom.addr = (caddr_t) appData.lowTimeWarningColor;
      vFrom.size = strlen(appData.lowTimeWarningColor);
      XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
      if (vTo.addr == NULL)
		appData.monoMode = True;
      else
		lowTimeWarningColor = *(Pixel *) vTo.addr;
    }

    if (appData.monoMode && appData.debugMode) {
	fprintf(stderr, _("white pixel = 0x%lx, black pixel = 0x%lx\n"),
		(unsigned long) XWhitePixel(xDisplay, xScreen),
		(unsigned long) XBlackPixel(xDisplay, xScreen));
    }

    ParseIcsTextColors();
    textColors[ColorNone].fg = textColors[ColorNone].bg = -1;
    textColors[ColorNone].attr = 0;

    XtAppAddActions(appContext, boardActions, XtNumber(boardActions));

    /*
     * widget hierarchy
     */
    if (tinyLayout) {
	layoutName = "tinyLayout";
    } else if (smallLayout) {
	layoutName = "smallLayout";
    } else {
	layoutName = "normalLayout";
    }
    /* Outer layoutWidget is there only to provide a name for use in
       resources that depend on the layout style */
    layoutWidget =
      XtCreateManagedWidget(layoutName, formWidgetClass, shellWidget,
			    layoutArgs, XtNumber(layoutArgs));
    formWidget =
      XtCreateManagedWidget("form", formWidgetClass, layoutWidget,
			    formArgs, XtNumber(formArgs));
    XtSetArg(args[0], XtNdefaultDistance, &sep);
    XtGetValues(formWidget, args, 1);

    j = 0;
    widgetList[j++] = menuBarWidget = CreateMenuBar(menuBar, boardWidth);
    XtSetArg(args[0], XtNtop,    XtChainTop);
    XtSetArg(args[1], XtNbottom, XtChainTop);
    XtSetArg(args[2], XtNright,  XtChainLeft);
    XtSetValues(menuBarWidget, args, 3);

    widgetList[j++] = whiteTimerWidget =
      XtCreateWidget("whiteTime", labelWidgetClass,
		     formWidget, timerArgs, XtNumber(timerArgs));
#if ENABLE_NLS
    XtSetArg(args[0], XtNfontSet, clockFontSet);
#else
    XtSetArg(args[0], XtNfont, clockFontStruct);
#endif
    XtSetArg(args[1], XtNtop,    XtChainTop);
    XtSetArg(args[2], XtNbottom, XtChainTop);
    XtSetValues(whiteTimerWidget, args, 3);

    widgetList[j++] = blackTimerWidget =
      XtCreateWidget("blackTime", labelWidgetClass,
		     formWidget, timerArgs, XtNumber(timerArgs));
#if ENABLE_NLS
    XtSetArg(args[0], XtNfontSet, clockFontSet);
#else
    XtSetArg(args[0], XtNfont, clockFontStruct);
#endif
    XtSetArg(args[1], XtNtop,    XtChainTop);
    XtSetArg(args[2], XtNbottom, XtChainTop);
    XtSetValues(blackTimerWidget, args, 3);

    if (appData.titleInWindow) {
	widgetList[j++] = titleWidget =
	  XtCreateWidget("title", labelWidgetClass, formWidget,
			 titleArgs, XtNumber(titleArgs));
	XtSetArg(args[0], XtNtop,    XtChainTop);
	XtSetArg(args[1], XtNbottom, XtChainTop);
	XtSetValues(titleWidget, args, 2);
    }

    if (appData.showButtonBar) {
      widgetList[j++] = buttonBarWidget = CreateButtonBar(buttonBar);
      XtSetArg(args[0], XtNleft,  XtChainRight); // [HGM] glue to right window edge
      XtSetArg(args[1], XtNright, XtChainRight); //       for good run-time sizing
      XtSetArg(args[2], XtNtop,    XtChainTop);
      XtSetArg(args[3], XtNbottom, XtChainTop);
      XtSetValues(buttonBarWidget, args, 4);
    }

    widgetList[j++] = messageWidget =
      XtCreateWidget("message", labelWidgetClass, formWidget,
		     messageArgs, XtNumber(messageArgs));
    XtSetArg(args[0], XtNtop,    XtChainTop);
    XtSetArg(args[1], XtNbottom, XtChainTop);
    XtSetValues(messageWidget, args, 2);

    widgetList[j++] = boardWidget =
      XtCreateWidget("board", widgetClass, formWidget, boardArgs,
		     XtNumber(boardArgs));

    XtManageChildren(widgetList, j);

    timerWidth = (boardWidth - sep) / 2;
    XtSetArg(args[0], XtNwidth, timerWidth);
    XtSetValues(whiteTimerWidget, args, 1);
    XtSetValues(blackTimerWidget, args, 1);

    XtSetArg(args[0], XtNbackground, &timerBackgroundPixel);
    XtSetArg(args[1], XtNforeground, &timerForegroundPixel);
    XtGetValues(whiteTimerWidget, args, 2);

    if (appData.showButtonBar) {
      XtSetArg(args[0], XtNbackground, &buttonBackgroundPixel);
      XtSetArg(args[1], XtNforeground, &buttonForegroundPixel);
      XtGetValues(XtNameToWidget(buttonBarWidget, PAUSE_BUTTON), args, 2);
    }

    /*
     * formWidget uses these constraints but they are stored
     * in the children.
     */
    i = 0;
    XtSetArg(args[i], XtNfromHoriz, 0); i++;
    XtSetValues(menuBarWidget, args, i);
    if (appData.titleInWindow) {
	if (smallLayout) {
	    i = 0;
	    XtSetArg(args[i], XtNfromVert, menuBarWidget); i++;
	    XtSetValues(whiteTimerWidget, args, i);
	    i = 0;
	    XtSetArg(args[i], XtNfromVert, menuBarWidget); i++;
	    XtSetArg(args[i], XtNfromHoriz, whiteTimerWidget); i++;
	    XtSetValues(blackTimerWidget, args, i);
	    i = 0;
	    XtSetArg(args[i], XtNfromVert, whiteTimerWidget); i++;
            XtSetArg(args[i], XtNjustify, XtJustifyLeft); i++;
	    XtSetValues(titleWidget, args, i);
	    i = 0;
	    XtSetArg(args[i], XtNfromVert, titleWidget); i++;
	    XtSetArg(args[i], XtNresizable, (XtArgVal) True); i++;
	    XtSetValues(messageWidget, args, i);
	    if (appData.showButtonBar) {
	      i = 0;
	      XtSetArg(args[i], XtNfromVert, titleWidget); i++;
	      XtSetArg(args[i], XtNfromHoriz, messageWidget); i++;
	      XtSetValues(buttonBarWidget, args, i);
	    }
	} else {
	    i = 0;
	    XtSetArg(args[i], XtNfromVert, titleWidget); i++;
	    XtSetValues(whiteTimerWidget, args, i);
	    i = 0;
	    XtSetArg(args[i], XtNfromVert, titleWidget); i++;
	    XtSetArg(args[i], XtNfromHoriz, whiteTimerWidget); i++;
	    XtSetValues(blackTimerWidget, args, i);
	    i = 0;
	    XtSetArg(args[i], XtNfromHoriz, menuBarWidget); i++;
	    XtSetValues(titleWidget, args, i);
	    i = 0;
	    XtSetArg(args[i], XtNfromVert, whiteTimerWidget); i++;
	    XtSetArg(args[i], XtNresizable, (XtArgVal) True); i++;
	    XtSetValues(messageWidget, args, i);
	    if (appData.showButtonBar) {
	      i = 0;
	      XtSetArg(args[i], XtNfromVert, whiteTimerWidget); i++;
	      XtSetArg(args[i], XtNfromHoriz, messageWidget); i++;
	      XtSetValues(buttonBarWidget, args, i);
	    }
	}
    } else {
	i = 0;
	XtSetArg(args[i], XtNfromVert, menuBarWidget); i++;
	XtSetValues(whiteTimerWidget, args, i);
	i = 0;
	XtSetArg(args[i], XtNfromVert, menuBarWidget); i++;
	XtSetArg(args[i], XtNfromHoriz, whiteTimerWidget); i++;
	XtSetValues(blackTimerWidget, args, i);
	i = 0;
	XtSetArg(args[i], XtNfromVert, whiteTimerWidget); i++;
	XtSetArg(args[i], XtNresizable, (XtArgVal) True); i++;
	XtSetValues(messageWidget, args, i);
	if (appData.showButtonBar) {
	  i = 0;
	  XtSetArg(args[i], XtNfromVert, whiteTimerWidget); i++;
	  XtSetArg(args[i], XtNfromHoriz, messageWidget); i++;
	  XtSetValues(buttonBarWidget, args, i);
	}
    }
    i = 0;
    XtSetArg(args[0], XtNfromVert, messageWidget);
    XtSetArg(args[1], XtNtop,    XtChainTop);
    XtSetArg(args[2], XtNbottom, XtChainBottom);
    XtSetArg(args[3], XtNleft,   XtChainLeft);
    XtSetArg(args[4], XtNright,  XtChainRight);
    XtSetValues(boardWidget, args, 5);

    XtRealizeWidget(shellWidget);

    if(wpMain.x > 0) {
      XtSetArg(args[0], XtNx, wpMain.x);
      XtSetArg(args[1], XtNy, wpMain.y);
      XtSetValues(shellWidget, args, 2);
    }

    /*
     * Correct the width of the message and title widgets.
     * It is not known why some systems need the extra fudge term.
     * The value "2" is probably larger than needed.
     */
    XawFormDoLayout(formWidget, False);

#define WIDTH_FUDGE 2
    i = 0;
    XtSetArg(args[i], XtNborderWidth, &bor);  i++;
    XtSetArg(args[i], XtNheight, &h);  i++;
    XtGetValues(messageWidget, args, i);
    if (appData.showButtonBar) {
      i = 0;
      XtSetArg(args[i], XtNwidth, &w);  i++;
      XtGetValues(buttonBarWidget, args, i);
      w = boardWidth - w - sep - 2*bor - WIDTH_FUDGE;
    } else {
      w = boardWidth - 2*bor + 1; /*!! +1 compensates for kludge below */
    }

    gres = XtMakeResizeRequest(messageWidget, w, h, &wr, &hr);
    if (gres != XtGeometryYes && appData.debugMode) {
      fprintf(stderr, _("%s: messageWidget geometry error %d %d %d %d %d\n"),
	      programName, gres, w, h, wr, hr);
    }

    /* !! Horrible hack to work around bug in XFree86 4.0.1 (X11R6.4.3) */
    /* The size used for the child widget in layout lags one resize behind
       its true size, so we resize a second time, 1 pixel smaller.  Yeech! */
    w--;
    gres = XtMakeResizeRequest(messageWidget, w, h, &wr, &hr);
    if (gres != XtGeometryYes && appData.debugMode) {
      fprintf(stderr, _("%s: messageWidget geometry error %d %d %d %d %d\n"),
	      programName, gres, w, h, wr, hr);
    }
    /* !! end hack */
    if(!textHeight) textHeight = hr; // [HGM] if !NLS textHeight is still undefined, and we grab it from here
    XtSetArg(args[0], XtNleft,  XtChainLeft);  // [HGM] glue ends for good run-time sizing
    XtSetArg(args[1], XtNright, XtChainRight);
    XtSetValues(messageWidget, args, 2);

    if (appData.titleInWindow) {
	i = 0;
	XtSetArg(args[i], XtNborderWidth, &bor); i++;
	XtSetArg(args[i], XtNheight, &h);  i++;
	XtGetValues(titleWidget, args, i);
	if (smallLayout) {
	    w = boardWidth - 2*bor;
	} else {
	    XtSetArg(args[0], XtNwidth, &w);
	    XtGetValues(menuBarWidget, args, 1);
	    w = boardWidth - w - sep - 2*bor - WIDTH_FUDGE;
	}

	gres = XtMakeResizeRequest(titleWidget, w, h, &wr, &hr);
	if (gres != XtGeometryYes && appData.debugMode) {
	    fprintf(stderr,
		    _("%s: titleWidget geometry error %d %d %d %d %d\n"),
		    programName, gres, w, h, wr, hr);
	}
    }
    XawFormDoLayout(formWidget, True);

    xBoardWindow = XtWindow(boardWidget);

    // [HGM] it seems the layout code ends here, but perhaps the color stuff is size independent and would
    //       not need to go into InitDrawingSizes().
#endif

    /*
     * Create X checkmark bitmap and initialize option menu checks.
     */
    ReadBitmap(&xMarkPixmap, "checkmark.bm",
	       checkmark_bits, checkmark_width, checkmark_height);
    InitMenuMarkers();

    /*
     * Create an icon.
     */
    ReadBitmap(&wIconPixmap, "icon_white.bm",
	       icon_white_bits, icon_white_width, icon_white_height);
    ReadBitmap(&bIconPixmap, "icon_black.bm",
	       icon_black_bits, icon_black_width, icon_black_height);
    iconPixmap = wIconPixmap;
    i = 0;
    XtSetArg(args[i], XtNiconPixmap, iconPixmap);  i++;
    XtSetValues(shellWidget, args, i);

    /*
     * Create a cursor for the board widget.
     */
    window_attributes.cursor = XCreateFontCursor(xDisplay, XC_hand2);
    XChangeWindowAttributes(xDisplay, xBoardWindow,
			    CWCursor, &window_attributes);

    /*
     * Inhibit shell resizing.
     */
    shellArgs[0].value = (XtArgVal) &w;
    shellArgs[1].value = (XtArgVal) &h;
    XtGetValues(shellWidget, shellArgs, 2);
    shellArgs[4].value = shellArgs[2].value = w;
    shellArgs[5].value = shellArgs[3].value = h;
    XtSetValues(shellWidget, &shellArgs[2], 4);
    marginW =  w - boardWidth; // [HGM] needed to set new shellWidget size when we resize board
    marginH =  h - boardHeight;

    CatchDeleteWindow(shellWidget, "QuitProc");

    CreateGCs(False);
    CreateGrid();
    CreateAnyPieces();

    CreatePieceMenus();

    if (appData.animate || appData.animateDragging)
      CreateAnimVars();

    XtAugmentTranslations(formWidget,
			  XtParseTranslationTable(globalTranslations));
    XtAugmentTranslations(boardWidget,
			  XtParseTranslationTable(boardTranslations));
    XtAugmentTranslations(whiteTimerWidget,
			  XtParseTranslationTable(whiteTranslations));
    XtAugmentTranslations(blackTimerWidget,
			  XtParseTranslationTable(blackTranslations));

    /* Why is the following needed on some versions of X instead
     * of a translation? */
    XtAddEventHandler(boardWidget, ExposureMask|PointerMotionMask, False,
		      (XtEventHandler) EventProc, NULL);
    /* end why */
    XtAddEventHandler(formWidget, KeyPressMask, False,
		      (XtEventHandler) MoveTypeInProc, NULL);
    XtAddEventHandler(shellWidget, StructureNotifyMask, False,
		      (XtEventHandler) EventProc, NULL);

    /* [AS] Restore layout */
    if( wpMoveHistory.visible ) {
      HistoryPopUp();
    }

    if( wpEvalGraph.visible )
      {
	EvalGraphPopUp();
      };

    if( wpEngineOutput.visible ) {
      EngineOutputPopUp();
    }

    InitBackEnd2();

    if (errorExitStatus == -1) {
	if (appData.icsActive) {
	    /* We now wait until we see "login:" from the ICS before
	       sending the logon script (problems with timestamp otherwise) */
	    /*ICSInitScript();*/
	    if (appData.icsInputBox) ICSInputBoxPopUp();
	}

    #ifdef SIGWINCH
    signal(SIGWINCH, TermSizeSigHandler);
    #endif
	signal(SIGINT, IntSigHandler);
	signal(SIGTERM, IntSigHandler);
	if (*appData.cmailGameName != NULLCHAR) {
	    signal(SIGUSR1, CmailSigHandler);
	}
    }

    gameInfo.boardWidth = 0; // [HGM] pieces: kludge to ensure InitPosition() calls InitDrawingSizes()
    InitPosition(TRUE);
//    XtSetKeyboardFocus(shellWidget, formWidget);
    XSetInputFocus(xDisplay, XtWindow(formWidget), RevertToPointerRoot, CurrentTime);

    XtAppMainLoop(appContext);
    if (appData.debugMode) fclose(debugFP); // [DM] debug
    return 0;
}

static Boolean noEcho;

void
ShutDownFrontEnd ()
{
    if (appData.icsActive && oldICSInteractionTitle != NULL) {
        DisplayIcsInteractionTitle(oldICSInteractionTitle);
    }
    if (saveSettingsOnExit) SaveSettings(settingsFileName);
    unlink(gameCopyFilename);
    unlink(gamePasteFilename);
    if(noEcho) EchoOn();
}

RETSIGTYPE
TermSizeSigHandler (int sig)
{
    update_ics_width();
}

RETSIGTYPE
IntSigHandler (int sig)
{
    ExitEvent(sig);
}

RETSIGTYPE
CmailSigHandler (int sig)
{
    int dummy = 0;
    int error;

    signal(SIGUSR1, SIG_IGN);	/* suspend handler     */

    /* Activate call-back function CmailSigHandlerCallBack()             */
    OutputToProcess(cmailPR, (char *)(&dummy), sizeof(int), &error);

    signal(SIGUSR1, CmailSigHandler); /* re-activate handler */
}

void
CmailSigHandlerCallBack (InputSourceRef isr, VOIDSTAR closure, char *message, int count, int error)
{
    BoardToTop();
    ReloadCmailMsgEvent(TRUE);	/* Reload cmail msg  */
}
/**** end signal code ****/


void
ICSInitScript ()
{
  /* try to open the icsLogon script, either in the location given
   * or in the users HOME directory
   */

  FILE *f;
  char buf[MSG_SIZ];
  char *homedir;

  f = fopen(appData.icsLogon, "r");
  if (f == NULL)
    {
      homedir = getenv("HOME");
      if (homedir != NULL)
	{
	  safeStrCpy(buf, homedir, sizeof(buf)/sizeof(buf[0]) );
	  strncat(buf, "/", MSG_SIZ - strlen(buf) - 1);
	  strncat(buf, appData.icsLogon,  MSG_SIZ - strlen(buf) - 1);
	  f = fopen(buf, "r");
	}
    }

  if (f != NULL)
    ProcessICSInitScript(f);
  else
    printf("Warning: Couldn't open icsLogon file (checked %s and %s).\n", appData.icsLogon, buf);

  return;
}

void
ResetFrontEnd ()
{
    CommentPopDown();
    TagsPopDown();
    return;
}

// [HGM] code borrowed from winboard.c (which should thus go to backend.c!)
#define HISTORY_SIZE 64
static char *history[HISTORY_SIZE];
int histIn = 0, histP = 0;

void
SaveInHistory (char *cmd)
{
  if (history[histIn] != NULL) {
    free(history[histIn]);
    history[histIn] = NULL;
  }
  if (*cmd == NULLCHAR) return;
  history[histIn] = StrSave(cmd);
  histIn = (histIn + 1) % HISTORY_SIZE;
  if (history[histIn] != NULL) {
    free(history[histIn]);
    history[histIn] = NULL;
  }
  histP = histIn;
}

char *
PrevInHistory (char *cmd)
{
  int newhp;
  if (histP == histIn) {
    if (history[histIn] != NULL) free(history[histIn]);
    history[histIn] = StrSave(cmd);
  }
  newhp = (histP - 1 + HISTORY_SIZE) % HISTORY_SIZE;
  if (newhp == histIn || history[newhp] == NULL) return NULL;
  histP = newhp;
  return history[histP];
}

char *
NextInHistory ()
{
  if (histP == histIn) return NULL;
  histP = (histP + 1) % HISTORY_SIZE;
  return history[histP];   
}
// end of borrowed code

#define Abs(n) ((n)<0 ? -(n) : (n))

#ifdef ENABLE_NLS
char *
InsertPxlSize (char *pattern, int targetPxlSize)
{
    char *base_fnt_lst, strInt[12], *p, *q;
    int alternatives, i, len, strIntLen;

    /*
     * Replace the "*" (if present) in the pixel-size slot of each
     * alternative with the targetPxlSize.
     */
    p = pattern;
    alternatives = 1;
    while ((p = strchr(p, ',')) != NULL) {
      alternatives++;
      p++;
    }
    snprintf(strInt, sizeof(strInt), "%d", targetPxlSize);
    strIntLen = strlen(strInt);
    base_fnt_lst = calloc(1, strlen(pattern) + strIntLen * alternatives + 1);

    p = pattern;
    q = base_fnt_lst;
    while (alternatives--) {
      char *comma = strchr(p, ',');
      for (i=0; i<14; i++) {
	char *hyphen = strchr(p, '-');
	if (!hyphen) break;
	if (comma && hyphen > comma) break;
	len = hyphen + 1 - p;
	if (i == 7 && *p == '*' && len == 2) {
	  p += len;
	  memcpy(q, strInt, strIntLen);
	  q += strIntLen;
	  *q++ = '-';
	} else {
	  memcpy(q, p, len);
	  p += len;
	  q += len;
	}
      }
      if (!comma) break;
      len = comma + 1 - p;
      memcpy(q, p, len);
      p += len;
      q += len;
    }
    strcpy(q, p);

    return base_fnt_lst;
}

XFontSet
CreateFontSet (char *base_fnt_lst)
{
    XFontSet fntSet;
    char **missing_list;
    int missing_count;
    char *def_string;

    fntSet = XCreateFontSet(xDisplay, base_fnt_lst,
			    &missing_list, &missing_count, &def_string);
    if (appData.debugMode) {
      int i, count;
      XFontStruct **font_struct_list;
      char **font_name_list;
      fprintf(debugFP, "Requested font set for list %s\n", base_fnt_lst);
      if (fntSet) {
	fprintf(debugFP, " got list %s, locale %s\n",
		XBaseFontNameListOfFontSet(fntSet),
		XLocaleOfFontSet(fntSet));
	count = XFontsOfFontSet(fntSet, &font_struct_list, &font_name_list);
	for (i = 0; i < count; i++) {
	  fprintf(debugFP, " got charset %s\n", font_name_list[i]);
	}
      }
      for (i = 0; i < missing_count; i++) {
	fprintf(debugFP, " missing charset %s\n", missing_list[i]);
      }
    }
    if (fntSet == NULL) {
      fprintf(stderr, _("Unable to create font set for %s.\n"), base_fnt_lst);
      exit(2);
    }
    return fntSet;
}
#else // not ENABLE_NLS
/*
 * Find a font that matches "pattern" that is as close as
 * possible to the targetPxlSize.  Prefer fonts that are k
 * pixels smaller to fonts that are k pixels larger.  The
 * pattern must be in the X Consortium standard format,
 * e.g. "-*-helvetica-bold-r-normal--*-*-*-*-*-*-*-*".
 * The return value should be freed with XtFree when no
 * longer needed.
 */
char *
FindFont (char *pattern, int targetPxlSize)
{
    char **fonts, *p, *best, *scalable, *scalableTail;
    int i, j, nfonts, minerr, err, pxlSize;

    fonts = XListFonts(xDisplay, pattern, 999999, &nfonts);
    if (nfonts < 1) {
	fprintf(stderr, _("%s: no fonts match pattern %s\n"),
		programName, pattern);
	exit(2);
    }

    best = fonts[0];
    scalable = NULL;
    minerr = 999999;
    for (i=0; i<nfonts; i++) {
	j = 0;
	p = fonts[i];
	if (*p != '-') continue;
	while (j < 7) {
	    if (*p == NULLCHAR) break;
	    if (*p++ == '-') j++;
	}
	if (j < 7) continue;
	pxlSize = atoi(p);
	if (pxlSize == 0) {
	    scalable = fonts[i];
	    scalableTail = p;
	} else {
	    err = pxlSize - targetPxlSize;
	    if (Abs(err) < Abs(minerr) ||
	        (minerr > 0 && err < 0 && -err == minerr)) {
	        best = fonts[i];
	        minerr = err;
	    }
	}
    }
    if (scalable && Abs(minerr) > appData.fontSizeTolerance) {
        /* If the error is too big and there is a scalable font,
	   use the scalable font. */
        int headlen = scalableTail - scalable;
        p = (char *) XtMalloc(strlen(scalable) + 10);
	while (isdigit(*scalableTail)) scalableTail++;
	sprintf(p, "%.*s%d%s", headlen, scalable, targetPxlSize, scalableTail);
    } else {
        p = (char *) XtMalloc(strlen(best) + 2);
        safeStrCpy(p, best, strlen(best)+1 );
    }
    if (appData.debugMode) {
        fprintf(debugFP, _("resolved %s at pixel size %d\n  to %s\n"),
		pattern, targetPxlSize, p);
    }
    XFreeFontNames(fonts);
    return p;
}
#endif

void
DeleteGCs ()
{   // [HGM] deletes GCs that are to be remade, to prevent resource leak;
    // must be called before all non-first callse to CreateGCs()
    XtReleaseGC(shellWidget, highlineGC);
    XtReleaseGC(shellWidget, lightSquareGC);
    XtReleaseGC(shellWidget, darkSquareGC);
    XtReleaseGC(shellWidget, lineGC);
    if (appData.monoMode) {
	if (DefaultDepth(xDisplay, xScreen) == 1) {
	    XtReleaseGC(shellWidget, wbPieceGC);
	} else {
	    XtReleaseGC(shellWidget, bwPieceGC);
	}
    } else {
	XtReleaseGC(shellWidget, prelineGC);
	XtReleaseGC(shellWidget, jailSquareGC);
	XtReleaseGC(shellWidget, wdPieceGC);
	XtReleaseGC(shellWidget, wlPieceGC);
	XtReleaseGC(shellWidget, wjPieceGC);
	XtReleaseGC(shellWidget, bdPieceGC);
	XtReleaseGC(shellWidget, blPieceGC);
	XtReleaseGC(shellWidget, bjPieceGC);
    }
}

static GC
CreateOneGC (XGCValues *gc_values, Pixel foreground, Pixel background)
{
    XtGCMask value_mask = GCLineWidth | GCLineStyle | GCForeground
      | GCBackground | GCFunction | GCPlaneMask;
    gc_values->foreground = foreground;
    gc_values->background = background;
    return XtGetGC(shellWidget, value_mask, gc_values);
}

static void
CreateGCs (int redo)
{
    XtGCMask value_mask = GCLineWidth | GCLineStyle | GCForeground
      | GCBackground | GCFunction | GCPlaneMask;
    XGCValues gc_values;
    GC copyInvertedGC;
    Pixel white = XWhitePixel(xDisplay, xScreen);
    Pixel black = XBlackPixel(xDisplay, xScreen);

    gc_values.plane_mask = AllPlanes;
    gc_values.line_width = lineGap;
    gc_values.line_style = LineSolid;
    gc_values.function = GXcopy;

  if(redo) {
    DeleteGCs(); // called a second time; clean up old GCs first
  } else { // [HGM] grid and font GCs created on first call only
    coordGC = CreateOneGC(&gc_values, black, white);
    XSetFont(xDisplay, coordGC, coordFontID);

    // [HGM] make font for holdings counts (white on black)
    countGC = CreateOneGC(&gc_values, white, black);
    XSetFont(xDisplay, countGC, countFontID);
  }
    lineGC = CreateOneGC(&gc_values, black, black);

    if (appData.monoMode) {

	highlineGC = CreateOneGC(&gc_values, white, white);
	lightSquareGC = wbPieceGC = CreateOneGC(&gc_values, white, black);
	darkSquareGC = bwPieceGC = CreateOneGC(&gc_values, black, white);

	if (DefaultDepth(xDisplay, xScreen) == 1) {
	    /* Avoid XCopyPlane on 1-bit screens to work around Sun bug */
	    gc_values.function = GXcopyInverted;
	    copyInvertedGC = CreateOneGC(&gc_values, black, white);
	    gc_values.function = GXcopy;
	    if (XBlackPixel(xDisplay, xScreen) == 1) {
		bwPieceGC = darkSquareGC;
		wbPieceGC = copyInvertedGC;
	    } else {
		bwPieceGC = copyInvertedGC;
		wbPieceGC = lightSquareGC;
	    }
	}
    } else {

	highlineGC = CreateOneGC(&gc_values, highlightSquareColor, highlightSquareColor);
	prelineGC = CreateOneGC(&gc_values, premoveHighlightColor, premoveHighlightColor);
	lightSquareGC = CreateOneGC(&gc_values, lightSquareColor, darkSquareColor);
	darkSquareGC = CreateOneGC(&gc_values, darkSquareColor, lightSquareColor);
	jailSquareGC = CreateOneGC(&gc_values, jailSquareColor, jailSquareColor);
	wdPieceGC = CreateOneGC(&gc_values, whitePieceColor, darkSquareColor);
	wlPieceGC = CreateOneGC(&gc_values, whitePieceColor, lightSquareColor);
	wjPieceGC = CreateOneGC(&gc_values, whitePieceColor, jailSquareColor);
	bdPieceGC = CreateOneGC(&gc_values, blackPieceColor, darkSquareColor);
	blPieceGC = CreateOneGC(&gc_values, blackPieceColor, lightSquareColor);
	bjPieceGC = CreateOneGC(&gc_values, blackPieceColor, jailSquareColor);
    }
}

void
loadXIM (XImage *xim, XImage *xmask, char *filename, Pixmap *dest, Pixmap *mask)
{
    int x, y, w, h, p;
    FILE *fp;
    Pixmap temp;
    XGCValues	values;
    GC maskGC;

    fp = fopen(filename, "rb");
    if (!fp) {
	fprintf(stderr, _("%s: error loading XIM!\n"), programName);
	exit(1);
    }

    w = fgetc(fp);
    h = fgetc(fp);

    for (y=0; y<h; ++y) {
	for (x=0; x<h; ++x) {
	    p = fgetc(fp);

	    switch (p) {
	      case 0:
		XPutPixel(xim, x, y, blackPieceColor);
		if (xmask)
		  XPutPixel(xmask, x, y, WhitePixel(xDisplay,xScreen));
		break;
	      case 1:
		XPutPixel(xim, x, y, darkSquareColor);
		if (xmask)
		  XPutPixel(xmask, x, y, BlackPixel(xDisplay,xScreen));
		break;
	      case 2:
		XPutPixel(xim, x, y, whitePieceColor);
		if (xmask)
		  XPutPixel(xmask, x, y, WhitePixel(xDisplay,xScreen));
		break;
	      case 3:
		XPutPixel(xim, x, y, lightSquareColor);
		if (xmask)
		  XPutPixel(xmask, x, y, BlackPixel(xDisplay,xScreen));
		break;
	    }
	}
    }

    fclose(fp);

    /* create Pixmap of piece */
    *dest = XCreatePixmap(xDisplay, DefaultRootWindow(xDisplay),
			  w, h, xim->depth);
    XPutImage(xDisplay, *dest, lightSquareGC, xim,
	      0, 0, 0, 0, w, h);

    /* create Pixmap of clipmask
       Note: We assume the white/black pieces have the same
             outline, so we make only 6 masks. This is okay
             since the XPM clipmask routines do the same. */
    if (xmask) {
      temp = XCreatePixmap(xDisplay, DefaultRootWindow(xDisplay),
			    w, h, xim->depth);
      XPutImage(xDisplay, temp, lightSquareGC, xmask,
	      0, 0, 0, 0, w, h);

      /* now create the 1-bit version */
      *mask = XCreatePixmap(xDisplay, DefaultRootWindow(xDisplay),
			  w, h, 1);

      values.foreground = 1;
      values.background = 0;

      /* Don't use XtGetGC, not read only */
      maskGC = XCreateGC(xDisplay, *mask,
		    GCForeground | GCBackground, &values);
      XCopyPlane(xDisplay, temp, *mask, maskGC,
		  0, 0, squareSize, squareSize, 0, 0, 1);
      XFreePixmap(xDisplay, temp);
    }
}


char pieceBitmapNames[] = "pnbrqfeacwmohijgdvlsukpnsl";

void
CreateXIMPieces ()
{
    int piece, kind;
    char buf[MSG_SIZ];
    u_int ss;
    static char *ximkind[] = { "ll", "ld", "dl", "dd" };
    XImage *ximtemp;

    ss = squareSize;

    /* The XSynchronize calls were copied from CreatePieces.
       Not sure if needed, but can't hurt */
    XSynchronize(xDisplay, True); /* Work-around for xlib/xt
				     buffering bug */

    /* temp needed by loadXIM() */
    ximtemp = XGetImage(xDisplay, DefaultRootWindow(xDisplay),
		 0, 0, ss, ss, AllPlanes, XYPixmap);

    if (strlen(appData.pixmapDirectory) == 0) {
      useImages = 0;
    } else {
	useImages = 1;
	if (appData.monoMode) {
	  DisplayFatalError(_("XIM pieces cannot be used in monochrome mode"),
			    0, 2);
	  ExitEvent(2);
	}
	fprintf(stderr, _("\nLoading XIMs...\n"));
	/* Load pieces */
	for (piece = (int) WhitePawn; piece <= (int) WhiteKing + 4; piece++) {
	    fprintf(stderr, "%d", piece+1);
	    for (kind=0; kind<4; kind++) {
		fprintf(stderr, ".");
		snprintf(buf, sizeof(buf), "%s/%s%c%s%u.xim",
			ExpandPathName(appData.pixmapDirectory),
			piece <= (int) WhiteKing ? "" : "w",
			pieceBitmapNames[piece],
			ximkind[kind], ss);
		ximPieceBitmap[kind][piece] =
		  XGetImage(xDisplay, DefaultRootWindow(xDisplay),
			    0, 0, ss, ss, AllPlanes, XYPixmap);
		if (appData.debugMode)
		  fprintf(stderr, _("(File:%s:) "), buf);
		loadXIM(ximPieceBitmap[kind][piece],
			ximtemp, buf,
			&(xpmPieceBitmap2[kind][piece]),
			&(ximMaskPm2[piece]));
		if(piece <= (int)WhiteKing)
		    xpmPieceBitmap[kind][piece] = xpmPieceBitmap2[kind][piece];
	    }
	    fprintf(stderr," ");
	}
	/* Load light and dark squares */
	/* If the LSQ and DSQ pieces don't exist, we will
	   draw them with solid squares. */
	snprintf(buf,sizeof(buf), "%s/lsq%u.xim", ExpandPathName(appData.pixmapDirectory), ss);
	if (access(buf, 0) != 0) {
	    useImageSqs = 0;
	} else {
	    useImageSqs = 1;
	    fprintf(stderr, _("light square "));
	    ximLightSquare=
	      XGetImage(xDisplay, DefaultRootWindow(xDisplay),
			0, 0, ss, ss, AllPlanes, XYPixmap);
	    if (appData.debugMode)
	      fprintf(stderr, _("(File:%s:) "), buf);

	    loadXIM(ximLightSquare, NULL, buf, &xpmLightSquare, NULL);
	    fprintf(stderr, _("dark square "));
	    snprintf(buf,sizeof(buf), "%s/dsq%u.xim",
		    ExpandPathName(appData.pixmapDirectory), ss);
	    if (appData.debugMode)
	      fprintf(stderr, _("(File:%s:) "), buf);
	    ximDarkSquare=
	      XGetImage(xDisplay, DefaultRootWindow(xDisplay),
			0, 0, ss, ss, AllPlanes, XYPixmap);
	    loadXIM(ximDarkSquare, NULL, buf, &xpmDarkSquare, NULL);
	    xpmJailSquare = xpmLightSquare;
	}
	fprintf(stderr, _("Done.\n"));
    }
    XSynchronize(xDisplay, False); /* Work-around for xlib/xt buffering bug */
}

static VariantClass oldVariant = (VariantClass) -1; // [HGM] pieces: redo every time variant changes

#if HAVE_LIBXPM
void
CreateXPMBoard (char *s, int kind)
{
    XpmAttributes attr;
    attr.valuemask = 0;
    if(!appData.useBitmaps || s == NULL || *s == 0 || *s == '*') { useTexture &= ~(kind+1); return; }
    if (XpmReadFileToPixmap(xDisplay, xBoardWindow, s, &(xpmBoardBitmap[kind]), NULL, &attr) == 0) {
	useTexture |= kind + 1; textureW[kind] = attr.width; textureH[kind] = attr.height;
    }
}

void
FreeXPMPieces ()
{   // [HGM] to prevent resoucre leak on calling CreaeXPMPieces() a second time,
    // thisroutine has to be called t free the old piece pixmaps
    int piece, kind;
    for (piece = (int) WhitePawn; piece <= (int) WhiteKing + 4; piece++)
	for (kind=0; kind<4; kind++) XFreePixmap(xDisplay, xpmPieceBitmap2[kind][piece]);
    if(useImageSqs) {
	XFreePixmap(xDisplay, xpmLightSquare);
	XFreePixmap(xDisplay, xpmDarkSquare);
    }
}

void
CreateXPMPieces ()
{
    int piece, kind, r;
    char buf[MSG_SIZ];
    u_int ss = squareSize;
    XpmAttributes attr;
    static char *xpmkind[] = { "ll", "ld", "dl", "dd" };
    XpmColorSymbol symbols[4];
    static int redo = False;

    if(redo) FreeXPMPieces(); else redo = 1;

    /* The XSynchronize calls were copied from CreatePieces.
       Not sure if needed, but can't hurt */
    XSynchronize(xDisplay, True); /* Work-around for xlib/xt buffering bug */

    /* Setup translations so piece colors match square colors */
    symbols[0].name = "light_piece";
    symbols[0].value = appData.whitePieceColor;
    symbols[1].name = "dark_piece";
    symbols[1].value = appData.blackPieceColor;
    symbols[2].name = "light_square";
    symbols[2].value = appData.lightSquareColor;
    symbols[3].name = "dark_square";
    symbols[3].value = appData.darkSquareColor;

    attr.valuemask = XpmColorSymbols;
    attr.colorsymbols = symbols;
    attr.numsymbols = 4;

    if (appData.monoMode) {
      DisplayFatalError(_("XPM pieces cannot be used in monochrome mode"),
			0, 2);
      ExitEvent(2);
    }
    if (strlen(appData.pixmapDirectory) == 0) {
	XpmPieces* pieces = builtInXpms;
	useImages = 1;
	/* Load pieces */
	while (pieces->size != squareSize && pieces->size) pieces++;
	if (!pieces->size) {
	  fprintf(stderr, _("No builtin XPM pieces of size %d\n"), squareSize);
	  exit(1);
	}
	for (piece = (int) WhitePawn; piece <= (int) WhiteKing + 4; piece++) {
	    for (kind=0; kind<4; kind++) {

		if ((r=XpmCreatePixmapFromData(xDisplay, xBoardWindow,
					       pieces->xpm[piece][kind],
					       &(xpmPieceBitmap2[kind][piece]),
					       NULL, &attr)) != 0) {
		  fprintf(stderr, _("Error %d loading XPM image \"%s\"\n"),
			  r, buf);
		  exit(1);
		}
		if(piece <= (int) WhiteKing)
		    xpmPieceBitmap[kind][piece] = xpmPieceBitmap2[kind][piece];
	    }
	}
	useImageSqs = 0;
	xpmJailSquare = xpmLightSquare;
    } else {
	useImages = 1;

	fprintf(stderr, _("\nLoading XPMs...\n"));

	/* Load pieces */
	for (piece = (int) WhitePawn; piece <= (int) WhiteKing + 4; piece++) {
	    fprintf(stderr, "%d ", piece+1);
	    for (kind=0; kind<4; kind++) {
	      snprintf(buf, sizeof(buf), "%s/%s%c%s%u.xpm",
			ExpandPathName(appData.pixmapDirectory),
			piece > (int) WhiteKing ? "w" : "",
			pieceBitmapNames[piece],
			xpmkind[kind], ss);
		if (appData.debugMode) {
		    fprintf(stderr, _("(File:%s:) "), buf);
		}
		if ((r=XpmReadFileToPixmap(xDisplay, xBoardWindow, buf,
					   &(xpmPieceBitmap2[kind][piece]),
					   NULL, &attr)) != 0) {
		    if(piece != (int)WhiteKing && piece > (int)WhiteQueen) {
		      // [HGM] missing: read of unorthodox piece failed; substitute King.
		      snprintf(buf, sizeof(buf), "%s/k%s%u.xpm",
				ExpandPathName(appData.pixmapDirectory),
				xpmkind[kind], ss);
			if (appData.debugMode) {
			    fprintf(stderr, _("(Replace by File:%s:) "), buf);
			}
			r=XpmReadFileToPixmap(xDisplay, xBoardWindow, buf,
						&(xpmPieceBitmap2[kind][piece]),
						NULL, &attr);
		    }
		    if (r != 0) {
			fprintf(stderr, _("Error %d loading XPM file \"%s\"\n"),
				r, buf);
			exit(1);
		    }
		}
		if(piece <= (int) WhiteKing)
		    xpmPieceBitmap[kind][piece] = xpmPieceBitmap2[kind][piece];
	    }
	}
	/* Load light and dark squares */
	/* If the LSQ and DSQ pieces don't exist, we will
	   draw them with solid squares. */
	fprintf(stderr, _("light square "));
	snprintf(buf, sizeof(buf), "%s/lsq%u.xpm", ExpandPathName(appData.pixmapDirectory), ss);
	if (access(buf, 0) != 0) {
	    useImageSqs = 0;
	} else {
	    useImageSqs = 1;
	    if (appData.debugMode)
	      fprintf(stderr, _("(File:%s:) "), buf);

	    if ((r=XpmReadFileToPixmap(xDisplay, xBoardWindow, buf,
				       &xpmLightSquare, NULL, &attr)) != 0) {
		fprintf(stderr, _("Error %d loading XPM file \"%s\"\n"), r, buf);
		exit(1);
	    }
	    fprintf(stderr, _("dark square "));
	    snprintf(buf, sizeof(buf), "%s/dsq%u.xpm",
		    ExpandPathName(appData.pixmapDirectory), ss);
	    if (appData.debugMode) {
		fprintf(stderr, _("(File:%s:) "), buf);
	    }
	    if ((r=XpmReadFileToPixmap(xDisplay, xBoardWindow, buf,
				       &xpmDarkSquare, NULL, &attr)) != 0) {
		fprintf(stderr, _("Error %d loading XPM file \"%s\"\n"), r, buf);
		exit(1);
	    }
	}
	xpmJailSquare = xpmLightSquare;
	fprintf(stderr, _("Done.\n"));
    }
    oldVariant = -1; // kludge to force re-makig of animation masks
    XSynchronize(xDisplay, False); /* Work-around for xlib/xt
				      buffering bug */
}
#endif /* HAVE_LIBXPM */

#if HAVE_LIBXPM
/* No built-in bitmaps */
void CreatePieces()
{
    int piece, kind;
    char buf[MSG_SIZ];
    u_int ss = squareSize;

    XSynchronize(xDisplay, True); /* Work-around for xlib/xt
				     buffering bug */

    for (kind = SOLID; kind <= (appData.monoMode ? OUTLINE : SOLID); kind++) {
	for (piece = (int) WhitePawn; piece <= (int) WhiteKing + 4; piece++) {
	  snprintf(buf, MSG_SIZ, "%s%c%u%c.bm", piece > (int)WhiteKing ? "w" : "",
		   pieceBitmapNames[piece],
		   ss, kind == SOLID ? 's' : 'o');
	  ReadBitmap(&pieceBitmap2[kind][piece], buf, NULL, ss, ss);
	  if(piece <= (int)WhiteKing)
	    pieceBitmap[kind][piece] = pieceBitmap2[kind][piece];
	}
    }

    XSynchronize(xDisplay, False); /* Work-around for xlib/xt
				      buffering bug */
}
#else
/* With built-in bitmaps */
void
CreatePieces ()
{
    BuiltInBits* bib = builtInBits;
    int piece, kind;
    char buf[MSG_SIZ];
    u_int ss = squareSize;

    XSynchronize(xDisplay, True); /* Work-around for xlib/xt
				     buffering bug */

    while (bib->squareSize != ss && bib->squareSize != 0) bib++;

    for (kind = SOLID; kind <= (appData.monoMode ? OUTLINE : SOLID); kind++) {
	for (piece = (int) WhitePawn; piece <= (int) WhiteKing + 4; piece++) {
	  snprintf(buf, MSG_SIZ, "%s%c%u%c.bm", piece > (int)WhiteKing ? "w" : "",
		   pieceBitmapNames[piece],
		   ss, kind == SOLID ? 's' : 'o');
	  ReadBitmap(&pieceBitmap2[kind][piece], buf,
		     bib->bits[kind][piece], ss, ss);
	  if(piece <= (int)WhiteKing)
	    pieceBitmap[kind][piece] = pieceBitmap2[kind][piece];
	}
    }

    XSynchronize(xDisplay, False); /* Work-around for xlib/xt
				      buffering bug */
}
#endif

void
ReadBitmap (Pixmap *pm, String name, unsigned char bits[], u_int wreq, u_int hreq)
{
    int x_hot, y_hot;
    u_int w, h;
    int errcode;
    char msg[MSG_SIZ], fullname[MSG_SIZ];

    if (*appData.bitmapDirectory != NULLCHAR) {
      safeStrCpy(fullname, appData.bitmapDirectory, sizeof(fullname)/sizeof(fullname[0]) );
      strncat(fullname, "/", MSG_SIZ - strlen(fullname) - 1);
      strncat(fullname, name, MSG_SIZ - strlen(fullname) - 1);
      errcode = XReadBitmapFile(xDisplay, xBoardWindow, fullname,
				&w, &h, pm, &x_hot, &y_hot);
      fprintf(stderr, "load %s\n", name);
	if (errcode != BitmapSuccess) {
	    switch (errcode) {
	      case BitmapOpenFailed:
		snprintf(msg, sizeof(msg), _("Can't open bitmap file %s"), fullname);
		break;
	      case BitmapFileInvalid:
		snprintf(msg, sizeof(msg), _("Invalid bitmap in file %s"), fullname);
		break;
	      case BitmapNoMemory:
		snprintf(msg, sizeof(msg), _("Ran out of memory reading bitmap file %s"),
			fullname);
		break;
	      default:
		snprintf(msg, sizeof(msg), _("Unknown XReadBitmapFile error %d on file %s"),
			errcode, fullname);
		break;
	    }
	    fprintf(stderr, _("%s: %s...using built-in\n"),
		    programName, msg);
	} else if (w != wreq || h != hreq) {
	    fprintf(stderr,
		    _("%s: Bitmap %s is %dx%d, not %dx%d...using built-in\n"),
		    programName, fullname, w, h, wreq, hreq);
	} else {
	    return;
	}
    }
    if (bits != NULL) {
	*pm = XCreateBitmapFromData(xDisplay, xBoardWindow, (char *) bits,
				    wreq, hreq);
    }
}

void
CreateGrid ()
{
    int i, j;

    if (lineGap == 0) return;

    /* [HR] Split this into 2 loops for non-square boards. */

    for (i = 0; i < BOARD_HEIGHT + 1; i++) {
        gridSegments[i].x1 = 0;
        gridSegments[i].x2 =
          lineGap + BOARD_WIDTH * (squareSize + lineGap);
        gridSegments[i].y1 = gridSegments[i].y2
          = lineGap / 2 + (i * (squareSize + lineGap));
    }

    for (j = 0; j < BOARD_WIDTH + 1; j++) {
        gridSegments[j + i].y1 = 0;
        gridSegments[j + i].y2 =
          lineGap + BOARD_HEIGHT * (squareSize + lineGap);
        gridSegments[j + i].x1 = gridSegments[j + i].x2
          = lineGap / 2 + (j * (squareSize + lineGap));
    }
}

int nrOfMenuItems = 7;
Widget menuWidget[150];
MenuListItem menuItemList[150] = {
    { "LoadNextGameProc", LoadNextGameProc },
    { "LoadPrevGameProc", LoadPrevGameProc },
    { "ReloadGameProc", ReloadGameProc },
    { "ReloadPositionProc", ReloadPositionProc },
#ifndef OPTIONSDIALOG
    { "AlwaysQueenProc", AlwaysQueenProc },
    { "AnimateDraggingProc", AnimateDraggingProc },
    { "AnimateMovingProc", AnimateMovingProc },
    { "AutoflagProc", AutoflagProc },
    { "AutoflipProc", AutoflipProc },
    { "BlindfoldProc", BlindfoldProc },
    { "FlashMovesProc", FlashMovesProc },
#if HIGHDRAG
    { "HighlightDraggingProc", HighlightDraggingProc },
#endif
    { "HighlightLastMoveProc", HighlightLastMoveProc },
//    { "IcsAlarmProc", IcsAlarmProc },
    { "MoveSoundProc", MoveSoundProc },
    { "PeriodicUpdatesProc", PeriodicUpdatesProc },
    { "PopupExitMessageProc", PopupExitMessageProc },
    { "PopupMoveErrorsProc", PopupMoveErrorsProc },
//    { "PremoveProc", PremoveProc },
    { "ShowCoordsProc", ShowCoordsProc },
    { "ShowThinkingProc", ShowThinkingProc },
    { "HideThinkingProc", HideThinkingProc },
    { "TestLegalityProc", TestLegalityProc },
#endif
    { "AboutGameProc", AboutGameEvent },
    { "DebugProc", DebugProc },
    { "NothingProc", NothingProc },
  {NULL, NothingProc}
};

void
MarkMenuItem (char *menuRef, int state)
{
    int nr = MenuToNumber(menuRef);
    if(nr >= 0) {
	Arg args[2];
	XtSetArg(args[0], XtNleftBitmap, state ? xMarkPixmap : None);
	XtSetValues(menuWidget[nr], args, 1);
    }
}

void
EnableMenuItem (char *menuRef, int state)
{
    int nr = MenuToNumber(menuRef);
    if(nr >= 0) XtSetSensitive(menuWidget[nr], state);
}

void
EnableButtonBar (int state)
{
    XtSetSensitive(buttonBarWidget, state);
}


void
SetMenuEnables (Enables *enab)
{
  while (enab->name != NULL) {
    EnableMenuItem(enab->name, enab->value);
    enab++;
  }
}

int
Equal(char *p, char *s)
{   // compare strings skipping spaces in second
    while(*s) {
	if(*s == ' ') { s++; continue; }
	if(*s++ != *p++) return 0;
    }
    return !*p;
}

void
KeyBindingProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // [HGM] new method of key binding: specify MenuItem(FlipView) in stead of FlipViewProc in translation string
    int i;
    if(*nprms == 0) return;
    for(i=0; menuItemList[i].name; i++) {
	if(Equal(prms[0], menuItemList[i].name)) {
	    (menuItemList[i].proc) ();
	    return;
	}
    }
}

static void
MenuBarSelect (Widget w, caddr_t addr, caddr_t index)
{
    MenuProc *proc = (MenuProc *) addr;

    (proc)();
}

static void
MenuEngineSelect (Widget w, caddr_t addr, caddr_t index)
{
    RecentEngineEvent((int) (intptr_t) addr);
}

// some stuff that must remain in front-end
static Widget mainBar, currentMenu;
static int wtot, nr = 0, widths[10];

void
AppendMenuItem (char *text, char *name, MenuProc *action)
{
    int j;
    Widget entry;
    Arg args[16];

    j = 0;
    XtSetArg(args[j], XtNleftMargin, 20);   j++;
    XtSetArg(args[j], XtNrightMargin, 20);  j++;

	if (strcmp(text, "----") == 0) {
	  entry = XtCreateManagedWidget(text, smeLineObjectClass,
					  currentMenu, args, j);
	} else {
          XtSetArg(args[j], XtNlabel, XtNewString(_(text)));
	    entry = XtCreateManagedWidget(name, smeBSBObjectClass,
					  currentMenu, args, j+1);
	    XtAddCallback(entry, XtNcallback,
			  (XtCallbackProc) (strcmp(name, "recent") ? MenuBarSelect : MenuEngineSelect),
			  (caddr_t) action);
	    menuWidget[nrOfMenuItems] = entry;
	}
}

void
CreateMenuButton (char *name, Menu *mb)
{   // create menu button on main bar, and shell for pull-down list
    int i, j;
    Arg args[16];
    Dimension w;

	j = 0;
	XtSetArg(args[j], XtNmenuName, XtNewString(name));  j++;
	XtSetArg(args[j], XtNlabel, XtNewString(_(mb->name)));  j++;
	XtSetArg(args[j], XtNborderWidth, 0);                   j++;
	mb->subMenu = XtCreateManagedWidget(mb->name, menuButtonWidgetClass,
				       mainBar, args, j);
    currentMenu = XtCreatePopupShell(name, simpleMenuWidgetClass,
			      mainBar, NULL, 0);
	j = 0;
	XtSetArg(args[j], XtNwidth, &w);                   j++;
	XtGetValues(mb->subMenu, args, j);
	wtot += mb->textWidth = widths[nr++] = w;
}

Widget
CreateMenuBar (Menu *mb, int boardWidth)
{
    int i, j;
    Arg args[16];
    char menuName[MSG_SIZ];
    Dimension w;
    Menu *ma = mb;

    // create bar itself
    j = 0;
    XtSetArg(args[j], XtNorientation, XtorientHorizontal);  j++;
    XtSetArg(args[j], XtNvSpace, 0);                        j++;
    XtSetArg(args[j], XtNborderWidth, 0);                   j++;
    mainBar = XtCreateWidget("menuBar", boxWidgetClass,
			     formWidget, args, j);

    CreateMainMenus(mb); // put menus in bar according to description in back-end

    // size buttons to make menu bar fit, clipping menu names where necessary
    while(wtot > boardWidth - 40) {
	int wmax=0, imax=0;
	for(i=0; i<nr; i++) if(widths[i] > wmax) wmax = widths[imax=i];
	widths[imax]--;
	wtot--;
    }
    for(i=0; i<nr; i++) if(widths[i] != ma[i].textWidth) {
	j = 0;
	XtSetArg(args[j], XtNwidth, widths[i]);                   j++;
	XtSetValues(ma[i].subMenu, args, j);
    }

    return mainBar;
}

Widget
CreateButtonBar (MenuItem *mi)
{
    int j;
    Widget button, buttonBar;
    Arg args[16];

    j = 0;
    XtSetArg(args[j], XtNorientation, XtorientHorizontal); j++;
    if (tinyLayout) {
	XtSetArg(args[j], XtNhSpace, 0); j++;
    }
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNvSpace, 0);                        j++;
    buttonBar = XtCreateWidget("buttonBar", boxWidgetClass,
			       formWidget, args, j);

    while (mi->string != NULL) {
	j = 0;
	if (tinyLayout) {
	    XtSetArg(args[j], XtNinternalWidth, 2); j++;
	    XtSetArg(args[j], XtNborderWidth, 0); j++;
	}
      XtSetArg(args[j], XtNlabel, XtNewString(_(mi->string))); j++;
	button = XtCreateManagedWidget(mi->string, commandWidgetClass,
				       buttonBar, args, j);
	XtAddCallback(button, XtNcallback,
		      (XtCallbackProc) MenuBarSelect,
		      (caddr_t) mi->proc);
	mi++;
    }
    return buttonBar;
}

Widget
CreatePieceMenu (char *name, int color)
{
    int i;
    Widget entry, menu;
    Arg args[16];
    ChessSquare selection;

    menu = XtCreatePopupShell(name, simpleMenuWidgetClass,
			      boardWidget, args, 0);

    for (i = 0; i < PIECE_MENU_SIZE; i++) {
	String item = pieceMenuStrings[color][i];

	if (strcmp(item, "----") == 0) {
	    entry = XtCreateManagedWidget(item, smeLineObjectClass,
					  menu, NULL, 0);
	} else {
          XtSetArg(args[0], XtNlabel, XtNewString(_(item)));
	    entry = XtCreateManagedWidget(item, smeBSBObjectClass,
                                menu, args, 1);
	    selection = pieceMenuTranslation[color][i];
	    XtAddCallback(entry, XtNcallback,
			  (XtCallbackProc) PieceMenuSelect,
			  (caddr_t) selection);
	    if (selection == WhitePawn || selection == BlackPawn) {
		XtSetArg(args[0], XtNpopupOnEntry, entry);
		XtSetValues(menu, args, 1);
	    }
	}
    }
    return menu;
}

void
CreatePieceMenus ()
{
    int i;
    Widget entry;
    Arg args[16];
    ChessSquare selection;

    whitePieceMenu = CreatePieceMenu("menuW", 0);
    blackPieceMenu = CreatePieceMenu("menuB", 1);

    if(appData.pieceMenu) // [HGM] sweep: no idea what this was good for, but it stopped reporting button events outside the window
    XtRegisterGrabAction(PieceMenuPopup, True,
			 (unsigned)(ButtonPressMask|ButtonReleaseMask),
			 GrabModeAsync, GrabModeAsync);

    XtSetArg(args[0], XtNlabel, _("Drop"));
    dropMenu = XtCreatePopupShell("menuD", simpleMenuWidgetClass,
				  boardWidget, args, 1);
    for (i = 0; i < DROP_MENU_SIZE; i++) {
	String item = dropMenuStrings[i];

	if (strcmp(item, "----") == 0) {
	    entry = XtCreateManagedWidget(item, smeLineObjectClass,
					  dropMenu, NULL, 0);
	} else {
          XtSetArg(args[0], XtNlabel, XtNewString(_(item)));
	    entry = XtCreateManagedWidget(item, smeBSBObjectClass,
                                dropMenu, args, 1);
	    selection = dropMenuTranslation[i];
	    XtAddCallback(entry, XtNcallback,
			  (XtCallbackProc) DropMenuSelect,
			  (caddr_t) selection);
	}
    }
}

void
SetupDropMenu ()
{
    int i, j, count;
    char label[32];
    Arg args[16];
    Widget entry;
    char* p;

    for (i=0; i<sizeof(dmEnables)/sizeof(DropMenuEnables); i++) {
	entry = XtNameToWidget(dropMenu, dmEnables[i].widget);
	p = strchr(gameMode == IcsPlayingWhite ? white_holding : black_holding,
		   dmEnables[i].piece);
	XtSetSensitive(entry, p != NULL || !appData.testLegality
		       /*!!temp:*/ || (gameInfo.variant == VariantCrazyhouse
				       && !appData.icsActive));
	count = 0;
	while (p && *p++ == dmEnables[i].piece) count++;
	snprintf(label, sizeof(label), "%s  %d", dmEnables[i].widget, count);
	j = 0;
	XtSetArg(args[j], XtNlabel, label); j++;
	XtSetValues(entry, args, j);
    }
}

void
PieceMenuPopup (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    String whichMenu; int menuNr = -2;
    shiftKey = strcmp(params[0], "menuW"); // used to indicate black
    if (event->type == ButtonRelease)
        menuNr = RightClick(Release, event->xbutton.x, event->xbutton.y, &pmFromX, &pmFromY);
    else if (event->type == ButtonPress)
        menuNr = RightClick(Press,   event->xbutton.x, event->xbutton.y, &pmFromX, &pmFromY);
    switch(menuNr) {
      case 0: whichMenu = params[0]; break;
      case 1: SetupDropMenu(); whichMenu = "menuD"; break;
      case 2:
      case -1: if (errorUp) ErrorPopDown();
      default: return;
    }
    XtPopupSpringLoaded(XtNameToWidget(boardWidget, whichMenu));
}

static void
PieceMenuSelect (Widget w, ChessSquare piece, caddr_t junk)
{
    if (pmFromX < 0 || pmFromY < 0) return;
    EditPositionMenuEvent(piece, pmFromX, pmFromY);
}

static void
DropMenuSelect (Widget w, ChessSquare piece, caddr_t junk)
{
    if (pmFromX < 0 || pmFromY < 0) return;
    DropMenuEvent(piece, pmFromX, pmFromY);
}

void
WhiteClock (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    shiftKey = prms[0][0] & 1;
    ClockClick(0);
}

void
BlackClock (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    shiftKey = prms[0][0] & 1;
    ClockClick(1);
}


static void
do_flash_delay (unsigned long msec)
{
    TimeDelay(msec);
}

void
DrawBorder (int x, int y, int type)
{
    GC gc = lineGC;

    if(type == 1) gc = highlineGC; else if(type == 2) gc = prelineGC;

    XDrawRectangle(xDisplay, xBoardWindow, gc, x, y,
		   squareSize+lineGap, squareSize+lineGap);
}

static int
CutOutSquare (int x, int y, int *x0, int *y0, int  kind)
{
    int W = BOARD_WIDTH, H = BOARD_HEIGHT;
    int nx = x/(squareSize + lineGap), ny = y/(squareSize + lineGap);
    *x0 = 0; *y0 = 0;
    if(textureW[kind] < squareSize || textureH[kind] < squareSize) return 0;
    if(textureW[kind] < W*squareSize)
	*x0 = (textureW[kind] - squareSize) * nx/(W-1);
    else
	*x0 = textureW[kind]*nx / W + (textureW[kind] - W*squareSize) / (2*W);
    if(textureH[kind] < H*squareSize)
	*y0 = (textureH[kind] - squareSize) * ny/(H-1);
    else
	*y0 = textureH[kind]*ny / H + (textureH[kind] - H*squareSize) / (2*H);
    return 1;
}

static void
BlankSquare (int x, int y, int color, ChessSquare piece, Drawable dest, int fac)
{   // [HGM] extra param 'fac' for forcing destination to (0,0) for copying to animation buffer
    int x0, y0;
    if (useImages && color != 2 && (useTexture & color+1) && CutOutSquare(x, y, &x0, &y0, color)) {
	XCopyArea(xDisplay, xpmBoardBitmap[color], dest, wlPieceGC, x0, y0,
		  squareSize, squareSize, x*fac, y*fac);
    } else
    if (useImages && useImageSqs) {
	Pixmap pm;
	switch (color) {
	  case 1: /* light */
	    pm = xpmLightSquare;
	    break;
	  case 0: /* dark */
	    pm = xpmDarkSquare;
	    break;
	  case 2: /* neutral */
	  default:
	    pm = xpmJailSquare;
	    break;
	}
	XCopyArea(xDisplay, pm, dest, wlPieceGC, 0, 0,
		  squareSize, squareSize, x*fac, y*fac);
    } else {
	GC gc;
	switch (color) {
	  case 1: /* light */
	    gc = lightSquareGC;
	    break;
	  case 0: /* dark */
	    gc = darkSquareGC;
	    break;
	  case 2: /* neutral */
	  default:
	    gc = jailSquareGC;
	    break;
	}
	XFillRectangle(xDisplay, dest, gc, x*fac, y*fac, squareSize, squareSize);
    }
}

/*
   I split out the routines to draw a piece so that I could
   make a generic flash routine.
*/
static void
monoDrawPiece_1bit (ChessSquare piece, int square_color, int x, int y, Drawable dest)
{
    /* Avoid XCopyPlane on 1-bit screens to work around Sun bug */
    switch (square_color) {
      case 1: /* light */
      case 2: /* neutral */
      default:
	XCopyArea(xDisplay, (int) piece < (int) BlackPawn
		  ? *pieceToOutline(piece)
		  : *pieceToSolid(piece),
		  dest, bwPieceGC, 0, 0,
		  squareSize, squareSize, x, y);
	break;
      case 0: /* dark */
	XCopyArea(xDisplay, (int) piece < (int) BlackPawn
		  ? *pieceToSolid(piece)
		  : *pieceToOutline(piece),
		  dest, wbPieceGC, 0, 0,
		  squareSize, squareSize, x, y);
	break;
    }
}

static void
monoDrawPiece (ChessSquare piece, int square_color, int x, int y, Drawable dest)
{
    switch (square_color) {
      case 1: /* light */
      case 2: /* neutral */
      default:
	XCopyPlane(xDisplay, (int) piece < (int) BlackPawn
		   ? *pieceToOutline(piece)
		   : *pieceToSolid(piece),
		   dest, bwPieceGC, 0, 0,
		   squareSize, squareSize, x, y, 1);
	break;
      case 0: /* dark */
	XCopyPlane(xDisplay, (int) piece < (int) BlackPawn
		   ? *pieceToSolid(piece)
		   : *pieceToOutline(piece),
		   dest, wbPieceGC, 0, 0,
		   squareSize, squareSize, x, y, 1);
	break;
    }
}

static void
colorDrawPiece (ChessSquare piece, int square_color, int x, int y, Drawable dest)
{
    if(pieceToSolid(piece) == NULL) return; // [HGM] bitmaps: make it non-fatal if we have no bitmap;
    switch (square_color) {
      case 1: /* light */
	XCopyPlane(xDisplay, *pieceToSolid(piece),
		   dest, (int) piece < (int) BlackPawn
		   ? wlPieceGC : blPieceGC, 0, 0,
		   squareSize, squareSize, x, y, 1);
	break;
      case 0: /* dark */
	XCopyPlane(xDisplay, *pieceToSolid(piece),
		   dest, (int) piece < (int) BlackPawn
		   ? wdPieceGC : bdPieceGC, 0, 0,
		   squareSize, squareSize, x, y, 1);
	break;
      case 2: /* neutral */
      default:
	XCopyPlane(xDisplay, *pieceToSolid(piece),
		   dest, (int) piece < (int) BlackPawn
		   ? wjPieceGC : bjPieceGC, 0, 0,
		   squareSize, squareSize, x, y, 1);
	break;
    }
}

static void
colorDrawPieceImage (ChessSquare piece, int square_color, int x, int y, Drawable dest)
{
    int kind, p = piece;

    switch (square_color) {
      case 1: /* light */
      case 2: /* neutral */
      default:
	if ((int)piece < (int) BlackPawn) {
	    kind = 0;
	} else {
	    kind = 2;
	    piece -= BlackPawn;
	}
	break;
      case 0: /* dark */
	if ((int)piece < (int) BlackPawn) {
	    kind = 1;
	} else {
	    kind = 3;
	    piece -= BlackPawn;
	}
	break;
    }
    if(appData.upsideDown && flipView) { kind ^= 2; p += p < BlackPawn ? BlackPawn : -BlackPawn; }// swap white and black pieces
    if(useTexture & square_color+1) {
        BlankSquare(x, y, square_color, piece, dest, 1); // erase previous contents with background
	XSetClipMask(xDisplay, wlPieceGC, xpmMask[p]);
	XSetClipOrigin(xDisplay, wlPieceGC, x, y);
	XCopyArea(xDisplay, xpmPieceBitmap[kind][piece], dest, wlPieceGC, 0, 0, squareSize, squareSize, x, y);
	XSetClipMask(xDisplay, wlPieceGC, None);
	XSetClipOrigin(xDisplay, wlPieceGC, 0, 0);
    } else
    XCopyArea(xDisplay, xpmPieceBitmap[kind][piece],
	      dest, wlPieceGC, 0, 0,
	      squareSize, squareSize, x, y);
}

typedef void (*DrawFunc)();

DrawFunc
ChooseDrawFunc ()
{
    if (appData.monoMode) {
	if (DefaultDepth(xDisplay, xScreen) == 1) {
	    return monoDrawPiece_1bit;
	} else {
	    return monoDrawPiece;
	}
    } else {
	if (useImages)
	  return colorDrawPieceImage;
	else
	  return colorDrawPiece;
    }
}

void
DrawDot (int marker, int x, int y, int r)
{
	if(appData.monoMode) {
	    XFillArc(xDisplay, xBoardWindow, marker == 2 ? darkSquareGC : lightSquareGC,
		    x, y, r, r, 0, 64*360);
	    XDrawArc(xDisplay, xBoardWindow, marker == 2 ? lightSquareGC : darkSquareGC,
		    x, y, r, r, 0, 64*360);
	} else
	XFillArc(xDisplay, xBoardWindow, marker == 2 ? prelineGC : highlineGC,
		    x, y, r, r, 0, 64*360);
}

void
DrawOneSquare (int x, int y, ChessSquare piece, int square_color, int marker, char *string, int align)
{   // basic front-end board-draw function: takes care of everything that can be in square:
    // piece, background, coordinate/count, marker dot
    int direction, font_ascent, font_descent;
    XCharStruct overall;
    DrawFunc drawfunc;

    if (piece == EmptySquare) {
	BlankSquare(x, y, square_color, piece, xBoardWindow, 1);
    } else {
	drawfunc = ChooseDrawFunc();
	drawfunc(piece, square_color, x, y, xBoardWindow);
    }

    if(align) { // square carries inscription (coord or piece count)
	int xx = x, yy = y;
	GC hGC = align < 3 ? coordGC : countGC;
	// first calculate where it goes
	XTextExtents(countFontStruct, string, 1, &direction,
			 &font_ascent, &font_descent, &overall);
	if (align == 1) {
	    xx += squareSize - overall.width - 2;
	    yy += squareSize - font_descent - 1;
	} else if (align == 2) {
	    xx += 2, yy += font_ascent + 1;
	} else if (align == 3) {
	    xx += squareSize - overall.width - 2;
	    yy += font_ascent + 1;
	} else if (align == 4) {
	    xx += 2, yy += font_ascent + 1;
	}
	// then draw it
	if (appData.monoMode) {
	    XDrawImageString(xDisplay, xBoardWindow, hGC, xx, yy, string, 1);
	} else {
	    XDrawString(xDisplay, xBoardWindow, hGC, xx, yy, string, 1);
	}
    }

    if(marker) { // print fat marker dot, if requested
	DrawDot(marker, x + squareSize/4, y+squareSize/4, squareSize/2);
    }
}

void
FlashDelay (int flash_delay)
{
	XSync(xDisplay, False);
	if(flash_delay) do_flash_delay(flash_delay);
}

double
Fraction (int x, int start, int stop)
{
   double f = ((double) x - start)/(stop - start);
   if(f > 1.) f = 1.; else if(f < 0.) f = 0.;
   return f;
}

static WindowPlacement wpNew;

void
CoDrag (Widget sh, WindowPlacement *wp)
{
    Arg args[16];
    int j=0, touch=0, fudge = 2;
    GetActualPlacement(sh, wp);
    if(abs(wpMain.x + wpMain.width + 2*frameX - wp->x)         < fudge) touch = 1; else // right touch
    if(abs(wp->x + wp->width + 2*frameX - wpMain.x)            < fudge) touch = 2; else // left touch
    if(abs(wpMain.y + wpMain.height + frameX + frameY - wp->y) < fudge) touch = 3; else // bottom touch
    if(abs(wp->y + wp->height + frameX + frameY - wpMain.y)    < fudge) touch = 4;      // top touch
    if(!touch ) return; // only windows that touch co-move
    if(touch < 3 && wpNew.height != wpMain.height) { // left or right and height changed
	int heightInc = wpNew.height - wpMain.height;
	double fracTop = Fraction(wp->y, wpMain.y, wpMain.y + wpMain.height + frameX + frameY);
	double fracBot = Fraction(wp->y + wp->height + frameX + frameY + 1, wpMain.y, wpMain.y + wpMain.height + frameX + frameY);
	wp->y += fracTop * heightInc;
	heightInc = (int) (fracBot * heightInc) - (int) (fracTop * heightInc);
	if(heightInc) XtSetArg(args[j], XtNheight, wp->height + heightInc), j++;
    } else if(touch > 2 && wpNew.width != wpMain.width) { // top or bottom and width changed
	int widthInc = wpNew.width - wpMain.width;
	double fracLeft = Fraction(wp->x, wpMain.x, wpMain.x + wpMain.width + 2*frameX);
	double fracRght = Fraction(wp->x + wp->width + 2*frameX + 1, wpMain.x, wpMain.x + wpMain.width + 2*frameX);
	wp->y += fracLeft * widthInc;
	widthInc = (int) (fracRght * widthInc) - (int) (fracLeft * widthInc);
	if(widthInc) XtSetArg(args[j], XtNwidth, wp->width + widthInc), j++;
    }
    wp->x += wpNew.x - wpMain.x;
    wp->y += wpNew.y - wpMain.y;
    if(touch == 1) wp->x += wpNew.width - wpMain.width; else
    if(touch == 3) wp->y += wpNew.height - wpMain.height;
    XtSetArg(args[j], XtNx, wp->x); j++;
    XtSetArg(args[j], XtNy, wp->y); j++;
    XtSetValues(sh, args, j);
}

static XtIntervalId delayedDragID = 0;

void
DragProc ()
{
	GetActualPlacement(shellWidget, &wpNew);
	if(wpNew.x == wpMain.x && wpNew.y == wpMain.y && // not moved
	   wpNew.width == wpMain.width && wpNew.height == wpMain.height) // not sized
	    return; // false alarm
	if(EngineOutputIsUp()) CoDrag(engineOutputShell, &wpEngineOutput);
	if(MoveHistoryIsUp()) CoDrag(shells[HistoryDlg], &wpMoveHistory);
	if(EvalGraphIsUp()) CoDrag(evalGraphShell, &wpEvalGraph);
	if(GameListIsUp()) CoDrag(gameListShell, &wpGameList);
	wpMain = wpNew;
	DrawPosition(True, NULL);
	delayedDragID = 0; // now drag executed, make sure next DelayedDrag will not cancel timer event (which could now be used by other)
}


void
DelayedDrag ()
{
    if(delayedDragID) XtRemoveTimeOut(delayedDragID); // cancel pending
    delayedDragID =
      XtAppAddTimeOut(appContext, 50, (XtTimerCallbackProc) DragProc, (XtPointer) 0); // and schedule new one 50 msec later
}

/* Why is this needed on some versions of X? */
void
EventProc (Widget widget, caddr_t unused, XEvent *event)
{
    if (!XtIsRealized(widget))
      return;
    switch (event->type) {
      case ConfigureNotify: // main window is being dragged: drag attached windows with it
	if(appData.useStickyWindows)
	    DelayedDrag(); // as long as events keep coming in faster than 50 msec, they destroy each other
	break;
      case Expose:
	if (event->xexpose.count > 0) return;  /* no clipping is done */
	DrawPosition(True, NULL);
	if(twoBoards) { // [HGM] dual: draw other board in other orientation
	    flipView = !flipView; partnerUp = !partnerUp;
	    DrawPosition(True, NULL);
	    flipView = !flipView; partnerUp = !partnerUp;
	}
	break;
      case MotionNotify:
        if(SeekGraphClick(Press, event->xbutton.x, event->xbutton.y, 1)) break;
      default:
	return;
    }
}
/* end why */

// [HGM] seekgraph: some low-level drawing routines cloned from xevalgraph
void
DrawSeekAxis (int x, int y, int xTo, int yTo)
{
      XDrawLine(xDisplay, xBoardWindow, lineGC, x, y, xTo, yTo);
}

void
DrawSeekBackground (int left, int top, int right, int bottom)
{
    XFillRectangle(xDisplay, xBoardWindow, lightSquareGC, left, top, right-left, bottom-top);
}

void
DrawSeekText (char *buf, int x, int y)
{
    XDrawString(xDisplay, xBoardWindow, coordGC, x, y+4, buf, strlen(buf));
}

void
DrawSeekDot (int x, int y, int colorNr)
{
    int square = colorNr & 0x80;
    GC color;
    colorNr &= 0x7F;
    color = colorNr == 0 ? prelineGC : colorNr == 1 ? darkSquareGC : highlineGC;
    if(square)
	XFillRectangle(xDisplay, xBoardWindow, color,
		x-squareSize/9, y-squareSize/9, 2*squareSize/9, 2*squareSize/9);
    else
	XFillArc(xDisplay, xBoardWindow, color,
		x-squareSize/8, y-squareSize/8, squareSize/4, squareSize/4, 0, 64*360);
}

void
DrawGrid (int second)
{
	  XDrawSegments(xDisplay, xBoardWindow, lineGC,
			second ? secondSegments : // [HGM] dual
			gridSegments, BOARD_HEIGHT + BOARD_WIDTH + 2);
}


/*
 * event handler for redrawing the board
 */
void
DrawPositionProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    DrawPosition(True, NULL);
}


/*
 * event handler for parsing user moves
 */
// [HGM] This routine will need quite some reworking. Although the backend still supports the old
//       way of doing things, by calling UserMoveEvent() to test the legality of the move and then perform
//       it at the end, and doing all kind of preliminary tests here (e.g. to weed out self-captures), it
//       should be made to use the new way, of calling UserMoveTest early  to determine the legality of the
//       move, (which will weed out the illegal selfcaptures and moves into the holdings, and flag promotions),
//       and at the end FinishMove() to perform the move after optional promotion popups.
//       For now I patched it to allow self-capture with King, and suppress clicks between board and holdings.
void
HandleUserMove (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    if (w != boardWidget || errorExitStatus != -1) return;
    if(nprms) shiftKey = !strcmp(prms[0], "1");

    if (promotionUp) {
	if (event->type == ButtonPress) {
	    XtPopdown(promotionShell);
	    XtDestroyWidget(promotionShell);
	    promotionUp = False;
	    ClearHighlights();
	    fromX = fromY = -1;
	} else {
	    return;
	}
    }

    // [HGM] mouse: the rest of the mouse handler is moved to the backend, and called here
    if(event->type == ButtonPress)   LeftClick(Press,   event->xbutton.x, event->xbutton.y);
    if(event->type == ButtonRelease) LeftClick(Release, event->xbutton.x, event->xbutton.y);
}

void
AnimateUserMove (Widget w, XEvent *event, String *params, Cardinal *nParams)
{
    if(!PromoScroll(event->xmotion.x, event->xmotion.y))
    DragPieceMove(event->xmotion.x, event->xmotion.y);
}

void
HandlePV (Widget w, XEvent * event, String * params, Cardinal * nParams)
{   // [HGM] pv: walk PV
    MovePV(event->xmotion.x, event->xmotion.y, lineGap + BOARD_HEIGHT * (squareSize + lineGap));
}

static int savedIndex;  /* gross that this is global */

void
CommentClick (Widget w, XEvent * event, String * params, Cardinal * nParams)
{
	String val;
	XawTextPosition index, dummy;
	Arg arg;

	XawTextGetSelectionPos(w, &index, &dummy);
	XtSetArg(arg, XtNstring, &val);
	XtGetValues(w, &arg, 1);
	ReplaceComment(savedIndex, val);
	if(savedIndex != currentMove) ToNrEvent(savedIndex);
	LoadVariation( index, val ); // [HGM] also does the actual moving to it, now
}

void
EditCommentPopUp (int index, char *title, char *text)
{
    savedIndex = index;
    if (text == NULL) text = "";
    NewCommentPopup(title, text, index);
}

void
ICSInputBoxPopUp ()
{
    InputBoxPopup();
}

extern Option boxOptions[];

void
ICSInputSendText ()
{
    Widget edit;
    int j;
    Arg args[16];
    String val;

    edit = boxOptions[0].handle;
    j = 0;
    XtSetArg(args[j], XtNstring, &val); j++;
    XtGetValues(edit, args, j);
    SaveInHistory(val);
    SendMultiLineToICS(val);
    XtCallActionProc(edit, "select-all", NULL, NULL, 0);
    XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
}

void
ICSInputBoxPopDown ()
{
    PopDown(InputBoxDlg);
}

void
CommentPopUp (char *title, char *text)
{
    savedIndex = currentMove; // [HGM] vari
    NewCommentPopup(title, text, currentMove);
}

void
CommentPopDown ()
{
    PopDown(CommentDlg);
}

static char *openName;
FILE *openFP;

void
DelayedLoad ()
{
  (void) (*fileProc)(openFP, 0, openName);
}

void
FileNamePopUp (char *label, char *def, char *filter, FileProc proc, char *openMode)
{
    fileProc = proc;		/* I can't see a way not */
    fileOpenMode = openMode;	/*   to use globals here */
    {   // [HGM] use file-selector dialog stolen from Ghostview
	int index; // this is not supported yet
	if(openFP = XsraSelFile(shellWidget, label, NULL, NULL, _("could not open: "),
			   (def[0] ? def : NULL), filter, openMode, NULL, &openName))
	  // [HGM] delay to give expose event opportunity to redraw board after browser-dialog popdown before lengthy load starts
	  ScheduleDelayedEvent(&DelayedLoad, 50);
    }
}

void
FileNamePopDown ()
{
    if (!filenameUp) return;
    XtPopdown(fileNameShell);
    XtDestroyWidget(fileNameShell);
    filenameUp = False;
    ModeHighlight();
}

void
FileNameCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    String name;
    Arg args[16];

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    if (strcmp(name, _("cancel")) == 0) {
        FileNamePopDown();
        return;
    }

    FileNameAction(w, NULL, NULL, NULL);
}

void
FileNameAction (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    char buf[MSG_SIZ];
    String name;
    FILE *f;
    char *p, *fullname;
    int index;

    name = XawDialogGetValueString(w = XtParent(w));

    if ((name != NULL) && (*name != NULLCHAR)) {
        safeStrCpy(buf, name, sizeof(buf)/sizeof(buf[0]) );
	XtPopdown(w = XtParent(XtParent(w)));
	XtDestroyWidget(w);
	filenameUp = False;

	p = strrchr(buf, ' ');
	if (p == NULL) {
	    index = 0;
	} else {
	    *p++ = NULLCHAR;
	    index = atoi(p);
	}
	fullname = ExpandPathName(buf);
	if (!fullname) {
	    ErrorPopUp(_("Error"), _("Can't open file"), FALSE);
	}
	else {
	    f = fopen(fullname, fileOpenMode);
	    if (f == NULL) {
		DisplayError(_("Failed to open file"), errno);
	    } else {
		(void) (*fileProc)(f, index, buf);
	    }
	}
	ModeHighlight();
	return;
    }

    XtPopdown(w = XtParent(XtParent(w)));
    XtDestroyWidget(w);
    filenameUp = False;
    ModeHighlight();
}

void
PromotionPopUp ()
{
    Arg args[16];
    Widget dialog, layout;
    Position x, y;
    Dimension bw_width, pw_width;
    int j;
    char *PromoChars = "wglcqrbnkac+=\0";

    j = 0;
    XtSetArg(args[j], XtNwidth, &bw_width); j++;
    XtGetValues(boardWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNresizable, True); j++;
    XtSetArg(args[j], XtNtitle, XtNewString(_("Promotion"))); j++;
    promotionShell =
      XtCreatePopupShell("Promotion", transientShellWidgetClass,
			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, promotionShell,
			    layoutArgs, XtNumber(layoutArgs));

    j = 0;
    XtSetArg(args[j], XtNlabel, _("Promote to what?")); j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    dialog = XtCreateManagedWidget("promotion", dialogWidgetClass,
				   layout, args, j);

  if(gameInfo.variant != VariantShogi) {
   if(gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove)) {
      XawDialogAddButton(dialog, _("Warlord"), PromotionCallback, PromoChars + 0);
      XawDialogAddButton(dialog, _("General"), PromotionCallback, PromoChars + 1);
      XawDialogAddButton(dialog, _("Lieutenant"), PromotionCallback, PromoChars + 2);
      XawDialogAddButton(dialog, _("Captain"), PromotionCallback, PromoChars + 3);
    } else {
    XawDialogAddButton(dialog, _("Queen"), PromotionCallback, PromoChars + 4);
    XawDialogAddButton(dialog, _("Rook"), PromotionCallback, PromoChars + 5);
    XawDialogAddButton(dialog, _("Bishop"), PromotionCallback, PromoChars + 6);
    XawDialogAddButton(dialog, _("Knight"), PromotionCallback, PromoChars + 7);
    }
    if (!appData.testLegality || gameInfo.variant == VariantSuicide ||
        gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove) ||
        gameInfo.variant == VariantGiveaway) {
      XawDialogAddButton(dialog, _("King"), PromotionCallback, PromoChars + 8);
    }
    if(gameInfo.variant == VariantCapablanca ||
       gameInfo.variant == VariantGothic ||
       gameInfo.variant == VariantCapaRandom) {
      XawDialogAddButton(dialog, _("Archbishop"), PromotionCallback, PromoChars + 9);
      XawDialogAddButton(dialog, _("Chancellor"), PromotionCallback, PromoChars + 10);
    }
  } else // [HGM] shogi
  {
      XawDialogAddButton(dialog, _("Promote"), PromotionCallback, PromoChars + 11);
      XawDialogAddButton(dialog, _("Defer"), PromotionCallback, PromoChars + 12);
  }
    XawDialogAddButton(dialog, _("cancel"), PromotionCallback, PromoChars + 13);

    XtRealizeWidget(promotionShell);
    CatchDeleteWindow(promotionShell, "PromotionPopDown");

    j = 0;
    XtSetArg(args[j], XtNwidth, &pw_width); j++;
    XtGetValues(promotionShell, args, j);

    XtTranslateCoords(boardWidget, (bw_width - pw_width) / 2,
		      lineGap + squareSize/3 +
		      ((toY == BOARD_HEIGHT-1) ^ (flipView) ?
		       0 : 6*(squareSize + lineGap)), &x, &y);

    j = 0;
    XtSetArg(args[j], XtNx, x); j++;
    XtSetArg(args[j], XtNy, y); j++;
    XtSetValues(promotionShell, args, j);

    XtPopup(promotionShell, XtGrabNone);

    promotionUp = True;
}

void
PromotionPopDown ()
{
    if (!promotionUp) return;
    XtPopdown(promotionShell);
    XtDestroyWidget(promotionShell);
    promotionUp = False;
}

void
PromotionCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    int promoChar = * (const char *) client_data;

    PromotionPopDown();

    if (fromX == -1) return;

    if (! promoChar) {
	fromX = fromY = -1;
	ClearHighlights();
	return;
    }
    UserMoveEvent(fromX, fromY, toX, toY, promoChar);

    if (!appData.highlightLastMove || gotPremove) ClearHighlights();
    if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
    fromX = fromY = -1;
}


void
ErrorCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    dialogError = errorUp = False;
    XtPopdown(w = XtParent(XtParent(XtParent(w))));
    XtDestroyWidget(w);
    if (errorExitStatus != -1) ExitEvent(errorExitStatus);
}


void
ErrorPopDown ()
{
    if (!errorUp) return;
    dialogError = errorUp = False;
    XtPopdown(errorShell);
    XtDestroyWidget(errorShell);
    if (errorExitStatus != -1) ExitEvent(errorExitStatus);
}

void
ErrorPopUp (char *title, char *label, int modal)
{
    Arg args[16];
    Widget dialog, layout;
    Position x, y;
    int xx, yy;
    Window junk;
    Dimension bw_width, pw_width;
    Dimension pw_height;
    int i;

    i = 0;
    XtSetArg(args[i], XtNresizable, True);  i++;
    XtSetArg(args[i], XtNtitle, title); i++;
    errorShell =
      XtCreatePopupShell("errorpopup", transientShellWidgetClass,
			 shellUp[TransientDlg] ? (dialogError = modal = TRUE, shells[TransientDlg]) : shellWidget, args, i);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, errorShell,
			    layoutArgs, XtNumber(layoutArgs));

    i = 0;
    XtSetArg(args[i], XtNlabel, label); i++;
    XtSetArg(args[i], XtNborderWidth, 0); i++;
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass,
				   layout, args, i);

    XawDialogAddButton(dialog, _("ok"), ErrorCallback, (XtPointer) dialog);

    XtRealizeWidget(errorShell);
    CatchDeleteWindow(errorShell, "ErrorPopDown");

    i = 0;
    XtSetArg(args[i], XtNwidth, &bw_width);  i++;
    XtGetValues(boardWidget, args, i);
    i = 0;
    XtSetArg(args[i], XtNwidth, &pw_width);  i++;
    XtSetArg(args[i], XtNheight, &pw_height);  i++;
    XtGetValues(errorShell, args, i);

#ifdef NOTDEF
    /* This code seems to tickle an X bug if it is executed too soon
       after xboard starts up.  The coordinates get transformed as if
       the main window was positioned at (0, 0).
       */
    XtTranslateCoords(boardWidget, (bw_width - pw_width) / 2,
		      0 - pw_height + squareSize / 3, &x, &y);
#else
    XTranslateCoordinates(xDisplay, XtWindow(boardWidget),
			  RootWindowOfScreen(XtScreen(boardWidget)),
			  (bw_width - pw_width) / 2,
			  0 - pw_height + squareSize / 3, &xx, &yy, &junk);
    x = xx;
    y = yy;
#endif
    if (y < 0) y = 0; /*avoid positioning top offscreen*/

    i = 0;
    XtSetArg(args[i], XtNx, x);  i++;
    XtSetArg(args[i], XtNy, y);  i++;
    XtSetValues(errorShell, args, i);

    errorUp = True;
    XtPopup(errorShell, modal ? XtGrabExclusive : XtGrabNone);
}

/* Disable all user input other than deleting the window */
static int frozen = 0;

void
FreezeUI ()
{
  if (frozen) return;
  /* Grab by a widget that doesn't accept input */
  XtAddGrab(messageWidget, TRUE, FALSE);
  frozen = 1;
}

/* Undo a FreezeUI */
void
ThawUI ()
{
  if (!frozen) return;
  XtRemoveGrab(messageWidget);
  frozen = 0;
}

void
ModeHighlight ()
{
    Arg args[16];
    static int oldPausing = FALSE;
    static GameMode oldmode = (GameMode) -1;
    char *wname;

    if (!boardWidget || !XtIsRealized(boardWidget)) return;

    if (pausing != oldPausing) {
	oldPausing = pausing;
	MarkMenuItem("Pause", pausing);

	if (appData.showButtonBar) {
	  /* Always toggle, don't set.  Previous code messes up when
	     invoked while the button is pressed, as releasing it
	     toggles the state again. */
	  {
	    Pixel oldbg, oldfg;
	    XtSetArg(args[0], XtNbackground, &oldbg);
	    XtSetArg(args[1], XtNforeground, &oldfg);
	    XtGetValues(XtNameToWidget(buttonBarWidget, PAUSE_BUTTON),
			args, 2);
	    XtSetArg(args[0], XtNbackground, oldfg);
	    XtSetArg(args[1], XtNforeground, oldbg);
	  }
	  XtSetValues(XtNameToWidget(buttonBarWidget, PAUSE_BUTTON), args, 2);
	}
    }

    wname = ModeToWidgetName(oldmode);
    if (wname != NULL) {
	MarkMenuItem(wname, False);
    }
    wname = ModeToWidgetName(gameMode);
    if (wname != NULL) {
	MarkMenuItem(wname, True);
    }
    oldmode = gameMode;
    MarkMenuItem("Machine Match", matchMode && matchGame < appData.matchGames);

    /* Maybe all the enables should be handled here, not just this one */
    EnableMenuItem("Training", gameMode == Training || gameMode == PlayFromGameFile);
}


/*
 * Button/menu procedures
 */
int
LoadGamePopUp (FILE *f, int gameNumber, char *title)
{
    cmailMsgLoaded = FALSE;
    if (gameNumber == 0) {
	int error = GameListBuild(f);
	if (error) {
	    DisplayError(_("Cannot build game list"), error);
	} else if (!ListEmpty(&gameList) &&
		   ((ListGame *) gameList.tailPred)->number > 1) {
	    GameListPopUp(f, title);
	    return TRUE;
	}
	GameListDestroy();
	gameNumber = 1;
    }
    return LoadGame(f, gameNumber, title, FALSE);
}

/* this variable is shared between CopyPositionProc and SendPositionSelection */
char *selected_fen_position=NULL;

Boolean
SendPositionSelection (Widget w, Atom *selection, Atom *target,
		       Atom *type_return, XtPointer *value_return,
		       unsigned long *length_return, int *format_return)
{
  char *selection_tmp;

  if (!selected_fen_position) return False; /* should never happen */
  if (*target == XA_STRING || *target == XA_UTF8_STRING(xDisplay)){
    /* note: since no XtSelectionDoneProc was registered, Xt will
     * automatically call XtFree on the value returned.  So have to
     * make a copy of it allocated with XtMalloc */
    selection_tmp= XtMalloc(strlen(selected_fen_position)+16);
    safeStrCpy(selection_tmp, selected_fen_position, strlen(selected_fen_position)+16 );

    *value_return=selection_tmp;
    *length_return=strlen(selection_tmp);
    *type_return=*target;
    *format_return = 8; /* bits per byte */
    return True;
  } else if (*target == XA_TARGETS(xDisplay)) {
    Atom *targets_tmp = (Atom *) XtMalloc(2 * sizeof(Atom));
    targets_tmp[0] = XA_UTF8_STRING(xDisplay);
    targets_tmp[1] = XA_STRING;
    *value_return = targets_tmp;
    *type_return = XA_ATOM;
    *length_return = 2;
#if 0
    // This code leads to a read of value_return out of bounds on 64-bit systems.
    // Other code which I have seen always sets *format_return to 32 independent of
    // sizeof(Atom) without adjusting *length_return. For instance see TextConvertSelection()
    // at http://cgit.freedesktop.org/xorg/lib/libXaw/tree/src/Text.c -- BJ
    *format_return = 8 * sizeof(Atom);
    if (*format_return > 32) {
      *length_return *= *format_return / 32;
      *format_return = 32;
    }
#else
    *format_return = 32;
#endif
    return True;
  } else {
    return False;
  }
}

/* note: when called from menu all parameters are NULL, so no clue what the
 * Widget which was clicked on was, or what the click event was
 */
void
CopyPositionProc ()
{
    /*
     * Set both PRIMARY (the selection) and CLIPBOARD, since we don't
     * have a notion of a position that is selected but not copied.
     * See http://www.freedesktop.org/wiki/Specifications/ClipboardsWiki
     */
    if(gameMode == EditPosition) EditPositionDone(TRUE);
    if (selected_fen_position) free(selected_fen_position);
    selected_fen_position = (char *)PositionToFEN(currentMove, NULL);
    if (!selected_fen_position) return;
    XtOwnSelection(menuBarWidget, XA_PRIMARY,
		   CurrentTime,
		   SendPositionSelection,
		   NULL/* lose_ownership_proc */ ,
		   NULL/* transfer_done_proc */);
    XtOwnSelection(menuBarWidget, XA_CLIPBOARD(xDisplay),
		   CurrentTime,
		   SendPositionSelection,
		   NULL/* lose_ownership_proc */ ,
		   NULL/* transfer_done_proc */);
}

/* function called when the data to Paste is ready */
static void
PastePositionCB (Widget w, XtPointer client_data, Atom *selection,
		 Atom *type, XtPointer value, unsigned long *len, int *format)
{
  char *fenstr=value;
  if (value==NULL || *len==0) return; /* nothing had been selected to copy */
  fenstr[*len]='\0'; /* normally this string is terminated, but be safe */
  EditPositionPasteFEN(fenstr);
  XtFree(value);
}

/* called when Paste Position button is pressed,
 * all parameters will be NULL */
void
PastePositionProc ()
{
    XtGetSelectionValue(menuBarWidget,
      appData.pasteSelection ? XA_PRIMARY: XA_CLIPBOARD(xDisplay), XA_STRING,
      /* (XtSelectionCallbackProc) */ PastePositionCB,
      NULL, /* client_data passed to PastePositionCB */

      /* better to use the time field from the event that triggered the
       * call to this function, but that isn't trivial to get
       */
      CurrentTime
    );
    return;
}

static Boolean
SendGameSelection (Widget w, Atom *selection, Atom *target,
		   Atom *type_return, XtPointer *value_return,
		   unsigned long *length_return, int *format_return)
{
  char *selection_tmp;

  if (*target == XA_STRING || *target == XA_UTF8_STRING(xDisplay)){
    FILE* f = fopen(gameCopyFilename, "r");
    long len;
    size_t count;
    if (f == NULL) return False;
    fseek(f, 0, 2);
    len = ftell(f);
    rewind(f);
    selection_tmp = XtMalloc(len + 1);
    count = fread(selection_tmp, 1, len, f);
    fclose(f);
    if (len != count) {
      XtFree(selection_tmp);
      return False;
    }
    selection_tmp[len] = NULLCHAR;
    *value_return = selection_tmp;
    *length_return = len;
    *type_return = *target;
    *format_return = 8; /* bits per byte */
    return True;
  } else if (*target == XA_TARGETS(xDisplay)) {
    Atom *targets_tmp = (Atom *) XtMalloc(2 * sizeof(Atom));
    targets_tmp[0] = XA_UTF8_STRING(xDisplay);
    targets_tmp[1] = XA_STRING;
    *value_return = targets_tmp;
    *type_return = XA_ATOM;
    *length_return = 2;
#if 0
    // This code leads to a read of value_return out of bounds on 64-bit systems.
    // Other code which I have seen always sets *format_return to 32 independent of
    // sizeof(Atom) without adjusting *length_return. For instance see TextConvertSelection()
    // at http://cgit.freedesktop.org/xorg/lib/libXaw/tree/src/Text.c -- BJ
    *format_return = 8 * sizeof(Atom);
    if (*format_return > 32) {
      *length_return *= *format_return / 32;
      *format_return = 32;
    }
#else
    *format_return = 32;
#endif
    return True;
  } else {
    return False;
  }
}

void
CopySomething ()
{
  /*
   * Set both PRIMARY (the selection) and CLIPBOARD, since we don't
   * have a notion of a game that is selected but not copied.
   * See http://www.freedesktop.org/wiki/Specifications/ClipboardsWiki
   */
  XtOwnSelection(menuBarWidget, XA_PRIMARY,
		 CurrentTime,
		 SendGameSelection,
		 NULL/* lose_ownership_proc */ ,
		 NULL/* transfer_done_proc */);
  XtOwnSelection(menuBarWidget, XA_CLIPBOARD(xDisplay),
		 CurrentTime,
		 SendGameSelection,
		 NULL/* lose_ownership_proc */ ,
		 NULL/* transfer_done_proc */);
}

/* note: when called from menu all parameters are NULL, so no clue what the
 * Widget which was clicked on was, or what the click event was
 */
/* function called when the data to Paste is ready */
static void
PasteGameCB (Widget w, XtPointer client_data, Atom *selection,
	     Atom *type, XtPointer value, unsigned long *len, int *format)
{
  FILE* f;
  if (value == NULL || *len == 0) {
    return; /* nothing had been selected to copy */
  }
  f = fopen(gamePasteFilename, "w");
  if (f == NULL) {
    DisplayError(_("Can't open temp file"), errno);
    return;
  }
  fwrite(value, 1, *len, f);
  fclose(f);
  XtFree(value);
  LoadGameFromFile(gamePasteFilename, 0, gamePasteFilename, TRUE);
}

/* called when Paste Game button is pressed,
 * all parameters will be NULL */
void
PasteGameProc ()
{
    XtGetSelectionValue(menuBarWidget,
      appData.pasteSelection ? XA_PRIMARY: XA_CLIPBOARD(xDisplay), XA_STRING,
      /* (XtSelectionCallbackProc) */ PasteGameCB,
      NULL, /* client_data passed to PasteGameCB */

      /* better to use the time field from the event that triggered the
       * call to this function, but that isn't trivial to get
       */
      CurrentTime
    );
    return;
}


void
QuitWrapper (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    QuitProc();
}

static void
MoveTypeInProc (Widget widget, caddr_t unused, XEvent *event)
{
    char buf[10], keys[32];
    KeySym sym;
    KeyCode metaL, metaR; //, ctrlL, ctrlR;
    int n = XLookupString(&(event->xkey), buf, 10, &sym, NULL);
    XQueryKeymap(xDisplay,keys);
    metaL = XKeysymToKeycode(xDisplay, XK_Meta_L);
    metaR = XKeysymToKeycode(xDisplay, XK_Meta_R);
//    ctrlL = XKeysymToKeycode(xDisplay, XK_Control_L);
//    ctrlR = XKeysymToKeycode(xDisplay, XK_Control_R);
    if ( n == 1 && *buf >= 32 // printable
	 && !(keys[metaL>>3]&1<<(metaL&7)) && !(keys[metaR>>3]&1<<(metaR&7)) // no alt key pressed
//	 && !(keys[ctrlL>>3]&1<<(ctrlL&7)) && !(keys[ctrlR>>3]&1<<(ctrlR&7)) // no ctrl key pressed
       ) BoxAutoPopUp (buf);
}

void
UpKeyProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // [HGM] input: let up-arrow recall previous line from history
    Widget edit;
    int j;
    Arg args[16];
    String val;
    XawTextBlock t;

    if (!shellUp[InputBoxDlg]) return;
    edit = boxOptions[0].handle;
    j = 0;
    XtSetArg(args[j], XtNstring, &val); j++;
    XtGetValues(edit, args, j);
    val = PrevInHistory(val);
    XtCallActionProc(edit, "select-all", NULL, NULL, 0);
    XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
    if(val) {
	t.ptr = val; t.firstPos = 0; t.length = strlen(val); t.format = XawFmt8Bit;
	XawTextReplace(edit, 0, 0, &t);
	XawTextSetInsertionPoint(edit, 9999);
    }
}

void
DownKeyProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // [HGM] input: let down-arrow recall next line from history
    Widget edit;
    String val;
    XawTextBlock t;

    if (!shellUp[InputBoxDlg]) return;
    edit = boxOptions[0].handle;
    val = NextInHistory();
    XtCallActionProc(edit, "select-all", NULL, NULL, 0);
    XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
    if(val) {
	t.ptr = val; t.firstPos = 0; t.length = strlen(val); t.format = XawFmt8Bit;
	XawTextReplace(edit, 0, 0, &t);
	XawTextSetInsertionPoint(edit, 9999);
    }
}

void
EnterKeyProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    if (shellUp[InputBoxDlg] == True)
      ICSInputSendText();
}

void
TempBackwardProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
	if (!TempBackwardActive) {
		TempBackwardActive = True;
		BackwardEvent();
	}
}

void
TempForwardProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
	/* Check to see if triggered by a key release event for a repeating key.
	 * If so the next queued event will be a key press of the same key at the same time */
	if (XEventsQueued(xDisplay, QueuedAfterReading)) {
		XEvent next;
		XPeekEvent(xDisplay, &next);
		if (next.type == KeyPress && next.xkey.time == event->xkey.time &&
			next.xkey.keycode == event->xkey.keycode)
				return;
	}
    ForwardEvent();
	TempBackwardActive = False;
}

void
ManInner (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // called as key binding
    char buf[MSG_SIZ];
    String name;
    if (nprms && *nprms > 0)
      name = prms[0];
    else
      name = "xboard";
    snprintf(buf, sizeof(buf), "xterm -e man %s &", name);
    system(buf);
}

void
DisplayMessage (char *message, char *extMessage)
{
  /* display a message in the message widget */

  char buf[MSG_SIZ];
  Arg arg;

  if (extMessage)
    {
      if (*message)
	{
	  snprintf(buf, sizeof(buf), "%s  %s", message, extMessage);
	  message = buf;
	}
      else
	{
	  message = extMessage;
	};
    };

    safeStrCpy(lastMsg, message, MSG_SIZ); // [HGM] make available

  /* need to test if messageWidget already exists, since this function
     can also be called during the startup, if for example a Xresource
     is not set up correctly */
  if(messageWidget)
    {
      XtSetArg(arg, XtNlabel, message);
      XtSetValues(messageWidget, &arg, 1);
    };

  return;
}

void
DisplayTitle (char *text)
{
    Arg args[16];
    int i;
    char title[MSG_SIZ];
    char icon[MSG_SIZ];

    if (text == NULL) text = "";

    if (appData.titleInWindow) {
	i = 0;
	XtSetArg(args[i], XtNlabel, text);   i++;
	XtSetValues(titleWidget, args, i);
    }

    if (*text != NULLCHAR) {
      safeStrCpy(icon, text, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, text, sizeof(title)/sizeof(title[0]) );
    } else if (appData.icsActive) {
        snprintf(icon, sizeof(icon), "%s", appData.icsHost);
	snprintf(title, sizeof(title), "%s: %s", programName, appData.icsHost);
    } else if (appData.cmailGameName[0] != NULLCHAR) {
        snprintf(icon, sizeof(icon), "%s", "CMail");
	snprintf(title,sizeof(title), "%s: %s", programName, "CMail");
#ifdef GOTHIC
    // [HGM] license: This stuff should really be done in back-end, but WinBoard already had a pop-up for it
    } else if (gameInfo.variant == VariantGothic) {
      safeStrCpy(icon,  programName, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, GOTHIC,     sizeof(title)/sizeof(title[0]) );
#endif
#ifdef FALCON
    } else if (gameInfo.variant == VariantFalcon) {
      safeStrCpy(icon, programName, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, FALCON, sizeof(title)/sizeof(title[0]) );
#endif
    } else if (appData.noChessProgram) {
      safeStrCpy(icon, programName, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, programName, sizeof(title)/sizeof(title[0]) );
    } else {
      safeStrCpy(icon, first.tidy, sizeof(icon)/sizeof(icon[0]) );
	snprintf(title,sizeof(title), "%s: %s", programName, first.tidy);
    }
    i = 0;
    XtSetArg(args[i], XtNiconName, (XtArgVal) icon);    i++;
    XtSetArg(args[i], XtNtitle, (XtArgVal) title);      i++;
    XtSetValues(shellWidget, args, i);
    XSync(xDisplay, False);
}


void
DisplayError (String message, int error)
{
    char buf[MSG_SIZ];

    if (error == 0) {
	if (appData.debugMode || appData.matchMode) {
	    fprintf(stderr, "%s: %s\n", programName, message);
	}
    } else {
	if (appData.debugMode || appData.matchMode) {
	    fprintf(stderr, "%s: %s: %s\n",
		    programName, message, strerror(error));
	}
	snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
	message = buf;
    }
    ErrorPopUp(_("Error"), message, FALSE);
}


void
DisplayMoveError (String message)
{
    fromX = fromY = -1;
    ClearHighlights();
    DrawPosition(FALSE, NULL);
    if (appData.debugMode || appData.matchMode) {
	fprintf(stderr, "%s: %s\n", programName, message);
    }
    if (appData.popupMoveErrors) {
	ErrorPopUp(_("Error"), message, FALSE);
    } else {
	DisplayMessage(message, "");
    }
}


void
DisplayFatalError (String message, int error, int status)
{
    char buf[MSG_SIZ];

    errorExitStatus = status;
    if (error == 0) {
	fprintf(stderr, "%s: %s\n", programName, message);
    } else {
	fprintf(stderr, "%s: %s: %s\n",
		programName, message, strerror(error));
	snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
	message = buf;
    }
    if (appData.popupExitMessage && boardWidget && XtIsRealized(boardWidget)) {
      ErrorPopUp(status ? _("Fatal Error") : _("Exiting"), message, TRUE);
    } else {
      ExitEvent(status);
    }
}

void
DisplayInformation (String message)
{
    ErrorPopDown();
    ErrorPopUp(_("Information"), message, TRUE);
}

void
DisplayNote (String message)
{
    ErrorPopDown();
    ErrorPopUp(_("Note"), message, FALSE);
}

static int
NullXErrorCheck (Display *dpy, XErrorEvent *error_event)
{
    return 0;
}

void
DisplayIcsInteractionTitle (String message)
{
  if (oldICSInteractionTitle == NULL) {
    /* Magic to find the old window title, adapted from vim */
    char *wina = getenv("WINDOWID");
    if (wina != NULL) {
      Window win = (Window) atoi(wina);
      Window root, parent, *children;
      unsigned int nchildren;
      int (*oldHandler)() = XSetErrorHandler(NullXErrorCheck);
      for (;;) {
	if (XFetchName(xDisplay, win, &oldICSInteractionTitle)) break;
	if (!XQueryTree(xDisplay, win, &root, &parent,
			&children, &nchildren)) break;
	if (children) XFree((void *)children);
	if (parent == root || parent == 0) break;
	win = parent;
      }
      XSetErrorHandler(oldHandler);
    }
    if (oldICSInteractionTitle == NULL) {
      oldICSInteractionTitle = "xterm";
    }
  }
  printf("\033]0;%s\007", message);
  fflush(stdout);
}

char pendingReplyPrefix[MSG_SIZ];
ProcRef pendingReplyPR;

void
AskQuestionProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    if (*nprms != 4) {
	fprintf(stderr, _("AskQuestionProc needed 4 parameters, got %d\n"),
		*nprms);
	return;
    }
    AskQuestionEvent(prms[0], prms[1], prms[2], prms[3]);
}

void
AskQuestionPopDown ()
{
    if (!askQuestionUp) return;
    XtPopdown(askQuestionShell);
    XtDestroyWidget(askQuestionShell);
    askQuestionUp = False;
}

void
AskQuestionReplyAction (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    char buf[MSG_SIZ];
    int err;
    String reply;

    reply = XawDialogGetValueString(w = XtParent(w));
    safeStrCpy(buf, pendingReplyPrefix, sizeof(buf)/sizeof(buf[0]) );
    if (*buf) strncat(buf, " ", MSG_SIZ - strlen(buf) - 1);
    strncat(buf, reply, MSG_SIZ - strlen(buf) - 1);
    strncat(buf, "\n",  MSG_SIZ - strlen(buf) - 1);
    OutputToProcess(pendingReplyPR, buf, strlen(buf), &err);
    AskQuestionPopDown();

    if (err) DisplayFatalError(_("Error writing to chess program"), err, 0);
}

void
AskQuestionCallback (Widget w, XtPointer client_data, XtPointer call_data)
{
    String name;
    Arg args[16];

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    if (strcmp(name, _("cancel")) == 0) {
        AskQuestionPopDown();
    } else {
	AskQuestionReplyAction(w, NULL, NULL, NULL);
    }
}

void
AskQuestion (char *title, char *question, char *replyPrefix, ProcRef pr)
{
    Arg args[16];
    Widget popup, layout, dialog, edit;
    Window root, child;
    int x, y, i;
    int win_x, win_y;
    unsigned int mask;

    safeStrCpy(pendingReplyPrefix, replyPrefix, sizeof(pendingReplyPrefix)/sizeof(pendingReplyPrefix[0]) );
    pendingReplyPR = pr;

    i = 0;
    XtSetArg(args[i], XtNresizable, True); i++;
    XtSetArg(args[i], XtNwidth, DIALOG_SIZE); i++;
    askQuestionShell = popup =
      XtCreatePopupShell(title, transientShellWidgetClass,
			 shellWidget, args, i);

    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, popup,
			    layoutArgs, XtNumber(layoutArgs));

    i = 0;
    XtSetArg(args[i], XtNlabel, question); i++;
    XtSetArg(args[i], XtNvalue, ""); i++;
    XtSetArg(args[i], XtNborderWidth, 0); i++;
    dialog = XtCreateManagedWidget("question", dialogWidgetClass,
				   layout, args, i);

    XawDialogAddButton(dialog, _("enter"), AskQuestionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("cancel"), AskQuestionCallback,
		       (XtPointer) dialog);

    XtRealizeWidget(popup);
    CatchDeleteWindow(popup, "AskQuestionPopDown");

    XQueryPointer(xDisplay, xBoardWindow, &root, &child,
		  &x, &y, &win_x, &win_y, &mask);

    XtSetArg(args[0], XtNx, x - 10);
    XtSetArg(args[1], XtNy, y - 30);
    XtSetValues(popup, args, 2);

    XtPopup(popup, XtGrabExclusive);
    askQuestionUp = True;

    edit = XtNameToWidget(dialog, "*value");
    XtSetKeyboardFocus(popup, edit);
}


void
PlaySound (char *name)
{
  if (*name == NULLCHAR) {
    return;
  } else if (strcmp(name, "$") == 0) {
    putc(BELLCHAR, stderr);
  } else {
    char buf[2048];
    char *prefix = "", *sep = "";
    if(appData.soundProgram[0] == NULLCHAR) return;
    if(!strchr(name, '/')) { prefix = appData.soundDirectory; sep = "/"; }
    snprintf(buf, sizeof(buf), "%s '%s%s%s' &", appData.soundProgram, prefix, sep, name);
    system(buf);
  }
}

void
RingBell ()
{
  PlaySound(appData.soundMove);
}

void
PlayIcsWinSound ()
{
  PlaySound(appData.soundIcsWin);
}

void
PlayIcsLossSound ()
{
  PlaySound(appData.soundIcsLoss);
}

void
PlayIcsDrawSound ()
{
  PlaySound(appData.soundIcsDraw);
}

void
PlayIcsUnfinishedSound ()
{
  PlaySound(appData.soundIcsUnfinished);
}

void
PlayAlarmSound ()
{
  PlaySound(appData.soundIcsAlarm);
}

void
PlayTellSound ()
{
  PlaySound(appData.soundTell);
}

void
EchoOn ()
{
    system("stty echo");
    noEcho = False;
}

void
EchoOff ()
{
    system("stty -echo");
    noEcho = True;
}

void
RunCommand (char *buf)
{
    system(buf);
}

void
Colorize (ColorClass cc, int continuation)
{
    char buf[MSG_SIZ];
    int count, outCount, error;

    if (textColors[(int)cc].bg > 0) {
	if (textColors[(int)cc].fg > 0) {
	  snprintf(buf, MSG_SIZ, "\033[0;%d;%d;%dm", textColors[(int)cc].attr,
		   textColors[(int)cc].fg, textColors[(int)cc].bg);
	} else {
	  snprintf(buf, MSG_SIZ, "\033[0;%d;%dm", textColors[(int)cc].attr,
		   textColors[(int)cc].bg);
	}
    } else {
	if (textColors[(int)cc].fg > 0) {
	  snprintf(buf, MSG_SIZ, "\033[0;%d;%dm", textColors[(int)cc].attr,
		    textColors[(int)cc].fg);
	} else {
	  snprintf(buf, MSG_SIZ, "\033[0;%dm", textColors[(int)cc].attr);
	}
    }
    count = strlen(buf);
    outCount = OutputToProcess(NoProc, buf, count, &error);
    if (outCount < count) {
	DisplayFatalError(_("Error writing to display"), error, 1);
    }

    if (continuation) return;
    switch (cc) {
    case ColorShout:
      PlaySound(appData.soundShout);
      break;
    case ColorSShout:
      PlaySound(appData.soundSShout);
      break;
    case ColorChannel1:
      PlaySound(appData.soundChannel1);
      break;
    case ColorChannel:
      PlaySound(appData.soundChannel);
      break;
    case ColorKibitz:
      PlaySound(appData.soundKibitz);
      break;
    case ColorTell:
      PlaySound(appData.soundTell);
      break;
    case ColorChallenge:
      PlaySound(appData.soundChallenge);
      break;
    case ColorRequest:
      PlaySound(appData.soundRequest);
      break;
    case ColorSeek:
      PlaySound(appData.soundSeek);
      break;
    case ColorNormal:
    case ColorNone:
    default:
      break;
    }
}

char *
UserName ()
{
    return getpwuid(getuid())->pw_name;
}

static char *
ExpandPathName (char *path)
{
    static char static_buf[4*MSG_SIZ];
    char *d, *s, buf[4*MSG_SIZ];
    struct passwd *pwd;

    s = path;
    d = static_buf;

    while (*s && isspace(*s))
      ++s;

    if (!*s) {
	*d = 0;
	return static_buf;
    }

    if (*s == '~') {
	if (*(s+1) == '/') {
	  safeStrCpy(d, getpwuid(getuid())->pw_dir, 4*MSG_SIZ );
	  strcat(d, s+1);
	}
	else {
	  safeStrCpy(buf, s+1, sizeof(buf)/sizeof(buf[0]) );
	  { char *p; if(p = strchr(buf, '/')) *p = 0; }
	  pwd = getpwnam(buf);
	  if (!pwd)
	    {
	      fprintf(stderr, _("ERROR: Unknown user %s (in path %s)\n"),
		      buf, path);
	      return NULL;
	    }
	  safeStrCpy(d, pwd->pw_dir, 4*MSG_SIZ );
	  strcat(d, strchr(s+1, '/'));
	}
    }
    else
      safeStrCpy(d, s, 4*MSG_SIZ );

    return static_buf;
}

char *
HostName ()
{
    static char host_name[MSG_SIZ];

#if HAVE_GETHOSTNAME
    gethostname(host_name, MSG_SIZ);
    return host_name;
#else  /* not HAVE_GETHOSTNAME */
# if HAVE_SYSINFO && HAVE_SYS_SYSTEMINFO_H
    sysinfo(SI_HOSTNAME, host_name, MSG_SIZ);
    return host_name;
# else /* not (HAVE_SYSINFO && HAVE_SYS_SYSTEMINFO_H) */
    return "localhost";
# endif/* not (HAVE_SYSINFO && HAVE_SYS_SYSTEMINFO_H) */
#endif /* not HAVE_GETHOSTNAME */
}

XtIntervalId delayedEventTimerXID = 0;
DelayedEventCallback delayedEventCallback = 0;

void
FireDelayedEvent ()
{
    delayedEventTimerXID = 0;
    delayedEventCallback();
}

void
ScheduleDelayedEvent (DelayedEventCallback cb, long millisec)
{
    if(delayedEventTimerXID && delayedEventCallback == cb)
	// [HGM] alive: replace, rather than add or flush identical event
	XtRemoveTimeOut(delayedEventTimerXID);
    delayedEventCallback = cb;
    delayedEventTimerXID =
      XtAppAddTimeOut(appContext, millisec,
		      (XtTimerCallbackProc) FireDelayedEvent, (XtPointer) 0);
}

DelayedEventCallback
GetDelayedEvent ()
{
  if (delayedEventTimerXID) {
    return delayedEventCallback;
  } else {
    return NULL;
  }
}

void
CancelDelayedEvent ()
{
  if (delayedEventTimerXID) {
    XtRemoveTimeOut(delayedEventTimerXID);
    delayedEventTimerXID = 0;
  }
}

XtIntervalId loadGameTimerXID = 0;

int
LoadGameTimerRunning ()
{
    return loadGameTimerXID != 0;
}

int
StopLoadGameTimer ()
{
    if (loadGameTimerXID != 0) {
	XtRemoveTimeOut(loadGameTimerXID);
	loadGameTimerXID = 0;
	return TRUE;
    } else {
	return FALSE;
    }
}

void
LoadGameTimerCallback (XtPointer arg, XtIntervalId *id)
{
    loadGameTimerXID = 0;
    AutoPlayGameLoop();
}

void
StartLoadGameTimer (long millisec)
{
    loadGameTimerXID =
      XtAppAddTimeOut(appContext, millisec,
		      (XtTimerCallbackProc) LoadGameTimerCallback,
		      (XtPointer) 0);
}

XtIntervalId analysisClockXID = 0;

void
AnalysisClockCallback (XtPointer arg, XtIntervalId *id)
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile
         || appData.icsEngineAnalyze) { // [DM]
	AnalysisPeriodicEvent(0);
	StartAnalysisClock();
    }
}

void
StartAnalysisClock ()
{
    analysisClockXID =
      XtAppAddTimeOut(appContext, 2000,
		      (XtTimerCallbackProc) AnalysisClockCallback,
		      (XtPointer) 0);
}

XtIntervalId clockTimerXID = 0;

int
ClockTimerRunning ()
{
    return clockTimerXID != 0;
}

int
StopClockTimer ()
{
    if (clockTimerXID != 0) {
	XtRemoveTimeOut(clockTimerXID);
	clockTimerXID = 0;
	return TRUE;
    } else {
	return FALSE;
    }
}

void
ClockTimerCallback (XtPointer arg, XtIntervalId *id)
{
    clockTimerXID = 0;
    DecrementClocks();
}

void
StartClockTimer (long millisec)
{
    clockTimerXID =
      XtAppAddTimeOut(appContext, millisec,
		      (XtTimerCallbackProc) ClockTimerCallback,
		      (XtPointer) 0);
}

void
DisplayTimerLabel (Widget w, char *color, long timer, int highlight)
{
    char buf[MSG_SIZ];
    Arg args[16];

    /* check for low time warning */
    Pixel foregroundOrWarningColor = timerForegroundPixel;

    if (timer > 0 &&
        appData.lowTimeWarning &&
        (timer / 1000) < appData.icsAlarmTime)
      foregroundOrWarningColor = lowTimeWarningColor;

    if (appData.clockMode) {
      snprintf(buf, MSG_SIZ, "%s: %s", color, TimeString(timer));
      XtSetArg(args[0], XtNlabel, buf);
    } else {
      snprintf(buf, MSG_SIZ, "%s  ", color);
      XtSetArg(args[0], XtNlabel, buf);
    }

    if (highlight) {

	XtSetArg(args[1], XtNbackground, foregroundOrWarningColor);
	XtSetArg(args[2], XtNforeground, timerBackgroundPixel);
    } else {
	XtSetArg(args[1], XtNbackground, timerBackgroundPixel);
	XtSetArg(args[2], XtNforeground, foregroundOrWarningColor);
    }

    XtSetValues(w, args, 3);
}

void
DisplayWhiteClock (long timeRemaining, int highlight)
{
    Arg args[16];

    if(appData.noGUI) return;
    DisplayTimerLabel(whiteTimerWidget, _("White"), timeRemaining, highlight);
    if (highlight && iconPixmap == bIconPixmap) {
	iconPixmap = wIconPixmap;
	XtSetArg(args[0], XtNiconPixmap, iconPixmap);
	XtSetValues(shellWidget, args, 1);
    }
}

void
DisplayBlackClock (long timeRemaining, int highlight)
{
    Arg args[16];

    if(appData.noGUI) return;
    DisplayTimerLabel(blackTimerWidget, _("Black"), timeRemaining, highlight);
    if (highlight && iconPixmap == wIconPixmap) {
	iconPixmap = bIconPixmap;
	XtSetArg(args[0], XtNiconPixmap, iconPixmap);
	XtSetValues(shellWidget, args, 1);
    }
}

#define CPNone 0
#define CPReal 1
#define CPComm 2
#define CPSock 3
#define CPLoop 4
typedef int CPKind;

typedef struct {
    CPKind kind;
    int pid;
    int fdTo, fdFrom;
} ChildProc;


int
StartChildProcess (char *cmdLine, char *dir, ProcRef *pr)
{
    char *argv[64], *p;
    int i, pid;
    int to_prog[2], from_prog[2];
    ChildProc *cp;
    char buf[MSG_SIZ];

    if (appData.debugMode) {
	fprintf(debugFP, "StartChildProcess (dir=\"%s\") %s\n",dir, cmdLine);
    }

    /* We do NOT feed the cmdLine to the shell; we just
       parse it into blank-separated arguments in the
       most simple-minded way possible.
       */
    i = 0;
    safeStrCpy(buf, cmdLine, sizeof(buf)/sizeof(buf[0]) );
    p = buf;
    for (;;) {
	while(*p == ' ') p++;
	argv[i++] = p;
	if(*p == '"' || *p == '\'')
	     p = strchr(++argv[i-1], *p);
	else p = strchr(p, ' ');
	if (p == NULL) break;
	*p++ = NULLCHAR;
    }
    argv[i] = NULL;

    SetUpChildIO(to_prog, from_prog);

    if ((pid = fork()) == 0) {
	/* Child process */
	// [HGM] PSWBTM: made order resistant against case where fd of created pipe was 0 or 1
	close(to_prog[1]);     // first close the unused pipe ends
	close(from_prog[0]);
	dup2(to_prog[0], 0);   // to_prog was created first, nd is the only one to use 0 or 1
	dup2(from_prog[1], 1);
	if(to_prog[0] >= 2) close(to_prog[0]); // if 0 or 1, the dup2 already cosed the original
	close(from_prog[1]);                   // and closing again loses one of the pipes!
	if(fileno(stderr) >= 2) // better safe than sorry...
		dup2(1, fileno(stderr)); /* force stderr to the pipe */

	if (dir[0] != NULLCHAR && chdir(dir) != 0) {
	    perror(dir);
	    exit(1);
	}

	nice(appData.niceEngines); // [HGM] nice: adjust priority of engine proc

        execvp(argv[0], argv);

	/* If we get here, exec failed */
	perror(argv[0]);
	exit(1);
    }

    /* Parent process */
    close(to_prog[0]);
    close(from_prog[1]);

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPReal;
    cp->pid = pid;
    cp->fdFrom = from_prog[0];
    cp->fdTo = to_prog[1];
    *pr = (ProcRef) cp;
    return 0;
}

// [HGM] kill: implement the 'hard killing' of AS's Winboard_x
static RETSIGTYPE
AlarmCallBack (int n)
{
    return;
}

void
DestroyChildProcess (ProcRef pr, int signalType)
{
    ChildProc *cp = (ChildProc *) pr;

    if (cp->kind != CPReal) return;
    cp->kind = CPNone;
    if (signalType == 10) { // [HGM] kill: if it does not terminate in 3 sec, kill
	signal(SIGALRM, AlarmCallBack);
	alarm(3);
	if(wait((int *) 0) == -1) { // process does not terminate on its own accord
	    kill(cp->pid, SIGKILL); // kill it forcefully
	    wait((int *) 0);        // and wait again
	}
    } else {
	if (signalType) {
	    kill(cp->pid, signalType == 9 ? SIGKILL : SIGTERM); // [HGM] kill: use hard kill if so requested
	}
	/* Process is exiting either because of the kill or because of
	   a quit command sent by the backend; either way, wait for it to die.
	*/
	wait((int *) 0);
    }
    close(cp->fdFrom);
    close(cp->fdTo);
}

void
InterruptChildProcess (ProcRef pr)
{
    ChildProc *cp = (ChildProc *) pr;

    if (cp->kind != CPReal) return;
    (void) kill(cp->pid, SIGINT); /* stop it thinking */
}

int
OpenTelnet (char *host, char *port, ProcRef *pr)
{
    char cmdLine[MSG_SIZ];

    if (port[0] == NULLCHAR) {
      snprintf(cmdLine, sizeof(cmdLine), "%s %s", appData.telnetProgram, host);
    } else {
      snprintf(cmdLine, sizeof(cmdLine), "%s %s %s", appData.telnetProgram, host, port);
    }
    return StartChildProcess(cmdLine, "", pr);
}

int
OpenTCP (char *host, char *port, ProcRef *pr)
{
#if OMIT_SOCKETS
    DisplayFatalError(_("Socket support is not configured in"), 0, 2);
#else  /* !OMIT_SOCKETS */
    struct addrinfo hints;
    struct addrinfo *ais, *ai;
    int error;
    int s=0;
    ChildProc *cp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    error = getaddrinfo(host, port, &hints, &ais);
    if (error != 0) {
      /* a getaddrinfo error is not an errno, so can't return it */
      fprintf(debugFP, "getaddrinfo(%s, %s): %s\n",
	      host, port, gai_strerror(error));
      return ENOENT;
    }
     
    for (ai = ais; ai != NULL; ai = ai->ai_next) {
      if ((s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) {
	error = errno;
	continue;
      }
      if (connect(s, ai->ai_addr, ai->ai_addrlen) < 0) {
	error = errno;
	continue;
      }
      error = 0;
      break;
    }
    freeaddrinfo(ais);

    if (error != 0) {
      return error;
    }

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPSock;
    cp->pid = 0;
    cp->fdFrom = s;
    cp->fdTo = s;
    *pr = (ProcRef) cp;
#endif /* !OMIT_SOCKETS */

    return 0;
}

int
OpenCommPort (char *name, ProcRef *pr)
{
    int fd;
    ChildProc *cp;

    fd = open(name, 2, 0);
    if (fd < 0) return errno;

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPComm;
    cp->pid = 0;
    cp->fdFrom = fd;
    cp->fdTo = fd;
    *pr = (ProcRef) cp;

    return 0;
}

int
OpenLoopback (ProcRef *pr)
{
    ChildProc *cp;
    int to[2], from[2];

    SetUpChildIO(to, from);

    cp = (ChildProc *) calloc(1, sizeof(ChildProc));
    cp->kind = CPLoop;
    cp->pid = 0;
    cp->fdFrom = to[0];		/* note not from[0]; we are doing a loopback */
    cp->fdTo = to[1];
    *pr = (ProcRef) cp;

    return 0;
}

int
OpenRcmd (char *host, char *user, char *cmd, ProcRef *pr)
{
    DisplayFatalError(_("internal rcmd not implemented for Unix"), 0, 1);
    return -1;
}

#define INPUT_SOURCE_BUF_SIZE 8192

typedef struct {
    CPKind kind;
    int fd;
    int lineByLine;
    char *unused;
    InputCallback func;
    XtInputId xid;
    char buf[INPUT_SOURCE_BUF_SIZE];
    VOIDSTAR closure;
} InputSource;

void
DoInputCallback (caddr_t closure, int *source, XtInputId *xid)
{
    InputSource *is = (InputSource *) closure;
    int count;
    int error;
    char *p, *q;

    if (is->lineByLine) {
	count = read(is->fd, is->unused,
		     INPUT_SOURCE_BUF_SIZE - (is->unused - is->buf));
	if (count <= 0) {
	    (is->func)(is, is->closure, is->buf, count, count ? errno : 0);
	    return;
	}
	is->unused += count;
	p = is->buf;
	while (p < is->unused) {
	    q = memchr(p, '\n', is->unused - p);
	    if (q == NULL) break;
	    q++;
	    (is->func)(is, is->closure, p, q - p, 0);
	    p = q;
	}
	q = is->buf;
	while (p < is->unused) {
	    *q++ = *p++;
	}
	is->unused = q;
    } else {
	count = read(is->fd, is->buf, INPUT_SOURCE_BUF_SIZE);
	if (count == -1)
	  error = errno;
	else
	  error = 0;
	(is->func)(is, is->closure, is->buf, count, error);
    }
}

InputSourceRef
AddInputSource (ProcRef pr, int lineByLine, InputCallback func, VOIDSTAR closure)
{
    InputSource *is;
    ChildProc *cp = (ChildProc *) pr;

    is = (InputSource *) calloc(1, sizeof(InputSource));
    is->lineByLine = lineByLine;
    is->func = func;
    if (pr == NoProc) {
	is->kind = CPReal;
	is->fd = fileno(stdin);
    } else {
	is->kind = cp->kind;
	is->fd = cp->fdFrom;
    }
    if (lineByLine) {
	is->unused = is->buf;
    }

    is->xid = XtAppAddInput(appContext, is->fd,
			    (XtPointer) (XtInputReadMask),
			    (XtInputCallbackProc) DoInputCallback,
			    (XtPointer) is);
    is->closure = closure;
    return (InputSourceRef) is;
}

void
RemoveInputSource (InputSourceRef isr)
{
    InputSource *is = (InputSource *) isr;

    if (is->xid == 0) return;
    XtRemoveInput(is->xid);
    is->xid = 0;
}

int
OutputToProcess (ProcRef pr, char *message, int count, int *outError)
{
    static int line = 0;
    ChildProc *cp = (ChildProc *) pr;
    int outCount;

    if (pr == NoProc)
    {
        if (appData.noJoin || !appData.useInternalWrap)
            outCount = fwrite(message, 1, count, stdout);
        else
        {
            int width = get_term_width();
            int len = wrap(NULL, message, count, width, &line);
            char *msg = malloc(len);
            int dbgchk;

            if (!msg)
                outCount = fwrite(message, 1, count, stdout);
            else
            {
                dbgchk = wrap(msg, message, count, width, &line);
                if (dbgchk != len && appData.debugMode)
                    fprintf(debugFP, "wrap(): dbgchk(%d) != len(%d)\n", dbgchk, len);
                outCount = fwrite(msg, 1, dbgchk, stdout);
                free(msg);
            }
        }
    }
    else
      outCount = write(cp->fdTo, message, count);

    if (outCount == -1)
      *outError = errno;
    else
      *outError = 0;

    return outCount;
}

/* Output message to process, with "ms" milliseconds of delay
   between each character. This is needed when sending the logon
   script to ICC, which for some reason doesn't like the
   instantaneous send. */
int
OutputToProcessDelayed (ProcRef pr, char *message, int count, int *outError, long msdelay)
{
    ChildProc *cp = (ChildProc *) pr;
    int outCount = 0;
    int r;

    while (count--) {
	r = write(cp->fdTo, message++, 1);
	if (r == -1) {
	    *outError = errno;
	    return outCount;
	}
	++outCount;
	if (msdelay >= 0)
	  TimeDelay(msdelay);
    }

    return outCount;
}

/****	Animation code by Hugh Fisher, DCS, ANU. ****/

/*	Masks for XPM pieces. Black and white pieces can have
	different shapes, but in the interest of retaining my
	sanity pieces must have the same outline on both light
	and dark squares, and all pieces must use the same
	background square colors/images.		*/

static int xpmDone = 0;
static Pixmap animBufs[3*NrOfAnims]; // newBuf, saveBuf
static GC animGCs[3*NrOfAnims]; // blitGC, pieceGC, outlineGC;

static void
CreateAnimMasks (int pieceDepth)
{
  ChessSquare   piece;
  Pixmap	buf;
  GC		bufGC, maskGC;
  int		kind, n;
  unsigned long	plane;
  XGCValues	values;

  /* Need a bitmap just to get a GC with right depth */
  buf = XCreatePixmap(xDisplay, xBoardWindow,
			8, 8, 1);
  values.foreground = 1;
  values.background = 0;
  /* Don't use XtGetGC, not read only */
  maskGC = XCreateGC(xDisplay, buf,
		    GCForeground | GCBackground, &values);
  XFreePixmap(xDisplay, buf);

  buf = XCreatePixmap(xDisplay, xBoardWindow,
		      squareSize, squareSize, pieceDepth);
  values.foreground = XBlackPixel(xDisplay, xScreen);
  values.background = XWhitePixel(xDisplay, xScreen);
  bufGC = XCreateGC(xDisplay, buf,
		    GCForeground | GCBackground, &values);

  for (piece = WhitePawn; piece <= BlackKing; piece++) {
    /* Begin with empty mask */
    if(!xpmDone) // [HGM] pieces: keep using existing
    xpmMask[piece] = XCreatePixmap(xDisplay, xBoardWindow,
				 squareSize, squareSize, 1);
    XSetFunction(xDisplay, maskGC, GXclear);
    XFillRectangle(xDisplay, xpmMask[piece], maskGC,
		   0, 0, squareSize, squareSize);

    /* Take a copy of the piece */
    if (White(piece))
      kind = 0;
    else
      kind = 2;
    XSetFunction(xDisplay, bufGC, GXcopy);
    XCopyArea(xDisplay, xpmPieceBitmap[kind][((int)piece) % (int)BlackPawn],
	      buf, bufGC,
	      0, 0, squareSize, squareSize, 0, 0);

    /* XOR the background (light) over the piece */
    XSetFunction(xDisplay, bufGC, GXxor);
    if (useImageSqs)
      XCopyArea(xDisplay, xpmLightSquare, buf, bufGC,
		0, 0, squareSize, squareSize, 0, 0);
    else {
      XSetForeground(xDisplay, bufGC, lightSquareColor);
      XFillRectangle(xDisplay, buf, bufGC, 0, 0, squareSize, squareSize);
    }

    /* We now have an inverted piece image with the background
       erased. Construct mask by just selecting all the non-zero
       pixels - no need to reconstruct the original image.	*/
    XSetFunction(xDisplay, maskGC, GXor);
    plane = 1;
    /* Might be quicker to download an XImage and create bitmap
       data from it rather than this N copies per piece, but it
       only takes a fraction of a second and there is a much
       longer delay for loading the pieces.	   	*/
    for (n = 0; n < pieceDepth; n ++) {
      XCopyPlane(xDisplay, buf, xpmMask[piece], maskGC,
		 0, 0, squareSize, squareSize,
		 0, 0, plane);
      plane = plane << 1;
    }
  }
  /* Clean up */
  XFreePixmap(xDisplay, buf);
  XFreeGC(xDisplay, bufGC);
  XFreeGC(xDisplay, maskGC);
}

static void
InitAnimState (AnimNr anr, XWindowAttributes *info)
{
  XtGCMask  mask;
  XGCValues values;

  /* Each buffer is square size, same depth as window */
  animBufs[anr+4] = xBoardWindow;
  animBufs[anr+2] = XCreatePixmap(xDisplay, xBoardWindow,
			squareSize, squareSize, info->depth);
  animBufs[anr] = XCreatePixmap(xDisplay, xBoardWindow,
			squareSize, squareSize, info->depth);

  /* Create a plain GC for blitting */
  mask = GCForeground | GCBackground | GCFunction |
         GCPlaneMask | GCGraphicsExposures;
  values.foreground = XBlackPixel(xDisplay, xScreen);
  values.background = XWhitePixel(xDisplay, xScreen);
  values.function   = GXcopy;
  values.plane_mask = AllPlanes;
  values.graphics_exposures = False;
  animGCs[anr] = XCreateGC(xDisplay, xBoardWindow, mask, &values);

  /* Piece will be copied from an existing context at
     the start of each new animation/drag. */
  animGCs[anr+2] = XCreateGC(xDisplay, xBoardWindow, 0, &values);

  /* Outline will be a read-only copy of an existing */
  animGCs[anr+4] = None;
}

void
CreateAnimVars ()
{
  XWindowAttributes info;

  if (xpmDone && gameInfo.variant == oldVariant) return;
  if(xpmDone) oldVariant = gameInfo.variant; // first time pieces might not be created yet
  XGetWindowAttributes(xDisplay, xBoardWindow, &info);

  InitAnimState(Game, &info);
  InitAnimState(Player, &info);

  /* For XPM pieces, we need bitmaps to use as masks. */
  if (useImages)
    CreateAnimMasks(info.depth), xpmDone = 1;
}

#ifndef HAVE_USLEEP

static Boolean frameWaiting;

static RETSIGTYPE
FrameAlarm (int sig)
{
  frameWaiting = False;
  /* In case System-V style signals.  Needed?? */
  signal(SIGALRM, FrameAlarm);
}

void
FrameDelay (int time)
{
  struct itimerval delay;

  XSync(xDisplay, False);

  if (time > 0) {
    frameWaiting = True;
    signal(SIGALRM, FrameAlarm);
    delay.it_interval.tv_sec =
      delay.it_value.tv_sec = time / 1000;
    delay.it_interval.tv_usec =
      delay.it_value.tv_usec = (time % 1000) * 1000;
    setitimer(ITIMER_REAL, &delay, NULL);
    while (frameWaiting) pause();
    delay.it_interval.tv_sec = delay.it_value.tv_sec = 0;
    delay.it_interval.tv_usec = delay.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &delay, NULL);
  }
}

#else

void
FrameDelay (int time)
{
  XSync(xDisplay, False);
  if (time > 0)
    usleep(time * 1000);
}

#endif

static void
SelectGCMask (ChessSquare piece, GC *clip, GC *outline, Pixmap *mask)
{
  GC source;

  /* Bitmap for piece being moved. */
  if (appData.monoMode) {
      *mask = *pieceToSolid(piece);
  } else if (useImages) {
#if HAVE_LIBXPM
      *mask = xpmMask[piece];
#else
      *mask = ximMaskPm[piece];
#endif
  } else {
      *mask = *pieceToSolid(piece);
  }

  /* GC for piece being moved. Square color doesn't matter, but
     since it gets modified we make a copy of the original. */
  if (White(piece)) {
    if (appData.monoMode)
      source = bwPieceGC;
    else
      source = wlPieceGC;
  } else {
    if (appData.monoMode)
      source = wbPieceGC;
    else
      source = blPieceGC;
  }
  XCopyGC(xDisplay, source, 0xFFFFFFFF, *clip);

  /* Outline only used in mono mode and is not modified */
  if (White(piece))
    *outline = bwPieceGC;
  else
    *outline = wbPieceGC;
}

static void
OverlayPiece (ChessSquare piece, GC clip, GC outline,  Drawable dest)
{
  int	kind;

  if (!useImages) {
    /* Draw solid rectangle which will be clipped to shape of piece */
    XFillRectangle(xDisplay, dest, clip,
		   0, 0, squareSize, squareSize);
    if (appData.monoMode)
      /* Also draw outline in contrasting color for black
	 on black / white on white cases		*/
      XCopyPlane(xDisplay, *pieceToOutline(piece), dest, outline,
		 0, 0, squareSize, squareSize, 0, 0, 1);
  } else {
    /* Copy the piece */
    if (White(piece))
      kind = 0;
    else
      kind = 2;
    if(appData.upsideDown && flipView) kind ^= 2;
    XCopyArea(xDisplay, xpmPieceBitmap[kind][piece],
	      dest, clip,
	      0, 0, squareSize, squareSize,
	      0, 0);
  }
}

void
InsertPiece (AnimNr anr, ChessSquare piece)
{
  OverlayPiece(piece, animGCs[anr+2], animGCs[anr+4], animBufs[anr]);
}

void
DrawBlank (AnimNr anr, int x, int y, int startColor)
{
    BlankSquare(x, y, startColor, EmptySquare, animBufs[anr+2], 0);
}

void CopyRectangle (AnimNr anr, int srcBuf, int destBuf,
		 int srcX, int srcY, int width, int height, int destX, int destY)
{
    XCopyArea(xDisplay, animBufs[anr+srcBuf], animBufs[anr+destBuf], animGCs[anr],
		srcX, srcY, width, height, destX, destY);
}

void
SetDragPiece (AnimNr anr, ChessSquare piece)
{
  Pixmap mask;
  /* The piece will be drawn using its own bitmap as a matte	*/
  SelectGCMask(piece, &animGCs[anr+2], &animGCs[anr+4], &mask);
  XSetClipMask(xDisplay, animGCs[anr+2], mask);
}

#include <sys/ioctl.h>
int
get_term_width ()
{
    int fd, default_width;

    fd = STDIN_FILENO;
    default_width = 79; // this is FICS default anyway...

#if !defined(TIOCGWINSZ) && defined(TIOCGSIZE)
    struct ttysize win;
    if (!ioctl(fd, TIOCGSIZE, &win))
        default_width = win.ts_cols;
#elif defined(TIOCGWINSZ)
    struct winsize win;
    if (!ioctl(fd, TIOCGWINSZ, &win))
        default_width = win.ws_col;
#endif
    return default_width;
}

void
update_ics_width ()
{
  static int old_width = 0;
  int new_width = get_term_width();

  if (old_width != new_width)
    ics_printf("set width %d\n", new_width);
  old_width = new_width;
}

void
NotifyFrontendLogin ()
{
    update_ics_width();
}

/* [AS] Arrow highlighting support */

void
DrawPolygon (Pnt arrow[], int nr)
{
    XPoint pts[10];
    int i;
    for(i=0; i<10; i++) pts[i].x = arrow[i].x, pts[i].y = arrow[i].y;
    XFillPolygon(xDisplay, xBoardWindow, highlineGC, pts, nr, Nonconvex, CoordModeOrigin);
    if(appData.monoMode) arrow[nr] = arrow[0], XDrawLines(xDisplay, xBoardWindow, darkSquareGC, pts, nr+1, CoordModeOrigin);
}

void
UpdateLogos (int displ)
{
    return; // no logos in XBoard yet
}

