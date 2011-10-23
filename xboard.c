/*
 * xboard.c -- Gtk front end for XBoard
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkkeysyms.h>


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
#include "gtk_helper.h"

// must be moved to xengineoutput.h
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

/* will the file chooser dialog be used for opening or saving? */
typedef enum {OPEN, SAVE} FileAction;

int main P((int argc, char **argv));
RETSIGTYPE CmailSigHandler P((int sig));
RETSIGTYPE IntSigHandler P((int sig));
RETSIGTYPE TermSizeSigHandler P((int sig));
void CreatePieces P((void));
void CreatePieceMenus P((void));
Widget CreateMenuBar P((Menu *mb));
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
int EventToSquare P((int x, int limit));
void DrawSquareGTK P((int row, int column, ChessSquare piece, int do_flash));
gboolean EventProcGTK P((GtkWidget *widget, GdkEventExpose *event, gpointer data));
void MoveTypeInProc P((Widget widget, caddr_t unused, XEvent *event));
gboolean HandleUserMoveGTK P((GtkWindow *window, GdkEventButton *eventbutton, gpointer data));
gboolean KeyPressProc P((GtkWindow *window, GdkEventKey *eventkey, gpointer data));
gboolean ButtonPressProc P((GtkWindow *window, GdkEventButton *eventbutton, gpointer data));
void AnimateUserMove P((GtkWidget *w, GdkEventMotion *event));
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
void GTKDrawPosition P((GtkWidget *w, /*Boolean*/int repaint,
		     Board board));
void CommentPopUp P((char *title, char *label));
void CommentPopDown P((void));
void ICSInputBoxPopUp P((void));
void ICSInputBoxPopDown P((void));
void FileNamePopUp P((char *label, char *def, char *filter,
		      FileProc proc, char *openMode, FileAction action));
void PromotionPopDown P((void));
void PromotionCallback P((GtkWidget *w, GtkResponseType resptype,
                          gpointer gdata));
void SelectCommand P((Widget w, XtPointer client_data, XtPointer call_data));
void LoadNextGameProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void LoadPrevGameProc P((Widget w, XEvent *event, String *prms,
			 Cardinal *nprms));
void ReloadGameProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void ReloadPositionProc P((Widget w, XEvent *event, String *prms,
		       Cardinal *nprms));
void TypeInProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void EnterKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void UpKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DownKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
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
void DebugProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void NothingProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
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
void SelectMove P((Widget w, XEvent * event, String * params, Cardinal * nParams));
void GameListOptionsPopDown P(());
//void GenericPopDown P(());
void update_ics_width P(());
int get_term_width P(());
int CopyMemoProc P(());
void DrawArrowHighlightGTK P((int fromX, int fromY, int toX,int toY));
Boolean IsDrawArrowEnabled P(());
GdkPixbuf *getPixbuf P((int piece));
void ScalePixbufs P((void));

/*
* XBoard depends on Xt R4 or higher
*/
int xtVersion = XtSpecificationRelease;

int xScreen;
Display *xDisplay;
Window xBoardWindow;
Pixel lightSquareColor, darkSquareColor, whitePieceColor, blackPieceColor,
   highlightSquareColor, premoveHighlightColor;
Pixel lowTimeWarningColor;
Widget shellWidget, layoutWidget, formWidget, boardWidget, messageWidget,
  whiteTimerWidget, blackTimerWidget, titleWidget, widgetList[16],
  commentShell, promotionShell, whitePieceMenu, blackPieceMenu, dropMenu,
  menuBarWidget, buttonBarWidget, editShell, analysisShell,
  ICSInputShell,  askQuestionShell;
GtkWidget *errorShell = NULL, *promotionShellGTK;
Widget historyShell, evalGraphShell, gameListShell;
int hOffset; // [HGM] dual
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

/* GTK stuff */
GtkBuilder      *builder;
GtkWidget       *mainwindow;
GtkWidget       *boardwidgetGTK=NULL;
GtkWidget       *boardaspect=NULL;
GtkWidget       *whiteTimerWidgetGTK;
GtkWidget       *blackTimerWidgetGTK;
GtkWidget       *messageWidgetGTK=NULL;
GtkWidget       *menubarGTK=NULL;

GtkEntryBuffer  *AskQuestionBuffer;

/* pixbufs */
GdkPixbuf       *mainwindowIcon=NULL;
GdkPixbuf       *WhiteIcon=NULL;
GdkPixbuf       *BlackIcon=NULL;

GdkPixbuf       *SVGLightSquare=NULL;
GdkPixbuf       *SVGDarkSquare=NULL;
GdkPixbuf       *SVGNeutralSquare=NULL;

GdkPixbuf       *SVGWhitePawn=NULL;
GdkPixbuf       *SVGWhiteKnight=NULL;
GdkPixbuf       *SVGWhiteBishop=NULL;
GdkPixbuf       *SVGWhiteRook=NULL;
GdkPixbuf       *SVGWhiteKing=NULL;
GdkPixbuf       *SVGWhiteQueen=NULL;
GdkPixbuf       *SVGWhiteCardinal=NULL;
GdkPixbuf       *SVGWhiteMarshall=NULL;
GdkPixbuf       *SVGWhite=NULL;

GdkPixbuf       *SVGBlackPawn=NULL;
GdkPixbuf       *SVGBlackKnight=NULL;
GdkPixbuf       *SVGBlackBishop=NULL;
GdkPixbuf       *SVGBlackRook=NULL;
GdkPixbuf       *SVGBlackKing=NULL;
GdkPixbuf       *SVGBlackQueen=NULL;
GdkPixbuf       *SVGBlackCardinal=NULL;
GdkPixbuf       *SVGBlackMarshall=NULL;
GdkPixbuf       *SVGBlack=NULL;

/* scaled pixbufs */
GdkPixbuf       *SVGscWhitePawn=NULL;
GdkPixbuf       *SVGscWhiteKnight=NULL;
GdkPixbuf       *SVGscWhiteBishop=NULL;
GdkPixbuf       *SVGscWhiteRook=NULL;
GdkPixbuf       *SVGscWhiteKing=NULL;
GdkPixbuf       *SVGscWhiteQueen=NULL;
GdkPixbuf       *SVGscWhiteCardinal=NULL;
GdkPixbuf       *SVGscWhiteMarshall=NULL;
GdkPixbuf       *SVGscWhite=NULL;

GdkPixbuf       *SVGscBlackPawn=NULL;
GdkPixbuf       *SVGscBlackKnight=NULL;
GdkPixbuf       *SVGscBlackBishop=NULL;
GdkPixbuf       *SVGscBlackRook=NULL;
GdkPixbuf       *SVGscBlackKing=NULL;
GdkPixbuf       *SVGscBlackQueen=NULL;
GdkPixbuf       *SVGscBlackCardinal=NULL;
GdkPixbuf       *SVGscBlackMarshall=NULL;
GdkPixbuf       *SVGscBlack=NULL;

FileProc fileProc;
char *fileOpenMode;
char installDir[] = "."; // [HGM] UCI: needed for UCI; probably needs run-time initializtion

Position commentX = -1, commentY = -1;
Dimension commentW, commentH;
typedef unsigned int BoardSize;
BoardSize boardSize;
Boolean chessProgram;

int  minX, minY; // [HGM] placement: volatile limits on upper-left corner
int squareSize, squareSizeGTK, smallLayout = 0, tinyLayout = 0,
  marginW, marginH, xMargin, yMargin, // [HGM] for run-time resizing
  fromX = -1, fromY = -1, toX, toY, commentUp = False, analysisUp = False,
  ICSInputBoxUp = False, askQuestionUp = False,
  filenameUp = False, promotionUp = False, pmFromX = -1, pmFromY = -1,
  errorUp = False, errorExitStatus = -1, lineGap, lineGapGTK, defaultLineGap;
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

extern Widget shells[];
extern Boolean shellUp[];

#define SOLID 0
#define OUTLINE 1

#define White(piece) ((int)(piece) < (int)BlackPawn)

/* Variables for doing smooth animation. This whole thing
   would be much easier if the board was double-buffered,
   but that would require a fairly major rewrite.	*/

typedef struct {
	Pixmap  saveBuf;
	Pixmap	newBuf;
	GC	blitGC, pieceGC, outlineGC;
	GdkPoint startSquare, prevFrame, mouseDelta;
	int	startColor;
	int	dragPiece;
	Boolean	dragActive;
        int     startBoardX, startBoardY;
    } AnimState;

/* There can be two pieces being animated at once: a player
   can begin dragging a piece before the remote opponent has moved. */

static AnimState game, player;

/* This magic number is the number of intermediate frames used
   in each half of the animation. For short moves it's reduced
   by 1. The total number of frames will be factor * 2 + 1.  */
#define kFactor	   4

SizeDefaults sizeDefaults[] = SIZE_DEFAULTS;

MenuItem fileMenu[] = {
    {N_("New Game        Ctrl+N"),        "New Game", NothingProc},
    {N_("New Shuffle Game ..."),          "New Shuffle Game", NothingProc},
    {N_("New Variant ...   Alt+Shift+V"), "New Variant", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Load Game       Ctrl+O"),        "Load Game", NothingProc},
    {N_("Load Position    Ctrl+Shift+O"), "Load Position", NothingProc},
//    {N_("Load Next Game"), "Load Next Game", LoadNextGameProc},
//    {N_("Load Previous Game"), "Load Previous Game", LoadPrevGameProc},
//    {N_("Reload Same Game"), "Reload Same Game", ReloadGameProc},
    {N_("Next Position     Shift+PgDn"), "Load Next Position", NothingProc},
    {N_("Prev Position     Shift+PgUp"), "Load Previous Position", NothingProc},
    {"----", NULL, NothingProc},
//    {N_("Reload Same Position"), "Reload Same Position", ReloadPositionProc},
    {N_("Save Game       Ctrl+S"),        "Save Game", NothingProc},
    {N_("Save Position    Ctrl+Shift+S"), "Save Position", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Mail Move"),            "Mail Move", NothingProc},
    {N_("Reload CMail Message"), "Reload CMail Message", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Quit                 Ctr+Q"), "Exit", NothingProc},
    {NULL, NULL, NULL}
};

MenuItem editMenu[] = {
    {N_("Copy Game    Ctrl+C"),        "Copy Game", NothingProc},
    {N_("Copy Position Ctrl+Shift+C"), "Copy Position", NothingProc},
    {N_("Copy Game List"),        "Copy Game List", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Paste Game    Ctrl+V"),        "Paste Game", NothingProc},
    {N_("Paste Position Ctrl+Shift+V"), "Paste Position", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Edit Game      Ctrl+E"),        "Edit Game", NothingProc},
    {N_("Edit Position   Ctrl+Shift+E"), "Edit Position", NothingProc},
    {N_("Edit Tags"),                    "Edit Tags", NothingProc},
    {N_("Edit Comment"),                 "Edit Comment", NothingProc},
    {N_("Edit Book"),                    "Edit Book", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Revert              Home"), "Revert", NothingProc},
    {N_("Annotate"),                 "Annotate", NothingProc},
    {N_("Truncate Game  End"),       "Truncate Game", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Backward         Alt+Left"),   "Backward", NothingProc},
    {N_("Forward           Alt+Right"), "Forward", NothingProc},
    {N_("Back to Start     Alt+Home"),  "Back to Start", NothingProc},
    {N_("Forward to End Alt+End"),      "Forward to End", NothingProc},
    {NULL, NULL, NULL}
};

MenuItem viewMenu[] = {
    {N_("Flip View             F2"),         "Flip View", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Engine Output      Alt+Shift+O"),   "Show Engine Output", NothingProc},
    {N_("Move History       Alt+Shift+H"),   "Show Move History", NothingProc},
    {N_("Evaluation Graph  Alt+Shift+E"),    "Show Evaluation Graph", EvalGraphProc},
    {N_("Game List            Alt+Shift+G"), "Show Game List", ShowGameListProc},
    {N_("ICS text menu"), "ICStex", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Tags"),             "Show Tags", NothingProc},
    {N_("Comments"),         "Show Comments", NothingProc},
    {N_("ICS Input Box"),    "ICS Input Box", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Board..."),          "Board Options", NothingProc},
    {N_("Game List Tags..."), "Game List", NothingProc},
    {NULL, NULL, NULL}
};

MenuItem modeMenu[] = {
    {N_("Machine White  Ctrl+W"), "Machine White", NothingProc},
    {N_("Machine Black  Ctrl+B"), "Machine Black", NothingProc},
    {N_("Two Machines   Ctrl+T"), "Two Machines", NothingProc},
    {N_("Analysis Mode  Ctrl+A"), "Analysis Mode", NothingProc},
    {N_("Analyze Game   Ctrl+G"), "Analyze File", NothingProc },
    {N_("Edit Game         Ctrl+E"), "Edit Game", NothingProc},
    {N_("Edit Position      Ctrl+Shift+E"), "Edit Position", NothingProc},
    {N_("Training"),      "Training", NothingProc},
    {N_("ICS Client"),    "ICS Client", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Machine Match"),         "Machine Match", NothingProc},
    {N_("Pause               Pause"),         "Pause", NothingProc},
    {NULL, NULL, NULL}
};

MenuItem actionMenu[] = {
    {N_("Accept             F3"), "Accept", NothingProc},
    {N_("Decline            F4"), "Decline", NothingProc},
    {N_("Rematch           F12"), "Rematch", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Call Flag          F5"), "Call Flag", NothingProc},
    {N_("Draw                F6"), "Draw", NothingProc},
    {N_("Adjourn            F7"),  "Adjourn", NothingProc},
    {N_("Abort                F8"),"Abort", NothingProc},
    {N_("Resign              F9"), "Resign", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Stop Observing  F10"), "Stop Observing", NothingProc},
    {N_("Stop Examining  F11"), "Stop Examining", NothingProc},
    {N_("Upload to Examine"),   "Upload to Examine", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Adjudicate to White"), "Adjudicate to White", NothingProc},
    {N_("Adjudicate to Black"), "Adjudicate to Black", NothingProc},
    {N_("Adjudicate Draw"),     "Adjudicate Draw", NothingProc},
    {NULL, NULL, NULL}
};

MenuItem engineMenu[] = {
    {N_("Load New Engine ..."), "Load Engine", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Engine #1 Settings ..."), "Engine #1 Settings", NothingProc},
    {N_("Engine #2 Settings ..."), "Engine #2 Settings", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Hint"), "Hint", NothingProc},
    {N_("Book"), "Book", NothingProc},
    {"----", NULL, NothingProc},
    {N_("Move Now     Ctrl+M"),     "Move Now", NothingProc},
    {N_("Retract Move  Ctrl+X"), "Retract Move", NothingProc},
    {NULL, NULL, NULL}
};

MenuItem optionsMenu[] = {
#define OPTIONSDIALOG
#ifdef OPTIONSDIALOG
    {N_("General ..."), "General", NothingProc},
#endif
    {N_("Time Control ...       Alt+Shift+T"), "Time Control", NothingProc},
    {N_("Common Engine ...  Alt+Shift+U"),     "Common Engine", NothingProc},
    {N_("Adjudications ...      Alt+Shift+J"), "Adjudications", NothingProc},
    {N_("ICS ..."),    "ICS", NothingProc},
    {N_("Match ..."), "Match", NothingProc},
    {N_("Load Game ..."),    "Load Game", NothingProc},
    {N_("Save Game ..."),    "Save Game", NothingProc},
    {N_("Game List ..."),    "Game List", NothingProc},
    {N_("Sounds ..."),    "Sounds", NothingProc},
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
    {N_("Save Settings Now"),     "Save Settings Now", NothingProc},
    {N_("Save Settings on Exit"), "Save Settings on Exit", NothingProc},
    {NULL, NULL, NULL}
};

MenuItem helpMenu[] = {
    {N_("Info XBoard"),     "Info XBoard", NothingProc},
    {N_("Man XBoard   F1"), "Man XBoard", NothingProc},
    {"----", NULL, NothingProc},
    {N_("About XBoard"), "About XBoard", NothingProc},
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
    {"<<", "<<", NothingProc},
    {"<", "<", NothingProc},
    {PAUSE_BUTTON, PAUSE_BUTTON, NothingProc},
    {">", ">", NothingProc},
    {">>", ">>", NothingProc},
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
    { "HandlePV", HandlePV },
    { "SelectPV", SelectPV },
    { "StopPV", StopPV },
    { "PieceMenuPopup", PieceMenuPopup },
    { "WhiteClock", WhiteClock },
    { "BlackClock", BlackClock },
    { "LoadNextGameProc", LoadNextGameProc },
    { "LoadPrevGameProc", LoadPrevGameProc },
//    { "LoadSelectedProc", LoadSelectedProc },
    { "SetFilterProc", SetFilterProc },
    { "ReloadGameProc", ReloadGameProc },
    { "ReloadPositionProc", ReloadPositionProc },
    { "EvalGraphProc", EvalGraphProc},       // [HGM] Winboard_x avaluation graph window
    { "ShowGameListProc", ShowGameListProc },
    { "EnterKeyProc", EnterKeyProc },
    { "UpKeyProc", UpKeyProc },
    { "DownKeyProc", DownKeyProc },
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
    { "DebugProc", DebugProc },
    { "NothingProc", NothingProc },
    //{ "CommentClick", (XtActionProc) CommentClick },
    { "CommentPopDown", (XtActionProc) CommentPopDown },
    { "TagsPopDown", (XtActionProc) TagsPopDown },
    { "ErrorPopDown", (XtActionProc) ErrorPopDown },
    { "ICSInputBoxPopDown", (XtActionProc) ICSInputBoxPopDown },
    { "GameListPopDown", (XtActionProc) GameListPopDown },
    //    { "GameListOptionsPopDown", (XtActionProc) GameListOptionsPopDown },
   // { "PromotionPopDown", (XtActionProc) PromotionPopDown },
    { "EngineOutputPopDown", (XtActionProc) EngineOutputPopDown },
    { "EvalGraphPopDown", (XtActionProc) EvalGraphPopDown },
   // { "GenericPopDown", (XtActionProc) GenericPopDown },
    { "CopyMemoProc", (XtActionProc) CopyMemoProc },
    { "SelectMove", (XtActionProc) SelectMove },
};

char globalTranslations[] =
  ":Meta<Key>Next: LoadNextGameProc() \n \
   :Meta<Key>Prior: LoadPrevGameProc() \n \
   :Meta<Key>E: EvalGraphProc() \n \
   :Meta<Key>G: ShowGameListProc() \n \
   :Meta Ctrl<Key>F12: DebugProc() \n \
   :Ctrl<Key>P: PonderNextMoveProc() \n ";
//#ifndef OPTIONSDIALOG
//    "\
//   :Ctrl<Key>Q: AlwaysQueenProc() \n \
//   :Ctrl<Key>F: AutoflagProc() \n \
//   :Ctrl<Key>A: AnimateMovingProc() \n \
//   :Ctrl<Key>L: TestLegalityProc() \n \
//   :Ctrl<Key>H: HideThinkingProc() \n "
//#endif

char boardTranslations[] =
   "<Btn3Motion>: HandlePV() \n \
   <Btn3Up>: PieceMenuPopup(menuB) \n \
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

String xboardResources[] = {
    "*errorpopup*translations: #override\\n <Key>Return: ErrorPopDown()",
    NULL
  };


/* Max possible square size */
#define MAXSQSIZE 256

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

void
BoardToTop()
{
  gtk_window_present(GTK_WINDOW(mainwindow));
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
  fontIsSet[number] = True; // [HGM] font: indicate a font was specified (not from settings file)
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
  if(MoveHistoryIsUp()) GetActualPlacement(shells[7], &wpMoveHistory);
  if(EvalGraphIsUp()) GetActualPlacement(evalGraphShell, &wpEvalGraph);
  if(GameListIsUp()) GetActualPlacement(gameListShell, &wpGameList);
  if(shellUp[1]) GetActualPlacement(shells[1], &wpComment);
  if(shellUp[2]) GetActualPlacement(shells[2], &wpTags);
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

    /* resizes for GTK */
    gtk_window_resize(GTK_WINDOW(mainwindow), BOARD_WIDTH * (squareSizeGTK + lineGapGTK) + lineGapGTK + xMargin,
                                              BOARD_HEIGHT * (squareSizeGTK + lineGapGTK) + lineGapGTK + yMargin);
    gtk_widget_set_size_request(GTK_WIDGET(boardwidgetGTK), BOARD_WIDTH * (squareSizeGTK + lineGapGTK) + lineGapGTK,
                                              BOARD_HEIGHT * (squareSizeGTK + lineGapGTK) + lineGapGTK);

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
    hOffset = boardWidth + 10; /* used for second board */

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

void SetPieceColor(GdkPixbuf *pb)
{

    int width, height, rowstride, n_channels;
    guchar *pixels, *p;
    int x, y;
    int col, r, g, b, i;

    /* RGB for black as used in the pieces in SVG folder */
    guchar blackRGB[2][3] = { {0, 0, 0}, {3, 3, 2} };

    /* RGB for white as used in the pieces in SVG folder */
    guchar whiteRGB[2][3] = { {0xff, 0xff, 0xcc}, {0xfc, 0xfc, 0xca} };

    /* RGB of new colour held in appData.blackPieceColor */
    guchar newBlackR;
    guchar newBlackG;
    guchar newBlackB;

    /* RGB of new colour held in appData.whitePieceColor */
    guchar newWhiteR;
    guchar newWhiteG;
    guchar newWhiteB;

    n_channels = gdk_pixbuf_get_n_channels(pb);
    width = gdk_pixbuf_get_width(pb);
    height = gdk_pixbuf_get_height(pb);

    g_assert(gdk_pixbuf_get_colorspace(pb) == GDK_COLORSPACE_RGB);
    g_assert(gdk_pixbuf_get_bits_per_sample(pb) == 8);
    g_assert(gdk_pixbuf_get_has_alpha(pb));
    g_assert(n_channels == 4);

    rowstride = gdk_pixbuf_get_rowstride(pb);
    pixels = gdk_pixbuf_get_pixels(pb);

    /* get RGB of new white colour */
    sscanf(appData.whitePieceColor, "#%x", &col);
    b = col & 0xFF; g = col & 0xFF00; r = col & 0xFF0000;
    newWhiteB = b;
    newWhiteG = g >> 8;
    newWhiteR = r >> 16;

    /* get RGB of new black colour */
    sscanf(appData.blackPieceColor, "#%x", &col);
    b = col & 0xFF; g = col & 0xFF00; r = col & 0xFF0000;
    newBlackB = b;
    newBlackG = g >> 8;
    newBlackR = r >> 16;

    /* change the colors to the new values in the pixbuf */
    x = y = 0;
    for (x=0;x<width;x++) {
        for (y=0;y<height;y++) {
            p = pixels + y * rowstride + x * n_channels;
            for (i=0;i<2;i++) {
                if (p[0] == blackRGB[i][0] && p[1] == blackRGB[i][1] && p[2] == blackRGB[i][2]) {
                    p[0] = newBlackR;
                    p[1] = newBlackG;
                    p[2] = newBlackB;
                    break;
                }
                else if (p[0] == whiteRGB[i][0] && p[1] == whiteRGB[i][1] && p[2] == whiteRGB[i][2]) {
                    p[0] = newWhiteR;
                    p[1] = newWhiteG;
                    p[2] = newWhiteB;
                    break;
                }
            }
        }
    }
}

void LoadSvgFiles()
{
    SVGNeutralSquare = load_pixbuf("NeutralSquare.svg", 0);

    if (appData.useBoardTexture) {

        /* Load background squares from texture files */
        /* At the moment these have to be svg files. It will work with other files */
        /* including xpm files but board resizing will be slow */

        /* set up dark square from texture file */
        if (appData.darkBackTextureFile == NULL) {
          SVGDarkSquare    = load_pixbuf("DarkSquare.svg", 0);  // texture file not set - use default
        }
        else if (strstr(appData.darkBackTextureFile, ".svg") == NULL) {
          SVGDarkSquare    = load_pixbuf("DarkSquare.svg", 0);  // texture file not svg - use default
        }
        else {
          // texture file is an svg file - try and load it
          SVGDarkSquare = gdk_pixbuf_new_from_file(appData.darkBackTextureFile, NULL);
          if (SVGDarkSquare == NULL) {
	    SVGDarkSquare    = load_pixbuf("DarkSquare.svg", 0); // texture file failed to load - use default
          }
        }

        /* set up light square from texture file */
        if (appData.liteBackTextureFile == NULL) {
          SVGLightSquare    = load_pixbuf("LightSquare.svg", 0); // texture file not set - use default
        }
        else if (strstr(appData.liteBackTextureFile, ".svg") == NULL) {
          SVGLightSquare    = load_pixbuf("LightSquare.svg", 0); // texture file not svg - use default
        }
        else {
          // texture file is an svg file - try and load it
          SVGLightSquare = gdk_pixbuf_new_from_file(appData.liteBackTextureFile, NULL);
          if (SVGLightSquare == NULL) {
	    SVGLightSquare    = load_pixbuf("LightSquare.svg", 0); // texture file failed to load - use default
          }
        }
    }
    else {
        int col;

        SVGLightSquare   = load_pixbuf("LightSquare.svg", 0);
        SVGDarkSquare    = load_pixbuf("DarkSquare.svg", 0);

        sscanf(appData.darkSquareColor, "#%x", &col);
        col = col << 8;
        col = col | 0xff; /* add 0xff to the end as alpha to set to opaque */
        gdk_pixbuf_fill(SVGDarkSquare, col);

        sscanf(appData.lightSquareColor, "#%x", &col);
        col = col << 8;
        col = col | 0xff; /* add 0xff to the end as alpha to set to opaque */
        gdk_pixbuf_fill(SVGLightSquare, col);
    }

    SVGWhitePawn     = load_pixbuf("WhitePawn.svg", 0);
    SVGWhiteKnight   = load_pixbuf("WhiteKnight.svg", 0);
    SVGWhiteBishop   = load_pixbuf("WhiteBishop.svg", 0);
    SVGWhiteRook     = load_pixbuf("WhiteRook.svg", 0);
    SVGWhiteQueen    = load_pixbuf("WhiteQueen.svg", 0);
    SVGWhiteCardinal = load_pixbuf("WhiteCrownedBishop.svg", 0);
    SVGWhiteMarshall = load_pixbuf("WhiteChancellor.svg", 0);
    SVGWhiteKing     = load_pixbuf("WhiteKing.svg", 0);

    //LoadBlackPieces
    SVGBlackPawn     = load_pixbuf("BlackPawn.svg", 0);
    SVGBlackKnight   = load_pixbuf("BlackKnight.svg", 0);
    SVGBlackBishop   = load_pixbuf("BlackBishop.svg", 0);
    SVGBlackRook     = load_pixbuf("BlackRook.svg", 0);
    SVGBlackQueen    = load_pixbuf("BlackQueen.svg", 0);
    SVGBlackCardinal = load_pixbuf("BlackCrownedBishop.svg", 0);
    SVGBlackMarshall = load_pixbuf("BlackChancellor.svg", 0);
    SVGBlackKing     = load_pixbuf("BlackKing.svg", 0);

    SetPieceColor(SVGWhitePawn);
    SetPieceColor(SVGWhiteKnight);
    SetPieceColor(SVGWhiteBishop);
    SetPieceColor(SVGWhiteRook);
    SetPieceColor(SVGWhiteQueen);
    SetPieceColor(SVGWhiteCardinal);
    SetPieceColor(SVGWhiteMarshall);
    SetPieceColor(SVGWhiteKing);

    SetPieceColor(SVGBlackPawn);
    SetPieceColor(SVGBlackKnight);
    SetPieceColor(SVGBlackBishop);
    SetPieceColor(SVGBlackRook);
    SetPieceColor(SVGBlackQueen);
    SetPieceColor(SVGBlackCardinal);
    SetPieceColor(SVGBlackMarshall);
    SetPieceColor(SVGBlackKing);

    ScalePixbufs();
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
    char *filename;
    GError *gtkerror=NULL;

    gfloat ar; /* board aspect ratio */

    srandom(time(0)); // [HGM] book: make random truly random

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    debugFP = stderr;

    if(argc > 1 && (!strcmp(argv[1], "-v" ) || !strcmp(argv[1], "--version" ))) {
	printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
	exit(0);
    }

    /* set up GTK */
    gtk_init (&argc, &argv);



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

    defaultLineGap = lineGap;
    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap;

    squareSizeGTK = squareSize;
    lineGapGTK = lineGap;

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
      char **font_name_list;
      XFontsOfFontSet(coordFontSet, &font_struct_list, &font_name_list);
      coordFontID = XLoadFont(xDisplay, font_name_list[0]);
      coordFontStruct = XQueryFont(xDisplay, coordFontID);
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



    /* GTK */
    builder = gtk_builder_new();
    filename = get_glade_filename ("mainboard.glade");
    if(! gtk_builder_add_from_file (builder, filename, &gtkerror) )
      {
      if(gtkerror)
        printf ("Error: %d %s\n",gtkerror->code,gtkerror->message);
      }

    //gtk_builder_add_from_file(builder, "mainboard.glade", NULL);
    /* load square colors and pieces */
    LoadSvgFiles();

    mainwindow = GTK_WIDGET(gtk_builder_get_object (builder, "mainwindow"));
    boardwidgetGTK  = GTK_WIDGET(gtk_builder_get_object (builder, "boardwidgetGTK"));
    if(!boardwidgetGTK) printf("Error: gtk_builder didn't work (boardwidgetGTK)!\n");

    /* set board bg color to black */
    GdkColor color;
    //gdk_color_parse( "#000000", &color );
    //gdk_color_parse ("black", &color);
    gdk_color_parse ("white", &color);
    gtk_widget_modify_bg(boardwidgetGTK, GTK_STATE_NORMAL, &color );

    gdk_color_parse ("white", &color);
    gtk_widget_modify_bg(mainwindow, GTK_STATE_NORMAL, &color );

    whiteTimerWidgetGTK = GTK_WIDGET(gtk_builder_get_object (builder, "whiteTimerWidgetGTK"));
    blackTimerWidgetGTK = GTK_WIDGET(gtk_builder_get_object (builder, "blackTimerWidgetGTK"));
    messageWidgetGTK = GTK_WIDGET(gtk_builder_get_object (builder, "messageWidgetGTK"));
    menubarGTK  = GTK_WIDGET (gtk_builder_get_object (builder, "MenuBar"));

    boardaspect = GTK_WIDGET(gtk_builder_get_object (builder, "boardaspect"));
    ar = (float) BOARD_WIDTH / BOARD_HEIGHT;
    gtk_aspect_frame_set(GTK_ASPECT_FRAME(boardaspect), 0.5, 0.5, ar, TRUE);

    gtk_widget_set_size_request(GTK_WIDGET(boardwidgetGTK), boardWidth, boardHeight);
    gtk_builder_connect_signals(builder, NULL);
    //g_object_unref (G_OBJECT(builder));


    /* use two icons to indicate if it is white's or black's turn */
    WhiteIcon  = load_pixbuf("icon_white.svg",0);
    BlackIcon  = load_pixbuf("icon_black.svg",0);
    mainwindowIcon = WhiteIcon;
    gtk_window_set_icon(GTK_WINDOW(mainwindow),mainwindowIcon);

    gtk_widget_show(mainwindow);

    /* create a text buffer for AskQuestion */
    AskQuestionBuffer = gtk_entry_buffer_new (NULL,-1);

    /* set the minimum size the user can resize the main window to */
    gtk_widget_set_size_request(mainwindow, 402, 314);
  { gint wx, hx, wb, hb;
    gtk_window_get_size(GTK_WINDOW(mainwindow), &wx, &hx);
    gdk_drawable_get_size(boardwidgetGTK->window, &wb, &hb);
    xMargin = wx - wb; yMargin = hx - hb;
  }
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

    XtAddEventHandler(formWidget, KeyPressMask, False,
		      (XtEventHandler) MoveTypeInProc, NULL);

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

    XtUnmapWidget(shellWidget);

    //    XtAppMainLoop(appContext);
    for (i=0;i<700;i++)
	{
	  XEvent event;
	  XtInputMask mask;

	  while (!(mask = XtAppPending(appContext)) && !gtk_events_pending())
	    poll(NULL,0,100);
	  if (mask & XtIMXEvent) {
	    XtAppNextEvent(appContext, &event); /* no blocking */
	    XtDispatchEvent(&event); /* Process it */
	  }
	  else /* not an XEvent, process it */
	    XtAppProcessEvent(appContext, mask); /* non blocking */
	}
    /* check for GTK events and process them */
    gtk_main();

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
    { "menuEngine.Load Engine", False },
    { "menuEdit.Annotate", False },
    { "menuOptions.Match", False },
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
    // [HGM] The following have been added to make a switch from ncp to GNU mode possible
    { "menuMode.Machine White", True },
    { "menuMode.Machine Black", True },
    { "menuMode.Analysis Mode", True },
    { "menuMode.Analyze File", True },
    { "menuMode.Two Machines", True },
    { "menuMode.Machine Match", True },
    { "menuEngine.Engine #1 Settings", True },
    { "menuEngine.Engine #2 Settings", True },
    { "menuEngine.Hint", True },
    { "menuEngine.Book", True },
    { "menuEngine.Move Now", True },
    { "menuEngine.Retract Move", True },
    { "Action", True },
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
//  { "menuMode.Machine Match", False },
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
//  { "menuMode.Machine Match", True },
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

#ifdef ENABLE_NLS
char *
InsertPxlSize(pattern, targetPxlSize)
     char *pattern;
     int targetPxlSize;
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
CreateFontSet(base_fnt_lst)
     char *base_fnt_lst;
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
FindFont(pattern, targetPxlSize)
     char *pattern;
     int targetPxlSize;
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

static VariantClass oldVariant = (VariantClass) -1; // [HGM] pieces: redo every time variant changes

void
DrawGrid(int x, int y, int Nx, int Ny)
{
  /* draws a grid starting around Nx, Ny squares starting at x,y */
  int i,j;

  int x1,x2,y1,y2;
  cairo_t *cr;

  /* get a cairo_t */
  cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

  cairo_set_line_width (cr, lineGapGTK);

  /* TODO: use appdata colors */
  cairo_set_source_rgba (cr, 0, 0, 0, 1.0);

  /* lines in X */
  for (i = y; i < MIN(BOARD_HEIGHT,y + Ny+1); i++)
    {
      x1 = x * (squareSizeGTK + lineGapGTK);;
      x2 = lineGapGTK + MIN(BOARD_WIDTH,x + Nx) * (squareSizeGTK + lineGapGTK);
      y1 = y2 = lineGapGTK / 2 + (i * (squareSizeGTK + lineGapGTK));

      cairo_move_to (cr, x1, y1);
      cairo_line_to (cr, x2,y2);
      cairo_stroke (cr);
    }

  /* lines in Y */
  for (j = x; j < MIN(BOARD_WIDTH,x + Nx+1) ; j++)
    {
      y1 = y * (squareSizeGTK + lineGapGTK);
      y2 = lineGapGTK + MIN(BOARD_HEIGHT,y + Ny) * (squareSizeGTK + lineGapGTK);
      x1 = x2  = lineGapGTK / 2 + (j * (squareSizeGTK + lineGapGTK));

      cairo_move_to (cr, x1, y1);
      cairo_line_to (cr, x2, y2);
      cairo_stroke (cr);
    }

  /* free memory */
  cairo_destroy (cr);

  return;
}


void CreateGridGTK()
{
    /* draws a grid starting around Nx, Ny squares starting at x,y */
    int i,j;

    int x1,x2,y1,y2;
    cairo_t *cr;

    if (lineGapGTK == 0) return;

    /* get a cairo_t */
    cr = gdk_cairo_create(GDK_WINDOW(boardwidgetGTK->window));

    cairo_set_line_width (cr, lineGapGTK);

    /* TODO: use appdata colors */
    cairo_set_source_rgba (cr, 0, 0, 0, 1.0);

    /* [HR] Split this into 2 loops for non-square boards. */

    for (i = 0; i < BOARD_HEIGHT + 1; i++) {
        x1 = 0;
        x2 = lineGapGTK + BOARD_WIDTH * (squareSizeGTK + lineGapGTK);
        y1 = y2 = lineGapGTK / 2 + (i * (squareSizeGTK + lineGapGTK));
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
    }

    for (j = 0; j < BOARD_WIDTH + 1; j++) {
        y1 = 0;
        y2 = lineGapGTK + BOARD_HEIGHT * (squareSizeGTK + lineGapGTK);
        x1 = x2 = lineGapGTK / 2 + (j * (squareSizeGTK + lineGapGTK));;
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
    }

    /* free memory */
    cairo_destroy (cr);

    return;
}

void CreateSecondGridGTK()
{
    /* draws a grid starting around Nx, Ny squares starting at x,y */
    int i,j;

    int x1,x2,y1,y2;
    cairo_t *cr;

    if (lineGapGTK == 0) return;

    /* get a cairo_t */
    cr = gdk_cairo_create(GDK_WINDOW(boardwidgetGTK->window));

    cairo_set_line_width (cr, lineGapGTK);

    /* TODO: use appdata colors */
    cairo_set_source_rgba (cr, 0, 0, 0, 1.0);

    /* [HR] Split this into 2 loops for non-square boards. */

    for (i = 0; i < BOARD_HEIGHT + 1; i++) {
        x1 = hOffset;
        x2 = hOffset + lineGapGTK + BOARD_WIDTH * (squareSizeGTK + lineGapGTK);
        y1 = y2 = lineGapGTK / 2 + (i * (squareSizeGTK + lineGapGTK));
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
    }

    for (j = 0; j < BOARD_WIDTH + 1; j++) {
        y1 = 0;
        y2 = lineGapGTK + BOARD_HEIGHT * (squareSizeGTK + lineGapGTK);
        x1 = x2 = hOffset + lineGapGTK / 2 + (j * (squareSizeGTK + lineGapGTK));;
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
    }

    /* free memory */
    cairo_destroy (cr);

    return;
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
	  entry = XtCreateManagedWidget(_(mi->string), smeLineObjectClass,
					  menu, args, j);
	} else {
          XtSetArg(args[j], XtNlabel, XtNewString(_(mi->string)));
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
	if (tinyLayout) {
	    char shortName[2];
            shortName[0] = mb->name[0];
	    shortName[1] = NULLCHAR;
	    XtSetArg(args[j], XtNlabel, XtNewString(shortName)); j++;
	}
      else {
	XtSetArg(args[j], XtNlabel, XtNewString(_(mb->name))); j++;
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


/* callback when user clicks on an edit position popup menu item */
gboolean PieceMenuSelectGTK(w, eventkey, gdata)
     GtkWidget *w;
     GdkEventKey  *eventkey;
     gpointer  gdata;
{
    int piece = (intptr_t) gdata;

    if (pmFromX < 0 || pmFromY < 0) return;
    EditPositionMenuEvent(piece, pmFromX, pmFromY);
}

gboolean PieceMenuPopupGTK(window, eventbutton, data)
     GtkWindow *window;
     GdkEventButton *eventbutton;
     gpointer data;
{
    String whichMenu; int menuNr = -2;
    int black = -1, white = 0;
    ChessSquare selection;

    switch(eventbutton->button) {
      case 2:                                           // button press on button 2 (middle button)
        if (eventbutton->state & GDK_SHIFT_MASK) {
            shiftKey = black;
        } else {
            shiftKey = white;
        }
        break;
      case 3:                                           // button press on button 3 (right button)
        if (eventbutton->state & GDK_SHIFT_MASK) {
            shiftKey = white;
        } else {
            shiftKey = black;
        }
        break;
      default:
        break;
    }

    if (eventbutton->type == GDK_BUTTON_RELEASE)
        menuNr = RightClick(Release, eventbutton->x, eventbutton->y, &pmFromX, &pmFromY);
    else if (eventbutton->type == GDK_BUTTON_PRESS)
        menuNr = RightClick(Press,   eventbutton->x, eventbutton->y, &pmFromX, &pmFromY);

    switch(menuNr) {
      case 0:
        if (shiftKey == white) {
            whichMenu = "menuW";
        } else {
            whichMenu = "menuB";
        }
        break;
      case 1: SetupDropMenu(); whichMenu = "menuD"; break;
      case 2:
      case -1: if (errorUp) ErrorPopDown();
      default: return;
    }

    GtkWidget *menu;
    menu = gtk_menu_new();

    int i;
    for (i = 0; i < PIECE_MENU_SIZE; i++) {
        int color;
        GtkWidget *mi; // menuitem

        if (shiftKey == white) {
            color = 0;  // white
        } else {
            color = 1;  // black
        }

        String item = pieceMenuStrings[color][i];

        if (strcmp(item, "----") == 0) {
            mi = gtk_separator_menu_item_new();      // separator
        } else {
            mi = gtk_menu_item_new_with_label(item); // menuitem
        }

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(mi));
        gtk_widget_show(mi);

        selection = pieceMenuTranslation[color][i];

        g_signal_connect(mi, "button-press-event",
                      G_CALLBACK(PieceMenuSelectGTK),
                      (gpointer)(intptr_t) selection);
    }

    gtk_menu_popup(GTK_MENU(menu),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   eventbutton->button,
                   eventbutton->time);

    gtk_widget_show(menu);

}

void PieceMenuPopup(w, event, params, num_params)
     Widget w;
     XEvent *event;
     String *params;
     Cardinal *num_params;
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
    shiftKey = prms[0][0] & 1;
    ClockClick(0);
}

void BlackClock(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    shiftKey = prms[0][0] & 1;
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
    if (x < lineGapGTK)
      return -1;
    x -= lineGapGTK;
    if ((x % (squareSizeGTK + lineGapGTK)) >= squareSizeGTK)
      return -1;
    x /= (squareSizeGTK + lineGapGTK);
    if (x >= limit)
      return -2;
    return x;
}

static void do_flash_delay(msec)
     unsigned long msec;
{
    TimeDelay(msec);
}

#define LINE_TYPE_NORMAL 0
#define LINE_TYPE_HIGHLIGHT 1
#define LINE_TYPE_PRE 2

static void
drawHighlightGTK(file, rank, line_type)
     int file, rank, line_type;
{
  int x, y;
  guint32 col, tmp;
  gdouble r, g, b = 0.0;
  cairo_t *cr;

  if (lineGapGTK == 0 || appData.blindfold) return;

  if (flipView)
    {
      x = lineGapGTK/2 + ((BOARD_WIDTH-1)-file) *
	(squareSizeGTK + lineGapGTK);
      y = lineGapGTK/2 + rank * (squareSizeGTK + lineGapGTK);
    }
  else
    {
      x = lineGapGTK/2 + file * (squareSizeGTK + lineGapGTK);
      y = lineGapGTK/2 + ((BOARD_HEIGHT-1)-rank) *
	(squareSizeGTK + lineGapGTK);
    }

  /* get a cairo_t */
  cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

  /* draw the highlight */
  cairo_move_to (cr, x, y);
  cairo_rel_line_to (cr, 0,squareSizeGTK+lineGapGTK);
  cairo_rel_line_to (cr, squareSizeGTK+lineGapGTK,0);
  cairo_rel_line_to (cr, 0,-squareSizeGTK-lineGapGTK);
  cairo_close_path (cr);

  cairo_set_line_width (cr, lineGapGTK);
  switch(line_type)
    {
    case LINE_TYPE_HIGHLIGHT:
      sscanf(appData.highlightSquareColor, "#%x", &col);

      tmp = (col & 0x00ff0000) >> 16;
      r = (gdouble)tmp;
      r = r / 255;

      tmp = (col & 0x0000ff00) >> 8;
      g = (gdouble)tmp;
      g = g / 255;

      tmp = (col & 0x000000ff);
      b = (gdouble)tmp;
      b = b / 255;

      cairo_set_source_rgba (cr, r, g, b, 1.0);
      break;
    case LINE_TYPE_PRE:
      sscanf(appData.premoveHighlightColor, "#%x", &col);

      tmp = (col & 0x00ff0000) >> 16;
      r = (gdouble)tmp;
      r = r / 255;

      tmp = (col & 0x0000ff00) >> 8;
      g = (gdouble)tmp;
      g = g / 255;

      tmp = (col & 0x000000ff);
      b = (gdouble)tmp;
      b = b / 255;
      cairo_set_source_rgba (cr, r, g, b, 1.0);
      break;
    case LINE_TYPE_NORMAL:
    default:
      cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    }

  cairo_stroke (cr);

  /* free memory */
  cairo_destroy (cr);

  return;
}

int hi1X = -1, hi1Y = -1, hi2X = -1, hi2Y = -1;
int pm1X = -1, pm1Y = -1, pm2X = -1, pm2Y = -1;

void
SetHighlights(fromX, fromY, toX, toY)
     int fromX, fromY, toX, toY;
{
  if (hi1X != fromX || hi1Y != fromY)
    {
      if (hi1X >= 0 && hi1Y >= 0)
	{
	  drawHighlightGTK(hi1X, hi1Y, LINE_TYPE_NORMAL);
	}
    }
  if (hi2X != toX || hi2Y != toY)
    {
      if (hi2X >= 0 && hi2Y >= 0)
	{
	  drawHighlightGTK(hi2X, hi2Y, LINE_TYPE_NORMAL);
	}
    }
  if (hi1X != fromX || hi1Y != fromY)
    {
      if (fromX >= 0 && fromY >= 0)
	{
	  drawHighlightGTK(fromX, fromY, LINE_TYPE_HIGHLIGHT);
	}
    }
  if (hi2X != toX || hi2Y != toY)
    {
      if (toX >= 0 && toY >= 0)
	{
	  drawHighlightGTK(toX, toY, LINE_TYPE_HIGHLIGHT);
	}
    }
  hi1X = fromX;
  hi1Y = fromY;
  hi2X = toX;
  hi2Y = toY;

  return;
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
	    drawHighlightGTK(pm1X, pm1Y, LINE_TYPE_NORMAL);
	}
	if (fromX >= 0 && fromY >= 0) {
	    drawHighlightGTK(fromX, fromY, LINE_TYPE_PRE);
	}
    }
    if (pm2X != toX || pm2Y != toY) {
	if (pm2X >= 0 && pm2Y >= 0) {
	    drawHighlightGTK(pm2X, pm2Y, LINE_TYPE_NORMAL);
	}
	if (toX >= 0 && toY >= 0) {
	    drawHighlightGTK(toX, toY, LINE_TYPE_PRE);
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

static void BlankSquareGTK(x, y, color, piece, dest, fac)
     int x, y, color, fac;
     ChessSquare piece;
     Drawable dest;
{   // [HGM] extra param 'fac' for forcing destination to (0,0) for copying to animation buffer
    GdkPixbuf *pb=NULL;

    switch (color) {
      case 1: /* light */
        pb = SVGLightSquare;
        break;
      case 0: /* dark */
        pb = SVGDarkSquare;
        break;
      case 2: /* neutral */
        default:
        pb = SVGNeutralSquare;
       break;
    }

    if (!GTK_IS_WIDGET(boardwidgetGTK)) {
        printf("boardwidgetGTK not valid\n");
        return;
    }

    gdk_draw_pixbuf(GDK_WINDOW(boardwidgetGTK->window), NULL, pb, 0, 0, x, y, squareSizeGTK, squareSizeGTK, GDK_RGB_DITHER_NORMAL, 0, 0);

    return;
}

static void colorDrawPieceImageGTK(piece, square_color, x, y, dest)
     ChessSquare piece;
     int square_color, x, y;
     Drawable dest;
{
    int kind, p = piece;
    GdkPixbuf *pb=NULL;

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
    if(square_color+1) {
        BlankSquareGTK(x, y, square_color, piece, dest, 1); // erase previous contents with background

        pb = getPixbuf(p);

        //pb = SVGPawn;
        if (boardwidgetGTK == NULL) return;
        gdk_draw_pixbuf(GDK_WINDOW(boardwidgetGTK->window), NULL, pb, 0, 0, x, y, squareSizeGTK, squareSizeGTK, GDK_RGB_DITHER_NORMAL, 0, 0);
	//XSetClipMask(xDisplay, wlPieceGC, xpmMa.0sk[p]);
	//XSetClipOrigin(xDisplay, wlPieceGC, x, y);
	//XCopyArea(xDisplay, xpmPieceBitmap[kind][piece], dest, wlPieceGC, 0, 0, squareSize, squareSize, x, y);
	//XSetClipMask(xDisplay, wlPieceGC, None);
	//XSetClipOrigin(xDisplay, wlPieceGC, 0, 0);
    }
}

typedef void (*DrawFunc)();

DrawFunc ChooseDrawFuncGTK()
{
    return colorDrawPieceImageGTK;
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

void DrawSquareGTK(row, column, piece, do_flash)
     int row, column, do_flash;
     ChessSquare piece;
{
    int square_color, x, y;
    int i;
    char string[2];
    DrawFunc drawfunc;
    int flash_delay;

    /* Calculate delay in milliseconds (2-delays per complete flash) */
    flash_delay = 500 / appData.flashRate;

    if (flipView) {
	x = lineGapGTK + ((BOARD_WIDTH-1)-column) *
	  (squareSizeGTK + lineGapGTK);
	y = lineGapGTK + row * (squareSizeGTK + lineGapGTK);
    } else {
	x = lineGapGTK + column * (squareSizeGTK + lineGapGTK);
	y = lineGapGTK + ((BOARD_HEIGHT-1)-row) *
	  (squareSizeGTK + lineGapGTK);
    }

    if(twoBoards && partnerUp) x += hOffset; // [HGM] dual: draw second board

    square_color = SquareColor(row, column);

    if ( // [HGM] holdings: blank out area between board and holdings
	column == BOARD_LEFT-1 ||  column == BOARD_RGHT
	|| (column == BOARD_LEFT-2 && row < BOARD_HEIGHT-gameInfo.holdingsSize)
	|| (column == BOARD_RGHT+1 && row >= gameInfo.holdingsSize) )
      {
	cairo_text_extents_t extents;
	cairo_font_extents_t fe;
        cairo_t *cr;
        int  xpos, ypos;

	/* get a cairo_t */
        cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

        /* GTK-TODO this has to go into the font-selection */
        cairo_select_font_face (cr, "Sans",
                                CAIRO_FONT_SLANT_NORMAL,
                                CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size (cr, 12.0);

	/* get offset for font */
	cairo_font_extents (cr, &fe);

	BlankSquareGTK(x, y, 2, EmptySquare, xBoardWindow, 1);

	// [HGM] print piece counts next to holdings
	string[1] = NULLCHAR;
	if (column == (flipView ? BOARD_LEFT-1 : BOARD_RGHT) && piece > 1 )
	  {
	    string[0] = '0' + piece;
	    cairo_text_extents (cr, string, &extents);

	    xpos = x + squareSizeGTK - extents.width - 2;
            ypos = y + fe.ascent + 1;

	    cairo_move_to (cr, xpos, ypos);
	    cairo_text_path (cr, string);
	    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	    cairo_fill_preserve (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1.0);
	    cairo_set_line_width (cr, 0.1);
	    cairo_stroke (cr);
	  }
	if (column == (flipView ? BOARD_RGHT : BOARD_LEFT-1) && piece > 1)
	  {
	    string[0] = '0' + piece;
	    cairo_text_extents (cr, string, &extents);

	    xpos = x + 2;
            ypos = y + fe.ascent + 1;

	    cairo_move_to (cr, xpos, ypos);
	    cairo_text_path (cr, string);
	    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	    cairo_fill_preserve (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1.0);
	    cairo_set_line_width (cr, 0.1);
	    cairo_stroke (cr);
	}

	/* free memory */
        cairo_destroy (cr);

      }
    else
      {
	if (piece == EmptySquare || appData.blindfold)
	  {
	    BlankSquareGTK(x, y, square_color, piece, xBoardWindow, 1);
	  }
	else
	  {
	    drawfunc = ChooseDrawFuncGTK();

	    if (do_flash && appData.flashCount > 0) {
	      for (i=0; i<appData.flashCount; ++i) {
		drawfunc(piece, square_color, x, y, xBoardWindow);
		do_flash_delay(flash_delay);

		BlankSquareGTK(x, y, square_color, piece, xBoardWindow, 1);
		do_flash_delay(flash_delay);
	      }
	    }
	    drawfunc(piece, square_color, x, y, xBoardWindow);
	  }
      }

    string[1] = NULLCHAR;
    if (appData.showCoords)
      {
	cairo_text_extents_t extents;
	cairo_font_extents_t fe;
        cairo_t *cr;
        int  xpos, ypos;

	/* get a cairo_t */
        cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

        /* GTK-TODO this has to go into the font-selection */
        cairo_select_font_face (cr, "Sans",
                                CAIRO_FONT_SLANT_NORMAL,
                                CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size (cr, 12.0);

	/* get offset for font */
	cairo_font_extents (cr, &fe);

	if( row == (flipView ? BOARD_HEIGHT-1 : 0)
	    && column >= BOARD_LEFT && column < BOARD_RGHT)
	  {
	    string[0] = 'a' + column - BOARD_LEFT;
	    cairo_text_extents (cr, string, &extents);

	    xpos = x + squareSizeGTK - extents.width - 2;
            ypos = y + squareSizeGTK - fe.descent - 1;

	    cairo_move_to (cr, xpos, ypos);
	    cairo_text_path (cr, string);
	    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	    cairo_fill_preserve (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1.0);
	    cairo_set_line_width (cr, 0.1);
	    cairo_stroke (cr);
	  }
	if (column == (flipView ? BOARD_RGHT-1 : BOARD_LEFT))
	  {
	    string[0] = ONE + row;
	    cairo_text_extents (cr, string, &extents);

	    xpos = x + 2;
            ypos = y + extents.height + 1;

	    cairo_move_to (cr, xpos, ypos);
	    cairo_text_path (cr, string);
	    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	    cairo_fill_preserve (cr);
	    cairo_set_source_rgb (cr, 0, 0, 1.0);
	    cairo_set_line_width (cr, 0.1);
	    cairo_stroke (cr);
	  }

        /* free memory */
        cairo_destroy (cr);

      }
    if(!partnerUp && marker[row][column])
      {
        cairo_t *cr;

	/* get a cairo_t */
        cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

     	cairo_arc(cr, x + squareSize/4,y+squareSize/4, squareSize/2, 0.0, 2*M_PI);

	cairo_set_line_width (cr, 0.1);

	if (marker[row][column] == 2 )
	  {
	    /* preline */
	    cairo_set_source_rgba(cr, 0, 0, 0,1.0);
	    cairo_stroke_preserve(cr);
	    cairo_set_source_rgba(cr, 1.0, 0, 0,1.0);
	    cairo_fill(cr);
	  }
	else
	  {
	    guint32 col, tmp;
	    gdouble r, g, b = 0.0;

	    /* highline */
	    sscanf(appData.highlightSquareColor, "#%x", &col);

	    tmp = (col & 0x00ff0000) >> 16;
	    r = (gdouble)tmp;
	    r = r / 255;

	    tmp = (col & 0x0000ff00) >> 8;
	    g = (gdouble)tmp;
	    g = g / 255;

	    tmp = (col & 0x000000ff);
	    b = (gdouble)tmp;
	    b = b / 255;

	    cairo_set_source_rgba (cr, r, g, b, 1.0);

	    cairo_stroke_preserve(cr);
	    cairo_fill(cr);
	  }
	/* free memory */
        cairo_destroy (cr);
    }
}

/* return linegap based on square size */
/* called from ConfigureProc in xboard.c and BoardOptionsOK in xoptions.c */
int GetLineGap()
{
    int gap;

    if (squareSizeGTK > 112) gap = 4;
    else if (squareSizeGTK > 57) gap = 3;
    else if (squareSizeGTK > 35) gap = 2;
    else gap = 1;

    return gap;
}

void ScalePixbufs() {
    SVGscWhitePawn     = gdk_pixbuf_scale_simple(SVGWhitePawn, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscWhiteKnight   = gdk_pixbuf_scale_simple(SVGWhiteKnight, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscWhiteBishop   = gdk_pixbuf_scale_simple(SVGWhiteBishop, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscWhiteRook     = gdk_pixbuf_scale_simple(SVGWhiteRook, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscWhiteQueen    = gdk_pixbuf_scale_simple(SVGWhiteQueen, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscWhiteCardinal = gdk_pixbuf_scale_simple(SVGWhiteCardinal, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscWhiteMarshall = gdk_pixbuf_scale_simple(SVGWhiteMarshall, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscWhiteKing     = gdk_pixbuf_scale_simple(SVGWhiteKing, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);

    SVGscBlackPawn     = gdk_pixbuf_scale_simple(SVGBlackPawn, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscBlackKnight   = gdk_pixbuf_scale_simple(SVGBlackKnight, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscBlackBishop   = gdk_pixbuf_scale_simple(SVGBlackBishop, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscBlackRook     = gdk_pixbuf_scale_simple(SVGBlackRook, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscBlackQueen    = gdk_pixbuf_scale_simple(SVGBlackQueen, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscBlackCardinal = gdk_pixbuf_scale_simple(SVGBlackCardinal, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscBlackMarshall = gdk_pixbuf_scale_simple(SVGBlackMarshall, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
    SVGscBlackKing     = gdk_pixbuf_scale_simple(SVGBlackKing, squareSizeGTK, squareSizeGTK, GDK_INTERP_HYPER);
}

/* The user has resized the main window so redraw the board with the correct size */
gboolean ConfigureProc(widget, event, data)
     GtkWidget *widget;
     GdkEvent *event;
     gpointer data;
{
    gint width, height, calcwidth, calcheight;;
    int w, h;

    gdk_drawable_get_size(boardwidgetGTK->window, &width, &height);

    /* calc squaresize based on previous linegap */
    //squareSizeGTK = ( (width - lineGapGTK * (BOARD_WIDTH + 1)) / BOARD_WIDTH);
    w = ( (width - lineGapGTK * (BOARD_WIDTH + 1)) / BOARD_WIDTH);
    h = ( (height - lineGapGTK * (BOARD_HEIGHT + 1)) / BOARD_HEIGHT);
    //w = (width - 4) / BOARD_WIDTH;
    //h = (height - 4) / BOARD_HEIGHT;
    squareSizeGTK = w < h ? w : h;

    /* calc linegap */
    if(appData.overrideLineGap >= 0)
        lineGapGTK = appData.overrideLineGap;
    else
        lineGapGTK = GetLineGap();

    /* recalc squaresize */
    squareSizeGTK = ( (width - lineGapGTK * (BOARD_WIDTH + 1)) / BOARD_WIDTH);

    calcwidth = BOARD_WIDTH * (squareSizeGTK + lineGapGTK) + lineGapGTK;
    calcheight = BOARD_HEIGHT * (squareSizeGTK + lineGapGTK) + lineGapGTK;

    /* set the size of boardwidgetGTK GdkWindow to exactly the same as the size of the board */
    /* (it may be a few pixels larger since after user resize of main window) */
    if (calcwidth != width || calcheight != height) {
        gdk_window_resize(boardwidgetGTK->window, calcwidth,calcheight);
    }

    ScalePixbufs(); /* scale pixbufs to correct size */

    return False;
}

/* callback for expose event on the main GtkDrawingArea (boardwidgetGTK) */
/* causes board to be redrawn */
gboolean EventProcGTK(widget, event, data)
     GtkWidget *widget;
     GdkEventExpose *event;
     gpointer data;
{
    switch (event->type) {
      case GDK_EXPOSE:
	//if (event->expose.count > 0) return;  // no clipping is done
	GTKDrawPosition(widget, True, NULL);
//	if(twoBoards) { // [HGM] dual: draw other board in other orientation
//	    flipView = !flipView; partnerUp = !partnerUp;
//	    XDrawPosition(widget, True, NULL);
//	    flipView = !flipView; partnerUp = !partnerUp;
//	}
	break;
//      case MotionNotify:
//        if(SeekGraphClick(Press, event->xbutton.x, event->xbutton.y, 1)) break;
      default:
	return False;
    }
    return False;
}

void DrawPosition(fullRedraw, board)
     /*Boolean*/int fullRedraw;
     Board board;
{
    GTKDrawPosition(boardwidgetGTK, fullRedraw, board);
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
    cairo_t *cr;

    /* get a cairo_t */
    cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

    cairo_move_to (cr, x, y);
    cairo_line_to(cr, xTo, yTo );

    /* GTK-TODO: use user colors */
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_stroke(cr);

    /* free memory */
    cairo_destroy (cr);
}

void DrawSeekBackground( int left, int top, int right, int bottom )
{
    cairo_t *cr;

    /* get a cairo_t */
    cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

    cairo_rectangle (cr, left, top, right-left, bottom-top);

    cairo_set_line_width(cr, 2);
    cairo_set_source_rgba (cr, 1.0, 1.0, 0.8, 1.0);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgba(cr, 0, 0, 0,1.0);
    cairo_fill(cr);

    /* free memory */
    cairo_destroy (cr);
}

void DrawSeekText(char *buf, int x, int y)
{
    cairo_t *cr;
    /* get a cairo_t */
    cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

    /* GTK-TODO: use user font */
    cairo_select_font_face (cr, "Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size (cr, 12.0);

    cairo_move_to (cr, x, y+4);
    cairo_show_text( cr, buf);

    cairo_set_source_rgba(cr, 0, 0, 0,1.0);
    cairo_stroke(cr);

    /* free memory */
    cairo_destroy (cr);
}

void DrawSeekDot(int x, int y, int colorNr)
{
    int square = colorNr & 0x80;
    colorNr &= 0x7F;

    cairo_t *cr;

    /* get a cairo_t */
    cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

    if(square)
      {
	cairo_rectangle (cr, x-squareSize/9, y-squareSize/9, 2*squareSize/9, 2*squareSize/9);
      }
    else
      {
	cairo_arc(cr, x-squareSize/8, y-squareSize/8, squareSize/4, 0.0, 2*M_PI);
      }

    cairo_set_line_width(cr, 2);

    /* GTK-TODO: use user colors */
    switch (colorNr)
      {
      case 0: /* preline */
	cairo_set_source_rgba(cr, 0, 0, 0,1.0);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgba(cr, 1.0, 0, 0,1.0);
	cairo_fill(cr);
	break;
      case 1: /* darkSquare */
	cairo_set_source_rgba(cr, 0, 0, 0,1.0);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgba (cr, 0.5, 0.5, 0.8, 1.0);
	cairo_fill(cr);
	break;
      default: /* lightSquare */
	cairo_set_source_rgba(cr, 0, 0, 0,1.0);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgba (cr, 1.0, 1.0, 0.8, 1.0);
	cairo_fill(cr);
	break;
      }

    /* free memory */
    cairo_destroy (cr);
}

static int damageGTK[2][BOARD_RANKS][BOARD_FILES];

void GTKDrawPosition(w, repaint, board)
     GtkWidget *w;
     /*Boolean*/int repaint;
     Board board;
{
    int i, j, do_flash;
    static int lastFlipView = 0;
    static int lastBoardValid[2] = {0, 0};
    static Board lastBoard[2];
    int rrow, rcol;
    int nr = twoBoards*partnerUp;

    if(DrawSeekGraph()) return; // [HGM] seekgraph: suppress any drawing if seek graph up

    if (board == NULL) {
	if (!lastBoardValid[nr]) return;
	board = lastBoard[nr];
    }
    if (!lastBoardValid[nr] || (nr == 0 && lastFlipView != flipView)) {
	//XtSetArg(args[0], XtNleftBitmap, (flipView ? xMarkPixmap : None));
	//XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Flip View"),
	//	    args, 1);
    }


    /*
     * It would be simpler to clear the window with XClearWindow()
     * but this causes a very distracting flicker.
     */

    if (!repaint && lastBoardValid[nr] && (nr == 1 || lastFlipView == flipView))
      {
	if ( lineGap && IsDrawArrowEnabled())
	  CreateGridGTK();

	/* If too much changes (begin observing new game, etc.), don't
	   do flashing */
	do_flash = too_many_diffs(board, lastBoard[nr]) ? 0 : 1;

	/* Special check for castling so we don't flash both the king
	   and the rook (just flash the king). */
	if (do_flash)
	  {
	    if (check_castle_draw(board, lastBoard[nr], &rrow, &rcol)) {
	      /* Draw rook with NO flashing. King will be drawn flashing later */
	      DrawSquareGTK(rrow, rcol, board[rrow][rcol], 0);
	      lastBoard[nr][rrow][rcol] = board[rrow][rcol];
	    }
	  }

	/* First pass -- Draw (newly) empty squares and repair damage.
	   This prevents you from having a piece show up twice while it
	   is flashing on its new square */
	for (i = 0; i < BOARD_HEIGHT; i++) {
	  for (j = 0; j < BOARD_WIDTH; j++) {
	    if ((board[i][j] != lastBoard[nr][i][j] && board[i][j] == EmptySquare)
		|| damageGTK[nr][i][j]) {
	      DrawSquareGTK(i, j, board[i][j], 0);
	      damageGTK[nr][i][j] = False;
	    }
          }
        }

	/* Second pass -- Draw piece(s) in new position and flash them */
	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++)
	    if (board[i][j] != lastBoard[nr][i][j]) {
	      DrawSquareGTK(i, j, board[i][j], do_flash);
	    }
      }
    else
      {
	if (lineGap > 0)
	  {
	    if(twoBoards & partnerUp)
	      CreateSecondGridGTK();
	    else
	      CreateGridGTK();
	  }

	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++) {
	      DrawSquareGTK(i, j, board[i][j], 0);
	      damageGTK[nr][i][j] = False;
	  }
    }

    CopyBoard(lastBoard[nr], board);
    lastBoardValid[nr] = 1;

    CreateGridGTK();

  if(nr == 0) { // [HGM] dual: no highlights on second board yet
    lastFlipView = flipView;

    /* Draw highlights */

    /* premove highlights */
    /* highlight 'from' square */
    if (pm1X >= 0 && pm1Y >= 0) {
      drawHighlightGTK(pm1X, pm1Y, LINE_TYPE_PRE);
    }
    /* highlight 'to' square */
    if (pm2X >= 0 && pm2Y >= 0) {
      drawHighlightGTK(pm2X, pm2Y, LINE_TYPE_PRE);
    }

    /* move highlights */
    /* highlight 'from' square */
    if (hi1X >= 0 && hi1Y >= 0) {
      drawHighlightGTK(hi1X, hi1Y, LINE_TYPE_HIGHLIGHT);
    }
    /* highlight 'to' square */
    if (hi2X >= 0 && hi2Y >= 0) {
      drawHighlightGTK(hi2X, hi2Y, LINE_TYPE_HIGHLIGHT);
    }
    DrawArrowHighlightGTK(hi1X, hi1Y, hi2X, hi2Y);
  }
    /* If piece being dragged around board, must redraw that too */
    DrawDragPiece();
}

gboolean KeyPressProc(window, eventkey, data)
     GtkWindow *window;
     GdkEventKey  *eventkey;
     gpointer data;
{
    // handle shift+<number> cases
    if (eventkey->state & GDK_SHIFT_MASK) {
        guint keyval;

        gdk_keymap_translate_keyboard_state(NULL, eventkey->hardware_keycode,
					    0, eventkey->group,
					    &keyval, NULL, NULL, NULL);
        switch(keyval) {
            case GDK_1:
                AskQuestionEvent("Direct command", "Send to chess program:", "", "1");
                break;
            case GDK_2:
                AskQuestionEvent("Direct command", "Send to second chess program:", "", "2");
                break;
            default:
                break;
        }
    }

    /* check for other key values */
    switch(eventkey->keyval) {
        case GDK_question:
	  AboutGameEvent();
	  break;
        default:
	  break;
    }
    return False;
}

gboolean ButtonPressProc(window, eventbutton, data)
     GtkWindow *window;
     GdkEventButton  *eventbutton;
     gpointer data;
{
    switch(eventbutton->button) {
      case 1:
        HandleUserMoveGTK(window, eventbutton, data);   // button press or release on button 1 (left button)
        break;
      case 2:
      case 3:
        PieceMenuPopupGTK(window, eventbutton, data);   // button press or release on button 2/3 (middle/right buton)
        break;
      default:
        break;
    }
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

gboolean HandleUserMoveGTK(window, eventbutton, data)
     GtkWindow *window;
     GdkEventButton *eventbutton;
     gpointer data;
{
  // GTK-TODO do we need to check for the correct window or is the callback set to this window anyway?
  //if (window != GDK_WINDOW(boardwidgetGTK->window) || errorExitStatus != -1) return;
    if (errorExitStatus != -1) return;
    shiftKey = eventbutton->state & GDK_BUTTON1_MASK;

    if (promotionUp) {
        if (eventbutton->type == GDK_BUTTON_PRESS) {
	    PromotionPopDown();
	    ClearHighlights();
	    fromX = fromY = -1;
	} else {
	    return False;
	}
    }

    // [HGM] mouse: the rest of the mouse handler is moved to the backend, and called here
    if(eventbutton->type == GDK_BUTTON_PRESS)   LeftClick(Press,   (int)eventbutton->x, (int)eventbutton->y);
    if(eventbutton->type == GDK_BUTTON_RELEASE) LeftClick(Release, (int)eventbutton->x, (int)eventbutton->y);

    return False;
}

void
AnimateUserMove (GtkWidget *w, GdkEventMotion *event)
{
  int x, y;
  GdkModifierType state;

  if (event->is_hint)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }

  if (state & GDK_BUTTON1_MASK)
    {
      DragPieceMove(x, y);
    }
}

/*
void AnimateUserMove (Widget w, XEvent * event,
		      String * params, Cardinal * nParams)
{
    if(!PromoScroll(event->xmotion.x, event->xmotion.y))
    DragPieceMove(event->xmotion.x, event->xmotion.y);
}
*/

void HandlePV (Widget w, XEvent * event,
		      String * params, Cardinal * nParams)
{   // [HGM] pv: walk PV
    MovePV(event->xmotion.x, event->xmotion.y, lineGap + BOARD_HEIGHT * (squareSize + lineGap));
}

static int savedIndex;  /* gross that this is global */

void CommentClick(tb)
    GtkTextBuffer *tb;
{
    String val;
    gint index;
    GtkTextIter start;
    GtkTextIter end;

    /* get cursor position into index */
    g_object_get(tb, "cursor-position", &index, NULL);

    /* get text from textbuffer */
    gtk_text_buffer_get_start_iter (tb, &start);
    gtk_text_buffer_get_end_iter (tb, &end);
    val = gtk_text_buffer_get_text (tb, &start, &end, FALSE);

    ReplaceComment(savedIndex, val);
    if(savedIndex != currentMove) ToNrEvent(savedIndex);
    LoadVariation( index, val ); // [HGM] also does the actual moving to it, now
}

void EditCommentPopUp(index, title, text)
     int index;
     char *title, *text;
{
    savedIndex = index;
    if (text == NULL) text = "";
    NewCommentPopup(title, text, index);
}

void ICSInputBoxPopUp()
{
    InputBoxPopup();
}

extern Option boxOptions[];

void ICSInputSendText()
{
    GtkWidget *edit;
    String val;

    edit = boxOptions[0].handle;
    val = (String)gtk_entry_get_text (GTK_ENTRY (edit));

    SaveInHistory(val);
    SendMultiLineToICS(val);

    /* clear the text in the GTKEntry */
    gtk_entry_set_text(GTK_ENTRY(edit), "");
}

void ICSInputBoxPopDown()
{
    PopDown(4);
}

void CommentPopUp(title, text)
     char *title, *text;
{
    savedIndex = currentMove; // [HGM] vari
    NewCommentPopup(title, text, currentMove);
}

void CommentPopDown()
{
    PopDown(1);
}

void FileNamePopUp(label, def, filter, proc, openMode, action)
     char *label;
     char *def;
     char *filter;
     FileProc proc;
     char *openMode;
     FileAction action;
{
  GtkWidget     *dialog;
  GtkFileFilter *gtkfilter;
  GtkFileFilter *gtkfilter_all;
  char space[]     = " ";
  char fileext[10] = "";
  char *result     = NULL;
  char *cp;

  /* make a copy of the filter string, so that strtok can work with it*/
  cp = strndup(filter,strlen(filter));

  /* add filters for file extensions */
  gtkfilter     = gtk_file_filter_new();
  gtkfilter_all = gtk_file_filter_new();

  /* one filter to show everything */
  gtk_file_filter_add_pattern(gtkfilter_all, "*.*");
  gtk_file_filter_set_name   (gtkfilter_all, "All Files");

  /* add filter if present */
  result = strtok(cp, space);
  while( result != NULL  ) {
    snprintf(fileext,10,"*%s",result);
    result = strtok( NULL, space );
    gtk_file_filter_add_pattern(gtkfilter, fileext);
  };

  /* second filter to only show what's useful */
  gtk_file_filter_set_name (gtkfilter,filter);

  if (action==OPEN)
    {
      dialog = gtk_file_chooser_dialog_new (label,
					    NULL,
					    GTK_FILE_CHOOSER_ACTION_OPEN,
					    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					    NULL);
    }
  else
    {
      dialog = gtk_file_chooser_dialog_new (label,
					    NULL,
					    GTK_FILE_CHOOSER_ACTION_SAVE,
					    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					    NULL);
      /* add filename suggestions */
      if (strlen(def) > 0 )
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), def);

      //gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER (dialog),TRUE);
    }

  /* add filters */
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog),gtkfilter_all);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog),gtkfilter);
  /* activate filter */
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(dialog),gtkfilter);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      FILE *f;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      //see loadgamepopup
      f = fopen(filename, openMode);
      if (f == NULL)
        {
          DisplayError(_("Failed to open file"), errno);
        }
      else
        {
          /* TODO add indec */
          (*proc)(f, 0, filename);
        }
      g_free (filename);
    };

  gtk_widget_destroy (dialog);
  ModeHighlight();

  free(cp);
  return;

}

void PromotionPopUp()
{
    GtkWidget *label;
    GtkWidget *content_area;

    promotionShellGTK = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(promotionShellGTK), "Promotion");

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(promotionShellGTK));
    label = gtk_label_new(_("Promote to what?"));
    gtk_container_add(GTK_CONTAINER(content_area), label);

    if(gameInfo.variant != VariantShogi) {
        if(gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove)) {
            gtk_dialog_add_buttons(GTK_DIALOG(promotionShellGTK),
                           _("Warlord"),  1,
                           _("General"),   2,
                           _("Lieutenant"), 3,
                           _("Captain"), 4,
                           NULL);
        } else {
            gtk_dialog_add_buttons(GTK_DIALOG(promotionShellGTK),
                           _("Queen"),  5,
                           _("Rook"),   6,
                           _("Bishop"), 7,
                           _("Knight"), 8,
                           NULL);
        }
        if (!appData.testLegality || gameInfo.variant == VariantSuicide ||
            gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove) ||
            gameInfo.variant == VariantGiveaway) {
            gtk_dialog_add_button(GTK_DIALOG(promotionShellGTK), _("King"), 9);
        }
        if(gameInfo.variant == VariantCapablanca ||
            gameInfo.variant == VariantGothic ||
            gameInfo.variant == VariantCapaRandom) {
            gtk_dialog_add_buttons(GTK_DIALOG(promotionShellGTK),
                           _("Archbishop"),  10,
                           _("Chancellor"),  11,
                           NULL);
        }
    } else { // [HGM] shogi
        gtk_dialog_add_buttons(GTK_DIALOG(promotionShellGTK),
                           _("Promote"),  12,
                            _("Defer"),   13,
                           NULL);
    }

    gtk_dialog_add_button(GTK_DIALOG(promotionShellGTK), _("cancel"), 14);

    gtk_widget_show_all(promotionShellGTK);

    g_signal_connect (promotionShellGTK, "response",
                      G_CALLBACK (PromotionCallback), NULL);

    g_signal_connect (promotionShellGTK, "delete-event",
                      G_CALLBACK (PromotionPopDown), NULL);

    promotionUp = True;
}

void PromotionPopDown()
{
    if (!promotionUp) return;
    gtk_widget_destroy(GTK_WIDGET(promotionShellGTK));
    promotionUp = False;
}

void PromotionCallback(w, resptype, gdata)
     GtkWidget *w;
     GtkResponseType  resptype;
     gpointer  gdata;
{
    int promoChar;
    GtkWidget *button;
    GList *gl;
    gint respid;
    gchar *name;

    /* get list of buttons in the dialog action area */
    gl = gtk_container_get_children(GTK_CONTAINER(GTK_CONTAINER(gtk_dialog_get_action_area(GTK_DIALOG(w)))));

    /* find the button that was clicked */
    /* cannot use 'gtk_dialog_get_widget_for_response()' since that requires gtk 2.20
       and we are targeting 2.16 */
    while (gl) {
        button = GTK_WIDGET(gl->data);
        respid = gtk_dialog_get_response_for_widget(GTK_DIALOG(w), GTK_WIDGET(button));
        if (respid == resptype) {
            name = g_strdup(gtk_button_get_label(GTK_BUTTON(button)));
            break;
        }
        gl = gl->next;
    }
    g_list_free(gl);

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

    g_free(name);

    UserMoveEvent(fromX, fromY, toX, toY, promoChar);

    if (!appData.highlightLastMove || gotPremove) ClearHighlights();
    if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
    fromX = fromY = -1;
}

void ErrorCallback(w, event, data)
     GtkWidget *w;
     GdkEvent  *event;
     gpointer  data;
{
    errorUp = False;
    gtk_widget_destroy (w);
    errorShell = NULL;
}


void ErrorPopDown()
{
    if (!errorUp) return;
    errorUp = False;
    if (errorShell != NULL)
      {
        gtk_widget_destroy (errorShell);
        errorShell = NULL;
      }
}

void ErrorPopUp(title, label, modal)
     char *title, *label;
     int modal;
{
    int msgtype;

    if (strcmp(title, "Fatal Error") == 0 || strcmp(title, "Exiting") == 0)
      msgtype = GTK_MESSAGE_ERROR;   /* Fatal error message */
    else if (strcmp (title, "Error") == 0)
      msgtype = GTK_MESSAGE_WARNING; /* Nonfatal warning message */
    else
      msgtype = GTK_MESSAGE_INFO;    /* Informational message */

    errorShell = gtk_message_dialog_new(GTK_WINDOW(mainwindow),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            msgtype,
            GTK_BUTTONS_OK,
            "%s", label);
    gtk_window_set_title (GTK_WINDOW(errorShell), title);
    gtk_widget_show (errorShell);
    errorUp = True;
    if (modal)
      {
        gtk_dialog_run (GTK_DIALOG(errorShell));
        gtk_widget_destroy (errorShell);
        errorShell = NULL;
        errorUp = False;
      }
    else
      g_signal_connect (errorShell, "response",
                        G_CALLBACK (ErrorCallback),
                        errorShell);
}

/* Disable all user input other than deleting the window */
static int frozen = 0;
void FreezeUI()
{
  if (frozen) return;
  /* Grab by a widget that doesn't accept input */
  //  XtAddGrab(messageWidget, TRUE, FALSE);
  frozen = 1;
}

/* Undo a FreezeUI */
void ThawUI()
{
  if (!frozen) return;
  //XtRemoveGrab(messageWidget);
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
    static int oldPausing = FALSE;
    static GameMode oldmode = (GameMode) -1;
    char *wname;

    if (!boardwidgetGTK || !gtk_widget_get_realized(GTK_WIDGET(boardwidgetGTK))) return;

    if (pausing != oldPausing) {
	oldPausing = pausing;
	if (pausing) {
	  //	    XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
	} else {
	  //	    XtSetArg(args[0], XtNleftBitmap, None);
	}
	//	XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Pause"),
	//	    args, 1);

	if (appData.showButtonBar) {
	  /* Always toggle, don't set.  Previous code messes up when
	     invoked while the button is pressed, as releasing it
	     toggles the state again. */
	  {
//	    Pixel oldbg, oldfg;
//	    XtSetArg(args[0], XtNbackground, &oldbg);
//	    XtSetArg(args[1], XtNforeground, &oldfg);
//	    XtGetValues(XtNameToWidget(buttonBarWidget, PAUSE_BUTTON),
//			args, 2);
//	    XtSetArg(args[0], XtNbackground, oldfg);
//	    XtSetArg(args[1], XtNforeground, oldbg);
	  }
//	  XtSetValues(XtNameToWidget(buttonBarWidget, PAUSE_BUTTON), args, 2);
	}
    }

    wname = ModeToWidgetName(oldmode);
    if (wname != NULL) {
      //	XtSetArg(args[0], XtNleftBitmap, None);
      //	XtSetValues(XtNameToWidget(menuBarWidget, wname), args, 1);
    }
    wname = ModeToWidgetName(gameMode);
    if (wname != NULL) {
      //	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
      //	XtSetValues(XtNameToWidget(menuBarWidget, wname), args, 1);
    }
    oldmode = gameMode;
    //    XtSetArg(args[0], XtNleftBitmap, matchMode && matchGame < appData.matchGames ? xMarkPixmap : None);
    //    XtSetValues(XtNameToWidget(menuBarWidget, "menuMode.Machine Match"), args, 1);

    /* Maybe all the enables should be handled here, not just this one */
    //    XtSetSensitive(XtNameToWidget(menuBarWidget, "menuMode.Training"),
    //		   gameMode == Training || gameMode == PlayFromGameFile);
}


/*
 * Button/menu procedures
 */
void ResetProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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
	} else {
            SetCheckMenuItemActive(NULL, 102, False); // set GTK menu item to unchecked
        }
	GameListDestroy();
	gameNumber = 1;
    }
    return LoadGame(f, gameNumber, title, FALSE);
}

void LoadGameProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	Reset(FALSE, TRUE);
    }
    FileNamePopUp(_("Load game file name?"), "", ".pgn .game", LoadGamePopUp, "rb", OPEN);
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

void LoadNextPositionProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadPosition(1);
}

void LoadPrevPositionProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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

void LoadPositionProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	Reset(FALSE, TRUE);
    }
    FileNamePopUp(_("Load position file name?"), "", ".fen .epd .pos", LoadPosition, "rb", OPEN);
}

void SaveGameProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    FileNamePopUp(_("Save game file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "game" : "pgn"),
		  appData.oldSaveStyle ? ".game" : ".pgn",
		  SaveGame, "a",SAVE);
}

void SavePositionProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    FileNamePopUp(_("Save position file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "pos" : "fen"),
		  appData.oldSaveStyle ? ".pos" : ".fen",
		  SavePosition, "a",SAVE);
}

void ReloadCmailMsgProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadCmailMsgEvent(FALSE);
}

void MailMoveProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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

void CopyPositionProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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
//    XtOwnSelection(menuBarWidget, XA_PRIMARY,
//		   CurrentTime,
//		   SendPositionSelection,
//		   NULL/* lose_ownership_proc */ ,
//		   NULL/* transfer_done_proc */);
//    XtOwnSelection(menuBarWidget, XA_CLIPBOARD(xDisplay),
//		   CurrentTime,
//		   SendPositionSelection,
//		   NULL/* lose_ownership_proc */ ,
//		   NULL/* transfer_done_proc */);
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
  //  XtFree(value);
}

void PastePositionProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
//    XtGetSelectionValue(menuBarWidget,
//      appData.pasteSelection ? XA_PRIMARY: XA_CLIPBOARD(xDisplay), XA_STRING,
//      /* (XtSelectionCallbackProc) */ PastePositionCB,
//      NULL, /* client_data passed to PastePositionCB */
//
//      /* better to use the time field from the event that triggered the
//       * call to this function, but that isn't trivial to get
//       */
//      CurrentTime
//    );
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

void CopySomething()
{
  /*
   * Set both PRIMARY (the selection) and CLIPBOARD, since we don't
   * have a notion of a game that is selected but not copied.
   * See http://www.freedesktop.org/wiki/Specifications/ClipboardsWiki
   */
//  XtOwnSelection(menuBarWidget, XA_PRIMARY,
//		 CurrentTime,
//		 SendGameSelection,
//		 NULL/* lose_ownership_proc */ ,
//		 NULL/* transfer_done_proc */);
//  XtOwnSelection(menuBarWidget, XA_CLIPBOARD(xDisplay),
//		 CurrentTime,
//		 SendGameSelection,
//		 NULL/* lose_ownership_proc */ ,
//		 NULL/* transfer_done_proc */);
}

void CopyGameProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  int ret;

  ret = SaveGameToFile(gameCopyFilename, FALSE);
  if (!ret) return;

  CopySomething();
}

void CopyGameListProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if(!SaveGameListAsText(fopen(gameCopyFilename, "w"))) return;
  CopySomething();
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
  //  XtFree(value);
  LoadGameFromFile(gamePasteFilename, 0, gamePasteFilename, TRUE);
}

void PasteGameProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
//    XtGetSelectionValue(menuBarWidget,
//      appData.pasteSelection ? XA_PRIMARY: XA_CLIPBOARD(xDisplay), XA_STRING,
//      /* (XtSelectionCallbackProc) */ PasteGameCB,
//      NULL, /* client_data passed to PasteGameCB */
//
//      /* better to use the time field from the event that triggered the
//       * call to this function, but that isn't trivial to get
//       */
//      CurrentTime
//    );
    return;
}

void AutoSaveGame()
{
    SaveGameProcGTK(NULL, NULL);
}

/* exit the application */

void
QuitProcGTK(GtkObject *object, gpointer user_data)
{
    ExitEvent(0);
    gtk_main_quit();
}

void PauseProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    PauseEvent();
}

void MachineBlackProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    MachineBlackEvent();
}

void MachineWhiteProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    MachineWhiteEvent();
}

void AnalyzeModeProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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

void AnalyzeFileProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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
    FileNamePopUp(_("File to analyze"), "", ".pgn .game", LoadGamePopUp, "rb",OPEN);
    AnalysisPeriodicEvent(1);
}

void TwoMachinesProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    TwoMachinesEvent();
}

void MatchProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    MatchEvent(2);
}

void IcsClientProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    IcsClientEvent();
}

void EditGameProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    EditGameEvent();
}

void EditPositionProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    EditPositionEvent();
}

void TrainingProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    TrainingEvent();
}

void EditCommentProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    Arg args[5];
    int j;
    if (PopDown(1)) { // popdown succesful
	j = 0;
	XtSetArg(args[j], XtNleftBitmap, None); j++;
	XtSetValues(XtNameToWidget(menuBarWidget, "menuEdit.Edit Comment"), args, j);
	XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Comments"), args, j);
    } else // was not up
	EditCommentEvent();
}

void IcsInputBoxProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    if (!PopDown(4)) ICSInputBoxPopUp();
}

void AcceptProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    AcceptEvent();
}

void DeclineProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    DeclineEvent();
}

void RematchProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    RematchEvent();
}

void CallFlagProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    CallFlagEvent();
}

void DrawProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    DrawEvent();
}

void AbortProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    AbortEvent();
}

void AdjournProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    AdjournEvent();
}

void ResignProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ResignEvent();
}

void AdjuWhiteProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    UserAdjudicationEvent(+1);
}

void AdjuBlackProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    UserAdjudicationEvent(-1);
}

void AdjuDrawProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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

void StopObservingProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    StopObservingEvent();
}

void StopExaminingProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    StopExaminingEvent();
}

void UploadProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    UploadGameEvent();
}

void ForwardProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ForwardEvent();
}

void BackwardProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    BackwardEvent();
}

void ToStartProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ToStartEvent();
}

void ToEndProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ToEndEvent();
}

void RevertProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    RevertEvent(False);
}

void AnnotateProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    RevertEvent(True);
}

void TruncateGameProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    TruncateGameEvent();
}

void RetractMoveProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    RetractMoveEvent();
}

void MoveNowProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    MoveNowEvent();
}

void FlipViewProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
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
    PonderNextMoveEvent(!appData.ponderNextMove);
}

#ifndef OPTIONSDIALOG
void AlwaysQueenProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.alwaysPromoteToQueen = !appData.alwaysPromoteToQueen;
}

void AnimateDraggingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.animateDragging = !appData.animateDragging;
}

void AnimateMovingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.animate = !appData.animate;
}

void AutoflagProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.autoCallFlag = !appData.autoCallFlag;
}

void AutoflipProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.autoFlipView = !appData.autoFlipView;
}

void BlindfoldProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.blindfold = !appData.blindfold;

    DrawPosition(True, NULL);
}

void TestLegalityProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.testLegality = !appData.testLegality;
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
      //	XtSetArg(args[0], XtNleftBitmap, xMarkPixmap);
    } else {
      //	XtSetArg(args[0], XtNleftBitmap, None);
    }
//    XtSetValues(XtNameToWidget(menuBarWidget, "menuOptions.Flash Moves"),
//		args, 1);
}

#if HIGHDRAG
void HighlightDraggingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.highlightDragging = !appData.highlightDragging;
}
#endif

void HighlightLastMoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.highlightLastMove = !appData.highlightLastMove;
}

void HighlightArrowProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.highlightMoveWithArrow = !appData.highlightMoveWithArrow;
}

#if 0
void IcsAlarmProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.icsAlarm = !appData.icsAlarm;
}
#endif

void MoveSoundProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.ringBellAfterMoves = !appData.ringBellAfterMoves;
}

void OneClickProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.oneClick = !appData.oneClick;
}

void PeriodicUpdatesProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    PeriodicUpdatesEvent(!appData.periodicUpdates);
}

void PopupExitMessageProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.popupExitMessage = !appData.popupExitMessage;
}

void PopupMoveErrorsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.popupMoveErrors = !appData.popupMoveErrors;
}

#if 0
void PremoveProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.premove = !appData.premove;
}
#endif

void ShowCoordsProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.showCoords = !appData.showCoords;
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
}
#endif

void SaveOnExitProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    saveSettingsOnExit = !saveSettingsOnExit;
}

void SaveSettingsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    SaveSettings(settingsFileName);
}

void InfoProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    char buf[MSG_SIZ];
    snprintf(buf, sizeof(buf), "xterm -e info --directory %s --directory . -f %s &",
	     INFODIR, INFOFILE);
    system(buf);
}

void ManProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    char buf[MSG_SIZ];
    String name;

    name = "xboard";
    snprintf(buf, sizeof(buf), "xterm -e man %s &", name);
    system(buf);
}

void HintProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    HintEvent();
}

void BookProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    BookEvent();
}

void AboutProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  GtkWidget *about;
  char buf[MSG_SIZ];

  const gchar *authors[] = {
    "Wayne Christopher",
    "Chris Sears",
    "Dan Sears",
    "Tim Mann <tim@tim-mann.org>",
    "John Chanak",
    "Evan Welsh <Evan.Welsh@msdw.com>",
    "Elmar Bartel <bartel@informatik.tu-muenchen.de>",
    "Jochen Wiedmann",
    "Frank McIngvale",
    "Hugh Fisher <Hugh.Fisher@cs.anu.edu.au>",
    "Allessandro Scotti",
    "H.G. Muller <h.g.muller AT hccnet DOT nl>",
    "Arun Persaud <arun@nubati.net>",
    "Eric Mullins <emwine AT earthlink DOT net>",
    "John Cheetham <developer AT johncheetham DOT com>",
    NULL};

  /* create about window */
  about = gtk_about_dialog_new();

  /* fill in some information */
#if ZIPPY
  char *zippy = " (with Zippy code)";
#else
  char *zippy = "";
#endif

  sprintf(buf, "%s%s",  programVersion, zippy);

  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about),buf);

  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about),
                                 "Copyright 1991 Digital Equipment Corporation\n"
                                 "Enhancements Copyright 1992-2011 Free Software Foundation\n"
                                 "Enhancements Copyright 2005 Alessandro Scotti");
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),"http://www.gnu.org/software/xboard/");
  gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about),authors);
  gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about),
					  " Translation project (http://translationproject.org)\n");

  /* show widget, destroy on close */
  gtk_widget_show_all( about );
  gtk_dialog_run(GTK_DIALOG (about));
  gtk_widget_destroy(about);

  return;
}

void DebugProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    appData.debugMode = !appData.debugMode;
}

void NothingProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
    return;
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

    safeStrCpy(lastMsg, message, MSG_SIZ); // [HGM] make available

  /* need to test if messageWidget already exists, since this function
     can also be called during the startup, if for example a Xresource
     is not set up correctly */
  if(messageWidgetGTK)
    {
      gtk_label_set_text(GTK_LABEL(messageWidgetGTK), message);
    }

  return;
}

void DisplayTitle(text)
     char *text;
{
    char title[MSG_SIZ];
    char icon[MSG_SIZ];

    if (text == NULL) text = "";

    if (appData.titleInWindow) {
	/* GTK TODO set label for title in window */
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

    gtk_window_set_title (GTK_WINDOW(mainwindow), title);
    /*  GTK-TODO can we also set the icon name? */
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
    if (appData.popupExitMessage && boardwidgetGTK && gtk_widget_get_realized(GTK_WIDGET(boardwidgetGTK)) ) {
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

void AskQuestion(title, question, replyPrefix, pr)
     char *title, *question, *replyPrefix;
     ProcRef pr;
{
    GtkWidget *askquestion, *label, *inputarea, *input;
    gint result;
    const gchar *reply;

    safeStrCpy(pendingReplyPrefix, replyPrefix, sizeof(pendingReplyPrefix)/sizeof(pendingReplyPrefix[0]) );
    pendingReplyPR = pr;

    askquestion = gtk_dialog_new_with_buttons (title,
					       GTK_WINDOW(mainwindow),
					       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					       GTK_STOCK_EXECUTE, GTK_RESPONSE_ACCEPT,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       NULL);

    /* add a label and a text input area to the dialog */
    inputarea = gtk_dialog_get_content_area (GTK_DIALOG (askquestion));
    label = gtk_label_new (question);
    input = gtk_entry_new_with_buffer (AskQuestionBuffer);

    gtk_container_add (GTK_CONTAINER (inputarea), label);
    gtk_container_add (GTK_CONTAINER (inputarea), input);
    gtk_widget_show_all(askquestion);

    result  = gtk_dialog_run (GTK_DIALOG(askquestion));

    /* check for output */
    if (result ==  GTK_RESPONSE_ACCEPT )
      {
	char buf[MSG_SIZ];
	int err;
	String replyPrefix;

	reply = gtk_entry_get_text (GTK_ENTRY(input));

	safeStrCpy(buf, pendingReplyPrefix, sizeof(buf)/sizeof(buf[0]) );
	if (*buf) strncat(buf, " ", MSG_SIZ - strlen(buf) - 1);
	strncat(buf, reply, MSG_SIZ - strlen(buf) - 1);
	strncat(buf, "\n",  MSG_SIZ - strlen(buf) - 1);
	OutputToProcess(pendingReplyPR, buf, strlen(buf), &err);

	if (err) DisplayFatalError(_("Error writing to chess program"), err, 0);
      }

    gtk_widget_destroy (askquestion);
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
PlayTellSound()
{
  PlaySound(appData.soundTell);
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
RunCommand(char *buf)
{
    system(buf);
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

guint delayedEventTimerTag = 0;
DelayedEventCallback delayedEventCallback = 0;

void
FireDelayedEvent(gpointer data)
{
  /* remove timer */
  g_source_remove(delayedEventTimerTag);
  delayedEventTimerTag = 0;

  /* call function */
  delayedEventCallback();

  return;
}

void
ScheduleDelayedEvent(cb, millisec)
     DelayedEventCallback cb; guint millisec;
{
  if(delayedEventTimerTag && delayedEventCallback == cb)
    // [HGM] alive: replace, rather than add or flush identical event
    g_source_remove(delayedEventTimerTag);
  delayedEventCallback = cb;
  delayedEventTimerTag = g_timeout_add(millisec,(GSourceFunc) FireDelayedEvent, NULL);
  return;
}

DelayedEventCallback
GetDelayedEvent()
{
  if (delayedEventTimerTag) {
    return delayedEventCallback;
  } else {
    return NULL;
  }
}

void
CancelDelayedEvent()
{
  if (delayedEventTimerTag)
    {
      g_source_remove(delayedEventTimerTag);
      delayedEventTimerTag = 0;
    }

  return;
}

guint loadGameTimerTag = 0;

int LoadGameTimerRunning()
{
    return loadGameTimerTag != 0;
}

int StopLoadGameTimer()
{
  if (loadGameTimerTag != 0) {
    g_source_remove(loadGameTimerTag);
    loadGameTimerTag = 0;
    return TRUE;
  } else {
    return FALSE;
  }
}

void
LoadGameTimerCallback(gpointer data)
{
  /* remove timer */
  g_source_remove(loadGameTimerTag);
  loadGameTimerTag = 0;

  AutoPlayGameLoop();
  return;
}

void
StartLoadGameTimer(millisec)
     long millisec;
{
  loadGameTimerTag =
    g_timeout_add( millisec, (GSourceFunc) LoadGameTimerCallback, NULL);
  return;
}

guint analysisClockTag = 0;

gboolean
AnalysisClockCallback(gpointer data)
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile
         || appData.icsEngineAnalyze)
      {
        AnalysisPeriodicEvent(0);
        return 1; /* keep on going */
      }
    return 0; /* stop timer */
}

void
StartAnalysisClock()
{
  analysisClockTag =
    g_timeout_add( 2000,(GSourceFunc) AnalysisClockCallback, NULL);
  return;
}

guint clockTimerTag = 0;

int ClockTimerRunning()
{
    return clockTimerTag != 0;
}

int StopClockTimer()
{
  if (clockTimerTag != 0)
    {
      g_source_remove(clockTimerTag);
      clockTimerTag = 0;
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

void
ClockTimerCallback(gpointer data)
{
  /* remove timer */
  g_source_remove(clockTimerTag);
  clockTimerTag = 0;

  DecrementClocks();
  return;
}

void
StartClockTimer(millisec)
     long millisec;
{
  clockTimerTag = g_timeout_add(millisec,(GSourceFunc) ClockTimerCallback,NULL);
  return;
}

void
DisplayTimerLabelGTK(w, color, timer, highlight)
     GtkWidget *w;
     char *color;
     long timer;
     int highlight;
{
    char *markup;
    char bgcolor[10];
    char fgcolor[10];

    if (highlight) {
	strcpy(bgcolor, "black");
        strcpy(fgcolor, "white");
    } else {
        strcpy(bgcolor, "white");
        strcpy(fgcolor, "black");
    }

    if (appData.clockMode) {
        markup = g_markup_printf_escaped("<span size=\"xx-large\" weight=\"heavy\" background=\"%s\" foreground=\"%s\">%s: %s</span>", bgcolor, fgcolor, color, TimeString(timer));
    } else {
        markup = g_markup_printf_escaped("<span size=\"xx-large\" weight=\"heavy\" background=\"%s\" foreground=\"%s\">%s  </span>", bgcolor, fgcolor, color);
    }
    gtk_label_set_markup(GTK_LABEL(w), markup);
    g_free(markup);
}

void
DisplayWhiteClock(timeRemaining, highlight)
     long timeRemaining;
     int highlight;
{
    if(appData.noGUI) return;
    DisplayTimerLabelGTK(whiteTimerWidgetGTK, _("White"), timeRemaining, highlight);
    if (highlight && mainwindowIcon == BlackIcon)
      {
	mainwindowIcon = WhiteIcon;
	gtk_window_set_icon(GTK_WINDOW(mainwindow),mainwindowIcon);
      }
}

void
DisplayBlackClock(timeRemaining, highlight)
     long timeRemaining;
     int highlight;
{
    if(appData.noGUI) return;
    DisplayTimerLabelGTK(blackTimerWidgetGTK, _("Black"), timeRemaining, highlight);

    if (highlight && mainwindowIcon == WhiteIcon)
      {
        mainwindowIcon = BlackIcon;
        gtk_window_set_icon(GTK_WINDOW(mainwindow),mainwindowIcon);
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
    guint sid;
    char buf[INPUT_SOURCE_BUF_SIZE];
    VOIDSTAR closure;
} InputSource;

void
DoInputCallback(io, cond, data)
     GIOChannel  *io;
     GIOCondition cond;
     gpointer    *data;
{
  /* read input from one of the input source (for example a chess program, ICS, etc).
   * and call a function that will handle the input
   */

    int count;
    int error;
    char *p, *q;

    /* All information (callback function, file descriptor, etc) is
     * saved in an InputSource structure
     */
    InputSource *is = (InputSource *) data;

    if (is->lineByLine) {
	count = read(is->fd, is->unused,
		     INPUT_SOURCE_BUF_SIZE - (is->unused - is->buf));
	if (count <= 0) {
	    (is->func)(is, is->closure, is->buf, count, count ? errno : 0);
	    return;
	}
	is->unused += count;
	p = is->buf;
	/* break input into lines and call the callback function on each
	 * line
	 */
	while (p < is->unused) {
	    q = memchr(p, '\n', is->unused - p);
	    if (q == NULL) break;
	    q++;
	    (is->func)(is, is->closure, p, q - p, 0);
	    p = q;
	}
	/* remember not yet used part of the buffer */
	q = is->buf;
	while (p < is->unused) {
	    *q++ = *p++;
	}
	is->unused = q;
    } else {
      /* read maximum length of input buffer and send the whole buffer
       * to the callback function
       */
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
    GIOChannel *channel;
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
    if (lineByLine)
      is->unused = is->buf;
    else
      is->unused = NULL;

   /* GTK-TODO: will this work on windows?*/

    channel = g_io_channel_unix_new(is->fd);
    g_io_channel_set_close_on_unref (channel, TRUE);
    is->sid = g_io_add_watch(channel, G_IO_IN,(GIOFunc) DoInputCallback, is);

    is->closure = closure;
    return (InputSourceRef) is;
}

void
RemoveInputSource(isr)
     InputSourceRef isr;
{
    InputSource *is = (InputSource *) isr;

    if (is->sid == 0) return;
    g_source_remove(is->sid);
    is->sid = 0;
    return;
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

static int xpmDone=0;

static void
CreateAnimVars ()
{
  XWindowAttributes info;

  if (xpmDone && gameInfo.variant == oldVariant) return;
  if(xpmDone) oldVariant = gameInfo.variant; // first time pieces might not be created yet
  XGetWindowAttributes(xDisplay, xBoardWindow, &info);

  InitAnimState(&game, &info);
  InitAnimState(&player, &info);

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
  if (time > 0)
    usleep(time * 1000);
}

#endif

void
DoSleep(int n)
{
    FrameDelay(n);
}

/*	Convert board position to corner of screen rect and color	*/

static void
ScreenSquare(column, row, pt, color)
     int column;
     int row;
     GdkPoint *pt;
     int *color;
{
  if (flipView) {
    pt->x = lineGapGTK + ((BOARD_WIDTH-1)-column) * (squareSizeGTK + lineGapGTK);
    pt->y = lineGapGTK + row * (squareSizeGTK + lineGapGTK);
  } else {
    pt->x = lineGapGTK + column * (squareSizeGTK + lineGapGTK);
    pt->y = lineGapGTK + ((BOARD_HEIGHT-1)-row) * (squareSizeGTK + lineGapGTK);
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
     GdkPoint *old; GdkPoint *new;
     int size; GdkRectangle *area; GdkPoint *pt;
{
  if (    abs(old->x - new->x) > size
       || abs(old->y - new->y) > size )
       return False;
  else
    {
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
     GdkPoint *old; GdkPoint *new; int size;
     GdkRectangle update[]; int *nUpdates;
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
     GdkPoint *start; GdkPoint *mid;
     GdkPoint *finish; int factor;
     GdkPoint frames[]; int *nFrames;
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

  return;
}

GdkPixbuf *getPixbuf(int piece) {
    GdkPixbuf *pb=NULL;

/*
    WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen,
    WhiteFerz, WhiteAlfil, WhiteAngel, WhiteMarshall, WhiteWazir, WhiteMan,
    WhiteCannon, WhiteNightrider, WhiteCardinal, WhiteDragon, WhiteGrasshopper,
    WhiteSilver, WhiteFalcon, WhiteLance, WhiteCobra, WhiteUnicorn, WhiteKing,
    BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen,
    BlackFerz, BlackAlfil, BlackAngel, BlackMarshall, BlackWazir, BlackMan,
    BlackCannon, BlackNightrider, BlackCardinal, BlackDragon, BlackGrasshopper,
    BlackSilver, BlackFalcon, BlackLance, BlackCobra, BlackUnicorn, BlackKing,
    EmptySquare,
*/

    switch (piece) {
      case WhitePawn:
        pb = SVGscWhitePawn;
        break;
      case WhiteKnight:
        pb = SVGscWhiteKnight;
        break;
      case WhiteBishop:
        pb = SVGscWhiteBishop;
        break;
      case WhiteRook:
        pb = SVGscWhiteRook;
        break;
      case WhiteQueen:
        pb = SVGscWhiteQueen;
        break;
      case WhiteAngel:
        pb = SVGscWhiteCardinal;
        break;
      case WhiteMarshall:
        pb = SVGscWhiteMarshall;
        break;
      case WhiteKing:
        pb = SVGscWhiteKing;
        break;

      case BlackPawn:
        pb = SVGscBlackPawn;
        break;
      case BlackKnight:
        pb = SVGscBlackKnight;
        break;
      case BlackBishop:
        pb = SVGscBlackBishop;
        break;
      case BlackRook:
        pb = SVGscBlackRook;
        break;
      case BlackQueen:
        pb = SVGscBlackQueen;
        break;
      case BlackAngel:
        pb = SVGscBlackCardinal;
        break;
      case BlackMarshall:
        pb = SVGscBlackMarshall;
        break;
      case BlackKing:
        pb = SVGscBlackKing;
        break;

      default:
        if ((int)piece < (int) BlackPawn) // white piece
            pb = SVGscWhiteKing;
        else
            pb = SVGscBlackKing;
        break;
    }
    return pb;
}

static void
OverlayPiece(piece, position, dest)
     ChessSquare piece;
     GdkPoint *position;
     Drawable dest;
{
    GdkPixbuf *pb=NULL;

    pb = getPixbuf(piece);

  /* draw piece */
  gdk_draw_pixbuf(GDK_WINDOW(boardwidgetGTK->window),NULL,
                  GDK_PIXBUF(pb),0,0,
		  position->x,position->y,-1,-1,
		  GDK_RGB_DITHER_NORMAL, 0, 0);
  return;
}

/* Animate the movement of a single piece */

static void
BeginAnimation(anim, piece, startColor, start)
     AnimState *anim;
     ChessSquare piece;
     int startColor;
     GdkPoint * start;
{
  Pixmap mask;

  if(appData.upsideDown && flipView) piece += piece < BlackPawn ? BlackPawn : -BlackPawn;
  /* not converted to GTK - causes the clicked on piece to flicker */
  //BlankSquareGTK(start->x, start->y, startColor, EmptySquare, anim->saveBuf, 0);
  anim->prevFrame = *start;

  /* The piece will be drawn using its own bitmap as a matte	*/
  //  SelectGCMask(piece, &anim->pieceGC, &anim->outlineGC, &mask);
  XSetClipMask(xDisplay, anim->pieceGC, mask);
}

static void
AnimationFrame(anim, frame, piece)
     AnimState *anim;
     GdkPoint *frame;
     ChessSquare piece;
{
  int x,y;
  int xb,yb, xoffset,yoffset,sx,sy;


  /* TODO: check lineGap, seems to be not correct   */

  /* clear pic from last frame */

  /* get coordinates */
  if(anim->prevFrame.x<0)
    anim->prevFrame.x += squareSizeGTK;
  if(anim->prevFrame.y<0)
    anim->prevFrame.y += squareSizeGTK;

  x = EventToSquare(anim->prevFrame.x, BOARD_WIDTH);
  y = EventToSquare(anim->prevFrame.y, BOARD_HEIGHT);

  /* for the pieces we need to include flipview */
  BoardSquare(anim->prevFrame.x,anim->prevFrame.y,&xb,&yb);
  BoardSquare(anim->startSquare.x,anim->startSquare.y,&sx,&sy);

  /* override the 4 squares that can be affected by a moving piece */
  if(x>=0 && y>=0 )
    {
      DrawGrid(x,y,2,2);

      if (flipView)
	{
	  xoffset=-1;
	  yoffset=+1;
	}
      else
	{
	  xoffset=+1;
	  yoffset=-1;
	}

      /* make sure start square stays empty */
      if(! (xb==sx && yb==sy) )
	DrawSquareGTK(yb  ,xb,    boards[currentMove][yb  ][xb  ], 0);
      else
	DrawSquareGTK(yb  ,xb,    EmptySquare, 0);

      if(! (xb==sx && yb+yoffset==sy) )
	DrawSquareGTK(yb+yoffset,xb,    boards[currentMove][yb+yoffset][xb  ], 0);
      else
	DrawSquareGTK(yb+yoffset  ,xb,    EmptySquare, 0);

      if(! (xb+xoffset==sx && yb==sy) )
	DrawSquareGTK(yb  ,xb+xoffset,  boards[currentMove][yb  ][xb+xoffset], 0);
      else
	DrawSquareGTK(yb  ,xb+xoffset,  EmptySquare, 0);

      if(! (xb+xoffset==sx && yb+yoffset==sy) )
	DrawSquareGTK(yb+yoffset,xb+xoffset,  boards[currentMove][yb+yoffset][xb+xoffset], 0);
      else
	DrawSquareGTK(yb+yoffset,xb+xoffset,  EmptySquare, 0);

    }

  /* Draw moving piece  */
  OverlayPiece(piece, frame, xBoardWindow);

  /* remember this position */
  anim->prevFrame = *frame;
  return;
}

static void
EndAnimation (anim, finish)
     AnimState *anim;
     GdkPoint *finish;
{
  GdkRectangle updates[4];
  GdkRectangle overlap;
  GdkPoint     pt;
  int	     count, i, j;

  /* kludge to force redraw of all squares on the board */
  for (i=0; i < BOARD_WIDTH ; i++) {
      for (j=0; j < BOARD_HEIGHT ; j++) {
          damageGTK[0][j][i] = True;
      }
  }

  /* The main code will redraw the final square, so we
     only need to erase the bits that don't overlap.	*/

/*
  if (Intersect(&anim->prevFrame, finish, squareSize, &overlap, &pt)) {
    CalcUpdateRects(&anim->prevFrame, finish, squareSize, updates, &count);
    for (i = 0; i < count; i++)
  //    XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
  //	updates[i].x - anim->prevFrame.x,
  //		updates[i].y - anim->prevFrame.y,
  //		updates[i].width, updates[i].height,
  //		updates[i].x, updates[i].y);
  } else {
//    XCopyArea(xDisplay, anim->saveBuf, xBoardWindow, anim->blitGC,
//		0, 0, squareSize, squareSize,
//		anim->prevFrame.x, anim->prevFrame.y);
  }
*/


  return;
}

static void
FrameSequence(anim, piece, startColor, start, finish, frames, nFrames)
     AnimState *anim;
     ChessSquare piece; int startColor;
     GdkPoint *start; GdkPoint *finish;
     GdkPoint frames[]; int nFrames;
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
	int rad = squareSize * 9 * i/(20*kFactor - 5);

	guint32 col, tmp;
	gdouble r, g, b = 0.0;

	cairo_t *cr;
	int i;

	/* get a cairo_t */
	cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

	/* highline */
	sscanf(appData.highlightSquareColor, "#%x", &col);

	tmp = (col & 0x00ff0000) >> 16;
	r = (gdouble)tmp;
	r = r / 255;

	tmp = (col & 0x0000ff00) >> 8;
	g = (gdouble)tmp;
	g = g / 255;

	tmp = (col & 0x000000ff);
	b = (gdouble)tmp;
	b = b / 255;

	cairo_set_source_rgba (cr, r, g, b, 1.0);

	cairo_arc(cr, x + squareSize/2 - rad, y+squareSize/2 - rad, 2*rad, 0.0, 2*M_PI);

	cairo_stroke_preserve(cr);
	cairo_fill(cr);

	/* free memory */
	cairo_destroy (cr);

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
  GdkPoint      start, finish, mid;
  GdkPoint      frames[kFactor * 2 + 1];
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
  hop = (piece == WhiteKnight || piece == BlackKnight);
#endif

  if (appData.debugMode) {
      fprintf(debugFP, hop ? _("AnimateMove: piece %d hops from %d,%d to %d,%d \n") :
                             _("AnimateMove: piece %d slides from %d,%d to %d,%d \n"),
             piece, fromX, fromY, toX, toY);  }

  ScreenSquare(fromX, fromY, &start, &startColor);
  ScreenSquare(toX, toY, &finish, &endColor);

  if (hop) {
    /* Knight: make diagonal movement then straight */
    if (abs(toY - fromY) < abs(toX - fromX)) {
       mid.x = start.x + (finish.x - start.x) / 2;
       mid.y = finish.y;
     } else {
       mid.x = finish.x;
       mid.y = start.y + (finish.y - start.y) / 2;
     }
  } else {
    mid.x = start.x + (finish.x - start.x) / 2;
    mid.y = start.y + (finish.y - start.y) / 2;
  }

  /* These lines commented out since they cause problems with gtk board */
  /* when animating computers move */

  /* Don't use as many frames for very short moves */
//if (abs(toY - fromY) + abs(toX - fromX) <= 2)
//  Tween(&start, &mid, &finish, kFactor - 1, frames, &nFrames);
//else
    Tween(&start, &mid, &finish, kFactor, frames, &nFrames);
  FrameSequence(&game, piece, startColor, &start, &finish, frames, nFrames);

  /* Be sure end square is redrawn */
  damageGTK[0][toY][toX] = True;

  return;
}

void
DragPieceBegin(x, y, instantly)
     int x; int y; Boolean instantly;
{
    int	 boardX, boardY, color;
    GdkPoint corner;

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
    player.mouseDelta.x = squareSizeGTK/2;
    player.mouseDelta.y = squareSizeGTK/2;
    /* Initialise animation */
    player.dragPiece = PieceForSquare(boardX, boardY);
    /* Sanity check */
    if (player.dragPiece >= 0 && player.dragPiece < EmptySquare) {
	player.dragActive = True;
	BeginAnimation(&player, player.dragPiece, color, &corner);
	/* Mark this square as needing to be redrawn. Note that
	   we don't remove the piece though, since logically (ie
	   as seen by opponent) the move hasn't been made yet. */
           //if(boardX == BOARD_RGHT+1 && PieceForSquare(boardX-1, boardY) > 1 ||
           //   boardX == BOARD_LEFT-2 && PieceForSquare(boardX+1, boardY) > 1)
           //XCopyArea(xDisplay, xBoardWindow, player.saveBuf, player.blitGC,
	   //          corner.x, corner.y, squareSize, squareSize,
	   //          0, 0); // [HGM] zh: unstack in stead of grab
           //if(gatingPiece != EmptySquare) {
           //    /* Kludge alert: When gating we want the introduced
           //       piece to appear on the from square. To generate an
           //       image of it, we draw it on the board, copy the image,
           //       and draw the original piece again. */
           //    ChessSquare piece = boards[currentMove][boardY][boardX];
           //    DrawSquare(boardY, boardX, gatingPiece, 0);
           //    XCopyArea(xDisplay, xBoardWindow, player.saveBuf, player.blitGC,
	   //          corner.x, corner.y, squareSize, squareSize, 0, 0);
           //    DrawSquare(boardY, boardX, piece, 0);
           //}
	damageGTK[0][boardY][boardX] = True;
    } else {
	player.dragActive = False;
    }
}

void
ChangeDragPiece(ChessSquare piece)
{
  player.dragPiece = piece;
  /* The piece will be drawn using its own bitmap as a matte	*/
//  SelectGCMask(piece, &player.pieceGC, &player.outlineGC, &mask);
//  XSetClipMask(xDisplay, player.pieceGC, mask);
}

static void
DragPieceMove(x, y)
     int x; int y;
{
    GdkPoint corner;

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
    GdkPoint corner;

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
    damageGTK[0][boardY][boardX] = True;

    /* This prevents weird things happening with fast successive
       clicks which on my Sun at least can cause motion events
       without corresponding press/release. */
    player.dragActive = False;

    return;
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
  BlankSquareGTK(player.startSquare.x, player.startSquare.y,
		player.startColor, EmptySquare, xBoardWindow, 1);
  AnimationFrame(&player, &player.prevFrame, player.dragPiece);
  damageGTK[0][player.startBoardY][player.startBoardX] = TRUE;
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

void SquareToPosGTK(int rank, int file, int *x, int *y)
{
    if (flipView) {
	*x = lineGapGTK + ((BOARD_WIDTH-1)-file) * (squareSizeGTK + lineGapGTK);
	*y = lineGapGTK + rank * (squareSizeGTK + lineGapGTK);
    } else {
	*x = lineGapGTK + file * (squareSizeGTK + lineGapGTK);
	*y = lineGapGTK + ((BOARD_HEIGHT-1)-rank) * (squareSizeGTK + lineGapGTK);
    }
}

/* Draw and fill arrow for GTK board */
void FillPolygon(GdkPoint *arrow)
{
    cairo_t *cr;
    int i;

    /* get a cairo_t */
    cr = gdk_cairo_create (GDK_WINDOW(boardwidgetGTK->window));

    cairo_move_to (cr, arrow[0].x, arrow[0].y);
    for (i=1;i<7;i++) {
        cairo_line_to(cr, arrow[i].x, arrow[i].y);
    }
    cairo_line_to(cr, arrow[0].x, arrow[0].y);

    cairo_set_line_width(cr, 2);
    cairo_set_source_rgba(cr, 1, 1, 0, 1.0);
    cairo_fill(cr);

    /* free memory */
    cairo_destroy (cr);
}

/* Draw an arrow between two points using current settings */
void DrawArrowBetweenPointsGTK( int s_x, int s_y, int d_x, int d_y )
{
    GdkPoint arrow[7];
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
    /* Draw and fill arrow for GTK board */
    FillPolygon(arrow);
}

/* [AS] Draw an arrow between two squares */
void DrawArrowBetweenSquaresGTK( int s_col, int s_row, int d_col, int d_row )
{
    int s_x, s_y, d_x, d_y, hor, vert, i;

    if( s_col == d_col && s_row == d_row ) {
        return;
    }

    /* Get source and destination points */
    SquareToPosGTK( s_row, s_col, &s_x, &s_y);
    SquareToPosGTK( d_row, d_col, &d_x, &d_y);

    if( d_y > s_y ) {
        d_y += squareSizeGTK / 2 - squareSizeGTK / 4; // [HGM] round towards same centers on all sides!
    }
    else if( d_y < s_y ) {
        d_y += squareSizeGTK / 2 + squareSizeGTK / 4;
    }
    else {
        d_y += squareSizeGTK / 2;
    }

    if( d_x > s_x ) {
        d_x += squareSizeGTK / 2 - squareSizeGTK / 4;
    }
    else if( d_x < s_x ) {
        d_x += squareSizeGTK / 2 + squareSizeGTK / 4;
    }
    else {
        d_x += squareSizeGTK / 2;
    }

    s_x += squareSizeGTK / 2;
    s_y += squareSizeGTK / 2;

    /* Adjust width */
    A_WIDTH = squareSizeGTK / 14.; //[HGM] make float

    DrawArrowBetweenPointsGTK( s_x, s_y, d_x, d_y );

    hor = 64*s_col + 32; vert = 64*s_row + 32;
    for(i=0; i<= 64; i++) {
            damageGTK[0][vert+6>>6][hor+6>>6] = True;
            damageGTK[0][vert-6>>6][hor+6>>6] = True;
            damageGTK[0][vert+6>>6][hor-6>>6] = True;
            damageGTK[0][vert-6>>6][hor-6>>6] = True;
            hor += d_col - s_col; vert += d_row - s_row;
    }
}

Boolean IsDrawArrowEnabled()
{
    return appData.highlightMoveWithArrow && squareSize >= 32;
}

void DrawArrowHighlightGTK(int fromX, int fromY, int toX,int toY)
{
    if( IsDrawArrowEnabled() && fromX >= 0 && fromY >= 0 && toX >= 0 && toY >= 0)
        DrawArrowBetweenSquaresGTK(fromX, fromY, toX, toY);
}

void UpdateLogos(int displ)
{
    return; // no logos in XBoard yet
}

