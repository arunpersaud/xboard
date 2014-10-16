/*
 * frontend.h -- Interface exported by all XBoard front ends
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
 ** See the file ChangeLog for a revision history.  */

#ifndef XB_FRONTEND
#define XB_FRONTEND

#include <stdio.h>

char *T_ P((char *s));
void ModeHighlight P((void));
void SetICSMode P((void));
void SetGNUMode P((void));
void SetNCPMode P((void));
void SetCmailMode P((void));
void SetTrainingModeOn P((void));
void SetTrainingModeOff P((void));
void SetUserThinkingEnables P((void));
void SetMachineThinkingEnables P((void));
void DisplayTitle P((String title));
void DisplayMessage P((String message, String extMessage));
void DisplayMoveError P((String message));

void DisplayNote P((String message));

void DisplayInformation P((String message));
void AskQuestion P((String title, String question, String replyPrefix,
		    ProcRef pr));
void DisplayIcsInteractionTitle P((String title));
void ParseArgsFromString P((char *p));
void ParseArgsFromFile P((FILE *f));
void DrawPosition P((int fullRedraw, Board board));
void ResetFrontEnd P((void));
void NotifyFrontendLogin P((void));
void CommentPopUp P((String title, String comment));
void CommentPopDown P((void));
void EditCommentPopUp P((int index, String title, String text));
void ErrorPopDown P((void));
int  EventToSquare P((int x, int limit));
void DrawSeekAxis P(( int x, int y, int xTo, int yTo ));
void DrawSeekBackground P(( int left, int top, int right, int bottom ));
void DrawSeekText P((char *buf, int x, int y));
void DrawSeekDot P((int x, int y, int color));
void PopUpMoveDialog P((char first));

void RingBell P((void));
int  Roar P((void));
void PlayIcsWinSound P((void));
void PlayIcsLossSound P((void));
void PlayIcsDrawSound P((void));
void PlayIcsUnfinishedSound P((void));
void PlayAlarmSound P((void));
void PlayTellSound P((void));
int  PlaySoundFile P((char *name));
void PlaySoundByColor P((void));
void EchoOn P((void));
void EchoOff P((void));
void Raw P((void));
void Colorize P((ColorClass cc, int continuation));
char *InterpretFileName P((char *name, char *dir));
void DoSleep P((int n));
void DoEvents P((void));

char *UserName P((void));
char *HostName P((void));

int ClockTimerRunning P((void));
int StopClockTimer P((void));
void StartClockTimer P((long millisec));
void DisplayWhiteClock P((long timeRemaining, int highlight));
void DisplayBlackClock P((long timeRemaining, int highlight));
void UpdateLogos P((int display));

int LoadGameTimerRunning P((void));
int StopLoadGameTimer P((void));
void StartLoadGameTimer P((long millisec));
void AutoSaveGame P((void));

void ScheduleDelayedEvent P((DelayedEventCallback cb, long millisec));
DelayedEventCallback GetDelayedEvent P((void));
void CancelDelayedEvent P((void));
// [HGM] mouse: next six used by mouse handler, which was moved to backend
extern int fromX, fromY, toX, toY;
void PromotionPopUp P((char choice));
void DragPieceBegin P((int x, int y, Boolean instantly));
void DragPieceEnd P((int x, int y));
void DragPieceMove P((int x, int y));
void LeftClick P((ClickType c, int x, int y));
int  RightClick P((ClickType c, int x, int y, int *col, int *row));

int StartChildProcess P((char *cmdLine, char *dir, ProcRef *pr));
void DestroyChildProcess P((ProcRef pr, int/*boolean*/ signal));
void InterruptChildProcess P((ProcRef pr));
void RunCommand P((char *buf));

int OpenTelnet P((char *host, char *port, ProcRef *pr));
int OpenTCP P((char *host, char *port, ProcRef *pr));
int OpenCommPort P((char *name, ProcRef *pr));
int OpenLoopback P((ProcRef *pr));
int OpenRcmd P((char *host, char *user, char *cmd, ProcRef *pr));

typedef void (*InputCallback) P((InputSourceRef isr, VOIDSTAR closure,
				 char *buf, int count, int error));
/* pr == NoProc means the local keyboard */
InputSourceRef AddInputSource P((ProcRef pr, int lineByLine,
				 InputCallback func, VOIDSTAR closure));
void RemoveInputSource P((InputSourceRef isr));

/* pr == NoProc means the local display */
int OutputToProcess P((ProcRef pr, char *message, int count, int *outError));
int OutputToProcessDelayed P((ProcRef pr, char *message, int count,
			      int *outError, long msdelay));

void CmailSigHandlerCallBack P((InputSourceRef isr, VOIDSTAR closure,
				char *buf, int count, int error));

extern ProcRef cmailPR;
extern int shiftKey, controlKey;

/* in xgamelist.c or winboard.c */
void GLT_ClearList();
void GLT_DeSelectList();
void GLT_AddToList( char *name );
Boolean GLT_GetFromList( int index, char *name );

extern char lpUserGLT[];
extern char *homeDir;

/* these are in wgamelist.c */
void GameListPopUp P((FILE *fp, char *filename));
void GameListPopDown P((void));
void GameListHighlight P((int index));
void GameListDestroy P((void));
void GameListUpdate P((void));
FILE *GameFile P((void));

/* these are in wedittags.c */
void EditTagsPopUp P((char *tags, char **dest));
void TagsPopUp P((char *tags, char *msg));
void TagsPopDown P((void));

void ParseIcsTextColors P((void));
int  ICSInitScript P((void));
void StartAnalysisClock P((void));
void EngineOutputPopUp P((void));
void EgineOutputPopDown P((void));

void SetHighlights P((int fromX, int fromY, int toX, int toY));
void ClearHighlights P((void));
void SetPremoveHighlights P((int fromX, int fromY, int toX, int toY));
void ClearPremoveHighlights P((void));

void AnimateAtomicCapture P((Board board, int fromX, int fromY, int toX, int toY));
void ShutDownFrontEnd P((void));
void BoardToTop P((void));
void AnimateMove P((Board board, int fromX, int fromY, int toX, int toY));
void HistorySet P((char movelist[][2*MOVE_LEN], int first, int last, int current));
void FreezeUI P((void));
void ThawUI P((void));
void ChangeDragPiece P((ChessSquare piece));
void CopyFENToClipboard P((void));
extern char *programName;
extern int commentUp;
extern char *firstChessProgramNames;

void GreyRevert P((Boolean grey));
void EnableNamedMenuItem P((char *menuRef, int state));

typedef struct FrontEndProgramStats_TAG {
    int which;
    int depth;
    u64 nodes;
    int score;
    int time;
    char * pv;
    char * hint;
    int an_move_index;
    int an_move_count;
} FrontEndProgramStats;

void SetProgramStats P(( FrontEndProgramStats * stats )); /* [AS] */

void EngineOutputPopUp P((void));
void EngineOutputPopDown P((void));
int  EngineOutputIsUp P((void));
int  EngineOutputDialogExists P((void));
void EvalGraphPopUp P((void));
Boolean EvalGraphIsUp P((void));
int  EvalGraphDialogExists P((void));
void SlavePopUp P((void));
void ActivateTheme P((int new));
char *Col2Text P((int n));

/* these are in xhistory.c  */
Boolean MoveHistoryIsUp P((void));
void HistoryPopUp P((void));
void FindMoveByCharIndex P(( int char_index ));

#endif /* XB_FRONTEND */
