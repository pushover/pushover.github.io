AM_CXXFLAGS = -Wall -DTOP_DATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS = pushover

pushover_LDFLAGS = $(LIBINTL)
pushover_SOURCES =                  \
  src/ant.cpp src/ant.h \
  src/decompress.cpp src/decompress.h \
  src/graphics.cpp src/graphics.h \
  src/graphicsn.cpp src/graphicsn.h \
  src/graphicso.cpp src/graphicso.h \
  src/level.cpp src/level.h \
  src/levelset.cpp src/levelset.h \
  src/luaclass.cpp src/luaclass.h \
  src/main.cpp \
  src/pngloader.cpp src/pngloader.h \
  src/recorder.cpp src/recorder.h \
  src/soundsys.cpp src/soundsys.h \
  src/textsections.cpp src/textsections.h

dist_pkgdata_DATA = \
  data/ant.png \
  data/dominos.png \
  data/carried.png \
  data/01_StandardFalling.wav \
  data/02_StopperHit.wav \
  data/03_Splitter.wav \
  data/04_Exploder.wav \
  data/05_Delay.wav \
  data/06_TumblerFalling.wav \
  data/07_BridgerFalling.wav \
  data/09_TriggerFalling.wav \
  data/0A_Ascender.wav \
  data/0B_Falling.wav \
  data/0C_Landing.wav \
  data/0D_PickUpDomino.wav \
  data/0E_Tapping.wav \
  data/0F_Schrugging.wav \
  data/10_DoorClose.wav \
  data/11_DoorOpen.wav \
  data/13_Victory.wav \
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

