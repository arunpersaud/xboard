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
    {N_("Edit Game      Ctrl+E"),        "Edit Game", EditGameEvent},
    {N_("Edit Position   Ctrl+Shift+E"), "Edit Position", EditPositionEvent},
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
	nrOfMenuItems++;
	mi++;
    }

    if(!strcmp(mb->name, "Engine")) AppendEnginesToMenu(appData.recentEngineList);
}

void
CreateMainMenus (Menu *mb)
{
    char menuName[MSG_SIZ];

    while (mb->name != NULL) {
        safeStrCpy(menuName, "menu", sizeof(menuName)/sizeof(menuName[0]) );
	strncat(menuName, mb->ref, MSG_SIZ - strlen(menuName) - 1);
	AddPullDownMenu(menuName, mb++);
    }
}


