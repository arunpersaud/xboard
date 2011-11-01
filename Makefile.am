### if we are compiling with ZIPPY we need some extra source files

if ZIPPY
  ZPY = zippy.c zippy.h
else
  ZPY =
endif

bin_PROGRAMS = xboard
xboard_SOURCES = backend.c backend.h backendz.h \
	 	 book.c \
		 childio.c childio.h \
	 	 common.h \
		 frontend.h \
	  	 gamelist.c \
	 	 gettext.h  \
		 gtk_helper.c gtk_helper.h \
		 lists.c lists.h \
		 moves.c moves.h \
		 parser.c parser.h \
		 pgntags.c \
	  	 uci.c \
	 	 xboard.c xboard.h args.h \
		 xedittags.c xedittags.h \
		 engineoutput.c engineoutput.h \
		 xengineoutput.c \
		 evalgraph.c evalgraph.h xevalgraph.c xevalgraph.h\
	 	 xgamelist.c xgamelist.h\
		 history.c xhistory.c xhistory.h \
		 xoptions.c \
		 $(ZPY)
SUBDIRS = po
xboard_LDADD = -lm @XAW_LIBS@ @X_LIBS@ @LIBINTL@ @GTK_LIBS@

svgdir = $(datadir)/games/xboard/svg
svg_DATA = svg/icon_white.svg svg/icon_black.svg \
           svg/DarkSquare.svg svg/LightSquare.svg svg/NeutralSquare.svg \
           svg/BlackArchbishop.svg  svg/WhiteArchbishop.svg	   \
           svg/BlackBishop.svg	    svg/WhiteBishop.svg         \
           svg/BlackCanon.svg	    svg/WhiteCanon.svg	      \
           svg/BlackChancellor.svg   svg/WhiteChancellor.svg    \
           svg/BlackCrownedBishop.svg  svg/WhiteCrownedBishop.svg  \
           svg/BlackCrownedRook.svg   svg/WhiteCrownedRook.svg    \
           svg/BlackGoldKnight.svg   svg/WhiteGoldKnight.svg    \
           svg/BlackGoldLance.svg   svg/WhiteGoldLance.svg    \
           svg/BlackGoldPawn.svg    svg/WhiteGoldPawn.svg     \
           svg/BlackGoldSilver.svg  svg/WhiteGoldSilver.svg   \
           svg/BlackGold.svg	    svg/WhiteGold.svg	      \
           svg/BlackKing.svg	    svg/WhiteKing.svg	      \
           svg/BlackKnight.svg	    svg/WhiteKnight.svg	      \
           svg/BlackLance.svg	    svg/WhiteLance.svg	      \
           svg/BlackMarshall.svg    svg/WhiteMarshall.svg     \
           svg/BlackNightrider.svg  svg/WhiteNightrider.svg   \
           svg/BlackPawn.svg	    svg/WhitePawn.svg	      \
           svg/BlackPrincess.svg    svg/WhitePrincess.svg     \
           svg/BlackQueen.svg	    svg/WhiteQueen.svg	      \
           svg/BlackRook.svg	    svg/WhiteRook.svg	      \
           svg/BlackUnicorn.svg	    svg/WhiteUnicorn.svg      \
           svg/eo_Analyzing.svg     svg/eo_Black.svg          \
           svg/eo_Clear.svg         svg/eo_Ponder.svg         \
           svg/eo_Thinking.svg      svg/eo_Unknown.svg        \
           svg/eo_White.svg

gladedir = $(datadir)/games/xboard/ui
glade_DATA = gtk/evalgraph.glade \
	     gtk/gamelist.glade \
	     gtk/mainboard.glade

EXTRA_DIST = svg sounds winboard  \
	xboard.texi gpl.texinfo texi2man texinfo.tex xboard.man xboard.desktop \
	COPYRIGHT FAQ.html engine-intf.html ics-parsing.txt readme.htm zippy.README \
	xboard.conf.in SHORTLOG DIFFSTAT history.c ABOUT-NLS $(glade_DATA)

DISTCLEANFILES = stamp-h

AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @X_CFLAGS@  -DSYSCONFDIR='"$(sysconfdir)"' \
	    -DGLADEDIR='"$(gladedir)"' \
	    -DSVGDIR='"$(svgdir)"' \
	    -DLOCALEDIR='"$(localedir)"'  @GTK_CFLAGS@

ACLOCAL_AMFLAGS = -I m4

info_TEXINFOS =  xboard.texi
xboard_TEXINFOS =  copyright.texi
man6_MANS = xboard.man


### config file

sysconf_DATA = xboard.conf


### icon files

icondir = $(datadir)/icons/hicolor/48x48/apps
dist_icon_DATA = xboard.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
dist_svgicon_DATA = xboard.svg


### desktop files for KDE and gnome

Applicationsdir = $(datadir)/applications
Applications_DATA = xboard.desktop


### directory and files for sound files

soundsdir = $(datadir)/games/xboard/sounds
dist_sounds_DATA = \
  sounds/cymbal.wav     sounds/pop2.wav   sounds/slap.wav     sounds/ding1.wav      sounds/laser.wav \
  sounds/woodthunk.wav  sounds/gong.wav   sounds/penalty.wav  sounds/honkhonk.wav   sounds/phone.wav


### files for svg files
dist_svg_DATA = \
  svg/BlackBishop.svg     svg/BlackKing.svg     svg/BlackKnight.svg     svg/BlackPawn.svg     svg/BlackQueen.svg \
  svg/BlackRook.svg       svg/DarkSquare.svg    svg/LightSquare.svg     svg/NeutralSquare.svg svg/WhiteBishop.svg \
  svg/WhiteKing.svg       svg/WhiteKnight.svg   svg/WhitePawn.svg       svg/WhiteQueen.svg    svg/WhiteRook.svg

### set correct dir in xboard.conf

edit = sed -e 's|@svgdir[@]|$(svgdir)|g' \
           -e 's|@soundsdir[@]|$(soundsdir)|g'

xboard.conf: Makefile $(srcdir)/xboard.conf.in
	@echo '  'GEN '  ' $@;
	@rm -f $@ $@.tmp
	@$(edit) $(srcdir)/$@.in >$@.tmp
	@mv $@.tmp $@

distclean-local:
	-rm -f xboard.conf

### the man page depends on several files, so we add a special rule

xboard.man: xboard.texi copyright.texi gpl.texinfo version.texi
	$(srcdir)/texi2man $(srcdir)/xboard.texi > xboard.man || (rm -f xboard.man ; false)
