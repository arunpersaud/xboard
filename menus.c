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
#include "backendz.h"
#include "moves.h"
#include "xgamelist.h"
#include "xhistory.h"
#include "xedittags.h"
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
AnalyzeModeProc ()
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
      ShowThinkingProc();
#endif

    AnalyzeModeEvent();
}

void
AnalyzeFileProc ()
{
    if (!first.analysisSupport) {
      char buf[MSG_SIZ];
      snprintf(buf, sizeof(buf), _("%s does not support analysis"), first.tidy);
      DisplayError(buf, 0);
      return;
    }
//    Reset(FALSE, TRUE);
#ifndef OPTIONSDIALOG
    if (!appData.showThinking)
      ShowThinkingProc();
#endif
    AnalyzeFileEvent();
//    FileNamePopUp(_("File to analyze"), "", ".pgn .game", LoadGamePopUp, "rb");
    AnalysisPeriodicEvent(1);
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
    flipView = !flipView;
    DrawPosition(True, NULL);
}

void
SaveOnExitProc ()
{
    Arg args[16];

    saveSettingsOnExit = !saveSettingsOnExit;

    MarkMenuItem("Save Settings on Exit", saveSettingsOnExit);
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
ManProc ()
{   // called from menu
    ManInner(NULL, NULL, NULL, NULL);
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
    Arg args[16];

    PonderNextMoveEvent(!appData.ponderNextMove);
    MARK_MENU_ITEM("Ponder Next Move", appData.ponderNextMove);
}

void
AlwaysQueenProc ()
{
    appData.alwaysPromoteToQueen = !appData.alwaysPromoteToQueen;
    MARK_MENU_ITEM("Always Queen", appData.alwaysPromoteToQueen);
}

void
AnimateDraggingProc ()
{
    appData.animateDragging = !appData.animateDragging;

    if (appData.animateDragging) CreateAnimVars();
    MARK_MENU_ITEM("Animate Dragging", appData.animateDragging);
}

void
AnimateMovingProc ()
{
    appData.animate = !appData.animate;
    if (appData.animate) CreateAnimVars();
    MARK_MENU_ITEM("Animate Moving", appData.animate);
}

void
AutoflagProc ()
{
    appData.autoCallFlag = !appData.autoCallFlag;
    MARK_MENU_ITEM("Auto Flag", appData.autoCallFlag);
}

void
AutoflipProc ()
{
    appData.autoFlipView = !appData.autoFlipView;
    MARK_MENU_ITEM("Auto Flip View", appData.autoFlipView);
}

void
BlindfoldProc ()
{
    appData.blindfold = !appData.blindfold;
    MARK_MENU_ITEM("Blindfold", appData.blindfold);
    DrawPosition(True, NULL);
}

void
TestLegalityProc ()
{
    appData.testLegality = !appData.testLegality;
    MARK_MENU_ITEM("Test Legality", appData.testLegality);
}


void
FlashMovesProc ()
{
    if (appData.flashCount == 0) {
	appData.flashCount = 3;
    } else {
	appData.flashCount = -appData.flashCount;
    }
    MARK_MENU_ITEM("Flash Moves", appData.flashCount > 0);
}

#if HIGHDRAG
void
HighlightDraggingProc ()
{
    appData.highlightDragging = !appData.highlightDragging;
    MARK_MENU_ITEM("Highlight Dragging", appData.highlightDragging);
}
#endif

void
HighlightLastMoveProc ()
{
    appData.highlightLastMove = !appData.highlightLastMove;
    MARK_MENU_ITEM("Highlight Last Move", appData.highlightLastMove);
}

void
HighlightArrowProc ()
{
    appData.highlightMoveWithArrow = !appData.highlightMoveWithArrow;
    MARK_MENU_ITEM("Arrow", appData.highlightMoveWithArrow);
}

void
IcsAlarmProc ()
{
    appData.icsAlarm = !appData.icsAlarm;
//    MARK_MENU_ITEM("ICS Alarm", appData.icsAlarm);
}

void
MoveSoundProc ()
{
    appData.ringBellAfterMoves = !appData.ringBellAfterMoves;
    MARK_MENU_ITEM("Move Sound", appData.ringBellAfterMoves);
}

void
OneClickProc ()
{
    appData.oneClick = !appData.oneClick;
    MARK_MENU_ITEM("OneClick", appData.oneClick);
}

void
PeriodicUpdatesProc ()
{
    PeriodicUpdatesEvent(!appData.periodicUpdates);
    MARK_MENU_ITEM("Periodic Updates", appData.periodicUpdates);
}

void
PopupExitMessageProc ()
{
    appData.popupExitMessage = !appData.popupExitMessage;
    MARK_MENU_ITEM("Popup Exit Message", appData.popupExitMessage);
}

void
PopupMoveErrorsProc ()
{
    appData.popupMoveErrors = !appData.popupMoveErrors;
    MARK_MENU_ITEM("Popup Move Errors", appData.popupMoveErrors);
}

void
PremoveProc ()
{
    appData.premove = !appData.premove;
//    MARK_MENU_ITEM("Premove", appData.premove);
}

void
ShowCoordsProc ()
{
    appData.showCoords = !appData.showCoords;
    MARK_MENU_ITEM("Show Coords", appData.showCoords);
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
    Arg args[16];

    appData.hideThinkingFromHuman = !appData.hideThinkingFromHuman; // [HGM] thinking: taken out of ShowThinkingEvent
    ShowThinkingEvent();

    MARK_MENU_ITEM("Hide Thinking", appData.hideThinkingFromHuman);
}

/*
 *  Menu definition tables
 */

MenuItem fileMenu[] = {
    {N_("New Game        Ctrl+N"),        "New Game", ResetGameEvent},
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
    {N_("Mail Move"),            "Mail Move", MailMoveEvent},
    {N_("Reload CMail Message"), "Reload CMail Message", ReloadCmailMsgProc},
    {"----", NULL, NothingProc},
    {N_("Quit                 Ctr+Q"), "Exit", QuitProc},
    {NULL, NULL, NULL}
};

MenuItem editMenu[] = {
    {N_("Copy Game    Ctrl+C"),        "Copy Game", CopyGameProc},
    {N_("Copy Position Ctrl+Shift+C"), "Copy Position", CopyPositionProc},
    {N_("Copy Game List"),        "Copy Game List", CopyGameListProc},
    {"----", NULL, NothingProc},
    {N_("Paste Game    Ctrl+V"),        "Paste Game", PasteGameProc},
    {N_("Paste Position Ctrl+Shift+V"), "Paste Position", PastePositionProc},
    {"----", NULL, NothingProc},
    {N_("Edit Game      Ctrl+E"),        "Edit Game 2", EditGameEvent},
    {N_("Edit Position   Ctrl+Shift+E"), "Edit Position 2", EditPositionEvent},
    {N_("Edit Tags"),                    "Edit Tags", EditTagsProc},
    {N_("Edit Comment"),                 "Edit Comment", EditCommentProc},
    {N_("Edit Book"),                    "Edit Book", EditBookEvent},
    {"----", NULL, NothingProc},
    {N_("Revert              Home"), "Revert", RevertProc},
    {N_("Annotate"),                 "Annotate", AnnotateProc},
    {N_("Truncate Game  End"),       "Truncate Game", TruncateGameEvent},
    {"----", NULL, NothingProc},
    {N_("Backward         Alt+Left"),   "Backward", BackwardEvent},
    {N_("Forward           Alt+Right"), "Forward", ForwardEvent},
    {N_("Back to Start     Alt+Home"),  "Back to Start", ToStartEvent},
    {N_("Forward to End Alt+End"),      "Forward to End", ToEndEvent},
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
    {N_("Machine White  Ctrl+W"), "Machine White", MachineWhiteEvent},
    {N_("Machine Black  Ctrl+B"), "Machine Black", MachineBlackEvent},
    {N_("Two Machines   Ctrl+T"), "Two Machines", TwoMachinesEvent},
    {N_("Analysis Mode  Ctrl+A"), "Analysis Mode", AnalyzeModeProc},
    {N_("Analyze Game   Ctrl+G"), "Analyze File", AnalyzeFileProc },
    {N_("Edit Game         Ctrl+E"), "Edit Game", EditGameEvent},
    {N_("Edit Position      Ctrl+Shift+E"), "Edit Position", EditPositionEvent},
    {N_("Training"),      "Training", TrainingEvent},
    {N_("ICS Client"),    "ICS Client", IcsClientEvent},
    {"----", NULL, NothingProc},
    {N_("Machine Match"),         "Machine Match", MatchProc},
    {N_("Pause               Pause"),         "Pause", PauseEvent},
    {NULL, NULL, NULL}
};

MenuItem actionMenu[] = {
    {N_("Accept             F3"), "Accept", AcceptEvent},
    {N_("Decline            F4"), "Decline", DeclineEvent},
    {N_("Rematch           F12"), "Rematch", RematchEvent},
    {"----", NULL, NothingProc},
    {N_("Call Flag          F5"), "Call Flag", CallFlagEvent},
    {N_("Draw                F6"), "Draw", DrawEvent},
    {N_("Adjourn            F7"),  "Adjourn", AdjournEvent},
    {N_("Abort                F8"),"Abort", AbortEvent},
    {N_("Resign              F9"), "Resign", ResignEvent},
    {"----", NULL, NothingProc},
    {N_("Stop Observing  F10"), "Stop Observing", StopObservingEvent},
    {N_("Stop Examining  F11"), "Stop Examining", StopExaminingEvent},
    {N_("Upload to Examine"),   "Upload to Examine", UploadGameEvent},
    {"----", NULL, NothingProc},
    {N_("Adjudicate to White"), "Adjudicate to White", AdjuWhiteProc},
    {N_("Adjudicate to Black"), "Adjudicate to Black", AdjuBlackProc},
    {N_("Adjudicate Draw"),     "Adjudicate Draw", AdjuDrawProc},
    {NULL, NULL, NULL}
};

MenuItem engineMenu[] = {
    {N_("Load New Engine ..."), "Load Engine", LoadEngineProc},
    {"----", NULL, NothingProc},
    {N_("Engine #1 Settings ..."), "Engine #1 Settings", FirstSettingsProc},
    {N_("Engine #2 Settings ..."), "Engine #2 Settings", SecondSettingsProc},
    {"----", NULL, NothingProc},
    {N_("Hint"), "Hint", HintEvent},
    {N_("Book"), "Book", BookEvent},
    {"----", NULL, NothingProc},
    {N_("Move Now     Ctrl+M"),     "Move Now", MoveNowEvent},
    {N_("Retract Move  Ctrl+X"), "Retract Move", RetractMoveEvent},
    {NULL, NULL, NULL}
};

MenuItem optionsMenu[] = {
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
    {N_("XBoard Home Page"), "Home Page", HomePageProc},
    {N_("On-line User Guide"), "User Guide", GuideProc},
    {N_("Development News"), "News Page", NewsPageProc},
    {N_("e-Mail Bug Report"), "Bug Report", BugReportProc},
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

int
MenuToNumber(char *menuName)
{
    int i;
    for(i=0; i<nrOfMenuItems; i++)
	if(!strcmp(menuName, menuItemList[i].name)) return i;
    return -1;
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
	if(i == 0) AppendMenuItem("----", "----", NULL); // at least one valid item to add
	*p = 0;
	AppendMenuItem(list, "recent", (MenuProc *) i);
	i++; *p = '\n'; list = p + 1;
    }
}

void
AddPullDownMenu (char *name, Menu *mb)
{
    MenuItem *mi;

    CreateMenuButton(name, mb);

    mi = mb->mi;
    while (mi->string != NULL) {
	AppendMenuItem(mi->string, mi->ref, mi->proc);
	menuItemList[nrOfMenuItems].name = mi->ref;
	menuItemList[nrOfMenuItems].proc = mi->proc;
	if(strcmp(mi->string, "----")) nrOfMenuItems++;
	mi++;
    }

    if(!strcmp(mb->name, "Engine")) AppendEnginesToMenu(appData.recentEngineList);
}

void
CreateMainMenus (Menu *mb)
{
    char menuName[MSG_SIZ];

    while(menuItemList[nrOfMenuItems].name) nrOfMenuItems++; // skip any predefined items

    while (mb->name != NULL) {
        safeStrCpy(menuName, "menu", sizeof(menuName)/sizeof(menuName[0]) );
	strncat(menuName, mb->ref, MSG_SIZ - strlen(menuName) - 1);
	AddPullDownMenu(menuName, mb++);
    }
}

Enables icsEnables[] = {
    { "Mail Move", False },
    { "Reload CMail Message", False },
    { "Machine Black", False },
    { "Machine White", False },
    { "Analysis Mode", False },
    { "Analyze File", False },
    { "Two Machines", False },
    { "Machine Match", False },
#ifndef ZIPPY
    { "Hint", False },
    { "Book", False },
    { "Move Now", False },
#ifndef OPTIONSDIALOG
    { "Periodic Updates", False },
    { "Hide Thinking", False },
    { "Ponder Next Move", False },
#endif
#endif
    { "Engine #1 Settings", False },
    { "Engine #2 Settings", False },
    { "Load Engine", False },
    { "Annotate", False },
    { "Match", False },
    { NULL, False }
};

Enables ncpEnables[] = {
    { "Mail Move", False },
    { "Reload CMail Message", False },
    { "Machine White", False },
    { "Machine Black", False },
    { "Analysis Mode", False },
    { "Analyze File", False },
    { "Two Machines", False },
    { "Machine Match", False },
    { "ICS Client", False },
    { "ICStex", False },
    { "ICS Input Box", False },
    { "Action", False },
    { "Revert", False },
    { "Annotate", False },
    { "Engine #1 Settings", False },
    { "Engine #2 Settings", False },
    { "Move Now", False },
    { "Retract Move", False },
    { "ICS", False },
#ifndef OPTIONSDIALOG
    { "Auto Flag", False },
    { "Auto Flip View", False },
//    { "ICS Alarm", False },
    { "Move Sound", False },
    { "Hide Thinking", False },
    { "Periodic Updates", False },
    { "Ponder Next Move", False },
#endif
    { "Hint", False },
    { "Book", False },
    { NULL, False }
};

Enables gnuEnables[] = {
    { "ICS Client", False },
    { "ICStex", False },
    { "ICS Input Box", False },
    { "Accept", False },
    { "Decline", False },
    { "Rematch", False },
    { "Adjourn", False },
    { "Stop Examining", False },
    { "Stop Observing", False },
    { "Upload to Examine", False },
    { "Revert", False },
    { "Annotate", False },
    { "ICS", False },

    /* The next two options rely on SetCmailMode being called *after*    */
    /* SetGNUMode so that when GNU is being used to give hints these     */
    /* menu options are still available                                  */

    { "Mail Move", False },
    { "Reload CMail Message", False },
    // [HGM] The following have been added to make a switch from ncp to GNU mode possible
    { "Machine White", True },
    { "Machine Black", True },
    { "Analysis Mode", True },
    { "Analyze File", True },
    { "Two Machines", True },
    { "Machine Match", True },
    { "Engine #1 Settings", True },
    { "Engine #2 Settings", True },
    { "Hint", True },
    { "Book", True },
    { "Move Now", True },
    { "Retract Move", True },
    { "Action", True },
    { NULL, False }
};

Enables cmailEnables[] = {
    { "Action", True },
    { "Call Flag", False },
    { "Draw", True },
    { "Adjourn", False },
    { "Abort", False },
    { "Stop Observing", False },
    { "Stop Examining", False },
    { "Mail Move", True },
    { "Reload CMail Message", True },
    { NULL, False }
};

Enables trainingOnEnables[] = {
  { "Edit Comment", False },
  { "Pause", False },
  { "Forward", False },
  { "Backward", False },
  { "Forward to End", False },
  { "Back to Start", False },
  { "Move Now", False },
  { "Truncate Game", False },
  { NULL, False }
};

Enables trainingOffEnables[] = {
  { "Edit Comment", True },
  { "Pause", True },
  { "Forward", True },
  { "Backward", True },
  { "Forward to End", True },
  { "Back to Start", True },
  { "Move Now", True },
  { "Truncate Game", True },
  { NULL, False }
};

Enables machineThinkingEnables[] = {
  { "Load Game", False },
//  { "Load Next Game", False },
//  { "Load Previous Game", False },
//  { "Reload Same Game", False },
  { "Paste Game", False },
  { "Load Position", False },
//  { "Load Next Position", False },
//  { "Load Previous Position", False },
//  { "Reload Same Position", False },
  { "Paste Position", False },
  { "Machine White", False },
  { "Machine Black", False },
  { "Two Machines", False },
//  { "Machine Match", False },
  { "Retract Move", False },
  { NULL, False }
};

Enables userThinkingEnables[] = {
  { "Load Game", True },
//  { "Load Next Game", True },
//  { "Load Previous Game", True },
//  { "Reload Same Game", True },
  { "Paste Game", True },
  { "Load Position", True },
//  { "Load Next Position", True },
//  { "Load Previous Position", True },
//  { "Reload Same Position", True },
  { "Paste Position", True },
  { "Machine White", True },
  { "Machine Black", True },
  { "Two Machines", True },
//  { "Machine Match", True },
  { "Retract Move", True },
  { NULL, False }
};

void
SetICSMode ()
{
  SetMenuEnables(icsEnables);

#if ZIPPY
  if (appData.zippyPlay && !appData.noChessProgram) { /* [DM] icsEngineAnalyze */
     EnableMenuItem("Analysis Mode", True);
     EnableMenuItem("Engine #1 Settings", True);
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
    EnableMenuItem(ModeToWidgetName(gameMode), True);
    break;
  default:
    break;
  }
}

void
GreyRevert (Boolean grey)
{
    MarkMenuItem("Revert", !grey);
    MarkMenuItem("Annotate", !grey);
}

char *
ModeToWidgetName (GameMode mode)
{
    switch (mode) {
      case BeginningOfGame:
	if (appData.icsActive)
	  return "ICS Client";
	else if (appData.noChessProgram ||
		 *appData.cmailGameName != NULLCHAR)
	  return "Edit Game";
	else
	  return "Machine Black";
      case MachinePlaysBlack:
	return "Machine Black";
      case MachinePlaysWhite:
	return "Machine White";
      case AnalyzeMode:
	return "Analysis Mode";
      case AnalyzeFile:
	return "Analyze File";
      case TwoMachinesPlay:
	return "Two Machines";
      case EditGame:
	return "Edit Game";
      case PlayFromGameFile:
	return "Load Game";
      case EditPosition:
	return "Edit Position";
      case Training:
	return "Training";
      case IcsPlayingWhite:
      case IcsPlayingBlack:
      case IcsObserving:
      case IcsIdle:
      case IcsExamining:
	return "ICS Client";
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
	MarkMenuItem("Always Queen", True);
    }
    if (appData.animateDragging) {
	MarkMenuItem("Animate Dragging", True);
    }
    if (appData.animate) {
	MarkMenuItem("Animate Moving", True);
    }
    if (appData.autoCallFlag) {
	MarkMenuItem("Auto Flag", True);
    }
    if (appData.autoFlipView) {
	XtSetValues(XtNameToWidget(menuBarWidget,"Auto Flip View", True);
    }
    if (appData.blindfold) {
	MarkMenuItem("Blindfold", True);
    }
    if (appData.flashCount > 0) {
	MarkMenuItem("Flash Moves", True);
    }
#if HIGHDRAG
    if (appData.highlightDragging) {
	MarkMenuItem("Highlight Dragging", True);
    }
#endif
    if (appData.highlightLastMove) {
	MarkMenuItem("Highlight Last Move", True);
    }
    if (appData.highlightMoveWithArrow) {
	MarkMenuItem("Arrow", True);
    }
//    if (appData.icsAlarm) {
//	MarkMenuItem("ICS Alarm", True);
//    }
    if (appData.ringBellAfterMoves) {
	MarkMenuItem("Move Sound", True);
    }
    if (appData.oneClick) {
	MarkMenuItem("OneClick", True);
    }
    if (appData.periodicUpdates) {
	MarkMenuItem("Periodic Updates", True);
    }
    if (appData.ponderNextMove) {
	MarkMenuItem("Ponder Next Move", True);
    }
    if (appData.popupExitMessage) {
	MarkMenuItem("Popup Exit Message", True);
    }
    if (appData.popupMoveErrors) {
	MarkMenuItem("Popup Move Errors", True);
    }
//    if (appData.premove) {
//	MarkMenuItem("Premove", True);
//    }
    if (appData.showCoords) {
	MarkMenuItem("Show Coords", True);
    }
    if (appData.hideThinkingFromHuman) {
	MarkMenuItem("Hide Thinking", True);
    }
    if (appData.testLegality) {
	MarkMenuItem("Test Legality", True);
    }
#endif
    if (saveSettingsOnExit) {
	MarkMenuItem("Save Settings on Exit", True);
    }
}
