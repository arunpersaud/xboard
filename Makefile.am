bin_PROGRAMS = xboard
xboard_SOURCES = backend.c lists.c pgntags.c xedittags.c xhistory.c zippy.c childio.c moves.c uci.c xengineoutput.c xoptions.c gamelist.c parser.l xboard.c xgamelist.c book.c

AM_CPPFLAGS=-DINFODIR='"$(infodir)"'
AM_LDFLAGS=-lXt -lm @XAW_LIBS@

info_TEXINFOS = copyright.texi xboard.texi
man6_MANS = xboard.man 

xboard.man: xboard.texinfo xboard.texi copyright.texi gpl.texinfo
	$(srcdir)/texi2man xboard.texinfo > xboard.man || (rm -f xboard.man ; false)

