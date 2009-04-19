Winboard_F.4.3.8 release notes

This Winboard supports the following new options (shown here with their default values):

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
 
The variant can be set from the newly added "File -> New Variant..." sub-menu. 
Extra board files are indicated by the letters i, j, k, l, ... For boards with more than 9 ranks, the counting starts at zero! Non-FIDE pieces will be referred to in FENs and PGN by letters that depend on the variant, and might collide with piece designators in other variants. E.g. in Xiangqi 'C' is a Cannon, in Capablanca Chess it is a Chancellor. Pieces that do not belong in a variant cannot be addressed in FEN and PGN either as long as that variant is selected, unless the letter assignment is overruled by the /pieceToCharTable option. The variant is not saved in the winboard.ini file; on start-up we always get variant "normal" unless we use the command-line opton, or have added the option to the winboard.ini file manually (in which case it will disappear when this file is overwritten). 
WinBoard_F knows the movement of all pieces occurring in Capablanca Chess (of which FIDE Chess is a subset), Shatranj, Courier, Xiangqi and 9x9 Shogi, so that these games can be played with legality testing enabled. 

/pieceToCharTable="PNBRQFWEMOUHACGSKpnbrqfwemouhacgsk" 
Each piece that WinBoard knows (in its legality test) has a letter associated with it, by which it will be referred to in FEN or PGN. The default assignment can be overruled with this option. The value has to be a string of even length, with at least 12 characters. The first half of the string designates the white pieces, the second half the black. 
The last letter for each color will be assigned to the King. (This is the piece that moves as an orthodox King; note that Nightmate and Xiangqi have a different royal piece.) All letters before it will be assigned to the other pieces in the order: 

P Pawn                 (move often depends on variant) 
N Knight               (move subtly different in Xiangqi (where it is written as H) or Shogi) 
B Bishop 
R Rook 
Q Queen                (Lance L in Shogi) 
F Ferz/General         (Silver General S in Shogi) 
W Wazir/GrandVizer     (Gold General G in Shogi, in Xiangqi it is royal and denoted by K) 
E Alfil/Elephant       (Moves subtly different in Xiangqi vs Shatranj/Courier) 
M Commoner/Man 
O Cannon/Pao 
U Unicorn              (representation of Royal Knight in Knightmate, used as promoted Pawn in Shogi) 
H Nightrider           (Promoted Knight in Shogi and CrazyHouse) 
A Archbishop/Cardinal  (Promoted Bishop in Shogi and CrazyHouse) 
C Chancellor/Marshall  (Promoted Rook   in Shogi and CrazyHouse) 
G Grasshopper          (Promoted Queen in Crazyhouse, promoted Lance in Shogi) 
S                      (Promoted Silver in Shogi) 
K King 
 
Pieces that are not mentioned (because the argument has less than 34 characters) will remain disabled. Mentioned pieces can be disabled by assigning them a '.' (period). They are then not recognized in FEN or PGN input. It is not advisable to disable a piece that is present in the opening position of the selected variant, though. 
Promoted pieces that need to be distinguished from original pieces of the same type (because of demotion on capture and transfer to the holdings) will be indicated by the letter for the unpromoted piece with a '+' in front of it (Shogi), or by the letter of the promoted piece with a '~' after it (Crazyhouse, Bughouse, in general everything with holdings that is not Shogi). 
All the new pieces have a native biytmap representation in the board sizes 'bulky' and 'middling'. For all window sizes that do not support such fairy bitmaps, promoted NBRQ are represented as a 2-sizes-smaller normal piece symbol, so that Crazyhouse can be played at any size. People disliking the fairy representations might even prefer that. 
There is an enhanced 'Edit Position' menu popup (right-clicking on the squares after selecting this mode in the main menu), featuring some common non-FIDE pieces, and 'promote' and 'demote' options to make those not directly in the menu. The promotion popup shows ArchBishop and Chancellor in Capablanca and Gothic, (or in fact in any game where this piece is not disabled), and leaves only the options YES / NO in Shogi. In Xiangqi there are no promotions.

/fontPieceToCharTable="PNBRQFWEMOUHACGSKpnbrqfwemouhacgsk" 
This option is similar to /pieceToCharTable, but sets the font character that is used to display the piece on the screen (when font-based rendering is in use), rather than in the FEN or PGN. The default setting should work with the WinboardF font, which uses the 'intuitive' mapping of font characters to symbols. 
Note that UHACGS are also used to represent the promoted versions of PNBRQF, in games like Crazyhouse and Shogi, where the promotion has to be undone on capture.

/boardWidth=-1 /boardHeight=-1 
Set a number of files and ranks of the playing board to a value that will override the defaults for the variant that is selected. A value of -1 means the variant default board size will be used for the corresponding parameter (and is itself the default value of these options). These parameters can be set in the "Files -> New Variant..." sub-menu, where they are reset to the default -1 is you OK the chosen variant without typing something to overrule it. These parameters are saved in the winboard.ini file. (But unless you saved while a variant with board-size override was selected, they will always be saved as -1.) 
A variant with a non-standard board size will be communicated to the engine(s) with the board size prefixed to the variant name, e.g. "variant 12x8_capablanca". In protocol 2 the engine must first enable this feature by sending "boardsizeFxR" amongst the accepted variants, where F is the maximum number of files, and R the maximum number of ranks, as decimal numbers. 

/holdingsSize=-1 
Set the size of the holdings for dropable pieces to a value that will override the default for the variant that is selected. A value of -1 means the variant default holdings size will be used for that parameter (and is itself the default value of this options). This parameter can be set in the Files -> New Variant... sub-menu, where it is reset to the default -1 is you OK the chosen variant without typing something to overrule it. This parameters is saved in the winboard.ini file. 
To disable holdings, set their size to 0. They will then not be displayed. For non-zero holding size N, the holdings are displayed left and right of the board, and piece drops can be effected by dragging pieces from the holdings to the drop square. In bughouse, the holdings will be filled by the ICS. In all other variants, captured pieces will go into the holdings (after reversing their color). Only the first N pieces of the /pieceToCharTable argument will go into the holdings. All other pieces will be converted to Pawns. (In Shogi, however they will be demoted in the regular way before determining if they fit.) Pieces that are disabled (per default and per /pieceToCharTable option) might not be counted when determining what are the first N pieces. 
Non-standard holdingsize will be communicated to the engine by prefixing it (together with the board size, even if this is standard) to the variant name, e.g. "variant 7x7+5_shogi". In protocol 2 the engine should enable this feature by sending "holdingsH" amongst the variant names, where H is the maximum acceptable holdings size as a decimal number. 

/alphaRank=FALSE 
When this parameter is true, a-h are converted to 1-9, and vice versa, in all move output and input (to PGN files or SAN move display as well as in communication with the engine). This might be useful for Shogi, where conventionally one uses letters to designate ranks, and digits to designate files. Engines that want to use this option must make sure pieces are never represented by lower case! This option can be set from the Files -> New Variant... menu, where it defaults to FALSE unless you explicitly set it. It is not saved in the winboard.ini file. 
This kludge does not seem to work for reading PGN files. Saving works fine. For now, using it is not recommended.
Note that the PGN format in Shogi also leaves out the trailing '+' as check indicator: In Shogi such a trailing '+' means promotion, while a trailing '=' means defer promotion. Prefix '+' signs are used on moves with promoted pieces, disambiguation is done western SAN style.

/allWhite=FALSE
Causes the outline of the 'white' pieces to be superimposed onto the 'black' piece symbols as well (as a black outline) when native bitmaps are used (as opposed to font-based rendering). This is useful if we choose a very light color to represent the 'black' pieces. It might be particularly useful in Shogi, where the conventional representation of the 'black' pieces is as upside-down white pieces, so that both colors would be white. This option is saved in the winboard.ini file, and can be set in the "Options -> Board..." sub-menu. 

/flipBlack=FALSE
Setting this option will cause upside-down display of the native piece bitmaps used to represent the pieces of the side that plays black, as would be needed for a traditional representation of Shogi pieces. It can be set from the "Options -> Board..." sub-menu, and it is saved in the winboard.ini file. For now, traditional Shogi bitmaps are not included, though.

/detectMate=TRUE 
/testClaim=TRUE 
/materialDraws=TRUE 
/trivialDraws=FALSE 
/ruleMoves=51 
/repeatsToDraw=6 
These are all options that only affect engine-engine play, and can be set from the "Options -> Engine..." sub-menu. They are all related to adjudication of games by the GUI. Legality checking must be switched on for them to work. 
If /detectMate is TRUE, the GUI recognizes checkmate and stalemate (but not in games with holdings!), and ends the game accordingly before the engines can claim. This is convenient for play with engines that fail to claim, and just exit. 
With /testClaim set, all result and illegal-move claims by engines that claim more than their own loss are scrutinized for validity, and false claims result in forfeit of the game. Useful with buggy engines. 
The option /materialDraws=TRUE causes games with insufficient mating material to be adjudicated immediately as draws, in case the engines would not claim them. 
The option /trivialDraws adjudicates KNNK, KBKB, KNKN, KBKN, KRKR and KQKQ to draws after 3 moves (to allow for a quick tactical win. Note that in KQKQ this might not be sound, but that problem would disappear once bitbase probing is added). 
The /ruleMoves determine after how many reversible moves the game is adjudicated as a draw. Setting this to 0 turns this option off. Draw claims by the engine are still accepted (by /testClaim) after 50 reversible moves, even if /ruleMoves species a larger number. Note that it is perfectly legal according to FIDE rules to play on after 50 reversible moves, but in tournaments having two engines that want to play on forever is a nuisance in endings like KBNKR, where one of the engines thinks it is ahead and can avoids repeats virtually forever. 
The option /repeatsToDraw makes the GUI adjudicate a game as draw after the same position has occurred the specified number of times. If it is set to a value > 3, engines can still claim the draw after 3-fold repeat. 
All these options are saved in the winboard.ini file. 

/matchPause=10000 
Determines the number of milliseconds that is paused between two games of a match. Saved in the Winboard.ini.

Clocks
There is an "Options -> flip Clocks" command, that swaps the position of white and black clocks (convenient in over-the-board matches, where the screen is next to the board, and you want your own time to be displayed on your side of the screen). The clocks can be adjusted in "edit game" mode: right-clicking them adds one minute, left-clicking subtracts one minute. (Also for OTB matches, to keep them synchronized with the official match clock.) The flag-fell condition is now indicated as (!) behind the time.

Other improvements / changes
Castling rights and e.p. rights are now fully maintained, and considered in legality testing. They are imported from and written to FEN, as is the 50-move counter. The time (in sec, or min:sec) is now always stored together with the PV information to the PGN, if storing the latter was requested (through ticking 'extended PGN info' in "Options -> General..."). The saved time is the Winboard clock time (as opposed to the time reported by the engine).
--------------------------------------------------------------------
Winboard_F.4.3.7 release notes

This Winboard supports the following new options (shown here with their default values):

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
 
The variant can be set from the newly added "File -> New Variant..." sub-menu. 
Extra board files are indicated by the letters i, j, k, l, ... For boards with more than 9 ranks, the counting starts at zero! Non-FIDE pieces will be referred to in FENs and PGN by letters that depend on the variant, and might collide with piece designators in other variants. E.g. in Xiangqi 'C' is a Cannon, in Capablanca Chess it is a Chancellor. Pieces that do not belong in a variant cannot be addressed in FEN and PGN either as long as that variant is selected, unless the letter assignment is overruled by the /pieceToCharTable option. The variant is not saved in the winboard.ini file; on start-up we always get variant "normal" unless we use the command-line opton, or have added the option to the winboard.ini file manually (in which case it will disappear when this file is overwritten). 
WinBoard_F knows the movement of all pieces occurring in Capablanca Chess (of which FIDE Chess is a subset), Shatranj, Courier, Xiangqi and 9x9 Shogi, so that these games can be played with legality testing enabled. 

/pieceToCharTable="PNBRQFWEMOUHACGSKpnbrqfwemouhacgsk" 
Each piece that WinBoard knows (in its legality test) has a letter associated with it, by which it will be referred to in FEN or PGN. The default assignment can be overruled with this option. The value has to be a string of even length, with at least 12 characters. The first half of the string designates the white pieces, the second half the black. 
The last letter for each color will be assigned to the King. (This is the piece that moves as an orthodox King; note that Nightmate and Xiangqi have a different royal piece.) All letters before it will be assigned to the other pieces in the order: 

P Pawn                 (move often depends on variant) 
N Knight               (move subtly different in Xiangqi (where it is written as H) or Shogi) 
B Bishop 
R Rook 
Q Queen                (Lance L in Shogi) 
F Ferz/General         (Silver General S in Shogi) 
W Wazir/GrandVizer     (Gold General G in Shogi, in Xiangqi it is royal and denoted by K) 
E Alfil/Elephant       (Moves subtly different in Xiangqi vs Shatranj/Courier) 
M Commoner/Man 
O Cannon/Pao 
U Unicorn              (representation of Royal Knight in Knightmate, used as promoted Pawn in Shogi) 
H Nightrider           (Promoted Knight in Shogi and CrazyHouse) 
A Archbishop/Cardinal  (Promoted Bishop in Shogi and CrazyHouse) 
C Chancellor/Marshall  (Promoted Rook   in Shogi and CrazyHouse) 
G Grasshopper          (Promoted Queen in Crazyhouse, promoted Lance in Shogi) 
S                      (Promoted Silver in Shogi) 
K King 
 
Pieces that are not mentioned (because the argument has less than 34 characters) will remain disabled. Mentioned pieces can be disabled by assigning them a '.' (period). They are then not recognized in FEN or PGN input. It is not advisable to disable a piece that is present in the opening position of the selected variant, though. 
Promoted pieces that need to be distinguished from original pieces of the same type (because of demotion on capture and transfer to the holdings) will be indicated by the letter for the unpromoted piece with a '+' in front of it (Shogi), or by the letter of the promoted piece with a '~' after it (Crazyhouse, Bughouse, in general everything with holdings that is not Shogi). 
All the new pieces have a native biytmap representation in the board sizes 'bulky' and 'middling'. For all window sizes that do not support such fairy bitmaps, promoted NBRQ are represented as a 2-sizes-smaller normal piece symbol, so that Crazyhouse can be played at any size. People disliking the fairy representations might even prefer that. 
There is an enhanced 'Edit Position' menu popup (right-clicking on the squares after selecting this mode in the main menu), featuring some common non-FIDE pieces, and 'promote' and 'demote' options to make those not directly in the menu. The promotion popup shows ArchBishop and Chancellor in Capablanca and Gothic, (or in fact in any game where this piece is not disabled), and leaves only the options YES / NO in Shogi. In Xiangqi there are no promotions.

/fontPieceToCharTable="PNBRQFWEMOUHACGSKpnbrqfwemouhacgsk" 
This option is similar to /pieceToCharTable, but sets the font character that is used to display the piece on the screen (when font-based rendering is in use), rather than in the FEN or PGN. The default setting should work with the WinboardF font, which uses the 'intuitive' mapping of font characters to symbols. 
Note that UHACGS are also used to represent the promoted versions of PNBRQF, in games like Crazyhouse and Shogi, where the promotion has to be undone on capture.

/boardWidth=-1 /boardHeight=-1 
Set a number of files and ranks of the playing board to a value that will override the defaults for the variant that is selected. A value of -1 means the variant default board size will be used for the corresponding parameter (and is itself the default value of these options). These parameters can be set in the "Files -> New Variant..." sub-menu, where they are reset to the default -1 is you OK the chosen variant without typing something to overrule it. These parameters are saved in the winboard.ini file. (But unless you saved while a variant with board-size override was selected, they will always be saved as -1.) 
A variant with a non-standard board size will be communicated to the engine(s) with the board size prefixed to the variant name, e.g. "variant 12x8_capablanca". In protocol 2 the engine must first enable this feature by sending "boardsizeFxR" amongst the accepted variants, where F is the maximum number of files, and R the maximum number of ranks, as decimal numbers. 

/holdingsSize=-1 
Set the size of the holdings for dropable pieces to a value that will override the default for the variant that is selected. A value of -1 means the variant default holdings size will be used for that parameter (and is itself the default value of this options). This parameter can be set in the Files -> New Variant... sub-menu, where it is reset to the default -1 is you OK the chosen variant without typing something to overrule it. This parameters is saved in the winboard.ini file. 
To disable holdings, set their size to 0. They will then not be displayed. For non-zero holding size N, the holdings are displayed left and right of the board, and piece drops can be effected by dragging pieces from the holdings to the drop square. In bughouse, the holdings will be filled by the ICS. In all other variants, captured pieces will go into the holdings (after reversing their color). Only the first N pieces of the /pieceToCharTable argument will go into the holdings. All other pieces will be converted to Pawns. (In Shogi, however they will be demoted in the regular way before determining if they fit.) Pieces that are disabled (per default and per /pieceToCharTable option) might not be counted when determining what are the first N pieces. 
Non-standard holdingsize will be communicated to the engine by prefixing it (together with the board size, even if this is standard) to the variant name, e.g. "variant 7x7+5_shogi". In protocol 2 the engine should enable this feature by sending "holdingsH" amongst the variant names, where H is the maximum acceptable holdings size as a decimal number. 

/alphaRank=FALSE 
When this parameter is true, a-h are converted to 1-9, and vice versa, in all move output and input (to PGN files or SAN move display as well as in communication with the engine). This might be useful for Shogi, where conventionally one uses letters to designate ranks, and digits to designate files. Engines that want to use this option must make sure pieces are never represented by lower case! This option can be set from the Files -> New Variant... menu, where it defaults to FALSE unless you explicitly set it. It is not saved in the winboard.ini file. 
Note that the PGN format in Shogi also leaves out the trailing '+' as check indicator: In Shogi such a trailing '+' means promotion, while a trailing '=' means defer promotion. Prefix '+' signs are used on moves with promoted pieces, disambiguation is done western SAN style.

/allWhite=FALSE
Causes the outline of the 'white' pieces to be superimposed onto the 'black' piece symbols as well (as a black outline) when native bitmaps are used (as opposed to font-based rendering). This is useful if we choose a very light color to represent the 'black' pieces. It might be particularly useful in Shogi, where the conventional representation of the 'black' pieces is as upside-down white pieces, so that both colors would be white. This option is saved in the winboard.ini file, and can be set in the "Options -> Board..." sub-menu. 

/flipBlack=FALSE
This option is reserved for future use. It will cause upside-down display of the 'black' native piece bitmaps, as would be needed for Shogi. For now it can be set from the "Options -> Board..." sub-menu, and it is saved in the winboard.ini file, but it is ignored.

/detectMate=TRUE 
/testClaim=TRUE 
/materialDraws=TRUE 
/trivialDraws=FALSE 
/ruleMoves=51 
/repeatsToDraw=6 
These are all options that only affect engine-engine play, and can be set from the "Options -> Engine..." sub-menu. They are all related to adjudication of games by the GUI. Legality checking must be switched on for them to work. 
If /detectMate is TRUE, the GUI recognizes checkmate and stalemate (but not in games with holdings!), and ends the game accordingly before the engines can claim. This is convenient for play with engines that fail to claim, and just exit. 
With /testClaim set, all result and illegal-move claims by engines that claim more than their own loss are scrutinized for validity, and false claims result in forfeit of the game. Useful with buggy engines. 
The option /materialDraws=TRUE causes games with insufficient mating material to be adjudicated immediately as draws, in case the engines would not claim them. 
The option /trivialDraws adjudicates KNNK, KBKB, KNKN, KBKN, KRKR and KQKQ to draws after 3 moves (to allow for a quick tactical win. Note that in KQKQ this might not be sound, but that problem would disappear once bitbase probing is added). 
The /ruleMoves determine after how many reversible moves the game is adjudicated as a draw. Setting this to 0 turns this option off. Draw claims by the engine are still accepted (by /testClaim) after 50 reversible moves, even if /ruleMoves species a larger number. Note that it is perfectly legal according to FIDE rules to play on after 50 reversible moves, but in tournaments having two engines that want to play on forever is a nuisance in endings like KBNKR, where one of the engines thinks it is ahead and can avoids repeats virtually forever. 
The option /repeatsToDraw makes the GUI adjudicate a game as draw after the same position has occurred the specified number of times. If it is set to a value > 3, engines can still claim the draw after 3-fold repeat. 
All these options are saved in the winboard.ini file. 

/matchPause=10000 
Determines the number of milliseconds that is paused between two games of a match. Saved in the Winboard.ini.

Clocks
There is an "Options -> flip Clocks" command, that swaps the position of white and black clocks (convenient in over-the-board matches, where the screen is next to the board, and you want your own time to be displayed on your side of the screen). The clocks can be adjusted in "edit game" mode: right-clicking them adds one minute, left-clicking subtracts one minute. (Also for OTB matches, to keep them synchronized with the official match clock.) The flag-fell condition is now indicated as (!) behind the time.

Other improvements
Castling rights and e.p. rights are now fully maintained, and considered in legality testing. They are imported from and written to FEN, as is the 50-move counter.
-------------------------------------------------------------------
These are the release notes of Winboard_F 4.3.2,
which is released under the GPL.
This version was derived from Allessandro Scotti's Winboard_x 4.2.7 source files.
It only includes the files that were changed:

config.h
common.h
parser.h
moves.h
resource.h

winboard.c
backend.c
moves.c

parser.l
winboard.rc

and some bitmaps for piece symbols



I made modifications in the following areas: 

1) Adjudication and claim verification 
2) Fairy pieces and board sizes other than 8x8 
3) Miscellaneous 

Miscellaneous 

/matchPause=10000 
is an option to set the length of the pause between two games of a match. The value is in msec, default value is 10000 (I will present all newly implemented options with their default value as example). Be aware that some engines might not be stopped yet if you make the pause too small, but might still be puking output, which then will interfere with the next game. But the fixed value of 10 sec of the old Winboard seemed like overdoing it. 

Time info in PGN 
When you ask for the PV-info to be stored in the PGN (a Winboard_x option), it now also stores the time spent on the move with it. 

Flag fell 
In engine-engine games the messge "white/black/both" flag(s) fell" no longer appears in the window caption, but as an exclamation point behind the clock time. (To prevent the annoying overwriting of the normal header line). 

Adjudications and Claim verification. 

These functions are only present in engine-engine games, and only if legality-testing is switched on. (The latter will be typically switched off in games with bizarre rules, which the GUI doesn't know, and in that case the GUI can never have an opinion on the outcome of a game.) 

Illegal-move forfeit 
As soon as one of the engines plays an illegal move, it forfeits the game. This feature was already present, but it should be 100% reliable now, as it also takes e.p. and castling rights into account, rather than erring on the safe side. 

Illegal-move claim 
From the above, it follows that any illegal-move claims by an engine must be false, and will result in forfeiting the game. (In Winboard_x this message is ignored, causing the game or match to hang.) 

Checkmate adjudication 
As soon as one of the engines does a move that results in checkmate, the GUI declares the game won, without waiting for the engine to claim it. 

Insufficient mating material 
As soon as the material on the board has shrunk to KK, KNK or KBK, the game is declared draw. 

/adjudicateLossThreshold=0 
This option was already present in Winboard_x, (to declare a game lost for which both engines agree for 3 moves that the score is below the given threshold), but a non-zero value is now also used to enable the following adjudications. If you only want the latter, just make the threshold impossibly low (-40000 will usually do the trick). 

Trivial draws 
If we are 3 moves into a KQKQ, KRKR, KBKB KBKN or KNKN end-game, the game is adjudicated as draw. 

/repeatsToDraw=6 
When the specified number of repeats occurs, the game is adjudicated draw. Should keep track of e.p. and castling rights. This does not require legality-testing to be switched on. The engines retain the legal right to claim after a 3-fold repetition, though. If you set this parameter to 3 or less, they will never get the chance. Better not set it to 1 or less. 

/ruleMoves=51 
After the given number of full moves without capture or Pawn move, the game is adjudicated draw. Even without legality testing. The engines retain the legal right to claim after 50 moves. 

/testClaims=FALSE 
When enabled, this option verifies all result claims made by the engines, and overrules the claim if it is false (forfeiting the game for the claimer). An engine can still safely claim a win for its opponent on a nonsense reason, though; this is taken to be the equivalent of 'resign'. Draw claims (made before a draw adjudication) are checked against the 50-move, 3-fold-repetition or insufficient-material rules. Win claims are always considered false, as the GUI adjudicates checkmates (and stalemates) before any engine can claim them. 

Fairy-Chess support 

/boardWidth=8 
Sets the number of files on the board. The additional files are named i, j, k, l... in PGN, and should be indicated this way in communicating moves to and from the engine. Currently works upto 12. No guarantees on how the rest of the display (clocks, etc.) looks if you make this number < 8. 

/boardHeight=8 
Sets the number of ranks. Extra ranks are numbered 9, 10, 11... in PGN. This is so far largely untested, and unlikely to work for double-digit ranks. Displaying boards with upto 12 ranks seems to work, though, but double-digit ranks might cause all kind of unforseen problems in PGN file and move parser, or in communication with the engine. To avoid such problems as much as possible, in boards with more than 9 ranks the counting of ranks will start at zero rather than one!

/fontPieceToCharTable="......." 
This paramater, controlling the mapping of font symbols to piece types, was already present in Winboard_x. The default is dependent on the font selected with the /renderPiecesWithFont option. It can now accept upto 32 pieces, but the length should always be even. The first half designates the white pieces, the second half the black, both in the order PNBRACHFEWDOGMQK. (The letters mentioned here are the letters by which the pieces will be indicated in PGN and FEN notation, what you have to give as argument depends on the font you use. A black king might be 'l', for instance).
If you give fewer then 32 pieces, this will go at the expense of what is just before Queen. So the last two symbols you give for each color are always Queen and King, the others are assigned in the order Pawn, Knight, ... Pieces that do not get a symbol assigned will remain invisible.

fairy-FEN support 
The letters ACDEFGHMOW are accepted in FENs in addition to the regular PNBRQK,(and of course the lower case versions for black), and are passed to the engine in a setboard or edit menu.
Double-digit skips are acceptable in FENs. 'x' is interpreted as a skip of 10. 
Castling rights should no longer be ignored. (Doesn't work for FRC yet, though.) 
The 50-move-plies field should also be meaningful now. 

Legality testing for Fairy pieces
There is a build in notion of what some of the new pieces can do, according to
A = Archbishop (aka Cardinal) = N+B
C = Chancellor (aka Marshall) = N+R
F = Ferz (aka General), moves 1 step diagonal
W = Wazir (aka Grand Vizer), moves 1 step orthogonal
E = Elephant (aka Alfil), jumps 2 steps diagonal
D = Dabbabah (aka War Machine), jumps 2 steps orthogonal
M = Man (aka Commoner), moves as King, but is not a royal piece
O = Cannon, moves along Rook lines, but cannot capture unless it jumps over
            exactly one piece (friend or foe), and captures the first piece
            it encounters thereafter. It can only jump if it captures something.
            The piece jumped over (the 'platform') is not affected.
The other pieces have not yet any moves implemented:
H = Nightrider
G = Grasshopper
In games that use the mentioned pieces as described (Shatranj, Courier, Capablanca Chess) you can leave legality testing on. If you use them to represent pieces that move differently, you should switch legality testing off.

Pawn moves
Pawn motion is made dependent on the variant played: in Shatranj and Courier the double move is forbidden. In those games promotions are always to Ferz. In Capablanca Chess the ArchBishop and Chancellor also appear as choices in the promotion popup box.


/variant="normal" 
Several new variants names are added (replacing "variant31" upto "variant36"). They affect the initial position. (Board size has to be set separately.) They are:
courier    (a Medieval predecessor of modern Chess, played on a 12x8 board)
capablanca (on a 10x8 board, featuring Archbishop and Chancellor)
gothic     (as Capablanca, but with a more exciting initial setup)
xiangqi    (Chinese Chess)
shogi      (Japanese Chess, no support yet)
fairy      (This variant plays on 8x8 with HEW in stead of NBR on the Queen side, 
            so that all back-rank pieces are (potentially) different)
Make sure the selected board size matches the variant; this is not automatic

Xiangqi and Shogi support (or lack thereof)
Xiangqi is only partially supported. The board display is western-style (the pieces play on squares, rather than grid intersections). The legality testing uses the Shatranj Knight and Elephant, that cannot be blocked, and thus allows some moves that should be illegal in Xiangqi. The Palace region is indicated on the board, but there is no testing if the King or Mandarins (implemented as ordinary Ferzes) leave the Palace. SAN might be non-standard, as it uses O to indicate Cannon. Most of this will be fixed in a future version.
Shogi is not yet supported at all, first the shaky implementation of Crazyhouse will have to be beefed up.

