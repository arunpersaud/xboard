var initDone = 0;
var board;
var save;
var savx;
var savy;
var msg;
var down = 0;
var size = 10;
var bw = 0;

function set(x, y, n) {
  sq = x + 'x' + y;
  if(n > 0) {
    board[x][y] = document.getElementById(sq).innerHTML;
    if(n == 2)
      document.getElementById(sq).innerHTML = '<img src="sym/cyan.png">';
    else if(n == 3 || n == 25)
      document.getElementById(sq).innerHTML = '<img src="sym/red.png">';
    else if(n == 10)
      document.getElementById(sq).innerHTML = '';
    else if(n == 4 || n == 20)
      document.getElementById(sq).innerHTML = '<img src="sym/green.png">';
    else if(n == 5)
      document.getElementById(sq).innerHTML = '<img src="sym/orange.png">';
    else if(n == 7)
      document.getElementById(sq).innerHTML = '<img src="sym/open.png">';
    else if(n == 8)
      document.getElementById(sq).innerHTML = '<img src="sym/WhitePawn.png">';
    else if(n == 11)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackPawn.png">';
    else if(n == 15)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackRook.png">';
    else if(n == 9 || n == 45)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackKing.png">';
    else if(n == 40)
      document.getElementById(sq).innerHTML = '<img src="sym/WhiteKnight.png">';
    else if(n == 50)
      document.getElementById(sq).innerHTML = '<img src="sym/WhiteHawk.png">';
    else if(n == 12 || n == 60)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackUnicorn.png">';
    else
      document.getElementById(sq).innerHTML = '<img src="sym/yellow.png">';
  } else
    document.getElementById(sq).innerHTML = board[x][y];
}

function text(s, t, n) {
  if(t == '') t = ':<br>:<br>:';
  if(n)
    document.getElementById('piece').innerHTML = s + '<br>' + t;
  else
    document.getElementById('piece').innerHTML = ':<br>:<br>:<br>:';
}

function slide(x, y, vx, vy, d, n) {
    xx = x + vx; yy = y + vy; c = d;
    if(vx > 1 || vx < -1 || vy > 1 || vy < -1) n = 5*n;
    while(d-- > 0 && xx >= bw && xx < size - bw && yy >= 0 && yy < size) {
	set(xx, yy, n);
	xx = xx + vx; yy = yy + vy;
	if(c == 2 && n == 1) n = 5;
    }
}

function slide_n(x, y, d, n) {
    slide(x, y, 2, 1, d, n);
    slide(x, y, 2,-1, d, n);
    slide(x, y,-2, 1, d, n);
    slide(x, y,-2,-1, d, n);
    slide(x, y, 1, 2, d, n);
    slide(x, y, 1,-2, d, n);
    slide(x, y,-1, 2, d, n);
    slide(x, y,-1,-2, d, n);
}

function slide_f(x, y, d, n) {
    slide(x, y, 0, 1, d, n);
}

function slide_b(x, y, d, n) {
    slide(x, y, 0, -1, d, n);
}

function slide_v(x, y, d, n) {
    slide_f(x, y, d, n);
    slide_b(x, y, d, n);
}

function slide_s(x, y, d, n) {
    slide(x, y, 1, 0, d, n);
    slide(x, y,-1, 0, d, n);
}

function slide_fd(x, y, d, n) {
    slide(x, y, 1, 1, d, n);
    slide(x, y,-1, 1, d, n);
}

function slide_bd(x, y, d, n) {
    slide(x, y, 1, -1, d, n);
    slide(x, y,-1, -1, d, n);
}

function slide_diag(x, y, d, n) {
    slide_fd(x, y, d, n);
    slide_bd(x, y, d, n);
}

function slide_orth(x, y, d, n) {
    slide_v(x, y, d, n);
    slide_s(x, y, d, n);
}

function highlight(x, y, m, n) {
   if(0) {
   } else if(x < 4 && y == 9) { // O-O-O
      if(x == 3) { x = 1; p = 12; } else p = (45-36*m); // Knightmate
      if(x == 2) { x = 0; z = 1; } else z = x; // Janus
      slide(x, y, 2 + m, 0, 1, 3*n);
      slide(x, y, 1 + m, 0, 1, p*n);
      slide(x, y, 5 - z, 0, 1, 2*n);
      slide(x, y, 0, 0, 1, 10*n);
      text('Castling', ': <i>King and Rook swing around each other</i><br>:<br>:', n);
   } else if(x > 6 && y == 9) { // O-O
      if(m == 2) { z = x + 1; m = 0; } else z = x; // King left
      if(x == 7) { x = 8; p = 12; z = x; } else p = (45-36*m); // Knightmate
      slide(x, y,-2 - m, 0, 1, 3*n);
      slide(x, y,-1 - m, 0, 1, p*n);
      slide(x, y, 5 - z, 0, 1, 2*n);
      slide(x, y, 0, 0, 1, 10*n);
      text('Castling', ': <i>King and Rook swing around each other</i><br>:<br>:', n);
   } else if((x == 4 || x == 5) && y == 9) { // castle
      if(m == 0) f = 15; else f = 3;
      slide(x, y, 2 + m, 0, 1, 4*n);
      if(m == 2) m = 1;
      slide(x, y,-2 - m, 0, 1, 4*n);
      text('Castling', ': <i>Click on Rook to see how</i><br>:<br>:', n);
   } else if(x == 1 && y == 3) { // FIDE e.p.
      slide(x, y, 1, 0, 1, 10*n);
      slide(x, y, 1,-1, 1, 11*n);
      slide(x, y, 0, 0, 1, 10*n);
      text('en-passant capture', ': <i>A Pawn that just made a double-push past you</i><br>: <i>is caught by its tail</i><br>:', n);
   } else if((x == 2 || x == 4) && y == 8) { // Berolina double-push
      slide_bd(x, y, 2, 4*n);
      slide_b(x, y, 1, 3*n);
      if(m == 1) text('Hoplit', ': <i>Different capture (red) and non-capture (green)</i><br>' +
                      ': <i>Initial double-push non-capture can jump!</i><br>:', n);
      else       text('Berlin Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>' +
                                     ': <i>Initial double-push non-capture can be blocked</i><br>' +
                                     ': <i>The white Pawn can e.p. capture to the skipped square in front of it', n);
   } else if(x == 2 && y == 1) { // FIDE double-push
      slide_fd(x, y, 1, 3*n);
      slide_f(x, y, 2, 4*n);
      if(m == 2) text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>' +
                              ': <i>Initial double-push non-capture can be blocked</i><br>' +
                              ': <i>The black Pawn can e.p. capture to the skipped square', n); else
      if(m == 1) text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>' +
                              ': <i>Initial double-push non-capture can be blocked</i><br>:', n);
      else       text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>:<br>:', n);
   } else if(x == 2 && y == 6) { // Berolina
      slide_bd(x, y, 1, 4*n);
      slide_b(x, y, 1, 3*n);
      if(m == 1) text('Hoplit', ': <i>Different capture (red) and non-capture (green)</i><br>:<br>:', n);
      else       text('Berlin Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>:<br>:', n);
   } else if(x == 2 && y == 3) { // FIDE Pawn
      slide_fd(x, y, 1, 3*n);
      slide_f(x, y, 1, 4*n);
      text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>:<br>:', n);
   } else if(x == 2 && y == 5) { // XQ passed Pawn
      slide_f(x, y, 1, n);
      slide_s(x, y, 1, n);
      text('Pawn (across the River)', '', n);
   } else if(x == 2 && y == 2) { // XQ Pawn
      slide_f(x, y, 1, n);
      text('Pawn', '', n);
   } else if(x == 3 && y == 6) { // Modern Elephant
      if(m == 1) { // Berolina e.p.
         slide(x, y, 0, 0, 1, 10*n);
         slide(x, y,-1, 0, 1, 10*n);
         slide(x, y, 0, 1, 1, 8*n);
         text('en-passant capture', ': <i>A Pawn that just made a double-push</i><br>' +
                                    ': <i>crossing your path is caught by its tail</i><br>' +
                                    ': <i>(Note the other white Pawn can NOT capture it!)</i>', n);
         return;
      }
      slide_diag(x, y, 2, n);
      if(m == 2) text('Modern Elephant', '', n); else {
         slide_s(x, y, 1, 4*n);
         text('Lieutenant', ': <i>Jumps directly to orange squares</i><br>' +
                            ': <i>Sideway step non-capture only</i><br>:', n);
      }
   } else if(x == 3 && y == 5) { // KAD
      slide_orth(x, y, 2, n);
      slide_diag(x, y, 2, n);
      text('Great General', '', n);
   } else if(x == 3 && y == 4) { // KN / NAD
      slide_n(x, y, 1, n);
      slide_diag(x, y, 2 - m, n);
      if(m == 1) { text('Veteran', '', n); slide_orth(x, y, 1, n); }
      else         text('High Priestess', '', n);
   } else if(x == 3 && y == 3) { // FvN
      slide_orth(x, y, size, n);
      text('Rook', '', n);
   } else if(x == 4 && y == 6) { // Dragon
      slide_diag(x, y, 1, n);
      slide_orth(x, y, size, n);
      text('General', '', n);
   } else if(x == 4 && y == 5) { // F
      slide_diag(x, y, 1, n);
      if(m == 2) text('Advisor (aka Mandarin or Palace Guard)', ': <i>Cannot leave Palace</i><br>:<br>:', n); else
      if(m == 1) text('Met (aka Ferz, General)', '', n);
      else       text('Ferz (aka General)', '', n);
   } else if(x == 4 && y == 4) { // Amazon / NWD
      slide_n(x, y, 1, n);
      slide_orth(x, y, 2 + m*size, n);
      if(m == 1) { text('Amazon', '', n); slide_diag(x, y, size, n); }
      else         text('Minister', '', n);
   } else if(x == 4 && y == 3) { // N
      if(m == 2) { slide_orth(x, y, 1, 7*n); f = 6; } else f = 1;
      slide_n(x, y, 1, f*n);
      if(m == 2) text('Horse', ': <i>Moves can be blocked on the adjacent square</i><br>' +
                               ': <i>marked with open circle, to which it cannot move</i><br>:', n); else
      if(m == 1) text('Royal Knight', ': <i>Royal piece that has to be checkmated</i><br>:<br>:', n);
      else       text('Knight', '', n);
   } else if(x == 5 && (y == 6 || y == 10)) { // BN
      if(y == 10) { text('Hawk', '', n); y = 6; } else
      if(m == 3) text('Janus', '', n); else
      if(m == 2) text('Princess', '', n); else
      if(m == 1) text('Warlord', '', n);
      else       text('Archbishop', '', n);
      slide_diag(x, y, size, n);
      slide_n(x, y, 1, n);
   } else if(x == 5 && y == 5) { // Alfil / XQ Elephant
      f = n*(1 + 5*m);
      slide(x, y, 2, 2, 1, f);
      slide(x, y,-2, 2, 1, f);
      slide(x, y, 2,-2, 1, f);
      slide(x, y,-2,-2, 1, f);
      if(m == 1) {
         slide_diag(x, y, 1, 7*n);
         text('Elephant', ': <i>Can be blocked on the nearest square marked</i><br>' +
                          ': <i>with open circle, to which it cannot move</i><br>' +
                          ': <i>It cannot cross the River separating board halves</i>', n);
      } else
      text('Elephant (aka Alfil)', '', n);
   } else if(x == 5 && y == 4) { // RN
      slide_orth(x, y, size, n);
      slide_n(x, y, 1, n);
      if(m == 3) text('Empress', '', n); else
      if(m == 2) text('Elephant', '', n); else
      if(m == 1) text('Marshall', '', n);
      else       text('Chancellor', '', n);
   } else if(x == 5 && y == 3) { // Q
      slide_orth(x, y, size, n);
      slide_diag(x, y, size, n);
      text('Queen', '', n);
   } else if(x == 6 && y == 6) { // WD
      slide_orth(x, y, 2, n);
      if(m == 2) text('War Machine', '', n);
      else       text('Captain', ': <i>Jumps directly to orange squares</i><br>:<br>:', n);
   } else if(x == 6 && y == 5) { // Wazir
      slide_orth(x, y, 1, n);
      if(m == 1) text('King (aka General)', ': <i>Royal piece that has to be checkmated</i><br>: <i>Cannot leave Palace</i><br>:', n);
      else       text('Wazir (aka Grandvizer)', '', n);
   } else if(x == 6 && y == 4) { // NN
      slide_n(x, y, size, 4*n);
      text('Nightrider', '', n);
   } else if(x == 6 && y == 3) { // B
      slide_diag(x, y, size, n);
      text('Bishop', '', n);
   } else if(x == 6 && y == 2) { // Silver
      slide_diag(x, y, 1, n);
      slide_f(x, y, 1, n);
      text('Elephant', '', n);
   } else if(x == 7 && y == 2) { // XQ Canon
      slide_v(x, y, 3, 4*n);
      slide_s(x, y, 4, 4*n);
      slide(x, y, 0, 7, 1, 5*n);
      slide(x, y,-6, 0, 1, 5*n);
      text('Canon', ': <i>Slides non-capturing to first obstacle</i><br>' +
                    ': <i>and can capture first piece behind that</i><br>:', n);
   } else if(x == 7 && y == 0) { // Gating
      slide(x, y, 0, 0, 1, 50*n);
      slide(x, y,-1, 2, 1, 8*n);
      bw = 0; slide(x, y, 2, 0, 1, 2*n); bw = 1;
      text('Gating:', ': <i>On the first move of a back-rank piece</i><br>' +
                      ': <i>your Hawk or Elephant can appear from under it</i><br>:', n);
   } else if(x == 7 && (y == 3 || y == 6)) { // K
      slide_orth(x, y, 1, n);
      slide_diag(x, y, 1, n);
      if(m == 2) text('Soldier', '', n); else
      if(m == 1) text('Commoner (aka Man)', '', n);
      else       text('King', '', n);
   }
}

function down_click(x, y) {
   if(!initDone) {
      initDone = 1;
      board = new Array();
      for(i=0; i<12; i = i + 1) board[i] = new Array();
      if(document.getElementById('0x0').innerHTML[1] == ' ') bw = 1;
   }
   if(down) return;
   if(y < 0) {
      y = -y; msg = 1;
   } else msg = 0;
   if(x < 0) {
      x = -x; msg = msg + 2;
   }
   highlight(x, y, msg, 1);
   savx = x; savy = y; down = 1;
//   save = document.getElementById(sq).innerHTML;
//   document.getElementById(sq).innerHTML = '<img src="sym/yellow.png">';
}

function up_click() {
   highlight(savx, savy, msg, 0);
   down = 0;
//   document.getElementById(x + 'x' + y).innerHTML = save;
}


