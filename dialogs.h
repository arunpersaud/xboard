/*
 * dialogs.h -- shared variables for generic dialog popup of XBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

typedef enum {
TransientDlg=0, CommentDlg, TagsDlg, TextMenuDlg, InputBoxDlg, ErrorDlg, BrowserDlg, HistoryDlg, NrOfDialogs
} DialogClass;

typedef void ButtonCallback(int n);
typedef int OKCallback(int n);

extern char commentTranslations[];
extern char historyTranslations[];
extern Pixel timerBackgroundPixel;
extern int values[];
extern ChessProgramState *currentCps;
extern int dialogError;
extern ButtonCallback *comboCallback;

extern WindowPlacement wpComment, wpTags, wpMoveHistory;
extern char *marked[];
extern Boolean shellUp[];
extern Option textOptions[], boxOptions[];


int DialogExists P((DialogClass n));
int GenericPopUp P((Option *option, char *title, DialogClass dlgNr));
int GenericReadout P((Option *currentOption, int selected));
int PopDown P((DialogClass n));
int AppendText P((Option *opt, char *s));
void SetColor P((char *colorName, Option *box));
void ColorChanged P((Widget w, XtPointer data, XEvent *event, Boolean *b));
void SetInsertPos P((Option *opt, int pos));
void HardSetFocus P((Option *opt));
void GetWidgetText  P((Option *opt, char **buf));
void SetWidgetText  P((Option *opt, char *buf, int n));
void GetWidgetState  P((Option *opt, int *state));
void SetWidgetState  P((Option *opt, int state));
void SetDialogTitle  P((DialogClass dlg, char *title));
void AddHandler  P((Option *opt, int nr));
void SendText P((int n));

void InitDrawingParams P(()); // in xboard.c
void ErrorPopUp P((char *title, char *text, int modal));
int  ShiftKeys P((void));

void BoxAutoPopUp P((char *buf));
void IcsKey P((int n));
void ICSInputBoxPopUp P((void));


