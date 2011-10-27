/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011 Free Software Foundation, Inc.
 *
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
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * ------------------------------------------------------------------------
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
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>

#include <gtk/gtk.h>


#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard.h"
#include "engineoutput.h"
#include "gettext.h"
#include "gtk_helper.h"


#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


// [HGM] pixmaps of some ICONS used in the engine-outut window
//#include "pixmaps/WHITE_14.xpm"
//#include "pixmaps/BLACK_14.xpm"
//#include "pixmaps/CLEAR_14.xpm"
//#include "pixmaps/UNKNOWN_14.xpm"
//#include "pixmaps/THINKING_14.xpm"
//#include "pixmaps/PONDER_14.xpm"
//#include "pixmaps/ANALYZING_14.xpm"

#ifdef SNAP
#include "wsnap.h"
#endif

#define _LL_ 100

Pixmap icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle
Widget outputField[2][7]; // [HGM] front-end array to translate output field to window handle
GtkWidget *outputFieldGTK[2][7]; // [HGM] front-end array to translate output field to window handle

static GtkBuilder *builder=NULL;
static GError *gtkerror=NULL; 
GtkWidget *engineOutputShellGTK=NULL;

void EngineOutputPopDown();
void engineOutputPopUp();
int  EngineOutputIsUp();
void SetEngineColorIcon( int which );

//extern WindowPlacement wpEngineOutput;

Position engineOutputX = -1, engineOutputY = -1;
Dimension engineOutputW, engineOutputH;
Widget engineOutputShell;
static int engineOutputDialogUp;

/* Module variables */
int  windowMode = 1;
static int currentPV, highTextStart[2], highTextEnd[2];

typedef struct {
    char * name;
    int which;
    int depth;
    u64 nodes;
    int score;
    int time;
    char * pv;
    char * hint;
    int an_move_index;
    int an_move_count;
} EngineOutputData;

//static void UpdateControls( EngineOutputData * ed );

void ReadIcon(char *pixData[], int iconNr)
{
    int r;

//	if ((r=XpmCreatePixmapFromData(xDisplay, XtWindow(outputField[0][nColorIcon]),
//				       pixData,
//				       &(icons[iconNr]),
//				       NULL, NULL /*&attr*/)) != 0) {
//	  fprintf(stderr, _("Error %d loading icon image\n"), r);
//	  exit(1);
//	}
}

static void InitializeEngineOutput()
{
//        ReadIcon(WHITE_14,   nColorWhite);
//        ReadIcon(BLACK_14,   nColorBlack);
//        ReadIcon(UNKNOWN_14, nColorUnknown);
//
//        ReadIcon(CLEAR_14,   nClear);
//        ReadIcon(PONDER_14,  nPondering);
//        ReadIcon(THINK_14,   nThinking);
//        ReadIcon(ANALYZE_14, nAnalyzing);
}

void DoSetWindowText(int which, int field, char *s_label)
{
    // which = 0 for 1st engine, 1 for second
    // field = 3 for engine name, 5 for NPS     
    if (field != 3 && field != 5) return;
    if (!GTK_IS_LABEL(outputFieldGTK[which][field])) return;    
    gtk_label_set_text(GTK_LABEL(outputFieldGTK[which][field]), s_label);
}

void SetEngineOutputTitle(char *title)
{
    gtk_window_set_title(GTK_WINDOW(engineOutputShellGTK), title);
}

void InsertIntoMemo( int which, char * text, int where )
{
	//XawTextBlock t;
	//Widget edit;

    gchar widgetname[50];
    GtkTextIter start;    
    
    strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
    GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);    
 
    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editGTK));            
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tb), &start);
    gtk_text_buffer_insert(tb, &start, text, -1);

	/* the backend adds \r\n, which is needed for winboard,
	 * for xboard we delete them again over here */
//	if(t.ptr = strchr(text, '\r')) *t.ptr = ' ';
//
//	t.ptr = text; t.firstPos = 0; t.length = strlen(text); t.format = XawFmt8Bit;
//	edit = XtNameToWidget(engineOutputShell, which ? "*form2.text" : "*form.text");
//	XawTextReplace(edit, where, where, &t);
//	if(where < highTextStart[which]) { // [HGM] multiPVdisplay: move highlighting
//	    int len = strlen(text);
//	    highTextStart[which] += len; highTextEnd[which] += len;
//	    XawTextSetSelection( outputField[which][nMemo], highTextStart[which], highTextEnd[which] );
//	}
}

void SetIcon( int which, int field, int nIcon )
{
    Arg arg;

    if( nIcon != 0 ) {
//	XtSetArg(arg, XtNleftBitmap, (XtArgVal) icons[nIcon]);
//	XtSetValues(outputField[which][field], &arg, 1);
    }
}

void DoClearMemo(int which)
{
    //Widget edit = XtNameToWidget(engineOutputShell, which ? "*form2.text" : "*form.text");
    //Arg arg;

    gchar widgetname[50];   
    
    strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
    GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);     
 
    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editGTK));        
    gtk_text_buffer_set_text(tb, "", -1);
//    XtCallActionProc(edit, "select-all", NULL, NULL, 0);
//    XtCallActionProc(edit, "kill-selection", NULL, NULL, 0);
//    XtSetArg(arg, XtNstring, ""); // clear without disturbing selection!
//    XtSetValues(edit, &arg, 1);
}

// cloned from CopyPositionProc. Abuse selected_fen_position to hold selection

Boolean SendPositionSelection(Widget w, Atom *selection, Atom *target,
		 Atom *type_return, XtPointer *value_return,
		 unsigned long *length_return, int *format_return); // from xboard.c

char memoTranslations[] =
":Ctrl<Key>c: CopyMemoProc() \n \
<Btn3Motion>: HandlePV() \n \
Shift<Btn3Down>: select-start() SelectPV(1) \n \
Any<Btn3Down>: select-start() SelectPV(0) \n \
<Btn3Up>: extend-end() StopPV() \n";

void
SelectPV (Widget w, XEvent * event, String * params, Cardinal * nParams)
{	// [HGM] pv: translate click to PV line, and load it for display
	String val;
	int start, end;
	XawTextPosition index, dummy;
	int x, y;
	Arg arg;

	x = event->xmotion.x; y = event->xmotion.y;
	currentPV = (w == outputField[1][nMemo]);
	XawTextGetSelectionPos(w, &index, &dummy);
	XtSetArg(arg, XtNstring, &val);
	XtGetValues(w, &arg, 1);
	shiftKey = strcmp(params[0], "0");
	if(LoadMultiPV(x, y, val, index, &start, &end)) {
	    XawTextSetSelection( outputField[currentPV][nMemo], start, end );
	    highTextStart[currentPV] = start; highTextEnd[currentPV] = end;
	}
}

void
StopPV (Widget w, XEvent * event, String * params, Cardinal * nParams)
{	// [HGM] pv: on right-button release, stop displaying PV
        XawTextUnsetSelection( w );
        highTextStart[currentPV] = highTextEnd[currentPV] = 0;
        UnLoadPV();
}

static void
MemoCB(Widget w, XtPointer client_data, Atom *selection,
	   Atom *type, XtPointer value, unsigned long *len, int *format)
{
  if (value==NULL || *len==0) return; /* nothing had been selected to copy */
  selected_fen_position = value;
  selected_fen_position[*len]='\0'; /* normally this string is terminated, but be safe */
    XtOwnSelection(menuBarWidget, XA_CLIPBOARD(xDisplay),
		   CurrentTime,
		   SendPositionSelection,
		   NULL/* lose_ownership_proc */ ,
		   NULL/* transfer_done_proc */);
}

void CopyMemoProc(w, event, prms, nprms)
  Widget w;
  XEvent *event;
  String *prms;
  Cardinal *nprms;
{
    if(appData.pasteSelection) return;
    if (selected_fen_position) free(selected_fen_position);
    XtGetSelectionValue(menuBarWidget,
      XA_PRIMARY, XA_STRING,
      /* (XtSelectionCallbackProc) */ MemoCB,
      NULL, /* client_data passed to PastePositionCB */

      /* better to use the time field from the event that triggered the
       * call to this function, but that isn't trivial to get
       */
      CurrentTime
    );
}

// The following routines are mutated clones of the commentPopUp routines
void PositionControlSetGTK(which)
    int which;
{
    //Arg args[16];
    //Widget edit, NameWidget, ColorWidget, ModeWidget, MoveWidget, NodesWidget;
    int j, mutable=1;

    gchar widgetname[50];
    strcpy(widgetname, which == 0 ? "Engine1Colorlabel" : "Engine2Colorlabel");    
    GtkWidget *ColorWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ColorWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nColorIcon] = ColorWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Namelabel" : "Engine2Namelabel"); 
    GtkWidget *NameWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!NameWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nLabel] = NameWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Modelabel" : "Engine2Modelabel"); 
    GtkWidget *ModeWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ModeWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nStateIcon] = ModeWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Movelabel" : "Engine2Movelabel");
    GtkWidget *MoveWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ModeWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nStateData] = MoveWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1Nodeslabel" : "Engine2Nodeslabel");
    GtkWidget *NodesWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!NodesWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nLabelNPS] = NodesWidgetGTK;

    strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
    GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nMemo] = editGTK;

}

void PositionControlSet(which, shell, form, bw_width)
     int which;
     Widget shell, form;
     Dimension bw_width;
{
    Arg args[16];
    Widget edit, NameWidget, ColorWidget, ModeWidget, MoveWidget, NodesWidget;
    int j, mutable=1;

/*
    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) ""); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNleft,      XtChainLeft); j++;
    XtSetArg(args[j], XtNright,     XtChainLeft); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) 17); j++;
    outputField[which][nColorIcon] = ColorWidget =
      XtCreateManagedWidget("Color", labelWidgetClass,
		     form, args, j);
*/

    gchar widgetname[50];
    strcpy(widgetname, which == 0 ? "Engine1Colorlabel" : "Engine2Colorlabel");    
    GtkWidget *ColorWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ColorWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nColorIcon] = ColorWidgetGTK;

/*
    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNjustify,   (XtArgVal) XtJustifyLeft); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) ColorWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNleft,      XtChainLeft); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) bw_width/2 - 57); j++;
    outputField[which][nLabel] = NameWidget =
      XtCreateManagedWidget("Engine", labelWidgetClass,
		     form, args, j);
*/

    strcpy(widgetname, which == 0 ? "Engine1Namelabel" : "Engine2Namelabel"); 
    GtkWidget *NameWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!NameWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nLabel] = NameWidgetGTK;

/*
    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) ""); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) NameWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) 20); j++;
    outputField[which][nStateIcon] = ModeWidget =
      XtCreateManagedWidget("Mode", labelWidgetClass,
		     form, args, j);
*/

    strcpy(widgetname, which == 0 ? "Engine1Modelabel" : "Engine2Modelabel"); 
    GtkWidget *ModeWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ModeWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nStateIcon] = ModeWidgetGTK;

/*
    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNjustify,   (XtArgVal) XtJustifyLeft); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) ""); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) ModeWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNright,     XtChainRight); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) bw_width/2 - 102); j++;
    outputField[which][nStateData] = MoveWidget =
      XtCreateManagedWidget("Move", labelWidgetClass,
		     form, args, j);
*/
    strcpy(widgetname, which == 0 ? "Engine1Movelabel" : "Engine2Movelabel");
    GtkWidget *MoveWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!ModeWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nStateData] = MoveWidgetGTK;
/*
    j = 0;
    XtSetArg(args[j], XtNborderWidth, (XtArgVal) 0); j++;
    XtSetArg(args[j], XtNjustify,   (XtArgVal) XtJustifyRight); j++;
    XtSetArg(args[j], XtNlabel,     (XtArgVal) _("NPS")); j++;
    XtSetArg(args[j], XtNfromHoriz, (XtArgVal) MoveWidget); j++;
    XtSetArg(args[j], XtNtop,       XtChainTop); j++;
    XtSetArg(args[j], XtNbottom,    XtChainTop); j++;
    XtSetArg(args[j], XtNleft,      XtChainRight); j++;
    XtSetArg(args[j], XtNright,     XtChainRight); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) 16); j++;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) 100); j++;
    outputField[which][nLabelNPS] = NodesWidget =
      XtCreateManagedWidget("Nodes", labelWidgetClass,
		     form, args, j);
*/
    strcpy(widgetname, which == 0 ? "Engine1Nodeslabel" : "Engine2Nodeslabel");
    GtkWidget *NodesWidgetGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!NodesWidgetGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nLabelNPS] = NodesWidgetGTK;

    // create "text" within "form"

 // j = 0;
 // if (mutable) {
 //     XtSetArg(args[j], XtNeditType, XawtextEdit);  j++;
 //     XtSetArg(args[j], XtNuseStringInPlace, False);  j++;
 // }
 // XtSetArg(args[j], XtNstring, "");  j++;
 // XtSetArg(args[j], XtNdisplayCaret, False);  j++;
 // XtSetArg(args[j], XtNtop, XtChainTop);  j++;
 // XtSetArg(args[j], XtNbottom, XtChainBottom);  j++;
 // XtSetArg(args[j], XtNleft, XtChainLeft);  j++;
 // XtSetArg(args[j], XtNright, XtChainRight);  j++;
 // XtSetArg(args[j], XtNresizable, True);  j++;
 // XtSetArg(args[j], XtNwidth, bw_width);  j++; /*force wider than buttons*/
    /* !!Work around an apparent bug in XFree86 4.0.1 (X11R6.4.3) */
 // XtSetArg(args[j], XtNscrollVertical, XawtextScrollAlways);  j++;
 // XtSetArg(args[j], XtNscrollHorizontal, XawtextScrollWhenNeeded);  j++;
//    XtSetArg(args[j], XtNautoFill, True);  j++;
//    XtSetArg(args[j], XtNwrap, XawtextWrapWord); j++;
 // outputField[which][nMemo] = edit =
 //   XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, j);

 // XtOverrideTranslations(edit, XtParseTranslationTable(memoTranslations));
    //    XtAddEventHandler(edit, ButtonPressMask, False, SetFocus, (XtPointer) shell);

/*
    j = 0;
    XtSetArg(args[j], XtNfromVert, ColorWidget); j++;
//    XtSetArg(args[j], XtNresizable, (XtArgVal) True); j++;
    XtSetValues(edit, args, j);
*/

    strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
    GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
    if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
    outputFieldGTK[which][nMemo] = editGTK;

}

GtkWidget *EngineOutputCreateGTK(name, text)
     char *name, *text;
{
    GtkWidget *shell=NULL;

    builder = gtk_builder_new();
    gchar *filename = get_glade_filename ("engineoutput.glade");
    if(!gtk_builder_add_from_file (builder, filename, &gtkerror)) {      
      if(gtkerror)
        printf ("Error: %d %s\n",gtkerror->code,gtkerror->message);
    }
    shell = GTK_WIDGET(gtk_builder_get_object(builder, "EngineOutput"));
    if(!shell) printf("Error: Failed to get engineoutput object with gtk_builder\n");
    
    PositionControlSetGTK(0);
    PositionControlSetGTK(1);
    
    return shell;
}


Widget EngineOutputCreate(name, text)
     char *name, *text;
{
    Arg args[16];
    Widget shell, layout, form, form2;
    Dimension bw_width, bw_height;
    int j;

    // get board width
    j = 0;
    XtSetArg(args[j], XtNwidth,  &bw_width);  j++;
    XtSetArg(args[j], XtNheight, &bw_height);  j++;
    XtGetValues(boardWidget, args, j);

    // define form within layout within shell.
    j = 0;
    XtSetArg(args[j], XtNresizable, True);  j++;
    shell =
#if TOPLEVEL
     XtCreatePopupShell(name, topLevelShellWidgetClass,
#else
      XtCreatePopupShell(name, transientShellWidgetClass,
#endif
			 shellWidget, args, j);
    layout =
      XtCreateManagedWidget(layoutName, formWidgetClass, shell,
			    layoutArgs, XtNumber(layoutArgs));
    // divide window vertically into two equal parts, by creating two forms
    form =
      XtCreateManagedWidget("form", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    form2 =
      XtCreateManagedWidget("form2", formWidgetClass, layout,
			    formArgs, XtNumber(formArgs));
    j = 0;
    XtSetArg(args[j], XtNfromVert,  (XtArgVal) form); j++;
    XtSetValues(form2, args, j);
    // make sure width is known in advance, for better placement of child widgets
    j = 0;
    XtSetArg(args[j], XtNwidth,     (XtArgVal) bw_width-16); j++;
    XtSetArg(args[j], XtNheight,    (XtArgVal) bw_height/2); j++;
    XtSetValues(shell, args, j);

    // fill up both forms with control elements
    PositionControlSet(0, shell, form,  bw_width);
    PositionControlSet(1, shell, form2, bw_width);

    XtRealizeWidget(shell);

    if(wpEngineOutput.width > 0) {
      engineOutputW = wpEngineOutput.width;
      engineOutputH = wpEngineOutput.height;
      engineOutputX = wpEngineOutput.x;
      engineOutputY = wpEngineOutput.y;
    }

    if (engineOutputX == -1) {
	int xx, yy;
	Window junk;

	engineOutputH = bw_height/2;
	engineOutputW = bw_width-16;

	XSync(xDisplay, False);
#ifdef NOTDEF
	/* This code seems to tickle an X bug if it is executed too soon
	   after xboard starts up.  The coordinates get transformed as if
	   the main window was positioned at (0, 0).
	   */
	XtTranslateCoords(shellWidget,
			  (bw_width - engineOutputW) / 2, 0 - engineOutputH / 2,
			  &engineOutputX, &engineOutputY);
#else  /*!NOTDEF*/
        XTranslateCoordinates(xDisplay, XtWindow(shellWidget),
			      RootWindowOfScreen(XtScreen(shellWidget)),
			      (bw_width - engineOutputW) / 2, 0 - engineOutputH / 2,
			      &xx, &yy, &junk);
	engineOutputX = xx;
	engineOutputY = yy;
#endif /*!NOTDEF*/
	if (engineOutputY < 0) engineOutputY = 0; /*avoid positioning top offscreen*/
    }
    j = 0;
    XtSetArg(args[j], XtNheight, engineOutputH);  j++;
    XtSetArg(args[j], XtNwidth, engineOutputW);  j++;
    XtSetArg(args[j], XtNx, engineOutputX);  j++;
    XtSetArg(args[j], XtNy, engineOutputY);  j++;
    XtSetValues(shell, args, j);

    return shell;
}

void ResizeWindowControls(mode)
	int mode;
{
    Widget form1, form2;
    Arg args[16];
    int j;
    Dimension ew_height, tmp;
    Widget shell = engineOutputShell;

    form1 = XtNameToWidget(shell, "*form");
    form2 = XtNameToWidget(shell, "*form2");

    j = 0;
    XtSetArg(args[j], XtNheight, (XtArgVal) &ew_height); j++;
    XtGetValues(form1, args, j);
    j = 0;
    XtSetArg(args[j], XtNheight, (XtArgVal) &tmp); j++;
    XtGetValues(form2, args, j);
    ew_height += tmp; // total height

    if(mode==0) {
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) 5); j++;
	XtSetValues(form2, args, j);
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) (ew_height-5)); j++;
	XtSetValues(form1, args, j);
    } else {
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) (ew_height/2)); j++;
	XtSetValues(form1, args, j);
	j = 0;
	XtSetArg(args[j], XtNheight, (XtArgVal) (ew_height/2)); j++;
	XtSetValues(form2, args, j);
    }
}

void
EngineOutputPopUp()
{
    Arg args[16];
    int j;
    Widget edit;
    static int  needInit = TRUE;
    static char *title = N_("Engine output"), *text = N_("This feature is experimental");    

    if (engineOutputShellGTK == NULL) {
        engineOutputShellGTK = EngineOutputCreateGTK(_(title), _(text));
        g_signal_connect(engineOutputShellGTK, "destroy",
                          G_CALLBACK(EngineOutputPopDown),
                          engineOutputShellGTK);
        gtk_widget_show_all(engineOutputShellGTK);
if (engineOutputShell == NULL) {
	engineOutputShell =
	  EngineOutputCreate(_(title), _(text));
}
    }
 /* else {
        gchar widgetname[50];
        int which=0;
        strcpy(widgetname, which == 0 ? "Engine1EditText" : "Engine2EditText");
        GtkWidget *editGTK = GTK_WIDGET(gtk_builder_get_object(builder, widgetname));
        if(!editGTK) printf("Error: Failed to get %s object with gtk_builder\n", widgetname);
        //outputFieldGTK[which][nMemo] = editGTK;
        
        printf("text=%s\n", text);
 
        GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editGTK));        
        gtk_text_buffer_set_text(tb, text, -1);
    }
*/
    
    
/*
    if (engineOutputShell == NULL) {
	engineOutputShell =
	  EngineOutputCreate(_(title), _(text));
	XtRealizeWidget(engineOutputShell);
	if( needInit ) {
	    InitializeEngineOutput();
	    needInit = FALSE;
	}
        SetEngineColorIcon( 0 );
        SetEngineColorIcon( 1 );
        SetEngineState( 0, STATE_IDLE, "" );
        SetEngineState( 1, STATE_IDLE, "" );
    } else {
	edit = XtNameToWidget(engineOutputShell, "*form.text");
	j = 0;
	XtSetArg(args[j], XtNstring, text); j++;
	XtSetValues(edit, args, j);
	j = 0;
	XtSetArg(args[j], XtNiconName, (XtArgVal) _(title));   j++;
	XtSetArg(args[j], XtNtitle, (XtArgVal) _(title));      j++;
	XtSetValues(engineOutputShell, args, j);
    }
*/

    //XtPopup(engineOutputShell, XtGrabNone);
    //XSync(xDisplay, False);

//    j=0;
//    XtSetArg(args[j], XtNleftBitmap, xMarkPixmap); j++;
//    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Engine Output"),
//		args, j);
//
    SetCheckMenuItemActive(NULL, 100, True); // set GTK menu item to checked

    engineOutputDialogUp = True;
    ShowThinkingEvent(); // [HGM] thinking: might need to prompt engine for thinking output
}

void EngineOutputPopDown()
{
    Arg args[16];
    int j;

    if (!engineOutputDialogUp) return;
    DoClearMemo(1);
/*
    j = 0;
    XtSetArg(args[j], XtNx, &engineOutputX); j++;
    XtSetArg(args[j], XtNy, &engineOutputY); j++;
    XtSetArg(args[j], XtNwidth, &engineOutputW); j++;
    XtSetArg(args[j], XtNheight, &engineOutputH); j++;
    XtGetValues(engineOutputShell, args, j);
    wpEngineOutput.x = engineOutputX - 4;
    wpEngineOutput.y = engineOutputY - 23;
    wpEngineOutput.width = engineOutputW;
    wpEngineOutput.height = engineOutputH;
    XtPopdown(engineOutputShell);
*/

    gtk_widget_destroy(engineOutputShellGTK);
    engineOutputShellGTK = NULL;

/*
    XSync(xDisplay, False);
    j=0;
    XtSetArg(args[j], XtNleftBitmap, None); j++;
    XtSetValues(XtNameToWidget(menuBarWidget, "menuView.Show Engine Output"),
		args, j);
*/

    SetCheckMenuItemActive(NULL, 100, False); // set GTK menu item to unchecked
    engineOutputDialogUp = False;
    ShowThinkingEvent(); // [HGM] thinking: might need to shut off thinking output
}

int EngineOutputIsUp()
{
    return engineOutputDialogUp;
}

int EngineOutputDialogExists()
{
    return engineOutputShell != NULL;
}

void EngineOutputProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
  if (engineOutputDialogUp) {
    EngineOutputPopDown();
  } else {
    EngineOutputPopUp();
  }
}
