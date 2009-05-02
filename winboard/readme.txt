                             WinBoard 4.3.16 RELEASE NOTES

Compared to version 4.3.15 described below, this version features

New command-line options
/niceEngines=N       for adjusting the priority of engine processes so they don't soak up all your system resources
/firstOptions="..."   Allows the setting of options that engines define through the feature option="..." commands
/secondOptions="..."
/firstLogo=filename.bmp  Displays the mentioned bitmap next to the clock (with H:W =1:2 aspect ratio) (WB only)
/secondLogo=filename.bmp
/autoLogo=false          get logo files automatically from engineDirectory\logo.bmp

General enhancements:
* New WB-protocol command: 'feature option="NAME -TYPE VALUE OTHER"', which engines can use to define options
* New WB-protocol command: 'option NAME VALUE' used to set value of engine-defned options.
* implements /delayAfterQuit and /delayBeforeQuit in XBoard, and uses SIGKILL to terminate rogue engine processes



                             WinBoard 4.3.15 RELEASE NOTES

Compared to version 4.3.14k described below, this version features

New command-line options
/rewindIndex=N (for the new auto-increment mode of the loadGameIndex or loadPositionIndex in match mode)
/sameColorGames=N (for playing a match where the same player has white all the time)
/egtFormats="..." (for specifying where various end-game tables are installed on the computer)

New menu items
+ Time-odds factors can be set in the time-control dialog
+ Nr of CPUs for SMP engines can be set in the Options -> UCI dialog
+ Own-Book options can be switched from the Options -> UCI dialog
+ The ScoreIsAbs options can be set from the Options -> Engine dialog
+ New-Variant menu adds Superchess

General Enhancements:
* WinBoard engines can now also use the Polyglot opening book (implemented as general GUI book)
* New WB-protocol commands memory, cores and egtpath make interactive setting of these parameters 
  possible on WB engines
* New Polyglot is available that relays the interactive setting of these parameters to UCI engines
* Match mode suports an auto-increment mode, so that all games or positions from a file can be played
* Draw claims with Kings and an arbitrary number of like Bishops (e.g. KBBKB) are accepted

The source tree in original xboard 4.2.7 format can now be compiled under Cygwin with the aid
of the cygwin.mak file in the ~/winboard/ subdirectory of the source releasy, after you put
the hep-file from an executabl distribution there. Maefiles for other compilers are not updated
since 4.2.7, as I do not have those compilers.


                             WinBoard 4.3.14k RELEASE NOTES

Compared to version 4.3.13 described below, this version features

New command-line options: 
/autoKibitz (for relaying the PV info of the engine to the ICS) 
/userName="..." (for setting the name of the Human player, also as menu) 
/engineDebugOutput=N (controlling the writing of engine output to debug file) 
/firstNPS=N (for time management by node count or CPU time) 
/secondNPS=N (likewise for the other engine) 

New Menu items 
+ Enter Username (same as /userName command-line option) 
+ Save Diagram (for saving the Chessboard display as bitmap file) 
+ Machine Both (not implemented yet, but menu item already provided) 
+ New-Variant menu adds CRC, Janus and Berolina (the latter only with legality testing off!) 
+ Any variant can be played from a shuffled opening setup 

Bugfixes: 
* Problems with switching the variant in ICS zippy mode solved 
* In ICS observing mode game history is now fully accessible 
* Moves are not fed to engine in zippy mode, when observing a game from a variant unknown to the engine 
* a problem with loading PGN of FRC games with move disambiguation and initial castling rights was fixed.
* A bug in the clock display that made previous versions of WinBoard crash for tiny displays is fixed

General enhancements: 
* variant name displayed in title bar in ICS mode, when not 'normal' 
* when receiving a challenge in ICS zippy mode, it is checked if the engine supports the variant (/zippyVariants="..." can still be used to limit the allowed variants, and for protocol-1 engines is still the only thing to go on) 
* when loading a game from a PGN file, WB automatically switches to the variant specified in the PGN tags 
* when starting from a loaded position (using /loadPositionFile), this position will be used on subsequent 'New Game' commands as well (until we switch variant)

New is also that the source tree is now brought back in the original WinBoard 4.2.7 format, including
xboard source files. Note, however, that the xboard sources are from an older date, and I did not test
if they still compile together with the much newer backend sources. I did add code in xboard.c to recognize
the new command-line options I added since then, and in so far they are back-end options that should be enough 
to make them work. This is completely untested, though; I did not even try to compile it. Last time anyone
built a working xboard.exe from this was at a stage where WinBoard did have adjustable board size, allowing
it to play Xiangqi. But no crazyhouse holdings yet.

                             WinBoard 4.3.13a RELEASE NOTES

This version of WinBoard_F fixes several bugs in 4.3.12, and also addse a few new features.
The new features include:
- some more fairy pieces, so that each side now has 22 piece types in stead of 17,
making most fairy pieces available in board size "petite" (next to "bulky" and "middling"),
making the ArchBishop and Chancellor, as well as one wildcard piece (the Lance) available in all 
sizes from "petite" to "bulky".
- The FRC support is fully fixed, both in local and ICS mode.
- A mechanism is provided for safe draw claiming in cases where a 3-fold repetition woud occur only
  after your own move. In this case a draw will be awarded by WinBoard if the engine sends "offer draw"
  before making its move.
- Genuine draw offers are not passed on immediately to the opponent but held up to when the offerer announces
  its move.
- Variants FRC, Cylinder and Falcon are added to the "New Variant..." menu.
- Support for playing time-odds games is added. (Options /firstTimeOdds, /secondTimeOdds, /timeOddsMode)
- A mechanism is provided for attaching WinBoard options to the engine command, to create options that
follow the engine (e.g. time odds) in a tournament run under a tournament manager.
Bugfixes include:
- Shatranj in ICS mode (did not work at all before)
- Some draw adjudications (QRKR was mistaken for KRKR, and KBKB with like Bishops is now recognized)
- time info in the PGN is now correct


                                        COMPILING

This version was developed using gcc under cygwin. To compile, the following set of commands was used,
given from the directory where all the source files are:
  flex -oparser.o -L parser.l
  windres --use-temp-file --include-dir . winboard.rc -O coff -o wbres.o
  gcc -O2 -mno-cygwin -mwindows -c *.c
  gcc -mno-cygwin -mwindows *.o -lwsock32 -lwinmm -o winboard.exe
  strip winboard.exe
  rm *.o
The first command is only needed to produce a new parser.c from the parser.l. The parser.c is included
in this release, so you only need flex if you want to modify the parser. 
The windres command builds the resource file wbres.o from winboard.rc and all bitmaps and sound files in
the sub-directories.
This release should be completely self-sufficient; no source files from other releases are needed,
even the unmodified files are included here,

                                       DESCRIPTION

This WinBoard (beta-)version is derived from Allessandro Scotti's Winboard_x, and supports the following new options, mainly in the area of adjudication of engine-engine games, improved Crazyhouse support, and allowing variants with non-conventional pieces and or board sizes. (All option are shown here with their default values):

/variant=normal 
This (already existing) option has been expanded with several new variants, involving non-conventional pieces and deviating board sizes. The board size is automatically adapted to the selected variant, unless explicitly overruled (see below). The new variants are (with default board size, files x ranks, in parentheses): 

variant name    Game           board     description
knightmate    Knightmate        (8x8)  Variant where the King moves as a Knight, and vice versa 
capablanca    Capablanca Chess (10x8)  Variant featuring Archbishop and Chancellor as new pieces 
gothic        Gothic Chess     (10x8)  Same as Capablanca, with a more interesting opening position 
courier       Courier Chess    (12x8)  a Medieval form that combines elements of Shatranj and modern Chess 
shogi         Shogi             (9x9)  Japanese Chess 
xiangqi       Xiangqi          (9x10)  Chinese Chess 
fairy         Fairy Chess       (8x8)  Variant were you can use all pieces of other variants together 
fischerandom  FRC               (8x8)  Shuffle variant with generalized castling rule
cylinder      Cylinder Chess    (8x8)  left and right board edge are connected
falcon        Falcon Chess     (10x8)  a patented variant featuring two Falcon pieces
 
The variant can be set from the newly added "File -> New Variant..." sub-menu. 
Extra board files are indicated by the letters i, j, k, l, ... For boards with more than 9 ranks, the counting starts at zero! More than 10 ranks is not tested and unlikely to work in the area of PGN saving / reading. Non-FIDE pieces will be referred to in FENs and PGN by letters that depend on the variant, and might collide with piece designators in other variants. E.g. in Xiangqi 'C' is a Cannon, in Capablanca Chess it is a Chancellor. Pieces that do not belong in a variant cannot be addressed in FEN and PGN either, for as long as that variant is selected, unless the letter assignment is overruled with the aid of the /pieceToCharTable option. The variant is not saved in the winboard.ini file; on start-up we always get variant "normal" unless we use the command-line option, or have added the option to the winboard.ini file manually (in which case it will disappear when this file is overwritten by WinBoard saving its options). 
WinBoard_F knows the movement of all pieces occurring in Capablanca Chess (of which FIDE Chess is a subset), Shatranj, Courier, Xiangqi and 9x9 Shogi, so that these games can be played with legality testing enabled. 

/pieceToCharTable="PNBRQFEACWMOHIJGDVLSUKpnbrqfeacwmohijgdvlsuk"  (*********** ALTERED IN 4.3.13 *******)
Each piece that WinBoard knows (in its legality test) has a letter associated with it, by which it will be referred to in FEN or PGN. The default assignment can be overruled with this option. The value has to be a string of even length, with at least 12 characters. The first half of the string designates the white pieces, the second half the black. 
The last letter for each color will be assigned to the King. (This is the piece that moves as an orthodox King; note that Nightmate and Xiangqi have a different royal piece.) All letters before it will be assigned to the other pieces in the order: 

P Pawn                 (move often depends on variant) 
N Knight               (move subtly different in Xiangqi (where it is written as H) or Shogi) 
B Bishop 
R Rook 
Q Queen                (Lance L in Shogi) 
F Ferz/General         (The Shatranj 'Queen' and Xiangqi 'Adviser', used for Silver General S in Shogi.) 
E Alfil/Elephant       (Moves subtly different in Xiangqi vs Shatranj/Courier) 
A Archbishop/Cardinal  
C Chancellor/Marshall  
W Wazir/GrandVizer     (Gold General G in Shogi, in Xiangqi it is royal and denoted by K) 
M Commoner/Man
O Cannon/Pao 
H Nightrider           (Promoted Knight in Shogi and CrazyHouse) 
I                      (Promoted Bishop in Shogi and CrazyHouse) 
J                      (Promoted Rook   in Shogi and CrazyHouse) 
G Grasshopper          (Promoted Queen in Crazyhouse, promoted Lance in Shogi) 
D Dabbaba              (Promoted Silver in Shogi) 
V Falcon               wildcard
L Lance                wildcard
S Snake                wildcard
U Unicorn              (representation of Royal Knight in Knightmate, used as promoted Pawn in Shogi) 
K King 
 
NOTE THIS ORDER HAS BEEN ALTERED IN 4.3.13 compred to 4.3.12. Sorry about that; this was unavoidable to accomodate an engine that can play a Crazyhouse version of Capablanca Chess. This made it impossible to continue using the Archbishop and Chancellor as promoted versions of Bishop and Rook.

NEW IN 4.3.13 is the concept of a wildcard piece: WinBoard will accept any move of such a piece as legal, even
when legality testing is switched on. This allows participaton of a limited number of pieces that WinBoard 
does not know, while still applying legality testing to the other pieces. The only rice to pay, is that WinBoard
cannot recognize checks (and thus mates) with such pieces. So if they participate, /testClaims should be off,
as WinBoard will no longer recognize all mates.

Pieces that are not mentioned (because the argument has less than 44 characters) will remain disabled. Mentioned pieces can be disabled by assigning them a '.' (period). They are then not recognized in FEN or PGN input. Non-FIDE pieces that are not assigned a letter will also not appear on the promotion menu. It is not advisable to disable a piece that is present in the opening position of the selected variant, though. 
  Promoted pieces that need to be distinguished from original pieces of the same type (because of demotion on capture and transfer to the holdings) will be indicated by the letter for the unpromoted piece with a '+' in front of it (Shogi), or by the letter of the promoted piece with a '~' after it (Crazyhouse, Bughouse, in general everything with holdings that is not Shogi). To achieve this, they should be assigned the characters '+' or '~', respectively.
  All the new pieces have a native bitmap representation in the board sizes 'bulky' and 'middling'. For all window sizes that do not support such fairy bitmaps, promoted NBRQ are represented as a 2-sizes-smaller normal piece symbol, so that Crazyhouse can be played at any size. People disliking the fairy representations might even prefer this. 
  There is an enhanced 'Edit Position' menu popup (right-clicking on the squares after selecting this mode in the main menu), featuring some common non-FIDE pieces, and 'promote' and 'demote' options to make those not directly in the menu. The promotion popup shows ArchBishop and Chancellor in Capablanca and Gothic, (or in fact in any game where this piece is not disabled or a promoted version of a normal piece), and leaves only the options YES / NO in Shogi. In Xiangqi there are no promotions.
  From version 4.3.13 on, the default assignment of characters to pieces is variant dependent. This makes the
need for using this option even smaller. Xiangqi now uses H for Horse and C for Cannon. Shatranj uses B for Elephant and Q for Ferz (because some existing engines and ICC expect this)

/fontPieceToCharTable="PNBRQFEACWMOHIJGDVLSUKpnbrqfeacwmohijgdvlsuk" (******* ALTERED IN 4.3.13 *******)
This option is similar to /pieceToCharTable, but sets the font character that is used to display the piece on the screen (when font-based rendering is in use), rather than in the FEN or PGN. The default setting should work with George Tsavdaris' WinboardF font, which uses the 'intuitive' mapping of font characters to symbols. With font-based rendering the fairy pieces can be used at any board size.
Note that UHIJGS are also used to represent the promoted versions of PNBRQF, in games like Crazyhouse and Shogi, where the promotion has to be undone on capture. In such games you are likely to prefer a different representation of those pieces then when they represent true fairy pieces.
NOTE THAT THE ORDER HAS CHANGED IN 4.3.13 compared to 4.3.12

/boardWidth=-1 /boardHeight=-1 
Set a number of files and ranks of the playing board to a value that will override the defaults for the variant that is selected. A value of -1 means the variant default board size will be used for the corresponding parameter (and is itself the default value of these options). These parameters can be set in the "Files -> New Variant..." sub-menu, where they are reset to the default -1 if you OK the chosen variant without typing something to overrule it. These parameters are saved in the winboard.ini file. (But unless you saved while a variant with board-size override was selected, they will always be saved as -1.) 
A variant with a non-standard board size will be communicated to the engine(s) with the board size prefixed to the variant name, e.g. "variant 12x8_capablanca". In protocol 2 the engine must first enable this feature by sending "boardsizeFxR" amongst the accepted variants, where F is the maximum number of files, and R the maximum number of ranks, as decimal numbers. 

/holdingsSize=-1 
Set the size of the holdings for dropable pieces to a value that will override the default for the variant that is selected. A value of -1 means the variant default holdings size will be used for that parameter (and is itself the default value of this options). This parameter can be set in the Files -> New Variant... sub-menu, where it is reset to the default -1 if you OK the chosen variant without typing something to overrule it. This parameters is saved in the winboard.ini file. 
To disable holdings, set their size to 0. They will then not be displayed. For non-zero holding size N, the holdings are displayed left and right of the board, and piece drops can be effected by dragging pieces from the holdings to the drop square. In bughouse, the holdings will be filled by the ICS. In all other variants, captured pieces will go into the holdings (after reversing their color). Only the first N pieces of the /pieceToCharTable argument will go into the holdings. All other pieces will be converted to Pawns. (In Shogi, however they will be demoted in the regular way before determining if they fit.) Pieces that are disabled (per default and per /pieceToCharTable option) might not be counted when determining what are the first N pieces. 
Non-standard holdingsize will be communicated to the engine by prefixing it (together with the board size, even if this is standard) to the variant name, e.g. "variant 7x7+5_shogi". In protocol 2 the engine should enable this feature by sending "holdingsH" amongst the variant names, where H is the maximum acceptable holdings size as a decimal number. 

/firstTimeOdds=1 /secondTimeOdds=1  (********** NEW IN 4.3.13 *********)
The time given to first or second engine (initially, for later sessions, as increment or in the 'st' command)
is divided by the given factor. This replaces the time odds feature of WinBoard_x, where two times separated by
a slash could be given for the /timeContron parameter or in the time-control field of the menu.

/timeOddsMode=1
This option determines how the case is handled where both engines have a time-oddsfactor differing from 1.
In mode 1 the times will be scaled such that the engine that gets the most time will get the nominal time,
in mode 2 both engines will play at reduced time.

/alphaRank=FALSE 
When this parameter is true, a-h are converted to 1-9, and vice versa, in all move output and input (to PGN files or SAN move display as well as in communication with the engine). This might be useful for Shogi, where conventionally one uses letters to designate ranks, and digits to designate files. Engines that want to use this option must make sure pieces are never represented by lower case! This option can be set from the Files -> New Variant... menu, where it defaults to FALSE unless you explicitly set it. It is not saved in the winboard.ini file. 
This kludge does not seem to work for reading PGN files. Saving works fine. For now, using it is not recommended. In the future it might be redefined as only affecting engine-engine communication, .
Note that the PGN format in Shogi also leaves out the trailing '+' as check indicator: In Shogi such a trailing '+' means promotion, while a trailing '=' means defer promotion. Prefix '+' signs are used on moves with promoted pieces, disambiguation is done western SAN style.

/allWhite=FALSE
Causes the outline of the 'white' pieces to be superimposed onto the 'black' piece symbols as well (as a black outline) when native bitmaps are used (as opposed to font-based rendering). This is useful if we choose a very light color to represent the 'black' pieces. It might be particularly useful in Shogi, where the conventional representation of the 'black' pieces is as upside-down white pieces, so that both colors would be white. This option is saved in the winboard.ini file, and can be set in the "Options -> Board..." sub-menu. 

/flipBlack=FALSE
Setting this option will cause upside-down display of the native piece bitmaps used to represent the pieces of the side that plays black, as would be needed for a traditional representation of Shogi pieces. It can be set from the "Options -> Board..." sub-menu, and it is saved in the winboard.ini file. For now, traditional Shogi bitmaps are only included for size "moderate". For other sizes you must depend on font-based rendering.

/detectMate=TRUE 
/testClaim=TRUE 
/materialDraws=TRUE 
/trivialDraws=FALSE 
/ruleMoves=51 
/repeatsToDraw=6 
These are all options that only affect engine-engine play, and can be set from the "Options -> Engine..." sub-menu. They are all related to adjudication of games by the GUI. Legality checking must be switched on for them to work. 
If /detectMate is TRUE, the GUI recognizes checkmate and stalemate (but not in games with holdings!), and ends the game accordingly before the engines can claim. This is convenient for play with engines that fail to claim, and just exit. 
 With /testClaim set, all result and illegal-move claims by engines that claim more than their own loss are scrutinized for validity, and false claims result in forfeit of the game. Useful with buggy engines. 
 The option /materialDraws=TRUE causes games with insufficient mating material to be adjudicated immediately as draws, in case the engines would not claim these draws. This applies to KK, KNK and KBK. (A bug in 4.3.12, which
overlooked KBKB with like Bishops as a legal draw, has been corrected in 4.3.13.)
 The option /trivialDraws adjudicates KNNK, KBKB, KNKN, KBKN, KRKR and KQKQ to draws after 3 moves (to allow for a quick tactical win). Note that in KQKQ this might not be sound, but that problem would disappear once bitbase probing is implemented. (A bug in 4.3.12, which led to KQKR being considered a trivail draw, is corrected in 4.3.13)
 The /ruleMoves determine after how many reversible moves the game is adjudicated as a draw. Setting this to 0 turns this option off. Draw claims by the engine are still accepted (with /testClaim=TRUE) after 50 reversible moves, even if /ruleMoves species a larger number. Note that it is perfectly legal according to FIDE rules to play on after 50 reversible moves, but in tournaments having two engines that want to play on forever is a nuisance in endings like KBNKR, where one of the engines thinks it is ahead and can avoids repeats virtually forever. 
The option /repeatsToDraw makes the GUI adjudicate a game as draw after the same position has occurred the specified number of times. If it is set to a value > 3, engines can still claim the draw after 3-fold repeat. 
All these options are saved in the winboard.ini file. 

/matchPause=10000 
Determines the number of milliseconds that is paused between two games of a match. In the old WinBoard this was always 10 sec, which was inconveniently long in fast games. If you make the pause too short, tardy engines might get into trouble, though. Saved in the Winboard.ini.

/zippyVariants="normal,fischerandom,crazyhouse,losers,suicide,3checks,twokings,bughouse,shatranj"
This option already existed, (giving the varinats that an engine can play on an ICS), but now by default setting
that icludes virtually all variants that WinBoard knows and that can be played on ICC. In this mode, however,
WinBoard now also checks if the variant the ICS wants to play is supportd by the engine (i.e. if it occurs
in the list given by the variant feature). Only for protocol-1 engines (which do not provide this info to WinBoard)
you would need to restrict the number of variants to those the engine can play, to avoid accepting challenges
the engine cannot handle.

Clocks
There is an "Options -> swap clocks" command, that swaps the position of white and black clocks (convenient in over-the-board matches, where the display is standing next to the board, and you want your own time to be displayed on your side of the screen). The clocks can be adjusted in "edit game" mode: right-clicking them adds one minute, left-clicking subtracts one minute. (Also for OTB matches, to keep them synchronized with the official match clock.) The flag-fell condition is now indicated as '(!)' behind the displayed time, to eliminate the necessity for overwriting the message in the title bar (which might contain indispensible information in match mode).

/pgnExtendedInfo  contains time (******** NEW IN 4.3.13 *********)
If you selected this option in the "Options -> General..." menu, the search time is now also saved as a comment 
in the PGN file with every move, together with the depth/score info (which WinBoard_x already did). The time
written is that reported by the engine if "Show Thinking" was on. If the engine does not give times,
the WinBoard clock is used in stead.

Other improvements / changes
Castling rights and e.p. rights are now fully maintained, and considered in legality testing. They are imported from and written to FEN, as is the 50-move counter. (When reading an incomplete FEN they are still guessed, though.) 
The time (in sec, or min:sec) is now always stored together with the PV information to the PGN, if storing the latter was requested (through ticking "extended PGN info" in "Options -> General..."). The saved time is the WinBoard clock time (as opposed to the time reported by the engine).

Options from the engine command line (******** NEW IN 4.3.13 *********)
The command to startup the engine, as typed in the startup dialog box, or given to WinBoard in the 
/firstChessProgram or /secondChessProgram (/fcp, /scp) can now contain options that are not passed to the
engine, but are interpreted by WinBoard. These would then overrule a similar option given in the winboard.ini
or in the command line. Every engine option following an option 'WBopt' would be treated this way. As tournament
managers use the /fcp and /scp options to pass the engines to WinBoard, this can be used to have engine-
dependent settings of WinBoard during a tournament. For instance, if we are playing a tournament at 40/5' time
control, but we want to accomodate an engine that only supports incremental time controls, we can install that engine in the tournament manager as "badengine.exe hashsize=128 WBopt /timeIncrement=3". Every time WinBoard
would start up this engine, it would then use 5'+3" TC for that game.
 Options that apply to one engine only can also be passed this way. Such options contain 'first' or 'second' as
part of their name, depending on the engine they should apply to. As a given engine will play some of its games
as first and others as second during the tourney, to create an option that follows the engine, we need WinBoard
to dynamically decide if it should use the 'first' or 'second' flavor of the option, depending on which
engine startup command provided the option. This is indicated by letting the option contain '%s' in places
where WinBoard should read 'first' or 'second'. For example "strongengine.exe WBopt /%sTimeOdds=10" would
give the engine a time-odds factor of 10, whenever it plays, irrespective if it plays as first or second engine.
 Note that options that do not come in a 'first' or 'second' flavor can still be passed to only one engine by
putting the WinBoard protocol commands in the init string. E.g., if we want to limit the search depth of
a particular engine to 6 ply in all its games. we can give 'engine.exe WBopt /%sInitString="newnrandom\nsd 6\n"',
so that only the applicable engine receives the 'sd 6' command.