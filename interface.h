/* GTK widgets */
GtkBuilder              *builder=NULL;

GtkWidget               *GUI_Window=NULL;
GtkWidget               *GUI_History=NULL;
GtkWidget               *GUI_Board=NULL;
GtkWidget               *GUI_Whiteclock=NULL;
GtkWidget               *GUI_Blackclock=NULL;
GtkWidget               *GUI_Error=NULL;
GtkWidget               *GUI_Menubar=NULL;
GtkWidget               *GUI_Timer=NULL;
GtkWidget               *GUI_Buttonbar=NULL;

GtkListStore            *LIST_MoveHistory=NULL;

gint                     boardWidth;
gint                     boardHeight;


GdkPixbuf               *WindowIcon=NULL;
GdkPixbuf               *WhiteIcon=NULL;
GdkPixbuf               *BlackIcon=NULL;
GdkPixbuf               *SVGpieces[100];
GdkPixbuf               *SVGLightSquare=NULL;
GdkPixbuf               *SVGDarkSquare=NULL;
GdkPixbuf               *SVGNeutralSquare=NULL;

GdkCursor               *BoardCursor=NULL;


GdkPixbuf *load_pixbuf(char *filename,int size);
void GUI_SetAspectRatio(int ratio);
