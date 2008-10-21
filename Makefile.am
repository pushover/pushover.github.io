bin_PROGRAMS = pushover

pushover_LDFLAGS = $(LIBINTL) $(LUA_LIBS)
pushover_CXXFLAGS = -Wall -DPKGDATADIR=\"$(pkgdatadir)\" $(LUA_CFLAGS)
pushover_SOURCES = \
  src/ant.cpp src/ant.h \
  src/graphics.cpp src/graphics.h \
  src/graphicsn.cpp src/graphicsn.h \
  src/level.cpp src/level.h \
  src/levelset.cpp src/levelset.h \
  src/luaclass.cpp src/luaclass.h \
  src/main.cpp \
  src/pngloader.cpp src/pngloader.h \
  src/recorder.cpp src/recorder.h \
  src/screen.cpp src/screen.h \
  src/sha1.cpp src/sha1.h \
  src/soundsys.cpp src/soundsys.h \
  src/text.cpp src/text.h \
  src/textsections.cpp src/textsections.h \
  src/window.cpp src/window.h

nobase_dist_pkgdata_DATA = \
  data/ant.png \
  data/dominos.png \
  data/carried.png \
  data/box.png \
  data/01_StandardFalling.ogg \
  data/02_StopperHit.ogg \
  data/03_Splitter.ogg \
  data/04_Exploder.ogg \
  data/05_Delay.ogg \
  data/06_TumblerFalling.ogg \
  data/07_BridgerFalling.ogg \
  data/09_TriggerFalling.ogg \
  data/0A_Ascender.ogg \
  data/0B_Falling.ogg \
  data/0C_Landing.ogg \
  data/0D_PickUpDomino.ogg \
  data/0E_Tapping.ogg \
  data/0F_Schrugging.ogg \
  data/10_DoorClose.ogg \
  data/11_DoorOpen.ogg \
  data/13_Victory.ogg \
  \
  data/FreeSans.ttf\
  \
  themes/tools.lua \
  themes/toxcity.lua  themes/toxcity.png \
  themes/aztec.lua    themes/aztec.png \
  themes/space.lua    themes/space.png \
  themes/electro.lua  themes/electro.png \
  themes/greek.lua    themes/greek.png \
  themes/castle.lua   themes/castle.png \
  themes/mechanic.lua themes/mechanic.png \
  themes/dungeon.lua  themes/dungeon.png \
  themes/japanese.lua themes/japanese.png \
  themes/cavern.lua   themes/cavern.png

dist_noinst_DATA = \
  levels/original/index.level \
  levels/original/001.level levels/original/002.level \
  levels/original/003.level levels/original/004.level \
  levels/original/005.level levels/original/006.level \
  levels/original/007.level levels/original/008.level \
  levels/original/009.level levels/original/010.level \
  levels/original/011.level levels/original/012.level \
  levels/original/013.level levels/original/014.level \
  levels/original/015.level levels/original/016.level \
  levels/original/017.level levels/original/018.level \
  levels/original/019.level levels/original/020.level \
  levels/original/021.level levels/original/022.level \
  levels/original/023.level levels/original/024.level \
  levels/original/025.level levels/original/026.level \
  levels/original/027.level levels/original/028.level \
  levels/original/029.level levels/original/030.level \
  levels/original/031.level levels/original/032.level \
  levels/original/033.level levels/original/034.level \
  levels/original/035.level levels/original/036.level \
  levels/original/037.level levels/original/038.level \
  levels/original/039.level levels/original/040.level \
  levels/original/041.level levels/original/042.level \
  levels/original/043.level levels/original/044.level \
  levels/original/045.level levels/original/046.level \
  levels/original/047.level levels/original/048.level \
  levels/original/049.level levels/original/050.level \
  levels/original/051.level levels/original/052.level \
  levels/original/053.level levels/original/054.level \
  levels/original/055.level levels/original/056.level \
  levels/original/057.level levels/original/058.level \
  levels/original/059.level levels/original/060.level \
  levels/original/061.level levels/original/062.level \
  levels/original/063.level levels/original/064.level \
  levels/original/065.level levels/original/066.level \
  levels/original/067.level levels/original/068.level \
  levels/original/069.level levels/original/070.level \
  levels/original/071.level levels/original/072.level \
  levels/original/073.level levels/original/074.level \
  levels/original/075.level levels/original/076.level \
  levels/original/077.level levels/original/078.level \
  levels/original/079.level levels/original/080.level \
  levels/original/081.level levels/original/082.level \
  levels/original/083.level levels/original/084.level \
  levels/original/085.level levels/original/086.level \
  levels/original/087.level levels/original/088.level \
  levels/original/089.level levels/original/090.level \
  levels/original/091.level levels/original/092.level \
  levels/original/093.level levels/original/094.level \
  levels/original/095.level levels/original/096.level \
  levels/original/097.level levels/original/098.level \
  levels/original/099.level levels/original/100.level \
  levels/original/101.level \
  levels/original/091_snes.level

install-data-local:
	$(mkdir_p) "$(DESTDIR)$(pkgdatadir)/levels"
	for p in $(patsubst levels/%/index.level,%,$(filter levels/%/index.level,$(dist_noinst_DATA))); do \
	  cat $(srcdir)/levels/$$p/* | gzip -c -9 > "$(DESTDIR)$(pkgdatadir)/levels/$$p.gz"; \
	  chmod 644 "$(DESTDIR)$(pkgdatadir)/levels/$$p.gz"; \
	done
