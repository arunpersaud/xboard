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

