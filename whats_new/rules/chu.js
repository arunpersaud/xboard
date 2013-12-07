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
  sq = 'sq' + x + 'x' + y;
  if(n > 0) {
    board[x][y] = document.getElementById(sq).innerHTML;
    if(n == 2)
      document.getElementById(sq).innerHTML = '<img src="sym/cyan.png">';
    else if(n == 3)
      document.getElementById(sq).innerHTML = '<img src="sym/red.png">';
    else if(n == 10)
      document.getElementById(sq).innerHTML = '';
    else if(n == 4 || n == 8 || n == 20)
      document.getElementById(sq).innerHTML = '<img src="sym/green.png">';
    else if(n == 5)
      document.getElementById(sq).innerHTML = '<img src="sym/orange.png">';
    else if(n == 15)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackRook.png">';
    else if(n == 9 || n == 45)
      document.getElementById(sq).innerHTML = '<img src="sym/BlackKing.png">';
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
    xx = x + vx; yy = y + vy;
    if(vx > 1 || vx < -1 || vy > 1 || vy < -1) n = 5*n;
    if(d == 2) {
        set(xx, yy, 2*n);
	xx = xx + vx; yy = yy + vy; d--; n = 5*n;
    }
    while(d-- > 0 && xx >= bw && xx < size - bw && yy >= 0 && yy < size) {
	set(xx, yy, n);
	xx = xx + vx; yy = yy + vy;
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
   } else if(x > 7 && y == 9) { // castle
      slide(x, y,-1-m, 0, 1, 9*n);
      slide(x, y, 0, 0, 1, 10*n);
      slide(x, y,-2-m, 0, 1, 3*n);
      slide(x, y,5-m-x, 0, 1, 2*n);
      text('Castling', ': <i>The King and Rook swing around each other</i><br>:<br>:', n);
   } else if(x < 2 && y == 9) { // castle
      slide(x, y, 2-m, 0, 1, 9*n);
      slide(x, y, 0, 0, 1, 10*n);
      slide(x, y, 3-m, 0, 1, 3*n);
      slide(x, y, 5-m-x, 0, 1, 2*n);
      text('Castling', ': <i>The King and Rook swing around each other</i><br>:<br>:', n);
   } else if((x == 4 || x == 5) && y == 9) { // castle
      slide(x, y, 2+m, 0, 1, 4*n);
      slide(x, y,-2-m, 0, 1, 4*n);
      text('Castling', ': <i>Click on Rook to see how</i><br>:<br>:', n);
   } else if(x == 2 && y == 1) { // FL
      slide_fd(x, y, 1, 3*n);
      slide_f(x, y, 2, 4*n);
      text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>' +
                   ': <i>Initial double-push non-capture can be blocked</i><br>' +
                   ': <i>The black Pawn can e.p. capture to the skipped square', n);
   } else if(x == 7 && y == 2) { // FL
      slide_fd(x, y, 1, 3*n);
      slide_f(x, y, 1, 4*n);
      text('Pawn', ': <i>Different capture (red) and non-capture (green)</i><br>:<br>:', n);
   } else if(x == 2 && y == 5) { // V
      slide_v(x, y, size, n);
      slide_s(x, y, 1, n);
      text('Vertical Mover', '', n);
   } else if(x == 2 && y == 4) { // H
      slide_s(x, y, size, n);
      slide_v(x, y, 1, n);
      text('Side Mover', '', n);
   } else if(x == 2 && y == 3) { // I
      slide_v(x, y, 1, n);
      text('Cobra (aka Go Between)', '', n);
   } else if(x == 3 && y == 7) { // G
      slide_orth(x, y, 1, n);
      slide_fd(x, y, 1, n);
      text('Gold General', '', n);
   } else if(x == 3 && y == 6) { // FL
      slide_diag(x, y, 1, n);
      slide_v(x, y, 1, n);
      text('Ferocious Leopard', '', n);
   } else if(x == 3 && y == 5) { // +V
      slide_diag(x, y, size, n);
      slide_v(x, y, size, n);
      text('Narrow Queen (aka Flying Ox)', '', n);
   } else if(x == 3 && y == 4) { // +H
      slide_diag(x, y, size, n);
      slide_s(x, y, size, n);
      text('Sleeping Queen (aka Free Boar)', '', n);
   } else if(x == 3 && y == 3) { // Ky
      slide_diag(x, y, 1, n);
      slide(x, y, 2, 0, 1, n);
      slide(x, y,-2, 0, 1, n);
      slide(x, y, 0,-2, 1, n);
      slide(x, y, 0, 2, 1, n);
      text('Kylin', '', n);
   } else if(x == 3 && y == 2) { // Ky
      slide(x, y, 1, 2, 1, n);
      slide(x, y,-1, 2, 1, n);
      text('(Shogi) Knight', '', n);
   } else if(x == 4 && y == 7) { // S
      slide_diag(x, y, 1, n);
      slide_f(x, y, 1, n);
      text('Silver General', '', n);
   } else if(x == 4 && y == 6) { // +A
      slide_v(x, y, size, n);
      slide_bd(x, y, size, n);
      text('Whale', '', n);
   } else if(x == 4 && y == 5) { // +D
      slide_orth(x, y, size, n);
      slide_bd(x, y, size, n);
      slide_fd(x, y, 2, n);
      text('Soaring Eagle',
           '<i>: Can shoot or capture en-passant what is on the cyan<br>' +
              ': square with the first of its two steps along the forward<br>' +
              ': diagonal, or jump directly to cyan or orange squares</i>', n);
   } else if(x == 4 && y == 4) { // Lion
      slide_diag(x, y, 2, n);
      slide_orth(x, y, 2, n);
      slide_n(x, y, 1, n);
      text('Lion', '<i>: Can shoot or capture en-passant what is on the<br>' +
                   ': cyan squares with the first of its two King steps,<br>' +
                   ': or jump directly to any of the colored squares</i>', n);
   } else if(x == 4 && y == 3) { // +L
      if(m == 1) {
         slide_n(x, y, 1, n);
         text('Knight', '', n);
      } else {
         slide_v(x, y, size, n);
         slide_fd(x, y, size, n);
         text('White Horse', '', n);
      }
   } else if(x == 4 && y == 2) { // L
      slide_f(x, y, size, n);
      text('Lance', '', n);
   } else if(x == 5 && y == 7) { // C
      slide_fd(x, y, 1, n);
      slide_v(x, y, 1, n);
      text('Copper General', '', n);
   } else if(x == 5 && y == 6) { // +A
      slide_v(x, y, size, n);
      text('Canon (aka Reverse Chariot)', '', n);
   } else if(x == 5 && y == 5) { // +H
      slide_diag(x, y, size, n);
      slide_s(x, y, size, n);
      slide_b(x, y, size, n);
      slide_f(x, y, 2, n);
      text('Unicorn (aka Horned Falcon)',
           '<i>: Can shoot or capture en-passant what is on the cyan<br>' +
              ': square with the first of its two forward/backward<br>' +
              ': steps, or jump directly to cyan or orange square</i>', n);
   } else if(x == 5 && y == 4) { // Q
      slide_diag(x, y, size, n);
      slide_orth(x, y, size, n);
      text('Queen', '', n);
   } else if(x == 5 && y == 3) { // +BT
      slide_v(x, y, size, n);
      slide_s(x, y, 1, n);
      slide_diag(x, y, 1, n);
      text('Flying Stag', '', n);
   } else if(x == 5 && y == 2) { // BT
      slide_diag(x, y, 1, n);
      slide_s(x, y, 1, n);
      slide_b(x, y, 1, n);
      text('Blind Tiger', '', n);
   } else if(x == 6 && y == 7) { // Commoner
      slide_diag(x, y, 1, n);
      slide_orth(x, y, 1, n);
      text('Soldier', '', n);
   } else if(x == 6 && y == 6) { // King
      slide_diag(x, y, 1, n);
      slide_orth(x, y, 1, n);
      text('King', '', n);
   } else if(x == 6 && y == 5) { // dragon
      slide_orth(x, y, size, n);
      slide_diag(x, y, 1, n);
      text('Dragon King', '', n);
   } else if(x == 6 && y == 4) { // horse
      slide_diag(x, y, size, n);
      slide_orth(x, y, 1, n);
      text('Dragon Horse', '', n);
   } else if(x == 6 && y == 3) { // Ph
      slide_orth(x, y, 1, n);
      slide(x, y, 2, 2, 1, n);
      slide(x, y,-2, 2, 1, n);
      slide(x, y, 2,-2, 1, n);
      slide(x, y,-2,-2, 1, n);
      text('Phoenix', '', n);
   } else if(x == 6 && y == 2) { // P
      slide_f(x, y, 1, n);
      text('Pawn', '', n);
   } else if(x == 7 && y == 6) { // E
      slide_diag(x, y, 1, n);
      slide_s(x, y, 1, n);
      slide_f(x, y, 1, n);
      text('Elephant', '', n);
   } else if(x == 7 && y == 5) { // rook
      slide_orth(x, y, size, n);
      text('Rook', '', n);
   } else if(x == 7 && y == 4) { // bishop
      slide_diag(x, y, size, n);
      text('Bishop', '', n);
   }
}

function down_click(x, y) {
   if(!initDone) {
      initDone = 1;
      board = new Array();
      for(i=0; i<12; i = i + 1) board[i] = new Array();
      if(document.getElementById('sq0x0').innerHTML[1] == ' ') bw = 1;
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
//   document.getElementById('sq' + x + 'x' + y).innerHTML = save;
}


