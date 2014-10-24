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
xboard_LDADD = -ldl -lm @FRONTEND_LIBS@ @X_LIBS@ @LIBINTL@ @CAIRO_LIBS@

EXTRA_DIST = pixmaps themes png sounds winboard \
	xboard.texi gpl.texinfo texi2man texinfo.tex xboard.man xboard.desktop xboard-config.desktop \
	xboard-fen-viewer.desktop  xboard-pgn-viewer.desktop  xboard-tourney.desktop xboard.xml \
	COPYRIGHT FAQ.html engine-intf.html ics-parsing.txt readme.htm zippy.README \
	xboard.conf SHORTLOG DIFFSTAT history.c ABOUT-NLS $(FRONTENDextras)

DISTCLEANFILES = stamp-h

GITVERSION=$(shell sh -c 'git describe --dirty --always 2>/dev/null')
AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @X_CFLAGS@ @CAIRO_CFLAGS@ @FRONTEND_CFLAGS@  -DSYSCONFDIR='"$(sysconfdir)"' \
	    -DLOCALEDIR='"$(localedir)"' -DSVGDIR='"$(svgdir)"' -D__GIT_VERSION='"$(GITVERSION)"' \
            -DCONFIGURE_OPTIONS='"@CONFIGURE_OPTIONS@"' -DDATADIR='"$(datadir)/games/xboard"' $(headers)

ACLOCAL_AMFLAGS = -I m4

info_TEXINFOS =  xboard.texi
xboard_TEXINFOS =  copyright.texi
man6_MANS = xboard.man


### config file

sysconf_DATA = xboard.conf

### icon files

icondir = @ICONSDIR@
dist_icon_DATA = xboard.png
svgicondir = @SVGICONSDIR@
dist_svgicon_DATA = xboard.svg


### desktop files for KDE and gnome

Applicationsdir = @DESKTOPDIR@
Applications_DATA = xboard.desktop xboard-fen-viewer.desktop  xboard-pgn-viewer.desktop  xboard-tourney.desktop  xboard-config.desktop

### mime file
mimedir = @MIMEDIR@
mime_DATA = xboard.xml

gamedatadir = @GAMEDATADIR@

### directory and files for svgs

svgdir = $(gamedatadir)/themes/default
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
           svg/BlackSword.svg	    svg/WhiteSword.svg        \
           svg/BlackHSword.svg	    svg/WhiteHSword.svg       \
           svg/BlackLeopard.svg	    svg/WhiteLeopard.svg      \
           svg/BlackLion.svg	    svg/WhiteLion.svg         \
           svg/BlackPromoBishop.svg svg/WhitePromoBishop.svg  \
           svg/BlackPromoRook.svg   svg/WhitePromoRook.svg    \
           svg/BlackPromoHorse.svg  svg/WhitePromoHorse.svg   \
           svg/BlackPromoDragon.svg svg/WhitePromoDragon.svg  \
           svg/BlackPromoSword.svg  svg/WhitePromoSword.svg   \
           svg/BlackPromoHSword.svg svg/WhitePromoHSword.svg  \
           svg/BlackHCrown.svg	    svg/WhiteHCrown.svg       \
           svg/BlackDolphin.svg	    svg/WhiteDolphin.svg      \
           svg/BlackClaw.svg	    svg/WhiteClaw.svg         \
           svg/eo_Analyzing.svg     svg/eo_Black.svg          \
           svg/eo_Clear.svg         svg/eo_Ponder.svg         \
           svg/eo_Thinking.svg      svg/eo_Unknown.svg        \
           svg/eo_White.svg

shogidir = $(gamedatadir)/themes/shogi
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

xiangqidir = $(gamedatadir)/themes/xiangqi
dist_xiangqi_DATA = themes/xiangqi/BlackAdvisor.svg  themes/xiangqi/WhiteAdvisor.svg \
           themes/xiangqi/BlackCanon.svg    themes/xiangqi/WhiteCanon.svg    \
           themes/xiangqi/BlackElephant.svg themes/xiangqi/WhiteElephant.svg \
           themes/xiangqi/BlackKnight.svg   themes/xiangqi/WhiteKnight.svg   \
           themes/xiangqi/BlackGold.svg	    themes/xiangqi/WhiteGold.svg     \
           themes/xiangqi/BlackPawn.svg	    themes/xiangqi/WhitePawn.svg     \
           themes/xiangqi/BlackRook.svg	    themes/xiangqi/WhiteRook.svg

### directory and files for themes (and other) configuration files

themesdir = $(datadir)/games/xboard/themes/conf
dist_themes_DATA = conf/shogi conf/xq conf/chu conf/ics conf/mini conf/mini.fen conf/sho conf/sho.fen

### directory and files for pixmaps

pixmapsdir = $(gamedatadir)/pixmaps/textures
dist_pixmaps_DATA = pixmaps/cross32.png pixmaps/cross48.png pixmaps/board32.png pixmaps/board48.png pixmaps/ini32.png pixmaps/ini48.png

### directory and files for pngs

pngdir = $(gamedatadir)/themes/textures
dist_png_DATA = png/hatch.png png/wood_d.png png/wood_l.png png/xqboard.png

### directory and files for sound files

soundsdir = $(gamedatadir)/sounds
dist_sounds_DATA = \
  sounds/cymbal.wav     sounds/pop2.wav   sounds/slap.wav     sounds/ding1.wav      sounds/laser.wav \
  sounds/woodthunk.wav  sounds/gong.wav   sounds/penalty.wav  sounds/honkhonk.wav   sounds/phone.wav  sounds/roar.wav

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
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 pixmaps/board32.png application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 pixmaps/cross32.png application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 32 pixmaps/ini32.png application-x-xboard-opt;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 48 pixmaps/board48.png application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 48 pixmaps/cross48.png application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) install --context mimetypes --size 48 pixmaps/ini48.png application-x-xboard-opt;\
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
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 board32.png application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 cross32.png application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 32 ini32.png application-x-xboard-opt;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 48 board48.png application-x-chess-pgn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 48 cross48.png application-x-xboard-trn;\
		$(XDG_ICON_RESOURCE) uninstall --context mimetypes --size 48 ini48.png application-x-xboard-opt;\
	fi
endif

if OSX_APP
osxapp: all-recursive install install-recursive
	$(MKDIR_P) $(datarootdir)/bin
	$(MKDIR_P) $(datarootdir)/lib
	$(MKDIR_P) $(sysconfdir)/gtk-2.0
	$(MKDIR_P) $(sysconfdir)/pango
	$(MKDIR_P) $(datadir)/themes
	$(MKDIR_P) $(gamedatadir)/themes
	$(INSTALL) ./osxapp/Info.plist $(prefix)
	$(INSTALL) ./osxapp/PkgInfo $(prefix)
	$(INSTALL) ./osxapp/XBoard.gtklauncher $(bindir)/XBoard
	$(INSTALL) ./xboard $(bindir)/xboard-bin
	cp -R ./osxapp/icons/. $(datarootdir)
	$(INSTALL) ./osxapp/man.command    $(datarootdir)/bin/man.command
	$(INSTALL) ./osxapp/info.command   $(datarootdir)/bin/info.command
	sed -e 's?-soundProgram "aplay -q"?-soundProgram "afplay"?'\
            -e 's?-firstChessProgram fairymax?-firstChessProgram "./fairymax"?'\
	    -e 's?-firstChessProgramNames {fairymax?-firstChessProgramNames {"fairymax" -fcp "./fairymax" -fd "~~/../../bin/fairymax"?'\
	    -e 's?"Fruit 2.1" -fcp fruit -fUCI?"fruit" -fcp "./fruit" -fd "~~/../../bin/fruit" -fUCI?'\
	    -e '/"Crafty" -fcp crafty/d'\
	    -e '/"GNU Chess" -fcp gnuchess/d'\
	    -e 's?-polyglotDir ""?-polyglotDir "~~/../../bin"?'\
	    -e 's?polyglot ?./polyglot ?'\
	    -e 's?-icsNames {"fics" -icshost freechess.org -icshelper timeseal?-icsNames {"fics" -icshost 69.36.243.188 -icshelper ~~/../../bin/timeseal?'\
	    -e 's?"icc" -icshost chessclub.com -icshelper timestamp?"icc" -icshost 207.99.83.228 -icshelper ~~/../../bin/timestamp?'\
	    -e 's?"kc" -icshost alanimus.com -icshelper timeseal?"vics" -icshost winboard.nl -icshelper ~~/../../bin/timeseal?'\
	    -e '/; Save user settings./d'\
	    -e '/; Must be last in file to make user options prevail over system-wide settings!/d'\
	    -e '/-saveSettingsOnExit true/d'\
	    -e '/-saveSettingsFile "~\/.xboardrc"/d'\
	    -e '/-settingsFile "~\/.xboardrc"/d'<./xboard.conf >$(sysconfdir)/xboard.conf
	echo '; OSX' >>$(sysconfdir)/xboard.conf
	echo ';'>>$(sysconfdir)/xboard.conf
	echo '-clockFont "Sans Bold 26" '>>$(sysconfdir)/xboard.conf
	echo '-pieceImageDirectory "~~/themes/default"' >>$(sysconfdir)/xboard.conf
	echo '-autoLogo true' >>$(sysconfdir)/xboard.conf
	echo '-logoDir "~~/logos"' >>$(sysconfdir)/xboard.conf
	echo '-logoSize 100' >>$(sysconfdir)/xboard.conf
	echo '-openCommand "cd %s/../../bin; open"' >>$(sysconfdir)/xboard.conf
	echo '-firstDirectory "~~/../../bin/fairymax"' >>$(sysconfdir)/xboard.conf
	echo '-secondDirectory "~~/../../bin/fairymax"' >>$(sysconfdir)/xboard.conf
	echo '-secondChessProgram "./fairymax"' >>$(sysconfdir)/xboard.conf
	echo ';' >>$(sysconfdir)/xboard.conf
	echo '; Save user settings.' >>$(sysconfdir)/xboard.conf
	echo '; Must be last in file to make user options prevail over system-wide settings!' >>$(sysconfdir)/xboard.conf
	echo ';' >>$(sysconfdir)/xboard.conf
	echo '-saveSettingsOnExit true' >>$(sysconfdir)/xboard.conf
	echo '-saveSettingsFile "~/Library/Preferences/XBoardOSX.conf"' >>$(sysconfdir)/xboard.conf
	echo '-settingsFile "~/Library/Preferences/XBoardOSX.conf"' >>$(sysconfdir)/xboard.conf

	sed 's?/opt/local/lib?/tmp/skl/XBoard.app/Contents/Resources/lib?' </opt/local/etc/gtk-2.0/gdk-pixbuf.loaders >$(sysconfdir)/gtk-2.0/gdk-pixbuf.loaders
	sed -e 's?/opt/local/lib?/tmp/skl/XBoard.app/Contents/Resources/lib?' -e 's?/opt/local/share/locale?/tmp/skl/XBoard.app/Contents/Resources/share/locale?' </opt/local/etc/gtk-2.0/gtk.immodules >$(sysconfdir)/gtk-2.0/gtk.immodules
	$(INSTALL) ./osxapp/launcher_rc $(datarootdir)/
	$(INSTALL) /opt/local/etc/gtk-2.0/im-multipress.conf $(sysconfdir)/gtk-2.0
	$(INSTALL) ./osxapp/gtkrc $(sysconfdir)/gtk-2.0/gtkrc
	$(INSTALL) ./osxapp/pango.modules $(sysconfdir)/pango/pangorc
	cp -R /opt/local/lib/gtk-2.0 $(datarootdir)/lib
	cp -R /opt/local/lib/gdk-pixbuf-2.0 $(datarootdir)/lib
	cp -R /opt/local/share/themes/Mac $(datadir)/themes
	cp -R ./osxapp/default/. $(gamedatadir)/themes/default
	$(INSTALL) ./png/marble_d.png $(gamedatadir)/themes/textures
	$(INSTALL) ./png/marble_l.png $(gamedatadir)/themes/textures
	$(INSTALL) ./png/xqwood.png $(gamedatadir)/themes/textures
	cp -R ./osxapp/logos $(gamedatadir)/
	cp -R ./osxapp/osx-localization/. $(datarootdir)/
	for i in po/*gmo; do lang=`basename $$i .gmo`; $(INSTALL) /opt/local/share/locale/$$lang/LC_MESSAGES/gtk20.mo $(localedir)/$$lang/LC_MESSAGES/; done
	dylibbundler -of -b -x $(bindir)/xboard-bin -d $(datarootdir)/lib -p @executable_path/../Resources/lib
	dylibbundler -of -b -x $(datarootdir)/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-png.so -d $(datarootdir)/lib -p @executable_path/../Resources/lib
	dylibbundler -of -b -x $(datarootdir)/lib/gtk-2.0/2.10.0/immodules/im-quartz.so -d $(datarootdir)/lib -p @executable_path/../Resources/lib
	dylibbundler -of -b -x $(datarootdir)/lib/gtk-2.0/2.10.0/engines/libpixmap.so -d $(datarootdir)/lib -p @executable_path/../Resources/lib
	find ./XBoard.app -name '*.a' -print0 | xargs -0 rm
	rm -r $(datarootdir)/lib/gtk-2.0/include
	rm -r $(datarootdir)/share/applications
	rm $(datarootdir)/share/info/dir
	rm -r $(datarootdir)/share/icons
	rm -r $(datarootdir)/share/mime
	mv $(datarootdir)/share/games/xboard/themes/conf $(datarootdir)/share/xboard/conf
	rm -r $(datarootdir)/share/xboard/pixmaps
	rm -r $(datarootdir)/share/games
endif

