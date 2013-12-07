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
    else if(n == 3)
      document.getElementById(sq).innerHTML = '<img src="sym/red.png">';
    else if(n == 10)
      document.getElementById(sq).innerHTML = '';
    else if(n == 4 || n == 20)
      document.getElementById(sq).innerHTML = '<img src="sym/green.png">';
    else if(n == 5)
      document.getElementById(sq).innerHTML = '<img src="sym/orange.png">';
    else if(n == 11)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackPawn.png">';
    else if(n == 15)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackRook.png">';
    else if(n == 30)
      document.getElementById(sq).innerHTML = '<img src="sym/WhiteCrownedBishop.png">';
    else if(n == 50)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackKing.png">';
    else if(n == 60)
      document.getElementById(sq).innerHTML = '<img src="sym/WhiteKing.png">';
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
   } else if(x == 1 && y == 9) { // castle
      slide(x, y, 0, 0, 1, 10*n);
      slide(x, y, 2, 0, 1, 10*n);
      slide(x, y, 3, 0, 1, 3*n);
      slide(x, y, 4, 0, 1, 2*n);
      text('Castling', ': <i>King and corner piece swing around each other</i><br>:<br>:', n);
   } else if(x == 8 && y == 9) { // castle
      slide(x, y, 0, 0, 1, 10*n);
      slide(x, y,-1, 0, 1, 50*n);
      slide(x, y,-2, 0, 1, 3*n);
      slide(x, y,-3, 0, 1, 2*n);
      text('Castling', ': <i>King and corner piece swing around each other</i><br>:<br>:', n);
   } else if(x == 5 && y == 9) { // castle
      slide(x, y, 2, 0, 1, 4*n);
      slide(x, y,-2, 0, 1, 4*n);
      text('Castling', ': <i>Click on a corner piece to see how</i><br>:<br>:', n);
   } else if(x == 1 && y == 0) { // castle
      slide(x, y, 0, 0, 1, 10*n);
      slide(x, y, 1, 0, 1, 60*n);
      slide(x, y, 2, 0, 1, 6*n);
      slide(x, y, 4, 0, 1, 2*n);
      text('Castling', ': <i>King and corner piece swing around each other</i><br>' +
                       ': <b>King goes to b1, to keep its partner on same color!</b><br>:', n);
   } else if(x == 8 && y == 0) { // castle
      slide(x, y, 0, 0, 1, 10*n);
      slide(x, y,-1, 0, 1, 60*n);
      slide(x, y,-2, 0, 1, 6*n);
      slide(x, y,-3, 0, 1, 2*n);
      text('Castling', ': <i>King and corner piece swing around each other</i><br>:<br>:', n);
   } else if(x == 5 && y == 0) { // castle
      slide(x, y, 2, 0, 1, 4*n);
      slide(x, y,-3, 0, 1, 4*n);
      text('Castling <b>with color-bound corner piece</b>', ': <i>Click on a corner piece to see how</i><br>:<br>:', n);
   } else if(x == 1 && y == 3) { // FL
      slide(x, y, 1, 0, 1, 10*n);
      slide(x, y, 1,-1, 1, 11*n);
      slide(x, y, 0, 0, 1, 10*n);
      text('en-passant capture', ': <i>A Pawn that just made a double-push past you</i><br>: <i>is caught by its tail</i><br>:', n);
   } else if(x == 2 && y == 1) { // FL
      slide_fd(x, y, 1, 3*n);
      slide_f(x, y, 2, 4*n);
      if(m == 2) text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>' +
                              ': <i>Initial double-push non-capture can be blocked</i><br>' +
                              ': <i>The black Pawn can e.p. capture to the skipped square', n);
      else       text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>:<br>:', n);
   } else if(x == 2 && y == 3) { // FL
      slide_fd(x, y, 1, 3*n);
      slide_f(x, y, 1, 4*n);
      text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>:<br>:', n);
   } else if(x == 3 && y == 6) { // FL
      slide_orth(x, y, size, n);
      text('Rook', '', n);
   } else if(x == 3 && y == 5) { // +V
      slide_orth(x, y, 4, n);
      text('Short Rook', ': <i>Slides upto 4 squares</i><br>:<br>:', n);
   } else if(x == 3 && y == 4) { // +H
      slide_orth(x, y, 1, n);
      slide(x, y, 2, 2, 1, n);
      slide(x, y,-2, 2, 1, n);
      slide(x, y, 2,-2, 1, n);
      slide(x, y,-2,-2, 1, n);
      text('Elephant', '', n);
   } else if(x == 3 && y == 3) { // fibnif
      slide_diag(x, y, 1, n);
      slide(x, y, 1, 2, 1, n);
      slide(x, y,-1, 2, 1, n);
      slide(x, y, 1,-2, 1, n);
      slide(x, y,-1,-2, 1, n);
      text('Horse', '', n);
   } else if(x == 4 && y == 6) { // +A
      slide_n(x, y, 1, n);
      text('Knight', '', n);
   } else if(x == 4 && y == 5) { // +D
      slide_diag(x, y, 1, n);
      slide(x, y, 2, 0, 1, n);
      slide(x, y,-2, 0, 1, n);
      slide(x, y, 0,-2, 1, n);
      slide(x, y, 0, 2, 1, n);
      slide(x, y, 3, 0, 1, n);
      slide(x, y,-3, 0, 1, n);
      slide(x, y, 0,-3, 1, n);
      slide(x, y, 0, 3, 1, n);
      text('Half-Duck', '', n);
   } else if(x == 4 && y == 4) { // Lion
      slide_diag(x, y, size, n);
      slide(x, y, 2, 0, 1, n);
      slide(x, y,-2, 0, 1, n);
      slide(x, y, 0,-2, 1, n);
      slide(x, y, 0, 2, 1, n);
      text('Leaping Bishop', ': <i>Color bound</i><br>:<br>:', n);
   } else if(x == 4 && y == 3) { // +L
      slide(x, y, 1, 2, 1, n);
      slide(x, y,-1, 2, 1, n);
      slide(x, y, 2, 1, 1, n);
      slide(x, y,-2, 1, 1, n);
      slide_b(x, y, 1, n);
      slide_s(x, y, 1, n);
      slide_bd(x, y, 1, n);
      text('Unicorn', '', n);
   } else if(x == 5 && y == 6) { // +A
      slide_orth(x, y, size, n);
      slide_diag(x, y, size, n);
      text('Queen', '', n);
   } else if(x == 5 && y == 5) { // +H
      slide_orth(x, y, size, n);
      slide_n(x, y, 1, n);
      text('Marshall', '', n);
   } else if(x == 5 && y == 4) { // Q
      slide_diag(x, y, size, n);
      slide_n(x, y, 1, n);
      text('Archbishop', '', n);
   } else if(x == 5 && y == 3) {
      slide(x, y, 1, 2, 1, n);
      slide(x, y,-1, 2, 1, n);
      slide(x, y, 2, 1, 1, n);
      slide(x, y,-2, 1, 1, n);
      slide_diag(x, y, 1, n);
      slide_f(x, y, size, n);
      slide_s(x, y, size, n);
      slide_b(x, y, 1, n);
      text('Colonel', '', n);
   } else if(x == 6 && y == 6) { // B
      slide_diag(x, y, size, n);
      text('Bishop', '', n);
   } else if(x == 6 && y == 5) { // dragon
      slide_orth(x, y, 2, n);
      text('Woody', '', n);
   } else if(x == 6 && y == 4) { // horse
      slide(x, y, 2, 0, 1, n);
      slide(x, y,-2, 0, 1, n);
      slide(x, y, 0,-2, 1, n);
      slide(x, y, 0, 2, 1, n);
      slide_diag(x, y, 2, n);
      text('Clobberer', ': <i>Color bound</i><br>:<br>:', n);
   } else if(x == 6 && y == 3) { // Ph
      slide_b(x, y, 1, n);
      slide_bd(x, y, 1, n);
      slide_f(x, y, size, n);
      slide_s(x, y, size, n);
      text('Turret', '', n);
   } else if(x == 7 && y == 6) { // Ph
      slide_diag(x, y, 1, n);
      slide_orth(x, y, 1, n);
      text('King', '', n);
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


