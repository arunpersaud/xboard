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
	  	 gamelist.c ngamelist.c \
	 	 gettext.h  \
		 lists.c lists.h \
		 moves.c moves.h \
		 parser.c parser.h \
		 pgntags.c \
	  	 uci.c \
	 	 xboard.c xboard.h args.h menus.c menus.h \
	 	 draw.c draw.h \
	 	 usounds.c usystem.c usystem.h \
	 	 board.c board.h \
		 xedittags.c xedittags.h \
		 engineoutput.c engineoutput.h \
		 nengineoutput.c xengineoutput.c \
		 evalgraph.c evalgraph.h xevalgraph.c xevalgraph.h\
	 	 xgamelist.c xgamelist.h\
		 history.c xhistory.c xhistory.h \
		 xoptions.c dialogs.c dialogs.h \
		 $(ZPY)
SUBDIRS = po
xboard_LDADD = -lm @XAW_LIBS@ @X_LIBS@ @LIBINTL@ @CAIRO_LIBS@
EXTRA_DIST = pixmaps bitmaps sounds winboard \
	xboard.texi gpl.texinfo texi2man texinfo.tex xboard.man xboard.desktop \
	xboard-fen-viewer.desktop  xboard-pgn-viewer.desktop  xboard-tourney.desktop xboard.xml \
	COPYRIGHT FAQ.html engine-intf.html ics-parsing.txt readme.htm zippy.README \
	xboard.conf.in SHORTLOG DIFFSTAT history.c ABOUT-NLS

DISTCLEANFILES = stamp-h

AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @CAIRO_CFLAGS@ @X_CFLAGS@  -DSYSCONFDIR='"$(sysconfdir)"' \
	    -DBITMAPDIR='"$(datadir)/games/xboard/bitmaps/default"' \
	    -DLOCALEDIR='"$(localedir)"'

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
Applications_DATA = xboard.desktop xboard-fen-viewer.desktop  xboard-pgn-viewer.desktop  xboard-tourney.desktop

### mime file
mimedir = $(datadir)/mime/packages
mime_DATA = xboard.xml

### directory and files for bitmaps

bitmapsdir = $(datadir)/games/xboard/bitmaps/default
dist_bitmaps_DATA = \
  bitmaps/a33o.bm    bitmaps/c45s.bm        bitmaps/icon_outline.bm  bitmaps/m49o.bm   bitmaps/p45o.bm   bitmaps/r29o.bm \
  bitmaps/a33s.bm    bitmaps/c49o.bm        bitmaps/icon_white.bm    bitmaps/m49s.bm   bitmaps/p45s.bm   bitmaps/r29s.bm \
  bitmaps/a37o.bm    bitmaps/c49s.bm        bitmaps/j33o.bm          bitmaps/m72o.bm   bitmaps/p49o.bm   bitmaps/r33o.bm \
  bitmaps/a37s.bm    bitmaps/c54o.bm        bitmaps/j33s.bm          bitmaps/m72s.bm   bitmaps/p49s.bm   bitmaps/r33s.bm \
  bitmaps/a40o.bm    bitmaps/c54s.bm        bitmaps/j49o.bm          bitmaps/n108o.bm  bitmaps/p54o.bm   bitmaps/r37o.bm \
  bitmaps/a40s.bm    bitmaps/c58o.bm        bitmaps/j49s.bm          bitmaps/n108s.bm  bitmaps/p54s.bm   bitmaps/r37s.bm \
  bitmaps/a45o.bm    bitmaps/c58s.bm        bitmaps/j72o.bm          bitmaps/n116o.bm  bitmaps/p58o.bm   bitmaps/r40o.bm \
  bitmaps/a45s.bm    bitmaps/c64o.bm        bitmaps/j72s.bm          bitmaps/n116s.bm  bitmaps/p58s.bm   bitmaps/r40s.bm \
  bitmaps/a49o.bm    bitmaps/c64s.bm        bitmaps/k108o.bm         bitmaps/n129o.bm  bitmaps/p64o.bm   bitmaps/r45o.bm \
  bitmaps/a49s.bm    bitmaps/c72o.bm        bitmaps/k108s.bm         bitmaps/n129s.bm  bitmaps/p64s.bm   bitmaps/r45s.bm \
  bitmaps/a54o.bm    bitmaps/c72s.bm        bitmaps/k116o.bm         bitmaps/n21o.bm   bitmaps/p72o.bm   bitmaps/r49o.bm \
  bitmaps/a54s.bm    bitmaps/checkmark.bm   bitmaps/k116s.bm         bitmaps/n21s.bm   bitmaps/p72s.bm   bitmaps/r49s.bm \
  bitmaps/a58o.bm    bitmaps/d33o.bm        bitmaps/k129o.bm         bitmaps/n25o.bm   bitmaps/p80o.bm   bitmaps/r54o.bm \
  bitmaps/a58s.bm    bitmaps/d33s.bm        bitmaps/k129s.bm         bitmaps/n25s.bm   bitmaps/p80s.bm   bitmaps/r54s.bm \
  bitmaps/a64o.bm    bitmaps/d37o.bm        bitmaps/k21o.bm          bitmaps/n29o.bm   bitmaps/p87o.bm   bitmaps/r58o.bm \
  bitmaps/a64s.bm    bitmaps/d37s.bm        bitmaps/k21s.bm          bitmaps/n29s.bm   bitmaps/p87s.bm   bitmaps/r58s.bm \
  bitmaps/a72o.bm    bitmaps/d40o.bm        bitmaps/k25o.bm          bitmaps/n33o.bm   bitmaps/p95o.bm   bitmaps/r64o.bm \
  bitmaps/a72s.bm    bitmaps/d40s.bm        bitmaps/k25s.bm          bitmaps/n33s.bm   bitmaps/p95s.bm   bitmaps/r64s.bm \
  bitmaps/as45o.bm   bitmaps/d45o.bm        bitmaps/k29o.bm          bitmaps/n37o.bm   bitmaps/q108o.bm  bitmaps/r72o.bm \
  bitmaps/as45s.bm   bitmaps/d45s.bm        bitmaps/k29s.bm          bitmaps/n37s.bm   bitmaps/q108s.bm  bitmaps/r72s.bm \
  bitmaps/b108o.bm   bitmaps/d49o.bm        bitmaps/k33o.bm          bitmaps/n40o.bm   bitmaps/q116o.bm  bitmaps/r80o.bm \
  bitmaps/b108s.bm   bitmaps/d49s.bm        bitmaps/k33s.bm          bitmaps/n40s.bm   bitmaps/q116s.bm  bitmaps/r80s.bm \
  bitmaps/b116o.bm   bitmaps/d54o.bm        bitmaps/k37o.bm          bitmaps/n45o.bm   bitmaps/q129o.bm  bitmaps/r87o.bm \
  bitmaps/b116s.bm   bitmaps/d54s.bm        bitmaps/k37s.bm          bitmaps/n45s.bm   bitmaps/q129s.bm  bitmaps/r87s.bm \
  bitmaps/b129o.bm   bitmaps/d58o.bm        bitmaps/k40o.bm          bitmaps/n49o.bm   bitmaps/q21o.bm   bitmaps/r95o.bm \
  bitmaps/b129s.bm   bitmaps/d58s.bm        bitmaps/k40s.bm          bitmaps/n49s.bm   bitmaps/q21s.bm   bitmaps/r95s.bm \
  bitmaps/b21o.bm    bitmaps/d64o.bm        bitmaps/k45o.bm          bitmaps/n54o.bm   bitmaps/q25o.bm \
  bitmaps/b21s.bm    bitmaps/d64s.bm        bitmaps/k45s.bm          bitmaps/n54s.bm   bitmaps/q25s.bm   bitmaps/s49o.bm \
  bitmaps/b25o.bm    bitmaps/d72o.bm        bitmaps/k49o.bm          bitmaps/n58o.bm   bitmaps/q29o.bm   bitmaps/s49s.bm \
  bitmaps/b25s.bm    bitmaps/d72s.bm        bitmaps/k49s.bm          bitmaps/n58s.bm   bitmaps/q29s.bm   bitmaps/s72o.bm \
  bitmaps/b29o.bm    bitmaps/k54o.bm        bitmaps/n64o.bm          bitmaps/q33o.bm   bitmaps/s72s.bm \
  bitmaps/b29s.bm    bitmaps/k54s.bm        bitmaps/n64s.bm          bitmaps/q33s.bm \
  bitmaps/b33o.bm    bitmaps/e33o.bm        bitmaps/k58o.bm          bitmaps/n72o.bm   bitmaps/q37o.bm   bitmaps/u49o.bm \
  bitmaps/b33s.bm    bitmaps/e33s.bm        bitmaps/k58s.bm          bitmaps/n72s.bm   bitmaps/q37s.bm   bitmaps/u49s.bm \
  bitmaps/b37o.bm    bitmaps/e49o.bm        bitmaps/k64o.bm          bitmaps/n80o.bm   bitmaps/q40o.bm   bitmaps/u72o.bm \
  bitmaps/b37s.bm    bitmaps/e49s.bm        bitmaps/k64s.bm          bitmaps/n80s.bm   bitmaps/q40s.bm   bitmaps/u72s.bm \
  bitmaps/b40o.bm    bitmaps/e72o.bm        bitmaps/k72o.bm          bitmaps/n87o.bm   bitmaps/q45o.bm   bitmaps/v33o.bm \
  bitmaps/b40s.bm    bitmaps/e72s.bm        bitmaps/k72s.bm          bitmaps/n87s.bm   bitmaps/q45s.bm   bitmaps/v33s.bm \
  bitmaps/b45o.bm    bitmaps/f33o.bm        bitmaps/k80o.bm          bitmaps/n95o.bm   bitmaps/q49o.bm   bitmaps/v49o.bm \
  bitmaps/b45s.bm    bitmaps/f33s.bm        bitmaps/k80s.bm          bitmaps/n95s.bm   bitmaps/q49s.bm   bitmaps/v49s.bm \
  bitmaps/b49o.bm    bitmaps/f49o.bm        bitmaps/k87o.bm          bitmaps/o33o.bm   bitmaps/q54o.bm   bitmaps/v72o.bm \
  bitmaps/b49s.bm    bitmaps/f49s.bm        bitmaps/k87s.bm          bitmaps/o33s.bm   bitmaps/q54s.bm   bitmaps/v72s.bm \
  bitmaps/b54o.bm    bitmaps/f72o.bm        bitmaps/k95o.bm          bitmaps/o49o.bm   bitmaps/q58o.bm   bitmaps/w33o.bm \
  bitmaps/b54s.bm    bitmaps/f72s.bm        bitmaps/k95s.bm          bitmaps/o49s.bm   bitmaps/q58s.bm   bitmaps/w33s.bm \
  bitmaps/b58o.bm    bitmaps/g33o.bm        bitmaps/l33o.bm          bitmaps/o72o.bm   bitmaps/q64o.bm   bitmaps/w49o.bm \
  bitmaps/b58s.bm    bitmaps/g33s.bm        bitmaps/l33s.bm          bitmaps/o72s.bm   bitmaps/q64s.bm   bitmaps/w49s.bm \
  bitmaps/b64o.bm    bitmaps/g49o.bm        bitmaps/l37o.bm          bitmaps/p108o.bm  bitmaps/q72o.bm   bitmaps/w72o.bm \
  bitmaps/b64s.bm    bitmaps/g49s.bm        bitmaps/l37s.bm          bitmaps/p108s.bm  bitmaps/q72s.bm   bitmaps/w72s.bm \
  bitmaps/b72o.bm    bitmaps/g72o.bm        bitmaps/l40o.bm          bitmaps/p116o.bm  bitmaps/q80o.bm   bitmaps/wl49o.bm \
  bitmaps/b72s.bm    bitmaps/g72s.bm        bitmaps/l40s.bm          bitmaps/p116s.bm  bitmaps/q80s.bm   bitmaps/wl49s.bm \
  bitmaps/b80o.bm    bitmaps/h33o.bm        bitmaps/l45o.bm          bitmaps/p129o.bm  bitmaps/q87o.bm   bitmaps/wl72o.bm \
  bitmaps/b80s.bm    bitmaps/h33s.bm        bitmaps/l45s.bm          bitmaps/p129s.bm  bitmaps/q87s.bm   bitmaps/wl72s.bm \
  bitmaps/b87o.bm    bitmaps/h49o.bm        bitmaps/l49o.bm          bitmaps/p21o.bm   bitmaps/q95o.bm   bitmaps/wn49o.bm \
  bitmaps/b87s.bm    bitmaps/h49s.bm        bitmaps/l49s.bm          bitmaps/p21s.bm   bitmaps/q95s.bm   bitmaps/wn49s.bm \
  bitmaps/b95o.bm    bitmaps/h72o.bm        bitmaps/l54o.bm          bitmaps/p25o.bm   bitmaps/r108o.bm  bitmaps/wn72o.bm \
  bitmaps/b95s.bm    bitmaps/h72s.bm        bitmaps/l54s.bm          bitmaps/p25s.bm   bitmaps/r108s.bm  bitmaps/wn72s.bm \
  bitmaps/i33o.bm    bitmaps/l58o.bm        bitmaps/p29o.bm          bitmaps/r116o.bm  bitmaps/wp49o.bm \
  bitmaps/c33o.bm    bitmaps/i33s.bm        bitmaps/l58s.bm          bitmaps/p29s.bm   bitmaps/r116s.bm  bitmaps/wp49s.bm \
  bitmaps/c33s.bm    bitmaps/i49o.bm        bitmaps/l64o.bm          bitmaps/p33o.bm   bitmaps/r129o.bm  bitmaps/wp72o.bm \
  bitmaps/c37o.bm    bitmaps/i49s.bm        bitmaps/l64s.bm          bitmaps/p33s.bm   bitmaps/r129s.bm  bitmaps/wp72s.bm \
  bitmaps/c37s.bm    bitmaps/i72o.bm        bitmaps/l72o.bm          bitmaps/p37o.bm   bitmaps/r21o.bm   bitmaps/ws49o.bm \
  bitmaps/c40o.bm    bitmaps/i72s.bm        bitmaps/l72s.bm          bitmaps/p37s.bm   bitmaps/r21s.bm   bitmaps/ws49s.bm \
  bitmaps/c40s.bm    bitmaps/m33o.bm        bitmaps/p40o.bm          bitmaps/r25o.bm   bitmaps/ws72o.bm \
  bitmaps/c45o.bm    bitmaps/icon_black.bm  bitmaps/m33s.bm          bitmaps/p40s.bm   bitmaps/r25s.bm   bitmaps/ws72s.bm

### directory and files for svgs

svgdir = $(datadir)/games/xboard/svg
dist_svg_DATA = svg/icon_white.svg         svg/icon_black.svg      \
           svg/DarkSquare.svg         svg/LightSquare.svg svg/NeutralSquare.svg \
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
           svg/eo_White.svg                                   \
           svg/shogi_bishop.svg     svg/shogi_bishop_promoted.svg \
           svg/shogi_gold.svg\
           svg/shogi_king.svg\
           svg/shogi_knight.svg     svg/shogi_knight_promoted.svg\
           svg/shogi_lance.svg      svg/shogi_lance_promoted.svg\
           svg/shogi_pawn.svg       svg/shogi_pawn_promoted.svg\
           svg/shogi_rook.svg       svg/shogi_rook_promoted.svg\
           svg/shogi_silver.svg     svg/shogi_silver_promoted.svg

### directory and files for pixmaps

pixmapsdir = $(datadir)/games/xboard/pixmaps/textures
dist_pixmaps_DATA = pixmaps/cross32.xpm pixmaps/cross48.xpm pixmaps/board32.xpm pixmaps/board48.xpm

### directory and files for pngs

pngdir = $(datadir)/games/xboard/themes/textures
dist_png_DATA = png/hatch.png png/wood_d.png png/wood_l.png

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
