WinBoard is a graphical chessboard for 32-bit Microsoft
Windows systems.  It can serve as a user interface for
GNU Chess or for internet chess servers, or it can be
used to play out games manually or from game files.
For full details, see the Help file (WinBoard.hlp).
For answers for frequently asked questions, see the
FAQ file (FAQ.html).

WinBoard is free software.  You can redistribute it
and/or modify it under the GPL, as described in the
files COPYRIGHT and COPYING.  If your distribution
does not include the source code, you can get it from
the author's Web page:

 http://www.tim-mann.org/chess.html

GNU CHESS

A version of GNU Chess is included in the WinBoard 
distribution.  GNU Chess is free software released
under the GPL.  Source code for GNU Chess is available
from the Free Software Foundation:

 http://www.gnu.org/

GNU Chess 5.02 was compiled with the Cygwin toolkit
and linked against the Cygwin1.dll library.  Cygwin
is free software released under the GPL; source code
is available through:

 http://www.cygnus.com/

BUGS AND IMPROVEMENTS

Report bugs in WinBoard or XBoard to the author,
Tim Mann <tim@tim-mann.org>.  Report bugs in 
GNU Chess to <bug-gnu-chess@gnu.org>.  Give full
details, including the program's version number
and the exact text of any error messages.  If you
improve WinBoard or XBoard, please send a message
about your changes to <tim@tim-mann.org>. 
Remember that these programs are released under 
the GPL, so source code is freely available, but
if you distribute modified versions, you must
distribute the source code modifications under 
the GPL as well.

HISTORY

WinBoard is a port of the Unix program XBoard to
Win32, the 32-bit Microsoft Windows API.
The graphical front end (WinBoard.c, etc.) is all
new, but the back end that understands chess rules,
chess notation, GNU Chess, the ICS, etc., is shared
with XBoard.  See WinBoard.hlp for a list of
contributors to WinBoard and XBoard.

MANUAL INSTALLATION

For a minimal installation of WinBoard and its
companion version of GNU Chess, create a new
directory and copy the following files to it:

WinBoard.exe  (WinBoard program)
WinBoard.hlp  (WinBoard documentation)
gnuchess.exe  (GNU Chess 4 engine used by WinBoard)
gnuchesr.exe  (Command-line GNU Chess 4 engine)
gnuchess.lan  (GNU Chess 4 messages file)
gnuchess.dat  (GNU Chess 4 opening book)
gnuchess.txt  (GNU Chess 4 documentation)
gnuches5.exe  (GNU Chess 5 engine)
book.dat      (GNU Chess 5 opening book)
gnuches5.txt  (GNU Chess 5 documentation)

Use the Program Manager or the Explorer to create an
icon for WinBoard.exe, and set its working directory
to the directory you created.  Besides looking for
its companion files in this directory, WinBoard also
loads and saves game, position, and setting files
there.
