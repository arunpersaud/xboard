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

void IcsClientProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    IcsClientEvent();
    return;
}

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


/* End File Menu */

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
    RevertEvent();
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
void AutocommProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.autoComment = !appData.autoComment;
    return;
}

void AutoflagProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.autoCallFlag = !appData.autoCallFlag;
    return;
}

void AutoflipProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.autoFlipView = !appData.autoFlipView;
    return;
}

void ShowThinkingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.showThinking = !appData.showThinking;
    ShowThinkingEvent();

    return;
}

void HideThinkingProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    appData.hideThinkingFromHuman = !appData.hideThinkingFromHuman;
    ShowThinkingEvent();

    return;
}

void FlipViewProc(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    flipView = !flipView;
    DrawPosition(True, NULL);
    return;
}


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
  EngineOutputPopDown();
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
    int x, y;
    Boolean saveAnimate;
    static int second = 0;

    if (errorExitStatus != -1) return;

    if (event->type == GDK_BUTTON_PRESS) ErrorPopDown();

    if (promotionUp)
      {
	if (event->type == GDK_BUTTON_PRESS)
	  {
	    /* todo add promotionshellwidget
	       XtPopdown(promotionShell);
	       XtDestroyWidget(promotionShell); */
	    promotionUp = False;
	    ClearHighlights();
	    fromX = fromY = -1;
	  }
	else
	  {
	    return;
	  }
      }

    x = EventToSquare( (int)event->button.x, BOARD_WIDTH  );
    y = EventToSquare( (int)event->button.y, BOARD_HEIGHT );
    if (!flipView && y >= 0)
      {
	y = BOARD_HEIGHT - 1 - y;
      }
    if (flipView && x >= 0)
      {
	x = BOARD_WIDTH - 1 - x;
      }

    if (fromX == -1)
      {
	if (event->type == ButtonPress)
	  {
	    /* First square */
	    if (OKToStartUserMove(x, y))
	      {
		fromX = x;
		fromY = y;
		second = 0;
		DragPieceBegin(event->button.x, event->button.y);
		if (appData.highlightDragging)
		  {
		    SetHighlights(x, y, -1, -1);
		  }
	      }
	  }
	return;
      }

    /* fromX != -1 */
    if (event->type == GDK_BUTTON_PRESS && gameMode != EditPosition &&
	x >= 0 && y >= 0) {
	ChessSquare fromP;
	ChessSquare toP;
	/* Check if clicking again on the same color piece */
	fromP = boards[currentMove][fromY][fromX];
	toP = boards[currentMove][y][x];
	if ((WhitePawn <= fromP && fromP <= WhiteKing &&
	     WhitePawn <= toP && toP <= WhiteKing) ||
	    (BlackPawn <= fromP && fromP <= BlackKing &&
	     BlackPawn <= toP && toP <= BlackKing)) {
	    /* Clicked again on same color piece -- changed his mind */
	    second = (x == fromX && y == fromY);
	    if (appData.highlightDragging) {
		SetHighlights(x, y, -1, -1);
	    } else {
		ClearHighlights();
	    }
	    if (OKToStartUserMove(x, y)) {
		fromX = x;
		fromY = y;
		DragPieceBegin(event->button.x, event->button.y);
	    }
	    return;
	}
    }

    if (event->type == GDK_BUTTON_RELEASE &&	x == fromX && y == fromY)
      {
	DragPieceEnd(event->button.x, event->button.y);
	if (appData.animateDragging)
	  {
	    /* Undo animation damage if any */
	    DrawPosition(FALSE, NULL);
	  }
	if (second)
	  {
	    /* Second up/down in same square; just abort move */
	    second = 0;
	    fromX = fromY = -1;
	    ClearHighlights();
	    gotPremove = 0;
	    ClearPremoveHighlights();
	  }
	else
	  {
	    /* First upclick in same square; start click-click mode */
	    SetHighlights(x, y, -1, -1);
	  }
	return;
      }

    /* Completed move */
    toX = x;
    toY = y;
    saveAnimate = appData.animate;

    if (event->type == GDK_BUTTON_PRESS)
      {
	/* Finish clickclick move */
	if (appData.animate || appData.highlightLastMove)
	  {
	    SetHighlights(fromX, fromY, toX, toY);
	  }
	else
	  {
	    ClearHighlights();
	  }
      }
    else
      {
	/* Finish drag move */
	if (appData.highlightLastMove)
	  {
	    SetHighlights(fromX, fromY, toX, toY);
	  }
	else
	  {
	    ClearHighlights();
	  }
	DragPieceEnd(event->button.x, event->button.y);
	/* Don't animate move and drag both */
	appData.animate = FALSE;
      }

    if (IsPromotion(fromX, fromY, toX, toY))
      {
	if (appData.alwaysPromoteToQueen)
	  {
	    UserMoveEvent(fromX, fromY, toX, toY, 'q');
	    if (!appData.highlightLastMove || gotPremove) ClearHighlights();
	    if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
	    fromX = fromY = -1;
	  }
	else
	  {
	    SetHighlights(fromX, fromY, toX, toY);
	    PromotionPopUp();
	  }
      }
    else
      {
	UserMoveEvent(fromX, fromY, toX, toY, NULLCHAR);

	if (!appData.highlightLastMove || gotPremove) ClearHighlights();
	if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
	fromX = fromY = -1;
      }

    appData.animate = saveAnimate;
    if (appData.animate || appData.animateDragging) {
	/* Undo animation damage if needed */
	DrawPosition(FALSE, NULL);
    }

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
