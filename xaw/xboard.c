/*
 * xboard.c -- X front end for XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

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
#include "menus.h"
#include "board.h"
#include "dialogs.h"
#include "engineoutput.h"
#include "usystem.h"
#include "gettext.h"
#include "draw.h"

#define SLASH '/'

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
RETSIGTYPE CmailSigHandler P((int sig));
RETSIGTYPE IntSigHandler P((int sig));
RETSIGTYPE TermSizeSigHandler P((int sig));
Widget CreateMenuBar P((Menu *mb, int boardWidth));
#if ENABLE_NLS
char *InsertPxlSize P((char *pattern, int targetPxlSize));
XFontSet CreateFontSet P((char *base_fnt_lst));
#else
char *FindFont P((char *pattern, int targetPxlSize));
#endif
void ReadBitmap P((Pixmap *pm, String name, unsigned char bits[],
		   u_int wreq, u_int hreq));
void EventProc P((Widget widget, caddr_t unused, XEvent *event));
void DelayedDrag P((void));
static void MoveTypeInProc P((Widget widget, caddr_t unused, XEvent *event));
void HandlePV P((Widget w, XEvent * event,
		     String * params, Cardinal * nParams));
void DrawPositionProc P((Widget w, XEvent *event,
		     String *prms, Cardinal *nprms));
void CommentClick P((Widget w, XEvent * event,
		   String * params, Cardinal * nParams));
void ICSInputBoxPopUp P((void));
void SelectCommand P((Widget w, XtPointer client_data, XtPointer call_data));
void KeyBindingProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void QuitWrapper P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
static void EnterKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
static void UpKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
static void DownKeyProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TempBackwardProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void TempForwardProc P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
Boolean TempBackwardActive = False;
void ManInner P((Widget w, XEvent *event, String *prms, Cardinal *nprms));
void DisplayMove P((int moveNumber));
void update_ics_width P(());
int CopyMemoProc P(());
static int FindLogo P((char *place, char *name, char *buf));

/*
* XBoard depends on Xt R4 or higher
*/
int xtVersion = XtSpecificationRelease;

int xScreen;
Display *xDisplay;
Window xBoardWindow;
Pixel lowTimeWarningColor, dialogColor, buttonColor; // used in widgets
Pixmap iconPixmap, wIconPixmap, bIconPixmap, xMarkPixmap;
Widget shellWidget, formWidget, boardWidget, titleWidget, dropMenu, menuBarWidget;
Option *optList; // contains all widgets of main window
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

char installDir[] = "."; // [HGM] UCI: needed for UCI; probably needs run-time initializtion

Position commentX = -1, commentY = -1;
Dimension commentW, commentH;
typedef unsigned int BoardSize;
BoardSize boardSize;
Boolean chessProgram;

int  minX, minY; // [HGM] placement: volatile limits on upper-left corner
int smallLayout = 0, tinyLayout = 0,
  marginW, marginH, // [HGM] for run-time resizing
  fromX = -1, fromY = -1, toX, toY, commentUp = False,
  errorExitStatus = -1, defaultLineGap;
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
WindowPlacement wpDualBoard;


/* This magic number is the number of intermediate frames used
   in each half of the animation. For short moves it's reduced
   by 1. The total number of frames will be factor * 2 + 1.  */
#define kFactor	   4

SizeDefaults sizeDefaults[] = SIZE_DEFAULTS;

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
    { "HandlePV", HandlePV },
    { "SelectPV", SelectPV },
    { "StopPV", StopPV },
    { "MenuItem", KeyBindingProc }, // [HGM] generic handler for key bindings
    { "QuitProc", QuitWrapper },
    { "ManProc", ManInner },
    { "TempBackwardProc", TempBackwardProc },
    { "TempForwardProc", TempForwardProc },
    { "CommentClick", (XtActionProc) CommentClick },
    { "GenericPopDown", (XtActionProc) GenericPopDown },
    { "ErrorPopDown", (XtActionProc) ErrorPopDown },
    { "CopyMemoProc", (XtActionProc) CopyMemoProc },
    { "SelectMove", (XtActionProc) SelectMoveX },
    { "LoadSelectedProc", LoadSelectedProc },
    { "SetFilterProc", SetFilterProc },
    { "TypeInProc", TypeInProc },
    { "EnterKeyProc", EnterKeyProc },
    { "UpKeyProc", UpKeyProc },
    { "DownKeyProc", DownKeyProc },
    { "WheelProc", WheelProc },
    { "TabProc", TabProc },
};

char globalTranslations[] =
  ":Meta<Key>Next: MenuItem(LoadNextGameProc) \n \
   :Meta<Key>Prior: MenuItem(LoadPrevGameProc) \n \
   :Ctrl<Key>Down: LoadSelectedProc(3) \n \
   :Ctrl<Key>Up: LoadSelectedProc(-3) \n \
   :Shift<Key>Next: MenuItem(LoadNextPositionProc) \n \
   :Shift<Key>Prior: MenuItem(LoadPrevPositionProc) \n \
   :<Key>Pause: MenuItem(Mode.Pause) \n \
   :Ctrl<Key>d: MenuItem(DebugProc) \n \
   :Meta Ctrl<Key>F12: MenuItem(DebugProc) \n \
   :<Key>Left: MenuItem(Edit.Backward) \n \
   :<Key>Right: MenuItem(Edit.Forward) \n \
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
   :<KeyDown>Return: TempBackwardProc() \n \
   :<KeyUp>Return: TempForwardProc() \n";

char ICSInputTranslations[] =
    "<Key>Up: UpKeyProc() \n "
    "<Key>Down: DownKeyProc() \n "
    "<Key>Return: EnterKeyProc() \n";

// [HGM] vari: another hideous kludge: call extend-end first so we can be sure select-start works,
//             as the widget is destroyed before the up-click can call extend-end
char commentTranslations[] = "<Btn3Down>: extend-end(PRIMARY) select-start() CommentClick() \n";

String xboardResources[] = {
    "*Error*translations: #override\\n <Key>Return: ErrorPopDown()",
    NULL
  };


/* Max possible square size */
#define MAXSQSIZE 256

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
    if(strstr(name, "-*-") &&        // only pay attention to things that look like X-fonts
       size >= 0 && size<MAX_SIZE) { // for now, fixed limit
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
  if(colorVariable[n] && *name == '#') *(char**)colorVariable[n] = strdup(name);
}

char *
Col2Text (int n)
{
    return *(char**)colorVariable[n];
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

int frameX, frameY;

void
GetActualPlacement (Widget wg, WindowPlacement *wp)
{
  XWindowAttributes winAt;
  Window win, dummy;
  int rx, ry;

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
GetPlacement (DialogClass dlg, WindowPlacement *wp)
{ // wrapper to shield back-end from widget type
  if(shellUp[dlg]) GetActualPlacement(shells[dlg], wp);
}

void
GetWindowCoords ()
{ // wrapper to shield use of window handles from back-end (make addressible by number?)
  // In XBoard this will have to wait until awareness of window parameters is implemented
  GetActualPlacement(shellWidget, &wpMain);
  if(shellUp[EngOutDlg]) GetActualPlacement(shells[EngOutDlg], &wpEngineOutput);
  if(shellUp[HistoryDlg]) GetActualPlacement(shells[HistoryDlg], &wpMoveHistory);
  if(shellUp[EvalGraphDlg]) GetActualPlacement(shells[EvalGraphDlg], &wpEvalGraph);
  if(shellUp[GameListDlg]) GetActualPlacement(shells[GameListDlg], &wpGameList);
  if(shellUp[CommentDlg]) GetActualPlacement(shells[CommentDlg], &wpComment);
  if(shellUp[TagsDlg]) GetActualPlacement(shells[TagsDlg], &wpTags);
}

void
PrintCommPortSettings (FILE *f, char *name)
{ // This option does not exist in XBoard
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

void
ResizeBoardWindow (int w, int h, int inhibit)
{
    w += marginW + 1; // [HGM] not sure why the +1 is (sometimes) needed...
    h += marginH;
    shellArgs[0].value = w;
    shellArgs[1].value = h;
    shellArgs[4].value = shellArgs[2].value = w;
    shellArgs[5].value = shellArgs[3].value = h;
    XtSetValues(shellWidget, &shellArgs[0], inhibit ? 6 : 2);

    XSync(xDisplay, False);
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

int
MakeColors ()
{   // [HGM] taken out of main(), so it can be called from BoardOptions dialog
    int forceMono = False;

    if (appData.lowTimeWarning)
	forceMono |= MakeOneColor(appData.lowTimeWarningColor, &lowTimeWarningColor);
    if(appData.dialogColor[0]) MakeOneColor(appData.dialogColor, &dialogColor);
    if(appData.buttonColor[0]) MakeOneColor(appData.buttonColor, &buttonColor);

    return forceMono;
}

void
InitializeFonts (int clockFontPxlSize, int coordFontPxlSize, int fontPxlSize)
{   // detervtomine what fonts to use, and create them
    XrmValue vTo;
    XrmDatabase xdb;

    if(!fontIsSet[CLOCK_FONT] && fontValid[CLOCK_FONT][squareSize])
	appData.clockFont = fontTable[CLOCK_FONT][squareSize];
    if(!fontIsSet[MESSAGE_FONT] && fontValid[MESSAGE_FONT][squareSize])
	appData.font = fontTable[MESSAGE_FONT][squareSize];
    if(!fontIsSet[COORD_FONT] && fontValid[COORD_FONT][squareSize])
	appData.coordFont = fontTable[COORD_FONT][squareSize];

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
    // textHeight in !NLS mode!
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
}

char *
PrintArg (ArgType t)
{
  char *p="";
  switch(t) {
    case ArgZ:
    case ArgInt:      p = " N"; break;
    case ArgString:   p = " STR"; break;
    case ArgBoolean:  p = " TF"; break;
    case ArgSettingsFilename:
    case ArgBackupSettingsFile:
    case ArgFilename: p = " FILE"; break;
    case ArgX:        p = " Nx"; break;
    case ArgY:        p = " Ny"; break;
    case ArgAttribs:  p = " TEXTCOL"; break;
    case ArgColor:    p = " COL"; break;
    case ArgFont:     p = " FONT"; break;
    case ArgBoardSize: p = " SIZE"; break;
    case ArgFloat: p = " FLOAT"; break;
    case ArgTrue:
    case ArgFalse:
    case ArgTwo:
    case ArgNone:
    case ArgCommSettings:
    case ArgMaster:
    case ArgInstall:
      break;
  }
  return p;
}

char *
GenerateGlobalTranslationTable (void)
{
  /* go through all menu items and extract the keyboard shortcuts, so that X11 can load them */
  char *output[2];

  int i,j,n=0;
  MenuItem *mi;

  output[0] = strdup(""); // build keystrokes with and wo mod keys separately
  output[1] = strdup(""); // so the more specific can preceed the other

  /* loop over all menu entries */
  for( i=0; menuBar[i-n].mi || !n++; i++)
    {
      mi = menuBar[i+n].mi; // kludge to access 'noMenu' behind sentinel
      for(j=0; mi[j].proc; j++)
	{
	  if (mi[j].accel)
	    {
	      int ctrl  = 0;
	      int shift = 0;
	      int alt   = 0;

	      char *key,*test, *mods;

	      /* check for Ctrl/Alt */
	      if( strstr(mi[j].accel, "<Ctrl>")  ) ctrl  = 1;
	      if( strstr(mi[j].accel, "<Shift>") ) shift = 1;
	      if( strstr(mi[j].accel, "<Alt>")   ) alt   = 1;

	      /* remove all <...> */
	      test = strrchr(mi[j].accel, '>');
	      if ( test==NULL )
		key = strdup(mi[j].accel);
	      else
		key = strdup(++test); // remove ">"

	      /* instead of shift X11 uses the uppercase letter directly*/
	      if (shift && strlen(key)==1 )
		{
		  *key  = toupper(*key);
		  shift = 0;
		}

	      /* handle some special cases which have different names in X11 */
	      if ( strncmp(key, "Page_Down", 9) == 0 )
		{
		  free(key);
		  key=strdup("Next");
		}
	      else if ( strncmp(key, "Page_Up", 7) == 0 )
		{
		  free(key);
		  key=strdup("Prior");
		};

	      /* create string of mods */
	      if (ctrl)
		mods = strdup("Ctrl ");
	      else
		mods = strdup("");

	      if(alt)
		{
		  mods = realloc(mods, strlen(mods) + strlen("Meta ")+1);
		  strncat(mods, "Meta ", 5);
		};

	      if(shift)
		{
		  mods = realloc(mods, strlen(mods) + strlen("Shift ")+1);
		  strncat(mods, "Shift ", 6);
		};

	      // remove trailing space
	      if( isspace(mods[strlen(mods)-1]) )
		mods[strlen(mods)-1]='\0';

	      /* get the name for the callback, we can use MenuItem() here that will call KeyBindingProc */
	      char *name = malloc(MSG_SIZ);
	      if(n) snprintf(name, MSG_SIZ, "%s", mi[j].ref);
	      else  snprintf(name, MSG_SIZ, "%s.%s", menuBar[i].ref, mi[j].ref);

	      char *buffer = malloc(MSG_SIZ);
	      snprintf(buffer, MSG_SIZ, ":%s<Key>%s: MenuItem(%s) \n ", mods, key, name);

	      /* add string to the output */
	      output[shift|alt|ctrl] = realloc(output[shift|alt|ctrl], strlen(output[shift|alt|ctrl]) + strlen(buffer)+1);
	      strncat(output[shift|alt|ctrl], buffer, strlen(buffer));

	      /* clean up */
	      free(key);
	      free(buffer);
	      free(name);
	      free(mods);
	    }
	}
    }
  output[1] = realloc(output[1], strlen(output[1]) + strlen(output[0])+1);
  strncat(output[1], output[0], strlen(output[0]));
  free(output[0]);
  return output[1];
}


void
PrintOptions ()
{
  char buf[MSG_SIZ];
  int len=0;
  ArgDescriptor *q, *p = argDescriptors+5;
  printf("\nXBoard accepts the following options:\n"
         "(N = integer, TF = true or false, STR = text string, FILE = filename,\n"
         " Nx, Ny = relative coordinates, COL = color, FONT = X-font spec,\n"
         " SIZE = board-size spec(s)\n"
         " Within parentheses are short forms, or options to set to true or false.\n"
         " Persistent options (saved in the settings file) are marked with *)\n\n");
  while(p->argName) {
    if(p->argType == ArgCommSettings) { p++; continue; } // XBoard has no comm port
    snprintf(buf+len, MSG_SIZ, "-%s%s", p->argName, PrintArg(p->argType));
    if(p->save) strcat(buf+len, "*");
    for(q=p+1; q->argLoc == p->argLoc; q++) {
      if(q->argName[0] == '-') continue;
      strcat(buf+len, q == p+1 ? " (" : " ");
      sprintf(buf+strlen(buf), "-%s%s", q->argName, PrintArg(q->argType));
    }
    if(q != p+1) strcat(buf+len, ")");
    len = strlen(buf);
    if(len > 39) len = 0, printf("%s\n", buf); else while(len < 39) buf[len++] = ' ';
    p = q;
  }
  if(len) buf[len] = NULLCHAR, printf("%s\n", buf);
}

void
SlaveResize (Option *opt)
{
}

int
main (int argc, char **argv)
{
    int i, clockFontPxlSize, coordFontPxlSize, fontPxlSize;
    XSetWindowAttributes window_attributes;
    Arg args[16];
    Dimension boardWidth, boardHeight, w, h;
    char *p;
    int forceMono = False;

    extern Option chatOptions[]; // FIXME: adapt Chat window, removing ICS pane and Hide button
    chatOptions[6].type = chatOptions[10].type = Skip;

    srandom(time(0)); // [HGM] book: make random truly random

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    debugFP = stderr;

    if(argc > 1 && (!strcmp(argv[1], "-v" ) || !strcmp(argv[1], "--version" ))) {
      printf("%s version %s\n\n  configure options: %s\n", PACKAGE_NAME, PACKAGE_VERSION, CONFIGURE_OPTIONS);
	exit(0);
    }

    if(argc > 1 && !strcmp(argv[1], "--help" )) {
	PrintOptions();
	exit(0);
    }

    if(argc > 1 && !strcmp(argv[1], "--show-config")) { // [HGM] install: called to print config info
	typedef struct {char *name, *value; } Config;
	static Config configList[] = {
	  { "Datadir", DATADIR },
	  { "Sysconfdir", SYSCONFDIR },
	  { NULL }
	};
	int i;

	for(i=0; configList[i].name; i++) {
	    if(argc > 2 && strcmp(argv[2], configList[i].name)) continue;
	    if(argc > 2) printf("%s", configList[i].value);
	    else printf("%-12s: %s\n", configList[i].name, configList[i].value);
	}
	exit(0);
    }

    programName = strrchr(argv[0], '/');
    if (programName == NULL)
      programName = argv[0];
    else
      programName++;

#ifdef ENABLE_NLS
    XtSetLanguageProc(NULL, NULL, NULL);
    if (appData.debugMode) {
      fprintf(debugFP, "locale = %s\n", setlocale(LC_ALL, NULL));
    }

    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    appData.boardSize = "";
    InitAppData(ConvertToLine(argc, argv));
    p = getenv("HOME");
    if (p == NULL) p = "/tmp";
    i = strlen(p) + strlen("/.xboardXXXXXx.pgn") + 1;
    gameCopyFilename = (char*) malloc(i);
    gamePasteFilename = (char*) malloc(i);
    snprintf(gameCopyFilename,i, "%s/.xboard%05uc.pgn", p, getpid());
    snprintf(gamePasteFilename,i, "%s/.xboard%05up.pgn", p, getpid());

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

    /* [HGM,HR] make sure board size is acceptable */
    if(appData.NrFiles > BOARD_FILES ||
       appData.NrRanks > BOARD_RANKS   )
	 DisplayFatalError(_("Recompile with larger BOARD_RANKS or BOARD_FILES to support this size"), 0, 2);

#if !HIGHDRAG
    /* This feature does not work; animation needs a rewrite */
    appData.highlightDragging = FALSE;
#endif
    InitBackEnd1();

	gameInfo.variant = StringToVariant(appData.variant);
	InitPosition(FALSE);

    shellWidget =
      XtAppInitialize(&appContext, "XBoard", shellOptions,
		      XtNumber(shellOptions),
		      &argc, argv, xboardResources, NULL, 0);

    XtGetApplicationResources(shellWidget, (XtPointer) &appData,
			      clientResources, XtNumber(clientResources),
			      NULL, 0);

    xDisplay = XtDisplay(shellWidget);
    xScreen = DefaultScreen(xDisplay);
    wm_delete_window = XInternAtom(xDisplay, "WM_DELETE_WINDOW", True);

    /*
     * determine size, based on supplied or remembered -size, or screen size
     */
    if (isdigit(appData.boardSize[0])) {
        i = sscanf(appData.boardSize, "%d,%d,%d,%d,%d,%d,%d", &squareSize,
		   &lineGap, &clockFontPxlSize, &coordFontPxlSize,
		   &fontPxlSize, &smallLayout, &tinyLayout);
        if (i == 0) {
	    fprintf(stderr, _("%s: bad boardSize syntax %s\n"),
		    programName, appData.boardSize);
	    exit(2);
	}
	if(BOARD_WIDTH > 8)
	    squareSize = (squareSize*8 + BOARD_WIDTH/2)/BOARD_WIDTH; // scale height
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
	    while (DisplayWidth(xDisplay, xScreen)  < (szd->minScreenSize*BOARD_WIDTH  + 4)/8 ||
		   DisplayHeight(xDisplay, xScreen) < (szd->minScreenSize*BOARD_HEIGHT + 4)/8) {
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

    defaultLineGap = lineGap;
    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap;

    /* [HR] height treated separately (hacked) */
    boardWidth = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);

    /*
     * Determine what fonts to use.
     */
    InitializeFonts(clockFontPxlSize, coordFontPxlSize, fontPxlSize);

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

    if (appData.monoMode && appData.debugMode) {
	fprintf(stderr, _("white pixel = 0x%lx, black pixel = 0x%lx\n"),
		(unsigned long) XWhitePixel(xDisplay, xScreen),
		(unsigned long) XBlackPixel(xDisplay, xScreen));
    }

    ParseIcsTextColors();

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

    optList = BoardPopUp(squareSize, lineGap, (void*)
#if ENABLE_NLS
						&clockFontSet);
#else
						clockFontStruct);
#endif
    InitDrawingHandle(optList + W_BOARD);
    currBoard        = &optList[W_BOARD];
    boardWidget      = optList[W_BOARD].handle;
    menuBarWidget    = optList[W_MENU].handle;
    dropMenu         = optList[W_DROP].handle;
    titleWidget = optList[optList[W_TITLE].type != -1 ? W_TITLE : W_SMALL].handle;
    formWidget  = XtParent(boardWidget);
    XtSetArg(args[0], XtNbackground, &timerBackgroundPixel);
    XtSetArg(args[1], XtNforeground, &timerForegroundPixel);
    XtGetValues(optList[W_WHITE].handle, args, 2);
    if (appData.showButtonBar) { // can't we use timer pixels for this? (Or better yet, just black & white?)
      XtSetArg(args[0], XtNbackground, &buttonBackgroundPixel);
      XtSetArg(args[1], XtNforeground, &buttonForegroundPixel);
      XtGetValues(optList[W_PAUSE].handle, args, 2);
    }

    xBoardWindow = XtWindow(boardWidget);

    // [HGM] it seems the layout code ends here, but perhaps the color stuff is size independent and would
    //       not need to go into InitDrawingSizes().

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
//    XtSetValues(shellWidget, &shellArgs[2], 4);
    marginW =  w - boardWidth; // [HGM] needed to set new shellWidget size when we resize board
    marginH =  h - boardHeight;

    CatchDeleteWindow(shellWidget, "QuitProc");

    CreateAnyPieces(1);
    CreateGrid();

    if(appData.logoSize)
    {   // locate and read user logo
	char buf[MSG_SIZ], name[MSG_SIZ];
	snprintf(name, MSG_SIZ, "/home/%s", UserName());
	if(!FindLogo(name, ".logo", buf))
	    FindLogo(appData.logoDir, name + 6, buf);
	ASSIGN(userLogo, buf);
    }

    if (appData.animate || appData.animateDragging)
      CreateAnimVars();


    char *TranslationsTableMenus=GenerateGlobalTranslationTable ();

    XtAugmentTranslations(formWidget,
			  XtParseTranslationTable(globalTranslations));
    XtAugmentTranslations(formWidget,
			  XtParseTranslationTable(TranslationsTableMenus));

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

    gameInfo.boardWidth = 0; // [HGM] pieces: kludge to ensure InitPosition() calls InitDrawingSizes()
    InitPosition(TRUE);

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

    UpdateLogos(TRUE);
//    XtSetKeyboardFocus(shellWidget, formWidget);
    XSetInputFocus(xDisplay, XtWindow(formWidget), RevertToPointerRoot, CurrentTime);

    XtAppMainLoop(appContext);
    if (appData.debugMode) fclose(debugFP); // [DM] debug
    return 0;
}

void
DoEvents ()
{
    XtInputMask m;
    while((m = XtAppPending(appContext))) XtAppProcessEvent(appContext, m);
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
        fprintf(debugFP, "resolved %s at pixel size %d\n  to %s\n",
		pattern, targetPxlSize, p);
    }
    XFreeFontNames(fonts);
    return p;
}
#endif

void
ReadBitmap (Pixmap *pm, String name, unsigned char bits[], u_int wreq, u_int hreq)
{
    if (bits != NULL) {
	*pm = XCreateBitmapFromData(xDisplay, xBoardWindow, (char *) bits,
				    wreq, hreq);
    }
}

void
MarkMenuItem (char *menuRef, int state)
{
    MenuItem *item = MenuNameToItem(menuRef);

    if(item) {
	Arg args[2];
	XtSetArg(args[0], XtNleftBitmap, state ? xMarkPixmap : None);
	XtSetValues(item->handle, args, 1);
    }
}

void
EnableNamedMenuItem (char *menuRef, int state)
{
    MenuItem *item = MenuNameToItem(menuRef);

    if(item) XtSetSensitive(item->handle, state);
}

void
EnableButtonBar (int state)
{
    XtSetSensitive(optList[W_BUTTON].handle, state);
}


void
SetMenuEnables (Enables *enab)
{
  while (enab->name != NULL) {
    EnableNamedMenuItem(enab->name, enab->value);
    enab++;
  }
}

void
KeyBindingProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // [HGM] new method of key binding: specify MenuItem(FlipView) in stead of FlipViewProc in translation string
    MenuItem *item;
    if(*nprms == 0) return;
    item = MenuNameToItem(prms[0]);
    if(item) ((MenuProc *) item->proc) ();
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

static void
do_flash_delay (unsigned long msec)
{
    TimeDelay(msec);
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

void
ReSize (WindowPlacement *wp)
{
	int sqx, sqy, w, h;
	if(wp->width == wpMain.width && wp->height == wpMain.height) return; // not sized
	sqx = (wp->width  - lineGap - marginW) / BOARD_WIDTH - lineGap;
	sqy = (wp->height - lineGap - marginH) / BOARD_HEIGHT - lineGap;
	if(sqy < sqx) sqx = sqy;
	if(sqx != squareSize) {
	    squareSize = sqx; // adopt new square size
	    CreatePNGPieces(); // make newly scaled pieces
	    InitDrawingSizes(0, 0); // creates grid etc.
	} else ResizeBoardWindow(BOARD_WIDTH * (squareSize + lineGap) + lineGap, BOARD_HEIGHT * (squareSize + lineGap) + lineGap, 0);
	w = BOARD_WIDTH * (squareSize + lineGap) + lineGap;
	h = BOARD_HEIGHT * (squareSize + lineGap) + lineGap;
	if(optList[W_BOARD].max   > w) optList[W_BOARD].max = w;
	if(optList[W_BOARD].value > h) optList[W_BOARD].value = h;
}

static XtIntervalId delayedDragID = 0;

void
DragProc ()
{
	static int busy;
	if(busy) return;

	busy = 1;
	GetActualPlacement(shellWidget, &wpNew);
	if(wpNew.x == wpMain.x && wpNew.y == wpMain.y && // not moved
	   wpNew.width == wpMain.width && wpNew.height == wpMain.height) { // not sized
	    busy = 0; return; // false alarm
	}
	ReSize(&wpNew);
	if(shellUp[EngOutDlg]) CoDrag(shells[EngOutDlg], &wpEngineOutput);
	if(shellUp[HistoryDlg]) CoDrag(shells[HistoryDlg], &wpMoveHistory);
	if(shellUp[EvalGraphDlg]) CoDrag(shells[EvalGraphDlg], &wpEvalGraph);
	if(shellUp[GameListDlg]) CoDrag(shells[GameListDlg], &wpGameList);
	wpMain = wpNew;
	DrawPosition(True, NULL);
	delayedDragID = 0; // now drag executed, make sure next DelayedDrag will not cancel timer event (which could now be used by other)
	busy = 0;
}


void
DelayedDrag ()
{
    if(delayedDragID) XtRemoveTimeOut(delayedDragID); // cancel pending
    delayedDragID =
      XtAppAddTimeOut(appContext, 200, (XtTimerCallbackProc) DragProc, (XtPointer) 0); // and schedule new one 50 msec later
}

void
EventProc (Widget widget, caddr_t unused, XEvent *event)
{
    if(XtIsRealized(widget) && event->type == ConfigureNotify || appData.useStickyWindows)
	DelayedDrag(); // as long as events keep coming in faster than 50 msec, they destroy each other
}

/*
 * event handler for redrawing the board
 */
void
DrawPositionProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    DrawPosition(True, NULL);
}


void
HandlePV (Widget w, XEvent * event, String * params, Cardinal * nParams)
{   // [HGM] pv: walk PV
    MovePV(event->xmotion.x, event->xmotion.y, lineGap + BOARD_HEIGHT * (squareSize + lineGap));
}

extern int savedIndex;  /* gross that this is global */

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


/* Disable all user input other than deleting the window */
static int frozen = 0;

void
FreezeUI ()
{
  if (frozen) return;
  /* Grab by a widget that doesn't accept input */
  XtAddGrab(optList[W_MESSG].handle, TRUE, FALSE);
  frozen = 1;
}

/* Undo a FreezeUI */
void
ThawUI ()
{
  if (!frozen) return;
  XtRemoveGrab(optList[W_MESSG].handle);
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
	MarkMenuItem("Mode.Pause", pausing);

	if (appData.showButtonBar) {
	  /* Always toggle, don't set.  Previous code messes up when
	     invoked while the button is pressed, as releasing it
	     toggles the state again. */
	  {
	    Pixel oldbg, oldfg;
	    XtSetArg(args[0], XtNbackground, &oldbg);
	    XtSetArg(args[1], XtNforeground, &oldfg);
	    XtGetValues(optList[W_PAUSE].handle,
			args, 2);
	    XtSetArg(args[0], XtNbackground, oldfg);
	    XtSetArg(args[1], XtNforeground, oldbg);
	  }
	  XtSetValues(optList[W_PAUSE].handle, args, 2);
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
    MarkMenuItem("Mode.MachineMatch", matchMode && matchGame < appData.matchGames);

    /* Maybe all the enables should be handled here, not just this one */
    EnableNamedMenuItem("Mode.Training", gameMode == Training || gameMode == PlayFromGameFile);

    DisplayLogos(&optList[W_WHITE-1], &optList[W_BLACK+1]);
}


/*
 * Button/menu procedures
 */

/* this variable is shared between CopyPositionProc and SendPositionSelection */
char *selected_fen_position=NULL;

Boolean
SendPositionSelection (Widget w, Atom *selection, Atom *target,
		       Atom *type_return, XtPointer *value_return,
		       unsigned long *length_return, int *format_return)
{
  char *selection_tmp;

//  if (!selected_fen_position) return False; /* should never happen */
  if (*target == XA_STRING || *target == XA_UTF8_STRING(xDisplay)){
   if (!selected_fen_position) { // since it never happens, we use it for indicating a game is being sent
    FILE* f = fopen(gameCopyFilename, "r"); // This code, taken from SendGameSelection, now merges the two
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
   } else {
    /* note: since no XtSelectionDoneProc was registered, Xt will
     * automatically call XtFree on the value returned.  So have to
     * make a copy of it allocated with XtMalloc */
    selection_tmp= XtMalloc(strlen(selected_fen_position)+16);
    safeStrCpy(selection_tmp, selected_fen_position, strlen(selected_fen_position)+16 );
   }

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
CopySomething (char *src)
{
    selected_fen_position = src;
    /*
     * Set both PRIMARY (the selection) and CLIPBOARD, since we don't
     * have a notion of a position that is selected but not copied.
     * See http://www.freedesktop.org/wiki/Specifications/ClipboardsWiki
     */
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

int
ShiftKeys ()
{   // bassic primitive for determining if modifier keys are pressed
    long int codes[] = { XK_Meta_L, XK_Meta_R, XK_Control_L, XK_Control_R, XK_Shift_L, XK_Shift_R };
    char keys[32];
    int i,j,  k=0;
    XQueryKeymap(xDisplay,keys);
    for(i=0; i<6; i++) {
	k <<= 1;
	j = XKeysymToKeycode(xDisplay, codes[i]);
	k += ( (keys[j>>3]&1<<(j&7)) != 0 );
    }
    return k;
}

static void
MoveTypeInProc (Widget widget, caddr_t unused, XEvent *event)
{
    char buf[10];
    KeySym sym;
    int n = XLookupString(&(event->xkey), buf, 10, &sym, NULL);
    if ( n == 1 && *buf >= 32 // printable
	 && !(ShiftKeys() & 0x3C) // no Alt, Ctrl
	) BoxAutoPopUp (buf);
}

static void
UpKeyProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // [HGM] input: let up-arrow recall previous line from history
    IcsKey(1);
}

static void
DownKeyProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // [HGM] input: let down-arrow recall next line from history
    IcsKey(-1);
}

static void
EnterKeyProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    IcsKey(0);
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
ManProc ()
{   // called from menu
    ManInner(NULL, NULL, NULL, NULL);
}

void
SetWindowTitle (char *text, char *title, char *icon)
{
    Arg args[16];
    int i;
    if (appData.titleInWindow) {
	i = 0;
	XtSetArg(args[i], XtNlabel, text);   i++;
	XtSetValues(titleWidget, args, i);
    }
    i = 0;
    XtSetArg(args[i], XtNiconName, (XtArgVal) icon);    i++;
    XtSetArg(args[i], XtNtitle, (XtArgVal) title);      i++;
    XtSetValues(shellWidget, args, i);
    XSync(xDisplay, False);
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
DisplayTimerLabel (Option *opt, char *color, long timer, int highlight)
{
    char buf[MSG_SIZ];
    Arg args[16];
    Widget w = (Widget) opt->handle;

    /* check for low time warning */
    Pixel foregroundOrWarningColor = timerForegroundPixel;

    if (timer > 0 &&
        appData.lowTimeWarning &&
        (timer / 1000) < appData.icsAlarmTime)
      foregroundOrWarningColor = lowTimeWarningColor;

    if (appData.clockMode) {
      snprintf(buf, MSG_SIZ, "%s:%s%s", color, appData.logoSize && !partnerUp ? "\n" : " ", TimeString(timer));
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

static Pixmap *clockIcons[] = { &wIconPixmap, &bIconPixmap };

void
SetClockIcon (int color)
{
    Arg args[16];
    Pixmap pm = *clockIcons[color];
    if (iconPixmap != pm) {
	iconPixmap = pm;
	XtSetArg(args[0], XtNiconPixmap, iconPixmap);
	XtSetValues(shellWidget, args, 1);
    }
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

static int
FindLogo (char *place, char *name, char *buf)
{   // check if file exists in given place
    FILE *f;
    if(!place) return 0;
    snprintf(buf, MSG_SIZ, "%s/%s.png", place, name);
    if(*place && strcmp(place, ".") && (f = fopen(buf, "r")) ) {
	fclose(f);
	return 1;
    }
    return 0;
}

static void
LoadLogo (ChessProgramState *cps, int n, Boolean ics)
{
    char buf[MSG_SIZ], *logoName = buf;
    if(appData.logo[n][0]) {
	logoName = appData.logo[n];
    } else if(appData.autoLogo) {
	if(ics) { // [HGM] logo: in ICS mode second can be used for ICS
	    sprintf(buf, "%s/%s.png", appData.logoDir, appData.icsHost);
	} else { // engine; cascade
	    if(!FindLogo(appData.logoDir, cps->tidy, buf) &&   // first try user log folder
	       !FindLogo(appData.directory[n], "logo", buf) && // then engine directory
	       !FindLogo("/usr/local/share/games/plugins/logos", cps->tidy, buf) ) // then system folders
		FindLogo("/usr/share/games/plugins/logos", cps->tidy, buf);
	}
    }
    if(logoName[0])
	{ ASSIGN(cps->programLogo, logoName); }
}

void
UpdateLogos (int displ)
{
    if(optList[W_WHITE-1].handle == NULL) return;
    LoadLogo(&first, 0, 0);
    LoadLogo(&second, 1, appData.icsActive);
    if(displ) DisplayLogos(&optList[W_WHITE-1], &optList[W_BLACK+1]);
    return;
}

