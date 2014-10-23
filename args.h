/*
 * args.c -- Option parsing and saving for X and Windows versions of XBoard
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
 ** See the file ChangeLog for a revision history.
*/

// Note: this file is not a normal header, but contains executable code
// for #inclusion in winboard.c and xboard.c, rather than separate compilation,
// so that it can make use of the proper context of #defined symbols and
// declarations in those files.

typedef enum {
  ArgString, ArgInt, ArgFloat, ArgBoolean, ArgTrue, ArgFalse, ArgNone,
  ArgColor, ArgAttribs, ArgFilename, ArgBoardSize, ArgFont, ArgCommSettings,
  ArgSettingsFilename, ArgBackupSettingsFile, ArgTwo, ArgInstall, ArgMaster,
  ArgX, ArgY, ArgZ // [HGM] placement: for window-placement options stored relative to main window
} ArgType;

typedef void *ArgIniType;

#define INVALID (ArgIniType) 6915 /* Some number unlikely to be needed as default for anything */
#define MAX_ARG_LEN 128*1024 /* [AS] For Roger Brown's very long list! */

typedef struct {
  char *argName;
  ArgType argType;
  /***
  union {
    String *pString;       // ArgString
    int *pInt;             // ArgInt
    float *pFloat;         // ArgFloat
    Boolean *pBoolean;     // ArgBoolean
    COLORREF *pColor;      // ArgColor
    ColorClass cc;         // ArgAttribs
    String *pFilename;     // ArgFilename
    BoardSize *pBoardSize; // ArgBoardSize
    int whichFont;         // ArgFont
    DCB *pDCB;             // ArgCommSettings
    String *pFilename;     // ArgSettingsFilename
  } argLoc;
  ***/
  void *argLoc;
  Boolean save;
  ArgIniType defaultValue;
} ArgDescriptor;

typedef struct {
  char *item;
  char *command;
  Boolean getname;
  Boolean immediate;
} IcsTextMenuEntry;

IcsTextMenuEntry icsTextMenuEntry[ICS_TEXT_MENU_SIZE];

int junk;
unsigned int saveDate;
unsigned int dateStamp;
Boolean singleList;
Boolean autoClose;
char *homeDir;
char *firstEngineLine;
char *secondEngineLine;
char *icsNick;
char *theme;

void EnsureOnScreen(int *x, int *y, int minX, int minY);
char StringGet(void *getClosure);
void ParseFont(char *name, int number);
void SetFontDefaults();
void CreateFonts();
void ParseColor(int n, char *name);
void ParseTextAttribs(ColorClass cc, char *s);
void ParseBoardSize(void * addr, char *name);
void ParseCommPortSettings(char *name);
void LoadAllSounds();
void SetCommPortDefaults();
void SaveFontArg(FILE *f, ArgDescriptor *ad);
void ExportSounds();
void SaveAttribsArg(FILE *f, ArgDescriptor *ad);
void SaveColor(FILE *f, ArgDescriptor *ad);
void SaveBoardSize(FILE *f, char *name, void *addr);
void PrintCommPortSettings(FILE *f, char *name);
void GetWindowCoords();
int  MainWindowUp();
void PopUpStartupDialog();
typedef char GetFunc(void *getClosure);
void ParseArgs(GetFunc get, void *cl);

// [HGM] this is an exact duplicate of something in winboard.c. Move to backend.c?
char *defaultTextAttribs[] =
{
  COLOR_SHOUT, COLOR_SSHOUT, COLOR_CHANNEL1, COLOR_CHANNEL, COLOR_KIBITZ,
  COLOR_TELL, COLOR_CHALLENGE, COLOR_REQUEST, COLOR_SEEK, COLOR_NORMAL,
  "#000000"
};

ArgDescriptor argDescriptors[] = {
  /* positional arguments */
  { "opt", ArgSettingsFilename, (void *) NULL, FALSE, INVALID },
  { "loadPositionFile", ArgFilename, (void *) &appData.loadPositionFile, FALSE, INVALID },
  { "tourneyFile", ArgFilename, (void *) &appData.tourneyFile, FALSE, INVALID },
  { "is", ArgString, (void *) &icsNick, FALSE, INVALID },
  { "loadGameFile", ArgFilename, (void *) &appData.loadGameFile, FALSE, INVALID },
  { "", ArgNone, NULL, FALSE, INVALID },
  /* keyword arguments */
  { "saveDate", ArgInt, (void *) &saveDate, TRUE, 0 },
  { "date", ArgInt, (void *) &dateStamp, FALSE, 0 },
  { "autoClose", ArgTrue, (void *) &autoClose, FALSE, FALSE },
  JAWS_ARGS
  { "whitePieceColor", ArgColor, (void *) 0, TRUE, (ArgIniType) WHITE_PIECE_COLOR },
  { "wpc", ArgColor, (void *) 0, FALSE, INVALID },
  { "blackPieceColor", ArgColor, (void *) 1, TRUE, (ArgIniType) BLACK_PIECE_COLOR },
  { "bpc", ArgColor, (void *) 1, FALSE, INVALID },
  { "lightSquareColor", ArgColor, (void *) 2, TRUE, (ArgIniType) LIGHT_SQUARE_COLOR },
  { "lsc", ArgColor, (void *) 2, FALSE, INVALID },
  { "darkSquareColor", ArgColor, (void *) 3, TRUE, (ArgIniType) DARK_SQUARE_COLOR },
  { "dsc", ArgColor, (void *) 3, FALSE, INVALID },
  { "highlightSquareColor", ArgColor, (void *) 4, TRUE, (ArgIniType) HIGHLIGHT_SQUARE_COLOR },
  { "hsc", ArgColor, (void *) 4, FALSE, INVALID },
  { "premoveHighlightColor", ArgColor, (void *) 5, TRUE, (ArgIniType) PREMOVE_HIGHLIGHT_COLOR },
  { "phc", ArgColor, (void *) 5, FALSE, INVALID },
  { "movesPerSession", ArgInt, (void *) &appData.movesPerSession, TRUE, (ArgIniType) MOVES_PER_SESSION },
  { "mps", ArgInt, (void *) &appData.movesPerSession, FALSE, INVALID },
  { "initString", ArgString, (void *) &appData.firstInitString, FALSE, INVALID },
  { "firstInitString", ArgString, (void *) &appData.firstInitString, FALSE, (ArgIniType) INIT_STRING },
  { "secondInitString", ArgString, (void *) &appData.secondInitString, FALSE, (ArgIniType) INIT_STRING },
  { "firstComputerString", ArgString, (void *) &appData.firstComputerString,
    FALSE, (ArgIniType) COMPUTER_STRING },
  { "secondComputerString", ArgString, (void *) &appData.secondComputerString,
    FALSE, (ArgIniType) COMPUTER_STRING },
  { "firstChessProgram", ArgFilename, (void *) &appData.firstChessProgram,
    FALSE, (ArgIniType) FIRST_CHESS_PROGRAM },
  { "fcp", ArgFilename, (void *) &appData.firstChessProgram, FALSE, INVALID },
  { "secondChessProgram", ArgFilename, (void *) &appData.secondChessProgram,
    FALSE, (ArgIniType) SECOND_CHESS_PROGRAM },
  { "scp", ArgFilename, (void *) &appData.secondChessProgram, FALSE, INVALID },
  { "fe", ArgString, (void *) &firstEngineLine, FALSE, "" },
  { "se", ArgString, (void *) &secondEngineLine, FALSE, "" },
  { "firstPlaysBlack", ArgBoolean, (void *) &appData.firstPlaysBlack, FALSE, FALSE },
  { "fb", ArgTrue, (void *) &appData.firstPlaysBlack, FALSE, FALSE },
  { "xfb", ArgFalse, (void *) &appData.firstPlaysBlack, FALSE, INVALID },
  { "-fb", ArgFalse, (void *) &appData.firstPlaysBlack, FALSE, INVALID },
  { "noChessProgram", ArgBoolean, (void *) &appData.noChessProgram, FALSE, FALSE },
  { "ncp", ArgTrue, (void *) &appData.noChessProgram, FALSE, INVALID },
  { "xncp", ArgFalse, (void *) &appData.noChessProgram, FALSE, INVALID },
  { "-ncp", ArgFalse, (void *) &appData.noChessProgram, FALSE, INVALID },
  { "firstHost", ArgString, (void *) &appData.firstHost, FALSE, (ArgIniType) FIRST_HOST },
  { "fh", ArgString, (void *) &appData.firstHost, FALSE, INVALID },
  { "secondHost", ArgString, (void *) &appData.secondHost, FALSE, (ArgIniType) SECOND_HOST },
  { "sh", ArgString, (void *) &appData.secondHost, FALSE, INVALID },
  { "firstDirectory", ArgFilename, (void *) &appData.firstDirectory, FALSE, (ArgIniType) FIRST_DIRECTORY },
  { "fd", ArgFilename, (void *) &appData.firstDirectory, FALSE, INVALID },
  { "secondDirectory", ArgFilename, (void *) &appData.secondDirectory, FALSE, (ArgIniType) SECOND_DIRECTORY },
  { "sd", ArgFilename, (void *) &appData.secondDirectory, FALSE, INVALID },
  { "variations", ArgBoolean, (void *) &appData.variations, TRUE, (ArgIniType) FALSE },
  { "appendPV", ArgBoolean, (void *) &appData.autoExtend, TRUE, (ArgIniType) FALSE },
  { "theme", ArgString, (void *) &theme, FALSE, (ArgIniType) "" },

  /* some options only used by the XBoard front end, and ignored in WinBoard         */
  /* Their saving is controlled by XBOARD, which in WinBoard is defined as FALSE */
  { "internetChessServerInputBox", ArgBoolean, (void *) &appData.icsInputBox, XBOARD, (ArgIniType) FALSE },
  { "icsinput", ArgTrue, (void *) &appData.icsInputBox, FALSE, INVALID },
  { "xicsinput", ArgFalse, (void *) &appData.icsInputBox, FALSE, INVALID },
  { "cmail", ArgString, (void *) &appData.cmailGameName, FALSE, (ArgIniType) "" },
  { "soundProgram", ArgFilename, (void *) &appData.soundProgram, XBOARD, (ArgIniType) "play" },
  { "fontSizeTolerance", ArgInt, (void *) &appData.fontSizeTolerance, XBOARD, (ArgIniType) 4 },
  { "lowTimeWarningColor", ArgColor, (void *) 6, XBOARD, (ArgIniType) LOWTIMEWARNING_COLOR },
  { "lowTimeWarning", ArgBoolean, (void *) &appData.lowTimeWarning, XBOARD, (ArgIniType) FALSE },
  { "titleInWindow", ArgBoolean, (void *) &appData.titleInWindow, XBOARD, (ArgIniType) FALSE },
  { "title", ArgTrue, (void *) &appData.titleInWindow, FALSE, INVALID },
  { "xtitle", ArgFalse, (void *) &appData.titleInWindow, FALSE, INVALID },
  { "flashCount", ArgInt, (void *) &appData.flashCount, XBOARD, INVALID }, // let X handle this
  { "flashRate", ArgInt, (void *) &appData.flashRate, XBOARD, (ArgIniType) FLASH_RATE },
  { "pieceImageDirectory", ArgFilename, (void *) &appData.pieceDirectory, TRUE, (ArgIniType) "" },
  { "pid", ArgFilename, (void *) &appData.pieceDirectory, FALSE, INVALID },
  { "trueColors", ArgBoolean, (void *) &appData.trueColors, TRUE, (ArgIniType) FALSE },
  { "soundDirectory", ArgFilename, (void *) &appData.soundDirectory, XBOARD, (ArgIniType) "" },
  { "msLoginDelay", ArgInt, (void *) &appData.msLoginDelay, XBOARD, (ArgIniType) MS_LOGIN_DELAY },
  { "pasteSelection", ArgBoolean, (void *) &appData.pasteSelection, XBOARD, (ArgIniType) FALSE },

  { "dropMenu", ArgBoolean, (void *) &appData.dropMenu, TRUE, (ArgIniType) FALSE },
  { "pieceMenu", ArgBoolean, (void *) &appData.pieceMenu, TRUE, (ArgIniType) TRUE },
  { "sweepPromotions", ArgBoolean, (void *) &appData.sweepSelect, TRUE, (ArgIniType) FALSE },
  { "remoteShell", ArgFilename, (void *) &appData.remoteShell, FALSE, (ArgIniType) REMOTE_SHELL },
  { "rsh", ArgFilename, (void *) &appData.remoteShell, FALSE, INVALID },
  { "remoteUser", ArgString, (void *) &appData.remoteUser, FALSE, (ArgIniType) "" },
  { "ruser", ArgString, (void *) &appData.remoteUser, FALSE, INVALID },
  { "timeDelay", ArgFloat, (void *) &appData.timeDelay, TRUE, INVALID },
  { "td", ArgFloat, (void *) &appData.timeDelay, FALSE, INVALID },
  { "timeControl", ArgString, (void *) &appData.timeControl, TRUE, (ArgIniType) TIME_CONTROL },
  { "tc", ArgString, (void *) &appData.timeControl, FALSE, INVALID },
  { "timeIncrement", ArgFloat, (void *) &appData.timeIncrement, FALSE, INVALID },
  { "inc", ArgFloat, (void *) &appData.timeIncrement, FALSE, INVALID },
  { "internetChessServerMode", ArgBoolean, (void *) &appData.icsActive, FALSE, INVALID },
  { "ics", ArgTrue, (void *) &appData.icsActive, FALSE, (ArgIniType) FALSE },
  { "xics", ArgFalse, (void *) &appData.icsActive, FALSE, INVALID },
  { "-ics", ArgFalse, (void *) &appData.icsActive, FALSE, INVALID },
  { "is", ArgString, (void *) &icsNick, FALSE, "" },
  { "internetChessServerHost", ArgString, (void *) &appData.icsHost, FALSE, (ArgIniType) "" },
  { "icshost", ArgString, (void *) &appData.icsHost, FALSE, INVALID },
  { "internetChessServerPort", ArgString, (void *) &appData.icsPort, FALSE, (ArgIniType) ICS_PORT },
  { "icsport", ArgString, (void *) &appData.icsPort, FALSE, INVALID },
  { "internetChessServerCommPort", ArgString, (void *) &appData.icsCommPort, FALSE, (ArgIniType) ICS_COMM_PORT },
  { "icscomm", ArgString, (void *) &appData.icsCommPort, FALSE, INVALID },
  { "internetChessServerComPort", ArgString, (void *) &appData.icsCommPort, FALSE, INVALID },
  { "icscom", ArgString, (void *) &appData.icsCommPort, FALSE, INVALID },
  { "internetChessServerLogonScript", ArgFilename, (void *) &appData.icsLogon, FALSE, (ArgIniType) ICS_LOGON },
  { "icslogon", ArgFilename, (void *) &appData.icsLogon, FALSE, INVALID },
  { "useTelnet", ArgBoolean, (void *) &appData.useTelnet, FALSE, INVALID },
  { "telnet", ArgTrue, (void *) &appData.useTelnet, FALSE, INVALID },
  { "xtelnet", ArgFalse, (void *) &appData.useTelnet, FALSE, INVALID },
  { "-telnet", ArgFalse, (void *) &appData.useTelnet, FALSE, INVALID },
  { "telnetProgram", ArgFilename, (void *) &appData.telnetProgram, FALSE, (ArgIniType) TELNET_PROGRAM },
  { "internetChessserverHelper", ArgFilename, (void *) &appData.icsHelper,
	FALSE, INVALID }, // for XB
  { "icshelper", ArgFilename, (void *) &appData.icsHelper, FALSE, (ArgIniType) "" },
  { "seekGraph", ArgBoolean, (void *) &appData.seekGraph, TRUE, (ArgIniType) FALSE },
  { "sg", ArgTrue, (void *) &appData.seekGraph, FALSE, INVALID },
  { "autoRefresh", ArgBoolean, (void *) &appData.autoRefresh, TRUE, (ArgIniType) FALSE },
  { "autoBox", ArgBoolean, (void *) &appData.autoBox, XBOARD, (ArgIniType) TRUE },
  { "gateway", ArgString, (void *) &appData.gateway, FALSE, (ArgIniType) "" },
  { "loadGameFile", ArgFilename, (void *) &appData.loadGameFile, FALSE, (ArgIniType) "" },
  { "lgf", ArgFilename, (void *) &appData.loadGameFile, FALSE, INVALID },
  { "loadGameIndex", ArgInt, (void *) &appData.loadGameIndex, FALSE, (ArgIniType) 0 },
  { "lgi", ArgInt, (void *) &appData.loadGameIndex, FALSE, INVALID },
  { "saveGameFile", ArgFilename, (void *) &appData.saveGameFile, TRUE, (ArgIniType) "" },
  { "sgf", ArgFilename, (void *) &appData.saveGameFile, FALSE, INVALID },
  { "autoSaveGames", ArgBoolean, (void *) &appData.autoSaveGames, TRUE, (ArgIniType) FALSE },
  { "autosave", ArgTrue, (void *) &appData.autoSaveGames, FALSE, INVALID },
  { "xautosave", ArgFalse, (void *) &appData.autoSaveGames, FALSE, INVALID },
  { "-autosave", ArgFalse, (void *) &appData.autoSaveGames, FALSE, INVALID },
  { "onlyOwnGames", ArgBoolean, (void *) &appData.onlyOwn, TRUE, (ArgIniType) FALSE },
  { "loadPositionFile", ArgFilename, (void *) &appData.loadPositionFile, FALSE, (ArgIniType) "" },
  { "lpf", ArgFilename, (void *) &appData.loadPositionFile, FALSE, INVALID },
  { "loadPositionIndex", ArgInt, (void *) &appData.loadPositionIndex, FALSE, (ArgIniType) 1 },
  { "lpi", ArgInt, (void *) &appData.loadPositionIndex, FALSE, INVALID },
  { "savePositionFile", ArgFilename, (void *) &appData.savePositionFile, FALSE, (ArgIniType) "" },
  { "spf", ArgFilename, (void *) &appData.savePositionFile, FALSE, INVALID },
  { "matchMode", ArgBoolean, (void *) &appData.matchMode, FALSE, (ArgIniType) FALSE },
  { "mm", ArgTrue, (void *) &appData.matchMode, FALSE, INVALID },
  { "xmm", ArgFalse, (void *) &appData.matchMode, FALSE, INVALID },
  { "-mm", ArgFalse, (void *) &appData.matchMode, FALSE, INVALID },
  { "matchGames", ArgInt, (void *) &appData.matchGames, FALSE, (ArgIniType) 0 },
  { "mg", ArgInt, (void *) &appData.matchGames, FALSE, INVALID },
  { "monoMode", ArgBoolean, (void *) &appData.monoMode, TRUE, (ArgIniType) FALSE },
  { "mono", ArgTrue, (void *) &appData.monoMode, FALSE, INVALID },
  { "xmono", ArgFalse, (void *) &appData.monoMode, FALSE, INVALID },
  { "-mono", ArgFalse, (void *) &appData.monoMode, FALSE, INVALID },
  { "debugMode", ArgBoolean, (void *) &appData.debugMode, FALSE, (ArgIniType) FALSE },
  { "debug", ArgTrue, (void *) &appData.debugMode, FALSE, INVALID },
  { "xdebug", ArgFalse, (void *) &appData.debugMode, FALSE, INVALID },
  { "-debug", ArgFalse, (void *) &appData.debugMode, FALSE, INVALID },
  { "clockMode", ArgBoolean, (void *) &appData.clockMode, FALSE, (ArgIniType) TRUE },
  { "clock", ArgTrue, (void *) &appData.clockMode, FALSE, INVALID },
  { "xclock", ArgFalse, (void *) &appData.clockMode, FALSE, INVALID },
  { "-clock", ArgFalse, (void *) &appData.clockMode, FALSE, INVALID },
  { "searchTime", ArgString, (void *) &appData.searchTime, FALSE, (ArgIniType) "" },
  { "st", ArgString, (void *) &appData.searchTime, FALSE, INVALID },
  { "searchDepth", ArgInt, (void *) &appData.searchDepth, FALSE, (ArgIniType) 0 },
  { "depth", ArgInt, (void *) &appData.searchDepth, FALSE, INVALID },
  { "showCoords", ArgBoolean, (void *) &appData.showCoords, TRUE, (ArgIniType) FALSE },
  { "coords", ArgTrue, (void *) &appData.showCoords, FALSE, INVALID },
  { "xcoords", ArgFalse, (void *) &appData.showCoords, FALSE, INVALID },
  { "-coords", ArgFalse, (void *) &appData.showCoords, FALSE, INVALID },
  { "showThinking", ArgBoolean, (void *) &appData.showThinking, TRUE, (ArgIniType) FALSE },
  { "thinking", ArgTrue, (void *) &appData.showThinking, FALSE, INVALID },
  { "xthinking", ArgFalse, (void *) &appData.showThinking, FALSE, INVALID },
  { "-thinking", ArgFalse, (void *) &appData.showThinking, FALSE, INVALID },
  { "ponderNextMove", ArgBoolean, (void *) &appData.ponderNextMove, TRUE, (ArgIniType) TRUE },
  { "ponder", ArgTrue, (void *) &appData.ponderNextMove, FALSE, INVALID },
  { "xponder", ArgFalse, (void *) &appData.ponderNextMove, FALSE, INVALID },
  { "-ponder", ArgFalse, (void *) &appData.ponderNextMove, FALSE, INVALID },
  { "periodicUpdates", ArgBoolean, (void *) &appData.periodicUpdates, TRUE, (ArgIniType) TRUE },
  { "periodic", ArgTrue, (void *) &appData.periodicUpdates, FALSE, INVALID },
  { "xperiodic", ArgFalse, (void *) &appData.periodicUpdates, FALSE, INVALID },
  { "-periodic", ArgFalse, (void *) &appData.periodicUpdates, FALSE, INVALID },
  { "popupExitMessage", ArgBoolean, (void *) &appData.popupExitMessage, TRUE, (ArgIniType) TRUE },
  { "exit", ArgTrue, (void *) &appData.popupExitMessage, FALSE, INVALID },
  { "xexit", ArgFalse, (void *) &appData.popupExitMessage, FALSE, INVALID },
  { "-exit", ArgFalse, (void *) &appData.popupExitMessage, FALSE, INVALID },
  { "popupMoveErrors", ArgBoolean, (void *) &appData.popupMoveErrors, TRUE, (ArgIniType) FALSE },
  { "popup", ArgTrue, (void *) &appData.popupMoveErrors, FALSE, INVALID },
  { "xpopup", ArgFalse, (void *) &appData.popupMoveErrors, FALSE, INVALID },
  { "-popup", ArgFalse, (void *) &appData.popupMoveErrors, FALSE, INVALID },
  { "popUpErrors", ArgBoolean, (void *) &appData.popupMoveErrors,
    FALSE, INVALID }, /* only so that old WinBoard.ini files from betas can be read */
  { "clockFont", ArgFont, (void *) CLOCK_FONT, TRUE, INVALID },
  { "messageFont", ArgFont, (void *) MESSAGE_FONT, TRUE, INVALID },
  { "font", ArgFont, (void *) MESSAGE_FONT, FALSE, INVALID }, /* only so that old .xboardrc files will parse. -font does not work from the command line because it is captured by the X libraries. */
  { "coordFont", ArgFont, (void *) COORD_FONT, TRUE, INVALID },
  { "tagsFont", ArgFont, (void *) EDITTAGS_FONT, TRUE, INVALID },
  { "commentFont", ArgFont, (void *) COMMENT_FONT, TRUE, INVALID },
  { "icsFont", ArgFont, (void *) CONSOLE_FONT, TRUE, INVALID },
  { "moveHistoryFont", ArgFont, (void *) MOVEHISTORY_FONT, TRUE, INVALID }, /* [AS] */
  { "gameListFont", ArgFont, (void *) GAMELIST_FONT, TRUE, INVALID }, /* [HGM] */
  { "boardSize", ArgBoardSize, (void *) &boardSize,
    TRUE, (ArgIniType) -1 }, /* must come after all fonts */
  { "size", ArgBoardSize, (void *) &boardSize, FALSE, INVALID },
  { "ringBellAfterMoves", ArgBoolean, (void *) &appData.ringBellAfterMoves,
    FALSE, (ArgIniType) TRUE }, /* historical; kept only so old winboard.ini files will parse */
  { "bell", ArgTrue, (void *) &appData.ringBellAfterMoves, FALSE, INVALID }, // for XB
  { "xbell", ArgFalse, (void *) &appData.ringBellAfterMoves, FALSE, INVALID }, // for XB
  { "movesound", ArgTrue, (void *) &appData.ringBellAfterMoves, FALSE, INVALID }, // for XB
  { "xmovesound", ArgFalse, (void *) &appData.ringBellAfterMoves, FALSE, INVALID }, // for XB
  { "alwaysOnTop", ArgBoolean, (void *) &alwaysOnTop, TRUE, INVALID },
  { "top", ArgTrue, (void *) &alwaysOnTop, FALSE, INVALID },
  { "xtop", ArgFalse, (void *) &alwaysOnTop, FALSE, INVALID },
  { "-top", ArgFalse, (void *) &alwaysOnTop, FALSE, INVALID },
  { "autoCallFlag", ArgBoolean, (void *) &appData.autoCallFlag, TRUE, (ArgIniType) FALSE },
  { "autoflag", ArgTrue, (void *) &appData.autoCallFlag, FALSE, INVALID },
  { "xautoflag", ArgFalse, (void *) &appData.autoCallFlag, FALSE, INVALID },
  { "-autoflag", ArgFalse, (void *) &appData.autoCallFlag, FALSE, INVALID },
  { "autoComment", ArgBoolean, (void *) &appData.autoComment, TRUE, (ArgIniType) FALSE },
  { "autocomm", ArgTrue, (void *) &appData.autoComment, FALSE, INVALID },
  { "xautocomm", ArgFalse, (void *) &appData.autoComment, FALSE, INVALID },
  { "-autocomm", ArgFalse, (void *) &appData.autoComment, FALSE, INVALID },
  { "autoCreateLogon", ArgBoolean, (void *) &appData.autoCreateLogon, TRUE, (ArgIniType) FALSE },
  { "autoObserve", ArgBoolean, (void *) &appData.autoObserve, TRUE, (ArgIniType) FALSE },
  { "autobs", ArgTrue, (void *) &appData.autoObserve, FALSE, INVALID },
  { "xautobs", ArgFalse, (void *) &appData.autoObserve, FALSE, INVALID },
  { "-autobs", ArgFalse, (void *) &appData.autoObserve, FALSE, INVALID },
  { "flipView", ArgBoolean, (void *) &appData.flipView, FALSE, (ArgIniType) FALSE },
  { "flip", ArgTrue, (void *) &appData.flipView, FALSE, INVALID },
  { "xflip", ArgFalse, (void *) &appData.flipView, FALSE, INVALID },
  { "-flip", ArgFalse, (void *) &appData.flipView, FALSE, INVALID },
  { "autoFlipView", ArgBoolean, (void *) &appData.autoFlipView, TRUE, (ArgIniType) TRUE },
  { "autoflip", ArgTrue, (void *) &appData.autoFlipView, FALSE, INVALID },
  { "xautoflip", ArgFalse, (void *) &appData.autoFlipView, FALSE, INVALID },
  { "-autoflip", ArgFalse, (void *) &appData.autoFlipView, FALSE, INVALID },
  { "autoRaiseBoard", ArgBoolean, (void *) &appData.autoRaiseBoard, TRUE, (ArgIniType) TRUE },
  { "autoraise", ArgTrue, (void *) &appData.autoRaiseBoard, FALSE, INVALID },
  { "xautoraise", ArgFalse, (void *) &appData.autoRaiseBoard, FALSE, INVALID },
  { "-autoraise", ArgFalse, (void *) &appData.autoRaiseBoard, FALSE, INVALID },
  { "alwaysPromoteToQueen", ArgBoolean, (void *) &appData.alwaysPromoteToQueen, TRUE, (ArgIniType) FALSE },
  { "queen", ArgTrue, (void *) &appData.alwaysPromoteToQueen, FALSE, INVALID },
  { "xqueen", ArgFalse, (void *) &appData.alwaysPromoteToQueen, FALSE, INVALID },
  { "-queen", ArgFalse, (void *) &appData.alwaysPromoteToQueen, FALSE, INVALID },
  { "oldSaveStyle", ArgBoolean, (void *) &appData.oldSaveStyle, TRUE, (ArgIniType) FALSE },
  { "oldsave", ArgTrue, (void *) &appData.oldSaveStyle, FALSE, INVALID },
  { "xoldsave", ArgFalse, (void *) &appData.oldSaveStyle, FALSE, INVALID },
  { "-oldsave", ArgFalse, (void *) &appData.oldSaveStyle, FALSE, INVALID },
  { "quietPlay", ArgBoolean, (void *) &appData.quietPlay, TRUE, (ArgIniType) FALSE },
  { "quiet", ArgTrue, (void *) &appData.quietPlay, FALSE, INVALID },
  { "xquiet", ArgFalse, (void *) &appData.quietPlay, FALSE, INVALID },
  { "-quiet", ArgFalse, (void *) &appData.quietPlay, FALSE, INVALID },
  { "getMoveList", ArgBoolean, (void *) &appData.getMoveList, TRUE, (ArgIniType) TRUE },
  { "moves", ArgTrue, (void *) &appData.getMoveList, FALSE, INVALID },
  { "xmoves", ArgFalse, (void *) &appData.getMoveList, FALSE, INVALID },
  { "-moves", ArgFalse, (void *) &appData.getMoveList, FALSE, INVALID },
  { "testLegality", ArgBoolean, (void *) &appData.testLegality, TRUE, (ArgIniType) TRUE },
  { "legal", ArgTrue, (void *) &appData.testLegality, FALSE, INVALID },
  { "xlegal", ArgFalse, (void *) &appData.testLegality, FALSE, INVALID },
  { "-legal", ArgFalse, (void *) &appData.testLegality, FALSE, INVALID },
  { "premove", ArgBoolean, (void *) &appData.premove, TRUE, (ArgIniType) TRUE },
  { "pre", ArgTrue, (void *) &appData.premove, FALSE, INVALID },
  { "xpre", ArgFalse, (void *) &appData.premove, FALSE, INVALID },
  { "-pre", ArgFalse, (void *) &appData.premove, FALSE, INVALID },
  { "premoveWhite", ArgBoolean, (void *) &appData.premoveWhite, TRUE, (ArgIniType) FALSE },
  { "prewhite", ArgTrue, (void *) &appData.premoveWhite, FALSE, INVALID },
  { "xprewhite", ArgFalse, (void *) &appData.premoveWhite, FALSE, INVALID },
  { "-prewhite", ArgFalse, (void *) &appData.premoveWhite, FALSE, INVALID },
  { "premoveWhiteText", ArgString, (void *) &appData.premoveWhiteText, TRUE, (ArgIniType) "" },
  { "premoveBlack", ArgBoolean, (void *) &appData.premoveBlack, TRUE, (ArgIniType) FALSE },
  { "preblack", ArgTrue, (void *) &appData.premoveBlack, FALSE, INVALID },
  { "xpreblack", ArgFalse, (void *) &appData.premoveBlack, FALSE, INVALID },
  { "-preblack", ArgFalse, (void *) &appData.premoveBlack, FALSE, INVALID },
  { "premoveBlackText", ArgString, (void *) &appData.premoveBlackText, TRUE, (ArgIniType) "" },
  { "icsAlarm", ArgBoolean, (void *) &appData.icsAlarm, TRUE, (ArgIniType) TRUE},
  { "alarm", ArgTrue, (void *) &appData.icsAlarm, FALSE},
  { "xalarm", ArgFalse, (void *) &appData.icsAlarm, FALSE},
  { "-alarm", ArgFalse, (void *) &appData.icsAlarm, FALSE},
  { "icsAlarmTime", ArgInt, (void *) &appData.icsAlarmTime, TRUE, (ArgIniType) 5000},
  { "localLineEditing", ArgBoolean, (void *) &appData.localLineEditing, FALSE, (ArgIniType) TRUE},
  { "edit", ArgTrue, (void *) &appData.localLineEditing, FALSE, INVALID },
  { "xedit", ArgFalse, (void *) &appData.localLineEditing, FALSE, INVALID },
  { "-edit", ArgFalse, (void *) &appData.localLineEditing, FALSE, INVALID },
  { "animateMoving", ArgBoolean, (void *) &appData.animate, TRUE, (ArgIniType) TRUE },
  { "animate", ArgTrue, (void *) &appData.animate, FALSE, INVALID },
  { "xanimate", ArgFalse, (void *) &appData.animate, FALSE, INVALID },
  { "-animate", ArgFalse, (void *) &appData.animate, FALSE, INVALID },
  { "animateSpeed", ArgInt, (void *) &appData.animSpeed, TRUE, (ArgIniType) 10 },
  { "animateDragging", ArgBoolean, (void *) &appData.animateDragging, TRUE, (ArgIniType) TRUE },
  { "drag", ArgTrue, (void *) &appData.animateDragging, FALSE, INVALID },
  { "xdrag", ArgFalse, (void *) &appData.animateDragging, FALSE, INVALID },
  { "-drag", ArgFalse, (void *) &appData.animateDragging, FALSE, INVALID },
  { "blindfold", ArgBoolean, (void *) &appData.blindfold, TRUE, (ArgIniType) FALSE },
  { "blind", ArgTrue, (void *) &appData.blindfold, FALSE, INVALID },
  { "xblind", ArgFalse, (void *) &appData.blindfold, FALSE, INVALID },
  { "-blind", ArgFalse, (void *) &appData.blindfold, FALSE, INVALID },
  { "highlightLastMove", ArgBoolean,
    (void *) &appData.highlightLastMove, TRUE, (ArgIniType) TRUE },
  { "highlight", ArgTrue, (void *) &appData.highlightLastMove, FALSE, INVALID },
  { "xhighlight", ArgFalse, (void *) &appData.highlightLastMove, FALSE, INVALID },
  { "-highlight", ArgFalse, (void *) &appData.highlightLastMove, FALSE, INVALID },
  { "highlightDragging", ArgBoolean,
    (void *) &appData.highlightDragging, !XBOARD, (ArgIniType) TRUE },
  { "highdrag", ArgTrue, (void *) &appData.highlightDragging, FALSE, INVALID },
  { "xhighdrag", ArgFalse, (void *) &appData.highlightDragging, FALSE, INVALID },
  { "-highdrag", ArgFalse, (void *) &appData.highlightDragging, FALSE, INVALID },
  { "colorizeMessages", ArgBoolean, (void *) &appData.colorize, TRUE, (ArgIniType) TRUE },
  { "colorize", ArgTrue, (void *) &appData.colorize, FALSE, INVALID },
  { "xcolorize", ArgFalse, (void *) &appData.colorize, FALSE, INVALID },
  { "-colorize", ArgFalse, (void *) &appData.colorize, FALSE, INVALID },
  { "colorShout", ArgAttribs, (void *) ColorShout, TRUE, INVALID },
  { "colorSShout", ArgAttribs, (void *) ColorSShout, TRUE, INVALID },
  { "colorCShout", ArgAttribs, (void *) ColorSShout, FALSE, INVALID }, // for XB
  { "colorChannel1", ArgAttribs, (void *) ColorChannel1, TRUE, INVALID },
  { "colorChannel", ArgAttribs, (void *) ColorChannel, TRUE, INVALID },
  { "colorKibitz", ArgAttribs, (void *) ColorKibitz, TRUE, INVALID },
  { "colorTell", ArgAttribs, (void *) ColorTell, TRUE, INVALID },
  { "colorChallenge", ArgAttribs, (void *) ColorChallenge, TRUE, INVALID },
  { "colorRequest", ArgAttribs, (void *) ColorRequest, TRUE, INVALID },
  { "colorSeek", ArgAttribs, (void *) ColorSeek, TRUE, INVALID },
  { "colorNormal", ArgAttribs, (void *) ColorNormal, TRUE, INVALID },
  { "colorBackground", ArgColor, (void *) 7, TRUE, COLOR_BKGD },
  { "soundShout", ArgFilename, (void *) &appData.soundShout, TRUE, (ArgIniType) "" },
  { "soundSShout", ArgFilename, (void *) &appData.soundSShout, TRUE, (ArgIniType) "" },
  { "soundCShout", ArgFilename, (void *) &appData.soundSShout, FALSE, (ArgIniType) "" }, // for XB
  { "soundChannel1", ArgFilename, (void *) &appData.soundChannel1, TRUE, (ArgIniType) "" },
  { "soundChannel", ArgFilename, (void *) &appData.soundChannel, TRUE, (ArgIniType) "" },
  { "soundKibitz", ArgFilename, (void *) &appData.soundKibitz, TRUE, (ArgIniType) "" },
  { "soundTell", ArgFilename, (void *) &appData.soundTell, TRUE, (ArgIniType) "" },
  { "soundChallenge", ArgFilename, (void *) &appData.soundChallenge, TRUE, (ArgIniType) "" },
  { "soundRequest", ArgFilename, (void *) &appData.soundRequest, TRUE, (ArgIniType) "" },
  { "soundSeek", ArgFilename, (void *) &appData.soundSeek, TRUE, (ArgIniType) "" },
  { "soundMove", ArgFilename, (void *) &appData.soundMove, TRUE, (ArgIniType) "" },
  { "soundBell", ArgFilename, (void *) &appData.soundBell, TRUE, (ArgIniType) SOUND_BELL },
  { "soundRoar", ArgFilename, (void *) &appData.soundRoar, TRUE, (ArgIniType) "" },
  { "soundIcsWin", ArgFilename, (void *) &appData.soundIcsWin, TRUE, (ArgIniType) "" },
  { "soundIcsLoss", ArgFilename, (void *) &appData.soundIcsLoss, TRUE, (ArgIniType) "" },
  { "soundIcsDraw", ArgFilename, (void *) &appData.soundIcsDraw, TRUE, (ArgIniType) "" },
  { "soundIcsUnfinished", ArgFilename, (void *) &appData.soundIcsUnfinished, TRUE, (ArgIniType) "" },
  { "soundIcsAlarm", ArgFilename, (void *) &appData.soundIcsAlarm, TRUE, (ArgIniType) "" },
  { "disguisePromotedPieces", ArgBoolean, (void *) &appData.disguise, TRUE, (ArgIniType) TRUE },
  { "reuseFirst", ArgBoolean, (void *) &appData.reuseFirst, FALSE, (ArgIniType) TRUE },
  { "reuse", ArgTrue, (void *) &appData.reuseFirst, FALSE, INVALID },
  { "xreuse", ArgFalse, (void *) &appData.reuseFirst, FALSE, INVALID },
  { "-reuse", ArgFalse, (void *) &appData.reuseFirst, FALSE, INVALID },
  { "reuseChessPrograms", ArgBoolean,
    (void *) &appData.reuseFirst, FALSE, INVALID }, /* backward compat only */
  { "reuseSecond", ArgBoolean, (void *) &appData.reuseSecond, FALSE, (ArgIniType) TRUE },
  { "reuse2", ArgTrue, (void *) &appData.reuseSecond, FALSE, INVALID },
  { "xreuse2", ArgFalse, (void *) &appData.reuseSecond, FALSE, INVALID },
  { "-reuse2", ArgFalse, (void *) &appData.reuseSecond, FALSE, INVALID },
  { "comPortSettings", ArgCommSettings, (void *) /*&dcb*/ 0, TRUE, INVALID },
  { "settingsFile", ArgSettingsFilename, (void *) &settingsFileName, FALSE, (ArgIniType) SETTINGS_FILE },
  { "ini", ArgSettingsFilename, (void *) &settingsFileName, FALSE, INVALID },
  { "at", ArgSettingsFilename, (void *) NULL, FALSE, INVALID },
  { "opt", ArgSettingsFilename, (void *) NULL, FALSE, INVALID },
  { "saveSettingsFile", ArgFilename, (void *) &settingsFileName, FALSE, INVALID },
  { "backupSettingsFile", ArgBackupSettingsFile, (void *) &settingsFileName, FALSE, INVALID },
  { "saveSettingsOnExit", ArgBoolean, (void *) &saveSettingsOnExit, TRUE, (ArgIniType) TRUE },
  { "chessProgram", ArgBoolean, (void *) &chessProgram, FALSE, (ArgIniType) FALSE },
  { "cp", ArgTrue, (void *) &chessProgram, FALSE, INVALID },
  { "xcp", ArgFalse, (void *) &chessProgram, FALSE, INVALID },
  { "-cp", ArgFalse, (void *) &chessProgram, FALSE, INVALID },
  { "icsMenu", ArgString, (void *) &icsTextMenuString, TRUE, (ArgIniType) ICS_TEXT_MENU_DEFAULT },
  { "icsNames", ArgString, (void *) &icsNames, TRUE, (ArgIniType) ICS_NAMES },
  { "singleEngineList", ArgBoolean, (void *) &singleList, !XBOARD, (ArgIniType) FALSE },
  { "recentEngines", ArgInt, (void *) &appData.recentEngines, TRUE, (ArgIniType) 6 },
  { "recentEngineList", ArgString, (void *) &appData.recentEngineList, TRUE, (ArgIniType) "" },
  { "firstChessProgramNames", ArgString, (void *) &firstChessProgramNames,
    TRUE, (ArgIniType) FCP_NAMES },
  { "secondChessProgramNames", ArgString, (void *) &secondChessProgramNames,
    !XBOARD, (ArgIniType) SCP_NAMES },
  { "themeNames", ArgString, (void *) &appData.themeNames, TRUE, (ArgIniType) "native -upf false -ub false -ubt false -pid \"\"\n" },
  { "addMasterOption", ArgMaster, NULL, FALSE, INVALID },
  { "installEngine", ArgInstall, (void *) &firstChessProgramNames, FALSE, (ArgIniType) "" },
  { "initialMode", ArgString, (void *) &appData.initialMode, FALSE, (ArgIniType) "" },
  { "mode", ArgString, (void *) &appData.initialMode, FALSE, INVALID },
  { "variant", ArgString, (void *) &appData.variant, FALSE, (ArgIniType) "normal" },
  { "firstProtocolVersion", ArgInt, (void *) &appData.firstProtocolVersion, FALSE, (ArgIniType) PROTOVER },
  { "secondProtocolVersion", ArgInt, (void *) &appData.secondProtocolVersion,FALSE, (ArgIniType) PROTOVER },
  { "showButtonBar", ArgBoolean, (void *) &appData.showButtonBar, TRUE, (ArgIniType) TRUE },
  { "buttons", ArgTrue, (void *) &appData.showButtonBar, FALSE, INVALID },
  { "xbuttons", ArgFalse, (void *) &appData.showButtonBar, FALSE, INVALID },
  { "-buttons", ArgFalse, (void *) &appData.showButtonBar, FALSE, INVALID },

  /* [AS] New features */
  { "firstScoreAbs", ArgBoolean, (void *) &appData.firstScoreIsAbsolute, FALSE, (ArgIniType) FALSE },
  { "secondScoreAbs", ArgBoolean, (void *) &appData.secondScoreIsAbsolute, FALSE, (ArgIniType) FALSE },
  { "pgnExtendedInfo", ArgBoolean, (void *) &appData.saveExtendedInfoInPGN, TRUE, (ArgIniType) FALSE },
  { "hideThinkingFromHuman", ArgBoolean, (void *) &appData.hideThinkingFromHuman, TRUE, (ArgIniType) FALSE },
  { "liteBackTextureFile", ArgFilename, (void *) &appData.liteBackTextureFile, TRUE, (ArgIniType) "" },
  { "lbtf", ArgFilename, (void *) &appData.liteBackTextureFile, FALSE, INVALID },
  { "darkBackTextureFile", ArgFilename, (void *) &appData.darkBackTextureFile, TRUE, (ArgIniType) "" },
  { "dbtf", ArgFilename, (void *) &appData.darkBackTextureFile, FALSE, INVALID },
  { "liteBackTextureMode", ArgInt, (void *) &appData.liteBackTextureMode, TRUE, (ArgIniType) BACK_TEXTURE_MODE_PLAIN },
  { "lbtm", ArgInt, (void *) &appData.liteBackTextureMode, FALSE, INVALID },
  { "darkBackTextureMode", ArgInt, (void *) &appData.darkBackTextureMode, TRUE, (ArgIniType) BACK_TEXTURE_MODE_PLAIN },
  { "dbtm", ArgInt, (void *) &appData.darkBackTextureMode, FALSE, INVALID },
  { "renderPiecesWithFont", ArgString, (void *) &appData.renderPiecesWithFont, TRUE, (ArgIniType) "" },
  { "pf", ArgString, (void *) &appData.renderPiecesWithFont, FALSE, INVALID },
  { "fontPieceToCharTable", ArgString, (void *) &appData.fontToPieceTable, TRUE, (ArgIniType) "" },
  { "fptc", ArgString, (void *) &appData.fontToPieceTable, FALSE, INVALID },
  { "fontPieceBackColorWhite", ArgColor, (void *) 8, TRUE, (ArgIniType) WHITE_PIECE_COLOR },
  { "fontPieceForeColorWhite", ArgColor, (void *) 9, TRUE, (ArgIniType) WHITE_PIECE_COLOR },
  { "fontPieceBackColorBlack", ArgColor, (void *) 10, TRUE, (ArgIniType) BLACK_PIECE_COLOR },
  { "fontPieceForeColorBlack", ArgColor, (void *) 11, TRUE, (ArgIniType) BLACK_PIECE_COLOR },
  { "fpfcw", ArgColor, (void *) 9, FALSE, INVALID },
  { "fpbcb", ArgColor, (void *) 10, FALSE, INVALID },
  { "fontPieceSize", ArgInt, (void *) &appData.fontPieceSize, TRUE, (ArgIniType) 80 },
  { "overrideLineGap", ArgInt, (void *) &appData.overrideLineGap, TRUE, (ArgIniType) 1 },
  { "adjudicateLossThreshold", ArgInt, (void *) &appData.adjudicateLossThreshold, TRUE, (ArgIniType) 0 },
  { "delayBeforeQuit", ArgInt, (void *) &appData.delayBeforeQuit, TRUE, (ArgIniType) 0 },
  { "delayAfterQuit", ArgInt, (void *) &appData.delayAfterQuit, TRUE, (ArgIniType) 0 },
  { "nameOfDebugFile", ArgFilename, (void *) &appData.nameOfDebugFile, FALSE, (ArgIniType) DEBUG_FILE },
  { "debugfile", ArgFilename, (void *) &appData.nameOfDebugFile, FALSE, INVALID },
  { "pgnEventHeader", ArgString, (void *) &appData.pgnEventHeader, TRUE, (ArgIniType) "Computer Chess Game" },
  { "defaultFrcPosition", ArgInt, (void *) &appData.defaultFrcPosition, TRUE, (ArgIniType) -1 },
  { "shuffleOpenings", ArgTrue, (void *) &shuffleOpenings, FALSE, INVALID },
  { "fischerCastling", ArgTrue, (void *) &appData.fischerCastling, FALSE, INVALID },
  { "gameListTags", ArgString, (void *) &appData.gameListTags, TRUE, (ArgIniType) GLT_DEFAULT_TAGS },
  { "saveOutOfBookInfo", ArgBoolean, (void *) &appData.saveOutOfBookInfo, TRUE, (ArgIniType) TRUE },
  { "showEvalInMoveHistory", ArgBoolean, (void *) &appData.showEvalInMoveHistory, TRUE, (ArgIniType) TRUE },
  { "evalHistColorWhite", ArgColor, (void *) 12, TRUE, (ArgIniType) "#FFFFB0" },
  { "evalHistColorBlack", ArgColor, (void *) 13, TRUE, (ArgIniType) "#AD5D3D" },
  { "highlightMoveWithArrow", ArgBoolean, (void *) &appData.highlightMoveWithArrow, TRUE, (ArgIniType) FALSE },
  { "highlightArrowColor", ArgColor, (void *) 14, TRUE, (ArgIniType) "#FFFF80" },
  { "stickyWindows", ArgBoolean, (void *) &appData.useStickyWindows, TRUE, (ArgIniType) TRUE },
  { "adjudicateDrawMoves", ArgInt, (void *) &appData.adjudicateDrawMoves, TRUE, (ArgIniType) 0 },
  { "autoDisplayComment", ArgBoolean, (void *) &appData.autoDisplayComment, TRUE, (ArgIniType) TRUE },
  { "autoDisplayTags", ArgBoolean, (void *) &appData.autoDisplayTags, TRUE, (ArgIniType) TRUE },
  { "firstIsUCI", ArgBoolean, (void *) &appData.firstIsUCI, FALSE, (ArgIniType) FALSE },
  { "fUCI", ArgTrue, (void *) &appData.firstIsUCI, FALSE, INVALID },
  { "firstUCI", ArgTrue, (void *) &appData.firstIsUCI, FALSE, INVALID },
  { "secondIsUCI", ArgBoolean, (void *) &appData.secondIsUCI, FALSE, (ArgIniType) FALSE },
  { "secondUCI", ArgTrue, (void *) &appData.secondIsUCI, FALSE, INVALID },
  { "sUCI", ArgTrue, (void *) &appData.secondIsUCI, FALSE, INVALID },
  { "fUCCI", ArgTwo, (void *) &appData.firstIsUCI, FALSE, INVALID },
  { "sUCCI", ArgTwo, (void *) &appData.secondIsUCI, FALSE, INVALID },
  { "fUSI", ArgTwo, (void *) &appData.firstIsUCI, FALSE, INVALID },
  { "sUSI", ArgTwo, (void *) &appData.secondIsUCI, FALSE, INVALID },
  { "firstHasOwnBookUCI", ArgBoolean, (void *) &appData.firstHasOwnBookUCI, FALSE, (ArgIniType) TRUE },
  { "fNoOwnBookUCI", ArgFalse, (void *) &appData.firstHasOwnBookUCI, FALSE, INVALID },
  { "firstXBook", ArgFalse, (void *) &appData.firstHasOwnBookUCI, FALSE, INVALID },
  { "secondHasOwnBookUCI", ArgBoolean, (void *) &appData.secondHasOwnBookUCI, FALSE, (ArgIniType) TRUE },
  { "sNoOwnBookUCI", ArgFalse, (void *) &appData.secondHasOwnBookUCI, FALSE, INVALID },
  { "secondXBook", ArgFalse, (void *) &appData.secondHasOwnBookUCI, FALSE, INVALID },
  { "adapterCommand", ArgFilename, (void *) &appData.adapterCommand, TRUE, (ArgIniType) "polyglot -noini -ec \"%fcp\" -ed \"%fd\"" },
  { "uxiAdapter", ArgFilename, (void *) &appData.ucciAdapter, TRUE, (ArgIniType) "" },
  { "polyglotDir", ArgFilename, (void *) &appData.polyglotDir, TRUE, (ArgIniType) "" },
  { "usePolyglotBook", ArgBoolean, (void *) &appData.usePolyglotBook, TRUE, (ArgIniType) FALSE },
  { "polyglotBook", ArgFilename, (void *) &appData.polyglotBook, TRUE, (ArgIniType) "" },
  { "bookDepth", ArgInt, (void *) &appData.bookDepth, TRUE, (ArgIniType) 12 },
  { "bookVariation", ArgInt, (void *) &appData.bookStrength, TRUE, (ArgIniType) 50 },
  { "discourageOwnBooks", ArgBoolean, (void *) &appData.defNoBook, TRUE, (ArgIniType) FALSE },
  { "mcBookMode", ArgTrue, (void *) &mcMode, FALSE, (ArgIniType) FALSE },
  { "defaultHashSize", ArgInt, (void *) &appData.defaultHashSize, TRUE, (ArgIniType) 64 },
  { "defaultCacheSizeEGTB", ArgInt, (void *) &appData.defaultCacheSizeEGTB, TRUE, (ArgIniType) 4 },
  { "defaultPathEGTB", ArgFilename, (void *) &appData.defaultPathEGTB, TRUE, (ArgIniType) "c:\\egtb" },
  { "language", ArgFilename, (void *) &appData.language, TRUE, (ArgIniType) "" },
  { "userFileDirectory", ArgFilename, (void *) &homeDir, FALSE, (ArgIniType) installDir },
  { "usePieceFont", ArgBoolean, (void *) &appData.useFont, TRUE, (ArgIniType) FALSE },
  { "upf", ArgBoolean, (void *) &appData.useFont, FALSE, INVALID },
  { "useBoardTexture", ArgBoolean, (void *) &appData.useBitmaps, TRUE, (ArgIniType) FALSE },
  { "ubt", ArgBoolean, (void *) &appData.useBitmaps, FALSE, INVALID },
  { "useBorder", ArgBoolean, (void *) &appData.useBorder, TRUE, (ArgIniType) FALSE },
  { "ub", ArgBoolean, (void *) &appData.useBorder, FALSE, INVALID },
  { "border", ArgFilename, (void *) &appData.border, TRUE, (ArgIniType) "" },
  { "finger", ArgFilename, (void *) &appData.finger, FALSE, (ArgIniType) "" },
  { "inscriptions", ArgString, (void *) &appData.inscriptions, XBOARD, (ArgIniType) "" },
  { "autoInstall", ArgString, (void *) &appData.autoInstall, XBOARD, (ArgIniType) "" },
  { "fixedSize", ArgBoolean, (void *) &appData.fixedSize, TRUE, (ArgIniType) FALSE },

  // [HGM] tournament options
  { "tourneyFile", ArgFilename, (void *) &appData.tourneyFile, FALSE, (ArgIniType) "" },
  { "tf", ArgFilename, (void *) &appData.tourneyFile, FALSE, INVALID },
  { "participants", ArgString, (void *) &appData.participants, FALSE, (ArgIniType) "" },
  { "tourneyType", ArgInt, (void *) &appData.tourneyType, FALSE, (ArgIniType) 0 },
  { "tt", ArgInt, (void *) &appData.tourneyType, FALSE, INVALID },
  { "tourneyCycles", ArgInt, (void *) &appData.tourneyCycles, FALSE, (ArgIniType) 1 },
  { "cy", ArgInt, (void *) &appData.tourneyCycles, FALSE, INVALID },
  { "results", ArgString, (void *) &appData.results, FALSE, (ArgIniType) "" },
  { "syncAfterRound", ArgBoolean, (void *) &appData.roundSync, FALSE, (ArgIniType) FALSE },
  { "syncAfterCycle", ArgBoolean, (void *) &appData.cycleSync, FALSE, (ArgIniType) TRUE },
  { "seedBase", ArgInt, (void *) &appData.seedBase, FALSE, (ArgIniType) 1 },
  { "pgnNumberTag", ArgBoolean, (void *) &appData.numberTag, TRUE, (ArgIniType) FALSE },
  { "afterGame", ArgString, (void *) &appData.afterGame, FALSE, INVALID },
  { "afterTourney", ArgString, (void *) &appData.afterTourney, FALSE, INVALID },

  /* [HGM] board-size, adjudication and misc. options */
  { "oneClickMove", ArgBoolean, (void *) &appData.oneClick, TRUE, (ArgIniType) FALSE },
  { "boardWidth", ArgInt, (void *) &appData.NrFiles, FALSE, (ArgIniType) -1 },
  { "boardHeight", ArgInt, (void *) &appData.NrRanks, FALSE, (ArgIniType) -1 },
  { "holdingsSize", ArgInt, (void *) &appData.holdingsSize, FALSE, (ArgIniType) -1 },
  { "defaultMatchGames", ArgInt, (void *) &appData.defaultMatchGames, TRUE, (ArgIniType) 10 },
  { "matchPause", ArgInt, (void *) &appData.matchPause, TRUE, (ArgIniType) 10000 },
  { "pieceToCharTable", ArgString, (void *) &appData.pieceToCharTable, FALSE, INVALID },
  { "pieceNickNames", ArgString, (void *) &appData.pieceNickNames, FALSE, INVALID },
  { "colorNickNames", ArgString, (void *) &appData.colorNickNames, FALSE, INVALID },
  { "flipBlack", ArgBoolean, (void *) &appData.upsideDown, FALSE, (ArgIniType) FALSE },
  { "allWhite", ArgBoolean, (void *) &appData.allWhite, FALSE, (ArgIniType) FALSE },
  { "alphaRank", ArgBoolean, (void *) &appData.alphaRank, FALSE, (ArgIniType) FALSE },
  { "firstAlphaRank", ArgBoolean, (void *) &first.alphaRank, FALSE, (ArgIniType) FALSE },
  { "secondAlphaRank", ArgBoolean, (void *) &second.alphaRank, FALSE, (ArgIniType) FALSE },
  { "testClaims", ArgBoolean, (void *) &appData.testClaims, TRUE, (ArgIniType) FALSE },
  { "checkMates", ArgBoolean, (void *) &appData.checkMates, TRUE, (ArgIniType) FALSE },
  { "materialDraws", ArgBoolean, (void *) &appData.materialDraws, TRUE, (ArgIniType) FALSE },
  { "trivialDraws", ArgBoolean, (void *) &appData.trivialDraws, TRUE, (ArgIniType) FALSE },
  { "ruleMoves", ArgInt, (void *) &appData.ruleMoves, TRUE, (ArgIniType) 51 },
  { "repeatsToDraw", ArgInt, (void *) &appData.drawRepeats, TRUE, (ArgIniType) 6 },
  { "backgroundObserve", ArgBoolean, (void *) &appData.bgObserve, TRUE, (ArgIniType) FALSE },
  { "dualBoard", ArgBoolean, (void *) &appData.dualBoard, TRUE, (ArgIniType) FALSE },
  { "autoKibitz", ArgTrue, (void *) &appData.autoKibitz, FALSE, INVALID },
  { "engineDebugOutput", ArgInt, (void *) &appData.engineComments, FALSE, (ArgIniType) 1 },
  { "userName", ArgString, (void *) &appData.userName, FALSE, INVALID },
  { "rewindIndex", ArgInt, (void *) &appData.rewindIndex, FALSE, INVALID },
  { "sameColorGames", ArgInt, (void *) &appData.sameColorGames, FALSE, INVALID },
  { "smpCores", ArgInt, (void *) &appData.smpCores, TRUE, (ArgIniType) 1 },
  { "egtFormats", ArgString, (void *) &appData.egtFormats, TRUE, (ArgIniType) "" },
  { "niceEngines", ArgInt, (void *) &appData.niceEngines, TRUE, INVALID },
  { "logoSize", ArgInt, (void *) &appData.logoSize, XBOARD, INVALID },
  { "logoDir", ArgFilename, (void *) &appData.logoDir, XBOARD, (ArgIniType) "." },
  { "firstLogo", ArgFilename, (void *) &appData.firstLogo, FALSE, (ArgIniType) "" },
  { "secondLogo", ArgFilename, (void *) &appData.secondLogo, FALSE, (ArgIniType) "" },
  { "autoLogo", ArgBoolean, (void *) &appData.autoLogo, TRUE, INVALID },
  { "firstOptions", ArgString, (void *) &appData.firstOptions, FALSE, (ArgIniType) "" },
  { "secondOptions", ArgString, (void *) &appData.secondOptions, FALSE, (ArgIniType) "" },
  { "firstFeatures", ArgString, (void *) &appData.features[0], FALSE, (ArgIniType) "" },
  { "secondFeatures", ArgString, (void *) &appData.features[1], FALSE, (ArgIniType) "" },
  { "featureDefaults", ArgString, (void *) &appData.featureDefaults, TRUE, (ArgIniType) "" },
  { "firstNeedsNoncompliantFEN", ArgString, (void *) &appData.fenOverride1, FALSE, (ArgIniType) NULL },
  { "secondNeedsNoncompliantFEN", ArgString, (void *) &appData.fenOverride2, FALSE, (ArgIniType) NULL },
  { "keepAlive", ArgInt, (void *) &appData.keepAlive, FALSE, INVALID },
  { "icstype", ArgInt, (void *) &ics_type, FALSE, INVALID },
  { "forceIllegalMoves", ArgTrue, (void *) &appData.forceIllegal, FALSE, INVALID },
  { "showTargetSquares", ArgBoolean, (void *) &appData.markers, TRUE, (ArgIniType) FALSE },
  { "firstPgnName", ArgString, (void *) &appData.pgnName[0], FALSE, (ArgIniType) "" },
  { "fn", ArgString, (void *) &appData.pgnName[0], FALSE, INVALID },
  { "secondPgnName", ArgString, (void *) &appData.pgnName[1], FALSE, (ArgIniType) "" },
  { "sn", ArgString, (void *) &appData.pgnName[1], FALSE, INVALID },
  { "absoluteAnalysisScores", ArgBoolean, (void *) &appData.whitePOV, TRUE, FALSE },
  { "scoreWhite", ArgBoolean, (void *) &appData.scoreWhite, TRUE, FALSE },
  { "evalZoom", ArgInt, (void *) &appData.zoom, TRUE, (ArgIniType) 1 },
  { "evalThreshold", ArgInt, (void *) &appData.evalThreshold, TRUE, (ArgIniType) 25 },
  { "firstPseudo", ArgTrue, (void *) &appData.pseudo[0], FALSE, FALSE },
  { "secondPseudo", ArgTrue, (void *) &appData.pseudo[1], FALSE, FALSE },
  { "fSAN", ArgTrue, (void *) &appData.pvSAN[0], FALSE, FALSE },
  { "sSAN", ArgTrue, (void *) &appData.pvSAN[1], FALSE, FALSE },
  { "pairingEngine", ArgFilename, (void *) &appData.pairingEngine, TRUE, "" },
  { "defaultTourneyName", ArgFilename, (void *) &appData.defName, TRUE, "" },
  { "eloThresholdAny", ArgInt, (void *) &appData.eloThreshold1, FALSE, (ArgIniType) 0 },
  { "eloThresholdBoth", ArgInt, (void *) &appData.eloThreshold2, FALSE, (ArgIniType) 0 },
  { "dateThreshold", ArgInt, (void *) &appData.dateThreshold, FALSE, (ArgIniType) 0 },
  { "searchMode", ArgInt, (void *) &appData.searchMode, FALSE, (ArgIniType) 1 },
  { "stretch", ArgInt, (void *) &appData.stretch, FALSE, (ArgIniType) 1 },
  { "ignoreColors", ArgBoolean, (void *) &appData.ignoreColors, FALSE, FALSE },
  { "findMirrorImage", ArgBoolean, (void *) &appData.findMirror, FALSE, FALSE },
  { "viewer", ArgTrue, (void *) &appData.viewer, FALSE, FALSE },
  { "viewerOptions", ArgString, (void *) &appData.viewerOptions, TRUE, (ArgIniType) "-ncp -engineOutputUp false -saveSettingsOnExit false" },
  { "tourneyOptions", ArgString, (void *) &appData.tourneyOptions, TRUE, (ArgIniType) "-ncp -mm -saveSettingsOnExit false" },
  { "autoCopyPV", ArgBoolean, (void *) &appData.autoCopyPV, TRUE, FALSE },
  { "topLevel", ArgBoolean, (void *) &appData.topLevel, XBOARD, (ArgIniType) TOPLEVEL },
  { "dialogColor", ArgString, (void *) &appData.dialogColor, XBOARD, (ArgIniType) "" },
  { "buttonColor", ArgString, (void *) &appData.buttonColor, XBOARD, (ArgIniType) "" },
  { "firstDrawDepth", ArgInt, (void *) &appData.drawDepth[0], FALSE, (ArgIniType) 0 },
  { "secondDrawDepth", ArgInt, (void *) &appData.drawDepth[1], FALSE, (ArgIniType) 0 },
  { "memoHeaders", ArgBoolean, (void *) &appData.headers, TRUE, (ArgIniType) FALSE },

#if ZIPPY
  { "zippyTalk", ArgBoolean, (void *) &appData.zippyTalk, FALSE, (ArgIniType) ZIPPY_TALK },
  { "zt", ArgTrue, (void *) &appData.zippyTalk, FALSE, INVALID },
  { "xzt", ArgFalse, (void *) &appData.zippyTalk, FALSE, INVALID },
  { "-zt", ArgFalse, (void *) &appData.zippyTalk, FALSE, INVALID },
  { "zippyPlay", ArgBoolean, (void *) &appData.zippyPlay, FALSE, (ArgIniType) ZIPPY_PLAY },
  { "zp", ArgTrue, (void *) &appData.zippyPlay, FALSE, INVALID },
  { "xzp", ArgFalse, (void *) &appData.zippyPlay, FALSE, INVALID },
  { "-zp", ArgFalse, (void *) &appData.zippyPlay, FALSE, INVALID },
  { "zippyLines", ArgFilename, (void *) &appData.zippyLines, FALSE, (ArgIniType) ZIPPY_LINES },
  { "zippyPinhead", ArgString, (void *) &appData.zippyPinhead, FALSE, (ArgIniType) ZIPPY_PINHEAD },
  { "zippyPassword", ArgString, (void *) &appData.zippyPassword, FALSE, (ArgIniType) ZIPPY_PASSWORD },
  { "zippyPassword2", ArgString, (void *) &appData.zippyPassword2, FALSE, (ArgIniType) ZIPPY_PASSWORD2 },
  { "zippyWrongPassword", ArgString, (void *) &appData.zippyWrongPassword,
    FALSE, (ArgIniType) ZIPPY_WRONG_PASSWORD },
  { "zippyAcceptOnly", ArgString, (void *) &appData.zippyAcceptOnly, FALSE, (ArgIniType) ZIPPY_ACCEPT_ONLY },
  { "zippyUseI", ArgBoolean, (void *) &appData.zippyUseI, FALSE, (ArgIniType) ZIPPY_USE_I },
  { "zui", ArgTrue, (void *) &appData.zippyUseI, FALSE, INVALID },
  { "xzui", ArgFalse, (void *) &appData.zippyUseI, FALSE, INVALID },
  { "-zui", ArgFalse, (void *) &appData.zippyUseI, FALSE, INVALID },
  { "zippyBughouse", ArgInt, (void *) &appData.zippyBughouse, FALSE, (ArgIniType) ZIPPY_BUGHOUSE },
  { "zippyNoplayCrafty", ArgBoolean, (void *) &appData.zippyNoplayCrafty,
    FALSE, (ArgIniType) ZIPPY_NOPLAY_CRAFTY },
  { "znc", ArgTrue, (void *) &appData.zippyNoplayCrafty, FALSE, INVALID },
  { "xznc", ArgFalse, (void *) &appData.zippyNoplayCrafty, FALSE, INVALID },
  { "-znc", ArgFalse, (void *) &appData.zippyNoplayCrafty, FALSE, INVALID },
  { "zippyGameEnd", ArgString, (void *) &appData.zippyGameEnd, FALSE, (ArgIniType) ZIPPY_GAME_END },
  { "zippyGameStart", ArgString, (void *) &appData.zippyGameStart, FALSE, (ArgIniType) ZIPPY_GAME_START },
  { "zippyAdjourn", ArgBoolean, (void *) &appData.zippyAdjourn, FALSE, (ArgIniType) ZIPPY_ADJOURN },
  { "zadj", ArgTrue, (void *) &appData.zippyAdjourn, FALSE, INVALID },
  { "xzadj", ArgFalse, (void *) &appData.zippyAdjourn, FALSE, INVALID },
  { "-zadj", ArgFalse, (void *) &appData.zippyAdjourn, FALSE, INVALID },
  { "zippyAbort", ArgBoolean, (void *) &appData.zippyAbort, FALSE, (ArgIniType) ZIPPY_ABORT },
  { "zab", ArgTrue, (void *) &appData.zippyAbort, FALSE, INVALID },
  { "xzab", ArgFalse, (void *) &appData.zippyAbort, FALSE, INVALID },
  { "-zab", ArgFalse, (void *) &appData.zippyAbort, FALSE, INVALID },
  { "zippyVariants", ArgString, (void *) &appData.zippyVariants, FALSE, (ArgIniType) ZIPPY_VARIANTS },
  { "zippyMaxGames", ArgInt, (void *)&appData.zippyMaxGames, FALSE, (ArgIniType) ZIPPY_MAX_GAMES},
  { "zippyReplayTimeout", ArgInt, (void *)&appData.zippyReplayTimeout, FALSE, (ArgIniType) ZIPPY_REPLAY_TIMEOUT },
  { "zippyShortGame", ArgInt, (void *)&appData.zippyShortGame, FALSE, INVALID },
  /* Kludge to allow winboard.ini files from buggy 4.0.4 to be read: */
  { "zippyReplyTimeout", ArgInt, (void *)&junk, FALSE, INVALID },
#endif
  /* [HGM] options for broadcasting and time odds */
  { "chatBoxes", ArgString, (void *) &appData.chatBoxes, !XBOARD, (ArgIniType) NULL },
  { "serverMoves", ArgString, (void *) &appData.serverMovesName, FALSE, (ArgIniType) NULL },
  { "serverFile", ArgString, (void *) &appData.serverFileName, FALSE, (ArgIniType) NULL },
  { "suppressLoadMoves", ArgBoolean, (void *) &appData.suppressLoadMoves, FALSE, (ArgIniType) FALSE },
  { "serverPause", ArgInt, (void *) &appData.serverPause, FALSE, (ArgIniType) 15 },
  { "firstTimeOdds", ArgInt, (void *) &appData.firstTimeOdds, FALSE, (ArgIniType) 1 },
  { "secondTimeOdds", ArgInt, (void *) &appData.secondTimeOdds, FALSE, (ArgIniType) 1 },
  { "timeOddsMode", ArgInt, (void *) &appData.timeOddsMode, TRUE, INVALID },
  { "firstAccumulateTC", ArgInt, (void *) &appData.firstAccumulateTC, FALSE, (ArgIniType) 1 },
  { "secondAccumulateTC", ArgInt, (void *) &appData.secondAccumulateTC, FALSE, (ArgIniType) 1 },
  { "firstNPS", ArgInt, (void *) &appData.firstNPS, FALSE, (ArgIniType) -1 },
  { "secondNPS", ArgInt, (void *) &appData.secondNPS, FALSE, (ArgIniType) -1 },
  { "noGUI", ArgTrue, (void *) &appData.noGUI, FALSE, INVALID },
  { "keepLineBreaksICS", ArgBoolean, (void *) &appData.noJoin, TRUE, INVALID },
  { "wrapContinuationSequence", ArgString, (void *) &appData.wrapContSeq, FALSE, INVALID },
  { "useInternalWrap", ArgTrue, (void *) &appData.useInternalWrap, FALSE, INVALID }, /* noJoin usurps this if set */
  { "openCommand", ArgString, (void *) &appData.sysOpen, FALSE, "xdg-open" },

  // [HGM] placement: put all window layouts last in ini file, but man X,Y before all others
  { "minX", ArgZ, (void *) &minX, FALSE, INVALID }, // [HGM] placement: to make sure auxiliary windows can be placed
  { "minY", ArgZ, (void *) &minY, FALSE, INVALID },
  { "winWidth",  ArgInt, (void *) &wpMain.width,  TRUE, INVALID }, // [HGM] placement: dummies to remember right & bottom
  { "winHeight", ArgInt, (void *) &wpMain.height, TRUE, INVALID }, //       for attaching auxiliary windows to them
  { "x", ArgInt, (void *) &wpMain.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "y", ArgInt, (void *) &wpMain.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "icsUp", ArgBoolean, (void *) &wpConsole.visible, XBOARD, (ArgIniType) FALSE },
  { "icsX", ArgX,   (void *) &wpConsole.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "icsY", ArgY,   (void *) &wpConsole.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "icsW", ArgInt, (void *) &wpConsole.width, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "icsH", ArgInt, (void *) &wpConsole.height, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "commentX", ArgX,   (void *) &wpComment.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "commentY", ArgY,   (void *) &wpComment.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "commentW", ArgInt, (void *) &wpComment.width, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "commentH", ArgInt, (void *) &wpComment.height, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "tagsX", ArgX,   (void *) &wpTags.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "tagsY", ArgY,   (void *) &wpTags.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "tagsW", ArgInt, (void *) &wpTags.width, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "tagsH", ArgInt, (void *) &wpTags.height, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "gameListX", ArgX,   (void *) &wpGameList.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "gameListY", ArgY,   (void *) &wpGameList.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "gameListW", ArgInt, (void *) &wpGameList.width, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "gameListH", ArgInt, (void *) &wpGameList.height, TRUE, (ArgIniType) CW_USEDEFAULT },
#if XBOARD
  { "slaveX", ArgX,   (void *) &wpDualBoard.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "slaveY", ArgY,   (void *) &wpDualBoard.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "slaveW", ArgInt, (void *) &wpDualBoard.width, FALSE, (ArgIniType) CW_USEDEFAULT },
  { "slaveH", ArgInt, (void *) &wpDualBoard.height, FALSE, (ArgIniType) CW_USEDEFAULT },
#endif
  /* [AS] Layout stuff */
  { "moveHistoryUp", ArgBoolean, (void *) &wpMoveHistory.visible, TRUE, (ArgIniType) TRUE },
  { "moveHistoryX", ArgX,   (void *) &wpMoveHistory.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "moveHistoryY", ArgY,   (void *) &wpMoveHistory.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "moveHistoryW", ArgInt, (void *) &wpMoveHistory.width, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "moveHistoryH", ArgInt, (void *) &wpMoveHistory.height, TRUE, (ArgIniType) CW_USEDEFAULT },

  { "evalGraphUp", ArgBoolean, (void *) &wpEvalGraph.visible, TRUE, (ArgIniType) TRUE },
  { "evalGraphX", ArgX,   (void *) &wpEvalGraph.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "evalGraphY", ArgY,   (void *) &wpEvalGraph.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "evalGraphW", ArgInt, (void *) &wpEvalGraph.width, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "evalGraphH", ArgInt, (void *) &wpEvalGraph.height, TRUE, (ArgIniType) CW_USEDEFAULT },

  { "engineOutputUp", ArgBoolean, (void *) &wpEngineOutput.visible, TRUE, (ArgIniType) TRUE },
  { "engineOutputX", ArgX,   (void *) &wpEngineOutput.x, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "engineOutputY", ArgY,   (void *) &wpEngineOutput.y, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "engineOutputW", ArgInt, (void *) &wpEngineOutput.width, TRUE, (ArgIniType) CW_USEDEFAULT },
  { "engineOutputH", ArgInt, (void *) &wpEngineOutput.height, TRUE, (ArgIniType) CW_USEDEFAULT },

  { NULL, ArgNone, NULL, FALSE, INVALID }
};


/* Kludge for indirection files on command line */
char* lastIndirectionFilename;
ArgDescriptor argDescriptorIndirection =
{ "", ArgSettingsFilename, (void *) NULL, FALSE };

void
ExitArgError(char *msg, char *badArg, Boolean quit)
{
  char buf[MSG_SIZ];
  int len;

  len = snprintf(buf, MSG_SIZ, msg, badArg);
  if( (len >= MSG_SIZ) && appData.debugMode )
    fprintf(debugFP, "ExitArgError: buffer truncated. Input: msg=%s badArg=%s\n", msg, badArg);

  if(!quit) { printf(_("%s in settings file\n"), buf); return; } // DisplayError does not work yet at this stage...
  DisplayFatalError(buf, 0, 2);
  exit(2);
}

void
AppendToSettingsFile (char *line)
{
  char buf[MSG_SIZ];
  FILE *f;
  int c;
  if(f = fopen(SETTINGS_FILE, "r")) {
    do {
      int i = 0;
      while((buf[i] = c = fgetc(f)) != '\n' && c != EOF) if(i < MSG_SIZ-1) i++;
      buf[i] = NULLCHAR;
      if(!strcmp(line, buf)) return; // line occurs
    } while(c != EOF);
    // line did not occur; add it
    fclose(f);
    if(f = fopen(SETTINGS_FILE, "a")) {
      TimeMark now;
      GetTimeMark(&now);
      fprintf(f, "-date %10lu\n%s\n", now.sec, line);
      fclose(f);
    }
  }
}

int
ValidateInt(char *s)
{
  char *p = s;
  if(*p == '-' || *p == '+') p++;
  while(*p) if(!isdigit(*p++)) ExitArgError(_("Bad integer value %s"), s, TRUE);
  return atoi(s);
}

char
StringGet(void *getClosure)
{
  char **p = (char **) getClosure;
  return *((*p)++);
}

char
FileGet(void *getClosure)
{
  int c;
  FILE* f = (FILE*) getClosure;

  c = getc(f);
  if (c == '\r') c = getc(f); // work around DOS format files by bypassing the '\r' completely
  if (c == EOF)
    return NULLCHAR;
  else
    return (char) c;
}

/* Parse settings file named "name". If file found, return the
   full name in fullname and return TRUE; else return FALSE */
Boolean
ParseSettingsFile(char *name, char **addr)
{
  FILE *f;
  int ok,len;
  char buf[MSG_SIZ], fullname[MSG_SIZ];


  ok = MySearchPath(installDir, name, fullname);
  if(!ok && strchr(name, '.') == NULL)
    { // append default file-name extension '.ini' when needed
      len = snprintf(buf,MSG_SIZ, "%s.ini", name);
      if( (len >= MSG_SIZ) && appData.debugMode )
	fprintf(debugFP, "ParseSettingsFile: buffer truncated. Input: name=%s \n",name);

      ok = MySearchPath(installDir, buf, fullname);
    }
  if (ok) {
    f = fopen(fullname, "r");
#ifdef DATADIR
    if(f == NULL && *fullname != '/' && !addr) {         // when a relative name did not work
	char buf[MSG_SIZ];
	snprintf(buf, MSG_SIZ, "~/.xboard/themes/conf/%s", name);
	MySearchPath(installDir, buf, fullname); // first look in user's own files
	f = fopen(fullname, "r");
	if(f == NULL) {
	    snprintf(buf, MSG_SIZ, "%s/themes/conf", DATADIR);
	    MySearchPath(buf, name, fullname); // also look in standard place
	    f = fopen(fullname, "r");
	}
    }
#endif
    if (f != NULL) {
      if (addr != NULL) {
	    ASSIGN(*addr, fullname);
      }
      ParseArgs(FileGet, f);
      fclose(f);
      return TRUE;
    }
  }
  return FALSE;
}

void
ParseArgs(GetFunc get, void *cl)
{
  char argName[MAX_ARG_LEN];
  char argValue[MAX_ARG_LEN];
  ArgDescriptor *ad;
  char start;
  char *q;
  int i, octval;
  char ch;
  int posarg = 4; // default is game file

  ch = get(cl);
  for (;;) {
    int posflag = 0;
    while (ch == ' ' || ch == '\n' || ch == '\t') ch = get(cl);
    if (ch == NULLCHAR) break;
    if (ch == ';') {
      /* Comment to end of line */
      ch = get(cl);
      while (ch != '\n' && ch != NULLCHAR) ch = get(cl);
      continue;
    } else if (ch == SLASH || ch == '-') {
      /* Switch */
      q = argName;
      while (ch != ' ' && ch != '=' && ch != ':' && ch != NULLCHAR &&
	     ch != '\n' && ch != '\t') {
	*q++ = ch;
	ch = get(cl);
      }
      *q = NULLCHAR;
      for (ad = argDescriptors; ad->argName != NULL; ad++)
	if (strcmp(ad->argName, argName + 1) == 0) break;
      if (ad->argName == NULL) {
	char endChar = (ch && ch != '\n' && (ch = get(cl)) == '{' ? '}' : '\n');
	ExitArgError(_("Unrecognized argument %s"), argName, get != &FileGet); // [HGM] make unknown argument non-fatal
	while (ch != endChar && ch != NULLCHAR) ch = get(cl); // but skip rest of line it is on (or until closing '}' )
	if(ch == '}') ch = get(cl);
	continue; // so that when it is in a settings file, it is the only setting that will be purged from it
      }
    } else if (ch == '@') {
      /* Indirection file */
      ad = &argDescriptorIndirection;
      ch = get(cl);
    } else {
      /* Positional argument */
      ad = &argDescriptors[posarg++];
      posflag++;
      strncpy(argName, ad->argName,sizeof(argName)/sizeof(argName[0]));
    }

    if (ad->argType == ArgTwo) { // [HGM] kludgey arg type, not suitable for saving
      *(Boolean *) ad->argLoc = 2;
      continue;
    }
    if (ad->argType == ArgTrue) {
      *(Boolean *) ad->argLoc = TRUE;
      continue;
    }
    if (ad->argType == ArgFalse) {
      *(Boolean *) ad->argLoc = FALSE;
      continue;
    }

    while (ch == ' ' || ch == '=' || ch == ':' || ch == '\t') ch = get(cl);
    if (ch == NULLCHAR || ch == '\n') {
      ExitArgError(_("No value provided for argument %s"), argName, TRUE);
    }
    q = argValue;
    if (ch == '{') {
      // Quoting with { }.  No characters have to (or can) be escaped.
      // Thus the string cannot contain a '}' character.
      start = ch;
      ch = get(cl);
      while (start) {
	switch (ch) {
	case NULLCHAR:
	  start = NULLCHAR;
	  break;

	case '}':
	  ch = get(cl);
	  start = NULLCHAR;
	  break;

	default:
	  *q++ = ch;
	  ch = get(cl);
	  break;
	}
      }
    } else if (ch == '\'' || ch == '"') {
      // Quoting with ' ' or " ", with \ as escape character.
      // Inconvenient for long strings that may contain Windows filenames.
      start = ch;
      ch = get(cl);
      while (start) {
	switch (ch) {
	case NULLCHAR:
	  start = NULLCHAR;
	  break;

	default:
        not_special:
	  *q++ = ch;
	  ch = get(cl);
	  break;

	case '\'':
	case '\"':
	  if (ch == start) {
	    ch = get(cl);
	    start = NULLCHAR;
	    break;
	  } else {
	    goto not_special;
	  }

	case '\\':
          if (ad->argType == ArgFilename
	      || ad->argType == ArgSettingsFilename) {
	      goto not_special;
	  }
	  ch = get(cl);
	  switch (ch) {
	  case NULLCHAR:
	    ExitArgError(_("Incomplete \\ escape in value for %s"), argName, TRUE);
	    break;
	  case 'n':
	    *q++ = '\n';
	    ch = get(cl);
	    break;
	  case 'r':
	    *q++ = '\r';
	    ch = get(cl);
	    break;
	  case 't':
	    *q++ = '\t';
	    ch = get(cl);
	    break;
	  case 'b':
	    *q++ = '\b';
	    ch = get(cl);
	    break;
	  case 'f':
	    *q++ = '\f';
	    ch = get(cl);
	    break;
	  default:
	    octval = 0;
	    for (i = 0; i < 3; i++) {
	      if (ch >= '0' && ch <= '7') {
		octval = octval*8 + (ch - '0');
		ch = get(cl);
	      } else {
		break;
	      }
	    }
	    if (i > 0) {
	      *q++ = (char) octval;
	    } else {
	      *q++ = ch;
	      ch = get(cl);
	    }
	    break;
	  }
	  break;
	}
      }
    } else {
      while ((ch != ' ' || posflag) && ch != NULLCHAR && ch != '\t' && ch != '\n') { // space allowed in positional arg
	*q++ = ch;
	ch = get(cl);
      }
    }
    *q = NULLCHAR;

    if(posflag) { // positional argument: the argName was implied, and per default set as -lgf
      int len = strlen(argValue) - 4; // start of filename extension
      if(len < 0) len = 0;
      if(!StrCaseCmp(argValue + len, ".trn")) {
        ad = &argDescriptors[2]; // correct implied type to -tf
        appData.tourney = TRUE; // let it parse -tourneyOptions later
      } else if(!StrCaseCmp(argValue + len, ".fen") || !StrCaseCmp(argValue + len, ".epd")) {
        ad = &argDescriptors[1]; // correct implied type to -lpf
        appData.viewer = TRUE;
      } else if(!StrCaseCmp(argValue + len, ".ini") || !StrCaseCmp(argValue + len, ".xop")) {
        ad = &argDescriptors[0]; // correct implied type to -opt
      } else if(GetEngineLine(argValue, 11)) {
        ad = &argDescriptors[3]; // correct implied type to -is
      } else { // keep default -lgf, but let it imply viewer mode as well
        appData.viewer = TRUE;
      }
      strncpy(argName, ad->argName,sizeof(argName)/sizeof(argName[0]));
    }

    switch (ad->argType) {
    case ArgInt:
      *(int *) ad->argLoc = ValidateInt(argValue);
      break;

    case ArgX:
      *(int *) ad->argLoc = ValidateInt(argValue) + wpMain.x; // [HGM] placement: translate stored relative to absolute
      break;

    case ArgY:
      *(int *) ad->argLoc = ValidateInt(argValue) + wpMain.y; // (this is really kludgey, it should be done where used...)
      break;

    case ArgZ:
      *(int *) ad->argLoc = ValidateInt(argValue);
      EnsureOnScreen(&wpMain.x, &wpMain.y, minX, minY);
      break;

    case ArgFloat:
      *(float *) ad->argLoc = (float) atof(argValue);
      break;

    case ArgString:
    case ArgFilename:
      if(argValue[0] == '~' && argValue[1] == '~') {
        char buf[4*MSG_SIZ]; // expand ~~
        snprintf(buf, 4*MSG_SIZ, "%s%s", DATADIR, argValue+2);
        ASSIGN(*(char **) ad->argLoc, buf);
        break;
      }
      ASSIGN(*(char **) ad->argLoc, argValue);
      break;

    case ArgBackupSettingsFile: // no-op if non-default settings-file already successfully read
	if(strcmp(*(char**)ad->argLoc, SETTINGS_FILE)) break;
    case ArgSettingsFilename:
      {
	if (ParseSettingsFile(argValue, (char**)ad->argLoc)) {
	} else {
	  if (ad->argLoc != NULL) {
	  } else {
	    ExitArgError(_("Failed to open indirection file %s"), argValue, TRUE);
	  }
	}
      }
      break;

    case ArgBoolean:
      switch (argValue[0]) {
      case 't':
      case 'T':
	*(Boolean *) ad->argLoc = TRUE;
	break;
      case 'f':
      case 'F':
	*(Boolean *) ad->argLoc = FALSE;
	break;
      default:
	ExitArgError(_("Unrecognized boolean argument value %s"), argValue, TRUE);
	break;
      }
      break;

    case ArgColor:
      ParseColor((int)(intptr_t)ad->argLoc, argValue);
      break;

    case ArgAttribs: {
      ColorClass cc = (ColorClass)ad->argLoc;
	ParseTextAttribs(cc, argValue); // [HGM] wrapper for platform independency
      }
      break;

    case ArgBoardSize:
      ParseBoardSize(ad->argLoc, argValue);
      break;

    case ArgFont:
      ParseFont(argValue, (int)(intptr_t)ad->argLoc);
      break;

    case ArgCommSettings:
      ParseCommPortSettings(argValue);
      break;

    case ArgMaster:
      AppendToSettingsFile(argValue);
      break;

    case ArgInstall:
      q = *(char **) ad->argLoc;
      if((saveDate == 0 || saveDate - dateStamp < 0) && !strstr(q, argValue) ) {
        int l = strlen(q) + strlen(argValue);
        *(char **) ad->argLoc = malloc(l+2);
        snprintf(*(char **) ad->argLoc, l+2, "%s%s\n", q, argValue);
        free(q);
      }
      break;

    case ArgNone:
      ExitArgError(_("Unrecognized argument %s"), argValue, TRUE);
      break;
    case ArgTwo:
    case ArgTrue:
    case ArgFalse: ;
    }
  }
}

void
ParseArgsFromString(char *p)
{
    ParseArgs(StringGet, &p);
}

void
ParseArgsFromFile(FILE *f)
{
    ParseArgs(FileGet, f);
}

void
ParseIcsTextMenu(char *icsTextMenuString)
{
//  int flags = 0;
  IcsTextMenuEntry *e = icsTextMenuEntry;
  char *p = icsTextMenuString;
  while (e->item != NULL && e < icsTextMenuEntry + ICS_TEXT_MENU_SIZE) {
    free(e->item);
    e->item = NULL;
    if (e->command != NULL) {
      free(e->command);
      e->command = NULL;
    }
    e++;
  }
  e = icsTextMenuEntry;
  while (*p && e < icsTextMenuEntry + ICS_TEXT_MENU_SIZE) {
    if (*p == ';' || *p == '\n') {
      e->item = strdup("-");
      e->command = NULL;
      p++;
    } else if (*p == '-') {
      e->item = strdup("-");
      e->command = NULL;
      p++;
      if (*p) p++;
    } else {
      char *q, *r, *s, *t;
      char c;
      q = strchr(p, ',');
      if (q == NULL) break;
      *q = NULLCHAR;
      r = strchr(q + 1, ',');
      if (r == NULL) break;
      *r = NULLCHAR;
      s = strchr(r + 1, ',');
      if (s == NULL) break;
      *s = NULLCHAR;
      c = ';';
      t = strchr(s + 1, c);
      if (t == NULL) {
	c = '\n';
	t = strchr(s + 1, c);
      }
      if (t != NULL) *t = NULLCHAR;
      e->item = strdup(p);
      e->command = strdup(q + 1);
      e->getname = *(r + 1) != '0';
      e->immediate = *(s + 1) != '0';
      *q = ',';
      *r = ',';
      *s = ',';
      if (t == NULL) break;
      *t = c;
      p = t + 1;
    }
    e++;
  }
}

void
SetDefaultTextAttribs()
{
  ColorClass cc;
  for (cc = (ColorClass)0; cc < ColorNone; cc++) {
    ParseTextAttribs(cc, defaultTextAttribs[cc]);
  }
}

void
SetDefaultsFromList()
{ // [HGM] ini: take defaults from argDescriptor list
  int i;

  for(i=0; argDescriptors[i].argName != NULL; i++) {
    if(argDescriptors[i].defaultValue != INVALID)
      switch(argDescriptors[i].argType) {
        case ArgBoolean:
        case ArgTwo:
        case ArgTrue:
        case ArgFalse:
          *(Boolean *) argDescriptors[i].argLoc = (int)(intptr_t)argDescriptors[i].defaultValue;
          break;
        case ArgInt:
        case ArgX:
        case ArgY:
        case ArgZ:
          *(int *) argDescriptors[i].argLoc = (int)(intptr_t)argDescriptors[i].defaultValue;
          break;
        case ArgString:
        case ArgFilename:
        case ArgSettingsFilename:
          if((char *)argDescriptors[i].defaultValue)
          *(char **) argDescriptors[i].argLoc = strdup((char *)argDescriptors[i].defaultValue);
          break;
        case ArgBoardSize:
          *(int *) argDescriptors[i].argLoc = (int)(intptr_t)argDescriptors[i].defaultValue;
          break;
        case ArgColor:
          ParseColor((int)(intptr_t)argDescriptors[i].argLoc, (char*)argDescriptors[i].defaultValue);
          break;
        case ArgFloat: // floats cannot be casted to int without precision loss
        default: ; // some arg types cannot be initialized through table
    }
  }
}

void
InitAppData(char *lpCmdLine)
{
  int i;
  char buf[MAX_ARG_LEN], currDir[MSG_SIZ];
  char *p;

  /* Initialize to defaults */
  SetDefaultsFromList(); // this sets most defaults

  // some parameters for which there are no options!
  appData.Iconic = FALSE; /*unused*/
  appData.icsEngineAnalyze = FALSE;

  // float: casting to int is not harmless, so default cannot be contained in table
  appData.timeDelay = TIME_DELAY;
  appData.timeIncrement = -314159;

  // some complex, platform-dependent stuff that could not be handled from table
  SetDefaultTextAttribs();
  SetFontDefaults();
  SetCommPortDefaults();

  /* Parse default settings file if any */
  ParseSettingsFile(settingsFileName, &settingsFileName);

  /* Parse command line */
  ParseArgs(StringGet, &lpCmdLine);

  if(appData.viewer && appData.viewerOptions[0]) ParseArgsFromString(appData.viewerOptions);
  if(appData.tourney && appData.tourneyOptions[0]) ParseArgsFromString(appData.tourneyOptions);
  chessProgram |= GetEngineLine(firstEngineLine, 0) || GetEngineLine(secondEngineLine, 1);
  appData.icsActive |= GetEngineLine(icsNick, 10);

  /* [HGM] make sure board size is acceptable */
  if(appData.NrFiles > BOARD_FILES ||
     appData.NrRanks > BOARD_RANKS   )
      DisplayFatalError("Recompile with BOARD_RANKS or BOARD_FILES, to support this size", 0, 2);

  if(!*appData.secondChessProgram) { ASSIGN(appData.secondChessProgram, appData.firstChessProgram); } // [HGM] scp defaults to fcp

  /* [HGM] After parsing the options from the .ini file, and overruling them
   * with options from the command line, we now make an even higher priority
   * overrule by WB options attached to the engine command line. This so that
   * tournament managers can use WB options (such as /timeOdds) that follow
   * the engines.
   */
  if(appData.firstChessProgram != NULL) {
      char *p = StrStr(appData.firstChessProgram, "WBopt");
      static char *f = "first";
      char buf[MSG_SIZ], *q = buf;
      int len;

      if(p != NULL)
	{ // engine command line contains WinBoard options
          len = snprintf(buf, MSG_SIZ, p+6, f, f, f, f, f, f, f, f, f, f); // replace %s in them by "first"
	  if( (len >= MSG_SIZ) && appData.debugMode )
	    fprintf(debugFP, "InitAppData: buffer truncated.\n");

          ParseArgs(StringGet, &q);
          p[-1] = 0; // cut them offengine command line
	}
  }
  // now do same for second chess program
  if(appData.secondChessProgram != NULL) {
      char *p = StrStr(appData.secondChessProgram, "WBopt");
      static char *s = "second";
      char buf[MSG_SIZ], *q = buf;
      int len;

      if(p != NULL)
	{ // engine command line contains WinBoard options
          len = snprintf(buf,MSG_SIZ, p+6, s, s, s, s, s, s, s, s, s, s); // replace %s in them by "first"
	  if( (len >= MSG_SIZ) && appData.debugMode )
	    fprintf(debugFP, "InitAppData: buffer truncated.\n");

          ParseArgs(StringGet, &q);
          p[-1] = 0; // cut them offengine command line
	}
  }

  /* Propagate options that affect others */
  if (appData.matchMode || appData.matchGames) chessProgram = TRUE;
  if (appData.icsActive || appData.noChessProgram) {
     chessProgram = FALSE;  /* not local chess program mode */
  }
  if(appData.timeIncrement == -314159) { // new storage mechanism of (mps,inc) in use and no -inc on command line
    if(appData.movesPerSession <= 0) { // new encoding of incremental mode
      appData.timeIncrement = -appData.movesPerSession/1000.;
    } else appData.timeIncrement = -1;
  }
  if(appData.movesPerSession <= 0) appData.movesPerSession = MOVES_PER_SESSION; // mps <= 0 is invalid in any case

  /* Open startup dialog if needed */
  if ((!appData.noChessProgram && !chessProgram && !appData.icsActive) ||
      (appData.icsActive && *appData.icsHost == NULLCHAR) ||
      (chessProgram && (*appData.firstChessProgram == NULLCHAR ||
                        *appData.secondChessProgram == NULLCHAR)))
		PopUpStartupDialog();

  /* Make sure save files land in the right (?) directory */
  if (MyGetFullPathName(appData.saveGameFile, buf)) {
    appData.saveGameFile = strdup(buf);
  }
  if (MyGetFullPathName(appData.savePositionFile, buf)) {
    appData.savePositionFile = strdup(buf);
  }

  if(autoClose) { // was called for updating settingsfile only
    if(saveSettingsOnExit) SaveSettings(settingsFileName);
    exit(0);
  }

  /* Finish initialization for fonts and sounds */
  CreateFonts();

  GetCurrentDirectory(MSG_SIZ, currDir);
  SetCurrentDirectory(installDir);
  LoadAllSounds();
  SetCurrentDirectory(currDir);

  p = icsTextMenuString;
  if (p[0] == '@') {
    FILE* f = fopen(p + 1, "r");
    if (f == NULL) {
      DisplayFatalError(p + 1, errno, 2);
      return;
    }
    i = fread(buf, 1, sizeof(buf)-1, f);
    fclose(f);
    buf[i] = NULLCHAR;
    p = buf;
  }
  ParseIcsTextMenu(strdup(p));
}

void
SaveSettings(char* name)
{
  FILE *f;
  ArgDescriptor *ad;
  char dir[MSG_SIZ], buf[MSG_SIZ];
  int mps = appData.movesPerSession;
  TimeMark now;

  if (!MainWindowUp() && !autoClose) return;

  GetTimeMark(&now); saveDate = now.sec;

  GetCurrentDirectory(MSG_SIZ, dir);
  if(MySearchPath(installDir, name, buf)) {
    f = fopen(buf, "w");
  } else {
    SetCurrentDirectory(installDir);
    f = fopen(name, "w");
    SetCurrentDirectory(dir);
  }
  if (f == NULL) {
    DisplayError(name, errno);
    return;
  }

  fprintf(f, ";\n");
  fprintf(f, "; %s Save Settings file\n", PACKAGE_STRING);
  fprintf(f, ";\n");
  fprintf(f, "; You can edit the values of options that are already set in this file,\n");
  fprintf(f, "; but if you add other options, the next Save Settings will not save them.\n");
  fprintf(f, "; Use a shortcut, an @indirection file, or a .bat file instead.\n");
  fprintf(f, ";\n");

  GetWindowCoords();

  /* [AS] Move history */
  wpMoveHistory.visible = MoveHistoryIsUp();

  /* [AS] Eval graph */
  wpEvalGraph.visible = EvalGraphIsUp();

  /* [AS] Engine output */
  wpEngineOutput.visible = EngineOutputIsUp();

  // [HGM] in WB we have to copy sound names to appData first
  ExportSounds();

  if(appData.timeIncrement >= 0) appData.movesPerSession = -1000*appData.timeIncrement; // kludge to store mps & inc as one

  for (ad = argDescriptors; ad->argName != NULL; ad++) {
    if (!ad->save) continue;
    switch (ad->argType) {
    case ArgString:
      {
	char *p = *(char **)ad->argLoc;
	if(p == NULL) break; // just in case
	if ((strchr(p, '\\') || strchr(p, '\n')) && !strchr(p, '}')) {
	  /* Quote multiline values or \-containing values
	     with { } if possible */
	  fprintf(f, OPTCHAR "%s" SEPCHAR "{%s}\n", ad->argName, p);
	} else {
	  /* Else quote with " " */
	  fprintf(f, OPTCHAR "%s" SEPCHAR "\"", ad->argName);
	  while (*p) {
	    if (*p == '\n') fprintf(f, "\n");
	    else if (*p == '\r') fprintf(f, "\\r");
	    else if (*p == '\t') fprintf(f, "\\t");
	    else if (*p == '\b') fprintf(f, "\\b");
	    else if (*p == '\f') fprintf(f, "\\f");
	    else if (*p < ' ') fprintf(f, "\\%03o", *p);
	    else if (*p == '\"') fprintf(f, "\\\"");
	    else if (*p == '\\') fprintf(f, "\\\\");
	    else putc(*p, f);
	    p++;
	  }
	  fprintf(f, "\"\n");
	}
      }
      break;
    case ArgInt:
    case ArgZ:
      fprintf(f, OPTCHAR "%s" SEPCHAR "%d\n", ad->argName, *(int *)ad->argLoc);
      break;
    case ArgX:
      fprintf(f, OPTCHAR "%s" SEPCHAR "%d\n", ad->argName, *(int *)ad->argLoc - wpMain.x); // [HGM] placement: store relative value
      break;
    case ArgY:
      fprintf(f, OPTCHAR "%s" SEPCHAR "%d\n", ad->argName, *(int *)ad->argLoc - wpMain.y);
      break;
    case ArgFloat:
      fprintf(f, OPTCHAR "%s" SEPCHAR "%g\n", ad->argName, *(float *)ad->argLoc);
      break;
    case ArgBoolean:
      fprintf(f, OPTCHAR "%s" SEPCHAR "%s\n", ad->argName,
	(*(Boolean *)ad->argLoc) ? "true" : "false");
      break;
    case ArgTrue:
      if (*(Boolean *)ad->argLoc) fprintf(f, OPTCHAR "%s\n", ad->argName);
      break;
    case ArgFalse:
      if (!*(Boolean *)ad->argLoc) fprintf(f, OPTCHAR "%s\n", ad->argName);
      break;
    case ArgColor:
      SaveColor(f, ad);
      break;
    case ArgAttribs:
      SaveAttribsArg(f, ad);
      break;
    case ArgFilename:
      if(*(char**)ad->argLoc == NULL) break; // just in case
      { char buf[MSG_SIZ];
        snprintf(buf, MSG_SIZ, "%s", *(char**)ad->argLoc);
#ifdef OSXAPP
        if(strstr(buf, DATADIR) == buf)
          snprintf(buf, MSG_SIZ, "~~%s", *(char**)ad->argLoc + strlen(DATADIR));
#endif
        if (strchr(buf, '\"')) {
          fprintf(f, OPTCHAR "%s" SEPCHAR "'%s'\n", ad->argName, buf);
        } else {
          fprintf(f, OPTCHAR "%s" SEPCHAR "\"%s\"\n", ad->argName, buf);
        }
      }
      break;
    case ArgBoardSize:
      SaveBoardSize(f, ad->argName, ad->argLoc);
      break;
    case ArgFont:
      SaveFontArg(f, ad);
      break;
    case ArgCommSettings:
      PrintCommPortSettings(f, ad->argName);
    case ArgTwo:
    case ArgNone:
    case ArgBackupSettingsFile:
    case ArgSettingsFilename: ;
    case ArgMaster: ;
    case ArgInstall: ;
    }
  }
  fclose(f);
  appData.movesPerSession = mps;
}

Boolean
GetArgValue(char *name)
{ // retrieve (as text) current value of string or int argument given by name
  // (this is used for maing the values available in the adapter command)
  ArgDescriptor *ad;
  int len;

  for (ad = argDescriptors; ad->argName != NULL; ad++)
    if (strcmp(ad->argName, name) == 0) break;

  if (ad->argName == NULL) return FALSE;

  switch(ad->argType) {
    case ArgString:
    case ArgFilename:
      strncpy(name, *(char**) ad->argLoc, MSG_SIZ);

      return TRUE;
    case ArgInt:
      len = snprintf(name, MSG_SIZ, "%d", *(int*) ad->argLoc);
      if( (len >= MSG_SIZ) && appData.debugMode )
	fprintf(debugFP, "GetArgValue: buffer truncated.\n");

      return TRUE;
    case ArgBoolean:
      len = snprintf(name, MSG_SIZ, "%s", *(Boolean*) ad->argLoc ? "true" : "false");
      if( (len >= MSG_SIZ) && appData.debugMode )
	fprintf(debugFP, "GetArgValue: buffer truncated.\n");

      return TRUE;
    default: ;
  }

  return FALSE;
}
