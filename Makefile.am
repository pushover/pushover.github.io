SUBDIRS = po
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = pushover

pushover_CXXFLAGS = -Wall -DPKGDATADIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\"
pushover_SOURCES = \
  src/ant.cpp src/ant.h \
  src/graphics.cpp src/graphics.h \
  src/graphicsn.cpp src/graphicsn.h \
  src/leveldata.cpp src/leveldata.h \
  src/leveldisplay.cpp src/leveldisplay.h \
  src/levelplayer.cpp src/levelplayer.h \
  src/levelset.cpp src/levelset.h \
  src/luaclass.cpp src/luaclass.h \
  src/main.cpp \
  src/pngloader.cpp src/pngloader.h \
  src/recorder.cpp src/recorder.h \
  src/screen.cpp src/screen.h \
  src/sha1.cpp src/sha1.h \
  src/solvedmap.cpp src/solvedmap.h \
  src/soundsys.cpp src/soundsys.h \
  src/textsections.cpp src/textsections.h \
  src/tools.cpp src/tools.h \
  src/window.cpp src/window.h

dist_doc_DATA = \
  README \
  AUTHORS \
  COPYING \
  NEWS \
  ChangeLog

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
  data/FreeSans.ttf \
  \
  themes/tools.lua \
  themes/toxcity.lua  themes/toxcity.png  themes/toxcity.ogg \
  themes/aztec.lua    themes/aztec.png    themes/aztec.ogg \
  themes/space.lua    themes/space.png    themes/space.ogg \
  themes/electro.lua  themes/electro.png \
  themes/greek.lua    themes/greek.png    themes/greek.ogg \
  themes/castle.lua   themes/castle.png   themes/castle.ogg \
  themes/mechanic.lua themes/mechanic.png themes/mechanic.ogg\
  themes/dungeon.lua  themes/dungeon.png  themes/dungeon.ogg \
  themes/japanese.lua themes/japanese.png themes/japanese.ogg \
  themes/cavern.lua   themes/cavern.png \
  \
  pushover.ico

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
  levels/original/091_snes.level \
  levels/av/index.level \
  levels/av/too_easy.level \
  levels/av/still_too_easy.level \
  levels/av/run.level \
  levels/av/run_n_bridge.level \
  levels/av/wrong_direction.level \
  levels/av/one_too_much.level \
  levels/av/missing_stopper.level \
  levels/av/missing_stopper_2.level \
  levels/av/easy_distance.level \
  levels/av/hard_distance.level \
  levels/av/crash.level \
  levels/av/crashed_again.level \
  levels/av/merger.level \
  levels/av/one_last_crash.level \
  levels/av/out_of_reach.level \
  levels/av/missing_quantum.level \
  levels/av/quantum_theory.level \
  levels/av/quantum_theory_2.level \
  levels/av/quantum_timing.level \
  levels/av/dominik_01.level levels/av/dominik_02.level levels/av/dominik_03.level levels/av/dominik_04.level \
  levels/av/dominik_05.level levels/av/dominik_06.level levels/av/dominik_07.level levels/av/dominik_08.level \
  levels/av/dominik_09.level levels/av/dominik_10.level levels/av/dominik_11.level levels/av/dominik_12.level

                    # BSD make                         # GNU make
LEVEL_INDEX_FILES = $(dist_noinst_DATA:M*/index.level) $(filter %/index.level,$(dist_noinst_DATA))

install-data-local:
	$(mkdir_p) "$(DESTDIR)$(pkgdatadir)/levels"
	for p in $(LEVEL_INDEX_FILES:levels/%/index.level=%); do \
	  cat $(srcdir)/levels/$$p/* | gzip -c -9 > "$(DESTDIR)$(pkgdatadir)/levels/$$p.gz"; \
	  chmod 644 "$(DESTDIR)$(pkgdatadir)/levels/$$p.gz"; \
	done

check: $(bin_PROGRAMS)
	./pushover -c recordings/finished/*
	./pushover -y recordings/mustFail
	./pushover -x recordings/mustCrash

abs_builddir = @abs_builddir@
win32dir = $(PACKAGE)-$(VERSION)-win32

dist-win32: all
	$(RM) -r   '$(abs_builddir)/$(win32dir)'
	$(MAKE) \
	    prefix='$(abs_builddir)/$(win32dir)' \
	    bindir='$(abs_builddir)/$(win32dir)' \
	    docdir='$(abs_builddir)/$(win32dir)' \
	    pkgdatadir='$(abs_builddir)/$(win32dir)' \
	    datarootdir='$(abs_builddir)/$(win32dir)' \
	    install-strip
	for p in $(bin_PROGRAMS); do \
	    upx --lzma '$(abs_builddir)/$(win32dir)'/$$p; \
	done
	cd $(srcdir) && for FILE in $(dist_doc_DATA); do \
	    sed 's,$$,\r,' < $$FILE > $(abs_builddir)/$(win32dir)/$$FILE.txt; \
	    rm $(abs_builddir)/$(win32dir)/$$FILE; \
	    done
	$(RM)      '$(abs_builddir)/$(win32dir).zip'
	cd '$(abs_builddir)' && zip -r9 $(win32dir).zip $(win32dir)
	$(RM) -r   '$(abs_builddir)/$(win32dir)'

