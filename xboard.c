/*
 * xboard.c -- X front end for XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include "gettext.h"

// must be moved to xengineoutput.h

void EngineOutputProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void EvalGraphProc P((Widget w, XEvent *event,
		      String *prms, Cardinal *nprms));


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

typedef struct {
    String string;
    String ref;
    XtActionProc proc;
} MenuItem;

typedef struct {
    String name;
    String ref;
    MenuItem *mi;
} Menu;

int main P((int argc, char **argv));
FILE * XsraSelFile P((Widget w, char *prompt, char *ok, char *cancel, char *failed,
		char *init_path, char *filter, char *mode, int (*show_entry)(), char **name_return));
RETSIGTYPE CmailSigHandler P((int sig));
RETSIGTYPE IntSigHandler P((int sig));
RETSIGTYPE TermSizeSigHandler P((int sig));
void CreateGCs P((int redo));
void CreateAnyPieces P((void));
void CreateXIMPieces P((void));
void CreateXPMPieces P((void));
void CreateXPMBoard P((char *s, int n));
void CreatePieces P((void));
void CreatePieceMenus P((void));
Widget CreateMenuBar P((Menu *mb));
Widget CreateButtonBar P ((MenuItem *mi));
char *FindFont P((char *pattern, int targetPxlSize));
void PieceMenuPopup P((Widget w, XEvent *event,
		       String *params, Cardinal *num_params));
static void PieceMenuSelect P((Widget w, ChessSquare piece, caddr_t junk));
static void DropMenuSelect P((Widget w, ChessSquare piece, caddr_t junk));
void ReadBitmap P((Pixmap *pm, String name, unsigned char bits[],
		   u_int wreq, u_int hreq));
void CreateGrid P((void));
int EventToSquare P((int x, int limit));
void DrawSquare P((int row, int column, ChessSquare piece, int do_flash));
void EventProc P((Widget widget, caddr_t unused, XEvent *event));
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
void CommentCallback P((Widget w, XtPointer client_data,
			XtPointer call_data));
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
void EditCommentPopDown P((void));
void EditCommentCallback P((Widget w, XtPointer client_data,
			    XtPointer call_data));
void SelectCommand P((Widget w, XtPointer client_data, XtPointer call_data));
void ResetProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void LoadGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void LoadNextGameProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void LoadPrevGameProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void ReloadGameProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void LoadPositionProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void LoadNextPositionProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void LoadPrevPositionProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void ReloadPositionProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void CopyPositionProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void PastePositionProc P((Widget w, XEvent *event, String *prms,
			  Cardinal *nprms));
void CopyGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void PasteGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SaveGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SavePositionProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void MailMoveProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ReloadCmailMsgProc P((Widget w, XEvent *event, String *prms,
			    Cardinal *nprms));
void QuitProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void PauseProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void MachineBlackProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void MachineWhiteProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void AnalyzeModeProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void AnalyzeFileProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void TwoMachinesProc P((Widget w, XEvent *event, String *prms,
			Cardinal *nprms));
void MatchProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void MatchOptionsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void IcsClientProc P((Widget w, XEvent *event, String *prms,
		      Cardinal *nprms));
void EditGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EditPositionProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void TrainingProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EditCommentProc P((Widget w, XEvent *event,
			String *prms, Cardinal *nprms));
void IcsInputBoxProc P((Widget w, XEvent *event,
			String *prms, Cardinal *nprms));
void AcceptProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DeclineProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void RematchProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void CallFlagProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DrawProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AbortProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AdjournProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ResignProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AdjuWhiteProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AdjuBlackProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AdjuDrawProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EnterKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void UpKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DownKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void StopObservingProc P((Widget w, XEvent *event, String *prms,
			  Cardinal *nprms));
void StopExaminingProc P((Widget w, XEvent *event, String *prms,
			  Cardinal *nprms));
void UploadProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void BackwardProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ForwardProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ToStartProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ToEndProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void RevertProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AnnotateProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TruncateGameProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void RetractMoveProc P((Widget w, XEvent *event, String *prms,
			Cardinal *nprms));
void MoveNowProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AlwaysQueenProc P((Widget w, XEvent *event, String *prms,
			Cardinal *nprms));
void AnimateDraggingProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void AnimateMovingProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void AutoflagProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AutoflipProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void BlindfoldProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void FlashMovesProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void FlipViewProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void HighlightDraggingProc P((Widget w, XEvent *event, String *prms,
			      Cardinal *nprms));
void HighlightLastMoveProc P((Widget w, XEvent *event, String *prms,
			      Cardinal *nprms));
void HighlightArrowProc P((Widget w, XEvent *event, String *prms,
			      Cardinal *nprms));
void MoveSoundProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
//void IcsAlarmProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void OneClickProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void PeriodicUpdatesProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void PonderNextMoveProc P((Widget w, XEvent *event, String *prms,
			   Cardinal *nprms));
void PopupMoveErrorsProc P((Widget w, XEvent *event, String *prms,
			Cardinal *nprms));
void PopupExitMessageProc P((Widget w, XEvent *event, String *prms,
			     Cardinal *nprms));
//void PremoveProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ShowCoordsProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void ShowThinkingProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void HideThinkingProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void TestLegalityProc P((Widget w, XEvent *event, String *prms,
			  Cardinal *nprms));
void SaveSettingsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SaveOnExitProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void InfoProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void ManProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void HintProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void BookProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AboutGameProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void AboutProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DebugProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void NothingProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void Iconify P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DisplayMove P((int moveNumber));
void DisplayTitle P((char *title));
void ICSInitScript P((void));
int LoadGamePopUp P((FILE *f, int gameNumber, char *title));
void ErrorPopUp P((char *title, char *text, int modal));
void ErrorPopDown P((void));
static char *ExpandPathName P((char *path));
static void CreateAnimVars P((void));
static void DragPieceMove P((int x, int y));
static void DrawDragPiece P((void));
char *ModeToWidgetName P((GameMode mode));
void ShuffleMenuProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EngineMenuProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void UciMenuProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TimeControlProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void OptionsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void NewVariantProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void IcsTextProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void FirstSettingsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SecondSettingsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void GameListOptionsPopUp P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void IcsOptionsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SoundOptionsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void BoardOptionsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void LoadOptionsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void SaveOptionsProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void GameListOptionsPopDown P(());
void ShufflePopDown P(());
void TimeControlPopDown P(());
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
int hOffset; // [HGM] dual
XSegment secondSegments[BOARD_RANKS + BOARD_FILES + 2];
XSegment gridSegments[BOARD_RANKS + BOARD_FILES + 2];
XSegment jailGridSegments[BOARD_RANKS + BOARD_FILES + 6];
Font clockFontID, coordFontID, countFontID;
XFontStruct *clockFontStruct, *coordFontStruct, *countFontStruct;
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
int squareSize, smallLayout = 0, tinyLayout = 0,
  marginW, marginH, // [HGM] for run-time resizing
  fromX = -1, fromY = -1, toX, toY, commentUp = False, analysisUp = False,
  ICSInputBoxUp = False, askQuestionUp = False,
  filenameUp = False, promotionUp = False, pmFromX = -1, pmFromY = -1,
  editUp = False, errorUp = False, errorExitStatus = -1, lineGap, defaultLineGap;
Pixel timerForegroundPixel, timerBackgroundPixel;
Pixel buttonForegroundPixel, buttonBackgroundPixel;
char *chessDir, *programName, *programVersion,
  *gameCopyFilename, *gamePasteFilename;
Boolean alwaysOnTop = False;
Boolean saveSettingsOnExit;
char *settingsFileName;
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

/* Variables for doing smooth animation. This whole thing
   would be much easier if the board was double-buffered,
   but that would require a fairly major rewrite.	*/

typedef struct {
	Pixmap  saveBuf;
	Pixmap	newBuf;
	GC	blitGC, pieceGC, outlineGC;
	XPoint	startSquare, prevFrame, mouseDelta;
	int	startColor;
	int	dragPiece;
	Boolean	dragActive;
        int     startBoardX, startBoardY;
    } AnimState;

/* There can be two pieces being animated at once: a player
   can begin dragging a piece before the remote opponent has moved. */

static AnimState game, player;

/* Bitmaps for use as masks when drawing XPM pieces.
   Need one for each black and white piece.		*/
static Pixmap xpmMask[BlackKing + 1];

/* This magic number is the number of intermediate frames used
   in each half of the animation. For short moves it's reduced
   by 1. The total number of frames will be factor * 2 + 1.  */
#define kFactor	   4

SizeDefaults sizeDefaults[] = SIZE_DEFAULTS;

MenuItem fileMenu[] = {
    {N_("New Game        Ctrl+N"),        "New Game", ResetProc},
    {N_("New Shuffle Game ..."),          "New Shuffle Game", ShuffleMenuProc},
    {N_("New Variant ...   Alt+Shift+V"), "New Variant", NewVariantProc},      // [HGM] variant: not functional yet
    {"----", NULL, NothingProc},
    {N_("Load Game       Ctrl+O"),        "Load Game", LoadGameProc},
    {N_("Load Position    Ctrl+Shift+O"), "Load Position", LoadPositionProc},
//    {N_("Load Next Game"), "Load Next Game", LoadNextGameProc},
//    {N_("Load Previous Game"), "Load Previous Game", LoadPrevGameProc},
//    {N_("Reload Same Game"), "Reload Same Game", ReloadGameProc},
    {N_("Next Position     Shift+PgDn"), "Load Next Position", LoadNextPositionProc},
    {N_("Prev Position     Shift+PgUp"), "Load Previous Position", LoadPrevPositionProc},
    {"----", NULL, NothingProc},
//    {N_("Reload Same Position"), "Reload Same Position", ReloadPositionProc},
    {N_("Save Game       Ctrl+S"),        "Save Game", SaveGameProc},
    {N_("Save Position    Ctrl+Shift+S"), "Save Position", SavePositionProc},
    {"----", NULL, NothingProc},
    {N_("Mail Move"),            "Mail Move", MailMoveProc},
    {N_("Reload CMail Message"), "Reload CMail Message", ReloadCmailMsgProc},
    {"----", NULL, NothingProc},
    {N_("Quit                 Ctr+Q"), "Exit", QuitProc},
    {NULL, NULL, NULL}
};

MenuItem editMenu[] = {
    {N_("Copy Game    Ctrl+C"),        "Copy Game", CopyGameProc},
    {N_("Copy Position Ctrl+Shift+C"), "Copy Position", CopyPositionProc},
    {"----", NULL, NothingProc},
    {N_("Paste Game    Ctrl+V"),        "Paste Game", PasteGameProc},
    {N_("Paste Position Ctrl+Shift+V"), "Paste Position", PastePositionProc},
    {"----", NULL, NothingProc},
    {N_("Edit Game      Ctrl+E"),        "Edit Game", EditGameProc},
    {N_("Edit Position   Ctrl+Shift+E"), "Edit Position", EditPositionProc},
    {N_("Edit Tags"),                    "Edit Tags", EditTagsProc},
    {N_("Edit Comment"),                 "Edit Comment", EditCommentProc},
    {"----", NULL, NothingProc},
    {N_("Revert              Home"), "Revert", RevertProc},
    {N_("Annotate"),                 "Annotate", AnnotateProc},
    {N_("Truncate Game  End"),       "Truncate Game", TruncateGameProc},
    {"----", NULL, NothingProc},
    {N_("Backward         Alt+Left"),   "Backward", BackwardProc},
    {N_("Forward           Alt+Right"), "Forward", ForwardProc},
    {N_("Back to Start     Alt+Home"),  "Back to Start", ToStartProc},
    {N_("Forward to End Alt+End"),      "Forward to End", ToEndProc},
    {NULL, NULL, NULL}
};

MenuItem viewMenu[] = {
    {N_("Flip View             F2"),         "Flip View", FlipViewProc},
    {"----", NULL, NothingProc},
    {N_("Engine Output      Alt+Shift+O"),   "Show Engine Output", EngineOutputProc},
    {N_("Move History       Alt+Shift+H"),   "Show Move History", HistoryShowProc}, // [HGM] hist: activate 4.2.7 code
    {N_("Evaluation Graph  Alt+Shift+E"),    "Show Evaluation Graph", EvalGraphProc},
    {N_("Game List            Alt+Shift+G"), "Show Game List", ShowGameListProc},
    {N_("ICS text menu"), "ICStex", IcsTextProc},
    {"----", NULL, NothingProc},
    {N_("Tags"),             "Show Tags", EditTagsProc},
    {N_("Comments"),         "Show Comments", EditCommentProc},
    {N_("ICS Input Box"),    "ICS Input Box", IcsInputBoxProc},
    {"----", NULL, NothingProc},
    {N_("Board..."),          "Board Options", BoardOptionsProc},
    {N_("Game List Tags..."), "Game List", GameListOptionsPopUp},
    {NULL, NULL, NULL}
};

MenuItem modeMenu[] = {
    {N_("Machine White  Ctrl+W"), "Machine White", MachineWhiteProc},
    {N_("Machine Black  Ctrl+B"), "Machine Black", MachineBlackProc},
    {N_("Two Machines   Ctrl+T"), "Two Machines", TwoMachinesProc},
    {N_("Analysis Mode  Ctrl+A"), "Analysis Mode", AnalyzeModeProc},
    {N_("Analyze File      Ctrl+F"), "Analyze File", AnalyzeFileProc },
    {N_("Edit Game         Ctrl+E"), "Edit Game", EditGameProc},
    {N_("Edit Position      Ctrl+Shift+E"), "Edit Position", EditPositionProc},
    {N_("Training"),      "Training", TrainingProc},
    {N_("ICS Client"),    "ICS Client", IcsClientProc},
    {"----", NULL, NothingProc},
    {N_("Machine Match"),         "Machine Match", MatchProc},
    {N_("Pause               Pause"),         "Pause", PauseProc},
    {NULL, NULL, NULL}
};

MenuItem actionMenu[] = {
    {N_("Accept             F3"), "Accept", AcceptProc},
    {N_("Decline            F4"), "Decline", DeclineProc},
    {N_("Rematch           F12"), "Rematch", RematchProc},
    {"----", NULL, NothingProc},
    {N_("Call Flag          F5"), "Call Flag", CallFlagProc},
    {N_("Draw                F6"), "Draw", DrawProc},
    {N_("Adjourn            F7"),  "Adjourn", AdjournProc},
    {N_("Abort                F8"),"Abort", AbortProc},
    {N_("Resign              F9"), "Resign", ResignProc},
    {"----", NULL, NothingProc},
    {N_("Stop Observing  F10"), "Stop Observing", StopObservingProc},
    {N_("Stop Examining  F11"), "Stop Examining", StopExaminingProc},
    {N_("Upload to Examine"),   "Upload to Examine", UploadProc},
    {"----", NULL, NothingProc},
    {N_("Adjudicate to White"), "Adjudicate to White", AdjuWhiteProc},
    {N_("Adjudicate to Black"), "Adjudicate to Black", AdjuBlackProc},
    {N_("Adjudicate Draw"),     "Adjudicate Draw", AdjuDrawProc},
    {NULL, NULL, NULL}
};

MenuItem engineMenu[] = {
    {N_("Engine #1 Settings ..."), "Engine #1 Settings", FirstSettingsProc},
    {N_("Engine #2 Settings ..."), "Engine #2 Settings", SecondSettingsProc},
    {"----", NULL, NothingProc},
    {N_("Hint"), "Hint", HintProc},
    {N_("Book"), "Book", BookProc},
    {"----", NULL, NothingProc},
    {N_("Move Now     Ctrl+M"),     "Move Now", MoveNowProc},
    {N_("Retract Move  Ctrl+X"), "Retract Move", RetractMoveProc},
    {NULL, NULL, NULL}
};

MenuItem optionsMenu[] = {
#define OPTIONSDIALOG
#ifdef OPTIONSDIALOG
    {N_("General ..."), "General", OptionsProc},
#endif
    {N_("Time Control ...       Alt+Shift+T"), "Time Control", TimeControlProc},
    {N_("Common Engine ...  Alt+Shift+U"),     "Common Engine", UciMenuProc},
    {N_("Adjudications ...      Alt+Shift+J"), "Adjudications", EngineMenuProc},
    {N_("ICS ..."),    "ICS", IcsOptionsProc},
    {N_("Match ..."), "Match", MatchOptionsProc},
    {N_("Load Game ..."),    "Load Game", LoadOptionsProc},
    {N_("Save Game ..."),    "Save Game", SaveOptionsProc},
//    {N_(" ..."),    "", OptionsProc},
    {N_("Game List ..."),    "Game List", GameListOptionsPopUp},
    {N_("Sounds ..."),    "Sounds", SoundOptionsProc},
    {"----", NULL, NothingProc},
#ifndef OPTIONSDIALOG
    {N_("Always Queen        Ctrl+Shift+Q"),   "Always Queen", AlwaysQueenProc},
    {N_("Animate Dragging"), "Animate Dragging", AnimateDraggingProc},
    {N_("Animate Moving      Ctrl+Shift+A"),   "Animate Moving", AnimateMovingProc},
    {N_("Auto Flag               Ctrl+Shift+F"), "Auto Flag", AutoflagProc},
    {N_("Auto Flip View"),   "Auto Flip View", AutoflipProc},
    {N_("Blindfold"),        "Blindfold", BlindfoldProc},
    {N_("Flash Moves"),      "Flash Moves", FlashMovesProc},
#if HIGHDRAG
    {N_("Highlight Dragging"),    "Highlight Dragging", HighlightDraggingProc},
#endif
    {N_("Highlight Last Move"),   "Highlight Last Move", HighlightLastMoveProc},
    {N_("Highlight With Arrow"),  "Arrow", HighlightArrowProc},
    {N_("Move Sound"),            "Move Sound", MoveSoundProc},
//    {N_("ICS Alarm"),             "ICS Alarm", IcsAlarmProc},
    {N_("One-Click Moving"),      "OneClick", OneClickProc},
    {N_("Periodic Updates"),      "Periodic Updates", PeriodicUpdatesProc},
    {N_("Ponder Next Move  Ctrl+Shift+P"), "Ponder Next Move", PonderNextMoveProc},
    {N_("Popup Exit Message"),    "Popup Exit Message", PopupExitMessageProc},
    {N_("Popup Move Errors"),     "Popup Move Errors", PopupMoveErrorsProc},
//    {N_("Premove"),               "Premove", PremoveProc},
    {N_("Show Coords"),           "Show Coords", ShowCoordsProc},
    {N_("Hide Thinking        Ctrl+Shift+H"),   "Hide Thinking", HideThinkingProc},
    {N_("Test Legality          Ctrl+Shift+L"), "Test Legality", TestLegalityProc},
    {"----", NULL, NothingProc},
#endif
    {N_("Save Settings Now"),     "Save Settings Now", SaveSettingsProc},
    {N_("Save Settings on Exit"), "Save Settings on Exit", SaveOnExitProc},
    {NULL, NULL, NULL}
};

MenuItem helpMenu[] = {
    {N_("Info XBoard"),     "Info XBoard", InfoProc},
    {N_("Man XBoard   F1"), "Man XBoard", ManProc},
    {"----", NULL, NothingProc},
    {N_("About XBoard"), "About XBoard", AboutProc},
    {NULL, NULL, NULL}
};

Menu menuBar[] = {
    {N_("File"),    "File", fileMenu},
    {N_("Edit"),    "Edit", editMenu},
    {N_("View"),    "View", viewMenu},
    {N_("Mode"),    "Mode", modeMenu},
    {N_("Action"),  "Action", actionMenu},
    {N_("Engine"),  "Engine", engineMenu},
    {N_("Options"), "Options", optionsMenu},
    {N_("Help"),    "Help", helpMenu},
    {NULL, NULL, NULL}
};

#define PAUSE_BUTTON "P"
MenuItem buttonBar[] = {
    {"<<", "<<", ToStartProc},
    {"<", "<", BackwardProc},
    {PAUSE_BUTTON, PAUSE_BUTTON, PauseProc},
    {">", ">", ForwardProc},
    {">>", ">>", ToEndProc},
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
/* must be in same order as PieceMenuStrings! */
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
/* must be in same order as PieceMenuStrings! */
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
    { "Iconify", Iconify },
    { "ResetProc", ResetProc },
    { "NewVariantProc", NewVariantProc },
    { "LoadGameProc", LoadGameProc },
    { "LoadNextGameProc", LoadNextGameProc },
    { "LoadPrevGameProc", LoadPrevGameProc },
    { "LoadSelectedProc", LoadSelectedProc },
    { "SetFilterProc", SetFilterProc },
    { "ReloadGameProc", ReloadGameProc },
    { "LoadPositionProc", LoadPositionProc },
    { "LoadNextPositionProc", LoadNextPositionProc },
    { "LoadPrevPositionProc", LoadPrevPositionProc },
    { "ReloadPositionProc", ReloadPositionProc },
    { "CopyPositionProc", CopyPositionProc },
    { "PastePositionProc", PastePositionProc },
    { "CopyGameProc", CopyGameProc },
    { "PasteGameProc", PasteGameProc },
    { "SaveGameProc", SaveGameProc },
    { "SavePositionProc", SavePositionProc },
    { "MailMoveProc", MailMoveProc },
    { "ReloadCmailMsgProc", ReloadCmailMsgProc },
    { "QuitProc", QuitProc },
    { "MachineWhiteProc", MachineWhiteProc },
    { "MachineBlackProc", MachineBlackProc },
    { "AnalysisModeProc", AnalyzeModeProc },
    { "AnalyzeFileProc", AnalyzeFileProc },
    { "TwoMachinesProc", TwoMachinesProc },
    { "IcsClientProc", IcsClientProc },
    { "EditGameProc", EditGameProc },
    { "EditPositionProc", EditPositionProc },
    { "TrainingProc", EditPositionProc },
    { "EngineOutputProc", EngineOutputProc}, // [HGM] Winboard_x engine-output window
    { "EvalGraphProc", EvalGraphProc},       // [HGM] Winboard_x avaluation graph window
    { "ShowGameListProc", ShowGameListProc },
    { "ShowMoveListProc", HistoryShowProc},
    { "EditTagsProc", EditCommentProc },
    { "EditCommentProc", EditCommentProc },
    { "IcsInputBoxProc", IcsInputBoxProc },
    { "PauseProc", PauseProc },
    { "AcceptProc", AcceptProc },
    { "DeclineProc", DeclineProc },
    { "RematchProc", RematchProc },
    { "CallFlagProc", CallFlagProc },
    { "DrawProc", DrawProc },
    { "AdjournProc", AdjournProc },
    { "AbortProc", AbortProc },
    { "ResignProc", ResignProc },
    { "AdjuWhiteProc", AdjuWhiteProc },
    { "AdjuBlackProc", AdjuBlackProc },
    { "AdjuDrawProc", AdjuDrawProc },
    { "EnterKeyProc", EnterKeyProc },
    { "UpKeyProc", UpKeyProc },
    { "DownKeyProc", DownKeyProc },
    { "StopObservingProc", StopObservingProc },
    { "StopExaminingProc", StopExaminingProc },
    { "UploadProc", UploadProc },
    { "BackwardProc", BackwardProc },
    { "ForwardProc", ForwardProc },
    { "ToStartProc", ToStartProc },
    { "ToEndProc", ToEndProc },
    { "RevertProc", RevertProc },
    { "AnnotateProc", AnnotateProc },
    { "TruncateGameProc", TruncateGameProc },
    { "MoveNowProc", MoveNowProc },
    { "RetractMoveProc", RetractMoveProc },
    { "EngineMenuProc", (XtActionProc) EngineMenuProc },
    { "UciMenuProc", (XtActionProc) UciMenuProc },
    { "TimeControlProc", (XtActionProc) TimeControlProc },
    { "FlipViewProc", FlipViewProc },
    { "PonderNextMoveProc", PonderNextMoveProc },
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
    { "SaveSettingsProc", SaveSettingsProc },
    { "SaveOnExitProc", SaveOnExitProc },
    { "InfoProc", InfoProc },
    { "ManProc", ManProc },
    { "HintProc", HintProc },
    { "BookProc", BookProc },
    { "AboutGameProc", AboutGameProc },
    { "AboutProc", AboutProc },
    { "DebugProc", DebugProc },
    { "NothingProc", NothingProc },
    { "CommentClick", (XtActionProc) CommentClick },
    { "CommentPopDown", (XtActionProc) CommentPopDown },
    { "EditCommentPopDown", (XtActionProc) EditCommentPopDown },
    { "TagsPopDown", (XtActionProc) TagsPopDown },
    { "ErrorPopDown", (XtActionProc) ErrorPopDown },
    { "ICSInputBoxPopDown", (XtActionProc) ICSInputBoxPopDown },
    { "FileNamePopDown", (XtActionProc) FileNamePopDown },
    { "AskQuestionPopDown", (XtActionProc) AskQuestionPopDown },
    { "GameListPopDown", (XtActionProc) GameListPopDown },
    { "GameListOptionsPopDown", (XtActionProc) GameListOptionsPopDown },
    { "PromotionPopDown", (XtActionProc) PromotionPopDown },
    { "HistoryPopDown", (XtActionProc) HistoryPopDown },
    { "EngineOutputPopDown", (XtActionProc) EngineOutputPopDown },
    { "EvalGraphPopDown", (XtActionProc) EvalGraphPopDown },
    { "ShufflePopDown", (XtActionProc) ShufflePopDown },
    { "TimeControlPopDown", (XtActionProc) TimeControlPopDown },
    { "GenericPopDown", (XtActionProc) GenericPopDown },
    { "CopyMemoProc", (XtActionProc) CopyMemoProc },
};

char globalTranslations[] =
  ":<Key>F9: ResignProc() \n \
   :Ctrl<Key>n: ResetProc() \n \
   :Meta<Key>V: NewVariantProc() \n \
   :Ctrl<Key>o: LoadGameProc() \n \
   :Meta<Key>Next: LoadNextGameProc() \n \
   :Meta<Key>Prior: LoadPrevGameProc() \n \
   :Ctrl<Key>s: SaveGameProc() \n \
   :Ctrl<Key>c: CopyGameProc() \n \
   :Ctrl<Key>v: PasteGameProc() \n \
   :Ctrl<Key>O: LoadPositionProc() \n \
   :Shift<Key>Next: LoadNextPositionProc() \n \
   :Shift<Key>Prior: LoadPrevPositionProc() \n \
   :Ctrl<Key>S: SavePositionProc() \n \
   :Ctrl<Key>C: CopyPositionProc() \n \
   :Ctrl<Key>V: PastePositionProc() \n \
   :Ctrl<Key>q: QuitProc() \n \
   :Ctrl<Key>w: MachineWhiteProc() \n \
   :Ctrl<Key>b: MachineBlackProc() \n \
   :Ctrl<Key>t: TwoMachinesProc() \n \
   :Ctrl<Key>a: AnalysisModeProc() \n \
   :Ctrl<Key>f: AnalyzeFileProc() \n \
   :Ctrl<Key>e: EditGameProc() \n \
   :Ctrl<Key>E: EditPositionProc() \n \
   :Meta<Key>O: EngineOutputProc() \n \
   :Meta<Key>E: EvalGraphProc() \n \
   :Meta<Key>G: ShowGameListProc() \n \
   :Meta<Key>H: ShowMoveListProc() \n \
   :<Key>Pause: PauseProc() \n \
   :<Key>F3: AcceptProc() \n \
   :<Key>F4: DeclineProc() \n \
   :<Key>F12: RematchProc() \n \
   :<Key>F5: CallFlagProc() \n \
   :<Key>F6: DrawProc() \n \
   :<Key>F7: AdjournProc() \n \
   :<Key>F8: AbortProc() \n \
   :<Key>F10: StopObservingProc() \n \
   :<Key>F11: StopExaminingProc() \n \
   :Meta Ctrl<Key>F12: DebugProc() \n \
   :Meta<Key>End: ToEndProc() \n \
   :Meta<Key>Right: ForwardProc() \n \
   :Meta<Key>Home: ToStartProc() \n \
   :Meta<Key>Left: BackwardProc() \n \
   :<Key>Home: RevertProc() \n \
   :<Key>End: TruncateGameProc() \n \
   :Ctrl<Key>m: MoveNowProc() \n \
   :Ctrl<Key>x: RetractMoveProc() \n \
   :Meta<Key>J: EngineMenuProc() \n \
   :Meta<Key>U: UciMenuProc() \n \
   :Meta<Key>T: TimeControlProc() \n \
   :Ctrl<Key>P: PonderNextMoveProc() \n "
#ifndef OPTIONSDIALOG
    "\
   :Ctrl<Key>Q: AlwaysQueenProc() \n \
   :Ctrl<Key>F: AutoflagProc() \n \
   :Ctrl<Key>A: AnimateMovingProc() \n \
   :Ctrl<Key>L: TestLegalityProc() \n \
   :Ctrl<Key>H: HideThinkingProc() \n "
#endif
   "\
   :<Key>-: Iconify() \n \
   :<Key>F1: ManProc() \n \
   :<Key>F2: FlipViewProc() \n \
   <KeyDown>.: BackwardProc() \n \
   <KeyUp>.: ForwardProc() \n \
   Shift<Key>1: AskQuestionProc(\"Direct command\",\
                                \"Send to chess program:\",,1) \n \
   Shift<Key>2: AskQuestionProc(\"Direct command\",\
                                \"Send to second chess program:\",,2) \n";

char boardTranslations[] =
   "<Btn1Down>: HandleUserMove(0) \n \
   Shift<Btn1Up>: HandleUserMove(1) \n \
   <Btn1Up>: HandleUserMove(0) \n \
   <Btn1Motion>: AnimateUserMove() \n \
   <Btn3Motion>: HandlePV() \n \
   <Btn3Up>: PieceMenuPopup(menuB) \n \
   Shift<Btn2Down>: XawPositionSimpleMenu(menuB) XawPositionSimpleMenu(menuD)\
                 PieceMenuPopup(menuB) \n \
   Any<Btn2Down>: XawPositionSimpleMenu(menuW) XawPositionSimpleMenu(menuD) \
                 PieceMenuPopup(menuW) \n \
   Shift<Btn3Down>: XawPositionSimpleMenu(menuW) XawPositionSimpleMenu(menuD)\
                 PieceMenuPopup(menuW) \n \
   Any<Btn3Down>: XawPositionSimpleMenu(menuB) XawPositionSimpleMenu(menuD) \
                 PieceMenuPopup(menuB) \n";

char whiteTranslations[] = "<BtnDown>: WhiteClock()\n";
char blackTranslations[] = "<BtnDown>: BlackClock()\n";

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
xpm_getsize(name, len, ext)
     char *name;
     int len;
     char *ext;
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
xpm_getavail(dirname, ext)
     char *dirname;
     char *ext;
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
xpm_print_avail(fp, ext)
     FILE *fp;
     char *ext;
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
xpm_closest_to(dirname, size, ext)
     char *dirname;
     int size;
     char *ext;
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
xpm_closest_to(dirname, size, ext)
     char *dirname;
     int size;
     char *ext;
{
    fprintf(stderr, _("\
Warning: No DIR structure found on this system --\n\
         Unable to autosize for XPM/XIM pieces.\n\
   Please report this error to frankm@hiwaay.net.\n\
   Include system type & operating system in message.\n"));
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
parse_color(str, which)
     char *str;
     int which;
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
parse_cpair(cc, str)
     ColorClass cc;
     char *str;
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
CatchDeleteWindow(Widget w, String procname)
{
  char buf[MSG_SIZ];
  XSetWMProtocols(xDisplay, XtWindow(w), &wm_delete_window, 1);
  snprintf(buf, sizeof(buf), "<Message>WM_PROTOCOLS: %s() \n", procname);
  XtAugmentTranslations(w, XtParseTranslationTable(buf));
}

void
BoardToTop()
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
Boolean fontSet[NUM_FONTS], fontValid[NUM_FONTS][MAX_SIZE];
char *fontTable[NUM_FONTS][MAX_SIZE];

void
ParseFont(char *name, int number)
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
  fontSet[number] = True; // [HGM] font: indicate a font was specified (not from settings file)
}

void
SetFontDefaults()
{ // only 2 fonts currently
  appData.clockFont = CLOCK_FONT_NAME;
  appData.coordFont = COORD_FONT_NAME;
  appData.font  =   DEFAULT_FONT_NAME;
}

void
CreateFonts()
{ // no-op, until we identify the code for this already in XBoard and move it here
}

void
ParseColor(int n, char *name)
{ // in XBoard, just copy the color-name string
  if(colorVariable[n]) *(char**)colorVariable[n] = strdup(name);
}

void
ParseTextAttribs(ColorClass cc, char *s)
{
    (&appData.colorShout)[cc] = strdup(s);
}

void
ParseBoardSize(void *addr, char *name)
{
    appData.boardSize = strdup(name);
}

void
LoadAllSounds()
{ // In XBoard the sound-playing program takes care of obtaining the actual sound
}

void
SetCommPortDefaults()
{ // for now, this is a no-op, as the corresponding option does not exist in XBoard
}

// [HGM] args: these three cases taken out to stay in front-end
void
SaveFontArg(FILE *f, ArgDescriptor *ad)
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
ExportSounds()
{ // nothing to do, as the sounds are at all times represented by their text-string names already
}

void
SaveAttribsArg(FILE *f, ArgDescriptor *ad)
{	// here the "argLoc" defines a table index. It could have contained the 'ta' pointer itself, though
	fprintf(f, OPTCHAR "%s" SEPCHAR "%s\n", ad->argName, (&appData.colorShout)[(int)(intptr_t)ad->argLoc]);
}

void
SaveColor(FILE *f, ArgDescriptor *ad)
{	// in WinBoard the color is an int and has to be converted to text. In X it would be a string already?
	if(colorVariable[(int)(intptr_t)ad->argLoc])
	fprintf(f, OPTCHAR "%s" SEPCHAR "%s\n", ad->argName, *(char**)colorVariable[(int)(intptr_t)ad->argLoc]);
}

void
SaveBoardSize(FILE *f, char *name, void *addr)
{ // wrapper to shield back-end from BoardSize & sizeInfo
  fprintf(f, OPTCHAR "%s" SEPCHAR "%s\n", name, appData.boardSize);
}

void
ParseCommPortSettings(char *s)
{ // no such option in XBoard (yet)
}

extern Widget engineOutputShell;
extern Widget tagsShell, editTagsShell;
void
GetActualPlacement(Widget wg, WindowPlacement *wp)
{
  Arg args[16];
  Dimension w, h;
  Position x, y;
  int i;

  if(!wg) return;

    i = 0;
    XtSetArg(args[i], XtNx, &x); i++;
    XtSetArg(args[i], XtNy, &y); i++;
    XtSetArg(args[i], XtNwidth, &w); i++;
    XtSetArg(args[i], XtNheight, &h); i++;
    XtGetValues(wg, args, i);
    wp->x = x - 4;
    wp->y = y - 23;
    wp->height = h;
    wp->width = w;
}

void
GetWindowCoords()
{ // wrapper to shield use of window handles from back-end (make addressible by number?)
  // In XBoard this will have to wait until awareness of window parameters is implemented
  GetActualPlacement(shellWidget, &wpMain);
  if(EngineOutputIsUp()) GetActualPlacement(engineOutputShell, &wpEngineOutput);
  if(MoveHistoryIsUp()) GetActualPlacement(historyShell, &wpMoveHistory);
  if(EvalGraphIsUp()) GetActualPlacement(evalGraphShell, &wpEvalGraph);
  if(GameListIsUp()) GetActualPlacement(gameListShell, &wpGameList);
  if(commentShell) GetActualPlacement(commentShell, &wpComment);
  else             GetActualPlacement(editShell,    &wpComment);
  if(tagsShell) GetActualPlacement(tagsShell, &wpTags);
  else      GetActualPlacement(editTagsShell, &wpTags);
}

void
PrintCommPortSettings(FILE *f, char *name)
{ // This option does not exist in XBoard
}

int
MySearchPath(char *installDir, char *name, char *fullname)
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
MyGetFullPathName(char *name, char *fullname)
{ // should use ExpandPath?
  name = ExpandPathName(name);
  safeStrCpy(fullname, name, MSG_SIZ );
  return 1;
}

void
EnsureOnScreen(int *x, int *y, int minX, int minY)
{
  return;
}

int
MainWindowUp()
{ // [HGM] args: allows testing if main window is realized from back-end
  return xBoardWindow != 0;
}

void
PopUpStartupDialog()
{  // start menu not implemented in XBoard
}

char *
ConvertToLine(int argc, char **argv)
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

extern Boolean twoBoards, partnerUp;

#ifdef IDSIZES
  // eventually, all layout determining code should go into a subroutine, but until then IDSIZE remains undefined
#else
#define BoardSize int
void InitDrawingSizes(BoardSize boardSize, int flags)
{   // [HGM] resize is functional now, but for board format changes only (nr of ranks, files)
    Dimension timerWidth, boardWidth, boardHeight, w, h, sep, bor, wr, hr;
    Arg args[16];
    XtGeometryResult gres;
    int i;

    if(!formWidget) return;

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

    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap;
    boardWidth = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);
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

    // [HGM] pieces: tailor piece bitmaps to needs of specific variant
    // (only for xpm)
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
#if HAVE_LIBXPM
    CreateAnimVars();
#endif
}
#endif

void ParseIcsTextColors()
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

int MakeColors()
{   // [HGM] taken out of main(), so it can be called from BoardOptions dialog
    XrmValue vFrom, vTo;
    int forceMono = False;

    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.lightSquareColor;
	vFrom.size = strlen(appData.lightSquareColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  lightSquareColor = *(Pixel *) vTo.addr;
	}
    }
    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.darkSquareColor;
	vFrom.size = strlen(appData.darkSquareColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  darkSquareColor = *(Pixel *) vTo.addr;
	}
    }
    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.whitePieceColor;
	vFrom.size = strlen(appData.whitePieceColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  whitePieceColor = *(Pixel *) vTo.addr;
	}
    }
    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.blackPieceColor;
	vFrom.size = strlen(appData.blackPieceColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  blackPieceColor = *(Pixel *) vTo.addr;
	}
    }

    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.highlightSquareColor;
	vFrom.size = strlen(appData.highlightSquareColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  highlightSquareColor = *(Pixel *) vTo.addr;
	}
    }

    if (!appData.monoMode) {
	vFrom.addr = (caddr_t) appData.premoveHighlightColor;
	vFrom.size = strlen(appData.premoveHighlightColor);
	XtConvert(shellWidget, XtRString, &vFrom, XtRPixel, &vTo);
	if (vTo.addr == NULL) {
	  appData.monoMode = True;
	  forceMono = True;
	} else {
	  premoveHighlightColor = *(Pixel *) vTo.addr;
	}
    }
    return forceMono;
}

void
CreateAnyPieces()
{   // [HGM] taken out of main
#if HAVE_LIBXPM
    if (appData.monoMode && // [HGM] no sense to go on to certain doom
       (appData.bitmapDirectory == NULL || appData.bitmapDirectory[0] == NULLCHAR))
	    appData.bitmapDirectory = DEF_BITMAP_DIR;

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

int
main(argc, argv)
     int argc;
     char **argv;
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
	EscapeExpand(buf, appData.initString);
	appData.initString = strdup(buf);
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
    if(!fontSet[CLOCK_FONT] && fontValid[CLOCK_FONT][squareSize])
	appData.clockFont = fontTable[CLOCK_FONT][squareSize];
    if(!fontSet[MESSAGE_FONT] && fontValid[MESSAGE_FONT][squareSize])
	appData.font = fontTable[MESSAGE_FONT][squareSize];
    if(!fontSet[COORD_FONT] && fontValid[COORD_FONT][squareSize])
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
    appData.clockFont = FindFont(appData.clockFont, clockFontPxlSize);
    clockFontID = XLoadFont(xDisplay, appData.clockFont);
    clockFontStruct = XQueryFont(xDisplay, clockFontID);
    appData.coordFont = FindFont(appData.coordFont, coordFontPxlSize);
    coordFontID = XLoadFont(xDisplay, appData.coordFont);
    coordFontStruct = XQueryFont(xDisplay, coordFontID);
    appData.font = FindFont(appData.font, fontPxlSize);
    countFontID = XLoadFont(xDisplay, appData.coordFont); // [HGM] holdings
    countFontStruct = XQueryFont(xDisplay, countFontID);
//    appData.font = FindFont(appData.font, fontPxlSize);

    xdb = XtDatabase(xDisplay);
    XrmPutStringResource(&xdb, "*font", appData.font);

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
    widgetList[j++] = menuBarWidget = CreateMenuBar(menuBar);
    XtSetArg(args[0], XtNtop,    XtChainTop);
    XtSetArg(args[1], XtNbottom, XtChainTop);
    XtSetArg(args[2], XtNright,  XtChainLeft);
    XtSetValues(menuBarWidget, args, 3);

    widgetList[j++] = whiteTimerWidget =
      XtCreateWidget("whiteTime", labelWidgetClass,
		     formWidget, timerArgs, XtNumber(timerArgs));
    XtSetArg(args[0], XtNfont, clockFontStruct);
    XtSetArg(args[1], XtNtop,    XtChainTop);
    XtSetArg(args[2], XtNbottom, XtChainTop);
    XtSetValues(whiteTimerWidget, args, 3);

    widgetList[j++] = blackTimerWidget =
      XtCreateWidget("blackTime", labelWidgetClass,
		     formWidget, timerArgs, XtNumber(timerArgs));
    XtSetArg(args[0], XtNfont, clockFontStruct);
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
    XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
#ifndef OPTIONSDIALOG
    if (appData.alwaysPromoteToQueen) {
	XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Always Queen"),
		    args, 1);
    }
    if (appData.animateDragging) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Animate Dragging"),
		    args, 1);
    }
    if (appData.animate) {
	XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Animate Moving"),
		    args, 1);
    }
    if (appData.autoCallFlag) {
	XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Flag"),
		    args, 1);
    }
    if (appData.autoFlipView) {
	XtSetValues(XtNameToWidget(menuBarWidget,"menuOptions.Auto Flip View"),
		    args, 1);
    }
    if (appData.blindfold) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Blindfold"), args, 1);
    }
    if (appData.flashCount > 0) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Flash Moves"),
		    args, 1);
    }
#if HIGHDRAG
    if (appData.highlightDragging) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Highlight Dragging"),
		    args, 1);
    }
#endif
    if (appData.highlightLastMove) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Highlight Last Move"),
		    args, 1);
    }
    if (appData.highlightMoveWithArrow) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Arrow"),
		    args, 1);
    }
//    if (appData.icsAlarm) {
//	XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.ICS Alarm"),
//		    args, 1);
//    }
    if (appData.ringBellAfterMoves) {
	XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Move Sound"),
		    args, 1);
    }
    if (appData.oneClick) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.OneClick"), args, 1);
    }
    if (appData.periodicUpdates) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Periodic Updates"), args, 1);
    }
    if (appData.ponderNextMove) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Ponder Next Move"), args, 1);
    }
    if (appData.popupExitMessage) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Popup Exit Message"), args, 1);
    }
    if (appData.popupMoveErrors) {
	XtSetValues(XtNameToWidget(menuBarWidget,
				   "menuOptions.Popup Move Errors"), args, 1);
    }
//    if (appData.premove) {
//	XtSetValues(XtNameToWidget(menuBarWidget,
//				   "menuOptions.Premove"), args, 1);
//    }
    if (appData.showCoords) {
	XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Show Coords"),
		    args, 1);
    }
    if (appData.hideThinkingFromHuman) {
	XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Hide Thinking"),
		    args, 1);
    }
    if (appData.testLegality) {
	XtSetValues(XtNameToWidget(menuBarWidget,"menuOptions.Test Legality"),
		    args, 1);
    }
#endif
    if (saveSettingsOnExit) {
	XtSetValues(XtNameToWidget(menuBarWidget,"menuOptions.Save Settings on Exit"),
		    args, 1);
    }

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
ShutDownFrontEnd()
{
    if (appData.icsActive && oldICSInteractionTitle != NULL) {
        DisplayIcsInteractionTitle(oldICSInteractionTitle);
    }
    if (saveSettingsOnExit) SaveSettings(settingsFileName);
    unlink(gameCopyFilename);
    unlink(gamePasteFilename);
    if(noEcho) EchoOn();
}

RETSIGTYPE TermSizeSigHandler(int sig)
{
    update_ics_width();
}

RETSIGTYPE
IntSigHandler(sig)
     int sig;
{
    ExitEvent(sig);
}

RETSIGTYPE
CmailSigHandler(sig)
     int sig;
{
    int dummy = 0;
    int error;

    signal(SIGUSR1, SIG_IGN);	/* suspend handler     */

    /* Activate call-back function CmailSigHandlerCallBack()             */
    OutputToProcess(cmailPR, (char *)(&dummy), sizeof(int), &error);

    signal(SIGUSR1, CmailSigHandler); /* re-activate handler */
}

void
CmailSigHandlerCallBack(isr, closure, message, count, error)
     InputSourceRef isr;
     VOIDSTAR closure;
     char *message;
     int count;
     int error;
{
    BoardToTop();
    ReloadCmailMsgEvent(TRUE);	/* Reload cmail msg  */
}
/**** end signal code ****/


void
ICSInitScript()
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
ResetFrontEnd()
{
    CommentPopDown();
    EditCommentPopDown();
    TagsPopDown();
    return;
}

typedef struct {
    char *name;
    Boolean value;
} Enables;

void
GreyRevert(grey)
     Boolean grey;
{
    Widget w;
    if (!menuBarWidget) return;
    w = XtNameToWidget(menuBarWidget, "menuEdit.Revert");
    if (w == NULL) {
      DisplayError("menuEdit.Revert", 0);
    } else {
      XtSetSensitive(w, !grey);
    }
    w = XtNameToWidget(menuBarWidget, "menuEdit.Annotate");
    if (w == NULL) {
      DisplayError("menuEdit.Annotate", 0);
    } else {
      XtSetSensitive(w, !grey);
    }
}

void
SetMenuEnables(enab)
     Enables *enab;
{
  Widget w;
  if (!menuBarWidget) return;
  while (enab->name != NULL) {
    w = XtNameToWidget(menuBarWidget, enab->name);
    if (w == NULL) {
      DisplayError(enab->name, 0);
    } else {
      XtSetSensitive(w, enab->value);
    }
    enab++;
  }
}

Enables icsEnables[] = {
    { "menuFile.Mail Move", False },
    { "menuFile.Reload CMail Message", False },
    { "menuMode.Machine Black", False },
    { "menuMode.Machine White", False },
    { "menuMode.Analysis Mode", False },
    { "menuMode.Analyze File", False },
    { "menuMode.Two Machines", False },
    { "menuMode.Machine Match", False },
#ifndef ZIPPY
    { "menuEngine.Hint", False },
    { "menuEngine.Book", False },
    { "menuEngine.Move Now", False },
#ifndef OPTIONSDIALOG
    { "menuOptions.Periodic Updates", False },
    { "menuOptions.Hide Thinking", False },
    { "menuOptions.Ponder Next Move", False },
#endif
#endif
    { "menuEngine.Engine #1 Settings", False },
    { "menuEngine.Engine #2 Settings", False },
    { "menuEdit.Annotate", False },
    { NULL, False }
};

Enables ncpEnables[] = {
    { "menuFile.Mail Move", False },
    { "menuFile.Reload CMail Message", False },
    { "menuMode.Machine White", False },
    { "menuMode.Machine Black", False },
    { "menuMode.Analysis Mode", False },
    { "menuMode.Analyze File", False },
    { "menuMode.Two Machines", False },
    { "menuMode.Machine Match", False },
    { "menuMode.ICS Client", False },
    { "menuView.ICStex", False },
    { "menuView.ICS Input Box", False },
    { "Action", False },
    { "menuEdit.Revert", False },
    { "menuEdit.Annotate", False },
    { "menuEngine.Engine #1 Settings", False },
    { "menuEngine.Engine #2 Settings", False },
    { "menuEngine.Move Now", False },
    { "menuEngine.Retract Move", False },
    { "menuOptions.ICS", False },
#ifndef OPTIONSDIALOG
    { "menuOptions.Auto Flag", False },
    { "menuOptions.Auto Flip View", False },
//    { "menuOptions.ICS Alarm", False },
    { "menuOptions.Move Sound", False },
    { "menuOptions.Hide Thinking", False },
    { "menuOptions.Periodic Updates", False },
    { "menuOptions.Ponder Next Move", False },
#endif
    { "menuEngine.Hint", False },
    { "menuEngine.Book", False },
    { NULL, False }
};

Enables gnuEnables[] = {
    { "menuMode.ICS Client", False },
    { "menuView.ICStex", False },
    { "menuView.ICS Input Box", False },
    { "menuAction.Accept", False },
    { "menuAction.Decline", False },
    { "menuAction.Rematch", False },
    { "menuAction.Adjourn", False },
    { "menuAction.Stop Examining", False },
    { "menuAction.Stop Observing", False },
    { "menuAction.Upload to Examine", False },
    { "menuEdit.Revert", False },
    { "menuEdit.Annotate", False },
    { "menuOptions.ICS", False },

    /* The next two options rely on SetCmailMode being called *after*    */
    /* SetGNUMode so that when GNU is being used to give hints these     */
    /* menu options are still available                                  */

    { "menuFile.Mail Move", False },
    { "menuFile.Reload CMail Message", False },
    { NULL, False }
};

Enables cmailEnables[] = {
    { "Action", True },
    { "menuAction.Call Flag", False },
    { "menuAction.Draw", True },
    { "menuAction.Adjourn", False },
    { "menuAction.Abort", False },
    { "menuAction.Stop Observing", False },
    { "menuAction.Stop Examining", False },
    { "menuFile.Mail Move", True },
    { "menuFile.Reload CMail Message", True },
    { NULL, False }
};

Enables trainingOnEnables[] = {
  { "menuMode.Edit Comment", False },
  { "menuMode.Pause", False },
  { "menuEdit.Forward", False },
  { "menuEdit.Backward", False },
  { "menuEdit.Forward to End", False },
  { "menuEdit.Back to Start", False },
  { "menuEngine.Move Now", False },
  { "menuEdit.Truncate Game", False },
  { NULL, False }
};

Enables trainingOffEnables[] = {
  { "menuMode.Edit Comment", True },
  { "menuMode.Pause", True },
  { "menuEdit.Forward", True },
  { "menuEdit.Backward", True },
  { "menuEdit.Forward to End", True },
  { "menuEdit.Back to Start", True },
  { "menuEngine.Move Now", True },
  { "menuEdit.Truncate Game", True },
  { NULL, False }
};

Enables machineThinkingEnables[] = {
  { "menuFile.Load Game", False },
//  { "menuFile.Load Next Game", False },
//  { "menuFile.Load Previous Game", False },
//  { "menuFile.Reload Same Game", False },
  { "menuEdit.Paste Game", False },
  { "menuFile.Load Position", False },
//  { "menuFile.Load Next Position", False },
//  { "menuFile.Load Previous Position", False },
//  { "menuFile.Reload Same Position", False },
  { "menuEdit.Paste Position", False },
  { "menuMode.Machine White", False },
  { "menuMode.Machine Black", False },
  { "menuMode.Two Machines", False },
  { "menuMode.Machine Match", False },
  { "menuEngine.Retract Move", False },
  { NULL, False }
};

Enables userThinkingEnables[] = {
  { "menuFile.Load Game", True },
//  { "menuFile.Load Next Game", True },
//  { "menuFile.Load Previous Game", True },
//  { "menuFile.Reload Same Game", True },
  { "menuEdit.Paste Game", True },
  { "menuFile.Load Position", True },
//  { "menuFile.Load Next Position", True },
//  { "menuFile.Load Previous Position", True },
//  { "menuFile.Reload Same Position", True },
  { "menuEdit.Paste Position", True },
  { "menuMode.Machine White", True },
  { "menuMode.Machine Black", True },
  { "menuMode.Two Machines", True },
  { "menuMode.Machine Match", True },
  { "menuEngine.Retract Move", True },
  { NULL, False }
};

void SetICSMode()
{
  SetMenuEnables(icsEnables);

#if ZIPPY
  if (appData.zippyPlay && !appData.noChessProgram) { /* [DM] icsEngineAnalyze */
     XtSetSensitive(XtNameToWidget(menuBarWidget, "menuMode.Analysis Mode"), True);
     XtSetSensitive(XtNameToWidget(menuBarWidget, "menuEngine.Engine #1 Settings"), True);
  }
#endif
}

void
SetNCPMode()
{
  SetMenuEnables(ncpEnables);
}

void
SetGNUMode()
{
  SetMenuEnables(gnuEnables);
}

void
SetCmailMode()
{
  SetMenuEnables(cmailEnables);
}

void
SetTrainingModeOn()
{
  SetMenuEnables(trainingOnEnables);
  if (appData.showButtonBar) {
    XtSetSensitive(buttonBarWidget, False);
  }
  CommentPopDown();
}

void
SetTrainingModeOff()
{
  SetMenuEnables(trainingOffEnables);
  if (appData.showButtonBar) {
    XtSetSensitive(buttonBarWidget, True);
  }
}

void
SetUserThinkingEnables()
{
  if (appData.noChessProgram) return;
  SetMenuEnables(userThinkingEnables);
}

void
SetMachineThinkingEnables()
{
  if (appData.noChessProgram) return;
  SetMenuEnables(machineThinkingEnables);
  switch (gameMode) {
  case MachinePlaysBlack:
  case MachinePlaysWhite:
  case TwoMachinesPlay:
    XtSetSensitive(XtNameToWidget(menuBarWidget,
				  ModeToWidgetName(gameMode)), True);
    break;
  default:
    break;
  }
}

// [HGM] code borrowed from winboard.c (which should thus go to backend.c!)
#define HISTORY_SIZE 64
static char *history[HISTORY_SIZE];
int histIn = 0, histP = 0;

void
SaveInHistory(char *cmd)
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
PrevInHistory(char *cmd)
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
NextInHistory()
{
  if (histP == histIn) return NULL;
  histP = (histP + 1) % HISTORY_SIZE;
  return history[histP];   
}
// end of borrowed code

#define Abs(n) ((n)<0 ? -(n) : (n))

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
FindFont(pattern, targetPxlSize)
     char *pattern;
     int targetPxlSize;
{
    char **fonts, *p, *best, *scalable, *scalableTail;
    int i, j, nfonts, minerr, err, pxlSize;

#ifdef ENABLE_NLS
    char **missing_list;
    int missing_count;
    char *def_string, *base_fnt_lst, strInt[3];
    XFontSet fntSet;
    XFontStruct **fnt_list;
    base_fnt_lst = calloc(1, strlen(pattern) + 3);
    snprintf(strInt, sizeof(strInt)/sizeof(strInt[0]), "%d", targetPxlSize);
    p = strstr(pattern, "--");
    strncpy(base_fnt_lst, pattern, p - pattern + 2);
    strcat(base_fnt_lst, strInt);
    strcat(base_fnt_lst, strchr(p + 2, '-'));

    if ((fntSet = XCreateFontSet(xDisplay,
                                 base_fnt_lst,
                                 &missing_list,
                                 &missing_count,
                                 &def_string)) == NULL) {

       fprintf(stderr, _("Unable to create font set.\n"));
       exit (2);
    }

    nfonts = XFontsOfFontSet(fntSet, &fnt_list, &fonts);
#else
    fonts = XListFonts(xDisplay, pattern, 999999, &nfonts);
    if (nfonts < 1) {
	fprintf(stderr, _("%s: no fonts match pattern %s\n"),
		programName, pattern);
	exit(2);
    }
#endif

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
#ifdef ENABLE_NLS
    if (missing_count > 0)
       XFreeStringList(missing_list);
    XFreeFontSet(xDisplay, fntSet);
#else
     XFreeFontNames(fonts);
#endif
    return p;
}

void DeleteGCs()
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

void CreateGCs(int redo)
{
    XtGCMask value_mask = GCLineWidth | GCLineStyle | GCForeground
      | GCBackground | GCFunction | GCPlaneMask;
    XGCValues gc_values;
    GC copyInvertedGC;

    gc_values.plane_mask = AllPlanes;
    gc_values.line_width = lineGap;
    gc_values.line_style = LineSolid;
    gc_values.function = GXcopy;

  if(redo) {
    DeleteGCs(); // called a second time; clean up old GCs first
  } else { // [HGM] grid and font GCs created on first call only
    gc_values.foreground = XBlackPixel(xDisplay, xScreen);
    gc_values.background = XWhitePixel(xDisplay, xScreen);
    coordGC = XtGetGC(shellWidget, value_mask, &gc_values);
    XSetFont(xDisplay, coordGC, coordFontID);

    // [HGM] make font for holdings counts (white on black)
    gc_values.foreground = XWhitePixel(xDisplay, xScreen);
    gc_values.background = XBlackPixel(xDisplay, xScreen);
    countGC = XtGetGC(shellWidget, value_mask, &gc_values);
    XSetFont(xDisplay, countGC, countFontID);
  }
    gc_values.foreground = XBlackPixel(xDisplay, xScreen);
    gc_values.background = XBlackPixel(xDisplay, xScreen);
    lineGC = XtGetGC(shellWidget, value_mask, &gc_values);

    if (appData.monoMode) {
	gc_values.foreground = XWhitePixel(xDisplay, xScreen);
	gc_values.background = XWhitePixel(xDisplay, xScreen);
	highlineGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = XWhitePixel(xDisplay, xScreen);
	gc_values.background = XBlackPixel(xDisplay, xScreen);
	lightSquareGC = wbPieceGC
	  = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = XBlackPixel(xDisplay, xScreen);
	gc_values.background = XWhitePixel(xDisplay, xScreen);
	darkSquareGC = bwPieceGC
	  = XtGetGC(shellWidget, value_mask, &gc_values);

	if (DefaultDepth(xDisplay, xScreen) == 1) {
	    /* Avoid XCopyPlane on 1-bit screens to work around Sun bug */
	    gc_values.function = GXcopyInverted;
	    copyInvertedGC = XtGetGC(shellWidget, value_mask, &gc_values);
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
	gc_values.foreground = highlightSquareColor;
	gc_values.background = highlightSquareColor;
	highlineGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = premoveHighlightColor;
	gc_values.background = premoveHighlightColor;
	prelineGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = lightSquareColor;
	gc_values.background = darkSquareColor;
	lightSquareGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = darkSquareColor;
	gc_values.background = lightSquareColor;
	darkSquareGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = jailSquareColor;
	gc_values.background = jailSquareColor;
	jailSquareGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = whitePieceColor;
	gc_values.background = darkSquareColor;
	wdPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = whitePieceColor;
	gc_values.background = lightSquareColor;
	wlPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = whitePieceColor;
	gc_values.background = jailSquareColor;
	wjPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = blackPieceColor;
	gc_values.background = darkSquareColor;
	bdPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = blackPieceColor;
	gc_values.background = lightSquareColor;
	blPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);

	gc_values.foreground = blackPieceColor;
	gc_values.background = jailSquareColor;
	bjPieceGC = XtGetGC(shellWidget, value_mask, &gc_values);
    }
}

void loadXIM(xim, xmask, filename, dest, mask)
     XImage *xim;
     XImage *xmask;
     char *filename;
     Pixmap *dest;
     Pixmap *mask;
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

void CreateXIMPieces()
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
void CreateXPMBoard(char *s, int kind)
{
    XpmAttributes attr;
    attr.valuemask = 0;
    if(s == NULL || *s == 0 || *s == '*') { useTexture &= ~(kind+1); return; }
    if (XpmReadFileToPixmap(xDisplay, xBoardWindow, s, &(xpmBoardBitmap[kind]), NULL, &attr) == 0) {
	useTexture |= kind + 1; textureW[kind] = attr.width; textureH[kind] = attr.height;
    }
}

void FreeXPMPieces()
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

void CreateXPMPieces()
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
void CreatePieces()
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

void ReadBitmap(pm, name, bits, wreq, hreq)
     Pixmap *pm;
     String name;
     unsigned char bits[];
     u_int wreq, hreq;
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

void CreateGrid()
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

static void MenuBarSelect(w, addr, index)
     Widget w;
     caddr_t addr;
     caddr_t index;
{
    XtActionProc proc = (XtActionProc) addr;

    (proc)(NULL, NULL, NULL, NULL);
}

void CreateMenuBarPopup(parent, name, mb)
     Widget parent;
     String name;
     Menu *mb;
{
    int j;
    Widget menu, entry;
    MenuItem *mi;
    Arg args[16];

    menu = XtCreatePopupShell(name, simpleMenuWidgetClass,
			      parent, NULL, 0);
    j = 0;
    XtSetArg(args[j], XtNleftMargin, 20);   j++;
    XtSetArg(args[j], XtNrightMargin, 20);  j++;
    mi = mb->mi;
    while (mi->string != NULL) {
	if (strcmp(mi->string, "----") == 0) {
	    entry = XtCreateManagedWidget(mi->string, smeLineObjectClass,
					  menu, args, j);
	} else {
          XtSetArg(args[j], XtNlabel, XtNewString(mi->string));
	    entry = XtCreateManagedWidget(mi->ref, smeBSBObjectClass,
					  menu, args, j+1);
	    XtAddCallback(entry, XtNcallback,
			  (XtCallbackProc) MenuBarSelect,
			  (caddr_t) mi->proc);
	}
	mi++;
    }
}

Widget CreateMenuBar(mb)
     Menu *mb;
{
    int j;
    Widget anchor, menuBar;
    Arg args[16];
    char menuName[MSG_SIZ];

    j = 0;
    XtSetArg(args[j], XtNorientation, XtorientHorizontal);  j++;
    XtSetArg(args[j], XtNvSpace, 0);                        j++;
    XtSetArg(args[j], XtNborderWidth, 0);                   j++;
    menuBar = XtCreateWidget("menuBar", boxWidgetClass,
			     formWidget, args, j);

    while (mb->name != NULL) {
        safeStrCpy(menuName, "menu", sizeof(menuName)/sizeof(menuName[0]) );
	strncat(menuName, mb->ref, MSG_SIZ - strlen(menuName) - 1);
	j = 0;
	XtSetArg(args[j], XtNmenuName, XtNewString(menuName));  j++;
	if (tinyLayout || squareSize+lineGap < 45) {
	    char shortName[20];
	    int len;
            strncpy(shortName, mb->name, 20);
	    len = tinyLayout ? 1 : ((squareSize+lineGap)*BOARD_WIDTH/8. - 10)/7 + 1;
	    shortName[len] = NULLCHAR;
	    XtSetArg(args[j], XtNlabel, XtNewString(shortName)); j++;
	}
      else {
          XtSetArg(args[j], XtNlabel, XtNewString(mb->name)); j++;
      }

	XtSetArg(args[j], XtNborderWidth, 0);                   j++;
	anchor = XtCreateManagedWidget(mb->name, menuButtonWidgetClass,
				       menuBar, args, j);
	CreateMenuBarPopup(menuBar, menuName, mb);
	mb++;
    }
    return menuBar;
}

Widget CreateButtonBar(mi)
     MenuItem *mi;
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
CreatePieceMenu(name, color)
     char *name;
     int color;
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
CreatePieceMenus()
{
    int i;
    Widget entry;
    Arg args[16];
    ChessSquare selection;

    whitePieceMenu = CreatePieceMenu("menuW", 0);
    blackPieceMenu = CreatePieceMenu("menuB", 1);

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

void SetupDropMenu()
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

void PieceMenuPopup(w, event, params, num_params)
     Widget w;
     XEvent *event;
     String *params;
     Cardinal *num_params;
{
    String whichMenu; int menuNr = -2;
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

static void PieceMenuSelect(w, piece, junk)
     Widget w;
     ChessSquare piece;
     caddr_t junk;
{
    if (pmFromX < 0 || pmFromY < 0) return;
    EditPositionMenuEvent(piece, pmFromX, pmFromY);
}

static void DropMenuSelect(w, piece, junk)
     Widget w;
     ChessSquare piece;
     caddr_t junk;
{
    if (pmFromX < 0 || pmFromY < 0) return;
    DropMenuEvent(piece, pmFromX, pmFromY);
}

void WhiteClock(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ClockClick(0);
}

void BlackClock(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ClockClick(1);
}


/*
 * If the user selects on a border boundary, return -1; if off the board,
 *   return -2.  Otherwise map the event coordinate to the square.
 */
int EventToSquare(x, limit)
     int x;
{
    if (x <= 0)
      return -2;
    if (x < lineGap)
      return -1;
    x -= lineGap;
    if ((x % (squareSize + lineGap)) >= squareSize)
      return -1;
    x /= (squareSize + lineGap);
    if (x >= limit)
      return -2;
    return x;
}

static void do_flash_delay(msec)
     unsigned long msec;
{
    TimeDelay(msec);
}

static void drawHighlight(file, rank, gc)
     int file, rank;
     GC gc;
{
    int x, y;

    if (lineGap == 0) return;

    if (flipView) {
	x = lineGap/2 + ((BOARD_WIDTH-1)-file) *
	  (squareSize + lineGap);
	y = lineGap/2 + rank * (squareSize + lineGap);
    } else {
	x = lineGap/2 + file * (squareSize + lineGap);
	y = lineGap/2 + ((BOARD_HEIGHT-1)-rank) *
	  (squareSize + lineGap);
    }

    XDrawRectangle(xDisplay, xBoardWindow, gc, x, y,
		   squareSize+lineGap, squareSize+lineGap);
}

int hi1X = -1, hi1Y = -1, hi2X = -1, hi2Y = -1;
int pm1X = -1, pm1Y = -1, pm2X = -1, pm2Y = -1;

void
SetHighlights(fromX, fromY, toX, toY)
     int fromX, fromY, toX, toY;
{
    if (hi1X != fromX || hi1Y != fromY) {
	if (hi1X >= 0 && hi1Y >= 0) {
	    drawHighlight(hi1X, hi1Y, lineGC);
	}
    } // [HGM] first erase both, then draw new!
    if (hi2X != toX || hi2Y != toY) {
	if (hi2X >= 0 && hi2Y >= 0) {
	    drawHighlight(hi2X, hi2Y, lineGC);
	}
    }
    if (hi1X != fromX || hi1Y != fromY) {
	if (fromX >= 0 && fromY >= 0) {
	    drawHighlight(fromX, fromY, highlineGC);
	}
    }
    if (hi2X != toX || hi2Y != toY) {
	if (toX >= 0 && toY >= 0) {
	    drawHighlight(toX, toY, highlineGC);
	}
    }
    hi1X = fromX;
    hi1Y = fromY;
    hi2X = toX;
    hi2Y = toY;
}

void
ClearHighlights()
{
    SetHighlights(-1, -1, -1, -1);
}


void
SetPremoveHighlights(fromX, fromY, toX, toY)
     int fromX, fromY, toX, toY;
{
    if (pm1X != fromX || pm1Y != fromY) {
	if (pm1X >= 0 && pm1Y >= 0) {
	    drawHighlight(pm1X, pm1Y, lineGC);
	}
	if (fromX >= 0 && fromY >= 0) {
	    drawHighlight(fromX, fromY, prelineGC);
	}
    }
    if (pm2X != toX || pm2Y != toY) {
	if (pm2X >= 0 && pm2Y >= 0) {
	    drawHighlight(pm2X, pm2Y, lineGC);
	}
	if (toX >= 0 && toY >= 0) {
	    drawHighlight(toX, toY, prelineGC);
	}
    }
    pm1X = fromX;
    pm1Y = fromY;
    pm2X = toX;
    pm2Y = toY;
}

void
ClearPremoveHighlights()
{
  SetPremoveHighlights(-1, -1, -1, -1);
}

static int CutOutSquare(x, y, x0, y0, kind)
     int x, y, *x0, *y0, kind;
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

static void BlankSquare(x, y, color, piece, dest, fac)
     int x, y, color, fac;
     ChessSquare piece;
     Drawable dest;
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
static void monoDrawPiece_1bit(piece, square_color, x, y, dest)
     ChessSquare piece;
     int square_color, x, y;
     Drawable dest;
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

static void monoDrawPiece(piece, square_color, x, y, dest)
     ChessSquare piece;
     int square_color, x, y;
     Drawable dest;
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

static void colorDrawPiece(piece, square_color, x, y, dest)
     ChessSquare piece;
     int square_color, x, y;
     Drawable dest;
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

static void colorDrawPieceImage(piece, square_color, x, y, dest)
     ChessSquare piece;
     int square_color, x, y;
     Drawable dest;
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

DrawFunc ChooseDrawFunc()
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

/* [HR] determine square color depending on chess variant. */
static int SquareColor(row, column)
     int row, column;
{
    int square_color;

    if (gameInfo.variant == VariantXiangqi) {
        if (column >= 3 && column <= 5 && row >= 0 && row <= 2) {
            square_color = 1;
        } else if (column >= 3 && column <= 5 && row >= 7 && row <= 9) {
            square_color = 0;
        } else if (row <= 4) {
            square_color = 0;
        } else {
            square_color = 1;
        }
    } else {
        square_color = ((column + row) % 2) == 1;
    }

    /* [hgm] holdings: next line makes all holdings squares light */
    if(column < BOARD_LEFT || column >= BOARD_RGHT) square_color = 1;

    return square_color;
}

void DrawSquare(row, column, piece, do_flash)
     int row, column, do_flash;
     ChessSquare piece;
{
    int square_color, x, y, direction, font_ascent, font_descent;
    int i;
    char string[2];
    XCharStruct overall;
    DrawFunc drawfunc;
    int flash_delay;

    /* Calculate delay in milliseconds (2-delays per complete flash) */
    flash_delay = 500 / appData.flashRate;

    if (flipView) {
	x = lineGap + ((BOARD_WIDTH-1)-column) *
	  (squareSize + lineGap);
	y = lineGap + row * (squareSize + lineGap);
    } else {
	x = lineGap + column * (squareSize + lineGap);
	y = lineGap + ((BOARD_HEIGHT-1)-row) *
	  (squareSize + lineGap);
    }

    if(twoBoards && partnerUp) x += hOffset; // [HGM] dual: draw second board

    square_color = SquareColor(row, column);

    if ( // [HGM] holdings: blank out area between board and holdings
                 column == BOARD_LEFT-1 ||  column == BOARD_RGHT
              || (column == BOARD_LEFT-2 && row < BOARD_HEIGHT-gameInfo.holdingsSize)
	          || (column == BOARD_RGHT+1 && row >= gameInfo.holdingsSize) ) {
			BlankSquare(x, y, 2, EmptySquare, xBoardWindow, 1);

			// [HGM] print piece counts next to holdings
			string[1] = NULLCHAR;
			if (column == (flipView ? BOARD_LEFT-1 : BOARD_RGHT) && piece > 1 ) {
			    string[0] = '0' + piece;
			    XTextExtents(countFontStruct, string, 1, &direction,
				 &font_ascent, &font_descent, &overall);
			    if (appData.monoMode) {
				XDrawImageString(xDisplay, xBoardWindow, countGC,
						 x + squareSize - overall.width - 2,
						 y + font_ascent + 1, string, 1);
			    } else {
				XDrawString(xDisplay, xBoardWindow, countGC,
					    x + squareSize - overall.width - 2,
					    y + font_ascent + 1, string, 1);
			    }
			}
			if (column == (flipView ? BOARD_RGHT : BOARD_LEFT-1) && piece > 1) {
			    string[0] = '0' + piece;
			    XTextExtents(countFontStruct, string, 1, &direction,
					 &font_ascent, &font_descent, &overall);
			    if (appData.monoMode) {
				XDrawImageString(xDisplay, xBoardWindow, countGC,
						 x + 2, y + font_ascent + 1, string, 1);
			    } else {
				XDrawString(xDisplay, xBoardWindow, countGC,
					    x + 2, y + font_ascent + 1, string, 1);
			    }
			}
    } else {
	    if (piece == EmptySquare || appData.blindfold) {
			BlankSquare(x, y, square_color, piece, xBoardWindow, 1);
	    } else {
			drawfunc = ChooseDrawFunc();

			if (do_flash && appData.flashCount > 0) {
			    for (i=0; i<appData.flashCount; ++i) {
					drawfunc(piece, square_color, x, y, xBoardWindow);
					XSync(xDisplay, False);
					do_flash_delay(flash_delay);

					BlankSquare(x, y, square_color, piece, xBoardWindow, 1);
					XSync(xDisplay, False);
					do_flash_delay(flash_delay);
			    }
			}
			drawfunc(piece, square_color, x, y, xBoardWindow);
    	}
	}

    string[1] = NULLCHAR;
    if (appData.showCoords && row == (flipView ? BOARD_HEIGHT-1 : 0)
		&& column >= BOARD_LEFT && column < BOARD_RGHT) {
	string[0] = 'a' + column - BOARD_LEFT;
	XTextExtents(coordFontStruct, string, 1, &direction,
		     &font_ascent, &font_descent, &overall);
	if (appData.monoMode) {
	    XDrawImageString(xDisplay, xBoardWindow, coordGC,
			     x + squareSize - overall.width - 2,
			     y + squareSize - font_descent - 1, string, 1);
	} else {
	    XDrawString(xDisplay, xBoardWindow, coordGC,
			x + squareSize - overall.width - 2,
			y + squareSize - font_descent - 1, string, 1);
	}
    }
    if (appData.showCoords && column == (flipView ? BOARD_RGHT-1 : BOARD_LEFT)) {
	string[0] = ONE + row;
	XTextExtents(coordFontStruct, string, 1, &direction,
		     &font_ascent, &font_descent, &overall);
	if (appData.monoMode) {
	    XDrawImageString(xDisplay, xBoardWindow, coordGC,
			     x + 2, y + font_ascent + 1, string, 1);
	} else {
	    XDrawString(xDisplay, xBoardWindow, coordGC,
			x + 2, y + font_ascent + 1, string, 1);
	}
    }
    if(!partnerUp && marker[row][column]) {
	XFillArc(xDisplay, xBoardWindow, marker[row][column] == 2 ? prelineGC : highlineGC,
		x + squareSize/4, y+squareSize/4, squareSize/2, squareSize/2, 0, 64*360);
    }
}


/* Why is this needed on some versions of X? */
void EventProc(widget, unused, event)
     Widget widget;
     caddr_t unused;
     XEvent *event;
{
    if (!XtIsRealized(widget))
      return;

    switch (event->type) {
      case Expose:
	if (event->xexpose.count > 0) return;  /* no clipping is done */
	XDrawPosition(widget, True, NULL);
	if(twoBoards) { // [HGM] dual: draw other board in other orientation
	    flipView = !flipView; partnerUp = !partnerUp;
	    XDrawPosition(widget, True, NULL);
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

void DrawPosition(fullRedraw, board)
     /*Boolean*/int fullRedraw;
     Board board;
{
    XDrawPosition(boardWidget, fullRedraw, board);
}

/* Returns 1 if there are "too many" differences between b1 and b2
   (i.e. more than 1 move was made) */
static int too_many_diffs(b1, b2)
     Board b1, b2;
{
    int i, j;
    int c = 0;

    for (i=0; i<BOARD_HEIGHT; ++i) {
	for (j=0; j<BOARD_WIDTH; ++j) {
	    if (b1[i][j] != b2[i][j]) {
		if (++c > 4)	/* Castling causes 4 diffs */
		  return 1;
	    }
	}
    }
    return 0;
}

/* Matrix describing castling maneuvers */
/* Row, ColRookFrom, ColKingFrom, ColRookTo, ColKingTo */
static int castling_matrix[4][5] = {
    { 0, 0, 4, 3, 2 },		/* 0-0-0, white */
    { 0, 7, 4, 5, 6 },		/* 0-0,   white */
    { 7, 0, 4, 3, 2 },		/* 0-0-0, black */
    { 7, 7, 4, 5, 6 }		/* 0-0,   black */
};

/* Checks whether castling occurred. If it did, *rrow and *rcol
   are set to the destination (row,col) of the rook that moved.

   Returns 1 if castling occurred, 0 if not.

   Note: Only handles a max of 1 castling move, so be sure
   to call too_many_diffs() first.
   */
static int check_castle_draw(newb, oldb, rrow, rcol)
     Board newb, oldb;
     int *rrow, *rcol;
{
    int i, *r, j;
    int match;

    /* For each type of castling... */
    for (i=0; i<4; ++i) {
	r = castling_matrix[i];

	/* Check the 4 squares involved in the castling move */
	match = 0;
	for (j=1; j<=4; ++j) {
	    if (newb[r[0]][r[j]] == oldb[r[0]][r[j]]) {
		match = 1;
		break;
	    }
	}

	if (!match) {
	    /* All 4 changed, so it must be a castling move */
	    *rrow = r[0];
	    *rcol = r[3];
	    return 1;
	}
    }
    return 0;
}

// [HGM] seekgraph: some low-level drawing routines cloned from xevalgraph
void DrawSeekAxis( int x, int y, int xTo, int yTo )
{
      XDrawLine(xDisplay, xBoardWindow, lineGC, x, y, xTo, yTo);
}

void DrawSeekBackground( int left, int top, int right, int bottom )
{
    XFillRectangle(xDisplay, xBoardWindow, lightSquareGC, left, top, right-left, bottom-top);
}

void DrawSeekText(char *buf, int x, int y)
{
    XDrawString(xDisplay, xBoardWindow, coordGC, x, y+4, buf, strlen(buf));
}

void DrawSeekDot(int x, int y, int colorNr)
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

static int damage[2][BOARD_RANKS][BOARD_FILES];

/*
 * event handler for redrawing the board
 */
void XDrawPosition(w, repaint, board)
     Widget w;
     /*Boolean*/int repaint;
     Board board;
{
    int i, j, do_flash;
    static int lastFlipView = 0;
    static int lastBoardValid[2] = {0, 0};
    static Board lastBoard[2];
    Arg args[16];
    int rrow, rcol;
    int nr = twoBoards*partnerUp;

    if(DrawSeekGraph()) return; // [HGM] seekgraph: suppress any drawing if seek graph up

    if (board == NULL) {
	if (!lastBoardValid[nr]) return;
	board = lastBoard[nr];
    }
    if (!lastBoardValid[nr] || (nr == 0 && lastFlipView != flipView)) {
	XtSetArg(args[0], XtNleftBitmap, (flipView ? xMarkPixmap : None));
	XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Flip View"),
		    args, 1);
    }

    /*
     * It would be simpler to clear the window with XClearWindow()
     * but this causes a very distracting flicker.
     */

    if (!repaint && lastBoardValid[nr] && (nr == 1 || lastFlipView == flipView)) {

	if ( lineGap && IsDrawArrowEnabled())
	    XDrawSegments(xDisplay, xBoardWindow, lineGC,
			gridSegments, BOARD_HEIGHT + BOARD_WIDTH + 2);

	/* If too much changes (begin observing new game, etc.), don't
	   do flashing */
	do_flash = too_many_diffs(board, lastBoard[nr]) ? 0 : 1;

	/* Special check for castling so we don't flash both the king
	   and the rook (just flash the king). */
	if (do_flash) {
	    if (check_castle_draw(board, lastBoard[nr], &rrow, &rcol)) {
		/* Draw rook with NO flashing. King will be drawn flashing later */
		DrawSquare(rrow, rcol, board[rrow][rcol], 0);
		lastBoard[nr][rrow][rcol] = board[rrow][rcol];
	    }
	}

	/* First pass -- Draw (newly) empty squares and repair damage.
	   This prevents you from having a piece show up twice while it
	   is flashing on its new square */
	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++)
	    if ((board[i][j] != lastBoard[nr][i][j] && board[i][j] == EmptySquare)
		|| damage[nr][i][j]) {
		DrawSquare(i, j, board[i][j], 0);
		damage[nr][i][j] = False;
	    }

	/* Second pass -- Draw piece(s) in new position and flash them */
	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++)
	    if (board[i][j] != lastBoard[nr][i][j]) {
		DrawSquare(i, j, board[i][j], do_flash);
	    }
    } else {
	if (lineGap > 0)
	  XDrawSegments(xDisplay, xBoardWindow, lineGC,
			twoBoards & partnerUp ? secondSegments : // [HGM] dual
			gridSegments, BOARD_HEIGHT + BOARD_WIDTH + 2);

	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++) {
	      DrawSquare(i, j, board[i][j], 0);
	      damage[nr][i][j] = False;
	  }
    }

    CopyBoard(lastBoard[nr], board);
    lastBoardValid[nr] = 1;
  if(nr == 0) { // [HGM] dual: no highlights on second board yet
    lastFlipView = flipView;

    /* Draw highlights */
    if (pm1X >= 0 && pm1Y >= 0) {
      drawHighlight(pm1X, pm1Y, prelineGC);
    }
    if (pm2X >= 0 && pm2Y >= 0) {
      drawHighlight(pm2X, pm2Y, prelineGC);
    }
    if (hi1X >= 0 && hi1Y >= 0) {
      drawHighlight(hi1X, hi1Y, highlineGC);
    }
    if (hi2X >= 0 && hi2Y >= 0) {
      drawHighlight(hi2X, hi2Y, highlineGC);
    }
    DrawArrowHighlight(hi1X, hi1Y, hi2X, hi2Y);
  }
    /* If piece being dragged around board, must redraw that too */
    DrawDragPiece();

    XSync(xDisplay, False);
}


/*
 * event handler for redrawing the board
 */
void DrawPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    XDrawPosition(w, True, NULL);
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
void HandleUserMove(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
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

void AnimateUserMove (Widget w, XEvent * event,
		      String * params, Cardinal * nParams)
{
    DragPieceMove(event->xmotion.x, event->xmotion.y);
}

void HandlePV (Widget w, XEvent * event,
		      String * params, Cardinal * nParams)
{   // [HGM] pv: walk PV
    MovePV(event->xmotion.x, event->xmotion.y, lineGap + BOARD_HEIGHT * (squareSize + lineGap));
}

Widget CommentCreate(name, text, mutable, callback, lines)
     char *name, *text;
     int /*Boolean*/ mutable;
     XtCallbackProc callback;
     int lines;
{
    Arg args[16];
    Widget shell, layout, form, edit, b_ok, b_cancel, b_clear, b_close, b_edit;
    Dimension bw_width;
    int j;

    j = 0;
    XtSetArg(args[j], XtNwidth, &bw_width);  j++;
    XtGetValues(boardWidget, args, j);

    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
#if TOPLEVEL
    shell =
      XtCreatePopupShell(name, topLevelShellWidgetClass,
			 shellWidget, args, j);
#else
    shell =
      XtCreatePopupShell(name, transientShellWidgetClass,
			 shellWidget, args, j);
#endif
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));

    j = 0;
    if (mutable) {
	XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
	XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
    }
    XtSetArg(args[j], XtNstring, text);  j++;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNwidth, bw_width);  j++; /*force wider than buttons*/
    /* !!Work around an apparent bug in XFree86 4.0.1 (X11R6.4.3) */
    XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++;
    XtSetArg(args[j], XtNautoFill, True);  j++;
    XtSetArg(args[j], XtNwrap, XawtextWrapWord); j++;
    edit =
      XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j);
    XtOverrideTranslations(edit, XtParseTranslationTable(commentTranslations));

    if (mutable) {
	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_ok =
	  XtCreateManagedWidget(_("ok"), commandWidgetClass, form, args, j);
	XtAddCallback(b_ok, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNfromHoriz, b_ok);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_cancel =
	  XtCreateManagedWidget(_("cancel"), commandWidgetClass, form, args, j);
	XtAddCallback(b_cancel, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNfromHoriz, b_cancel);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_clear =
	  XtCreateManagedWidget(_("clear"), commandWidgetClass, form, args, j);
	XtAddCallback(b_clear, XtNcallback, callback, (XtPointer) 0);
    } else {
	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_close =
	  XtCreateManagedWidget(_("close"), commandWidgetClass, form, args, j);
	XtAddCallback(b_close, XtNcallback, callback, (XtPointer) 0);

	j = 0;
	XtSetArg(args[j], XtNfromVert, edit);  j++;
	XtSetArg(args[j], XtNfromHoriz, b_close);  j++;
	XtSetArg(args[j], XtNtop, XtChainBottom); j++;
	XtSetArg(args[j], XtNbottom, XtChainBottom); j++;
	XtSetArg(args[j], XtNleft, XtChainLeft); j++;
	XtSetArg(args[j], XtNright, XtChainLeft); j++;
	b_edit =
	  XtCreateManagedWidget(_("edit"), commandWidgetClass, form, args, j);
	XtAddCallback(b_edit, XtNcallback, callback, (XtPointer) 0);
    }

    XtRealizeWidget(shell);

    if (commentX == -1) {
	int xx, yy;
	Window junk;
	Dimension pw_height;
	Dimension ew_height;

	j = 0;
	XtSetArg(args[j], XtNheight, &ew_height);  j++;
	XtGetValues(edit, args, j);

	j = 0;
	XtSetArg(args[j], XtNheight, &pw_height);  j++;
	XtGetValues(shell, args, j);
	commentH = pw_height + (lines - 1) * ew_height;
	commentW = bw_width - 16;

	XSync(xDisplay, False);
#ifdef NOTDEF
	/* This code seems to tickle an X bug if it is executed too soon
	   after xboard starts up.  The coordinates get transformed as if
	   the main window was positioned at (0, 0).
	   */
	XtTranslateCoords(shellWidget,
			  (bw_width - commentW) / 2, 0 - commentH / 2,
			  &commentX, &commentY);
#else  /*!NOTDEF*/
        XTranslateCoordinates(xDisplay, XtWindow(shellWidget),
			      RootWindowOfScreen(XtScreen(shellWidget)),
			      (bw_width - commentW) / 2, 0 - commentH / 2,
			      &xx, &yy, &junk);
	commentX = xx;
	commentY = yy;
#endif /*!NOTDEF*/
	if (commentY < 0) commentY = 0; /*avoid positioning top offscreen*/
    }

    if(wpComment.width > 0) {
      commentX = wpComment.x;
      commentY = wpComment.y;
      commentW = wpComment.width;
      commentH = wpComment.height;
    }

    j = 0;
    XtSetArg(args[j], XtNheight, commentH);  j++;
    XtSetArg(args[j], XtNwidth, commentW);  j++;
    XtSetArg(args[j], XtNx, commentX);  j++;
    XtSetArg(args[j], XtNy, commentY);  j++;
    XtSetValues(shell, args, j);
    XtSetKeyboardFocus(shell, edit);

    return shell;
}

static int savedIndex;  /* gross that this is global */

void CommentClick (Widget w, XEvent * event, String * params, Cardinal * nParams)
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

void EditCommentPopUp(index, title, text)
     int index;
     char *title, *text;
{
    Widget edit;
    Arg args[16];
    int j;

    savedIndex = index;
    if (text == NULL) text = "";

    if (editShell == NULL) {
	editShell =
	  CommentCreate(title, text, True, EditCommentCallback, 4);
	XtRealizeWidget(editShell);
	CatchDeleteWindow(editShell, "EditCommentPopDown");
    } else {
	edit = XtNameToWidget(editShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, text); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(editShell, args, j);
    }

    XtPopup(editShell, XtGrabNone);

    editUp = True;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuEdit.Edit Comment"),
		args, j);
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Comments"),
		args, j);
}

void EditCommentCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name, val;
    Arg args[16];
    int j;
    Widget edit;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("ok")) == 0) {
	edit = XtNameToWidget(editShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, &val); j++;
	XtGetValues(edit, args, j);
	ReplaceComment(savedIndex, val);
	EditCommentPopDown();
    } else if (strcmp(name, _("cancel")) == 0) {
	EditCommentPopDown();
    } else if (strcmp(name, _("clear")) == 0) {
	edit = XtNameToWidget(editShell, "*form.text");
	XtCallActionProc(edit, "select-all", NULL, NULL, 0);
	XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
    }
}

void EditCommentPopDown()
{
    Arg args[16];
    int j;

    if (!editUp) return;
    j = 0;
    XtSetArg(args[j], XtNx, &commentX); j++;
    XtSetArg(args[j], XtNy, &commentY); j++;
    XtSetArg(args[j], XtNheight, &commentH); j++;
    XtSetArg(args[j], XtNwidth, &commentW); j++;
    XtGetValues(editShell, args, j);
    XtPopdown(editShell);
    editUp = False;
    j = 0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuEdit.Edit Comment"),
		args, j);
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Comments"),
		args, j);
}

void ICSInputBoxPopUp()
{
    InputBoxPopup();
}

extern Option boxOptions[];

void ICSInputSendText()
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

void ICSInputBoxPopDown()
{
    PopDown(4);
}

void CommentPopUp(title, text)
     char *title, *text;
{
    Arg args[16];
    int j;
    Widget edit;

    savedIndex = currentMove; // [HGM] vari
    if (commentShell == NULL) {
	commentShell =
	  CommentCreate(title, text, False, CommentCallback, 4);
	XtRealizeWidget(commentShell);
	CatchDeleteWindow(commentShell, "CommentPopDown");
    } else {
	edit = XtNameToWidget(commentShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, text); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) title);   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) title);      j++;
	XtSetValues(commentShell, args, j);
    }

    XtPopup(commentShell, XtGrabNone);
    XSync(xDisplay, False);

    commentUp = True;
}

void CommentCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int j;

    j = 0;
    XtSetArg(args[j], XtNlabel, &name);  j++;
    XtGetValues(w, args, j);

    if (strcmp(name, _("close")) == 0) {
	CommentPopDown();
    } else if (strcmp(name, _("edit")) == 0) {
	CommentPopDown();
	EditCommentEvent();
    }
}


void CommentPopDown()
{
    Arg args[16];
    int j;

    if (!commentUp) return;
    j = 0;
    XtSetArg(args[j], XtNx, &commentX); j++;
    XtSetArg(args[j], XtNy, &commentY); j++;
    XtSetArg(args[j], XtNwidth, &commentW); j++;
    XtSetArg(args[j], XtNheight, &commentH); j++;
    XtGetValues(commentShell, args, j);
    XtPopdown(commentShell);
    XSync(xDisplay, False);
    commentUp = False;
}

void FileNamePopUp(label, def, filter, proc, openMode)
     char *label;
     char *def;
     char *filter;
     FileProc proc;
     char *openMode;
{
    fileProc = proc;		/* I can't see a way not */
    fileOpenMode = openMode;	/*   to use globals here */
    {   // [HGM] use file-selector dialog stolen from Ghostview
	char *name;
	int index; // this is not supported yet
	FILE *f;
	if(f = XsraSelFile(shellWidget, label, NULL, NULL, "could not open: ",
			   (def[0] ? def : NULL), filter, openMode, NULL, &name))
	  (void) (*fileProc)(f, index=0, name);
    }
}

void FileNamePopDown()
{
    if (!filenameUp) return;
    XtPopdown(fileNameShell);
    XtDestroyWidget(fileNameShell);
    filenameUp = False;
    ModeHighlight();
}

void FileNameCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
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

void FileNameAction(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
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

void PromotionPopUp()
{
    Arg args[16];
    Widget dialog, layout;
    Position x, y;
    Dimension bw_width, pw_width;
    int j;

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
      XawDialogAddButton(dialog, _("Warlord"), PromotionCallback,
			 (XtPointer) dialog);
      XawDialogAddButton(dialog, _("General"), PromotionCallback,
			 (XtPointer) dialog);
      XawDialogAddButton(dialog, _("Lieutenant"), PromotionCallback,
			 (XtPointer) dialog);
      XawDialogAddButton(dialog, _("Captain"), PromotionCallback,
			 (XtPointer) dialog);
    } else {
    XawDialogAddButton(dialog, _("Queen"), PromotionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("Rook"), PromotionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("Bishop"), PromotionCallback,
		       (XtPointer) dialog);
    XawDialogAddButton(dialog, _("Knight"), PromotionCallback,
		       (XtPointer) dialog);
    }
    if (!appData.testLegality || gameInfo.variant == VariantSuicide ||
        gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove) ||
        gameInfo.variant == VariantGiveaway) {
      XawDialogAddButton(dialog, _("King"), PromotionCallback,
			 (XtPointer) dialog);
    }
    if(gameInfo.variant == VariantCapablanca ||
       gameInfo.variant == VariantGothic ||
       gameInfo.variant == VariantCapaRandom) {
      XawDialogAddButton(dialog, _("Archbishop"), PromotionCallback,
			 (XtPointer) dialog);
      XawDialogAddButton(dialog, _("Chancellor"), PromotionCallback,
			 (XtPointer) dialog);
    }
  } else // [HGM] shogi
  {
      XawDialogAddButton(dialog, _("Promote"), PromotionCallback,
			 (XtPointer) dialog);
      XawDialogAddButton(dialog, _("Defer"), PromotionCallback,
			 (XtPointer) dialog);
  }
    XawDialogAddButton(dialog, _("cancel"), PromotionCallback,
		       (XtPointer) dialog);

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

void PromotionPopDown()
{
    if (!promotionUp) return;
    XtPopdown(promotionShell);
    XtDestroyWidget(promotionShell);
    promotionUp = False;
}

void PromotionCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    String name;
    Arg args[16];
    int promoChar;

    XtSetArg(args[0], XtNlabel, &name);
    XtGetValues(w, args, 1);

    PromotionPopDown();

    if (fromX == -1) return;

    if (strcmp(name, _("cancel")) == 0) {
	fromX = fromY = -1;
	ClearHighlights();
	return;
    } else if (strcmp(name, _("Knight")) == 0) {
	promoChar = 'n';
    } else if (strcmp(name, _("Promote")) == 0) {
	promoChar = '+';
    } else if (strcmp(name, _("Defer")) == 0) {
	promoChar = '=';
    } else {
	promoChar = ToLower(name[0]);
    }

    UserMoveEvent(fromX, fromY, toX, toY, promoChar);

    if (!appData.highlightLastMove || gotPremove) ClearHighlights();
    if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
    fromX = fromY = -1;
}


void ErrorCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    errorUp = False;
    XtPopdown(w = XtParent(XtParent(XtParent(w))));
    XtDestroyWidget(w);
    if (errorExitStatus != -1) ExitEvent(errorExitStatus);
}


void ErrorPopDown()
{
    if (!errorUp) return;
    errorUp = False;
    XtPopdown(errorShell);
    XtDestroyWidget(errorShell);
    if (errorExitStatus != -1) ExitEvent(errorExitStatus);
}

void ErrorPopUp(title, label, modal)
     char *title, *label;
     int modal;
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
			 shellWidget, args, i);
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
void FreezeUI()
{
  if (frozen) return;
  /* Grab by a widget that doesn't accept input */
  XtAddGrab(messageWidget, TRUE, FALSE);
  frozen = 1;
}

/* Undo a FreezeUI */
void ThawUI()
{
  if (!frozen) return;
  XtRemoveGrab(messageWidget);
  frozen = 0;
}

char *ModeToWidgetName(mode)
     GameMode mode;
{
    switch (mode) {
      case BeginningOfGame:
	if (appData.icsActive)
	  return "menuMode.ICS Client";
	else if (appData.noChessProgram ||
		 *appData.cmailGameName != NULLCHAR)
	  return "menuMode.Edit Game";
	else
	  return "menuMode.Machine Black";
      case MachinePlaysBlack:
	return "menuMode.Machine Black";
      case MachinePlaysWhite:
	return "menuMode.Machine White";
      case AnalyzeMode:
	return "menuMode.Analysis Mode";
      case AnalyzeFile:
	return "menuMode.Analyze File";
      case TwoMachinesPlay:
	return "menuMode.Two Machines";
      case EditGame:
	return "menuMode.Edit Game";
      case PlayFromGameFile:
	return "menuFile.Load Game";
      case EditPosition:
	return "menuMode.Edit Position";
      case Training:
	return "menuMode.Training";
      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case IcsObserving:
      case IcsIdle:
      case IcsExamining:
	return "menuMode.ICS Client";
      default:
      case EndOfGame:
	return NULL;
    }
}

void ModeHighlight()
{
    Arg args[16];
    static int oldPausing = FALSE;
    static GameMode oldmode = (GameMode) -1;
    char *wname;

    if (!boardWidget || !XtIsRealized(boardWidget)) return;

    if (pausing != oldPausing) {
	oldPausing = pausing;
	if (pausing) {
	    XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
	} else {
	    XtSetArg(args[0], XtNleftBitmap, None);
	}
	XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Pause"),
		    args, 1);

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
	XtSetArg(args[0], XtNleftBitmap, None);
	XtSetValues(XtNameToWidget(menuBarWidget, wname), args, 1);
    }
    wname = ModeToWidgetName(gameMode);
    if (wname != NULL) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
	XtSetValues(XtNameToWidget(menuBarWidget, wname), args, 1);
    }
    oldmode = gameMode;

    /* Maybe all the enables should be handled here, not just this one */
    XtSetSensitive(XtNameToWidget(menuBarWidget, "menuMode.Training"),
		   gameMode == Training || gameMode == PlayFromGameFile);
}


/*
 * Button/menu procedures
 */
void ResetProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ResetGameEvent();
}

int LoadGamePopUp(f, gameNumber, title)
     FILE *f;
     int gameNumber;
     char *title;
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

void LoadGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	Reset(FALSE, TRUE);
    }
    FileNamePopUp(_("Load game file name?"), "", ".pgn .game", LoadGamePopUp, "rb");
}

void LoadNextGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadGame(1);
}

void LoadPrevGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadGame(-1);
}

void ReloadGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadGame(0);
}

void LoadNextPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadPosition(1);
}

void LoadPrevPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadPosition(-1);
}

void ReloadPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadPosition(0);
}

void LoadPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	Reset(FALSE, TRUE);
    }
    FileNamePopUp(_("Load position file name?"), "", ".fen .epd .pos", LoadPosition, "rb");
}

void SaveGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    FileNamePopUp(_("Save game file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "game" : "pgn"),
		  appData.oldSaveStyle ? ".game" : ".pgn",
		  SaveGame, "a");
}

void SavePositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    FileNamePopUp(_("Save position file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "pos" : "fen"),
		  appData.oldSaveStyle ? ".pos" : ".fen",
		  SavePosition, "a");
}

void ReloadCmailMsgProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ReloadCmailMsgEvent(FALSE);
}

void MailMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    MailMoveEvent();
}

/* this variable is shared between CopyPositionProc and SendPositionSelection */
char *selected_fen_position=NULL;

Boolean
SendPositionSelection(Widget w, Atom *selection, Atom *target,
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
    *format_return = 8 * sizeof(Atom);
    if (*format_return > 32) {
      *length_return *= *format_return / 32;
      *format_return = 32;
    }
    return True;
  } else {
    return False;
  }
}

/* note: when called from menu all parameters are NULL, so no clue what the
 * Widget which was clicked on was, or what the click event was
 */
void CopyPositionProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
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
PastePositionCB(Widget w, XtPointer client_data, Atom *selection,
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
void PastePositionProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
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
SendGameSelection(Widget w, Atom *selection, Atom *target,
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
    *format_return = 8 * sizeof(Atom);
    if (*format_return > 32) {
      *length_return *= *format_return / 32;
      *format_return = 32;
    }
    return True;
  } else {
    return False;
  }
}

/* note: when called from menu all parameters are NULL, so no clue what the
 * Widget which was clicked on was, or what the click event was
 */
void CopyGameProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
{
  int ret;

  ret = SaveGameToFile(gameCopyFilename, FALSE);
  if (!ret) return;

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

/* function called when the data to Paste is ready */
static void
PasteGameCB(Widget w, XtPointer client_data, Atom *selection,
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
void PasteGameProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
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


void AutoSaveGame()
{
    SaveGameProc(NULL, NULL, NULL, NULL);
}


void QuitProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ExitEvent(0);
}

void PauseProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    PauseEvent();
}


void MachineBlackProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    MachineBlackEvent();
}

void MachineWhiteProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    MachineWhiteEvent();
}

void AnalyzeModeProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    char buf[MSG_SIZ];

    if (!first.analysisSupport) {
      snprintf(buf, sizeof(buf), _("%s does not support analysis"), first.tidy);
      DisplayError(buf, 0);
      return;
    }
    /* [DM] icsEngineAnalyze [HGM] This is horrible code; reverse the gameMode and isEngineAnalyze tests! */
    if (appData.icsActive) {
        if (gameMode != IcsObserving) {
	  snprintf(buf, MSG_SIZ, _("You are not observing a game"));
            DisplayError(buf, 0);
            /* secure check */
            if (appData.icsEngineAnalyze) {
                if (appData.debugMode)
                    fprintf(debugFP, _("Found unexpected active ICS engine analyze \n"));
                ExitAnalyzeMode();
                ModeHighlight();
            }
            return;
        }
        /* if enable, use want disable icsEngineAnalyze */
        if (appData.icsEngineAnalyze) {
                ExitAnalyzeMode();
                ModeHighlight();
                return;
        }
        appData.icsEngineAnalyze = TRUE;
        if (appData.debugMode)
            fprintf(debugFP, _("ICS engine analyze starting... \n"));
    }
#ifndef OPTIONSDIALOG
    if (!appData.showThinking)
      ShowThinkingProc(w,event,prms,nprms);
#endif

    AnalyzeModeEvent();
}

void AnalyzeFileProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (!first.analysisSupport) {
      char buf[MSG_SIZ];
      snprintf(buf, sizeof(buf), _("%s does not support analysis"), first.tidy);
      DisplayError(buf, 0);
      return;
    }
    Reset(FALSE, TRUE);
#ifndef OPTIONSDIALOG
    if (!appData.showThinking)
      ShowThinkingProc(w,event,prms,nprms);
#endif
    AnalyzeFileEvent();
    FileNamePopUp(_("File to analyze"), "", ".pgn .game", LoadGamePopUp, "rb");
    AnalysisPeriodicEvent(1);
}

void TwoMachinesProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    TwoMachinesEvent();
}

void MatchProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    MatchEvent(2);
}

void IcsClientProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    IcsClientEvent();
}

void EditGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    EditGameEvent();
}

void EditPositionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    EditPositionEvent();
}

void TrainingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    TrainingEvent();
}

void EditCommentProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (editUp) {
	EditCommentPopDown();
    } else {
	EditCommentEvent();
    }
}

void IcsInputBoxProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (!PopDown(4)) ICSInputBoxPopUp();
}

void AcceptProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    AcceptEvent();
}

void DeclineProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    DeclineEvent();
}

void RematchProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    RematchEvent();
}

void CallFlagProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    CallFlagEvent();
}

void DrawProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    DrawEvent();
}

void AbortProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    AbortEvent();
}

void AdjournProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    AdjournEvent();
}

void ResignProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ResignEvent();
}

void AdjuWhiteProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    UserAdjudicationEvent(+1);
}

void AdjuBlackProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    UserAdjudicationEvent(-1);
}

void AdjuDrawProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    UserAdjudicationEvent(0);
}

void EnterKeyProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (shellUp[4] == True)
      ICSInputSendText();
}

void UpKeyProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{   // [HGM] input: let up-arrow recall previous line from history
    Widget edit;
    int j;
    Arg args[16];
    String val;
    XawTextBlock t;

    if (!shellUp[4]) return;
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

void DownKeyProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{   // [HGM] input: let down-arrow recall next line from history
    Widget edit;
    String val;
    XawTextBlock t;

    if (!shellUp[4]) return;
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

void StopObservingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    StopObservingEvent();
}

void StopExaminingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    StopExaminingEvent();
}

void UploadProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    UploadGameEvent();
}


void ForwardProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ForwardEvent();
}


void BackwardProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    BackwardEvent();
}

void ToStartProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ToStartEvent();
}

void ToEndProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    ToEndEvent();
}

void RevertProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    RevertEvent(False);
}

void AnnotateProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    RevertEvent(True);
}

void TruncateGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    TruncateGameEvent();
}
void RetractMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    RetractMoveEvent();
}

void MoveNowProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    MoveNowEvent();
}

void FlipViewProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    flipView = !flipView;
    DrawPosition(True, NULL);
}

void PonderNextMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    PonderNextMoveEvent(!appData.ponderNextMove);
#ifndef OPTIONSDIALOG
    if (appData.ponderNextMove) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Ponder Next Move"),
		args, 1);
#endif
}

#ifndef OPTIONSDIALOG
void AlwaysQueenProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.alwaysPromoteToQueen = !appData.alwaysPromoteToQueen;

    if (appData.alwaysPromoteToQueen) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Always Queen"),
		args, 1);
}

void AnimateDraggingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.animateDragging = !appData.animateDragging;

    if (appData.animateDragging) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
        CreateAnimVars();
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Animate Dragging"),
		args, 1);
}

void AnimateMovingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.animate = !appData.animate;

    if (appData.animate) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
        CreateAnimVars();
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Animate Moving"),
		args, 1);
}

void AutoflagProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoCallFlag = !appData.autoCallFlag;

    if (appData.autoCallFlag) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Flag"),
		args, 1);
}

void AutoflipProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.autoFlipView = !appData.autoFlipView;

    if (appData.autoFlipView) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Auto Flip View"),
		args, 1);
}

void BlindfoldProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.blindfold = !appData.blindfold;

    if (appData.blindfold) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Blindfold"),
		args, 1);

    DrawPosition(True, NULL);
}

void TestLegalityProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.testLegality = !appData.testLegality;

    if (appData.testLegality) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Test Legality"),
		args, 1);
}


void FlashMovesProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    if (appData.flashCount == 0) {
	appData.flashCount = 3;
    } else {
	appData.flashCount = -appData.flashCount;
    }

    if (appData.flashCount > 0) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Flash Moves"),
		args, 1);
}

#if HIGHDRAG
void HighlightDraggingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.highlightDragging = !appData.highlightDragging;

    if (appData.highlightDragging) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Highlight Dragging"), args, 1);
}
#endif

void HighlightLastMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.highlightLastMove = !appData.highlightLastMove;

    if (appData.highlightLastMove) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Highlight Last Move"), args, 1);
}

void HighlightArrowProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.highlightMoveWithArrow = !appData.highlightMoveWithArrow;

    if (appData.highlightMoveWithArrow) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Arrow"), args, 1);
}

#if 0
void IcsAlarmProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.icsAlarm = !appData.icsAlarm;

    if (appData.icsAlarm) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.ICS Alarm"), args, 1);
}
#endif

void MoveSoundProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.ringBellAfterMoves = !appData.ringBellAfterMoves;

    if (appData.ringBellAfterMoves) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Move Sound"),
		args, 1);
}

void OneClickProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.oneClick = !appData.oneClick;

    if (appData.oneClick) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.OneClick"),
		args, 1);
}

void PeriodicUpdatesProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    PeriodicUpdatesEvent(!appData.periodicUpdates);

    if (appData.periodicUpdates) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Periodic Updates"),
		args, 1);
}

void PopupExitMessageProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.popupExitMessage = !appData.popupExitMessage;

    if (appData.popupExitMessage) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Popup Exit Message"), args, 1);
}

void PopupMoveErrorsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.popupMoveErrors = !appData.popupMoveErrors;

    if (appData.popupMoveErrors) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Popup Move Errors"),
		args, 1);
}

#if 0
void PremoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.premove = !appData.premove;

    if (appData.premove) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget,
			       "menuOptions.Premove"), args, 1);
}
#endif

void ShowCoordsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.showCoords = !appData.showCoords;

    if (appData.showCoords) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Show Coords"),
		args, 1);

    DrawPosition(True, NULL);
}

void ShowThinkingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.showThinking = !appData.showThinking; // [HGM] thinking: tken out of ShowThinkingEvent
    ShowThinkingEvent();
}

void HideThinkingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    appData.hideThinkingFromHuman = !appData.hideThinkingFromHuman; // [HGM] thinking: tken out of ShowThinkingEvent
    ShowThinkingEvent();

    if (appData.hideThinkingFromHuman) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Hide Thinking"),
		args, 1);
}
#endif

void SaveOnExitProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    saveSettingsOnExit = !saveSettingsOnExit;

    if (saveSettingsOnExit) {
	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
	XtSetArg(args[0], XtNleftBitmap, None);
    }
    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Save Settings on Exit"),
		args, 1);
}

void SaveSettingsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
     SaveSettings(settingsFileName);
}

void InfoProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    char buf[MSG_SIZ];
    snprintf(buf, sizeof(buf), "xterm -e info --directory %s --directory . -f %s &",
	    INFODIR, INFOFILE);
    system(buf);
}

void ManProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    char buf[MSG_SIZ];
    String name;
    if (nprms && *nprms > 0)
      name = prms[0];
    else
      name = "xboard";
    snprintf(buf, sizeof(buf), "xterm -e man %s &", name);
    system(buf);
}

void HintProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    HintEvent();
}

void BookProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    BookEvent();
}

void AboutProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    char buf[MSG_SIZ];
#if ZIPPY
    char *zippy = " (with Zippy code)";
#else
    char *zippy = "";
#endif
    snprintf(buf, sizeof(buf), "%s%s\n\n%s\n%s\n%s\n\n%s%s\n%s",
	    programVersion, zippy,
	    "Copyright 1991 Digital Equipment Corporation",
	    "Enhancements Copyright 1992-2009 Free Software Foundation",
	    "Enhancements Copyright 2005 Alessandro Scotti",
	    PACKAGE, " is free software and carries NO WARRANTY;",
	    "see the file COPYING for more information.");
    ErrorPopUp(_("About XBoard"), buf, FALSE);
}

void DebugProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.debugMode = !appData.debugMode;
}

void AboutGameProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    AboutGameEvent();
}

void NothingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    return;
}

void Iconify(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    Arg args[16];

    fromX = fromY = -1;
    XtSetArg(args[0], XtNiconic, True);
    XtSetValues(shellWidget, args, 1);
}

void DisplayMessage(message, extMessage)
     char *message, *extMessage;
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

void DisplayTitle(text)
     char *text;
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
}


void
DisplayError(message, error)
     String message;
     int error;
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


void DisplayMoveError(message)
     String message;
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


void DisplayFatalError(message, error, status)
     String message;
     int error, status;
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

void DisplayInformation(message)
     String message;
{
    ErrorPopDown();
    ErrorPopUp(_("Information"), message, TRUE);
}

void DisplayNote(message)
     String message;
{
    ErrorPopDown();
    ErrorPopUp(_("Note"), message, FALSE);
}

static int
NullXErrorCheck(dpy, error_event)
     Display *dpy;
     XErrorEvent *error_event;
{
    return 0;
}

void DisplayIcsInteractionTitle(message)
     String message;
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

void AskQuestionProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    if (*nprms != 4) {
	fprintf(stderr, _("AskQuestionProc needed 4 parameters, got %d\n"),
		*nprms);
	return;
    }
    AskQuestionEvent(prms[0], prms[1], prms[2], prms[3]);
}

void AskQuestionPopDown()
{
    if (!askQuestionUp) return;
    XtPopdown(askQuestionShell);
    XtDestroyWidget(askQuestionShell);
    askQuestionUp = False;
}

void AskQuestionReplyAction(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
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

void AskQuestionCallback(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
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

void AskQuestion(title, question, replyPrefix, pr)
     char *title, *question, *replyPrefix;
     ProcRef pr;
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
PlaySound(name)
     char *name;
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
RingBell()
{
  PlaySound(appData.soundMove);
}

void
PlayIcsWinSound()
{
  PlaySound(appData.soundIcsWin);
}

void
PlayIcsLossSound()
{
  PlaySound(appData.soundIcsLoss);
}

void
PlayIcsDrawSound()
{
  PlaySound(appData.soundIcsDraw);
}

void
PlayIcsUnfinishedSound()
{
  PlaySound(appData.soundIcsUnfinished);
}

void
PlayAlarmSound()
{
  PlaySound(appData.soundIcsAlarm);
}

void
EchoOn()
{
    system("stty echo");
    noEcho = False;
}

void
EchoOff()
{
    system("stty -echo");
    noEcho = True;
}

void
Colorize(cc, continuation)
     ColorClass cc;
     int continuation;
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

char *UserName()
{
    return getpwuid(getuid())->pw_name;
}

static char *
ExpandPathName(path)
     char *path;
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

char *HostName()
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
FireDelayedEvent()
{
    delayedEventTimerXID = 0;
    delayedEventCallback();
}

void
ScheduleDelayedEvent(cb, millisec)
     DelayedEventCallback cb; long millisec;
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
GetDelayedEvent()
{
  if (delayedEventTimerXID) {
    return delayedEventCallback;
  } else {
    return NULL;
  }
}

void
CancelDelayedEvent()
{
  if (delayedEventTimerXID) {
    XtRemoveTimeOut(delayedEventTimerXID);
    delayedEventTimerXID = 0;
  }
}

XtIntervalId loadGameTimerXID = 0;

int LoadGameTimerRunning()
{
    return loadGameTimerXID != 0;
}

int StopLoadGameTimer()
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
LoadGameTimerCallback(arg, id)
     XtPointer arg;
     XtIntervalId *id;
{
    loadGameTimerXID = 0;
    AutoPlayGameLoop();
}

void
StartLoadGameTimer(millisec)
     long millisec;
{
    loadGameTimerXID =
      XtAppAddTimeOut(appContext, millisec,
		      (XtTimerCallbackProc) LoadGameTimerCallback,
		      (XtPointer) 0);
}

XtIntervalId analysisClockXID = 0;

void
AnalysisClockCallback(arg, id)
     XtPointer arg;
     XtIntervalId *id;
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile
         || appData.icsEngineAnalyze) { // [DM]
	AnalysisPeriodicEvent(0);
	StartAnalysisClock();
    }
}

void
StartAnalysisClock()
{
    analysisClockXID =
      XtAppAddTimeOut(appContext, 2000,
		      (XtTimerCallbackProc) AnalysisClockCallback,
		      (XtPointer) 0);
}

XtIntervalId clockTimerXID = 0;

int ClockTimerRunning()
{
    return clockTimerXID != 0;
}

int StopClockTimer()
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
ClockTimerCallback(arg, id)
     XtPointer arg;
     XtIntervalId *id;
{
    clockTimerXID = 0;
    DecrementClocks();
}

void
StartClockTimer(millisec)
     long millisec;
{
    clockTimerXID =
      XtAppAddTimeOut(appContext, millisec,
		      (XtTimerCallbackProc) ClockTimerCallback,
		      (XtPointer) 0);
}

void
DisplayTimerLabel(w, color, timer, highlight)
     Widget w;
     char *color;
     long timer;
     int highlight;
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
DisplayWhiteClock(timeRemaining, highlight)
     long timeRemaining;
     int highlight;
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
DisplayBlackClock(timeRemaining, highlight)
     long timeRemaining;
     int highlight;
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


int StartChildProcess(cmdLine, dir, pr)
     char *cmdLine;
     char *dir;
     ProcRef *pr;
{
    char *argv[64], *p;
    int i, pid;
    int to_prog[2], from_prog[2];
    ChildProc *cp;
    char buf[MSG_SIZ];

    if (appData.debugMode) {
	fprintf(stderr, "StartChildProcess (dir=\"%s\") %s\n",dir, cmdLine);
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
static RETSIGTYPE AlarmCallBack(int n)
{
    return;
}

void
DestroyChildProcess(pr, signalType)
     ProcRef pr;
     int signalType;
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
InterruptChildProcess(pr)
     ProcRef pr;
{
    ChildProc *cp = (ChildProc *) pr;

    if (cp->kind != CPReal) return;
    (void) kill(cp->pid, SIGINT); /* stop it thinking */
}

int OpenTelnet(host, port, pr)
     char *host;
     char *port;
     ProcRef *pr;
{
    char cmdLine[MSG_SIZ];

    if (port[0] == NULLCHAR) {
      snprintf(cmdLine, sizeof(cmdLine), "%s %s", appData.telnetProgram, host);
    } else {
      snprintf(cmdLine, sizeof(cmdLine), "%s %s %s", appData.telnetProgram, host, port);
    }
    return StartChildProcess(cmdLine, "", pr);
}

int OpenTCP(host, port, pr)
     char *host;
     char *port;
     ProcRef *pr;
{
#if OMIT_SOCKETS
    DisplayFatalError(_("Socket support is not configured in"), 0, 2);
#else  /* !OMIT_SOCKETS */
    int s;
    struct sockaddr_in sa;
    struct hostent     *hp;
    unsigned short uport;
    ChildProc *cp;

    if ((s = socket(AF_INET, SOCK_STREAM, 6)) < 0) {
	return errno;
    }

    memset((char *) &sa, (int)0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    uport = (unsigned short) 0;
    sa.sin_port = htons(uport);
    if (bind(s, (struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
	return errno;
    }

    memset((char *) &sa, (int)0, sizeof(struct sockaddr_in));
    if (!(hp = gethostbyname(host))) {
	int b0, b1, b2, b3;
	if (sscanf(host, "%d.%d.%d.%d", &b0, &b1, &b2, &b3) == 4) {
	    hp = (struct hostent *) calloc(1, sizeof(struct hostent));
	    hp->h_addrtype = AF_INET;
	    hp->h_length = 4;
	    hp->h_addr_list = (char **) calloc(2, sizeof(char *));
	    hp->h_addr_list[0] = (char *) malloc(4);
	    hp->h_addr_list[0][0] = b0;
	    hp->h_addr_list[0][1] = b1;
	    hp->h_addr_list[0][2] = b2;
	    hp->h_addr_list[0][3] = b3;
	} else {
	    return ENOENT;
	}
    }
    sa.sin_family = hp->h_addrtype;
    uport = (unsigned short) atoi(port);
    sa.sin_port = htons(uport);
    memcpy((char *) &sa.sin_addr, hp->h_addr, hp->h_length);

    if (connect(s, (struct sockaddr *) &sa,
		sizeof(struct sockaddr_in)) < 0) {
	return errno;
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

int OpenCommPort(name, pr)
     char *name;
     ProcRef *pr;
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

int OpenLoopback(pr)
     ProcRef *pr;
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

int OpenRcmd(host, user, cmd, pr)
     char *host, *user, *cmd;
     ProcRef *pr;
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
DoInputCallback(closure, source, xid)
     caddr_t closure;
     int *source;
     XtInputId *xid;
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

InputSourceRef AddInputSource(pr, lineByLine, func, closure)
     ProcRef pr;
     int lineByLine;
     InputCallback func;
     VOIDSTAR closure;
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
RemoveInputSource(isr)
     InputSourceRef isr;
{
    InputSource *is = (InputSource *) isr;

    if (is->xid == 0) return;
    XtRemoveInput(is->xid);
    is->xid = 0;
}

int OutputToProcess(pr, message, count, outError)
     ProcRef pr;
     char *message;
     int count;
     int *outError;
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
int OutputToProcessDelayed(pr, message, count, outError, msdelay)
     ProcRef pr;
     char *message;
     int count;
     int *outError;
     long msdelay;
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

/****	Animation code by Hugh Fisher, DCS, ANU.

	Known problem: if a window overlapping the board is
	moved away while a piece is being animated underneath,
	the newly exposed area won't be updated properly.
	I can live with this.

	Known problem: if you look carefully at the animation
	of pieces in mono mode, they are being drawn as solid
	shapes without interior detail while moving. Fixing
	this would be a major complication for minimal return.
****/

/*	Masks for XPM pieces. Black and white pieces can have
	different shapes, but in the interest of retaining my
	sanity pieces must have the same outline on both light
	and dark squares, and all pieces must use the same
	background square colors/images.		*/

static int xpmDone = 0;

static void
CreateAnimMasks (pieceDepth)
     int pieceDepth;
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
InitAnimState (anim, info)
  AnimState * anim;
  XWindowAttributes * info;
{
  XtGCMask  mask;
  XGCValues values;

  /* Each buffer is square size, same depth as window */
  anim->saveBuf = XCreatePixmap(xDisplay, xBoardWindow,
			squareSize, squareSize, info->depth);
  anim->newBuf = XCreatePixmap(xDisplay, xBoardWindow,
			squareSize, squareSize, info->depth);

  /* Create a plain GC for blitting */
  mask = GCForeground | GCBackground | GCFunction |
         GCPlaneMask | GCGraphicsExposures;
  values.foreground = XBlackPixel(xDisplay, xScreen);
  values.background = XWhitePixel(xDisplay, xScreen);
  values.function   = GXcopy;
  values.plane_mask = AllPlanes;
  values.graphics_exposures = False;
  anim->blitGC = XCreateGC(xDisplay, xBoardWindow, mask, &values);

  /* Piece will be copied from an existing context at
     the start of each new animation/drag. */
  anim->pieceGC = XCreateGC(xDisplay, xBoardWindow, 0, &values);

  /* Outline will be a read-only copy of an existing */
  anim->outlineGC = None;
}

static void
CreateAnimVars ()
{
  XWindowAttributes info;

  if (xpmDone && gameInfo.variant == oldVariant) return;
  if(xpmDone) oldVariant = gameInfo.variant; // first time pieces might not be created yet
  XGetWindowAttributes(xDisplay, xBoardWindow, &info);

  InitAnimState(&game, &info);
  InitAnimState(&player, &info);

  /* For XPM pieces, we need bitmaps to use as masks. */
  if (useImages)
    CreateAnimMasks(info.depth), xpmDone = 1;
}

#ifndef HAVE_USLEEP

static Boolean frameWaiting;

static RETSIGTYPE FrameAlarm (sig)
     int sig;
{
  frameWaiting = False;
  /* In case System-V style signals.  Needed?? */
  signal(SIGALRM, FrameAlarm);
}

static void
FrameDelay (time)
     int time;
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

static void
FrameDelay (time)
     int time;
{
  XSync(xDisplay, False);
  if (time > 0)
    usleep(time * 1000);
}

#endif

/*	Convert board position to corner of screen rect and color	*/

static void
ScreenSquare(column, row, pt, color)
     int column; int row; XPoint * pt; int * color;
{
  if (flipView) {
    pt->x = lineGap + ((BOARD_WIDTH-1)-column) * (squareSize + lineGap);
    pt->y = lineGap + row * (squareSize + lineGap);
  } else {
    pt->x = lineGap + column * (squareSize + lineGap);
    pt->y = lineGap + ((BOARD_HEIGHT-1)-row) * (squareSize + lineGap);
  }
  *color = SquareColor(row, column);
}

/*	Convert window coords to square			*/

static void
BoardSquare(x, y, column, row)
     int x; int y; int * column; int * row;
{
  *column = EventToSquare(x, BOARD_WIDTH);
  if (flipView && *column >= 0)
    *column = BOARD_WIDTH - 1 - *column;
  *row = EventToSquare(y, BOARD_HEIGHT);
  if (!flipView && *row >= 0)
    *row = BOARD_HEIGHT - 1 - *row;
}

/*   Utilities	*/

#undef Max  /* just in case */
#undef Min
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))

static void
SetRect(rect, x, y, width, height)
     XRectangle * rect; int x; int y; int width; int height;
{
  rect->x = x;
  rect->y = y;
  rect->width  = width;
  rect->height = height;
}

/*	Test if two frames overlap. If they do, return
	intersection rect within old and location of
	that rect within new. */

static Boolean
Intersect(old, new, size, area, pt)
     XPoint * old; XPoint * new;
     int size; XRectangle * area; XPoint * pt;
{
  if (old->x > new->x + size || new->x > old->x + size ||
      old->y > new->y + size || new->y > old->y + size) {
    return False;
  } else {
    SetRect(area, Max(new->x - old->x, 0), Max(new->y - old->y, 0),
            size - abs(old->x - new->x), size - abs(old->y - new->y));
    pt->x = Max(old->x - new->x, 0);
    pt->y = Max(old->y - new->y, 0);
    return True;
  }
}

/*	For two overlapping frames, return the rect(s)
	in the old that do not intersect with the new.   */

static void
CalcUpdateRects(old, new, size, update, nUpdates)
     XPoint * old; XPoint * new; int size;
     XRectangle update[]; int * nUpdates;
{
  int	     count;

  /* If old = new (shouldn't happen) then nothing to draw */
  if (old->x == new->x && old->y == new->y) {
    *nUpdates = 0;
    return;
  }
  /* Work out what bits overlap. Since we know the rects
     are the same size we don't need a full intersect calc. */
  count = 0;
  /* Top or bottom edge? */
  if (new->y > old->y) {
    SetRect(&(update[count]), old->x, old->y, size, new->y - old->y);
    count ++;
  } else if (old->y > new->y) {
    SetRect(&(update[count]), old->x, old->y + size - (old->y - new->y),
			      size, old->y - new->y);
    count ++;
  }
  /* Left or right edge - don't overlap any update calculated above. */
  if (new->x > old->x) {
    SetRect(&(update[count]), old->x, Max(new->y, old->y),
			      new->x - old->x, size - abs(new->y - old->y));
    count ++;
  } else if (old->x > new->x) {
    SetRect(&(update[count]), new->x + size, Max(new->y, old->y),
			      old->x - new->x, size - abs(new->y - old->y));
    count ++;
  }
  /* Done */
  *nUpdates = count;
}

/*	Generate a series of frame coords from start->mid->finish.
	The movement rate doubles until the half way point is
	reached, then halves back down to the final destination,
	which gives a nice slow in/out effect. The algorithmn
	may seem to generate too many intermediates for short
	moves, but remember that the purpose is to attract the
	viewers attention to the piece about to be moved and
	then to where it ends up. Too few frames would be less
	noticeable.						*/

static void
Tween(start, mid, finish, factor, frames, nFrames)
     XPoint * start; XPoint * mid;
     XPoint * finish; int factor;
     XPoint frames[]; int * nFrames;
{
  int fraction, n, count;

  count = 0;

  /* Slow in, stepping 1/16th, then 1/8th, ... */
  fraction = 1;
  for (n = 0; n < factor; n++)
    fraction *= 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = start->x + (mid->x - start->x) / fraction;
    frames[count].y = start->y + (mid->y - start->y) / fraction;
    count ++;
    fraction = fraction / 2;
  }

  /* Midpoint */
  frames[count] = *mid;
  count ++;

  /* Slow out, stepping 1/2, then 1/4, ... */
  fraction = 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = finish->x - (finish->x - mid->x) / fraction;
    frames[count].y = finish->y - (finish->y - mid->y) / fraction;
    count ++;
    fraction = fraction * 2;
  }
  *nFrames = count;
}

/*	Draw a piece on the screen without disturbing what's there	*/

static void
SelectGCMask(piece, clip, outline, mask)
     ChessSquare piece; GC * clip; GC * outline; Pixmap * mask;
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
OverlayPiece(piece, clip, outline,  dest)
     ChessSquare piece; GC clip; GC outline; Drawable dest;
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

/* Animate the movement of a single piece */

static void
BeginAnimation(anim, piece, startColor, start)
     AnimState *anim;
     ChessSquare piece;
     int startColor;
     XPoint * start;
{
  Pixmap mask;

  if(appData.upsideDown && flipView) piece += piece < BlackPawn ? BlackPawn : -BlackPawn;
  /* The old buffer is initialised with the start square (empty) */
  BlankSquare(start->x, start->y, startColor, EmptySquare, anim->saveBuf, 0);
  anim->prevFrame = *start;

  /* The piece will be drawn using its own bitmap as a matte	*/
  SelectGCMask(piece, &anim->pieceGC, &anim->outlineGC, &mask);
  XSetClipMask(xDisplay, anim->pieceGC, mask);
}

static void
AnimationFrame(anim, frame, piece)
     AnimState *anim;
     XPoint *frame;
     ChessSquare piece;
{
  XRectangle updates[4];
  XRectangle overlap;
  XPoint     pt;
  int	     count, i;

  /* Save what we are about to draw into the new buffer */
  XCopyArea(xDisplay, xBoardWindow, anim->newBuf, anim->blitGC,
	    frame->x, frame->y, squareSize, squareSize,
	    0, 0);

  /* Erase bits of the previous frame */
  if (Intersect(&anim->prevFrame, frame, squareSize, &overlap, &pt)) {
    /* Where the new frame overlapped the previous,
       the contents in newBuf are wrong. */
    XCopyArea(xDisplay, anim->saveBuf, anim->newBuf, anim->blitGC,
	      overlap.x, overlap.y,
	      overlap.width, overlap.height,
	      pt.x, pt.y);
    /* Repaint the areas in the old that don't overlap new */
    CalcUpdateRects(&anim->prevFrame, frame, squareSize, updates, &count);
    for (i = 0; i < count; i++)
      XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
		updates[i].x - anim->prevFrame.x,
		updates[i].y - anim->prevFrame.y,
		updates[i].width, updates[i].height,
		updates[i].x, updates[i].y);
  } else {
    /* Easy when no overlap */
    XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
		  0, 0, squareSize, squareSize,
		  anim->prevFrame.x, anim->prevFrame.y);
  }

  /* Save this frame for next time round */
  XCopyArea(xDisplay, anim->newBuf, anim->saveBuf, anim->blitGC,
		0, 0, squareSize, squareSize,
		0, 0);
  anim->prevFrame = *frame;

  /* Draw piece over original screen contents, not current,
     and copy entire rect. Wipes out overlapping piece images. */
  OverlayPiece(piece, anim->pieceGC, anim->outlineGC, anim->newBuf);
  XCopyArea(xDisplay, anim->newBuf, xBoardWindow, anim->blitGC,
		0, 0, squareSize, squareSize,
		frame->x, frame->y);
}

static void
EndAnimation (anim, finish)
     AnimState *anim;
     XPoint *finish;
{
  XRectangle updates[4];
  XRectangle overlap;
  XPoint     pt;
  int	     count, i;

  /* The main code will redraw the final square, so we
     only need to erase the bits that don't overlap.	*/
  if (Intersect(&anim->prevFrame, finish, squareSize, &overlap, &pt)) {
    CalcUpdateRects(&anim->prevFrame, finish, squareSize, updates, &count);
    for (i = 0; i < count; i++)
      XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
		updates[i].x - anim->prevFrame.x,
		updates[i].y - anim->prevFrame.y,
		updates[i].width, updates[i].height,
		updates[i].x, updates[i].y);
  } else {
    XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
		0, 0, squareSize, squareSize,
		anim->prevFrame.x, anim->prevFrame.y);
  }
}

static void
FrameSequence(anim, piece, startColor, start, finish, frames, nFrames)
     AnimState *anim;
     ChessSquare piece; int startColor;
     XPoint * start; XPoint * finish;
     XPoint frames[]; int nFrames;
{
  int n;

  BeginAnimation(anim, piece, startColor, start);
  for (n = 0; n < nFrames; n++) {
    AnimationFrame(anim, &(frames[n]), piece);
    FrameDelay(appData.animSpeed);
  }
  EndAnimation(anim, finish);
}

void
AnimateAtomicCapture(Board board, int fromX, int fromY, int toX, int toY)
{
    int i, x, y;
    ChessSquare piece = board[fromY][toY];
    board[fromY][toY] = EmptySquare;
    DrawPosition(FALSE, board);
    if (flipView) {
	x = lineGap + ((BOARD_WIDTH-1)-toX) * (squareSize + lineGap);
	y = lineGap + toY * (squareSize + lineGap);
    } else {
	x = lineGap + toX * (squareSize + lineGap);
	y = lineGap + ((BOARD_HEIGHT-1)-toY) * (squareSize + lineGap);
    }
    for(i=1; i<4*kFactor; i++) {
	int r = squareSize * 9 * i/(20*kFactor - 5);
	XFillArc(xDisplay, xBoardWindow, highlineGC,
		x + squareSize/2 - r, y+squareSize/2 - r, 2*r, 2*r, 0, 64*360);
	FrameDelay(appData.animSpeed);
    }
    board[fromY][toY] = piece;
}

/* Main control logic for deciding what to animate and how */

void
AnimateMove(board, fromX, fromY, toX, toY)
     Board board;
     int fromX;
     int fromY;
     int toX;
     int toY;
{
  ChessSquare piece;
  int hop;
  XPoint      start, finish, mid;
  XPoint      frames[kFactor * 2 + 1];
  int	      nFrames, startColor, endColor;

  /* Are we animating? */
  if (!appData.animate || appData.blindfold)
    return;

  if(board[toY][toX] == WhiteRook && board[fromY][fromX] == WhiteKing ||
     board[toY][toX] == BlackRook && board[fromY][fromX] == BlackKing)
	return; // [HGM] FRC: no animtion of FRC castlings, as to-square is not true to-square

  if (fromY < 0 || fromX < 0 || toX < 0 || toY < 0) return;
  piece = board[fromY][fromX];
  if (piece >= EmptySquare) return;

#if DONT_HOP
  hop = FALSE;
#else
  hop = abs(fromX-toX) == 1 && abs(fromY-toY) == 2 || abs(fromX-toX) == 2 && abs(fromY-toY) == 1;
#endif

  if (appData.debugMode) {
      fprintf(debugFP, hop ? _("AnimateMove: piece %d hops from %d,%d to %d,%d \n") :
                             _("AnimateMove: piece %d slides from %d,%d to %d,%d \n"),
             piece, fromX, fromY, toX, toY);  }

  ScreenSquare(fromX, fromY, &start, &startColor);
  ScreenSquare(toX, toY, &finish, &endColor);

  if (hop) {
    /* Knight: make straight movement then diagonal */
    if (abs(toY - fromY) < abs(toX - fromX)) {
       mid.x = start.x + (finish.x - start.x) / 2;
       mid.y = start.y;
     } else {
       mid.x = start.x;
       mid.y = start.y + (finish.y - start.y) / 2;
     }
  } else {
    mid.x = start.x + (finish.x - start.x) / 2;
    mid.y = start.y + (finish.y - start.y) / 2;
  }

  /* Don't use as many frames for very short moves */
  if (abs(toY - fromY) + abs(toX - fromX) <= 2)
    Tween(&start, &mid, &finish, kFactor - 1, frames, &nFrames);
  else
    Tween(&start, &mid, &finish, kFactor, frames, &nFrames);
  FrameSequence(&game, piece, startColor, &start, &finish, frames, nFrames);
  if(Explode(board, fromX, fromY, toX, toY)) { // mark as damaged
    int i,j;
    for(i=0; i<BOARD_WIDTH; i++) for(j=0; j<BOARD_HEIGHT; j++)
      if((i-toX)*(i-toX) + (j-toY)*(j-toY) < 6) damage[0][j][i] = True;
  }

  /* Be sure end square is redrawn */
  damage[0][toY][toX] = True;
}

void
DragPieceBegin(x, y)
     int x; int y;
{
    int	 boardX, boardY, color;
    XPoint corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Figure out which square we start in and the
       mouse position relative to top left corner. */
    BoardSquare(x, y, &boardX, &boardY);
    player.startBoardX = boardX;
    player.startBoardY = boardY;
    ScreenSquare(boardX, boardY, &corner, &color);
    player.startSquare  = corner;
    player.startColor   = color;
    /* As soon as we start dragging, the piece will jump slightly to
       be centered over the mouse pointer. */
    player.mouseDelta.x = squareSize/2;
    player.mouseDelta.y = squareSize/2;
    /* Initialise animation */
    player.dragPiece = PieceForSquare(boardX, boardY);
    /* Sanity check */
    if (player.dragPiece >= 0 && player.dragPiece < EmptySquare) {
	player.dragActive = True;
	BeginAnimation(&player, player.dragPiece, color, &corner);
	/* Mark this square as needing to be redrawn. Note that
	   we don't remove the piece though, since logically (ie
	   as seen by opponent) the move hasn't been made yet. */
           if(boardX == BOARD_RGHT+1 && PieceForSquare(boardX-1, boardY) > 1 ||
              boardX == BOARD_LEFT-2 && PieceForSquare(boardX+1, boardY) > 1)
           XCopyArea(xDisplay, xBoardWindow, player.saveBuf, player.blitGC,
	             corner.x, corner.y, squareSize, squareSize,
	             0, 0); // [HGM] zh: unstack in stead of grab
           if(gatingPiece != EmptySquare) {
               /* Kludge alert: When gating we want the introduced
                  piece to appear on the from square. To generate an
                  image of it, we draw it on the board, copy the image,
                  and draw the original piece again. */
               ChessSquare piece = boards[currentMove][boardY][boardX];
               DrawSquare(boardY, boardX, gatingPiece, 0);
               XCopyArea(xDisplay, xBoardWindow, player.saveBuf, player.blitGC,
	             corner.x, corner.y, squareSize, squareSize, 0, 0);
               DrawSquare(boardY, boardX, piece, 0);
           }
	damage[0][boardY][boardX] = True;
    } else {
	player.dragActive = False;
    }
}

static void
DragPieceMove(x, y)
     int x; int y;
{
    XPoint corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Sanity check */
    if (! player.dragActive)
      return;
    /* Move piece, maintaining same relative position
       of mouse within square	 */
    corner.x = x - player.mouseDelta.x;
    corner.y = y - player.mouseDelta.y;
    AnimationFrame(&player, &corner, player.dragPiece);
#if HIGHDRAG*0
    if (appData.highlightDragging) {
	int boardX, boardY;
	BoardSquare(x, y, &boardX, &boardY);
	SetHighlights(fromX, fromY, boardX, boardY);
    }
#endif
}

void
DragPieceEnd(x, y)
     int x; int y;
{
    int boardX, boardY, color;
    XPoint corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Sanity check */
    if (! player.dragActive)
      return;
    /* Last frame in sequence is square piece is
       placed on, which may not match mouse exactly. */
    BoardSquare(x, y, &boardX, &boardY);
    ScreenSquare(boardX, boardY, &corner, &color);
    EndAnimation(&player, &corner);

    /* Be sure end square is redrawn */
    damage[0][boardY][boardX] = True;

    /* This prevents weird things happening with fast successive
       clicks which on my Sun at least can cause motion events
       without corresponding press/release. */
    player.dragActive = False;
}

/* Handle expose event while piece being dragged */

static void
DrawDragPiece ()
{
  if (!player.dragActive || appData.blindfold)
    return;

  /* What we're doing: logically, the move hasn't been made yet,
     so the piece is still in it's original square. But visually
     it's being dragged around the board. So we erase the square
     that the piece is on and draw it at the last known drag point. */
  BlankSquare(player.startSquare.x, player.startSquare.y,
		player.startColor, EmptySquare, xBoardWindow, 1);
  AnimationFrame(&player, &player.prevFrame, player.dragPiece);
  damage[0][player.startBoardY][player.startBoardX] = TRUE;
}

#include <sys/ioctl.h>
int get_term_width()
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
update_ics_width()
{
  static int old_width = 0;
  int new_width = get_term_width();

  if (old_width != new_width)
    ics_printf("set width %d\n", new_width);
  old_width = new_width;
}

void NotifyFrontendLogin()
{
    update_ics_width();
}

/* [AS] Arrow highlighting support */

static double A_WIDTH = 5; /* Width of arrow body */

#define A_HEIGHT_FACTOR 6   /* Length of arrow "point", relative to body width */
#define A_WIDTH_FACTOR  3   /* Width of arrow "point", relative to body width */

static double Sqr( double x )
{
    return x*x;
}

static int Round( double x )
{
    return (int) (x + 0.5);
}

void SquareToPos(int rank, int file, int *x, int *y)
{
    if (flipView) {
	*x = lineGap + ((BOARD_WIDTH-1)-file) * (squareSize + lineGap);
	*y = lineGap + rank * (squareSize + lineGap);
    } else {
	*x = lineGap + file * (squareSize + lineGap);
	*y = lineGap + ((BOARD_HEIGHT-1)-rank) * (squareSize + lineGap);
    }
}

/* Draw an arrow between two points using current settings */
void DrawArrowBetweenPoints( int s_x, int s_y, int d_x, int d_y )
{
    XPoint arrow[7];
    double dx, dy, j, k, x, y;

    if( d_x == s_x ) {
        int h = (d_y > s_y) ? +A_WIDTH*A_HEIGHT_FACTOR : -A_WIDTH*A_HEIGHT_FACTOR;

        arrow[0].x = s_x + A_WIDTH + 0.5;
        arrow[0].y = s_y;

        arrow[1].x = s_x + A_WIDTH + 0.5;
        arrow[1].y = d_y - h;

        arrow[2].x = arrow[1].x + A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;
        arrow[2].y = d_y - h;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[5].x = arrow[1].x - 2*A_WIDTH + 0.5;
        arrow[5].y = d_y - h;

        arrow[4].x = arrow[5].x - A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;
        arrow[4].y = d_y - h;

        arrow[6].x = arrow[1].x - 2*A_WIDTH + 0.5;
        arrow[6].y = s_y;
    }
    else if( d_y == s_y ) {
        int w = (d_x > s_x) ? +A_WIDTH*A_HEIGHT_FACTOR : -A_WIDTH*A_HEIGHT_FACTOR;

        arrow[0].x = s_x;
        arrow[0].y = s_y + A_WIDTH + 0.5;

        arrow[1].x = d_x - w;
        arrow[1].y = s_y + A_WIDTH + 0.5;

        arrow[2].x = d_x - w;
        arrow[2].y = arrow[1].y + A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[5].x = d_x - w;
        arrow[5].y = arrow[1].y - 2*A_WIDTH + 0.5;

        arrow[4].x = d_x - w;
        arrow[4].y = arrow[5].y - A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;

        arrow[6].x = s_x;
        arrow[6].y = arrow[1].y - 2*A_WIDTH + 0.5;
    }
    else {
        /* [AS] Needed a lot of paper for this! :-) */
        dy = (double) (d_y - s_y) / (double) (d_x - s_x);
        dx = (double) (s_x - d_x) / (double) (s_y - d_y);

        j = sqrt( Sqr(A_WIDTH) / (1.0 + Sqr(dx)) );

        k = sqrt( Sqr(A_WIDTH*A_HEIGHT_FACTOR) / (1.0 + Sqr(dy)) );

        x = s_x;
        y = s_y;

        arrow[0].x = Round(x - j);
        arrow[0].y = Round(y + j*dx);

        arrow[1].x = Round(arrow[0].x + 2*j);   // [HGM] prevent width to be affected by rounding twice
        arrow[1].y = Round(arrow[0].y - 2*j*dx);

        if( d_x > s_x ) {
            x = (double) d_x - k;
            y = (double) d_y - k*dy;
        }
        else {
            x = (double) d_x + k;
            y = (double) d_y + k*dy;
        }

        x = Round(x); y = Round(y); // [HGM] make sure width of shaft is rounded the same way on both ends

        arrow[6].x = Round(x - j);
        arrow[6].y = Round(y + j*dx);

        arrow[2].x = Round(arrow[6].x + 2*j);
        arrow[2].y = Round(arrow[6].y - 2*j*dx);

        arrow[3].x = Round(arrow[2].x + j*(A_WIDTH_FACTOR-1));
        arrow[3].y = Round(arrow[2].y - j*(A_WIDTH_FACTOR-1)*dx);

        arrow[4].x = d_x;
        arrow[4].y = d_y;

        arrow[5].x = Round(arrow[6].x - j*(A_WIDTH_FACTOR-1));
        arrow[5].y = Round(arrow[6].y + j*(A_WIDTH_FACTOR-1)*dx);
    }

    XFillPolygon(xDisplay, xBoardWindow, highlineGC, arrow, 7, Nonconvex, CoordModeOrigin);
//    Polygon( hdc, arrow, 7 );
}

/* [AS] Draw an arrow between two squares */
void DrawArrowBetweenSquares( int s_col, int s_row, int d_col, int d_row )
{
    int s_x, s_y, d_x, d_y, hor, vert, i;

    if( s_col == d_col && s_row == d_row ) {
        return;
    }

    /* Get source and destination points */
    SquareToPos( s_row, s_col, &s_x, &s_y);
    SquareToPos( d_row, d_col, &d_x, &d_y);

    if( d_y > s_y ) {
        d_y += squareSize / 2 - squareSize / 4; // [HGM] round towards same centers on all sides!
    }
    else if( d_y < s_y ) {
        d_y += squareSize / 2 + squareSize / 4;
    }
    else {
        d_y += squareSize / 2;
    }

    if( d_x > s_x ) {
        d_x += squareSize / 2 - squareSize / 4;
    }
    else if( d_x < s_x ) {
        d_x += squareSize / 2 + squareSize / 4;
    }
    else {
        d_x += squareSize / 2;
    }

    s_x += squareSize / 2;
    s_y += squareSize / 2;

    /* Adjust width */
    A_WIDTH = squareSize / 14.; //[HGM] make float

    DrawArrowBetweenPoints( s_x, s_y, d_x, d_y );

    hor = 64*s_col + 32; vert = 64*s_row + 32;
    for(i=0; i<= 64; i++) {
            damage[0][vert+6>>6][hor+6>>6] = True;
            damage[0][vert-6>>6][hor+6>>6] = True;
            damage[0][vert+6>>6][hor-6>>6] = True;
            damage[0][vert-6>>6][hor-6>>6] = True;
            hor += d_col - s_col; vert += d_row - s_row;
    }
}

Boolean IsDrawArrowEnabled()
{
    return appData.highlightMoveWithArrow && squareSize >= 32;
}

void DrawArrowHighlight(int fromX, int fromY, int toX,int toY)
{
    if( IsDrawArrowEnabled() && fromX >= 0 && fromY >= 0 && toX >= 0 && toY >= 0)
        DrawArrowBetweenSquares(fromX, fromY, toX, toY);
}
