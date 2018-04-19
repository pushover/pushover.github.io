CXXFLAGS := -Wall -g -O2

DATADIR := /usr/share

CROSS :=
CXX := $(CROSS)g++
MSGFMT := msgfmt
MSGMERGE := msgmerge
PKG_CONFIG := $(CROSS)pkg-config
POVRAY := povray
XGETTEXT := xgettext

MSGID_BUGS_ADDRESS := roever@users.sf.net

FILES_H := $(wildcard src/*.h src/linebreak/*.h)
FILES_CPP := $(wildcard src/*.cpp src/linebreak/*.c)
FILES_O := $(patsubst src/%,build_tmp/o/%.o,$(FILES_CPP))
FILES_PO := $(wildcard po/*.po)
FILES_MO := $(patsubst po/%.po,pushover_data/locale/%/LC_MESSAGES/pushover.mo,$(FILES_PO))
FILES_DATA_SRC := $(wildcard data/*.ogg data/*.png)
FILES_DATA := $(patsubst data/%,pushover_data/pushover/data/%,$(FILES_DATA_SRC))
FILES_THEMES_SRC := $(wildcard themes/*)
FILES_THEMES := $(patsubst themes/%,pushover_data/pushover/themes/%,$(FILES_THEMES_SRC))
FILES_LEVELS_SRC := $(wildcard levels/*/*)
FILES_LEVELS_SRCDIRS := $(wildcard levels/*)
FILES_LEVELS := $(patsubst levels/%,pushover_data/pushover/levels/%.gz,$(FILES_LEVELS_SRCDIRS))
FILES_GENERATED_SRC := generated/dominos.png
FILES_GENERATED := pushover_data/pushover/data/dominos.png
FILES_EXTRA := AUTHORS COPYING Makefile NEWS pushover.ico README
FILES_DATADIR := $(FILES_MO) $(FILES_DATA) $(FILES_THEMES) $(FILES_LEVELS) $(FILES_GENERATED)
FILES_DIST := src/version $(FILES_EXTRA) $(FILES_H) $(FILES_CPP) $(FILES_PO) $(FILES_DATA_SRC) $(FILES_THEMES_SRC) $(FILES_LEVELS_SRC) $(FILES_GENERATED_SRC)
VERSION := $(shell cat src/version)
PKG_LUA := $(shell pkg-config --exists lua-5.2 && echo lua-5.2 || echo lua)

DIST := pushover-$(VERSION).tgz

DEF_VERSION := -DVERSION='"$(VERSION)"'
DEF_DATADIR := -DDATADIR='"$(DATADIR)"'

PKGS += $(PKG_LUA)
PKGS += SDL_mixer
PKGS += SDL_ttf
PKGS += fribidi
PKGS += libpng
PKGS += sdl
PKGS += zlib

LIBS += -lboost_filesystem
LIBS += -lboost_system

PKGS_ASSEMBLER += SDL_image
PKGS_ASSEMBLER += libpng
PKGS_ASSEMBLER += sdl

.DELETE_ON_ERROR:

.PHONY: default
default: pushover $(FILES_DATADIR)

.PHONY: dist
dist: $(DIST)

.PHONY: check
check: pushover $(FILES_DATADIR)
	./pushover -c recordings/finish/*
	./pushover -y recordings/fail
	./pushover -x recordings/crash
	@echo OK

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

.SECONDARY: $(FILES_O)
build_tmp/o/%.o: src/% src/version $(FILES_H)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) `$(PKG_CONFIG) --cflags $(PKGS)` $(DEF_VERSION) $(DEF_DATADIR) -c -o $@ $<

pushover: $(FILES_O)
	$(CXX) $(CXXFLAGS) `$(PKG_CONFIG) --libs $(PKGS)` $(LIBS) -o $@ $(FILES_O)

.SECONDARY: build_tmp/assembler
build_tmp/assembler: data/sources/assembler.cpp data/sources/pngsaver.h
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) `$(PKG_CONFIG) --cflags $(PKGS_ASSEMBLER)` `$(PKG_CONFIG) --libs $(PKGS_ASSEMBLER)` -o $@ $<

.SECONDARY: build_tmp/domino_images/done
build_tmp/domino_images/done: data/sources/domino.ini data/sources/*.pov
	mkdir -p $(dir $@)
	$(POVRAY) $<
	touch $@

generated/dominos.png: build_tmp/assembler build_tmp/domino_images/done
	mkdir -p $(dir $@)
	build_tmp/assembler $@ 58 2 200 data/sources/dominos.lst

pushover_data/locale/%/LC_MESSAGES/pushover.mo: po/%.po
	mkdir -p $(dir $@)
	$(MSGFMT) -c -o $@ $<

pushover_data/pushover/data/%: data/%
	mkdir -p $(dir $@)
	cp $< $@

pushover_data/pushover/data/dominos.png: generated/dominos.png
	mkdir -p $(dir $@)
	cp $< $@

pushover_data/pushover/themes/%: themes/%
	mkdir -p $(dir $@)
	cp $< $@

pushover_data/pushover/levels/%.gz: levels/%/*.level
	mkdir -p $(dir $@)
	cat levels/$*/*.level | gzip -9 >$@

build_tmp/po/leveltexts.cpp: levels/*/*.level
	mkdir -p $(dir $@)
	sed -n '/^\(Description\|Hint\|Name\|Tutorial\)$$/,/^[^|]/ s,^| \(.*\)$$,_("\1"),p' $^ | LC_ALL=C sort -u >$@

build_tmp/po/messages.pot: build_tmp/po/leveltexts.cpp src/*.cpp
	mkdir -p $(dir $@)
	$(XGETTEXT) --msgid-bugs-address=$(MSGID_BUGS_ADDRESS) -cTRANSLATORS: -k_ -kN_ -o $@ $^

$(DIST): $(FILES_DIST)
	rm -rf build_tmp/$(basename $@)
	mkdir -p build_tmp/$(basename $@)
	tar c $^ | tar x -C build_tmp/$(basename $@)
	tar c -C build_tmp $(basename $@) | gzip -9n >$@
