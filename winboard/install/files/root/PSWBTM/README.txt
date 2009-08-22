=================
PSWBTM 2.0 README
=================

Website
=======
http://www.prism.gatech.edu/~gtg365v/PSWBTM/

Release Note
============
I had promised there would be a Linux release with this PSWBTM but I haven't had the
time to do it yet and I've also found out that Xboard is very different from Winboard.
Perhaps it would be a better idea to port Winboard to various operating systems by
using a cross-platform programming API like wxWidgets.

Licence
=======
Copyright (C) 2006 Pradu Kannan

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use of
this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
that you wrote the original software. If you use this software in a product, an
acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution. 


Instructions
============

PSWBTM should be mostly intuitive so the instructions will only point out
peculiar behaviours of the program.

General
-------
All windows are resizable.

Engine Manager
--------------
The "save" button only updates the engine list you are editing; you will have to
click "save as" to save the changes permenantly.

"Select Duplicates" selects all engines with the same name.
"Select Invalid" does not check java engines for validity.

You can "Import" from Toms's Simple Engine Manger produced winboard.ini files as
well as PSWBTM export files.

"Export" will only export engines that exists inside the directory where the
save file is located and it saves all paths as relative. This way you can move
directories of engines from one computer/drive to another and update PSWBTM
easily.

You can select multiple engines from the engine list.

New Tournament
--------------

PSWBTM allows engine updates during a tourney and therefore a pgn file which has
the updated names will be placed in the Result Table PGN for the purpose of
producing cross tables.

All debug files will be placed in the debug folder will have the number of its
respective game in the PGN.

NOTE: winboard.debug will show up in the PSWBTM folder not the Winboard folder.
NOTE: winboard.ini is loaded from the PSWBTM folder for the tourney.

Starting Postions
You can setup starting positions using FEN or PGN files.  The position loaded
will increment every two games in a pairing.  You must make sure there are
enough positions for the number of games otherwise the game won't start.  You can have PSWBTM rewind after a certain number of starting positions have been played by checking the Rewind checkbox and typing the number of positions to parameter box next to the Rewind checkbox.  If you check two games, then PSWBTM will check play two games with one starting position.  This is helpful say when you are doing two games per pairing and you want to use the same starting position twice with the engines having the colors alternated.

Doubble clicking in the Available or Participants box will move engines to the
other side.  You can select multiple engines and use the move buttons as well.  You can import pairings from a PGN file or just a regular text file which has lines for "White Player" and "Black Player".  PSWBTM can find matches to the engine names by itself.  For example it can tell that "Buzz, 2.01" is the same engine as "Buzz" or "Buzz 2.01" or "Buzz v201" or pretty much anything else that looks close to "Buzz, 2.01".

Round Robin pairings done FIDE Berger style.

Pairings Tab
------------
You can move the subdivision between the rounds list and pairings list.  If you
drag it all the way to the side of the window it will hide whichever window you
wish (this might be useful for gauntlets).  Pressing ctrl-S will show all
windows again.

Controls Tab
------------
First button is to start/pause the tourney.  Tourney will be paused after the
current game has finished.

Kill Pause forces winboard and the engines to quit and the killed game will be
replayed when the tourney continues.

Abort stopts the tournament.

Round Pause automatically pauses the tourney every round.



Winboard Size and Location
---------------------------

If you want to change the size and location of winboard when a tournament starts
up then do this:

1) Open the engine manager
2) Launch a quick loading engine
3) Resize/Move or change other options with the Winboard
4) Close Winboard

It might be useful know that the working directory while running Winboard is the
working directory that PSWBTM uses.  Therefore if you double-click to open PSWBTM
under Windows, the working directory is the directory PSWBTM is in.  Under Linux,
it is whatever directory you run PSWBTM from.

During Tournament
-----------------

When a tournament is paused, you can close <,shutdown><,restart> and reopen
PSWBTM and resume the tournament as if nothing happened.

When a game did not start PSWBTM will produce an error message and a prompt
asking whether you want to pause the game. This allows you to fix any problems
that occur during a tournament.

Advanced Features
-----------------
If you want certain commands run after every game, put them in a file call
aftergame.txt.  This will run before moving any pgn or debug files.

For instance you could use this to kill misbehaving engines that hang in memory
after a game. I suggest you call shell scripts or bat files from aftergame.txt
to also to be able to use system utilities.

For example to kill certain misbehaving engines in Windows you can do this:

Download a utility that will kill engines (pv.exe)
http://www.teamcti.com/pview/prcview.htm
Lets say we put this in a folder called utils in the PSWBTM folder.

Now make a batfile (pv.bat) of misbehaving engines that you will kill and lets
put this in the utils folder too

pv.bat
=======
pv –kf misbehavingengine1.exe
pv –kf misbehavingengine2.exe
pv –kf misbehavingengine3.exe
...

aftergame.txt
=============
utils\pv.bat

This should now kill all misbehaving winboard engines.  You can also use
aftergame.txt to do a number of other things like uploading result tables to a
server.  The possibilities are endless.  Aftergame is not called after the last
game of the tourney.  Instead, aftertourney.txt is called.