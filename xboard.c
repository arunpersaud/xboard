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
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <gtk/gtk.h>

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

// [HGM] bitmaps: put before incuding the bitmaps / pixmaps, to know how many piece types there are.
#include "common.h"

#if HAVE_LIBXPM
#include <X11/xpm.h>
#define IMAGE_EXT "xpm"
#else
#define IMAGE_EXT "xim"
#endif

#include "bitmaps/icon_white.bm"
#include "bitmaps/icon_black.bm"
#include "bitmaps/checkmark.bm"

#include "frontend.h"
#include "backend.h"
#include "backendz.h"
#include "moves.h"
#include "xboard.h"
#include "xboard2.h"
#include "childio.h"
#include "xhistory.h"
#include "menus.h"
#include "board.h"
#include "dialogs.h"
#include "engineoutput.h"
#include "usystem.h"
#include "gettext.h"
#include "draw.h"


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
#if ENABLE_NLS
char *InsertPxlSize P((char *pattern, int targetPxlSize));
XFontSet CreateFontSet P((char *base_fnt_lst));
#else
char *FindFont P((char *pattern, int targetPxlSize));
#endif
void DelayedDrag P((void));
void ICSInputBoxPopUp P((void));
gboolean KeyPressProc P((GtkWindow *window, GdkEventKey *eventkey, gpointer data));
Boolean TempBackwardActive = False;
void DisplayMove P((int moveNumber));
void ICSInitScript P((void));
void update_ics_width P(());
int CopyMemoProc P(());
static gboolean EventProc P((GtkWidget *widget, GdkEvent *event, gpointer g));

#ifdef TODO_GTK
#if ENABLE_NLS
XFontSet fontSet, clockFontSet;
#else
Font clockFontID;
XFontStruct *clockFontStruct;
#endif
Font coordFontID, countFontID;
XFontStruct *coordFontStruct, *countFontStruct;
#else
void *shellWidget, *formWidget, *boardWidget, *titleWidget, *dropMenu, *menuBarWidget;
GtkWidget       *mainwindow;
#endif
Option *optList; // contains all widgets of main window
char *layoutName;

char installDir[] = "."; // [HGM] UCI: needed for UCI; probably needs run-time initializtion

/* pixbufs */
static GdkPixbuf       *mainwindowIcon=NULL;
static GdkPixbuf       *WhiteIcon=NULL;
static GdkPixbuf       *BlackIcon=NULL;

typedef unsigned int BoardSize;
BoardSize boardSize;
Boolean chessProgram;

int  minX, minY; // [HGM] placement: volatile limits on upper-left corner
int smallLayout = 0, tinyLayout = 0,
  marginW, marginH, // [HGM] for run-time resizing
  fromX = -1, fromY = -1, toX, toY, commentUp = False,
  errorExitStatus = -1, defaultLineGap;
#ifdef TODO_GTK
Dimension textHeight;
#endif
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

#ifdef TODO_GTK
XtResource clientResources[] = {
    { "flashCount", "flashCount", XtRInt, sizeof(int),
	XtOffset(AppDataPtr, flashCount), XtRImmediate,
	(XtPointer) FLASH_COUNT  },
};
#endif

char globalTranslations[] =
  ":<Key>F9: MenuItem(Actions.Resign) \n \
   :Ctrl<Key>n: MenuItem(File.NewGame) \n \
   :Meta<Key>V: MenuItem(File.NewVariant) \n \
   :Ctrl<Key>o: MenuItem(File.LoadGame) \n \
   :Meta<Key>Next: MenuItem(LoadNextGameProc) \n \
   :Meta<Key>Prior: MenuItem(LoadPrevGameProc) \n \
   :Ctrl<Key>Down: LoadSelectedProc(3) \n \
   :Ctrl<Key>Up: LoadSelectedProc(-3) \n \
   :Ctrl<Key>s: MenuItem(File.SaveGame) \n \
   :Ctrl<Key>c: MenuItem(Edit.CopyGame) \n \
   :Ctrl<Key>v: MenuItem(Edit.PasteGame) \n \
   :Ctrl<Key>O: MenuItem(File.LoadPosition) \n \
   :Shift<Key>Next: MenuItem(LoadNextPositionProc) \n \
   :Shift<Key>Prior: MenuItem(LoadPrevPositionProc) \n \
   :Ctrl<Key>S: MenuItem(File.SavePosition) \n \
   :Ctrl<Key>C: MenuItem(Edit.CopyPosition) \n \
   :Ctrl<Key>V: MenuItem(Edit.PastePosition) \n \
   :Ctrl<Key>q: MenuItem(File.Quit) \n \
   :Ctrl<Key>w: MenuItem(Mode.MachineWhite) \n \
   :Ctrl<Key>b: MenuItem(Mode.MachineBlack) \n \
   :Ctrl<Key>t: MenuItem(Mode.TwoMachines) \n \
   :Ctrl<Key>a: MenuItem(Mode.AnalysisMode) \n \
   :Ctrl<Key>g: MenuItem(Mode.AnalyzeFile) \n \
   :Ctrl<Key>e: MenuItem(Mode.EditGame) \n \
   :Ctrl<Key>E: MenuItem(Mode.EditPosition) \n \
   :Meta<Key>O: MenuItem(View.EngineOutput) \n \
   :Meta<Key>E: MenuItem(View.EvaluationGraph) \n \
   :Meta<Key>G: MenuItem(View.GameList) \n \
   :Meta<Key>H: MenuItem(View.MoveHistory) \n \
   :<Key>Pause: MenuItem(Mode.Pause) \n \
   :<Key>F3: MenuItem(Action.Accept) \n \
   :<Key>F4: MenuItem(Action.Decline) \n \
   :<Key>F12: MenuItem(Action.Rematch) \n \
   :<Key>F5: MenuItem(Action.CallFlag) \n \
   :<Key>F6: MenuItem(Action.Draw) \n \
   :<Key>F7: MenuItem(Action.Adjourn) \n \
   :<Key>F8: MenuItem(Action.Abort) \n \
   :<Key>F10: MenuItem(Action.StopObserving) \n \
   :<Key>F11: MenuItem(Action.StopExamining) \n \
   :Ctrl<Key>d: MenuItem(DebugProc) \n \
   :Meta Ctrl<Key>F12: MenuItem(DebugProc) \n \
   :Meta<Key>End: MenuItem(Edit.ForwardtoEnd) \n \
   :Meta<Key>Right: MenuItem(Edit.Forward) \n \
   :Meta<Key>Home: MenuItem(Edit.BacktoStart) \n \
   :Meta<Key>Left: MenuItem(Edit.Backward) \n \
   :<Key>Left: MenuItem(Edit.Backward) \n \
   :<Key>Right: MenuItem(Edit.Forward) \n \
   :<Key>Home: MenuItem(Edit.Revert) \n \
   :<Key>End: MenuItem(Edit.TruncateGame) \n \
   :Ctrl<Key>m: MenuItem(Engine.MoveNow) \n \
   :Ctrl<Key>x: MenuItem(Engine.RetractMove) \n \
   :Meta<Key>J: MenuItem(Options.Adjudications) \n \
   :Meta<Key>U: MenuItem(Options.CommonEngine) \n \
   :Meta<Key>T: MenuItem(Options.TimeControl) \n \
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
   :<Key>F1: MenuItem(Help.ManXBoard) \n \
   :<Key>F2: MenuItem(View.FlipView) \n \
   :<KeyDown>Return: TempBackwardProc() \n \
   :<KeyUp>Return: TempForwardProc() \n";

char ICSInputTranslations[] =
    "<Key>Up: UpKeyProc() \n "
    "<Key>Down: DownKeyProc() \n "
    "<Key>Return: EnterKeyProc() \n";

// [HGM] vari: another hideous kludge: call extend-end first so we can be sure select-start works,
//             as the widget is destroyed before the up-click can call extend-end
char commentTranslations[] = "<Btn3Down>: extend-end() select-start() CommentClick() \n";

#ifdef TODO_GTK
String xboardResources[] = {
    "*Error*translations: #override\\n <Key>Return: ErrorPopDown()",
    NULL
  };
#endif

/* Max possible square size */
#define MAXSQSIZE 256

static int xpm_avail[MAXSQSIZE];


void
BoardToTop ()
{
#ifdef TODO_GTK
  Arg args[16];
  XtSetArg(args[0], XtNiconic, False);
  XtSetValues(shellWidget, args, 1);

  XtPopup(shellWidget, XtGrabNone); /* Raise if lowered  */
#endif
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

int frameX, frameY;

void
GetActualPlacement (GtkWidget *shell, WindowPlacement *wp)
{
  GtkAllocation a;
  if(!shell) return;
  gtk_widget_get_allocation(shell, &a);
  wp->x = a.x;
  wp->y = a.y;
  wp->width = a.width;
  wp->height = a.height;
printf("placement\n");
  frameX = a.x; frameY = a.y; // remember to decide if windows touch
}
#ifdef TODO_GTK
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
#endif

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
  return DialogExists(BoardWindow);
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
//    gtk_window_resize(GTK_WINDOW(shellWidget), w, h);
#ifdef TODO_GTK
    w += marginW + 1; // [HGM] not sure why the +1 is (sometimes) needed...
    h += marginH;
    shellArgs[0].value = w;
    shellArgs[1].value = h;
    shellArgs[4].value = shellArgs[2].value = w;
    shellArgs[5].value = shellArgs[3].value = h;
    XtSetValues(shellWidget, &shellArgs[0], inhibit ? 6 : 2);

    XSync(xDisplay, False);
#endif
}

int
MakeColors ()
{   // dummy, as the GTK code does not make colors in advance
    return FALSE;
}

void
InitializeFonts (int clockFontPxlSize, int coordFontPxlSize, int fontPxlSize)
{   // determine what fonts to use, and create them
#ifdef TODO_GTK
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
      break;
  }
  return p;
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

int
main (int argc, char **argv)
{
    int i, clockFontPxlSize, coordFontPxlSize, fontPxlSize;
    int boardWidth, boardHeight, w, h;
    char *p;
    int forceMono = False;
    GError *gtkerror=NULL;

    srandom(time(0)); // [HGM] book: make random truly random

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    debugFP = stderr;

    if(argc > 1 && (!strcmp(argv[1], "-v" ) || !strcmp(argv[1], "--version" ))) {
	printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
	exit(0);
    }

    if(argc > 1 && !strcmp(argv[1], "--help" )) {
	PrintOptions();
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
//    if (appData.debugMode) {
//      fprintf(debugFP, "locale = %s\n", setlocale(LC_ALL, NULL));
//    }

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

	gameInfo.variant = StringToVariant(appData.variant);
	InitPosition(FALSE);

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
            GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(mainwindow));
            guint screenwidth = gdk_screen_get_width(screen);
            guint screenheight = gdk_screen_get_height(screen);
	    while (screenwidth < szd->minScreenSize ||
		   screenheight < szd->minScreenSize) {
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
#ifdef TODO_GTK
    InitializeFonts(clockFontPxlSize, coordFontPxlSize, fontPxlSize);
#endif

    /*
     * Detect if there are not enough colors available and adapt.
     */
#ifdef TODO_GTK
    if (DefaultDepth(xDisplay, xScreen) <= 2) {
      appData.monoMode = True;
    }
#endif

    forceMono = MakeColors();

    if (forceMono) {
      fprintf(stderr, _("%s: too few colors available; trying monochrome mode\n"),
	      programName);
	appData.monoMode = True;
    }

    ParseIcsTextColors();

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
#ifdef TODO_GTK
#if ENABLE_NLS
						&clockFontSet);
#else
						clockFontStruct);
#endif
#else
0);
#endif
    InitDrawingHandle(optList + W_BOARD);
    shellWidget      = shells[BoardWindow];
    currBoard        = &optList[W_BOARD];
    boardWidget      = optList[W_BOARD].handle;
    menuBarWidget    = optList[W_MENU].handle;
    dropMenu         = optList[W_DROP].handle;
    titleWidget = optList[optList[W_TITLE].type != -1 ? W_TITLE : W_SMALL].handle;
#ifdef TODO_GTK
    formWidget  = XtParent(boardWidget);
    XtSetArg(args[0], XtNbackground, &timerBackgroundPixel);
    XtSetArg(args[1], XtNforeground, &timerForegroundPixel);
    XtGetValues(optList[W_WHITE].handle, args, 2);
    if (appData.showButtonBar) { // can't we use timer pixels for this? (Or better yet, just black & white?)
      XtSetArg(args[0], XtNbackground, &buttonBackgroundPixel);
      XtSetArg(args[1], XtNforeground, &buttonForegroundPixel);
      XtGetValues(optList[W_PAUSE].handle, args, 2);
    }
#endif

    // [HGM] it seems the layout code ends here, but perhaps the color stuff is size independent and would
    //       not need to go into InitDrawingSizes().

    InitMenuMarkers();

    /*
     * Create an icon. (Use two icons, to indicate whther it is white's or black's turn.)
     */
    WhiteIcon  = gdk_pixbuf_new_from_file(SVGDIR "/icon_white.svg", NULL);
    BlackIcon  = gdk_pixbuf_new_from_file(SVGDIR "/icon_black.svg", NULL);
    mainwindowIcon = WhiteIcon;
    gtk_window_set_icon(GTK_WINDOW(shellWidget), mainwindowIcon);


    /*
     * Create a cursor for the board widget.
     */
#ifdef TODO_GTK
    window_attributes.cursor = XCreateFontCursor(xDisplay, XC_hand2);
    XChangeWindowAttributes(xDisplay, xBoardWindow,
			    CWCursor, &window_attributes);
#endif

    /*
     * Inhibit shell resizing.
     */
#ifdef TODO_GTK
    shellArgs[0].value = (XtArgVal) &w;
    shellArgs[1].value = (XtArgVal) &h;
    XtGetValues(shellWidget, shellArgs, 2);
    shellArgs[4].value = shellArgs[2].value = w;
    shellArgs[5].value = shellArgs[3].value = h;
//    XtSetValues(shellWidget, &shellArgs[2], 4);
#endif
    {
	GtkAllocation a;
	gtk_widget_get_allocation(shells[BoardWindow], &a);
	w = a.width; h = a.height;
printf("start size (%d,%d), %dx%d\n", a.x, a.y, w, h);
    }
    marginW =  w - boardWidth; // [HGM] needed to set new shellWidget size when we resize board
    marginH =  h - boardHeight;

    CreateAnyPieces();
    CreateGrid();

    if(appData.logoSize)
    {   // locate and read user logo
	char buf[MSG_SIZ];
	snprintf(buf, MSG_SIZ, "%s/%s.png", appData.logoDir, UserName());
	ASSIGN(userLogo, buf);
    }

    if (appData.animate || appData.animateDragging)
      CreateAnimVars();

    g_signal_connect(shells[BoardWindow], "key-press-event", G_CALLBACK(KeyPressProc), NULL);
    g_signal_connect(shells[BoardWindow], "configure-event", G_CALLBACK(EventProc), NULL);

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
    UpdateLogos(TRUE);
//    XtSetKeyboardFocus(shellWidget, formWidget);
#ifdef TODO_GTK
    XSetInputFocus(xDisplay, XtWindow(formWidget), RevertToPointerRoot, CurrentTime);
#endif

    /* check for GTK events and process them */
//    gtk_main();
while(1) {
gtk_main_iteration();
}

    if (appData.debugMode) fclose(debugFP); // [DM] debug
    return 0;
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

#ifdef TODO_GTK
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
#endif
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

#ifdef TODO_GTK
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
#endif
    return p;
}
#endif

void
EnableNamedMenuItem (char *menuRef, int state)
{
    MenuItem *item = MenuNameToItem(menuRef);

    if(item) gtk_widget_set_sensitive(item->handle, state);
}

void
EnableButtonBar (int state)
{
#ifdef TODO_GTK
    XtSetSensitive(optList[W_BUTTON].handle, state);
#endif
}


void
SetMenuEnables (Enables *enab)
{
  while (enab->name != NULL) {
    EnableNamedMenuItem(enab->name, enab->value);
    enab++;
  }
}

gboolean KeyPressProc(window, eventkey, data)
     GtkWindow *window;
     GdkEventKey  *eventkey;
     gpointer data;
{

    MoveTypeInProc(eventkey); // pop up for typed in moves

#ifdef TODO_GTK
    /* check for other key values */
    switch(eventkey->keyval) {
        case GDK_question:
	  AboutGameEvent();
	  break;
        default:
	  break;
    }
#endif
    return False;
}
#ifdef TODO_GTK
void
KeyBindingProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{   // [HGM] new method of key binding: specify MenuItem(FlipView) in stead of FlipViewProc in translation string
    MenuItem *item;
    if(*nprms == 0) return;
    item = MenuNameToItem(prms[0]);
    if(item) ((MenuProc *) item->proc) ();
}
#endif

void
SetupDropMenu ()
{
#ifdef TODO_GTK
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
#endif
}

static void
do_flash_delay (unsigned long msec)
{
    TimeDelay(msec);
}

void
FlashDelay (int flash_delay)
{
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
CoDrag (GtkWidget *sh, WindowPlacement *wp)
{
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
#ifdef TODO_GTK
	if(heightInc) XtSetArg(args[j], XtNheight, wp->height + heightInc), j++;
#endif
    } else if(touch > 2 && wpNew.width != wpMain.width) { // top or bottom and width changed
	int widthInc = wpNew.width - wpMain.width;
	double fracLeft = Fraction(wp->x, wpMain.x, wpMain.x + wpMain.width + 2*frameX);
	double fracRght = Fraction(wp->x + wp->width + 2*frameX + 1, wpMain.x, wpMain.x + wpMain.width + 2*frameX);
	wp->y += fracLeft * widthInc;
	widthInc = (int) (fracRght * widthInc) - (int) (fracLeft * widthInc);
#ifdef TODO_GTK
	if(widthInc) XtSetArg(args[j], XtNwidth, wp->width + widthInc), j++;
#endif
    }
    wp->x += wpNew.x - wpMain.x;
    wp->y += wpNew.y - wpMain.y;
    if(touch == 1) wp->x += wpNew.width - wpMain.width; else
    if(touch == 3) wp->y += wpNew.height - wpMain.height;
#ifdef TODO_GTK
    XtSetArg(args[j], XtNx, wp->x); j++;
    XtSetArg(args[j], XtNy, wp->y); j++;
    XtSetValues(sh, args, j);
#endif
}

void
ReSize (WindowPlacement *wp)
{
	int sqx, sqy;
	if(wp->width == wpMain.width && wp->height == wpMain.height) return; // not sized
	sqx = (wp->width  - lineGap - marginW) / BOARD_WIDTH - lineGap;
	sqy = (wp->height - lineGap - marginH) / BOARD_HEIGHT - lineGap;
	if(sqy < sqx) sqx = sqy;
	if(sqx != squareSize) {
	    squareSize = sqx; // adopt new square size
	    CreatePNGPieces(); // make newly scaled pieces
	    InitDrawingSizes(0, 0); // creates grid etc.
	} else ResizeBoardWindow(BOARD_WIDTH * (squareSize + lineGap) + lineGap, BOARD_HEIGHT * (squareSize + lineGap) + lineGap, 0);
}

static guint delayedDragTag = 0;

void
DragProc ()
{
	static int busy;
	if(busy) return;

	busy = 1;
//	GetActualPlacement(shellWidget, &wpNew);
printf("drag proc (%d,%d) %dx%d\n", wpNew.x, wpNew.y, wpNew.width, wpNew.height);
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
	if(delayedDragTag) g_source_remove(delayedDragTag);
	delayedDragTag = 0; // now drag executed, make sure next DelayedDrag will not cancel timer event (which could now be used by other)
	busy = 0;
}

void
DelayedDrag ()
{
printf("old timr = %d\n", delayedDragTag);
    if(delayedDragTag) g_source_remove(delayedDragTag);
    delayedDragTag = g_timeout_add( 200, (GSourceFunc) DragProc, NULL);
printf("new timr = %d\n", delayedDragTag);
}

static gboolean
EventProc (GtkWidget *widget, GdkEvent *event, gpointer g)
{
printf("event proc (%d,%d) %dx%d\n", event->configure.x, event->configure.y, event->configure.width, event->configure.height);
    // immediately
    wpNew.x = event->configure.x;
    wpNew.y = event->configure.y;
    wpNew.width  = event->configure.width;
    wpNew.height = event->configure.height;
    if(appData.useStickyWindows)
	DelayedDrag(); // as long as events keep coming in faster than 50 msec, they destroy each other
    return FALSE;
}



/* Disable all user input other than deleting the window */
static int frozen = 0;

void
FreezeUI ()
{
  if (frozen) return;
  /* Grab by a widget that doesn't accept input */
  gtk_grab_add(optList[W_MESSG].handle);
  frozen = 1;
}

/* Undo a FreezeUI */
void
ThawUI ()
{
  if (!frozen) return;
  gtk_grab_remove(optList[W_MESSG].handle);
  frozen = 0;
}

void
ModeHighlight ()
{
    static int oldPausing = FALSE;
    static GameMode oldmode = (GameMode) -1;
    char *wname;
    if (!boardWidget) return;

    if (pausing != oldPausing) {
	oldPausing = pausing;
	MarkMenuItem("Mode.Pause", pausing);

	if (appData.showButtonBar) {
	  /* Always toggle, don't set.  Previous code messes up when
	     invoked while the button is pressed, as releasing it
	     toggles the state again. */
	    GdkColor color;     
            gdk_color_parse( pausing ? "#808080" : "#F0F0F0", &color );
            gtk_widget_modify_bg ( GTK_WIDGET(optList[W_PAUSE].handle), GTK_STATE_NORMAL, &color );
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

void CopyFileToClipboard(gchar *filename)
{
    gchar *selection_tmp;
    GtkClipboard *cb;

    // read the file
    FILE* f = fopen(filename, "r");
    long len;
    size_t count;
    if (f == NULL) return;
    fseek(f, 0, 2);
    len = ftell(f);
    rewind(f);
    selection_tmp = g_try_malloc(len + 1);
    if (selection_tmp == NULL) {
        printf("Malloc failed in CopyFileToClipboard\n");
        return;
    }
    count = fread(selection_tmp, 1, len, f);
    fclose(f);
    if (len != count) {
      g_free(selection_tmp);
      return;
    }
    selection_tmp[len] = NULLCHAR; // file is now in selection_tmp
    
    // copy selection_tmp to clipboard
    GdkDisplay *gdisp = gdk_display_get_default();
    if (!gdisp) {
        g_free(selection_tmp);
        return;
    }
    cb = gtk_clipboard_get_for_display(gdisp, GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(cb, selection_tmp, -1);
    g_free(selection_tmp);    
}

void
CopySomething (char *src)
{
    GdkDisplay *gdisp = gdk_display_get_default();
    GtkClipboard *cb;
    if(!src) { CopyFileToClipboard(gameCopyFilename); return; }
    if (gdisp == NULL) return;
    cb = gtk_clipboard_get_for_display(gdisp, GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(cb, src, -1);
}

void
PastePositionProc ()
{
    GdkDisplay *gdisp = gdk_display_get_default();
    GtkClipboard *cb;
    gchar *fenstr;

    if (gdisp == NULL) return;
    cb = gtk_clipboard_get_for_display(gdisp, GDK_SELECTION_CLIPBOARD);    
    fenstr = gtk_clipboard_wait_for_text(cb);
    if (fenstr==NULL) return; // nothing had been selected to copy  
    EditPositionPasteFEN(fenstr);
    return;
}

void
PasteGameProc ()
{
    gchar *text=NULL;
    GtkClipboard *cb;
    guint len=0;
    FILE* f;

    // get game from clipboard
    GdkDisplay *gdisp = gdk_display_get_default();
    if (gdisp == NULL) return;
    cb = gtk_clipboard_get_for_display(gdisp, GDK_SELECTION_CLIPBOARD);    
    text = gtk_clipboard_wait_for_text(cb);
    if (text == NULL) return; // nothing to paste  
    len = strlen(text);

    // write to temp file
    if (text == NULL || len == 0) {
      return; //nothing to paste 
    }
    f = fopen(gamePasteFilename, "w");
    if (f == NULL) {
      DisplayError(_("Can't open temp file"), errno);
      return;
    }
    fwrite(text, 1, len, f);
    fclose(f);

    // load from file 
    LoadGameFromFile(gamePasteFilename, 0, gamePasteFilename, TRUE);
    return;
}


#ifdef TODO_GTK
void
QuitWrapper (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    QuitProc();
}
#endif

void MoveTypeInProc(eventkey)
    GdkEventKey  *eventkey;
{
    char buf[10];

    // ingnore if ctrl or alt is pressed
    if (eventkey->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) {
        return;
    }

    buf[0]=eventkey->keyval;
    buf[1]='\0';
    if (*buf >= 32)        
	BoxAutoPopUp (buf);
}

#ifdef TODO_GTK
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
#endif

void
ManProc ()
{   // called from menu
#ifdef TODO_GTK
    ManInner(NULL, NULL, NULL, NULL);
#endif
}

void
SetWindowTitle (char *text, char *title, char *icon)
{
#ifdef TODO_GTK
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
#endif
    if (appData.titleInWindow) {
	SetWidgetLabel(titleWidget, text);
    }
    gtk_window_set_title (GTK_WINDOW(shells[BoardWindow]), title);
}


static int
NullXErrorCheck (Display *dpy, XErrorEvent *error_event)
{
    return 0;
}

void
DisplayIcsInteractionTitle (String message)
{
#ifdef TODO_GTK
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
#endif
}


void
DisplayTimerLabel (Option *opt, char *color, long timer, int highlight)
{
    GtkWidget *w = (GtkWidget *) opt->handle;
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
    if (timer > 0 &&
        appData.lowTimeWarning &&
        (timer / 1000) < appData.icsAlarmTime) {
        strcpy(fgcolor, appData.lowTimeWarningColor);
    }

    if (appData.clockMode) {
        markup = g_markup_printf_escaped("<span size=\"xx-large\" weight=\"heavy\" background=\"%s\" foreground=\"%s\">%s:%s%s</span>",
					 bgcolor, fgcolor, color, appData.logoSize && !partnerUp ? "\n" : " ", TimeString(timer));
    } else {
        markup = g_markup_printf_escaped("<span size=\"xx-large\" weight=\"heavy\" background=\"%s\" foreground=\"%s\">%s  </span>",
					 bgcolor, fgcolor, color);
    }
    gtk_label_set_markup(GTK_LABEL(w), markup);
    g_free(markup);
}

static GdkPixbuf **clockIcons[] = { &WhiteIcon, &BlackIcon };

void
SetClockIcon (int color)
{
    GdkPixbuf *pm = *clockIcons[color];
    if (mainwindowIcon != pm) {
        mainwindowIcon = pm;
	gtk_window_set_icon(GTK_WINDOW(shellWidget), mainwindowIcon);
    }
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

gboolean
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
	    return True;
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
    return True; // Must return true or the watch will be removed
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
#ifdef TODO_GTK
  XSync(xDisplay, False);
#endif
//  gtk_main_iteration_do(False);

  if (time > 0)
    usleep(time * 1000);
}

#endif

static void
LoadLogo (ChessProgramState *cps, int n, Boolean ics)
{
    char buf[MSG_SIZ], *logoName = buf;
    if(appData.logo[n][0]) {
	logoName = appData.logo[n];
    } else if(appData.autoLogo) {
	if(ics) { // [HGM] logo: in ICS mode second can be used for ICS
	    sprintf(buf, "%s/%s.png", appData.logoDir, appData.icsHost);
	} else if(appData.directory[n] && appData.directory[n][0]) {
	    sprintf(buf, "%s/%s.png", appData.logoDir, cps->tidy);
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

void FileNamePopUpGTK(label, def, filter, proc, openMode)
     char *label;
     char *def;
     char *filter;
     FileProc proc;
     char *openMode;
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

  if (openMode[0] == 'r')
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

