/*
 * menus.h -- platform-indendent menu handling code for XBoard
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



typedef void MenuProc P((void));

typedef struct {
    char *string;
    char *accel;
    char *ref;
    MenuProc *proc;
    void *handle;
} MenuItem;

typedef struct {
    char *name;
    char *ref;
    MenuItem *mi;
} Menu;

typedef struct {
    char *name;
    Boolean value;
} Enables;

extern Menu menuBar[];

void ErrorPopUp P((char *title, char *text, int modal));
void AppendEnginesToMenu P((char *list));
void LoadGameProc P((void));
void LoadNextGameProc P((void));
void LoadPrevGameProc P((void));
void ReloadGameProc P((void));
void LoadPositionProc P((void));
void LoadNextPositionProc P((void));
void LoadPrevPositionProc P((void));
void ReloadPositionProc P((void));
void CopyPositionProc P((void));
void PastePositionProc P((void));
void CopyGameProc P((void));
void CopyGameListProc P((void));
void PasteGameProc P((void));
void SaveGameProc P((void));
void SavePositionProc P((void));
void ReloadCmailMsgProc P((void));
void QuitProc P((void));
void AnalyzeModeProc P((void));
void AnalyzeFileProc P((void));
void MatchProc P((void));
void MatchOptionsProc P((void));
void EditTagsProc P((void));
void EditCommentProc P((void));
void IcsInputBoxProc P((void));
void ChatProc P((void));
void AdjuWhiteProc P((void));
void AdjuBlackProc P((void));
void AdjuDrawProc P((void));
void RevertProc P((void));
void AnnotateProc P((void));
void AlwaysQueenProc P((void));
void AnimateDraggingProc P((void));
void AnimateMovingProc P((void));
void AutoflagProc P((void));
void AutoflipProc P((void));
void BlindfoldProc P((void));
void FlashMovesProc P((void));
void FlipViewProc P((void));
void HighlightDraggingProc P((void));
void HighlightLastMoveProc P((void));
void HighlightArrowProc P((void));
void MoveSoundProc P((void));
//void IcsAlarmProc P((void));
void OneClickProc P((void));
void PeriodicUpdatesProc P((void));
void PonderNextMoveProc P((void));
void PopupMoveErrorsProc P((void));
void PopupExitMessageProc P((void));
//void PremoveProc P((void));
void ShowCoordsProc P((void));
void ShowThinkingProc P((void));
void HideThinkingProc P((void));
void TestLegalityProc P((void));
void SaveSettingsProc P((void));
void SaveOnExitProc P((void));
void InfoProc P((void));
void ManProc P((void));
void GuideProc P((void));
void HomePageProc P((void));
void NewsPageProc P((void));
void BugReportProc P((void));
void AboutGameProc P((void));
void AboutProc P((void));
void DebugProc P((void));
void NothingProc P((void));
void ShuffleMenuProc P((void));
void EngineMenuProc P((void));
void UciMenuProc P((void));
void TimeControlProc P((void));
void OptionsProc P((void));
void NewVariantProc P((void));
void IcsTextProc P((void));
void LoadEngine1Proc P((void));
void LoadEngine2Proc P((void));
void FirstSettingsProc P((void));
void SecondSettingsProc P((void));
void GameListOptionsProc P((void));
void IcsOptionsProc P((void));
void SoundOptionsProc P((void));
void BoardOptionsProc P((void));
void LoadOptionsProc P((void));
void SaveOptionsProc P((void));
void SaveSettings P((char *));
void EditBookProc P((void));
void InitMenuMarkers P((void));
void ShowGameListProc P((void)); // in ngamelist.c
void HistoryShowProc P((void));  // in nhistory.c

// only here because it is the only header shared by xoptions.c and usystem.c
void SetTextColor P((char **cnames, int fg, int bg, int attr));
void ConsoleWrite P((char *message, int count));

// must be moved to xengineoutput.h

void EngineOutputProc P((void));
void EvalGraphProc P((void));

int SaveGameListAsText P((FILE *f));
void FileNamePopUp P((char *label, char *def, char *filter,
		      FileProc proc, char *openMode));

void AppendMenuItem P((char *text, int n));
MenuItem *MenuNameToItem P((char *menuName));
void SetMenuEnables P((Enables *enab));
void EnableButtonBar P((int state));
char *ModeToWidgetName P((GameMode mode));
void CreateAnimVars P((void));
void CopySomething P((char *s));


extern char  *gameCopyFilename, *gamePasteFilename;
extern Boolean saveSettingsOnExit;
extern char *settingsFileName;
extern int firstEngineItem;



#define CHECK (void *) 1
#define RADIO (void *) 2

#define OPTIONSDIALOG
#define INFOFILE     "xboard.info"
