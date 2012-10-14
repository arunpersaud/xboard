/*
 * menus.c -- platform-indendent menu handling code for XBoard
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

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if ENABLE_NLS
#include <locale.h>
#endif

// [HGM] bitmaps: put before incuding the bitmaps / pixmaps, to know how many piece types there are.
#include "common.h"

#include "frontend.h"
#include "backend.h"
#include "xhistory.h"
#include "menus.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

/*
 * Button/menu procedures
 */

char  *gameCopyFilename, *gamePasteFilename;
Boolean saveSettingsOnExit;
char *settingsFileName;

static int
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

void
LoadGameProc ()
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	Reset(FALSE, TRUE);
    }
    FileNamePopUp(_("Load game file name?"), "", ".pgn .game", LoadGamePopUp, "rb");
}

void
LoadNextGameProc ()
{
    ReloadGame(1);
}

void
LoadPrevGameProc ()
{
    ReloadGame(-1);
}

void
ReloadGameProc ()
{
    ReloadGame(0);
}

void
LoadNextPositionProc ()
{
    ReloadPosition(1);
}

void
LoadPrevPositionProc ()
{
    ReloadPosition(-1);
}

void
ReloadPositionProc ()
{
    ReloadPosition(0);
}

void
LoadPositionProc() 
{
    if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
	Reset(FALSE, TRUE);
    }
    FileNamePopUp(_("Load position file name?"), "", ".fen .epd .pos", LoadPosition, "rb");
}

void
SaveGameProc ()
{
    FileNamePopUp(_("Save game file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "game" : "pgn"),
		  appData.oldSaveStyle ? ".game" : ".pgn",
		  SaveGame, "a");
}

void
SavePositionProc ()
{
    FileNamePopUp(_("Save position file name?"),
		  DefaultFileName(appData.oldSaveStyle ? "pos" : "fen"),
		  appData.oldSaveStyle ? ".pos" : ".fen",
		  SavePosition, "a");
}

void
ReloadCmailMsgProc ()
{
    ReloadCmailMsgEvent(FALSE);
}

void
CopyFENToClipboard ()
{ // wrapper to make call from back-end possible
  CopyPositionProc();
}

void
CopyPositionProc ()
{
    static char *selected_fen_position=NULL;
    if(gameMode == EditPosition) EditPositionDone(TRUE);
    if (selected_fen_position) free(selected_fen_position);
    selected_fen_position = (char *)PositionToFEN(currentMove, NULL);
    if (!selected_fen_position) return;
    CopySomething(selected_fen_position);
}

void
CopyGameProc ()
{
  int ret;

  ret = SaveGameToFile(gameCopyFilename, FALSE);
  if (!ret) return;

  CopySomething(NULL);
}

void
CopyGameListProc ()
{
  if(!SaveGameListAsText(fopen(gameCopyFilename, "w"))) return;
  CopySomething(NULL);
}

void
AutoSaveGame ()
{
    SaveGameProc();
}


void
QuitProc ()
{
    ExitEvent(0);
}

void
MatchProc ()
{
    MatchEvent(2);
}

void
AdjuWhiteProc ()
{
    UserAdjudicationEvent(+1);
}

void
AdjuBlackProc ()
{
    UserAdjudicationEvent(-1);
}

void
AdjuDrawProc ()
{
    UserAdjudicationEvent(0);
}

void
RevertProc ()
{
    RevertEvent(False);
}

void
AnnotateProc ()
{
    RevertEvent(True);
}

void
FlipViewProc ()
{
    if(twoBoards) { partnerUp = 1; DrawPosition(True, NULL); partnerUp = 0; }
    flipView = !flipView;
    DrawPosition(True, NULL);
}

void
SaveOnExitProc ()
{
  saveSettingsOnExit = !saveSettingsOnExit;

  MarkMenuItem("Options.SaveSettingsonExit", saveSettingsOnExit);
}

void
SaveSettingsProc ()
{
  SaveSettings(settingsFileName);
}

void
InfoProc ()
{
    char buf[MSG_SIZ];
    snprintf(buf, sizeof(buf), "xterm -e info --directory %s --directory . -f %s &",
	    INFODIR, INFOFILE);
    system(buf);
}

void
BugReportProc ()
{
    char buf[MSG_SIZ];
    snprintf(buf, MSG_SIZ, "%s mailto:bug-xboard@gnu.org", appData.sysOpen);
    system(buf);
}

void
GuideProc ()
{
    char buf[MSG_SIZ];
    snprintf(buf, MSG_SIZ, "%s http://www.gnu.org/software/xboard/user_guide/UserGuide.html", appData.sysOpen);
    system(buf);
}

void
HomePageProc ()
{
    char buf[MSG_SIZ];
    snprintf(buf, MSG_SIZ, "%s http://www.gnu.org/software/xboard/", appData.sysOpen);
    system(buf);
}

void
NewsPageProc ()
{
    char buf[MSG_SIZ];
    snprintf(buf, MSG_SIZ, "%s http://www.gnu.org/software/xboard/whats_new/portal.html", appData.sysOpen);
    system(buf);
}

void
AboutProc ()
{
    char buf[2 * MSG_SIZ];
#if ZIPPY
    char *zippy = _(" (with Zippy code)");
#else
    char *zippy = "";
#endif
    snprintf(buf, sizeof(buf), 
_("%s%s\n\n"
"Copyright 1991 Digital Equipment Corporation\n"
"Enhancements Copyright 1992-2012 Free Software Foundation\n"
"Enhancements Copyright 2005 Alessandro Scotti\n\n"
"%s is free software and carries NO WARRANTY;"
"see the file COPYING for more information.\n\n"
"Visit XBoard on the web at: http://www.gnu.org/software/xboard/\n"
"Check out the newest features at: http://www.gnu.org/software/xboard/whats_new.html\n\n"
"Report bugs via email at: <bug-xboard@gnu.org>\n\n"
  ),
	    programVersion, zippy, PACKAGE);
    ErrorPopUp(_("About XBoard"), buf, FALSE);
}

void
DebugProc ()
{
    appData.debugMode = !appData.debugMode;
    if(!strcmp(appData.nameOfDebugFile, "stderr")) return; // stderr is already open, and should never be closed
    if(!appData.debugMode) fclose(debugFP);
    else {
	debugFP = fopen(appData.nameOfDebugFile, "w");
	if(debugFP == NULL) debugFP = stderr;
	else setbuf(debugFP, NULL);
    }
}

void
NothingProc ()
{
    return;
}

#ifdef OPTIONSDIALOG
#   define MARK_MENU_ITEM(X,Y)
#else
#   define MARK_MENU_ITEM(X,Y) MarkMenuItem(X, Y)
#endif

void
PonderNextMoveProc ()
{
  PonderNextMoveEvent(!appData.ponderNextMove);
  MARK_MENU_ITEM("Options.PonderNextMove", appData.ponderNextMove);
}

void
AlwaysQueenProc ()
{
    appData.alwaysPromoteToQueen = !appData.alwaysPromoteToQueen;
    MARK_MENU_ITEM("Options.AlwaysQueen", appData.alwaysPromoteToQueen);
}

void
AnimateDraggingProc ()
{
    appData.animateDragging = !appData.animateDragging;

    if (appData.animateDragging) CreateAnimVars();
    MARK_MENU_ITEM("Options.AnimateDragging", appData.animateDragging);
}

void
AnimateMovingProc ()
{
    appData.animate = !appData.animate;
    if (appData.animate) CreateAnimVars();
    MARK_MENU_ITEM("Options.AnimateMoving", appData.animate);
}

void
AutoflagProc ()
{
    appData.autoCallFlag = !appData.autoCallFlag;
    MARK_MENU_ITEM("Options.AutoFlag", appData.autoCallFlag);
}

void
AutoflipProc ()
{
    appData.autoFlipView = !appData.autoFlipView;
    MARK_MENU_ITEM("Options.AutoFlipView", appData.autoFlipView);
}

void
BlindfoldProc ()
{
    appData.blindfold = !appData.blindfold;
    MARK_MENU_ITEM("Options.Blindfold", appData.blindfold);
    DrawPosition(True, NULL);
}

void
TestLegalityProc ()
{
    appData.testLegality = !appData.testLegality;
    MARK_MENU_ITEM("Options.TestLegality", appData.testLegality);
}


void
FlashMovesProc ()
{
    if (appData.flashCount == 0) {
	appData.flashCount = 3;
    } else {
	appData.flashCount = -appData.flashCount;
    }
    MARK_MENU_ITEM("Options.FlashMoves", appData.flashCount > 0);
}

#if HIGHDRAG
void
HighlightDraggingProc ()
{
    appData.highlightDragging = !appData.highlightDragging;
    MARK_MENU_ITEM("Options.HighlightDragging", appData.highlightDragging);
}
#endif

void
HighlightLastMoveProc ()
{
    appData.highlightLastMove = !appData.highlightLastMove;
    MARK_MENU_ITEM("Options.HighlightLastMove", appData.highlightLastMove);
}

void
HighlightArrowProc ()
{
    appData.highlightMoveWithArrow = !appData.highlightMoveWithArrow;
    MARK_MENU_ITEM("Options.HighlightWithArrow", appData.highlightMoveWithArrow);
}

void
IcsAlarmProc ()
{
    appData.icsAlarm = !appData.icsAlarm;
//    MARK_MENU_ITEM("Options.ICSAlarm", appData.icsAlarm);
}

void
MoveSoundProc ()
{
    appData.ringBellAfterMoves = !appData.ringBellAfterMoves;
    MARK_MENU_ITEM("Options.MoveSound", appData.ringBellAfterMoves);
}

void
OneClickProc ()
{
    appData.oneClick = !appData.oneClick;
    MARK_MENU_ITEM("Options.OneClickMoving", appData.oneClick);
}

void
PeriodicUpdatesProc ()
{
    PeriodicUpdatesEvent(!appData.periodicUpdates);
    MARK_MENU_ITEM("Options.PeriodicUpdates", appData.periodicUpdates);
}

void
PopupExitMessageProc ()
{
    appData.popupExitMessage = !appData.popupExitMessage;
    MARK_MENU_ITEM("Options.PopupExitMessage", appData.popupExitMessage);
}

void
PopupMoveErrorsProc ()
{
    appData.popupMoveErrors = !appData.popupMoveErrors;
    MARK_MENU_ITEM("Options.PopupMoveErrors", appData.popupMoveErrors);
}

void
PremoveProc ()
{
    appData.premove = !appData.premove;
//    MARK_MENU_ITEM("Options.Premove", appData.premove);
}

void
ShowCoordsProc ()
{
    appData.showCoords = !appData.showCoords;
    MARK_MENU_ITEM("Options.ShowCoords", appData.showCoords);
    DrawPosition(True, NULL);
}

void
ShowThinkingProc ()
{
    appData.showThinking = !appData.showThinking; // [HGM] thinking: taken out of ShowThinkingEvent
    ShowThinkingEvent();
}

void
HideThinkingProc ()
{
  appData.hideThinkingFromHuman = !appData.hideThinkingFromHuman; // [HGM] thinking: taken out of ShowThinkingEvent
  ShowThinkingEvent();

  MARK_MENU_ITEM("Options.HideThinking", appData.hideThinkingFromHuman);
}

/*
 *  Menu definition tables
 */

MenuItem fileMenu[] = {
    {N_("New Game        Ctrl+N"),        "NewGame", ResetGameEvent},
    {N_("New Shuffle Game ..."),          "NewShuffleGame", ShuffleMenuProc},
    {N_("New Variant ...   Alt+Shift+V"), "NewVariant", NewVariantProc},      // [HGM] variant: not functional yet
    {"----", NULL, NothingProc},
    {N_("Load Game       Ctrl+O"),        "LoadGame", LoadGameProc},
    {N_("Load Position    Ctrl+Shift+O"), "LoadPosition", LoadPositionProc},
//    {N_("Load Next Game"), "LoadNextGame", LoadNextGameProc},
//    {N_("Load Previous Game"), "LoadPreviousGame", LoadPrevGameProc},
//    {N_("Reload Same Game"), "ReloadSameGame", ReloadGameProc},
    {N_("Next Position     Shift+PgDn"), "LoadNextPosition", LoadNextPositionProc},
    {N_("Prev Position     Shift+PgUp"), "LoadPreviousPosition", LoadPrevPositionProc},
    {"----", NULL, NothingProc},
//    {N_("Reload Same Position"), "Reload Same Position", ReloadPositionProc},
    {N_("Save Game       Ctrl+S"),        "SaveGame", SaveGameProc},
    {N_("Save Position    Ctrl+Shift+S"), "SavePosition", SavePositionProc},
    {"----", NULL, NothingProc},
    {N_("Mail Move"),            "MailMove", MailMoveEvent},
    {N_("Reload CMail Message"), "ReloadCMailMessage", ReloadCmailMsgProc},
    {"----", NULL, NothingProc},
    {N_("Quit                 Ctr+Q"), "Quit", QuitProc},
    {NULL, NULL, NULL}
};

MenuItem editMenu[] = {
    {N_("Copy Game    Ctrl+C"),        "CopyGame", CopyGameProc},
    {N_("Copy Position Ctrl+Shift+C"), "CopyPosition", CopyPositionProc},
    {N_("Copy Game List"),        "CopyGameList", CopyGameListProc},
    {"----", NULL, NothingProc},
    {N_("Paste Game    Ctrl+V"),        "PasteGame", PasteGameProc},
    {N_("Paste Position Ctrl+Shift+V"), "PastePosition", PastePositionProc},
    {"----", NULL, NothingProc},
    {N_("Edit Game      Ctrl+E"),        "EditGame", EditGameEvent},
    {N_("Edit Position   Ctrl+Shift+E"), "EditPosition", EditPositionEvent},
    {N_("Edit Tags"),                    "EditTags", EditTagsProc},
    {N_("Edit Comment"),                 "EditComment", EditCommentProc},
    {N_("Edit Book"),                    "EditBook", EditBookEvent},
    {"----", NULL, NothingProc},
    {N_("Revert              Home"), "Revert", RevertProc},
    {N_("Annotate"),                 "Annotate", AnnotateProc},
    {N_("Truncate Game  End"),       "TruncateGame", TruncateGameEvent},
    {"----", NULL, NothingProc},
    {N_("Backward         Alt+Left"),   "Backward", BackwardEvent},
    {N_("Forward           Alt+Right"), "Forward", ForwardEvent},
    {N_("Back to Start     Alt+Home"),  "BacktoStart", ToStartEvent},
    {N_("Forward to End Alt+End"),      "ForwardtoEnd", ToEndEvent},
    {NULL, NULL, NULL}
};

MenuItem viewMenu[] = {
    {N_("Flip View             F2"),         "FlipView", FlipViewProc},
    {"----", NULL, NothingProc},
    {N_("Engine Output      Alt+Shift+O"),   "EngineOutput", EngineOutputProc},
    {N_("Move History       Alt+Shift+H"),   "MoveHistory", HistoryShowProc}, // [HGM] hist: activate 4.2.7 code
    {N_("Evaluation Graph  Alt+Shift+E"),    "EvaluationGraph", EvalGraphProc},
    {N_("Game List            Alt+Shift+G"), "GameList", ShowGameListProc},
    {N_("ICS text menu"), "ICStextmenu", IcsTextProc},
    {"----", NULL, NothingProc},
    {N_("Tags"),             "Tags", EditTagsProc},
    {N_("Comments"),         "Comments", EditCommentProc},
    {N_("ICS Input Box"),    "ICSInputBox", IcsInputBoxProc},
    {N_("Open Chat Window"), "OpenChatWindow", ChatProc},
    {"----", NULL, NothingProc},
    {N_("Board..."),          "Board", BoardOptionsProc},
    {N_("Game List Tags..."), "GameListTags", GameListOptionsProc},
    {NULL, NULL, NULL}
};

MenuItem modeMenu[] = {
    {N_("Machine White  Ctrl+W"), "MachineWhite", MachineWhiteEvent},
    {N_("Machine Black  Ctrl+B"), "MachineBlack", MachineBlackEvent},
    {N_("Two Machines   Ctrl+T"), "TwoMachines", TwoMachinesEvent},
    {N_("Analysis Mode  Ctrl+A"), "AnalysisMode", (MenuProc*) AnalyzeModeEvent},
    {N_("Analyze Game   Ctrl+G"), "AnalyzeFile", AnalyzeFileEvent },
    {N_("Edit Game         Ctrl+E"), "EditGame", EditGameEvent},
    {N_("Edit Position      Ctrl+Shift+E"), "EditPosition", EditPositionEvent},
    {N_("Training"),      "Training", TrainingEvent},
    {N_("ICS Client"),    "ICSClient", IcsClientEvent},
    {"----", NULL, NothingProc},
    {N_("Machine Match"),         "MachineMatch", MatchProc},
    {N_("Pause               Pause"),         "Pause", PauseEvent},
    {NULL, NULL, NULL}
};

MenuItem actionMenu[] = {
    {N_("Accept             F3"), "Accept", AcceptEvent},
    {N_("Decline            F4"), "Decline", DeclineEvent},
    {N_("Rematch           F12"), "Rematch", RematchEvent},
    {"----", NULL, NothingProc},
    {N_("Call Flag          F5"), "CallFlag", CallFlagEvent},
    {N_("Draw                F6"), "Draw", DrawEvent},
    {N_("Adjourn            F7"),  "Adjourn", AdjournEvent},
    {N_("Abort                F8"),"Abort", AbortEvent},
    {N_("Resign              F9"), "Resign", ResignEvent},
    {"----", NULL, NothingProc},
    {N_("Stop Observing  F10"), "StopObserving", StopObservingEvent},
    {N_("Stop Examining  F11"), "StopExamining", StopExaminingEvent},
    {N_("Upload to Examine"),   "UploadtoExamine", UploadGameEvent},
    {"----", NULL, NothingProc},
    {N_("Adjudicate to White"), "AdjudicatetoWhite", AdjuWhiteProc},
    {N_("Adjudicate to Black"), "AdjudicatetoBlack", AdjuBlackProc},
    {N_("Adjudicate Draw"),     "AdjudicateDraw", AdjuDrawProc},
    {NULL, NULL, NULL}
};

MenuItem engineMenu[] = {
    {N_("Load New 1st Engine ..."), "LoadNew1stEngine", LoadEngine1Proc},
    {N_("Load New 2nd Engine ..."), "LoadNew2ndEngine", LoadEngine2Proc},
    {"----", NULL, NothingProc},
    {N_("Engine #1 Settings ..."), "Engine#1Settings", FirstSettingsProc},
    {N_("Engine #2 Settings ..."), "Engine#2Settings", SecondSettingsProc},
    {"----", NULL, NothingProc},
    {N_("Hint"), "Hint", HintEvent},
    {N_("Book"), "Book", BookEvent},
    {"----", NULL, NothingProc},
    {N_("Move Now     Ctrl+M"),     "MoveNow", MoveNowEvent},
    {N_("Retract Move  Ctrl+X"), "RetractMove", RetractMoveEvent},
    {NULL, NULL, NULL}
};

MenuItem optionsMenu[] = {
#ifdef OPTIONSDIALOG
    {N_("General ..."), "General", OptionsProc},
#endif
    {N_("Time Control ...       Alt+Shift+T"), "TimeControl", TimeControlProc},
    {N_("Common Engine ...  Alt+Shift+U"),     "CommonEngine", UciMenuProc},
    {N_("Adjudications ...      Alt+Shift+J"), "Adjudications", EngineMenuProc},
    {N_("ICS ..."),    "ICS", IcsOptionsProc},
    {N_("Match ..."), "Match", MatchOptionsProc},
    {N_("Load Game ..."),    "LoadGame", LoadOptionsProc},
    {N_("Save Game ..."),    "SaveGame", SaveOptionsProc},
//    {N_(" ..."),    "", OptionsProc},
    {N_("Game List ..."),    "GameList", GameListOptionsProc},
    {N_("Sounds ..."),    "Sounds", SoundOptionsProc},
    {"----", NULL, NothingProc},
#ifndef OPTIONSDIALOG
    {N_("Always Queen        Ctrl+Shift+Q"),   "AlwaysQueen", AlwaysQueenProc},
    {N_("Animate Dragging"), "AnimateDragging", AnimateDraggingProc},
    {N_("Animate Moving      Ctrl+Shift+A"),   "AnimateMoving", AnimateMovingProc},
    {N_("Auto Flag               Ctrl+Shift+F"), "AutoFlag", AutoflagProc},
    {N_("Auto Flip View"),   "AutoFlipView", AutoflipProc},
    {N_("Blindfold"),        "Blindfold", BlindfoldProc},
    {N_("Flash Moves"),      "FlashMoves", FlashMovesProc},
#if HIGHDRAG
    {N_("Highlight Dragging"),    "HighlightDragging", HighlightDraggingProc},
#endif
    {N_("Highlight Last Move"),   "HighlightLastMove", HighlightLastMoveProc},
    {N_("Highlight With Arrow"),  "HighlightWithArrow", HighlightArrowProc},
    {N_("Move Sound"),            "MoveSound", MoveSoundProc},
//    {N_("ICS Alarm"),             "ICSAlarm", IcsAlarmProc},
    {N_("One-Click Moving"),      "OneClickMoving", OneClickProc},
    {N_("Periodic Updates"),      "PeriodicUpdates", PeriodicUpdatesProc},
    {N_("Ponder Next Move  Ctrl+Shift+P"), "PonderNextMove", PonderNextMoveProc},
    {N_("Popup Exit Message"),    "PopupExitMessage", PopupExitMessageProc},
    {N_("Popup Move Errors"),     "PopupMoveErrors", PopupMoveErrorsProc},
//    {N_("Premove"),               "Premove", PremoveProc},
    {N_("Show Coords"),           "ShowCoords", ShowCoordsProc},
    {N_("Hide Thinking        Ctrl+Shift+H"),   "HideThinking", HideThinkingProc},
    {N_("Test Legality          Ctrl+Shift+L"), "TestLegality", TestLegalityProc},
    {"----", NULL, NothingProc},
#endif
    {N_("Save Settings Now"),     "SaveSettingsNow", SaveSettingsProc},
    {N_("Save Settings on Exit"), "SaveSettingsonExit", SaveOnExitProc},
    {NULL, NULL, NULL}
};

MenuItem helpMenu[] = {
    {N_("Info XBoard"),     "InfoXBoard", InfoProc},
    {N_("Man XBoard   F1"), "ManXBoard", ManProc},
    {"----", NULL, NothingProc},
    {N_("XBoard Home Page"), "XBoardHomePage", HomePageProc},
    {N_("On-line User Guide"), "On-lineUserGuide", GuideProc},
    {N_("Development News"), "DevelopmentNews", NewsPageProc},
    {N_("e-Mail Bug Report"), "e-MailBugReport", BugReportProc},
    {"----", NULL, NothingProc},
    {N_("About XBoard"), "AboutXBoard", AboutProc},
    {NULL, NULL, NULL}
};

MenuItem noMenu[] = {
    { "", "LoadNextGame", LoadNextGameProc },
    { "", "LoadPrevGame", LoadPrevGameProc },
    { "", "ReloadGame", ReloadGameProc },
    { "", "ReloadPosition", ReloadPositionProc },
#ifndef OPTIONSDIALOG
    { "", "AlwaysQueen", AlwaysQueenProc },
    { "", "AnimateDragging", AnimateDraggingProc },
    { "", "AnimateMoving", AnimateMovingProc },
    { "", "Autoflag", AutoflagProc },
    { "", "Autoflip", AutoflipProc },
    { "", "Blindfold", BlindfoldProc },
    { "", "FlashMoves", FlashMovesProc },
#if HIGHDRAG
    { "", "HighlightDragging", HighlightDraggingProc },
#endif
    { "", "HighlightLastMove", HighlightLastMoveProc },
//    { "", "IcsAlarm", IcsAlarmProc },
    { "", "MoveSound", MoveSoundProc },
    { "", "PeriodicUpdates", PeriodicUpdatesProc },
    { "", "PopupExitMessage", PopupExitMessageProc },
    { "", "PopupMoveErrors", PopupMoveErrorsProc },
//    { "", "Premove", PremoveProc },
    { "", "ShowCoords", ShowCoordsProc },
    { "", "ShowThinking", ShowThinkingProc },
    { "", "HideThinking", HideThinkingProc },
    { "", "TestLegality", TestLegalityProc },
#endif
    { "", "AboutGame", AboutGameEvent },
    { "", "DebugProc", DebugProc },
    { "", "Nothing", NothingProc },
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

MenuItem *
MenuNameToItem (char *menuName)
{
    int i=0;
    char buf[MSG_SIZ], *p;
    MenuItem *menuTab;
    static MenuItem a = { NULL, NULL, NothingProc };
    extern Option mainOptions[];
    safeStrCpy(buf, menuName, MSG_SIZ);
    p = strchr(buf, '.');
    if(!p) menuTab = noMenu, p = menuName; else {
	*p++ = NULLCHAR;
	for(i=0; menuBar[i].name; i++)
	    if(!strcmp(buf, menuBar[i].name)) break;
	if(!menuBar[i].name) return NULL; // main menu not found
	menuTab = menuBar[i].mi;
    }
    if(*p == NULLCHAR) { a.handle = mainOptions[i+1].handle; return &a; } // main menu bar
    for(i=0; menuTab[i].string; i++)
	if(menuTab[i].ref && !strcmp(p, menuTab[i].ref)) return menuTab + i;
    return NULL; // item not found
}

void
AppendEnginesToMenu (char *list)
{
    int i=0;
    char *p;
    if(appData.icsActive || appData.recentEngines <= 0) return;
    recentEngines = strdup(list);
    while (*list) {
	p = strchr(list, '\n'); if(p == NULL) break;
	if(i == 0) AppendMenuItem("----", 0); // at least one valid item to add
	*p = 0;
	AppendMenuItem(list, i);
	i++; *p = '\n'; list = p + 1;
    }
}

Enables icsEnables[] = {
    { "File.MailMove", False },
    { "File.ReloadCMailMessage", False },
    { "Mode.MachineBlack", False },
    { "Mode.MachineWhite", False },
    { "Mode.AnalysisMode", False },
    { "Mode.AnalyzeFile", False },
    { "Mode.TwoMachines", False },
    { "Mode.MachineMatch", False },
#ifndef ZIPPY
    { "Engine.Hint", False },
    { "Engine.Book", False },
    { "Engine.MoveNow", False },
#ifndef OPTIONSDIALOG
    { "PeriodicUpdates", False },
    { "HideThinking", False },
    { "PonderNextMove", False },
#endif
#endif
    { "Engine.Engine#1Settings", False },
    { "Engine.Engine#2Settings", False },
    { "Engine.Load1stEngine", False },
    { "Engine.Load2ndEngine", False },
    { "Edit.Annotate", False },
    { "Options.Match", False },
    { NULL, False }
};

Enables ncpEnables[] = {
    { "File.MailMove", False },
    { "File.ReloadCMailMessage", False },
    { "Mode.MachineWhite", False },
    { "Mode.MachineBlack", False },
    { "Mode.AnalysisMode", False },
    { "Mode.AnalyzeFile", False },
    { "Mode.TwoMachines", False },
    { "Mode.MachineMatch", False },
    { "Mode.ICSClient", False },
    { "View.ICStextmenu", False },
    { "View.ICSInputBox", False },
    { "View.OpenChatWindow", False },
    { "Action.", False },
    { "Edit.Revert", False },
    { "Edit.Annotate", False },
    { "Engine.Engine#1Settings", False },
    { "Engine.Engine#2Settings", False },
    { "Engine.MoveNow", False },
    { "Engine.RetractMove", False },
    { "Options.ICS", False },
#ifndef OPTIONSDIALOG
    { "Options.AutoFlag", False },
    { "Options.AutoFlip View", False },
//    { "Options.ICSAlarm", False },
    { "Options.MoveSound", False },
    { "Options.HideThinking", False },
    { "Options.PeriodicUpdates", False },
    { "Options.PonderNextMove", False },
#endif
    { "Engine.Hint", False },
    { "Engine.Book", False },
    { NULL, False }
};

Enables gnuEnables[] = {
    { "Mode.ICSClient", False },
    { "View.ICStextmenu", False },
    { "View.ICSInputBox", False },
    { "View.OpenChatWindow", False },
    { "Action.Accept", False },
    { "Action.Decline", False },
    { "Action.Rematch", False },
    { "Action.Adjourn", False },
    { "Action.StopExamining", False },
    { "Action.StopObserving", False },
    { "Action.UploadtoExamine", False },
    { "Edit.Revert", False },
    { "Edit.Annotate", False },
    { "Options.ICS", False },

    /* The next two options rely on SetCmailMode being called *after*    */
    /* SetGNUMode so that when GNU is being used to give hints these     */
    /* menu options are still available                                  */

    { "File.MailMove", False },
    { "File.ReloadCMailMessage", False },
    // [HGM] The following have been added to make a switch from ncp to GNU mode possible
    { "Mode.MachineWhite", True },
    { "Mode.MachineBlack", True },
    { "Mode.AnalysisMode", True },
    { "Mode.AnalyzeFile", True },
    { "Mode.TwoMachines", True },
    { "Mode.MachineMatch", True },
    { "Engine.Engine#1Settings", True },
    { "Engine.Engine#2Settings", True },
    { "Engine.Hint", True },
    { "Engine.Book", True },
    { "Engine.MoveNow", True },
    { "Engine.RetractMove", True },
    { "Action.", True },
    { NULL, False }
};

Enables cmailEnables[] = {
    { "Action.", True },
    { "Action.CallFlag", False },
    { "Action.Draw", True },
    { "Action.Adjourn", False },
    { "Action.Abort", False },
    { "Action.StopObserving", False },
    { "Action.StopExamining", False },
    { "File.MailMove", True },
    { "File.ReloadCMailMessage", True },
    { NULL, False }
};

Enables trainingOnEnables[] = {
  { "Edit.EditComment", False },
  { "Mode.Pause", False },
  { "Edit.Forward", False },
  { "Edit.Backward", False },
  { "Edit.ForwardtoEnd", False },
  { "Edit.BacktoStart", False },
  { "Engine.MoveNow", False },
  { "Edit.TruncateGame", False },
  { NULL, False }
};

Enables trainingOffEnables[] = {
  { "Edit.EditComment", True },
  { "Mode.Pause", True },
  { "Edit.Forward", True },
  { "Edit.Backward", True },
  { "Edit.ForwardtoEnd", True },
  { "Edit.BacktoStart", True },
  { "Engine.MoveNow", True },
  { "Engine.TruncateGame", True },
  { NULL, False }
};

Enables machineThinkingEnables[] = {
  { "File.LoadGame", False },
//  { "LoadNextGame", False },
//  { "LoadPreviousGame", False },
//  { "ReloadSameGame", False },
  { "Edit.PasteGame", False },
  { "File.LoadPosition", False },
//  { "LoadNextPosition", False },
//  { "LoadPreviousPosition", False },
//  { "ReloadSamePosition", False },
  { "Edit.PastePosition", False },
  { "Mode.MachineWhite", False },
  { "Mode.MachineBlack", False },
  { "Mode.TwoMachines", False },
//  { "MachineMatch", False },
  { "Engine.RetractMove", False },
  { NULL, False }
};

Enables userThinkingEnables[] = {
  { "File.LoadGame", True },
//  { "LoadNextGame", True },
//  { "LoadPreviousGame", True },
//  { "ReloadSameGame", True },
  { "Edit.PasteGame", True },
  { "File.LoadPosition", True },
//  { "LoadNextPosition", True },
//  { "LoadPreviousPosition", True },
//  { "ReloadSamePosition", True },
  { "Edit.PastePosition", True },
  { "Mode.MachineWhite", True },
  { "Mode.MachineBlack", True },
  { "Mode.TwoMachines", True },
//  { "MachineMatch", True },
  { "Engine.RetractMove", True },
  { NULL, False }
};

void
SetICSMode ()
{
  SetMenuEnables(icsEnables);

#if ZIPPY
  if (appData.zippyPlay && !appData.noChessProgram) { /* [DM] icsEngineAnalyze */
     EnableNamedMenuItem("Mode.AnalysisMode", True);
     EnableNamedMenuItem("Engine.Engine#1Settings", True);
  }
#endif
}

void
SetNCPMode ()
{
  SetMenuEnables(ncpEnables);
}

void
SetGNUMode ()
{
  SetMenuEnables(gnuEnables);
}

void
SetCmailMode ()
{
  SetMenuEnables(cmailEnables);
}

void
SetTrainingModeOn ()
{
  SetMenuEnables(trainingOnEnables);
  if (appData.showButtonBar) {
    EnableButtonBar(False);
  }
  CommentPopDown();
}

void
SetTrainingModeOff ()
{
  SetMenuEnables(trainingOffEnables);
  if (appData.showButtonBar) {
    EnableButtonBar(True);
  }
}

void
SetUserThinkingEnables ()
{
  if (appData.noChessProgram) return;
  SetMenuEnables(userThinkingEnables);
}

void
SetMachineThinkingEnables ()
{
  if (appData.noChessProgram) return;
  SetMenuEnables(machineThinkingEnables);
  switch (gameMode) {
  case MachinePlaysBlack:
  case MachinePlaysWhite:
  case TwoMachinesPlay:
    EnableNamedMenuItem(ModeToWidgetName(gameMode), True);
    break;
  default:
    break;
  }
}

void
GreyRevert (Boolean grey)
{
    MarkMenuItem("Edit.Revert", !grey);
    MarkMenuItem("Edit.Annotate", !grey);
}

char *
ModeToWidgetName (GameMode mode)
{
    switch (mode) {
      case BeginningOfGame:
	if (appData.icsActive)
	  return "Mode.ICSClient";
	else if (appData.noChessProgram ||
		 *appData.cmailGameName != NULLCHAR)
	  return "Mode.EditGame";
	else
	  return "Mode.MachineBlack";
      case MachinePlaysBlack:
	return "Mode.MachineBlack";
      case MachinePlaysWhite:
	return "Mode.MachineWhite";
      case AnalyzeMode:
	return "Mode.AnalysisMode";
      case AnalyzeFile:
	return "Mode.AnalyzeFile";
      case TwoMachinesPlay:
	return "Mode.TwoMachines";
      case EditGame:
	return "Mode.EditGame";
      case PlayFromGameFile:
	return "File.LoadGame";
      case EditPosition:
	return "Mode.EditPosition";
      case Training:
	return "Mode.Training";
      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case IcsObserving:
      case IcsIdle:
      case IcsExamining:
	return "Mode.ICSClient";
      default:
      case EndOfGame:
	return NULL;
    }
}

void
InitMenuMarkers()
{
#ifndef OPTIONSDIALOG
    if (appData.alwaysPromoteToQueen) {
	MarkMenuItem("Options.Always Queen", True);
    }
    if (appData.animateDragging) {
	MarkMenuItem("Options.Animate Dragging", True);
    }
    if (appData.animate) {
	MarkMenuItem("Options.Animate Moving", True);
    }
    if (appData.autoCallFlag) {
	MarkMenuItem("Options.Auto Flag", True);
    }
    if (appData.autoFlipView) {
	XtSetValues(XtNameToWidget(menuBarWidget,"Options.Auto Flip View", True);
    }
    if (appData.blindfold) {
	MarkMenuItem("Options.Blindfold", True);
    }
    if (appData.flashCount > 0) {
	MarkMenuItem("Options.Flash Moves", True);
    }
#if HIGHDRAG
    if (appData.highlightDragging) {
	MarkMenuItem("Options.Highlight Dragging", True);
    }
#endif
    if (appData.highlightLastMove) {
	MarkMenuItem("Options.Highlight Last Move", True);
    }
    if (appData.highlightMoveWithArrow) {
	MarkMenuItem("Options.Arrow", True);
    }
//    if (appData.icsAlarm) {
//	MarkMenuItem("Options.ICS Alarm", True);
//    }
    if (appData.ringBellAfterMoves) {
	MarkMenuItem("Options.Move Sound", True);
    }
    if (appData.oneClick) {
	MarkMenuItem("Options.OneClick", True);
    }
    if (appData.periodicUpdates) {
	MarkMenuItem("Options.Periodic Updates", True);
    }
    if (appData.ponderNextMove) {
	MarkMenuItem("Options.Ponder Next Move", True);
    }
    if (appData.popupExitMessage) {
	MarkMenuItem("Options.Popup Exit Message", True);
    }
    if (appData.popupMoveErrors) {
	MarkMenuItem("Options.Popup Move Errors", True);
    }
//    if (appData.premove) {
//	MarkMenuItem("Options.Premove", True);
//    }
    if (appData.showCoords) {
	MarkMenuItem("Options.Show Coords", True);
    }
    if (appData.hideThinkingFromHuman) {
	MarkMenuItem("Options.Hide Thinking", True);
    }
    if (appData.testLegality) {
	MarkMenuItem("Options.Test Legality", True);
    }
#endif
    if (saveSettingsOnExit) {
	MarkMenuItem("Options.SaveSettingsonExit", True);
    }
}


