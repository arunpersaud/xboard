
if ZIPPY
  ZPY = zippy.c
else 
  ZPY =
endif 

bin_PROGRAMS = xboard
xboard_SOURCES = backend.c lists.c pgntags.c xedittags.c xhistory.c $(ZPY) childio.c moves.c uci.c xengineoutput.c xoptions.c gamelist.c parser.l xboard.c xgamelist.c book.c callback.c interface.c

AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @GTK_CFLAGS@
AM_LDFLAGS= -lm @X_PRE_LIBS@ @XAW_LIBS@ @X_LIBS@ @X_EXTRA_LIBS@ @GTK_LIBS@

info_TEXINFOS = copyright.texi xboard.texi
man6_MANS = xboard.man 

xboard.man: xboard.texi copyright.texi gpl.texinfo version.texi
	$(srcdir)/texi2man xboard.texi > xboard.man || (rm -f xboard.man ; false)
