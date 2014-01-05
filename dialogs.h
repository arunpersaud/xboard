/*
 * dialogs.h -- shared variables for generic dialog popup of XBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

// [HGM] Some remarks about the generic dialog creator of XBoard:
// GenericPopUp is needed to create a dialog from the lists of options supplied by the engines.
// But once it is there, it provides a very easy way for creating other settings dialogs as well,
// by letting XBoard provide its own, compiled-in lists of XBoard options (located in dialogs.c).
// The Option struct uses the following fields (E = for engine options, X = for XBoard options):
//                    Option types                  | XBoard-only ->
// TYPE    NAME       spin check string combo button box label list graph menu break end
// int     value       E     E    (h)    X/E         [w]       (h)   (h)
// int     min        X/E         (2)    (3)         (1)  (1)  (1)   (1)  (3)   (1)  (4)
// int     max        X/E   (w)   (w)    (w)   (w)   (w)  (w)  (w)   (w)
// void*   handle     X/E   X/E   X/E    X/E   X/E    X    X    X     X    X
// void*   target      X     X     X     X/C    C          X    X     C    C
// char*   textValue               E     X/E    *
// char ** choice                        X/E    *                          X
// enum    type       X/E   X/E   X/E    X/E    X     X    X    X     X    X     X    X
// char[]  name       X/E   X/E   X/E    X/E    X          X    X     X    X
// File and Path options are like String (but get a browse button added in the dialog), and Slider
// is like Spin. Menu can be PopUp or PopDown; both need the COMBO_CALLBACK bit (3) set!
// (h) or (w) means the field optionally (when non-null) specifies the height or width of the main
// control element (excluding accompanying description texts). [w] means the width is written there.
// C specifies the 'target' is a user-supplied callback function, which will be executed when the
// option is exercised.


/* Flags Option.min used (2) for TextBox (-string): */
#define T_VSCRL		(1 << 0)
#define T_HSCRL		(1 << 1)
#define T_FILL		(1 << 2)
#define T_WRAP		(1 << 3)
#define T_TOP		(1 << 4)

/* Flags Option.min used (3) for ComboBox (-combo) and menus (PopUp, PopDown): */
#define COMBO_CALLBACK	(1 << 0)
#define NO_GETTEXT	(1 << 2)

/* Flags for Option.min used (1) for Button, SaveButton, ListBox, Label: */
#define SAME_ROW	(1 << 0) /* also in Break & EndMark */
#define BORDER		(1 << 1) /* Label */
#define FIX_H		(1 << 1) /* in other, this bit specifies top and botom of the control chain to same window edge */
#define B2B		(1 << 2) /* chain bottom to bottom (by default, no chaining is done) */
#define T2T		(1 << 3)
#define R2R		(1 << 4)
#define L2R		(1 << 5)
#define R2L		(1 << 6)
#define L2L		(1 << 7)
#define TT		(T2T|FIX_H) /* useful combinations: 0xA = entirely to top */
#define BB		(B2B|FIX_H) /*   6 = entirely to bottom */
#define TB		(B2B|T2T)   /*   0xC = absorb all vertical size change */
#define LL		(L2L|R2L)   /*   0xC0 = entirely to left */
#define RR		(L2R|R2R)   /*   0x30 = entirely to right */
#define LR		(L2L|R2R)   /*   0x90 = absorb all horizontal size change */

/* Flags for Option.min used (4) for EndMark: */
#define NO_OK		(1 << 1)
#define NO_CANCEL	(1 << 2)

#define MODAL 1
#define NONMODAL 0

/* Board widget numbers, MUST correspond to mainOptions array */

#define W_MENU   0  // main menu bar
#define W_ENGIN  6  // engine menu
#define W_TITLE 10
#define W_WHITE 12
#define W_BLACK 13
#define W_SMALL 15  // title in small layout
#define W_MESSG 16
#define W_BUTTON 17 // button bar
#define W_PAUSE 20
#define W_BOARD 24
#define W_MENUW 25
#define W_MENUB 26
#define W_DROP  27  // drop (popup) menu

typedef enum {  // identifier of dialogs done by GenericPopup
TransientDlg=0, // transient: grabs mouse events and is destroyed at pop-down (so other dialog can use this ID next time)
CommentDlg, TagsDlg, TextMenuDlg, InputBoxDlg, ChatDlg, DummyDlg, HistoryDlg, // persistent: no grab and reused
GameListDlg,
EngOutDlg,
EvalGraphDlg,
PromoDlg,       // this and beyond are destroyed at pop-down
ErrorDlg,
AskDlg,         // this and beyond do grab mouse events (and are destroyed)
FatalDlg,
BoardWindow,
BrowserDlg,
NrOfDialogs     // dummy for total
} DialogClass;

typedef int MemoCallback (Option *opt, int n, int x, int y, char *text, int index);
typedef Option *PointerCallback(int n, int x, int y);
typedef void ListBoxCallback(int n, int selected);
typedef void ButtonCallback(int n);
typedef int OKCallback(int n);

extern char commentTranslations[];
extern char historyTranslations[];
//extern Pixel timerBackgroundPixel;
extern int values[];
extern ChessProgramState *currentCps;
extern int dialogError;
extern ButtonCallback *comboCallback;
extern void *userLogo;

extern WindowPlacement wpComment, wpTags, wpMoveHistory, wpMain, wpDualBoard;
extern char *marked[];
extern Boolean shellUp[];
extern Option textOptions[], typeOptions[], dualOptions[], mainOptions[];


int DialogExists P((DialogClass n));
int GenericPopUp P((Option *option, char *title, DialogClass dlgNr, DialogClass parent, int modal, int topLevel));
int GenericReadout P((Option *currentOption, int selected));
int PopDown P((DialogClass n));
void MarkMenu P((char *item, int dlgNr));
int AppendText P((Option *opt, char *s));
void HighlightText P((Option *opt, int from, int to, Boolean highlight));
void SetColor P((char *colorName, Option *box));
//void ColorChanged P((Widget w, XtPointer data, XEvent *event, Boolean *b));
void SetInsertPos P((Option *opt, int pos));
void HardSetFocus P((Option *opt));
void CursorAtEnd P((Option *opt));
void GetWidgetText  P((Option *opt, char **buf));
void SetWidgetText  P((Option *opt, char *buf, int n));
void GetWidgetState  P((Option *opt, int *state));
void SetWidgetState  P((Option *opt, int state));
void SetWidgetLabel P((Option *opt, char *buf));
void SetDialogTitle  P((DialogClass dlg, char *title));
void LoadListBox P((Option *opt, char *emptyText, int n1, int n2));
void HighlightListBoxItem P((Option *opt, int nr));
void HighlightWithScroll P((Option *opt, int sel, int max));
void ScrollToCursor P((Option *opt, int pos));
int  SelectedListBoxItem P((Option *opt));
void BoardFocus P((void));
void FocusOnWidget P((Option *opt, DialogClass dlg));
void UnCaret P((void));
void SetIconName P((DialogClass dlg, char *name));
int  ReadScroll P((Option *opt, float *top, float *bottom));
void SetScroll P((Option *opt, float f));
void AddHandler  P((Option *opt, DialogClass dlg, int nr));
void SendText P((int n));
void DisplayLogos P((Option *left, Option *right));
void Browse P((DialogClass dlg, char *label, char *proposed, char *ext,
                       Boolean pathFlag, char *mode, char **name, FILE **fp));
void FileNamePopUpWrapper P((char *label, char *def, char *filter, FileProc proc,
                  Boolean pathFlag, char *openMode, char **openName, FILE **openFP));

void InitDrawingParams P((int reload)); // in draw.c
void InitDrawingHandle P((Option *opt));
void ExposeRedraw P((Option *opt, int x, int y, int w, int h));
void DrawLogo P((Option *opt, void *logo));
void ErrorPopUp P((char *title, char *text, int modal));
int  ShiftKeys P((void));
void SetClockIcon P((int color));
void DelayedLoad P((void));
void DisplayTimerLabel P((Option *opt, char *color, long timer, int highlight));
void SetWindowTitle P((char *text, char *title, char *icon));
void SetupDropMenu P((void));
Option *BoardPopUp P((int squareSize, int lineGap, void *clockFontThingy));
void SlaveResize P((Option *opt));

int  SetCurrentComboSelection P((Option *opt));
void BoxAutoPopUp P((char *buf));
void IcsKey P((int n));
void ICSInputBoxPopUp P((void));
void LoadOptionsPopUp P((DialogClass parent));
void GameListOptionsPopUp P((DialogClass parent));
void RefreshColor P((int source, int n));
void SendString P((char *p));

// in ngamelist.c
int GameListClicks P((int direction));
void SetFilter P((void));
