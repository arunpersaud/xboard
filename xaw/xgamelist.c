/*
 * xgamelist.c -- Game list window, part of X front end for XBoard
 *
 * Copyright 1995, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
#include <X11/cursorfont.h>
#if USE_XAW3D
#include <X11/Xaw3d/Dialog.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/List.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/SmeLine.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/MenuButton.h>
#include <X11/Xaw3d/Text.h>
#include <X11/Xaw3d/AsciiText.h>
#include <X11/Xaw3d/Viewport.h>
#else
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Viewport.h>
#endif

#include "common.h"
#include "backend.h"
#include "xboard.h"
#include "xgamelist.h"
#include "dialogs.h"


char gameListTranslations[] =
  "<Btn4Down>: WheelProc(-3) \n \
   <Btn5Down>: WheelProc(3) \n \
   <Btn1Down>: LoadSelectedProc(100) Set() \n \
   <Btn1Up>(2): LoadSelectedProc(0) \n \
   <Key>Home: LoadSelectedProc(-2) \n \
   <Key>End: LoadSelectedProc(2) \n \
   Ctrl<Key>Up: LoadSelectedProc(-3) \n \
   Ctrl<Key>Down: LoadSelectedProc(3) \n \
   <Key>Up: LoadSelectedProc(-1) \n \
   <Key>Down: LoadSelectedProc(1) \n \
   <Key>Left: LoadSelectedProc(-1) \n \
   <Key>Right: LoadSelectedProc(1) \n \
   <Key>Prior: LoadSelectedProc(-4) \n \
   <Key>Next: LoadSelectedProc(4) \n \
   <Key>Return: LoadSelectedProc(0) \n";
char filterTranslations[] =
  "<Key>Return: SetFilterProc() \n";


void
LoadSelectedProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    if(GameListClicks(atoi(prms[0]))) return; // if no game loaded, no focus transfer
    XSync(xDisplay, False);
    XSetInputFocus(xDisplay, XtWindow(boardWidget), RevertToPointerRoot, CurrentTime);
}

void
SetFilterProc (Widget w, XEvent *event, String *prms, Cardinal *nprms)
{
    SetFilter();
}
