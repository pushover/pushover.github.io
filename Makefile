CXXFLAGS := -Wall -g -O2

PREFIX := /usr
BINDIR := $(PREFIX)/bin
DATADIR := $(PREFIX)/share
DESTDIR :=

CROSS :=

CONVERT := convert
CXX := $(CROSS)g++
INSTALL := install
MSGFMT := msgfmt
MSGMERGE := msgmerge
PKG_CONFIG := $(CROSS)pkg-config
POVRAY := povray
STRIP := $(CROSS)strip
XGETTEXT := xgettext

PKG_LUA := $(shell $(PKG_CONFIG) --exists lua-5.2 && echo lua-5.2 || echo lua)

MSGID_BUGS_ADDRESS := roever@users.sf.net

.DELETE_ON_ERROR:

.PHONY: default
default: all

VERSION := $(shell cat src/version)
FILES_DIST += src/version

PKGS += $(PKG_LUA)
PKGS += SDL_mixer
PKGS += SDL_ttf
PKGS += fribidi
PKGS += libpng
PKGS += sdl
PKGS += zlib

LIBS += -lboost_filesystem
LIBS += -lboost_system

DEFS += -DVERSION='"$(VERSION)"'
DEFS += -DDATADIR='"$(DATADIR)"'

FILES_H := $(wildcard src/*.h src/linebreak/*.h src/sha1/*.hpp)
FILES_DIST += $(FILES_H)
FILES_CPP := $(wildcard src/*.cpp src/linebreak/*.c src/sha1/*.cpp)
FILES_DIST += $(FILES_CPP)
FILES_O := $(patsubst src/%,build_tmp/o/%.o,$(FILES_CPP))

.SECONDARY: $(FILES_O)
build_tmp/o/%.o: src/% src/version $(FILES_H)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) `$(PKG_CONFIG) --cflags $(PKGS)` $(DEFS) -c -o $@ $<

FILES_DEBUG += pushover.debug

pushover.debug: $(FILES_O)
	$(CXX) $(CXXFLAGS) `$(PKG_CONFIG) --libs $(PKGS)` $(LIBS) -o $@ $(FILES_O)

FILES_BINDIR += pushover

pushover: pushover.debug
	cp $< $@
	$(STRIP) -s $@

ASSEMBLER_PKGS += SDL_image
ASSEMBLER_PKGS += libpng
ASSEMBLER_PKGS += sdl

.SECONDARY: build_tmp/assembler
build_tmp/assembler: data/sources/assembler.cpp data/sources/pngsaver.h
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) `$(PKG_CONFIG) --cflags $(ASSEMBLER_PKGS)` `$(PKG_CONFIG) --libs $(ASSEMBLER_PKGS)` -o $@ $<

.SECONDARY: build_tmp/domino_images/done
build_tmp/domino_images/done: data/sources/domino.ini data/sources/*.pov
	mkdir -p $(dir $@)
	$(POVRAY) $<
	touch $@

FILES_DIST += generated/dominos.png
FILES_DATADIR += pushover_data/pushover/data/dominos.png

generated/dominos.png: build_tmp/assembler build_tmp/domino_images/done
	mkdir -p $(dir $@)
	build_tmp/assembler $@ 58 2 200 data/sources/dominos.lst

FILES_PO := $(wildcard po/*.po)
FILES_DIST += $(FILES_PO)
FILES_DATADIR += $(patsubst po/%.po,pushover_data/locale/%/LC_MESSAGES/pushover.mo,$(FILES_PO))

pushover_data/locale/%/LC_MESSAGES/pushover.mo: po/%.po
	mkdir -p $(dir $@)
	$(MSGFMT) -c -o $@ $<

FILES_DATA_SRC := $(wildcard data/*.ogg data/*.png)
FILES_DIST += $(FILES_DATA_SRC)
FILES_DATADIR += $(patsubst data/%,pushover_data/pushover/data/%,$(FILES_DATA_SRC))

pushover_data/pushover/data/%: data/%
	mkdir -p $(dir $@)
	cp $< $@

pushover_data/pushover/data/dominos.png: generated/dominos.png
	mkdir -p $(dir $@)
	cp $< $@

FILES_THEMES_SRC := $(wildcard themes/*)
FILES_DIST += $(FILES_THEMES_SRC)
FILES_DATADIR += $(patsubst themes/%,pushover_data/pushover/themes/%,$(FILES_THEMES_SRC))

pushover_data/pushover/themes/%: themes/%
	mkdir -p $(dir $@)
	cp $< $@

FILES_DIST += pushover.desktop
FILES_DATADIR += pushover_data/applications/pushover.desktop

pushover_data/applications/pushover.desktop: pushover.desktop
	mkdir -p $(dir $@)
	cp $< $@

FILES_DIST += pushover.ico
FILES_DATADIR += pushover_data/icons/hicolor/16x16/apps/pushover.png
FILES_DATADIR += pushover_data/icons/hicolor/32x32/apps/pushover.png
FILES_DATADIR += pushover_data/icons/hicolor/48x48/apps/pushover.png
FILES_DATADIR += pushover_data/icons/hicolor/64x64/apps/pushover.png

pushover_data/icons/hicolor/%/apps/pushover.png: pushover.ico
	mkdir -p $(dir $@)
	$(CONVERT) $<[$(shell expr substr $* 1 2 / 16 - 1)] $@

FILES_DIST += pushover.6
FILES_DATADIR += pushover_data/man/man6/pushover.6.gz

pushover_data/man/man6/pushover.6.gz: pushover.6
	mkdir -p $(dir $@)
	gzip -9n <$< >$@

FILES_DIST += $(wildcard levels/*/*)
FILES_LEVELS_SRCDIRS := $(wildcard levels/*)
FILES_DATADIR += $(patsubst levels/%,pushover_data/pushover/levels/%.gz,$(FILES_LEVELS_SRCDIRS))

pushover_data/pushover/levels/%.gz: levels/%/*.level
	mkdir -p $(dir $@)
	cat levels/$*/*.level | gzip -9 >$@

build_tmp/po/leveltexts.cpp: levels/*/*.level
	mkdir -p $(dir $@)
	sed -n '/^\(Description\|Hint\|Name\|Tutorial\)$$/,/^[^|]/ s,^| \(.*\)$$,_("\1"),p' $^ | LC_ALL=C sort -u >$@

.PHONY: all
all: $(FILES_BINDIR) $(FILES_DATADIR) $(FILES_DEBUG)

DIST := pushover-$(VERSION).tgz

.PHONY: dist
dist: $(DIST)

FILES_DIST += AUTHORS
FILES_DIST += COPYING
FILES_DIST += Makefile
FILES_DIST += NEWS
FILES_DIST += README

$(DIST): $(FILES_DIST)
	rm -rf build_tmp/$(basename $@)
	mkdir -p build_tmp/$(basename $@)
	tar c $^ | tar x -C build_tmp/$(basename $@)
	tar c -C build_tmp $(basename $@) | gzip -9n >$@

.PHONY: check
check: all
	./pushover -c recordings/finish/*
	./pushover -y recordings/fail
	./pushover -x recordings/crash
	@echo OK

FILES_INSTALL += $(patsubst %,$(DESTDIR)$(BINDIR)/%,$(FILES_BINDIR))

$(DESTDIR)$(BINDIR)/%: %
	$(INSTALL) -m 755 -d $(dir $@)
	$(INSTALL) -m 755 $< $@

FILES_INSTALL += $(patsubst pushover_data/%,$(DESTDIR)$(DATADIR)/%,$(FILES_DATADIR))

$(DESTDIR)$(DATADIR)/%: pushover_data/%
	$(INSTALL) -m 755 -d $(dir $@)
	$(INSTALL) -m 644 $< $@

.PHONY: install
install: $(FILES_INSTALL)

.PHONY: clean
clean:
	rm -f pushover
	rm -f $(DIST)
	rm -rf build_tmp/
	rm -rf pushover_data/
	@echo Not removing generated/

.PHONY: update-po
update-po: build_tmp/po/messages.pot
	for PO_FILE in po/*.po; do $(MSGMERGE) -Uq --backup=none $$PO_FILE $<; done

build_tmp/po/messages.pot: build_tmp/po/leveltexts.cpp src/*.cpp
	mkdir -p $(dir $@)
	$(XGETTEXT) --msgid-bugs-address=$(MSGID_BUGS_ADDRESS) -cTRANSLATORS: -k_ -kN_ -o $@ $^
