void SendToProgram P((char *message, ChessProgramState *cps));
void SendToICS P((char *buf));
void InitDrawingSizes P((int i, int j));

extern int searchTime;
extern int squareSize, lineGap, defaultLineGap;
extern int startedFromPositionFile;
extern char *icsTextMenuString;

