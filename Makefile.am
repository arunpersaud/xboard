bin_PROGRAMS = xboard

### if we are compiling with ZIPPY we need some extra source files

if ZIPPY
  ZPY = zippy.c zippy.h
else
  ZPY =
endif

### define sources for the front-end and backend
GTKsources = gtk/xboard.c    gtk/xboard.h \
             gtk/xoptions.c  gtk/xtimer.c \
             gtk/xengineoutput.c

Xsources   = xaw/xboard.c    xaw/xboard.h    \
             xaw/xengineoutput.c             \
             xaw/xgamelist.c xaw/xgamelist.h \
             xaw/xhistory.c  xaw/xhistory.h  \
             xaw/xoptions.c \
             bitmaps/checkmark.bm bitmaps/icon_black.bm bitmaps/icon_white.bm

if withGTK
  FRONTENDsources = $(GTKsources)
  FRONTENDextras = $(Xsources)
  headers = -I gtk
endif
if withXaw
  FRONTENDsources = $(Xsources)
  FRONTENDextras = $(GTKsources)
  headers = -I xaw -DX11
endif

xboard_SOURCES = backend.c backend.h backendz.h \
	 	 book.c \
		 childio.c childio.h \
	 	 common.h \
		 frontend.h \
	  	 gamelist.c ngamelist.c \
	 	 gettext.h  \
		 lists.c lists.h \
		 moves.c moves.h \
		 parser.c parser.h \
		 pgntags.c \
	  	 uci.c \
	 	 args.h xboard2.h \
	 	 board.c board.h \
	 	 draw.c draw.h \
		 dialogs.c dialogs.h \
		 engineoutput.c nengineoutput.c engineoutput.h \
		 evalgraph.c nevalgraph.c evalgraph.h \
		 history.c nhistory.c \
	 	 menus.c menus.h \
	 	 usounds.c usystem.c usystem.h \
		 $(ZPY) $(FRONTENDsources)

###

SUBDIRS = po
xboard_LDADD = -lm @FRONTEND_LIBS@ @X_LIBS@ @LIBINTL@ @CAIRO_LIBS@

EXTRA_DIST = pixmaps themes png sounds winboard \
	xboard.texi gpl.texinfo texi2man texinfo.tex xboard.man xboard.desktop xboard-config.desktop \
	xboard-fen-viewer.desktop  xboard-pgn-viewer.desktop  xboard-tourney.desktop xboard.xml \
	COPYRIGHT FAQ.html engine-intf.html ics-parsing.txt readme.htm zippy.README \
	xboard.conf.in SHORTLOG DIFFSTAT history.c ABOUT-NLS $(FRONTENDextras)

DISTCLEANFILES = stamp-h

GITVERSION=$(shell sh -c 'git describe --dirty --always 2>/dev/null')
AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @X_CFLAGS@ @CAIRO_CFLAGS@ @FRONTEND_CFLAGS@  -DSYSCONFDIR='"$(sysconfdir)"' \
	    -DLOCALEDIR='"$(localedir)"' -DSVGDIR='"$(svgdir)"' -D__GIT_VERSION='"$(GITVERSION)"' $(headers)

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
Applications_DATA = xboard.desktop xboard-fen-viewer.desktop  xboard-pgn-viewer.desktop  xboard-tourney.desktop  xboard-config.desktop

### mime file
mimedir = $(datadir)/mime/packages
mime_DATA = xboard.xml

### directory and files for svgs

svgdir = $(datadir)/games/xboard/themes/default
dist_svg_DATA = svg/icon_white.svg         svg/icon_black.svg      \
           svg/BlackAdvisor.svg       svg/WhiteAdvisor.svg    \
           svg/BlackArchbishop.svg    svg/WhiteArchbishop.svg  \
           svg/BlackBishop.svg	      svg/WhiteBishop.svg      \
           svg/BlackCanon.svg	      svg/WhiteCanon.svg       \
           svg/BlackChancellor.svg    svg/WhiteChancellor.svg  \
           svg/BlackCobra.svg         svg/WhiteCobra.svg       \
           svg/BlackCommoner.svg      svg/WhiteCommoner.svg    \
           svg/BlackCrownedBishop.svg svg/WhiteCrownedBishop.svg  \
           svg/BlackCrownedRook.svg   svg/WhiteCrownedRook.svg \
           svg/BlackElephant.svg      svg/WhiteElephant.svg    \
           svg/BlackGoldKnight.svg    svg/WhiteGoldKnight.svg  \
           svg/BlackGoldLance.svg   svg/WhiteGoldLance.svg    \
           svg/BlackGoldPawn.svg    svg/WhiteGoldPawn.svg     \
           svg/BlackGoldSilver.svg  svg/WhiteGoldSilver.svg   \
           svg/BlackGold.svg	    svg/WhiteGold.svg	      \
           svg/BlackHawk.svg        svg/WhiteHawk.svg         \
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

shogidir = $(datadir)/games/xboard/themes/shogi
dist_shogi_DATA = \
           themes/shogi/WhiteGold.svg        themes/shogi/WhiteCrownedBishop.svg\
           themes/shogi/WhiteBishop.svg      themes/shogi/WhiteKing.svg\
           themes/shogi/WhiteKnight.svg      themes/shogi/WhiteGoldKnight.svg\
           themes/shogi/WhiteLance.svg       themes/shogi/WhiteGoldLance.svg\
           themes/shogi/WhitePawn.svg        themes/shogi/WhiteGoldPawn.svg\
           themes/shogi/WhiteRook.svg        themes/shogi/WhiteCrownedRook.svg\
           themes/shogi/WhiteAdvisor.svg     themes/shogi/WhiteGoldSilver.svg\
           themes/shogi/BlackGold.svg        themes/shogi/BlackCrownedBishop.svg\
           themes/shogi/BlackBishop.svg      themes/shogi/BlackKing.svg\
           themes/shogi/BlackKnight.svg      themes/shogi/BlackGoldKnight.svg\
           themes/shogi/BlackLance.svg       themes/shogi/BlackGoldLance.svg\
           themes/shogi/BlackPawn.svg        themes/shogi/BlackGoldPawn.svg\
           themes/shogi/BlackRook.svg        themes/shogi/BlackCrownedRook.svg\
           themes/shogi/BlackAdvisor.svg     themes/shogi/BlackGoldSilver.svg

xiangqidir = $(datadir)/games/xboard/themes/xiangqi
dist_xiangqi_DATA = themes/xiangqi/BlackAdvisor.svg  themes/xiangqi/WhiteAdvisor.svg \
           themes/xiangqi/BlackCanon.svg    themes/xiangqi/WhiteCanon.svg    \
           themes/xiangqi/BlackElephant.svg themes/xiangqi/WhiteElephant.svg \
           themes/xiangqi/BlackKnight.svg   themes/xiangqi/WhiteKnight.svg   \
           themes/xiangqi/BlackGold.svg	    themes/xiangqi/WhiteGold.svg     \
           themes/xiangqi/BlackPawn.svg	    themes/xiangqi/WhitePawn.svg     \
           themes/xiangqi/BlackRook.svg	    themes/xiangqi/WhiteRook.svg

### directory and files for pixmaps

pixmapsdir = $(datadir)/games/xboard/pixmaps/textures
dist_pixmaps_DATA = pixmaps/cross32.xpm pixmaps/cross48.xpm pixmaps/board32.xpm pixmaps/board48.xpm pixmaps/ini32.xpm pixmaps/ini48.xpm

### directory and files for pngs

pngdir = $(datadir)/games/xboard/themes/textures
dist_png_DATA = png/hatch.png png/wood_d.png png/wood_l.png png/xqboard.png

### directory and files for sound files

soundsdir = $(datadir)/games/xboard/sounds
dist_sounds_DATA = \
  sounds/cymbal.wav     sounds/pop2.wav   sounds/slap.wav     sounds/ding1.wav      sounds/laser.wav \
  sounds/woodthunk.wav  sounds/gong.wav   sounds/penalty.wav  sounds/honkhonk.wav   sounds/phone.wav


### set correct dir in xboard.conf

edit = sed -e 's|@pngdir[@]|$(pngdir)|g' \
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

### install mime-type files and corresponding icons and menu entries if enabled during configure

install-data-hook: install-mime-database
uninstall-hook: uninstall-mime-database

install-mime-database:
if ENABLE_UPDATE_MIMEDB
	-if test -z "$(DESTDIR)" -a -n "$(XDG_MIME)" -a -n "$(XDG_DESKTOP_MENU)" -a -n "$(XDG_ICON_RESOURCE)" ; then \
		$(XDG_MIME) install --mode system --novendor xboard.xml ;\
		$(XDG_DESKTOP_MENU) install --mode system --novendor xboard-pgn-viewer.desktop;\
		$(XDG_DESKTOP_MENU) install --mode system --novendor xboard-fen-viewer.desktop;\
		$(XDG_DESKTOP_MENU) install --mode system --novendor xboard-tourney.desktop;\
		$(XDG_DESKTOP_MENU) install --mode system --novendor xboard-config.desktop;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 pixmaps/board32.xpm application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 pixmaps/cross32.xpm application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 pixmaps/ini32.xpm application-x-xboard-opt;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 48 pixmaps/board48.xpm application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 48 pixmaps/cross48.xpm application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 48 pixmaps/ini48.xpm application-x-xboard-opt;\
	fi
endif

uninstall-mime-database:
if ENABLE_UPDATE_MIMEDB
	-if test -z "$(DESTDIR)" -a -n "$(XDG_MIME)" -a -n "$(XDG_DESKTOP_MENU)" -a -n "$(XDG_ICON_RESOURCE)" ; then \
		$(XDG_MIME) uninstall --mode system --novendor xboard.xml;\
		$(XDG_DESKTOP_MENU) uninstall --mode system --novendor xboard-pgn-viewer.desktop;\
		$(XDG_DESKTOP_MENU) uninstall --mode system --novendor xboard-fen-viewer.desktop;\
		$(XDG_DESKTOP_MENU) uninstall --mode system --novendor xboard-tourney.desktop;\
		$(XDG_DESKTOP_MENU) uninstall --mode system --novendor xboard-config.desktop;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 board32.xpm application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 cross32.xpm application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 ini32.xpm application-x-xboard-opt;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 48 board48.xpm application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 48 cross48.xpm application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 48 ini48.xpm application-x-xboard-opt;\
	fi
endif
