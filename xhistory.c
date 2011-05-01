/*
 * xhistory.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>

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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/cursorfont.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Viewport.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "xhistory.h"
#include "gettext.h"
#include "xevalgraph.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

#define _LL_ 100

struct History{
  String *Nr,*white,*black;
  int     aNr;  /* space actually alocated */
  Widget mvn,mvw,mvb,vbox,viewport,sh;
  char Up;
};

struct History *hist=0;
String dots=" ... ";
Position gameHistoryX, gameHistoryY;
Dimension gameHistoryW, gameHistoryH;

void
HistoryPopDown(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
  Arg args[16];
  int j;
  if(hist) {

    // [HGM] remember old position
    j = 0;
    XtSetArg(args[j], XtNx, &gameHistoryX);  j++;
    XtSetArg(args[j], XtNy, &gameHistoryY);  j++;
    XtSetArg(args[j], XtNwidth, &gameHistoryW);  j++;
    XtSetArg(args[j], XtNheight, &gameHistoryH);  j++;
    XtGetValues(hist->sh, args, j);
    wpMoveHistory.x = gameHistoryX - 4;
    wpMoveHistory.y = gameHistoryY - 23;
    wpMoveHistory.width  = gameHistoryW;
    wpMoveHistory.height = gameHistoryH;

    XtPopdown(hist->sh);
    hist->Up=False;
  }
  j=0;
  XtSetArg(args[j], XtNleftBitmap, None); j++;
  XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Move History"),
		args, j);
}

void HistoryMoveProc(Widget w, XtPointer closure, XtPointer call_data)
{
    int to;
    XawListReturnStruct *R = (XawListReturnStruct *) call_data;
    if (w == hist->mvn || w == hist->mvw) {
      to=2*R->list_index-1;
      ToNrEvent(to);
    }
    else if (w == hist->mvb) {
      to=2*R->list_index;
      ToNrEvent(to);
    }
}

void HistoryAlloc(int len){
  int i;
  if(hist){
    free(hist->Nr[0]);free(hist->white[0]);free(hist->black[0]);
    free(hist->Nr);free(hist->white);free(hist->black);
  }
  else{
    hist=(struct History*)malloc(sizeof(struct History));
  }
    hist->aNr=len;
    hist->Nr=(String*)malloc(hist->aNr*sizeof(String*));
    hist->white=(String*)malloc(hist->aNr*sizeof(String*));
    hist->black=(String*)malloc(hist->aNr*sizeof(String*));

    hist->Nr[0]=(String)malloc(hist->aNr*6);
    hist->white[0]=(String)malloc(hist->aNr*MOVE_LEN);
    hist->black[0]=(String)malloc(hist->aNr*MOVE_LEN);

      sprintf(hist->Nr[0],"    ");
      sprintf(hist->white[0],_("White "));
      sprintf(hist->black[0],_("Black "));
    for(i=1;i<hist->aNr;i++){
      hist->Nr[i]= hist->Nr[i-1]+6;
      hist->white[i]= hist->white[i-1]+MOVE_LEN;
      hist->black[i]= hist->black[i-1]+MOVE_LEN;
      sprintf(hist->Nr[i],"%i.",i);
      sprintf(hist->white[i],"-----");
      sprintf(hist->black[i],"-----");
     }
}


/* Find empty space inside vbox form widget and redistribute it amongst
   the list widgets inside it. */
/* This version sort of works */
void
HistoryFill()
{
  Dimension w, bw;
  long extra;
  Position x, x1, x2;
  int j, dd;
  Arg args[16];

  j = 0;
  XtSetArg(args[j], XtNx, &x);  j++;
  XtSetArg(args[j], XtNwidth, &w);  j++;
  XtSetArg(args[j], XtNborderWidth, &bw);  j++;
  XtGetValues(hist->mvb, args, j);
  x1 = x + w + 2*bw;

  j = 0;
  XtSetArg(args[j], XtNwidth, &w);  j++;
  XtSetArg(args[j], XtNdefaultDistance, &dd);  j++;
  XtGetValues(hist->vbox, args, j);
  x2 = w - dd;

  extra = x2 - x1;
  if (extra < 0) {
    extra = -((-extra)/2);
  } else {
    extra = extra/2;
  }

  j = 0;
  XtSetArg(args[j], XtNwidth, &w);  j++;
  XtGetValues(hist->mvw, args, j);
  w += extra;
  j = 0;
  XtSetArg(args[j], XtNwidth, w);  j++;
  XtSetValues(hist->mvw, args, j);

  j = 0;
  XtSetArg(args[j], XtNwidth, &w);  j++;
  XtGetValues(hist->mvb, args, j);
  w += extra;
  j = 0;
  XtSetArg(args[j], XtNwidth, w);  j++;
  XtSetValues(hist->mvb, args, j);
}

void HistorySet(char movelist[][2*MOVE_LEN],int first,int last,int current){
  int i,b,m;
  Widget scroll;
  if(hist){
    if(last >= hist->aNr) HistoryAlloc(last+_LL_);
    for(i=0;i<last;i++) {
      if((i%2)==0) {
	if(movelist[i][0]) {
	  char* p = strchr(movelist[i], ' ');
	  if (p) {
	    strncpy(hist->white[i/2+1], movelist[i], p-movelist[i]);
	    hist->white[i/2+1][p-movelist[i]] = NULLCHAR;
	  } else {
	    safeStrCpy(hist->white[i/2+1],movelist[i], MOVE_LEN);
	  }
	} else {
	  safeStrCpy(hist->white[i/2+1],dots, MOVE_LEN);
      	}
      } else {
	if(movelist[i][0]) {
	  char* p = strchr(movelist[i], ' ');
	  if (p) {
	    strncpy(hist->black[i/2+1], movelist[i], p-movelist[i]);
	    hist->black[i/2+1][p-movelist[i]] = NULLCHAR;
	  } else {
	    safeStrCpy(hist->black[i/2+1],movelist[i], MOVE_LEN);
	  }
	} else {
	  safeStrCpy(hist->black[i/2+1],"", MOVE_LEN);
      	}
      }
    }
    safeStrCpy(hist->black[last/2+1],"", MOVE_LEN);
    b=first/2;
    m=(last+3)/2-b;
    XawFormDoLayout(hist->vbox, False);
    XawListChange(hist->mvn,hist->Nr+b,m,0,True);
    XawListChange(hist->mvw,hist->white+b,m,0,True);
    XawListChange(hist->mvb,hist->black+b,m,0,True);
    HistoryFill();
    XawFormDoLayout(hist->vbox, True);
    if(current<0){
      XawListUnhighlight(hist->mvw);
      XawListUnhighlight(hist->mvb);
    }
    else if((current%2)==0){
      XawListHighlight(hist->mvw, current/2+1);
      XawListUnhighlight(hist->mvb);
    }
    else{
      XawListUnhighlight(hist->mvw);
      if(current) XawListHighlight(hist->mvb, current/2+1);
      else XawListUnhighlight(hist->mvb);
    }
    if(scroll = XtNameToWidget(hist->sh, "*form.viewport.vertical")) { // [HGM] always scroll to bottom
      static char *params[3] = { "", "Forward", "FullLength" };
      static XEvent event;
      XtCallActionProc(scroll, "StartScroll", &event, params+1, 1);
      XtCallActionProc(scroll, "NotifyScroll", &event, params+2, 1);
      XtCallActionProc(scroll, "EndScroll", &event, params, 0);
    }
  }
  EvalGraphSet( first, last, current, pvInfoList ); // piggy-backed
}

Widget HistoryCreate()
{
    Arg args[16];
    int i,j;

    Widget layout,form,b_close;
    String trstr=
             "<Key>Up: BackwardProc() \n \
             <Key>Left: BackwardProc() \n \
             <Key>Down: ForwardProc() \n \
             <Key>Right: ForwardProc() \n";
    /*--- allocate memory for move-strings ---*/
    HistoryAlloc(_LL_);

    /*-------- create the widgets ---------------*/
    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
    XtSetArg(args[j], XtNallowShellResize, True);  j++;
#if TOPLEVEL
    hist->sh = historyShell =
      XtCreatePopupShell(_("Move list"), topLevelShellWidgetClass,
			 shellWidget, args, j);
#else
    hist->sh = historyShell =
      XtCreatePopupShell(_("Move list"), transientShellWidgetClass,
			 shellWidget, args, j);
#endif
    j = 0;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNdefaultDistance, 0);  j++;
      layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, hist->sh,
			    args, j);

    j = 0;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNresizable, True);  j++;

    form =
      XtCreateManagedWidget("form", formWidgetClass, layout, args, j);

    j = 0;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainRight);  j++;

    XtSetArg(args[j], XtNborderWidth, 1); j++;
    XtSetArg(args[j], XtNresizable, False);  j++;
    XtSetArg(args[j], XtNallowVert, True); j++;
    XtSetArg(args[j], XtNallowHoriz, True);  j++;
    XtSetArg(args[j], XtNforceBars, False); j++;
    XtSetArg(args[j], XtNheight, 280); j++;
    hist->viewport =
      XtCreateManagedWidget("viewport", viewportWidgetClass,
			    form, args, j);
    j=0;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNorientation,XtorientHorizontal);j++;
    hist->vbox =
      XtCreateManagedWidget("vbox", formWidgetClass, hist->viewport, args, j);

    j=0;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;

    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
    XtSetArg(args[j], XtNforceColumns, True);  j++;
    XtSetArg(args[j], XtNverticalList, True);  j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNresizable,True);j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    hist->mvn = XtCreateManagedWidget("movesn", listWidgetClass,
				      hist->vbox, args, j);
    XtAddCallback(hist->mvn, XtNcallback, HistoryMoveProc, (XtPointer) hist);

    j=0;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtRubber);  j++;

    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
    XtSetArg(args[j], XtNforceColumns, True);  j++;
    XtSetArg(args[j], XtNverticalList, True);  j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNresizable,True);j++;
    XtSetArg(args[j], XtNfromHoriz, hist->mvn);  j++;
    hist->mvw = XtCreateManagedWidget("movesw", listWidgetClass,
				      hist->vbox, args, j);
    XtAddCallback(hist->mvw, XtNcallback, HistoryMoveProc, (XtPointer) hist);

    j=0;
    XtSetArg(args[j], XtNtop, XtChainTop);  j++;
    XtSetArg(args[j], XtNbottom, XtChainTop);  j++;
    XtSetArg(args[j], XtNleft, XtRubber);  j++;
    XtSetArg(args[j], XtNright,  XtRubber);  j++;

    XtSetArg(args[j], XtNdefaultColumns, 1);  j++;
    XtSetArg(args[j], XtNforceColumns, True);  j++;
    XtSetArg(args[j], XtNverticalList, True);  j++;
    XtSetArg(args[j], XtNborderWidth, 0); j++;
    XtSetArg(args[j], XtNresizable,True);j++;
    XtSetArg(args[j], XtNfromHoriz, hist->mvw);  j++;
    hist->mvb = XtCreateManagedWidget("movesb", listWidgetClass,
				      hist->vbox, args, j);
    XtAddCallback(hist->mvb, XtNcallback, HistoryMoveProc, (XtPointer) hist);

    j=0;
    XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
    XtSetArg(args[j], XtNtop, XtChainBottom);  j++;
    XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
    XtSetArg(args[j], XtNright, XtChainLeft);  j++;
    XtSetArg(args[j], XtNfromVert, hist->viewport);  j++;
    b_close= XtCreateManagedWidget(_("Close"), commandWidgetClass,
				   form, args, j);
    XtAddCallback(b_close, XtNcallback, HistoryPopDown, (XtPointer) 0);

    XtAugmentTranslations(hist->sh,XtParseTranslationTable (trstr));

    XtRealizeWidget(hist->sh);
    CatchDeleteWindow(hist->sh, "HistoryPopDown");

    for(i=1;i<hist->aNr;i++){
      safeStrCpy(hist->white[i],dots, MOVE_LEN);
      safeStrCpy(hist->black[i],"", MOVE_LEN);
     }

    if(wpMoveHistory.width > 0) {
      gameHistoryW = wpMoveHistory.width;
      gameHistoryH = wpMoveHistory.height;
      gameHistoryX = wpMoveHistory.x;
      gameHistoryY = wpMoveHistory.y;
    }

  // [HGM] restore old position
  if(gameHistoryW > 0) {
  j = 0;
    XtSetArg(args[j], XtNx, gameHistoryX);  j++;
  XtSetArg(args[j], XtNy, gameHistoryY);  j++;
    XtSetArg(args[j], XtNwidth, gameHistoryW);  j++;
    XtSetArg(args[j], XtNheight, gameHistoryH);  j++;
  XtSetValues(hist->sh, args, j);
  }
    XtRealizeWidget(hist->sh);

    return hist->sh;
}

void
HistoryPopUp()
{
  Arg args[16];
  int j;

  if(!hist) HistoryCreate();

  XtPopup(hist->sh, XtGrabNone);

  j=0;
  XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
  XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Move History"),
		args, j);
  hist->Up=True;
}


void
HistoryShowProc(w, event, prms, nprms)
     Widget w;
     XEvent *event;
     String *prms;
     Cardinal *nprms;
{
  if (!hist) {
    HistoryCreate();
    HistoryPopUp();
  } else if (hist->Up) {
    HistoryPopDown(0,0,0);
  } else {
    HistoryPopUp();
  }
  ToNrEvent(currentMove);
}

Boolean
MoveHistoryIsUp()
{
  return hist && hist->Up;
}
