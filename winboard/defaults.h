/*
 * defaults.h -- Default settings for Windows NT front end to XBoard
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

/* Static */
#define POSITION_FILT "Position files (*.fen,*.epd,*.pos)\0*.fen;*.epd;*.pos\0All files (*.*)\0*.*\0"
#define GAME_FILT     "Game files (*.pgn,*.gam)\0*.pgn;*.gam\0All files (*.*)\0*.*\0"
#define DIAGRAM_FILT  "bitmap files (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0"
#define SOUND_FILT    "Wave files (*.wav)\0*.wav\0All files (*.*)\0*.*\0"
#define OUTER_MARGIN (tinyLayout ? 0 : 4)
#define INNER_MARGIN (tinyLayout ? 0 : 2)
#define MESSAGE_LINE_LEFTMARGIN 2
#define MESSAGE_TEXT_MAX 256
/*#define COLOR_ECHOOFF RGB(192,192,192)*/
#define COLOR_ECHOOFF consoleBackgroundColor
#define WRAP_INDENT 200

/* Settable */
#define FIRST_CHESS_PROGRAM	""
#define FIRST_DIRECTORY		"."
#define SECOND_CHESS_PROGRAM	""
#define SECOND_DIRECTORY	"."

#define CLOCK_FONT_TINY        "Arial:9.0 b"
#define CLOCK_FONT_TEENY       "Arial:9.0 b"
#define CLOCK_FONT_DINKY       "Arial:10.0 b"
#define CLOCK_FONT_PETITE      "Arial:10.0 b"
#define CLOCK_FONT_SLIM        "Arial:12.0 b"
#define CLOCK_FONT_SMALL       "Arial:14.0 b"
#define CLOCK_FONT_MEDIOCRE    "Arial:14.0 b"
#define CLOCK_FONT_MIDDLING    "Arial:14.0 b"
#define CLOCK_FONT_AVERAGE     "Arial:15.0 b"
#define CLOCK_FONT_MODERATE    "Arial:16.0 b"
#define CLOCK_FONT_MEDIUM      "Arial:16.0 b"
#define CLOCK_FONT_BULKY       "Arial:17.0 b"
#define CLOCK_FONT_LARGE       "Arial:19.0 b"
#define CLOCK_FONT_BIG         "Arial:20.0 b"
#define CLOCK_FONT_HUGE        "Arial:21.0 b"
#define CLOCK_FONT_GIANT       "Arial:22.0 b"
#define CLOCK_FONT_COLOSSAL    "Arial:23.0 b"
#define CLOCK_FONT_TITANIC     "Arial:24.0 b"

#define MESSAGE_FONT_TINY      "Small Fonts:6.0"
#define MESSAGE_FONT_TEENY     "Small Fonts:6.0"
#define MESSAGE_FONT_DINKY     "Small Fonts:7.0"
#define MESSAGE_FONT_PETITE    "Small Fonts:7.0"
#define MESSAGE_FONT_SLIM      "Arial:8.0 b"
#define MESSAGE_FONT_SMALL     "Arial:9.0 b"
#define MESSAGE_FONT_MEDIOCRE  "Arial:9.0 b"
#define MESSAGE_FONT_MIDDLING  "Arial:9.0 b"
#define MESSAGE_FONT_AVERAGE   "Arial:10.0 b"
#define MESSAGE_FONT_MODERATE  "Arial:10.0 b"
#define MESSAGE_FONT_MEDIUM    "Arial:10.0 b"
#define MESSAGE_FONT_BULKY     "Arial:10.0 b"
#define MESSAGE_FONT_LARGE     "Arial:10.0 b"
#define MESSAGE_FONT_BIG       "Arial:11.0 b"
#define MESSAGE_FONT_HUGE      "Arial:11.0 b"
#define MESSAGE_FONT_GIANT     "Arial:11.0 b"
#define MESSAGE_FONT_COLOSSAL  "Arial:12.0 b"
#define MESSAGE_FONT_TITANIC   "Arial:12.0 b"

#define COORD_FONT_TINY        "Small Fonts:4.0"
#define COORD_FONT_TEENY       "Small Fonts:4.0"
#define COORD_FONT_DINKY       "Small Fonts:5.0"
#define COORD_FONT_PETITE      "Small Fonts:5.0"
#define COORD_FONT_SLIM        "Small Fonts:6.0"
#define COORD_FONT_SMALL       "Small Fonts:7.0"
#define COORD_FONT_MEDIOCRE    "Small Fonts:7.0"
#define COORD_FONT_MIDDLING    "Small Fonts:7.0"
#define COORD_FONT_AVERAGE     "Arial:7.0 b"
#define COORD_FONT_MODERATE    "Arial:7.0 b"
#define COORD_FONT_MEDIUM      "Arial:7.0 b"
#define COORD_FONT_BULKY       "Arial:7.0 b"
#define COORD_FONT_LARGE       "Arial:7.0 b"
#define COORD_FONT_BIG         "Arial:8.0 b"
#define COORD_FONT_HUGE        "Arial:8.0 b"
#define COORD_FONT_GIANT       "Arial:8.0 b"
#define COORD_FONT_COLOSSAL    "Arial:9.0 b"
#define COORD_FONT_TITANIC     "Arial:9.0 b"

#define CONSOLE_FONT_TINY      "Courier New:8.0"
#define CONSOLE_FONT_TEENY     "Courier New:8.0"
#define CONSOLE_FONT_DINKY     "Courier New:8.0"
#define CONSOLE_FONT_PETITE    "Courier New:8.0"
#define CONSOLE_FONT_SLIM      "Courier New:8.0"
#define CONSOLE_FONT_SMALL     "Courier New:8.0"
#define CONSOLE_FONT_MEDIOCRE  "Courier New:8.0"
#define CONSOLE_FONT_MIDDLING  "Courier New:8.0"
#define CONSOLE_FONT_AVERAGE   "Courier New:8.0"
#define CONSOLE_FONT_MODERATE  "Courier New:8.0"
#define CONSOLE_FONT_MEDIUM    "Courier New:8.0"
#define CONSOLE_FONT_BULKY     "Courier New:8.0"
#define CONSOLE_FONT_LARGE     "Courier New:8.0"
#define CONSOLE_FONT_BIG       "Courier New:8.0"
#define CONSOLE_FONT_HUGE      "Courier New:8.0"
#define CONSOLE_FONT_GIANT     "Courier New:8.0"
#define CONSOLE_FONT_COLOSSAL  "Courier New:8.0"
#define CONSOLE_FONT_TITANIC   "Courier New:8.0"

#define COMMENT_FONT_TINY      "Arial:9.0"
#define COMMENT_FONT_TEENY     "Arial:9.0"
#define COMMENT_FONT_DINKY     "Arial:9.0"
#define COMMENT_FONT_PETITE    "Arial:9.0"
#define COMMENT_FONT_SLIM      "Arial:9.0"
#define COMMENT_FONT_SMALL     "Arial:9.0"
#define COMMENT_FONT_MEDIOCRE  "Arial:9.0"
#define COMMENT_FONT_MIDDLING  "Arial:9.0"
#define COMMENT_FONT_AVERAGE   "Arial:9.0"
#define COMMENT_FONT_MODERATE  "Arial:9.0"
#define COMMENT_FONT_MEDIUM    "Arial:9.0"
#define COMMENT_FONT_BULKY     "Arial:9.0"
#define COMMENT_FONT_LARGE     "Arial:9.0"
#define COMMENT_FONT_BIG       "Arial:9.0"
#define COMMENT_FONT_HUGE      "Arial:9.0"
#define COMMENT_FONT_GIANT     "Arial:9.0"
#define COMMENT_FONT_COLOSSAL  "Arial:9.0"
#define COMMENT_FONT_TITANIC   "Arial:9.0"

#define EDITTAGS_FONT_TINY     "Courier New:8.0"
#define EDITTAGS_FONT_TEENY    "Courier New:8.0"
#define EDITTAGS_FONT_DINKY    "Courier New:8.0"
#define EDITTAGS_FONT_PETITE   "Courier New:8.0"
#define EDITTAGS_FONT_SLIM     "Courier New:8.0"
#define EDITTAGS_FONT_SMALL    "Courier New:8.0"
#define EDITTAGS_FONT_MEDIUM   "Courier New:8.0"
#define EDITTAGS_FONT_MEDIOCRE "Courier New:8.0"
#define EDITTAGS_FONT_MIDDLING "Courier New:8.0"
#define EDITTAGS_FONT_AVERAGE  "Courier New:8.0"
#define EDITTAGS_FONT_MODERATE "Courier New:8.0"
#define EDITTAGS_FONT_BULKY    "Courier New:8.0"
#define EDITTAGS_FONT_LARGE    "Courier New:8.0"
#define EDITTAGS_FONT_BIG      "Courier New:8.0"
#define EDITTAGS_FONT_HUGE     "Courier New:8.0"
#define EDITTAGS_FONT_GIANT    "Courier New:8.0"
#define EDITTAGS_FONT_COLOSSAL "Courier New:8.0"
#define EDITTAGS_FONT_TITANIC  "Courier New:8.0"

#define MOVEHISTORY_FONT_ALL    "MS Sans Serif:8.0"
#define GAMELIST_FONT_ALL       "MS Sans Serif:8.0"

#define COLOR_SHOUT            "#209000"
#define COLOR_SSHOUT         "b #289808"
#define COLOR_CHANNEL1         "#2020E0"
#define COLOR_CHANNEL        "b #4040FF"
#define COLOR_KIBITZ         "b #FF00FF"
#define COLOR_TELL           "b #FF0000"
#define COLOR_CHALLENGE     "bi #FF0000"
#define COLOR_REQUEST       "bi #FF0000"
#define COLOR_SEEK             "#980808"
#define COLOR_NORMAL           "#000000"
#define COLOR_NONE             "#000000"
#define COLOR_BKGD             "#FFFFFF"

#define SOUND_BELL "$"

#define BUILT_IN_SOUND_NAMES {\
  "Beepbeep", "Ching", "Click", "Cymbal", "Ding", "Drip", \
  "Gong", "Laser", "Move", "Penalty", "Phone", "Pop", "Pop2", \
  "Roar", "Slap", "Squeak", "Swish", "Thud", "Whipcrack", \
  "Alarm", "Challenge", "Channel", "Channel1", "Draw", "Kibitz", \
  "Lose", "Request", "Seek", "Shout", "SShout", "Tell", "Unfinished", \
  "Win", NULL \
}

#define SETTINGS_FILE         "winboard.ini"
#define DEBUG_FILE            "winboard.debug"

#define ICS_LOGON             "ics.ini"

#define ICS_NAMES "\
chessclub.com /icsport=5000 /icshelper=timestamp\n\
freechess.org /icsport=5000 /icshelper=timeseal\n\
global.chessparlor.com /icsport=6000 /icshelper=timeseal\n\
chessanytime.com /icsport=5000\n\
chess.net /icsport=5000\n\
chess.deepnet.com /icsport=5000 /icshelper=timeseal\n\
zics.org /icsport=5000\n\
jogo.cex.org.br /icsport=5000\n\
ajedrez.cec.uchile.cl /icsport=5000\n\
fly.cc.fer.hr /icsport=7890\n\
freechess.nl /icsport=5000 /icshelper=timeseal\n\
jeu.echecs.com /icsport=5000\n\
chess.unix-ag.uni-kl.de /icsport=5000 /icshelper=timeseal\n\
chess.mds.mdh.se /icsport=5000\n\
"

#define ICS_TEXT_MENU_DEFAULT "\
-\n\
&Who,who,0,1\n\
Playe&rs,players,0,1\n\
&Games,games,0,1\n\
&Sought,sought,0,1\n\
| ,none,0,0\n\
Open Chat &Box (name),chat,1,0\n\
&Tell (name),tell,1,0\n\
M&essage (name),message,1,0\n\
-\n\
&Finger (name),finger,1,1\n\
&Vars (name),vars,1,1\n\
&Observe (name),observe,1,1\n\
&Match (name),match,1,1\n\
Pl&ay (name),play,1,1\n\
"

#define FCP_NAMES "\
fmax /fd=Fairy-Max\n\
GNUChess\n\
\"GNUChes5 xboard\"\n\
"

#define SCP_NAMES "\
fmax /sd=Fairy-Max\n\
GNUChess\n\
\"GNUChes5 xboard\"\n\
"
