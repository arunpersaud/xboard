/*
 * callback.c -- gtk-interface
 *
 * Copyright 2009, 2010 Free Software Foundation, Inc.
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

#include <gtk/gtk.h>
#include "common.h"
#include "xboard.h"
#include <errno.h>
#include "backend.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

extern GtkWidget  *about;
extern GtkWidget  *GUI_Window;
extern GtkWidget  *GUI_Aspect;
extern GtkWidget  *GUI_Menubar;
extern GtkWidget  *GUI_Timer;
extern GtkWidget  *GUI_Buttonbar;
extern GtkWidget  *GUI_Board;

extern char *programVersion;
extern int errorExitStatus;
extern int promotionUp;
extern int fromX;
extern int fromY;
extern int toX;
extern int toY;
extern int squareSize,lineGap;

extern int LoadGamePopUp P((FILE *f, int gameNumber, char *title));
extern int LoadPosition P((FILE *f, int gameNumber, char *title));
extern int SaveGame P((FILE *f, int gameNumber, char *title));
extern int SavePosition P((FILE *f, int gameNumber, char *title));

gboolean
ExposeProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  /* do resizing to a fixed aspect ratio */
  GtkRequisition w;
  int totalh=0,nw,nh;
  float ratio;
  int boardWidth,boardHeight,old,new;

  nw=GTK_WIDGET(object)->allocation.width;
  nh=GTK_WIDGET(object)->allocation.height;

  old=squareSize;
  squareSize  = nw/(BOARD_WIDTH*1.05+0.05);

  if(old!=squareSize)
    {
      lineGap = squareSize*0.05;

      boardWidth  = lineGap + BOARD_WIDTH  * (squareSize + lineGap);
      boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);

      /* get the height of the menus, etc. and calculate the aspect ratio */
      gtk_widget_size_request(GTK_WIDGET(GUI_Menubar),   &w);
      totalh += w.height;
      gtk_widget_size_request(GTK_WIDGET(GUI_Timer),   &w);
      totalh += w.height;
      gtk_widget_size_request(GTK_WIDGET(GUI_Buttonbar),   &w);
      totalh += w.height;

      ratio  = ((float)totalh+boardHeight)/((float)boardWidth) ;

      gtk_widget_set_size_request(GTK_WIDGET(GUI_Board),
				  boardWidth,boardHeight);

      gtk_aspect_frame_set (GTK_ASPECT_FRAME(GUI_Aspect),0,0,ratio,TRUE);

      /* recreate pieces with new size... TODO: keep svg in memory and just recreate pixmap instead of reloading files */
      CreatePieces();
    }
  return FALSE; /* return false, so that other expose events are called too */
}

void
QuitProc (object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  gtk_main_quit();
  ExitEvent(0);
}

/* Help Menu */
void InfoProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    char buf[MSG_SIZ];
    snprintf(buf, sizeof(buf), "xterm -e info --directory %s --directory . -f %s &",
	    INFODIR, INFOFILE);
    system(buf);
    return;
}

void ManProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    char buf[MSG_SIZ];
    snprintf(buf, sizeof(buf), "xterm -e man xboard &");
    system(buf);
    return;
}

void HintProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    HintEvent();
    return;
}

void BookProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    BookEvent();
    return;
}

void AboutProc (object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  GtkWidget               *about;

  const gchar *authors[] = {"Tim Mann <tim@tim-mann.org>",
			    "John Chanak",
			    "Evan Welsh <Evan.Welsh@msdw.com>",
			    "Elmar Bartel <bartel@informatik.tu-muenchen.de>",
			    "Jochen Wiedmann",
			    "Frank McIngvale",
			    "Hugh Fisher <Hugh.Fisher@cs.anu.edu.au>",
			    "Allessandro Scotti",
			    "H.G. Muller <h.g.muller AT hccnet DOT nl>",
			    "Eric Mullins <emwine AT earthlink DOT net>",
			    "Arun Persaud <arun@nubati.net>"};

  /* set up about window */
  about =  GTK_WIDGET(gtk_about_dialog_new());

  /* fill in some information */
  char buf[MSG_SIZ];
#if ZIPPY
  char *zippy = " (with Zippy code)";
#else
  char *zippy = "";
#endif
  sprintf(buf, "%s%s",  programVersion, zippy);

  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about),buf);

  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about),
				 "Copyright 1991 Digital Equipment Corporation\n"
				 "Enhancements Copyright 1992-2009 Free Software Foundation\n"
				 "Enhancements Copyright 2005 Alessandro Scotti");
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),"http://www.gnu.org/software/xboard/");
  gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about),authors);
  gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about),
					  " A. Alper (turkish)\n"
					  " A. Persaud (german)\n");

  /* end set up about window */
  gtk_dialog_run(GTK_DIALOG (about));
  gtk_widget_destroy(about);
}

/* End Help Menu */

/* Mode Menu */

void MachineWhiteProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    MachineWhiteEvent();
    return;
}

void MachineBlackProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    MachineBlackEvent();
    return;
}

void TwoMachinesProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    TwoMachinesEvent();
    return;
}

void IcsClientProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    IcsClientEvent();
    return;
}

void
AnalyzeFileProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if (!first.analysisSupport)
    {
      char buf[MSG_SIZ];
      snprintf(buf, sizeof(buf), _("%s does not support analysis"), first.tidy);
      DisplayError(buf, 0);
      return;
    };
  Reset(FALSE, TRUE);

  if (!appData.showThinking)
    ShowThinkingProc(NULL,NULL);

  AnalyzeFileEvent();
  FileNamePopUp(_("File to analyze"), "", LoadGamePopUp, "rb");
  AnalysisPeriodicEvent(1);
  return;
}

void 
AnalyzeModeProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    char buf[MSG_SIZ];

    if (!first.analysisSupport) 
      {
	snprintf(buf, sizeof(buf), _("%s does not support analysis"), first.tidy);
	DisplayError(buf, 0);
	return;
      }
    /* [DM] icsEngineAnalyze [HGM] This is horrible code; reverse the gameMode and isEngineAnalyze tests! */
    if (appData.icsActive) 
      {
        if (gameMode != IcsObserving) 
	  {
	    sprintf(buf,_("You are not observing a game"));
	    DisplayError(buf, 0);
            /* secure check */
            if (appData.icsEngineAnalyze) 
	      {
		if (appData.debugMode)
		  fprintf(debugFP, _("Found unexpected active ICS engine analyze \n"));
                ExitAnalyzeMode();
                ModeHighlight();
	      }
            return;
	  }
        /* if enable, use want disable icsEngineAnalyze */
        if (appData.icsEngineAnalyze) 
	  {
	    ExitAnalyzeMode();
	    ModeHighlight();
	    return;
	  }
        appData.icsEngineAnalyze = TRUE;
        if (appData.debugMode)
	  fprintf(debugFP, _("ICS engine analyze starting... \n"));
      }
    if (!appData.showThinking)
      ShowThinkingProc(NULL,NULL);
    
    AnalyzeModeEvent();
    return;
}

void 
EditGameProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  EditGameEvent();
  return;
}

void 
EditPositionProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  EditPositionEvent();
  return;
}

void 
TrainingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  TrainingEvent();
  return;
}



/* End Mode Menu */

/*
 * File menu
 */

void
LoadNextGameProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadGame(1);
    return;
}

void
LoadPrevGameProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadGame(-1);
    return;
}

void
ReloadGameProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadGame(0);
    return;
}

void
LoadNextPositionProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadPosition(1);
    return;
}

void
LoadPrevPositionProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadPosition(-1);
    return;
}

void
ReloadPositionProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ReloadPosition(0);
    return;
}

void
LoadPositionProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if (gameMode == AnalyzeMode || gameMode == AnalyzeFile)
    {
      Reset(FALSE, TRUE);
    };

  FileNamePopUp(_("Load position file name?"), "", LoadPosition, "rb");
  return;
}

void
SaveGameProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  FileNamePopUp(_("Save game file name?"),
		DefaultFileName(appData.oldSaveStyle ? "game" : "pgn"),
		SaveGame, "a");
  return;
}

void
SavePositionProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  FileNamePopUp(_("Save position file name?"),
		DefaultFileName(appData.oldSaveStyle ? "pos" : "fen"),
		SavePosition, "a");
  return;
}


/* End File Menu */

void AcceptProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    AcceptEvent();
    return;
}

void DeclineProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    DeclineEvent();
    return;
}

void RematchProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    RematchEvent();
    return;
}

void CallFlagProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    CallFlagEvent();
    return;
}

void DrawProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    DrawEvent();
    return;
}

void AbortProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    AbortEvent();
    return;
}

void AdjournProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    AdjournEvent();
    return;
}

void ResignProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ResignEvent();
    return;
}

void StopObservingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    StopObservingEvent();
    return;
}

void StopExaminingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    StopExaminingEvent();
    return;
}

void AdjuWhiteProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    UserAdjudicationEvent(+1);
    return;
}

void AdjuBlackProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    UserAdjudicationEvent(-1);
    return;
}

void AdjuDrawProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    UserAdjudicationEvent(0);
    return;
}

void BackwardProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    BackwardEvent();
    return;
}

void ForwardProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ForwardEvent();
    return;
}

void ToStartProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ToStartEvent();
    return;
}

void ToEndProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ToEndEvent();
    return;
}

void RevertProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    RevertEvent(False);
    return;
}

void TruncateGameProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    TruncateGameEvent();
    return;
}

void MoveNowProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    MoveNowEvent();
    return;
}

void RetractMoveProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    RetractMoveEvent();
    return;
}

/* Option Menu */
void 
AutocommProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.autoComment = !appData.autoComment;
    return;
}

void 
AutoflagProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.autoCallFlag = !appData.autoCallFlag;
    return;
}

void 
AutoflipProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.autoFlipView = !appData.autoFlipView;
    return;
}

void 
ShowThinkingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.showThinking = !appData.showThinking;
    ShowThinkingEvent();

    return;
}

void 
HideThinkingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.hideThinkingFromHuman = !appData.hideThinkingFromHuman;
    ShowThinkingEvent();

    return;
}

void 
FlipViewProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    flipView = !flipView;
    DrawPosition(True, NULL);
    return;
}

void 
AlwaysQueenProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.alwaysPromoteToQueen = !appData.alwaysPromoteToQueen;
  return;
}

void 
AnimateDraggingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.animateDragging = !appData.animateDragging;

  if (appData.animateDragging) 
    {
      // TODO convert to gtk
      //      CreateAnimVars();
    };

  return;
}

void 
AnimateMovingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.animate = !appData.animate;
  
  if (appData.animate) 
    {
      // TODO convert to gtk
      //      CreateAnimVars();
    };

  return;
}

void 
AutobsProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.autoObserve = !appData.autoObserve;
  return;
}

void 
AutoraiseProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.autoRaiseBoard = !appData.autoRaiseBoard;
  return;
}

void 
AutosaveProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.autoSaveGames = !appData.autoSaveGames;
  return;
}

void 
BlindfoldProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.blindfold = !appData.blindfold;
  DrawPosition(True, NULL);
  return;
}

void 
TestLegalityProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.testLegality = !appData.testLegality;
  return;
}

void 
FlashMovesProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if (appData.flashCount == 0) 
    {
      appData.flashCount = 3;
    }
  else 
    {
      appData.flashCount = -appData.flashCount;
    };
  
    // TODO: check if this is working correct*/
    return;
}

#if HIGHDRAG
void 
HighlightDraggingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  /* TODO: connect to option menu */
  appData.highlightDragging = !appData.highlightDragging;
  return;
}
#endif

void 
HighlightLastMoveProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.highlightLastMove = !appData.highlightLastMove;
  return;
}

void 
IcsAlarmProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.icsAlarm = !appData.icsAlarm;
  return;
}

void 
MoveSoundProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.ringBellAfterMoves = !appData.ringBellAfterMoves;
  return;
}

void 
OldSaveStyleProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.oldSaveStyle = !appData.oldSaveStyle;
  return;
}

void 
PeriodicUpdatesProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  PeriodicUpdatesEvent(!appData.periodicUpdates);
  return;
}

void 
PremoveProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.premove = !appData.premove;
  return;
}

void 
QuietPlayProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.quietPlay = !appData.quietPlay;
  return;
}


void 
PonderNextMoveProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  PonderNextMoveEvent(!appData.ponderNextMove);
  return;
}

void 
PopupExitMessageProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.popupExitMessage = !appData.popupExitMessage;
  return;
}

void 
PopupMoveErrorsProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.popupMoveErrors = !appData.popupMoveErrors;
  return;
}



/* end option menu */

gboolean CloseWindowProc(GtkWidget *button)
{
    gtk_widget_destroy(gtk_widget_get_toplevel(button));
    return TRUE;
}

void
ResetProc (object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  ResetGameEvent();
  return;
}

void WhiteClockProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    if (gameMode == EditPosition || gameMode == IcsExamining) {
	SetWhiteToPlayEvent();
    } else if (gameMode == IcsPlayingBlack || gameMode == MachinePlaysWhite) {
	CallFlagEvent();
    }
}

void BlackClockProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    if (gameMode == EditPosition || gameMode == IcsExamining) {
	SetBlackToPlayEvent();
    } else if (gameMode == IcsPlayingWhite || gameMode == MachinePlaysBlack) {
	CallFlagEvent();
    }
}


void ShowCoordsProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.showCoords = !appData.showCoords;

    DrawPosition(True, NULL);
}

void ErrorPopDownProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  gtk_widget_destroy(GTK_WIDGET(object));
  ErrorPopDown();
}

void PauseProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    // todo this toggling of the pause button doesn't seem to work?
    // e.g. select pause from buttonbar doesn't activate menumode.pause
  PauseEvent();
}


void LoadGameProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  FileNamePopUp(_("Load game file name?"),"",LoadGamePopUp,"rb");
  return;
}


/*************
 * EVENTS
 *************/

void EventProc(window, event, data)
     GtkWindow *window;
     GdkEvent *event;
     gpointer data;
{
  /* todo do we still need this?
    if (!XtIsRealized(widget))
      return;
  */

    switch (event->type) {
      case GDK_EXPOSE:
	if (event->expose.count > 0) return;  /* no clipping is done */
	DrawPosition(True, NULL);
	break;
      default:
	return;
    }
}


/*
 * event handler for parsing user moves
 */
void UserMoveProc(window, event, data)
     GtkWindow *window;
     GdkEvent *event;
     gpointer data;
{
    if (errorExitStatus != -1) return;

    if (promotionUp) 
      {
	if (event->type == GDK_BUTTON_PRESS) 
	  {
	    promotionUp = False;
	    ClearHighlights();
	    fromX = fromY = -1;
	  }
	else 
	  {
	    return;
	  }
      }
    
    // [HGM] mouse: the rest of the mouse handler is moved to the backend, and called here
    if(event->type == GDK_BUTTON_PRESS)   LeftClick(Press,   (int)event->button.x, (int)event->button.y);
    if(event->type == GDK_BUTTON_RELEASE) LeftClick(Release, (int)event->button.x, (int)event->button.y);

    return;
}

void GetMoveListProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  appData.getMoveList = !appData.getMoveList;

  if (appData.getMoveList)
    {
      GetMoveListEvent();
    }

  // gets set automatically? if we set it with set_active we end up in an endless loop switching between 0 and 1
  //  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (object),(gboolean) appData.getMoveList );

  return;
}
